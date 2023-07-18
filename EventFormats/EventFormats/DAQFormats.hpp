/*
  Copyright (C) 2019-2020 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// DAQFormats.hpp, (c) FASER Detector software
///////////////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>
#include <stdexcept>
#include <chrono>
#include <vector>
#include <iomanip>
#include <map>
#include <fstream>
#include "Exceptions/Exceptions.hpp"
#include <zstd.h>
#include <zlib.h> 
#include "Logging.hpp"
using namespace std::chrono_literals;
using namespace std::chrono;
CREATE_EXCEPTION_TYPE(EFormatException,DAQFormats)
namespace DAQFormats {
  typedef std::vector<uint8_t> byteVector;
  
  //FIXME: add Doxygen

  enum EventTags {
    PhysicsTag = 0x00,
    CalibrationTag = 0x01,
    MonitoringTag = 0x02,
    TLBMonitoringTag = 0x03,
    MaxRegularTag = 0x03,  //  to be updated if new tags are added!
    CorruptedTag = 0x10,
    IncompleteTag = 0x11,
    DuplicateTag = 0x12,
    MaxAnyTag = 0x13       //  to be updated if new error tags are added! Note that is last tag value+1 !
  };

  enum SourceIDs {
    TriggerSourceID = 0x020000,
    TrackerSourceID = 0x030000,
    PMTSourceID     = 0x040000,
    BOBRSourceID    = 0x050000
  };
  
  const uint16_t EventHeaderVersion = 0x0001;

  enum EventStatus { // to be reviewed as not all are relevant for FASER
    UnclassifiedError = 1,
    BCIDMismatch = 1<<1,
    TagMismatch = 1<<2,
    Timeout = 1<<3,
    Overflow = 1<<4,
    CorruptedFragment = 1<<5,
    DummyFragment = 1<<6,
    MissingFragment = 1<<7,  
    EmptyFragment = 1<<8,
    DuplicateFragment = 1<<9,
    ErrorFragment = 1<<10,    // used by event builder to wrap incoming non-deciphable data
    Compressed = 1 << 11 // Indicates a Compressed event
  };

  /** \brief This class define DAQ fragment header encapsulating raw data
   *  from the experiment. Encoding/decoding and access functions are provided
   */
 
  class EventFragment {
  public:

    EventFragment() = delete;

    /** \brief Constructor given a detector payload 
     */
    EventFragment(uint8_t fragment_tag, uint32_t source_id,
		  uint64_t event_id, uint16_t bc_id, const void * payload, size_t payloadsize ) {
      microseconds timestamp;
      timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch());

      header.marker         = FragmentMarker;
      header.fragment_tag   = fragment_tag;
      header.trigger_bits   = 0;
      header.version_number = FragmentVersionLatest;
      header.header_size    = sizeof(struct EventFragmentHeader);
      header.payload_size   = payloadsize;
      header.source_id      = source_id;
      header.event_id       = event_id;
      header.bc_id          = bc_id;
      header.status         = 0;
      header.timestamp      = static_cast<uint64_t>(timestamp.count());
      fragment=byteVector(reinterpret_cast<const uint8_t *>(payload),
			  reinterpret_cast<const uint8_t *>(payload)+payloadsize);
    }
  
    /** \brief Constructor given an already encoded fragment
     */
    EventFragment(const uint8_t *data, size_t size, bool allowExcessData=false) {
      if (size<8) THROW(EFormatException,"Too little data for fragment header");
      const struct EventFragmentHeader* newHeader=reinterpret_cast<const struct EventFragmentHeader*>(data);
      if (newHeader->marker!=FragmentMarker) THROW(EFormatException,"No fragment header");
      if (newHeader->version_number!=FragmentVersionLatest) {
	//FIXMEL should do conversion here
	THROW(EFormatException,"Unsupported fragment version");
      }
      if (size<newHeader->header_size) THROW(EFormatException,"Too little data for fragment header");
      if (size<newHeader->header_size+newHeader->payload_size) THROW(EFormatException,"Too little data for fragment");
      if ((size!=newHeader->header_size+newHeader->payload_size)&&!allowExcessData) THROW(EFormatException,"fragment size does not match header information");
      size=newHeader->header_size+newHeader->payload_size;
      fragment=byteVector(data+newHeader->header_size,data+newHeader->header_size+newHeader->payload_size);
      header=*(reinterpret_cast<const struct EventFragmentHeader *>(data));
    }

    /// \brief Returns the payload as pointer of desired type
    template <typename T = void *> T payload() const {
      static_assert(std::is_pointer<T>(), "Type parameter must be a pointer type");
      return reinterpret_cast<T>(fragment.data());
    }
  
    /// Return fragment as vector of bytes
    const byteVector * raw() const {
      const uint8_t *rawHeader=reinterpret_cast<const uint8_t *>(&header);
      byteVector* data=new byteVector(rawHeader,rawHeader+sizeof(header));
      data->insert(data->end(),fragment.begin(),fragment.end());
      return data;
    }

    /// Append fragment to existing vector of bytes (for building events)
    void rawAppend(byteVector *data) const {
      const uint8_t *rawHeader=reinterpret_cast<const uint8_t *>(&header);
      data->insert(data->end(),rawHeader,rawHeader+sizeof(header));
      data->insert(data->end(),fragment.begin(),fragment.end());
    }
  
    /// Set status bits
    void set_status(uint16_t status) {
      header.status = status;
    } 

    /// Set trigger bits
    void set_trigger_bits(uint16_t trigger_bits) {
      header.trigger_bits = trigger_bits;
    } 

    /// Set fragment tag - should normally not be used
    void set_fragment_tag(uint8_t fragment_tag) {
      header.fragment_tag = fragment_tag;
    } 

    //getters here
    uint64_t event_id() const { return header.event_id; }
    uint8_t  fragment_tag() const { return header.fragment_tag; }
    uint32_t source_id() const { return header.source_id; }
    uint16_t bc_id() const { return header.bc_id; }
    uint16_t status() const { return header.status; }
    uint16_t trigger_bits() const { return header.trigger_bits; }
    uint32_t size() const { return header.header_size+header.payload_size; }
    uint16_t header_size() const { return header.header_size; }
    uint32_t payload_size() const { return header.payload_size; }
    uint64_t timestamp() const { return header.timestamp; }
    
  private:
    const uint16_t FragmentVersionLatest = 0x0001; // Change to indicate the support for compressed event
    /*
    Current Version
    00 01
    Compressed Fragment
    01 01
    */
    const uint8_t FragmentMarker = 0xAA; // indicates good raw data events
    struct EventFragmentHeader {
      uint8_t marker;
      uint8_t fragment_tag;
      uint16_t trigger_bits;
      uint16_t version_number;
      uint16_t header_size;
      uint32_t payload_size;
      uint32_t source_id; 
      uint64_t event_id;
      uint16_t bc_id;
      uint16_t status; 
      uint64_t timestamp;
    }  __attribute__((__packed__)) header;
    byteVector fragment;
  };

    /** \brief This class define DAQ event header encapsulating one or more
     *	event fragments. Encoding/decoding and access functions are provided
     */

  class EventFull {
  public:

    /** \brief Constructor with fragments to be added later
     *  Default parameeters allow this to be used as a default constructor as well
     */
    std::vector<uint8_t> compressedData;

    EventFull(uint8_t event_tag=IncompleteTag, unsigned int run_number=0, uint64_t event_number=0) {
      microseconds timestamp;
      timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch());

      header.marker         = EventMarker;
      header.event_tag      = event_tag;
      header.trigger_bits   = 0;
      header.version_number = EventVersionLatest;
      header.compressionCode = 0x00;
      header.header_size    = sizeof(struct EventHeader);
      header.payload_size   = 0;
      header.fragment_count = 0;
      header.run_number     = run_number;
      header.event_id       = 0;
      header.event_counter  = event_number;
      header.bc_id          = 0xFFFF;
      header.status         = 0;
      header.timestamp      = static_cast<uint64_t>(timestamp.count());
    }

    /// \brief Constructor given an existing event in stream of bytes 
    EventFull(const uint8_t *data,size_t eventsize) {

      // Read EventHeader out of byte stream
      loadHeader(data, eventsize);

      // Skip forward by amount of data actually read
      uint32_t dataLeft = eventsize - header.header_size;
      data+=header.header_size;
      
      if (header.status & 1<<11) // Compressed Event Detected
      { 
        // DEBUG("A Compressed Event is being read");
        std::vector<uint8_t> decompressed_data_vector;
        //Copy the data array into the compressed data vector for processing
        this->compressedData.insert(this->compressedData.begin(),data,data+dataLeft);
        
        // Detect Compression Algorithm And Decompress
        if (decompressPayload(header.compressionCode,compressedData,decompressed_data_vector ))
        {
        uint8_t* decompressed_data = &decompressed_data_vector[0];
        updatePayloadSize(decompressed_data_vector.size());
        loadPayload(decompressed_data,decompressed_data_vector.size());
        decompressed_data_vector.clear();
        this->compressedData.clear();
        delete [] decompressed_data;
        }
        else{
          ERROR("DECOMPRESSION FAILED SKIPPING EVENT READ");
        }
      }
      else{
      // Read payload objects from remaining data
      loadPayload(data, dataLeft);
      }

    }

    /// \brief Constructor reading an existing event from a file stream
    // FIXME: no format migration support or for partially corrupted events
    EventFull(std::ifstream &in) {
      in.read(reinterpret_cast<char *>(&header),sizeof(header));
      if (in.fail()) THROW(EFormatException,"Too small to be event");
      if (header.marker!=EventMarker) THROW(EFormatException,"Wrong event header");
      //uint16_t mask = 0x00ff;
      //if ((header.version_number & mask) != (EventVersionLatest & mask)) 
      if (header.version_number  != EventVersionLatest )
      {
	    //should do conversion here
	    THROW(EFormatException,"Unsupported event format version");
      }
      if (header.payload_size>1000000) THROW(EFormatException,"Payload size too large (>1000000)");
      uint8_t* data=new uint8_t[header.payload_size];
      uint8_t* full=data;
      in.read(reinterpret_cast<char *>(data),header.payload_size);
      if (in.fail()) THROW(EFormatException,"Event size does not match header information");

      uint32_t dataLeft=header.payload_size;
      // Now check if the Event header has Compression enabled
      if (header.status & 1<<11) // Compressed Event Detected
      {
        // DEBUG("A Compressed Event is being read");
        // There seems to be a bug while rendering these values in std out see if this happens for anything else
        uint16_t triggerNow = header.trigger_bits;
        header.trigger_bits = triggerNow & 0xffff; // FIXME
        uint64_t counterNow = header.event_counter;
        header.event_counter = counterNow & 0xffffffff; // FIXME
        std::vector<uint8_t> decompressed_data_vector;
        //Copy the data array into the compressed data vector for processing
        this->compressedData.insert(this->compressedData.begin(),data,data+dataLeft);
        
        // Detect Compression Algorithm And Decompress
        // decompressPayload(header.version_number,compressedData,decompressed_data_vector)
        if (decompressPayload(header.compressionCode,compressedData,decompressed_data_vector ))
        { 
          // DEBUG("Decompression Of Compressed Data Is Successful");
            uint8_t* decompressed_data = &decompressed_data_vector[0];
            updatePayloadSize(decompressed_data_vector.size());
            toggleCompression();
            //setCompressionAlgo(0x0100);
            setCompressionAlgo(this->header.compressionCode); // toggle it off
            dataLeft = decompressed_data_vector.size();
            //loadPayload(decompressed_data,decompressed_data_vector.size());
            for(int fragNum=0;fragNum<header.fragment_count;fragNum++) 
            {
              EventFragment *fragment=new EventFragment(decompressed_data,dataLeft,true);
              decompressed_data+=fragment->size();
              dataLeft-=fragment->size();
              fragments[fragment->source_id()]=fragment;
            } 
              
            decompressed_data_vector.clear();
            this->compressedData.clear();
            //delete [] decompressed_data; // ? Does this get Deallocated some where 
        }
        else{
          ERROR("DECOMPRESSION FAILED SKIPPING EVENT READ");
        } 
      }
      else
      {
        for(int fragNum=0;fragNum<header.fragment_count;fragNum++) 
        {
          EventFragment *fragment=new EventFragment(data,dataLeft,true);
          data+=fragment->size();
          dataLeft-=fragment->size();
          fragments[fragment->source_id()]=fragment;
        }      
      }
      
    delete [] full; 
    }



    /// OR's new error flags into existing ones
    void updateStatus(uint16_t status) {
      header.status|=status;
    }
    void updatePayloadSize(uint32_t size){
      header.payload_size=size;
    }
    void toggleCompression()
    {
      header.status^=1<<11;
    }
    bool isCompressed()
    {
      if(header.status & 1<<11)
        return true;
      else
        return false;
    }
    void setCompressionAlgo(uint8_t algoCode)
    {
        // // header.version is as 0x00 0x00 last byte is for FASER version hence that must not be altered
        // // All Algo Code must be from 0xffff to 0x00ff
        // if (algoCode <= 255)
        // {
        //   WARNING("Invalid Code Passes, Not Altering");
        // }else{
          header.compressionCode ^=algoCode;
        
    }
    
    /// Return full event as vector of bytes
    byteVector* raw() {
      const uint8_t *rawHeader=reinterpret_cast<const uint8_t *>(&header);
      byteVector* full=new byteVector(rawHeader,rawHeader+sizeof(header));
      /* if its a compressed event populate the byteVector with the contents of the 
      compressed payload
      */
      if(this->isCompressed())
      {
        // DEBUG("raw () invoked on compressed event");
        full->insert(full->end(),this->compressedData.begin(),this->compressedData.end());
        return full;
      }
      for(const auto& frag: fragments) {
	        frag.second->rawAppend(full); // Note the key of the hashMap is discarded
      }
      return full;
    }

    byteVector* raw_fragments() {
      //const uint8_t *rawHeader=reinterpret_cast<const uint8_t *>(&header);
      byteVector* fragmentraw=new byteVector();
      for(const auto& frag: fragments) {
	        frag.second->rawAppend(fragmentraw);
      }
      return fragmentraw;
    }

    ~EventFull() {
      if(this->isCompressed())
      {
        // DEBUG("Since Fragments are cleared during compression Skipping that part");
        this->compressedData.clear();
      }
      else{
      for(const auto& frag: this->fragments) { // FIXME: should use smarter pointers for memory management
	    delete frag.second;
      }
      }
      
    }
    /** \brief Appends fragment to list of fragments in event
     *
     *  Ownership is taken of fragment, i.e. don't delete it later
     */
    bool decompressPayload(uint8_t header_compressionCode,std::vector<uint8_t>& compressedFragments, std::vector<uint8_t>& outputFragments)
  {
    //uint16_t mask = 0xff00;
    uint8_t zstd_code = 0x01;
    uint8_t zlib_code = 0x02;
    //if ((header_version_number & mask) == (zstd_mask&mask))
    if (header_compressionCode == zstd_code)
    {
      // DEBUG("Detected ZSTD Compression in Event");
      const size_t maxDecompressedSize = ZSTD_getFrameContentSize(compressedFragments.data(), compressedFragments.size());
      if (maxDecompressedSize == ZSTD_CONTENTSIZE_ERROR || maxDecompressedSize == ZSTD_CONTENTSIZE_UNKNOWN) {
          std::cerr << "Error: Invalid compressed data" << std::endl;
          return false;
      }

    outputFragments.resize(maxDecompressedSize);

    const size_t decompressedSize = ZSTD_decompress(outputFragments.data(), maxDecompressedSize,
                                                    compressedFragments.data(), compressedFragments.size());
    if (ZSTD_isError(decompressedSize)) {
        std::cerr << "Error: Failed to decompress data: " << ZSTD_getErrorName(decompressedSize) << std::endl;
        return false;
    }

    outputFragments.resize(decompressedSize);
    return true;
    }
    //else if ((header_version_number & mask) == (zlib_mask&mask))
    else if (header_compressionCode == zlib_code)
    {
      // DEBUG("Detected Zlib Compression in Event");
      outputFragments.clear();
      uLongf decompressedSize = compressedFragments.size() * 3;
      outputFragments.resize(decompressedSize);
      int result = uncompress(outputFragments.data(), &decompressedSize, compressedFragments.data(), compressedFragments.size());

      if (result != Z_OK) {
          return false;
      }
      outputFragments.resize(decompressedSize);
      return true;
    }
    ERROR("Invalid Compression code detected");
    return false;
  }

    int16_t addFragment(const EventFragment* fragment) { // FIXME: should use smarter pointers for memory management
      int16_t status=0;
      if (fragments.find(fragment->source_id())!=fragments.end()) 
	    THROW(EFormatException,"Duplicate fragment addition!");
      fragments[fragment->source_id()]=fragment;
      if (!header.fragment_count) {
          header.bc_id=fragment->bc_id();
          header.event_id = fragment->event_id();
      }
      header.fragment_count++;
      header.trigger_bits |= fragment->trigger_bits(); //? should only come from trigger fragment - BP: add check?
      header.payload_size+=fragment->size();
      if (header.bc_id!=fragment->bc_id()) {
	status |= EventStatus::BCIDMismatch;
      }
      if (fragment->source_id()==SourceIDs::TriggerSourceID)
	header.bc_id=fragment->bc_id();  //TLB should be primary source BCID

      //BP: could check for event ID mismatch, but should not happen...

      updateStatus(fragment->status()|status);
      return status;
    }

    // \brief Load header from stream of bytes
    // Return actual size of header
    void loadCompressedData(std::vector<uint8_t>& compressedDataToLoad)
    {
      for(const auto& frag: fragments)  // FIXME: should use smarter pointers for memory management
	    delete frag.second;
      // DEBUG("Dealocated memory for all fragments after compression");
      this->compressedData.insert(this->compressedData.end(),compressedDataToLoad.begin(),compressedDataToLoad.end());
      this->updatePayloadSize(compressedData.size());
      this->toggleCompression();
      
    }

    uint16_t loadHeader(const uint8_t *data, size_t datasize) {
      if (datasize<sizeof(struct EventHeader)) THROW(EFormatException,"Too small to be event");
      header=*reinterpret_cast<const struct EventHeader *>(data);
      if (header.marker!=EventMarker) THROW(EFormatException,"Wrong event header");
      //uint16_t mask = 0x00ff;
      //if ((header.version_number & mask) != (EventVersionLatest & mask)) { // * Added support for compression algo in version
	    //should do conversion here
      if (header.version_number == EventVersionLatest){
	    THROW(EFormatException,"Unsupported event format version");
      }

      return header_size(); 
    }

    // \brief Load payload from stream of bytes
    void loadPayload(const uint8_t *data, size_t datasize) {
    if (datasize != header.payload_size) {
    THROW(EFormatException, "Payload size does not match header information");
    }
    for(int fragNum=0;fragNum<header.fragment_count;fragNum++) {
        EventFragment *fragment=new EventFragment(data,datasize,true);
        data+=fragment->size();
        datasize-=fragment->size();
        fragments[fragment->source_id()]=fragment;
      }
    }

    // getters here
    uint8_t event_tag() const { return header.event_tag; }
    uint16_t status() const { return header.status; }
    uint64_t event_id() const { return header.event_id; }
    uint64_t event_counter() const { return header.event_counter; }
    uint16_t bc_id() const { return header.bc_id; }
    uint32_t size() const { return header.header_size+header.payload_size; }
    uint16_t header_size() const { return header.header_size; }
    uint32_t payload_size() const { return header.payload_size; }
    uint64_t timestamp() const { return header.timestamp; }
    uint64_t run_number() const { return header.run_number; }
    uint16_t trigger_bits() const { return header.trigger_bits; }
    uint16_t fragment_count() const { return header.fragment_count; }
    uint8_t event_version() const { return header.version_number; }
    uint8_t event_compressionCode() const { return header.compressionCode; }

    /// Get list of fragment source ids
    std::vector<uint32_t> getFragmentIDs() const
    {
      std::vector<uint32_t> ids;
      for (const auto &frag : fragments)
      {
        ids.push_back(frag.first);
      }
      return ids;
    }

    /// Find fragment with specific source id
    const EventFragment *find_fragment(uint32_t source_id) const
    {
      if (fragments.find(source_id) == fragments.end())
          return nullptr;
      return fragments.find(source_id)->second;
    }

  private:
    const uint8_t EventVersionLatest = 0x01;
    const uint8_t EventMarker = 0xBB;
    struct EventHeader {
      uint8_t marker;
      uint8_t event_tag;
      uint16_t trigger_bits;
      uint8_t version_number; // Could be used to record algorithm for compression
      uint8_t compressionCode;
      uint16_t header_size;
      uint32_t payload_size;
      uint8_t  fragment_count;
      unsigned int run_number : 24;
      uint64_t event_id;  
      uint64_t event_counter;
      uint16_t bc_id;
      uint16_t status; 
      uint64_t timestamp;
    }  __attribute__((__packed__)) header;
    std::map<uint32_t,const EventFragment*> fragments;
    //std::vector<uint8_t> compressedData;
    // pointer to compressed event
  };

}

inline std::ostream &operator<<(std::ostream &out, const  DAQFormats::EventFragment &frag) {
  out<<" Fragment: tag="<<static_cast<int>(frag.fragment_tag())
     <<" source=0x"<<std::hex<<std::setfill('0')<<std::setw(4)<<std::hex<<frag.source_id()
     <<" bc="<<std::dec<<std::setfill(' ')<<std::setw(4)<<frag.bc_id()
     <<" status=0x"<<std::hex<<std::setw(4)<<std::setfill('0')<<frag.status()
     <<" payload="<<std::dec<<std::setfill(' ')<<std::setw(5)<<frag.payload_size()
     <<" bytes";
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const  DAQFormats::EventFull &ev) {
    out<<"Event: "<<std::setw(8)<<ev.event_counter()<<" (0x"<<std::hex<<std::setfill('0') <<std::setw(8)<<std::right<<ev.event_id()<<") "
       <<std::setfill(' ')
       <<" run="<<std::dec<<ev.run_number()
       <<" tag="<<std::dec<<static_cast<int>(ev.event_tag())
       <<" bc="<<std::dec<<std::setw(4)<<ev.bc_id()
       <<" trig=0x"<<std::hex<<std::setfill('0')<<std::setw(4)<<std::right<<ev.trigger_bits()
       <<" status=0x"<<std::hex<<std::setw(5)<<static_cast<int>(ev.status())
       <<std::setfill(' ')
       <<" Version= "<<std::dec<<static_cast<int>(ev.event_version())
       <<" Compression Code= "<<std::dec<<static_cast<int>(ev.event_compressionCode())
       <<" time="<<std::dec<<ev.timestamp()  //FIXME: should be readable
       <<" #fragments="<<ev.fragment_count()
       <<" payload="<<std::dec<<std::setw(6)<<ev.payload_size()
       <<" bytes";
    return out;
}

#define customdatatypeList (DataFragment<EventFull>)(DataFragment<EventFragment>)

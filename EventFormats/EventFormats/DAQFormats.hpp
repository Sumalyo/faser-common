#pragma once

#include <stdint.h>
#include <stdexcept>
#include <chrono>
#include <vector>
#include <iomanip>
#include <map>
#include <fstream>
#include "Exceptions/Exceptions.hpp"

using namespace std::chrono_literals;
using namespace std::chrono;

namespace DAQFormats {
  typedef std::vector<uint8_t> byteVector;

  class EFormatException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };
  
  //FIXME: add Doxygen

  enum EventTags {
    PhysicsTag = 0x00,
    CalibrationTag = 0x01,
    MonitoringTag = 0x02,
    TLBMonitoringTag = 0x03
  };

  enum SourceIDs {
    TriggerSourceID = 0x020000,
    TrackerSourceID = 0x030000,
    PMTSourceID = 0x040000
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
    DuplicateFragment = 1<<9
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
      header.timestamp      = timestamp.count();
      fragment=byteVector(reinterpret_cast<const uint8_t *>(payload),
			  reinterpret_cast<const uint8_t *>(payload)+payloadsize);
    }
  
    /** \brief Constructor given an already encoded fragment
     */
    EventFragment(const uint8_t *data,size_t size, bool allowExcessData=false) {
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

    //getters here
    uint64_t event_id() const { return header.event_id; }
    uint8_t  fragment_tag() const { return header.fragment_tag; }
    uint32_t source_id() const { return header.source_id; }
    uint16_t bc_id() const { return header.bc_id; }
    uint16_t status() const { return header.status; }
    uint16_t trigger_bits() const { return header.trigger_bits; }
    uint32_t size() const { return header.header_size+header.payload_size; }
    uint32_t payload_size() const { return header.payload_size; }
    uint64_t timestamp() const { return header.timestamp; }
    
  private:
    const uint16_t FragmentVersionLatest = 0x0001;
    const uint8_t FragmentMarker = 0xAA;
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
     */

    EventFull(uint8_t event_tag, unsigned int run_number,uint64_t event_number) {
      microseconds timestamp;
      timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch());

      header.marker         = EventMarker;
      header.event_tag      = event_tag;
      header.trigger_bits   = 0;
      header.version_number = EventVersionLatest;
      header.header_size    = sizeof(struct EventHeader);
      header.payload_size   = 0;
      header.fragment_count = 0;
      header.run_number     = run_number;
      header.event_id       = 0;
      header.event_counter  = event_number;
      header.bc_id          = 0xFFFF;
      header.status         = 0;
      header.timestamp      = timestamp.count();
    }

    /// \brief Constructor given an existing event in stream of bytes 
    EventFull(const uint8_t *data,size_t eventsize) {
      if (eventsize<sizeof(struct EventHeader)) THROW(EFormatException,"Too small to be event");
      header=*reinterpret_cast<const struct EventHeader *>(data);
      if (header.marker!=EventMarker) THROW(EFormatException,"Wrong event header");
      if (header.version_number!=EventVersionLatest) {
	//should do conversion here
	THROW(EFormatException,"Unsupported event format version");
      }
      if (size()!=eventsize) THROW(EFormatException,"Event size does not match header information");

      data+=header.header_size;
      uint32_t dataLeft=header.payload_size;

      for(int fragNum=0;fragNum<header.fragment_count;fragNum++) {
	EventFragment *fragment=new EventFragment(data,dataLeft,true);
	data+=fragment->size();
	dataLeft-=fragment->size();
	fragments[fragment->source_id()]=fragment;
      }
    }

    /// \brief Constructor reading an existing event from a file stream
    // FIXME: no format migration support or for partially corrupted events
    EventFull(std::ifstream &in) {
      in.read(reinterpret_cast<char *>(&header),sizeof(header));
      if (in.fail()) THROW(EFormatException,"Too small to be event");
      if (header.marker!=EventMarker) THROW(EFormatException,"Wrong event header");
      if (header.version_number!=EventVersionLatest) {
	//should do conversion here
	THROW(EFormatException,"Unsupported event format version");
      }
      if (header.payload_size>1000000) THROW(EFormatException,"Payload size too large (>1000000)");
      uint8_t* data=new uint8_t[header.payload_size];
      uint8_t* full=data;
      in.read(reinterpret_cast<char *>(data),header.payload_size);
      if (in.fail()) THROW(EFormatException,"Event size does not match header information");

      uint32_t dataLeft=header.payload_size;

      for(int fragNum=0;fragNum<header.fragment_count;fragNum++) {
	EventFragment *fragment=new EventFragment(data,dataLeft,true);
	data+=fragment->size();
	dataLeft-=fragment->size();
	fragments[fragment->source_id()]=fragment;
      }
      delete [] full;
    }



    /// OR's new error flags into existing ones
    void updateStatus(uint16_t status) {
      header.status|=status;
    }

    /// Return full event as vector of bytes
    byteVector* raw() {
      const uint8_t *rawHeader=reinterpret_cast<const uint8_t *>(&header);
      byteVector* full=new byteVector(rawHeader,rawHeader+sizeof(header));
      for(const auto& frag: fragments) {
	frag.second->rawAppend(full);
      }
      return full;
    }

    ~EventFull() {
      for(const auto& frag: fragments) { // FIXME: should use smarter pointers for memory management
	delete frag.second;
      }
    }
    /** \brief Appends fragment to list of fragments in event
     *
     *  Ownership is taken of fragment, i.e. don't delete it later
     */

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
      header.trigger_bits |= fragment->trigger_bits(); //should only come from trigger fragment - BP: add check?
      header.payload_size+=fragment->size();
      if (header.bc_id!=fragment->bc_id()) {
	status |= EventStatus::BCIDMismatch;
      }
      //BP: could check for event ID mismatch, but should not happen...

      updateStatus(fragment->status()|status);
      return status;
    }

    // getters here
    uint8_t event_tag() const { return header.event_tag; }
    uint8_t status() const { return header.status; }
    uint64_t event_id() const { return header.event_id; }
    uint64_t event_counter() const { return header.event_counter; }
    uint16_t bc_id() const { return header.bc_id; }
    uint32_t size() const { return header.header_size+header.payload_size; }
    uint32_t payload_size() const { return header.payload_size; }
    uint64_t timestamp() const { return header.timestamp; }
    uint64_t run_number() const { return header.run_number; }
    uint16_t trigger_bits() const { return header.trigger_bits; }
    uint16_t fragment_count() const { return header.fragment_count; }

    /// Get list of fragment source ids
    std::vector<uint32_t> getFragmentIDs() {
      std::vector<uint32_t> ids;
      for(const auto & frag : fragments) {
	ids.push_back(frag.first);
      }
      return ids;
    }
    
    /// Find fragment with specific source id
    const EventFragment* find_fragment(uint32_t source_id) const {
      if (fragments.find(source_id)==fragments.end()) return nullptr;
      return fragments.find(source_id)->second;
    }

  private:
    const uint16_t EventVersionLatest = 0x0001;
    const uint8_t EventMarker = 0xBB;
    struct EventHeader {
      uint8_t marker;
      uint8_t event_tag;
      uint16_t trigger_bits;
      uint16_t version_number;
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
    out<<"Event: "<<std::setw(8)<<ev.event_counter()<<" (0x"<<std::hex<<std::setfill('0') <<std::setw(8)<<ev.event_id()<<") "
       <<std::setfill(' ')
       <<" tag="<<std::dec<<static_cast<int>(ev.event_tag())
       <<" bc="<<std::dec<<std::setw(4)<<ev.bc_id()
       <<" trig=0x"<<std::hex<<std::setfill('0')<<std::setw(4)<<ev.trigger_bits()
       <<" status=0x"<<std::hex<<std::setw(4)<<static_cast<int>(ev.status())
       <<std::setfill(' ')
       <<" time="<<std::dec<<ev.timestamp()  //FIXME: should be readable
       <<" #fragments="<<ev.fragment_count()
       <<" payload="<<std::dec<<std::setw(6)<<ev.payload_size()
       <<" bytes";
    return out;
}



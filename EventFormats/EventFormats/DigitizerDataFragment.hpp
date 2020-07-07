#pragma once
#include <bitset>
#include <cstring> //memcpy
#include "Exceptions/Exceptions.hpp"
#include "Logging.hpp"

#define N_MAX_CHAN 16

//#define CERR std::cout<<__LINE__<<std::endl;

/*! A test class */

static inline unsigned int GetBit(unsigned int word, int bit_location){
  // obtain the value of a particular bit in a word
  word =  (word>>bit_location);
  word &= 0x1;
  return word;
}

class DigitizerDataException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };

struct DigitizerDataFragment { 

////////////////////////////////////////////////////
/// Constructor for creating a parsed digitizer event fragment
////////////////////////////////////////////////////  
  DigitizerDataFragment( const uint32_t *data, size_t size ) {
    m_size = size;
    
    // is there at least a header
    if( size < 16 ){
      //      ERROR("Cannot find a header with at least 4 32 bit words");
      //      ERROR("Data size : "<<size);
      THROW(DigitizerDataException, "The fragment is not big enough to even be a header");
    }

    std::cout<<"Word[0] : "<<data[0]<<std::endl;
    
    // decode header
    event.event_size            = data[0] & 0x0FFFFFFF;
    event.board_id              = data[1] >> 27;
    event.board_fail_flag       = GetBit(data[1], 26);
    event.pattern_trig_options  = (data[1] & 0x00FFFFFF) >> 8;
    event.channel_mask          = (data[1] & 0x000000FF) | ((data[2] & 0xFF000000) >> 16);
    event.event_counter         = data[2] & 0x00FFFFFF;
    event.trigger_time_tag      = data[3];

    // check the consistency of the apparent size of the event and the size recorded in the payload
    // note that you need to multiply by 4 because the size is given in bytes of 8 bits but the event size is encoded
    // as the number of 32 bit words
    if( (event.event_size*4) != size ){
      //      ERROR("Expected and observed size of payload do not agree");
      ERROR("Expected = "<<size<<"  vs.  Observed = "<<event.event_size*4);
      THROW(DigitizerDataException, "Mismatch in payload size and expected size");
    }
    
    // parse the ADC count data
    // subtract 4 for the header to get the size of the data payload
    unsigned int event_size_no_header = event.event_size-4;

    // count the number of active channels
    unsigned int n_channels_active=0;
    for(int iChan=0; iChan<N_MAX_CHAN; iChan++){
      if( GetBit(event.channel_mask,iChan)==1 )
        n_channels_active++;
    }

    // divide modified event size by number of channels
    if(event_size_no_header%n_channels_active != 0){
      //      ERROR("The amount of data and the number of channels are not divisible");
      //      ERROR("DataLength = "<<event_size_no_header<<"  /  NChannels = "<<n_channels_active);
      THROW(DigitizerDataException, "Mismatch in data length and number of enabled channels");
    }
    unsigned int words_per_channel = event_size_no_header/n_channels_active;

    // there are two readings per word
    unsigned int samples_per_channel = 2*words_per_channel;
    event.n_samples = samples_per_channel;

    // location of pointer to start at begin of channel
    // starts at 4 because that is size of header
    unsigned int current_start_location = 4;
    
    for(int iChan=0; iChan<N_MAX_CHAN; iChan++){
        
      // make an empty vector entry for the data for every channel, regardless of whether it was enabled
      event.adc_counts[iChan] = {};
      
      // only fill it if it is enabled
      if( GetBit(event.channel_mask,iChan)==0 )
        continue;
      
      for(unsigned int iDat=current_start_location; iDat<current_start_location+words_per_channel; iDat++){
        // two readings are stored in one word
        unsigned int chData = data[iDat];

        // the data is actually arranged in a perhaps nonintuitive way
        // the top half of the word is actually the second made in this doublet
        // while the bottom half of the word is the first measurement
        unsigned int adccount_second_measurement    = (chData & 0xFFFF0000) >> 16;  // sample[n+1]
        unsigned int adccount_first_measurement     = (chData & 0x0000FFFF);        // sample[n]

        event.adc_counts[iChan].push_back(adccount_first_measurement );
        event.adc_counts[iChan].push_back(adccount_second_measurement );

      }

      // move the starting location to the start of the next channels data
      current_start_location += words_per_channel;
    }
      
  }

////////////////////////////////////////////////////
/// Check the validity of the fragment that has been parsed
/// 
/// This checks the following
/// - The full fragment size is as expected and matches the size that the fragment
///   in the EventPayload finds
/// - The number of samples in all channels with event data (as specified in the
///   channel mask) have the same number of samples
////////////////////////////////////////////////////
  bool valid() const {
    bool validityFlag = true; // assume innocence until proven guilty
    
    // perform check to ensure that the decoded readouts, for active channels
    // have the same length
    for(int iChan=0; iChan<N_MAX_CHAN; iChan++){
      // only check enabled channels
      if( this->channel_has_data(iChan) ){
        if( this->channel_adc_counts(iChan).size()!=event.n_samples){
	  //          ERROR("The number of samples for channel="<<iChan<<" is not as expected");
	  //          ERROR("Expected="<<event.n_samples<<"  Actual="<<this->channel_adc_counts(iChan).size()<<std::endl);
          validityFlag = false;
        }
      }
    }
  
    return validityFlag;
  }

  public:
////////////////////////////////////////////////////
/// Retrieves the total size of the event, including the header.  This should be equal to :
/// - 4 words for the header
/// - N*(M/2) words for the readout data where N is the number of channels enabled for readout and M is the length of the buffer
////////////////////////////////////////////////////
    uint32_t event_size() const { return event.event_size; }
    
////////////////////////////////////////////////////
/// Retrieves the hardware board identifier
////////////////////////////////////////////////////
    uint32_t board_id() const { return event.board_id; }
    
////////////////////////////////////////////////////
/// Retrieves the board failure flag (a single bit) which should be set to 0 if everything is fine
/// and is set to 1 in the case of a hardware issue (e.g. PLL unlock).  
////////////////////////////////////////////////////
    uint32_t board_fail_flag() const { return event.board_fail_flag; }
    
////////////////////////////////////////////////////
/// Retrieves the 16 bit pattern that was on the LVDS I/O flat cable upon the trigger reception
////////////////////////////////////////////////////
    uint32_t pattern_trig_options() const { return event.pattern_trig_options; }
    
////////////////////////////////////////////////////
/// Retrieves the channel mask that specifies which channels were enabled for readout in a given event.
/// This data can also be accessed via the channel_has_data() method
////////////////////////////////////////////////////
    uint32_t channel_mask() const { return event.channel_mask; }
    
////////////////////////////////////////////////////
/// Retrieves the local event counter of all triggered events
////////////////////////////////////////////////////
    uint32_t event_counter() const { return event.event_counter; }
    
////////////////////////////////////////////////////
/// Retrieves the Trigger Time Tag
////////////////////////////////////////////////////
    uint32_t trigger_time_tag() const { return event.trigger_time_tag; }
    
////////////////////////////////////////////////////
/// Retrieves the number of samples in the buffer for a single channel
////////////////////////////////////////////////////
    unsigned int n_samples() const { return event.n_samples; }
    
////////////////////////////////////////////////////
/// Retrieves a copy of the full ADC count structure for all channels. This is available for completeness
/// but it is recommended to retrieve data from a single channel at a time using the channel_adc_counts()
/// function which simply provides a reference 
////////////////////////////////////////////////////
    std::map<int, std::vector<uint16_t> > adc_counts() const { return event.adc_counts; }
    
////////////////////////////////////////////////////
/// Retrieves the data for a single channel, regardless of whether that channel was enabled for reading.
/// If it was not enabled, it will print a WARNING to the screen but proceed to give you empty data.
////////////////////////////////////////////////////
    const std::vector<uint16_t>& channel_adc_counts(int channel) const {
      
      // verify that the channel requested is in the map of adc counts
      if( event.adc_counts.find(channel)==event.adc_counts.end()){
	//        ERROR("You are requesting data for channel "<<channel<<" for which there is no entry in the adc counts map.");
        THROW(DigitizerDataException, "The requested channel is not in the adc counts map.");
      }
      
      // if they requested data from an empty channel, then tell them
      if( GetBit(event.channel_mask, channel)==false ){
	WARNING("You are requesting data for channel "<<channel<<" which was not enabled for reading in data taking.  Are you sure you want to use this?");
      }
      
      return event.adc_counts.find(channel)->second;
    
    }
    
////////////////////////////////////////////////////
/// Helper function to let you determine if a given channel has data stored after decoding the event
////////////////////////////////////////////////////
    bool channel_has_data(int channel) const {
      return GetBit(event.channel_mask, channel);
    }
    
////////////////////////////////////////////////////
/// Retrieves the size of the full event fragment, including the header as the number of 8 bit words
////////////////////////////////////////////////////
    size_t size() const { return m_size; }
    
////////////////////////////////////////////////////
/// Sets the debug flag to on for the decoding object
////////////////////////////////////////////////////
    void set_debug_on( bool debug = true ) { m_debug = debug; }

  private:
    struct DigitizerEvent {
      uint32_t event_size;   /// The total size of the event including the header
      uint32_t board_id;
      bool     board_fail_flag;
      bool     event_format; // should always be 0
      uint16_t pattern_trig_options;
      uint16_t channel_mask;
      uint32_t event_counter;
      uint32_t trigger_time_tag;
      
      unsigned int n_samples;
      std::map<int, std::vector<uint16_t> > adc_counts;
    } event;
    size_t m_size; // number of words in full fragment
    bool m_debug = false;
};

inline std::ostream &operator<<(std::ostream &out, const DigitizerDataFragment &event) {
  try {
    out<<"Digitizer Fragment"<<std::endl
       <<std::setw(30)<<" event_size:           "<<std::setfill(' ')<<std::setw(32)<<std::hex<<event.event_size()<<std::setfill(' ')<<std::endl
       <<std::setw(30)<<" board_id:             "<<std::setfill(' ')<<std::setw(32)<<std::dec<<event.board_id()<<std::setfill(' ')<<std::endl
       <<std::setw(30)<<" board_fail_flag:      "<<std::setfill(' ')<<std::setw(32)<<std::dec<<event.board_fail_flag()<<std::setfill(' ')<<std::endl
       <<std::setw(30)<<" pattern_trig_options: "<<std::setfill(' ')<<std::setw(32)<<std::dec<<event.pattern_trig_options()<<std::setfill(' ')<<std::endl
       <<std::setw(30)<<" channel_mask:         "<<std::setfill(' ')<<std::setw(32)<<std::dec<<event.channel_mask()<<std::setfill(' ')<<std::endl
       <<std::setw(30)<<" event_counter:        "<<std::setfill(' ')<<std::setw(32)<<std::dec<<event.event_counter()<<std::setfill(' ')<<std::endl
       <<std::setw(30)<<" trigger_time_tag:     "<<std::setfill(' ')<<std::setw(32)<<std::dec<<event.trigger_time_tag()<<std::setfill(' ')<<std::endl;

    // print global header of channels
    out<<std::setw(10)<<std::dec<<"Time"<<"|";
    for(int iChan=0; iChan<N_MAX_CHAN; iChan++){
      out<<std::setw(6)<<std::dec<<iChan<<"["<<GetBit(event.channel_mask(), iChan)<<"]";
    }
    out<<std::endl;
    
    // print data after checking that channels were enabled
    for(unsigned int iSamp=0; iSamp<event.n_samples(); iSamp++){
      out<<std::setw(10)<<std::dec<<iSamp<<"|";
      for(int iChan=0; iChan<N_MAX_CHAN; iChan++){
        if( event.channel_has_data(iChan) ){
          out<<std::setw(9)<<std::dec<<event.adc_counts()[iChan].at(iSamp);
        }
        else{
          out<<std::setw(9)<<" - ";
        }  
      }
      out<<std::endl;
    }

  } catch ( DigitizerDataException& e ) {
    out<<e.what()<<std::endl;
    out<<"Corrupted data for Digitizer event"<<std::endl;
  }

 return out;
}


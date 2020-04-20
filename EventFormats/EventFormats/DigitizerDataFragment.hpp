#pragma once
#include <bitset>
#include <cstring> //memcpy
#include "Exceptions/Exceptions.hpp"

#define N_MAX_CHAN 16

#define CERR std::cout<<__LINE__<<std::endl;

int GetBit(unsigned int word, int bit_location){
  // obtain the value of a particular bit in a word
  word =  (word>>bit_location);
  word &= 0x1;
  return word;
}

class DigitizerDataException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };

struct DigitizerDataFragment { 
  
  DigitizerDataFragment( const uint32_t *data, size_t size ) {
    m_size = size;
    
    if(m_debug){
      const uint32_t* payload=data;
      unsigned int ii=0;
      for(;ii<size/4;ii++) {
        if (ii%8==0) std::cout<<" ";
        std::cout<<" 0x"<<std::setw(8)<<std::hex<<std::setfill('0')<<payload[ii];
        if (ii%8==7) std::cout<<std::endl;
      }
      if (ii%8!=0) std::cout<<std::endl;
      std::cout<<std::dec<<std::setfill(' ');
    }

    // decode header
    event.event_size            = data[0] & 0x0FFFFFFF;
    event.board_id              = data[1] >> 27;
    event.board_fail_flag       = GetBit(data[1], 26);
    event.pattern_trig_options  = (data[1] & 0x00FFFFFF) >> 8;
    event.channel_mask          = (data[1] & 0x000000FF) || ((data[2] & 0xFF000000) >> 16);
    event.event_counter         = data[2] & 0x00FFFFFF;
    event.trigger_time_tag      = data[3];
    
    // parse the ADC count data
    // subtract 4 for the header to get the size of the data payload
    int eSizeMod = event.event_size-4;

    // count the number of active channels
    int n_channels_active=0;
    for(int iChan=0; iChan<N_MAX_CHAN; iChan++){
      std::cout<<iChan<<"  "<<GetBit(event.channel_mask,iChan)<<std::endl;
      if( GetBit(event.channel_mask,iChan)==1 )
        n_channels_active++;
    }

    // divide modified event size by number of channels
    int words_per_channel = eSizeMod/n_channels_active;

    // there are two readings per word
    int samples_per_channel = 2*words_per_channel;
    event.n_samples = samples_per_channel;

    // location of pointer to start at begin of channel
    // starts at 4 because that is size of header
    int current_start_location = 4;
    
    for(int iChan=0; iChan<N_MAX_CHAN; iChan++){
      if( GetBit(event.channel_mask,iChan)==0 )
        continue;
        
      // make an empty vector entry for the data
      event.adc_counts[iChan] = {};
      for(int iDat=current_start_location; iDat<current_start_location+words_per_channel; iDat++){
        // two readings are stored in one word
        unsigned int chData = data[iDat];

        unsigned int ev0    = (chData & 0xFFFF0000) >> 16;  // first half of event doublet
        unsigned int ev1    = (chData & 0x0000FFFF);        // second half of event doublet

        event.adc_counts[iChan].push_back(ev1);
        event.adc_counts[iChan].push_back(ev0);

      }

      // move the starting location to the start of the next channels data
      current_start_location += words_per_channel;
    }
  
  
  }

//  bool valid() const {
//    if (m_size==sizeof(DigitizerEvent) && header()== TRIGGER_HEADER ) return true;
//    return false;
//  }

  public:
    // getters
    uint32_t event_size() const { return event.event_size; }
    uint32_t board_id() const { return event.board_id; }
    uint32_t board_fail_flag() const { return event.board_fail_flag; }
    uint32_t pattern_trig_options() const { return event.pattern_trig_options; }
    uint32_t channel_mask() const { return event.channel_mask; }
    uint32_t event_counter() const { return event.event_counter; }
    uint32_t trigger_time_tag() const { return event.trigger_time_tag; }
    int n_samples() const { return event.n_samples; }
    std::map<int, std::vector<float> > adc_counts() const { return event.adc_counts; }
//    uint8_t  input_bits_next_clk() const {
//      if ( valid() || m_debug )  return event.m_input_bits_next_clk;
//      THROW(DigitizerDataException, "Data not valid");
//    }
    size_t size() const { return m_size; }
    //setters
    void set_debug_on( bool debug = true ) { m_debug = debug; }

  private:
    struct DigitizerEvent {
      uint32_t event_size;
      uint32_t board_id;
      bool     board_fail_flag;
      bool     event_format; // should always be 0
      uint16_t pattern_trig_options;
      uint16_t channel_mask;
      uint32_t event_counter;
      uint32_t trigger_time_tag;
      
      int n_samples;
      std::map<int, std::vector<float> > adc_counts;
    }  __attribute__((__packed__)) event;
    size_t m_size; // number of words in full fragment
    bool m_debug = false;
}  __attribute__((__packed__));

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
    
    // print data
    for(int iSamp=0; iSamp<event.n_samples(); iSamp++){
      out<<std::setw(10)<<std::dec<<iSamp<<"|";
      for(int iChan=0; iChan<N_MAX_CHAN; iChan++){
        if( GetBit(event.channel_mask(), iChan)==1 ){
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


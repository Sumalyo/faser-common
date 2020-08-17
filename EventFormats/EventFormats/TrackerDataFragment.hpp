#pragma once
#include <bitset>
#include "Exceptions/Exceptions.hpp"
#include <iomanip>
#include <bitset>
#include <map>

#include "TrackerDataFragment.icc" 

class TrackerDataException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };

struct TrackerDataFragment { 
  
  
  TrackerDataFragment( const uint32_t *data, size_t size ) {
    m_size = size;
    m_debug = false;
    event.m_event_id = 0xffffff;
    event.m_bc_id = 0xffff;

    // now to fill data members from data - can refer to TRBEvent in gpiodrivers.
    const uint32_t errorMask   = 0x0000000F; //  4 bits
    const uint32_t bcidMask    = 0x00000FFF; // 12 bits
    const uint32_t payloadMask = 0x00FFFFFF; // 24 bits

    for (size_t i=0; i<size/4;i++ )
    {
       int frameType = (data[i] & 0x3<<30)>>30;//(d & 0b11<<30)>>30;
       int frameCounter=(data[i] & 0x7<<27)>>27;
       int moduleOrInfo=(data[i] & 0x7<<24)>>24;
       int modNer=0; //mod ID in error message
       int errId=(data[i]&0xF);
       
      // std::cout<<frameType<<" | "<<frameCounter<<std::endl;
       if (frameType==0 and moduleOrInfo==0){event.m_event_id=data[i]&payloadMask;}
       if (frameType==1)
       { 
         switch(moduleOrInfo)
         {
           case 0:event.m_bc_id=data[i]&bcidMask; break;
           case 1:        
                  std::cout<<"TRB ERROR: "<<errId<<std::endl; 
                  event.m_trb_error_id=errId;
                  break;
           case 2:
           case 3:
                  modNer=(data[i]&0x7<<24)>>24;
                  event.m_module_error_ids.push_back((frameType%2)<<7 | (modNer) << 4 | errId);
                  std::cout<<"LED(x) Module error: Module "<<modNer<<" ErrID: "<<errId<<std::endl; break;

         }

       }
       
       if (frameType == 2 || frameType == 3)
	{
	     std::pair<uint8_t, uint8_t> key { moduleOrInfo , (frameType == 2 ? 0 : 1) };
	     event.m_modDB[key].push_back(data[i] & payloadMask);
	}       
    }
  
  DecodeClass blep;
  blep.DecodeModuleData(event.m_modDB);
}
    
  bool valid() const {
    if (m_size<sizeof(event.m_event_id) ) return false; //example. Eventually to check dimensions of data, check for error ids.
  
    return true;
  }

  public:
    // getters
    uint32_t event_id() const { return event.m_event_id; }
    uint32_t bc_id() const { return event.m_bc_id; }
    size_t size() const { return m_size; }
    uint8_t trb_error_id() const { return event.m_trb_error_id;}
    std::vector<uint8_t>  module_error_id() const { return event.m_module_error_ids;}
    std::map< std::pair<uint8_t, uint8_t>,std::vector<uint32_t> >  module_modDB() const {return  event.m_modDB;}
    //setters
    void set_debug_on( bool debug = true ) { m_debug = debug; }

  private:
    struct TRBEvent {
      uint32_t m_event_id;
      uint32_t m_bc_id;
      uint8_t m_trb_error_id;
      std::vector< uint8_t > m_module_error_ids;
      std::map< std::pair<uint8_t, uint8_t>, std::vector<uint32_t> > m_modDB;
      std::vector < SCTEvent > m_hits_per_module; // SCTEvent to be implemented in 2nd step.
    }  event;
    size_t m_size;
    bool m_debug;
};

inline std::ostream &operator<<(std::ostream &out, const TrackerDataFragment &event) {
  try {
    out
    <<std::setw(11)<<" event_id: "<<std::setfill(' ')<<std::setw(32)<<event.event_id()<<std::setfill(' ')<<std::endl
    <<std::setw(11)<<" bc_id: "<<std::setfill(' ')<<std::setw(32)<<event.bc_id()<<std::setfill(' ')<<std::endl
    <<std::endl;
  } catch ( TrackerDataException& e ) {
    out<<e.what()<<std::endl;
    out<<"Corrupted data for Tracker data event "<<event.event_id()<<", bcid "<<event.bc_id()<<std::endl;
    out<<"Fragment size is "<<event.size()<<" bytes total"<<std::endl;
  }

 return out;
}




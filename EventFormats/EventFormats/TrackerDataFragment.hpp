#pragma once
#include <bitset>
#include "Exceptions/Exceptions.hpp"
#include <iomanip>
#include <bitset>
#include <map>


class TrackerDataException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };

struct TrackerDataFragment { 
  
  TrackerDataFragment( const uint32_t *data, size_t size ) {
    m_size = size;
    m_debug = false;
    event.m_event_id = 0xffffff;
    event.m_bc_id = 0xffff;

    std::map<int,uint64_t>modDB;
    // now to fill data members from data - can refer to TRBEvent in gpiodrivers.
    const uint32_t errorMask   = 0x0000000F; //  4 bits
    const uint32_t bcidMask    = 0x00000FFF; // 12 bits
    const uint32_t payloadMask = 0x00FFFFFF; // 24 bits

    for (size_t i=0; i<size/4;i++ )
    {
       int frameType = (data[i] & 0x3<<30)>>30;//(d & 0b111<<30)>>30;
       int frameCounter=(data[i] & 0x7<<27)>>27;
       int moduleOrInfo=(data[i] & 0x7<<24)>>24;
       int payloadLength=24; //Change to be more general later?
       int modNer=0; //mod ID in error message
       int errId=(data[i]&0x15);
       
      // std::cout<<frameType<<" | "<<frameCounter<<std::endl;
       
       if (frameType==0)
       { 
         switch(moduleOrInfo)
         {
           case 0:event.m_event_id=data[i]&payloadMask; break;
           case 1:
                  
                  std::cout<<"TRB ERROR: "<<errId<<std::endl; 
                  event.m_trb_error_id=errId;
                  break;
           case 2:
                  modNer=(data[i]&0x7<<24)>>24;
                  event.m_module_error_ids.push_back( (modNer) << 4 | errId);
                  std::cout<<"LED Module error: Module "<<modNer<<" ErrID: "<<errId<<std::endl; break;
           case 3:
                  modNer=(data[i]&0x7<<24)>>24;
                  event.m_module_error_ids.push_back( (modNer) << 4 | errId);
                  std::cout<<"LEDX Module error: Module "<<modNer<<" ErrID: "<<errId<<std::endl; break;

         }

       }
       if (frameType==1 and moduleOrInfo==0){event.m_bc_id=data[i]&bcidMask;}
       if (frameType==2)
       {
          int modN=moduleOrInfo;
          if (modDB.count(modN)!=0)
          {
            int oldV = modDB[modN];
            int newV = (oldV<<32)|(data[i]&payloadMask);
	    modDB[modN]=newV;
          }
          else {modDB[modN]=data[i]&payloadMask;}
       }
       if (frameType==3)
       {
          int modN=moduleOrInfo+10;
	  if (modDB.count(modN)!=0)
          {
            int oldV = modDB[modN];
            int newV = (oldV<<32)|(data[i]&payloadMask);
            modDB[modN]=newV;
          }
          else {modDB[modN]=data[i]&payloadMask;}
       }
    }
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
    std::vector<uint8_t> module_error_id() const { return event.m_module_error_ids;}
    //setters
    void set_debug_on( bool debug = true ) { m_debug = debug; }

  private:
    struct TRBEvent {
      uint32_t m_event_id;
      uint32_t m_bc_id;
      uint8_t m_trb_error_id;
      std::vector< uint8_t > m_module_error_ids;
      // std::vector < SCTEvents > m_hits_per_module; // SCTEvent to be implemented in 2nd step.
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


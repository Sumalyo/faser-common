#pragma once
#include <bitset>
#include "Exceptions/Exceptions.hpp"

class TrackerDataException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };

struct TrackerDataFragment { 
  
  TrackerDataFragment( const uint32_t *data, size_t size ) {
    m_size = size;
    m_debug = false;
    event.m_event_id = 0xffffff;
    event.m_bc_id = 0xffff;
    // now to fill data members from data - can refer to TRBEvent in gpiodrivers.
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
    out << " to dump TRB info here "
    << std::endl;
  } catch ( TrackerDataException& e ) {
    out<<e.what()<<std::endl;
    out<<"Corrupted data for Tracker data event "<<event.event_id()<<", bcid "<<event.bc_id()<<std::endl;
    out<<"Fragment size is "<<event.size()<<" bytes total"<<std::endl;
  }

 return out;
}


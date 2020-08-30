/*
  Copyright (C) 2019-2020 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// TLBMonitoringFragment.hpp, (c) FASER Detector software
///////////////////////////////////////////////////////////////////

#pragma once
#include <cstring> //memcpy, memset
#include "Exceptions/Exceptions.hpp"

#define MONITORING_HEADER 0xFEAD0050

class TLBMonException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };

struct TLBMonitoringFragment { 

  TLBMonitoringFragment( const uint32_t *data, size_t size ) {
    m_size = size;
    m_debug = false;
    event.m_event_id = 0xffffff;
    event.m_orbit_id = 0xffffffff;
    event.m_bc_id = 0xffff;
    memset(event.m_tbp, 0, sizeof(event.m_tbp));
    memset(event.m_tap, 0, sizeof(event.m_tap));
    memset(event.m_tav, 0, sizeof(event.m_tav));
    event.m_deadtime_veto_counter = 0xffffff;
    event.m_busy_veto_counter = 0xffffff;
    event.m_rate_limiter_veto_counter = 0xffffff;
    event.m_bcr_veto_counter = 0xffffff;
    memcpy(&event, data, std::min(size, sizeof(TLBMonEvent)));
  }

  bool valid() const {
    if (m_size==sizeof(TLBMonEvent) && header()== MONITORING_HEADER ) return true;
    return false;
  }

  public:
    // getters
    uint32_t header() const { return event.m_header; }
    uint32_t event_id() const { return event.m_event_id; }
    uint32_t orbit_id() const {
      if ( valid() || m_debug )  return event.m_orbit_id;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t bc_id() const { return event.m_bc_id; }
    uint32_t tbp( uint8_t trig_line ) const { 
      if ( trig_line >= max_trig_line ) THROW(TLBMonException, "index out of range");
      if ( valid() || m_debug ) return *(event.m_tbp+trig_line);
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t tap( uint8_t trig_line ) const { 
      if ( trig_line >= max_trig_line ) THROW(TLBMonException, "index out of range");
      if ( valid() || m_debug ) return *(event.m_tap+trig_line);
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t tav( uint8_t trig_line ) const { 
      if ( trig_line >= max_trig_line ) THROW(TLBMonException, "index out of range");
      if ( valid() || m_debug ) return *(event.m_tav+trig_line);
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t deadtime_veto_counter() const {
      if ( valid() || m_debug )  return event.m_deadtime_veto_counter;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t busy_veto_counter() const {
      if ( valid() || m_debug )  return event.m_busy_veto_counter;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t rate_limiter_veto_counter() const {
      if ( valid() || m_debug )  return event.m_rate_limiter_veto_counter;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t bcr_veto_counter() const {
      if ( valid() || m_debug )  return event.m_bcr_veto_counter;
      THROW(TLBMonException, "Data not valid");
    }
    size_t size() const { return m_size; }
    //setters
    void set_debug_on( bool debug = true ) { m_debug = debug; }

  private:
    static const uint8_t max_trig_line = 6;
    struct TLBMonEvent {
      uint32_t m_header;
      uint32_t m_event_id;
      uint32_t m_orbit_id;
      uint32_t m_bc_id;
      uint32_t m_tbp[max_trig_line];
      uint32_t m_tap[max_trig_line];
      uint32_t m_tav[max_trig_line];
      uint32_t m_deadtime_veto_counter;
      uint32_t m_busy_veto_counter;
      uint32_t m_rate_limiter_veto_counter;
      uint32_t m_bcr_veto_counter;
    }  __attribute__((__packed__)) event;
    size_t m_size;
    bool m_debug;
};

inline std::ostream &operator<<(std::ostream &out, const TLBMonitoringFragment &event) {
  try {
    out
    <<" event_id: "<<event.event_id()
    <<", orbit_id: "<<event.orbit_id()
    <<", bc_id:    "<<event.bc_id() << std::endl;
    for (unsigned i = 0; i < 6; i++ ){
      out<<"\ttbp"<<i<<": "<<std::setw(24)<<event.tbp(i)<<std::setfill(' ')
         <<"\ttap"<<i<<": "<<std::setw(24)<<event.tap(i)<<std::setfill(' ')
         <<"\ttav"<<i<<": "<<std::setw(24)<<event.tav(i)<<std::setfill(' ')<<std::endl;}
    out<<" deadtime veto count: "<< event.deadtime_veto_counter()
    <<", busy veto count: "<< event.busy_veto_counter()
    <<", rate_limiter veto count: "<< event.rate_limiter_veto_counter()
    <<", bcr veto count: "<< event.bcr_veto_counter() << std::endl;
  } catch ( TLBMonException& e ) {
    out<<e.what()<<std::endl;
    out<<"Corrupted data for TLB mon event "<<event.event_id()<<", bcid "<<event.bc_id()<<std::endl;
    out<<"Fragment size is "<<event.size()<<" bytes total"<<std::endl;
  }

 return out;
}

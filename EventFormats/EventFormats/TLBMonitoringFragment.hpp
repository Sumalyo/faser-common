/*
  Copyright (C) 2019-2020 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// TLBMonitoringFragment.hpp, (c) FASER Detector software
///////////////////////////////////////////////////////////////////

#pragma once
#include <cstring> //memcpy, memset
#include "Exceptions/Exceptions.hpp"

#define MONITORING_HEADER_V1 0xFEAD0050
#define MONITORING_HEADER_V2 0xFEAD0005
#define MASK_DATA 0xFFFFFF
#define MASK_FRAMEID_32b 0xF0000000
#define MASK_FRAMEID_TRIGLINE 0xFF000000


namespace TLBMonFormat {
class TLBMonException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };

FASER::FletcherChecksum fletcher = FASER::FletcherChecksum();

const uint32_t FID_EVENT_ID = 0x1<<28;
const uint32_t FID_BC_ID = 0x3<<28;
const uint32_t FID_TBP = 0x4<<28;
const uint32_t FID_TAP = 0x5<<28;
const uint32_t FID_TAV = 0x6<<28;
const uint32_t FID_CRC = 0xD<<28;

const uint8_t MAX_TRIG_LINE = 6;

typedef struct TLBMonEventV1 {
  uint32_t m_header;
  uint32_t m_event_id;
  uint32_t m_orbit_id;
  uint32_t m_bc_id;
  uint32_t m_tbp[MAX_TRIG_LINE];
  uint32_t m_tap[MAX_TRIG_LINE];
  uint32_t m_tav[MAX_TRIG_LINE];
  uint32_t m_deadtime_veto_counter;
  uint32_t m_busy_veto_counter;
  uint32_t m_rate_limiter_veto_counter;
  uint32_t m_bcr_veto_counter;
} __attribute__((__packed__));

struct TLBMonitoringFragment { 

  TLBMonitoringFragment( const uint32_t *data, size_t size ) {
    m_size = size;
    m_debug = false;
    event.v1.m_header = 0x0;
    event.v1.m_event_id = 0xffffff;
    event.v1.m_orbit_id = 0xffffffff;
    event.v1.m_bc_id = 0xffff;
    memset(event.v1.m_tbp, 0, sizeof(event.v1.m_tbp));
    memset(event.v1.m_tap, 0, sizeof(event.v1.m_tap));
    memset(event.v1.m_tav, 0, sizeof(event.v1.m_tav));
    event.v1.m_deadtime_veto_counter = 0xffffff;
    event.v1.m_busy_veto_counter = 0xffffff;
    event.v1.m_rate_limiter_veto_counter = 0xffffff;
    event.v1.m_bcr_veto_counter = 0xffffff;
    event.m_digitizer_busy_counter = 0xffffff;
    memcpy(&event, data, std::min(size, sizeof(TLBMonEvent)));
    m_version=0x2;
    if (data[0] == MONITORING_HEADER_V1) m_version=0x1; 
    m_crc_calculated = fletcher.ReturnChecksum(data, size);
  }

  bool frame_check() const{
    if (version() < 0x2) return true;
    if (((event.v1.m_event_id&MASK_FRAMEID_32b)) != FID_EVENT_ID) return false;
    if (((event.v1.m_bc_id&MASK_FRAMEID_32b)) != FID_BC_ID) return false;
    uint32_t FID_CNT = 0x0;
    for (unsigned i = 0; i < MAX_TRIG_LINE; i++){
      if ( (*(event.v1.m_tbp+i)&(MASK_FRAMEID_TRIGLINE)) != (FID_TBP|(FID_CNT<<24))) return false;
      if ( (*(event.v1.m_tap+i)&(MASK_FRAMEID_TRIGLINE)) != (FID_TAP|(FID_CNT<<24))) return false;
      if ( (*(event.v1.m_tav+i)&(MASK_FRAMEID_TRIGLINE)) != (FID_TAV|(FID_CNT<<24))) return false;
      FID_CNT+=1;
    }
    if (((event.m_checksum&MASK_FRAMEID_32b)) != FID_CRC) return false;
    return true;
  }

  bool valid() const {
    if ( version() > 0x1 ){
      if (m_crc_calculated != checksum()) return false;
      if (!frame_check()) return false;
      if (m_size==sizeof(TLBMonEvent)) return true;
    }
    else if (m_size==sizeof(TLBMonEventV1)) return true;
    return false;
  }

  public:
    // getters
    uint32_t header() const { return event.v1.m_header; }
    uint32_t event_id() const { return event.v1.m_event_id & MASK_DATA; }
    uint32_t orbit_id() const {
      if ( valid() || m_debug )  return event.v1.m_orbit_id;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t bc_id() const { return event.v1.m_bc_id & MASK_DATA; }
    uint32_t tbp( uint8_t trig_line ) const { 
      if ( trig_line >= MAX_TRIG_LINE ) THROW(TLBMonException, "index out of range");
      if ( valid() || m_debug ) return *(event.v1.m_tbp+trig_line)&MASK_DATA;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t tap( uint8_t trig_line ) const { 
      if ( trig_line >= MAX_TRIG_LINE ) THROW(TLBMonException, "index out of range");
      if ( valid() || m_debug ) return *(event.v1.m_tap+trig_line)&MASK_DATA;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t tav( uint8_t trig_line ) const { 
      if ( trig_line >= MAX_TRIG_LINE ) THROW(TLBMonException, "index out of range");
      if ( valid() || m_debug ) return *(event.v1.m_tav+trig_line)&MASK_DATA;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t deadtime_veto_counter() const {
      if ( valid() || m_debug )  return event.v1.m_deadtime_veto_counter&MASK_DATA;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t busy_veto_counter() const {
      if ( valid() || m_debug )  return event.v1.m_busy_veto_counter&MASK_DATA;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t rate_limiter_veto_counter() const {
      if ( valid() || m_debug )  return event.v1.m_rate_limiter_veto_counter&MASK_DATA;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t bcr_veto_counter() const {
      if ( valid() || m_debug )  return event.v1.m_bcr_veto_counter&MASK_DATA;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t digitizer_busy_counter() const {
      if ( valid() || m_debug )  return event.m_digitizer_busy_counter&MASK_DATA;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t checksum() const { return event.m_checksum & MASK_DATA; }
    size_t size() const { return m_size; }
    bool has_checksum_error() const { return (m_crc_calculated != checksum()); }
    bool has_frameid_error() const { return !frame_check();}
    uint8_t version() const { return m_version; }
    //setters
    void set_debug_on( bool debug = true ) { m_debug = debug; }

  private:
    struct TLBMonEvent {
      TLBMonEventV1 v1;
      uint32_t m_digitizer_busy_counter;
      uint32_t m_checksum;
    }  __attribute__((__packed__)) event;
    size_t m_size;
    uint8_t m_version;
    bool m_debug;
    uint32_t m_crc_calculated;
};

}

inline std::ostream &operator<<(std::ostream &out, const TLBMonFormat::TLBMonitoringFragment &event) {
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
  } catch ( TLBMonFormat::TLBMonException& e ) {
    out<<e.what()<<std::endl;
    out<<"Corrupted data for TLB mon event "<<event.event_id()<<", bcid "<<event.bc_id()<<std::endl;
    out<<"Fragment size is "<<event.size()<<" bytes total"<<std::endl;
    out<<"checksum errors present "<<event.has_checksum_error()<<std::endl;
    out<<"frameid errors present "<<event.has_frameid_error()<<std::endl;
  }

 return out;
}

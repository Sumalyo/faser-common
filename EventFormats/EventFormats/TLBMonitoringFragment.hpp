/*
  Copyright (C) 2019-2020 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// TLBMonitoringFragment.hpp, (c) FASER Detector software
///////////////////////////////////////////////////////////////////

#pragma once
#include <cstring> //memcpy, memset
#include "Exceptions/Exceptions.hpp"
#include "FletcherChecksum.hpp"

namespace TLBMonFormat {

class TLBMonException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };

const uint32_t MONITORING_HEADER_V1 = 0xFEAD0050;
const uint32_t MONITORING_HEADER_V2 = 0xFEAD0005;
const uint32_t MASK_DATA = 0xFFFFFF;
const uint32_t MASK_FRAMEID_32b = 0xF0000000;
const uint32_t MASK_FRAMEID_TRIGLINE = 0xFF000000;
const uint32_t FID_EVENT_ID = 0x10000000;
const uint32_t FID_BC_ID = 0x30000000;
const uint32_t FID_TBP = 0x40000000;
const uint32_t FID_TAP = 0x50000000;
const uint32_t FID_TAV = 0x60000000;
const uint32_t FID_TAPORed = 0xF0000000;
const uint32_t FID_TAVORed = 0xF0000000;
const uint32_t FID_CRC = 0xD0000000;

const uint8_t MAX_TRIG_LINE = 6;
const uint8_t MONDATA_V2_OLD_SIZE = 112;

struct TLBMonEventV1 {
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
    event.m_tap_ORed = 0xf0ffffff;
    event.m_tav_ORed = 0xf0ffffff;
    memcpy(&event, data, std::min(size, sizeof(TLBMonEvent))-sizeof(uint32_t)); // don't fill CRC yet
    event.m_checksum = data[size/sizeof(data[0])-1];
    m_version = 0xff;
    if (data[0] == MONITORING_HEADER_V1) m_version=0x1; 
    else if (data[0] == MONITORING_HEADER_V2) m_version=0x2;
    m_crc_calculated = FletcherChecksum::ReturnFletcherChecksum(data, size);
    m_verified = false;
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
    if (((event.m_tap_ORed&MASK_FRAMEID_32b)) != FID_TAPORed) return false;
    if (((event.m_tav_ORed&MASK_FRAMEID_32b)) != FID_TAVORed) return false;
    if (((event.m_checksum&MASK_FRAMEID_32b)) != FID_CRC) return false;
    return true;
  }

  bool valid() const {
    if (m_verified) return m_valid;
    m_valid = true;
    if ( version() == 0xff ) m_valid = false;
    if ( version() > 0x1 ){
      if (m_crc_calculated != checksum()) m_valid = false;
      if (!frame_check()) m_valid = false;
      if (m_size!=sizeof(TLBMonEvent) && m_size!= sizeof(TLBMonEvent)-2*sizeof(uint32_t)) m_valid = false; // extra size check for old v2 mon data that included 2 less counters
    }
    else if (m_size!=sizeof(TLBMonEventV1)) m_valid = false; // v1 has no trailer
    m_verified = true;
    return m_valid;
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
    uint32_t tap_ORed() const {
      if ( valid() || m_debug )  return event.m_tap_ORed&MASK_DATA;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t tav_ORed() const {
      if ( valid() || m_debug )  return event.m_tav_ORed&MASK_DATA;
      THROW(TLBMonException, "Data not valid");
    }
    uint32_t checksum() const { return event.m_checksum & MASK_DATA; }
    size_t size() const { return m_size; }
    bool has_checksum_error() const { if (version()<0x2) return false; return (m_crc_calculated != checksum()); }
    bool has_frameid_error() const { return !frame_check();}
    uint8_t version() const { return m_version; }
    //setters
    void set_debug_on( bool debug = true ) { m_debug = debug; }

  private:
    struct TLBMonEvent {
      TLBMonEventV1 v1;
      uint32_t m_digitizer_busy_counter;
      uint32_t m_tap_ORed;
      uint32_t m_tav_ORed;
      uint32_t m_checksum;
    }  __attribute__((__packed__)) event;
    size_t m_size;
    uint8_t m_version;
    bool m_debug;
    uint32_t m_crc_calculated;
    mutable bool m_verified;
    mutable bool m_valid;
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
    <<", bcr veto count: "<< event.bcr_veto_counter();
    if (event.version()>0x1) {
      out<<", digi busy veto count: "<< event.digitizer_busy_counter()<<std::endl;
      if (event.size()!=TLBMonFormat::MONDATA_V2_OLD_SIZE)
         out<<" TAP ORed: "<< event.tap_ORed()<<", TAV ORed: "<<event.tav_ORed();
    }
    out<<std::endl;
  } catch ( TLBMonFormat::TLBMonException& e ) {
    out<<e.what()<<std::endl;
    out<<"Corrupted data for TLB mon event "<<event.event_id()<<", bcid "<<event.bc_id()
       <<". Fragment size is "<<event.size()<<" bytes total"<<std::endl;
    if (event.version()>0x1){
      out<<"checksum errors present "<<event.has_checksum_error()
         <<", frameid errors present "<<event.has_frameid_error()<<std::endl;
    }
  }
  out<<"\ndata format version: 0x"<<std::hex<<static_cast<int>(event.version())<<std::dec<<std::endl;
  if (event.version() == 0xff ) out<<"WARNING This is an invalid format version number! Either this data is corrupted or this is not TLB monitoring data."<<std::endl;

 return out;
}

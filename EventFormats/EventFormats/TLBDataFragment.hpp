/*
  Copyright (C) 2019-2020 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// TLBDataFragment.hpp, (c) FASER Detector software
///////////////////////////////////////////////////////////////////

#pragma once
#include <bitset>
#include <cstring> //memcpy
#include "Exceptions/Exceptions.hpp"
#include "FletcherChecksum.hpp"

CREATE_EXCEPTION_TYPE(TLBDataException,TLBDataFormat)
namespace TLBDataFormat {

const uint32_t TRIGGER_HEADER_V1 = 0xFEAD000A;
const uint32_t TRIGGER_HEADER_V2 = 0xFEAD00A0;
const uint32_t MASK_DATA = 0xFFFFFF;
const uint8_t MASK_TBP = 0x3F;
const uint16_t MASK_TAP_V1 = 0x3F00;
const uint16_t MASK_TAP_V2 = 0xFC0;
const uint32_t MASK_FRAMEID_32b = 0xF0000000;
const uint16_t MASK_FRAMEID_16b = 0xF000;
const uint32_t FID_EVENT_ID = 0x10000000;
const uint32_t FID_BC_ID = 0x30000000;
const uint16_t FID_TBPTAP = 0xC000;
const uint32_t FID_CRC = 0xE0000000;

struct TLBEventV1{
 uint32_t m_header;
 uint32_t m_event_id;
 uint32_t m_orbit_id;
 uint32_t m_bc_id;
 uint8_t  m_input_bits_next_clk;
 uint8_t  m_input_bits;
 uint16_t  m_tbptap;
} __attribute__((__packed__));

struct TLBDataFragment { 
  
  TLBDataFragment( const uint32_t *data, size_t size ) {
    m_size = size;
    m_debug = false;
    event.v1.m_header = 0x0;
    event.v1.m_event_id = 0xffffff;
    event.v1.m_orbit_id = 0xffffffff;
    event.v1.m_bc_id = 0xffff;
    event.v1.m_input_bits = 0;
    event.v1.m_input_bits_next_clk = 0;
    event.v1.m_tbptap = 0;
    memcpy(&event, data, std::min(size, sizeof(TLBEvent)));
    m_version=0xff;
    if (data[0] == TRIGGER_HEADER_V1) m_version=0x1; 
    else if (data[0] == TRIGGER_HEADER_V2) m_version=0x2; 
    m_crc_calculated = FletcherChecksum::ReturnFletcherChecksum(data, size);
    m_verified = false;
  }

  bool frame_check() const{
    if (version() < 0x2) return true;
    if (((event.v1.m_event_id&MASK_FRAMEID_32b)) != FID_EVENT_ID) return false;
    if (((event.v1.m_bc_id&MASK_FRAMEID_32b)) != FID_BC_ID) return false;
    if (((event.v1.m_tbptap&MASK_FRAMEID_16b)) != FID_TBPTAP) return false;
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
      if (m_size!=sizeof(TLBEvent)) m_valid = false;
    }
    else if (m_size!=sizeof(TLBEventV1)) m_valid = false; // v1 has no trailer
    m_verified = true;
    return m_valid;
  }

  public:
    // getters
    uint32_t header() const { return event.v1.m_header; }
    uint32_t event_id() const { return event.v1.m_event_id & MASK_DATA; }
    uint32_t orbit_id() const {
      if ( valid() || m_debug )  return event.v1.m_orbit_id;
      THROW(TLBDataException, "Data not valid");
    }
    uint32_t bc_id() const { return event.v1.m_bc_id & MASK_DATA; }
    uint8_t  tap() const {
      if ( valid() || m_debug )  {
        if (m_version < 0x2) return (event.v1.m_tbptap & MASK_TAP_V1)>>8;
        else return (event.v1.m_tbptap&MASK_TAP_V2)>>6;
      }
      THROW(TLBDataException, "Data not valid");
    }
    uint8_t  tbp() const {
      if ( valid() || m_debug ) {;
        return event.v1.m_tbptap & MASK_TBP;
      }
      THROW(TLBDataException, "Data not valid");
    }
    uint8_t  input_bits() const {
      if ( valid() || m_debug )  return event.v1.m_input_bits;
      THROW(TLBDataException, "Data not valid");
    }
    uint8_t  input_bits_next_clk() const {
      if ( valid() || m_debug )  return event.v1.m_input_bits_next_clk;
      THROW(TLBDataException, "Data not valid");
    }
    uint32_t checksum() const { return event.m_checksum & MASK_DATA; }
    bool has_checksum_error() const { if (version()<0x2) return false; return (m_crc_calculated != checksum()); }
    bool has_frameid_error() const { return !frame_check();}
    size_t size() const { return m_size; }
    uint8_t version() const { return m_version; }
    //setters
    void set_debug_on( bool debug = true ) { m_debug = debug; }

  private:
    struct TLBEvent {
      TLBEventV1 v1;
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

inline std::ostream &operator<<(std::ostream &out, const TLBDataFormat::TLBDataFragment &event) {
  try {
    out
    <<std::setw(22)<<" event_id: "<<std::setfill(' ')<<std::setw(32)<<event.event_id()<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" orbit_id: "<<std::setfill(' ')<<std::setw(32)<<event.orbit_id()<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" bc_id: "<<std::setfill(' ')<<std::setw(32)<<event.bc_id()<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" TAP: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<6>(event.tap())<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" TBP: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<6>(event.tbp())<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" input_bits: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<8>(event.input_bits())<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" input_bits_next_clk: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<8>(event.input_bits_next_clk())<<std::setfill(' ')<<std::endl;
  } catch ( TLBDataFormat::TLBDataException& e ) {
    out<<e.what()<<std::endl;
    out<<"Corrupted data for TLB data event "<<event.event_id()<<", bcid "<<event.bc_id()<<std::endl;
    out<<"Fragment size is "<<event.size()<<" bytes total"<<std::endl;
    if (event.version()>0x1){
      out<<"checksum errors present "<<event.has_checksum_error()<<std::endl;
      out<<"frameid errors present "<<event.has_frameid_error()<<std::endl;
    }
  }
  out<<"\ndata format version: 0x"<<std::hex<<static_cast<int>(event.version())<<std::dec<<std::endl;
  if (event.version() == 0xff ) out<<"WARNING This is an invalid format version number! Either this data is corrupted or this is not TLB physics data."<<std::endl;

 return out;
}


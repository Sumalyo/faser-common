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

#define TRIGGER_HEADER_V1 0xFEAD000A
#define TRIGGER_HEADER_V2 0xFEAD00A0
#define MASK_EVENT_ID 0xFFFFFF
#define MASK_BC_ID 0xFFF
#define MASK_TBP 0x3F
#define MASK_TAP_FROM_TBP 0xC0
#define MASK_TAP 0x3F

class TLBDataException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };

struct TLBDataFragment { 
  
  TLBDataFragment( const uint32_t *data, size_t size ) {
    m_size = size;
    m_debug = false;
    event.m_header = 0x0;
    event.m_event_id = 0xffffff;
    event.m_orbit_id = 0xffffffff;
    event.m_bc_id = 0xffff;
    event.m_tbp = 0;
    event.m_tap  = 0;
    event.m_input_bits = 0;
    event.m_input_bits_next_clk = 0;
    memcpy(&event, data, std::min(size, sizeof(TLBEvent)));
    m_version=0x1;
    if (data[0] == TRIGGER_HEADER_V2) m_version=0x2; 
  }

  bool valid() const {
    if ( header()== TRIGGER_HEADER_V1 ) {
      if (m_size==(sizeof(TLBEvent)-4)) return true; } //FIXME
    if ( header()== TRIGGER_HEADER_V2 ) {
      if (m_size==sizeof(TLBEvent)) return true; }
    return false;
  }

  public:
    // getters
    uint32_t header() const { return event.m_header; }
    uint32_t event_id() const { return event.m_event_id & MASK_EVENT_ID; }
    uint32_t orbit_id() const {
      if ( valid() || m_debug )  return event.m_orbit_id;
      THROW(TLBDataException, "Data not valid");
    }
    uint32_t bc_id() const { return event.m_bc_id & MASK_BC_ID; }
    uint8_t  tap() const {
      if ( valid() || m_debug )  {
        if (m_version < 0x2) return event.m_tap;
        else {
          uint8_t tap = (event.m_tap << 2); // FIXME
          uint8_t tap_part = (event.m_tbp & MASK_TAP_FROM_TBP)>>4;
          tap |= tap_part;
          return tap & MASK_TAP;
        }
      }
      THROW(TLBDataException, "Data not valid");
    }
    uint8_t  tbp() const {
      if ( valid() || m_debug ) {;
        return event.m_tbp & MASK_TBP;
      }
      THROW(TLBDataException, "Data not valid");
    }
    uint8_t  input_bits() const {
      if ( valid() || m_debug )  return event.m_input_bits;
      THROW(TLBDataException, "Data not valid");
    }
    uint8_t  input_bits_next_clk() const {
      if ( valid() || m_debug )  return event.m_input_bits_next_clk;
      THROW(TLBDataException, "Data not valid");
    }
    size_t size() const { return m_size; }
    uint8_t version() const { return m_version; }
    //setters
    void set_debug_on( bool debug = true ) { m_debug = debug; }

  private:
    struct TLBEvent {
      uint32_t m_header;
      uint32_t m_event_id;
      uint32_t m_orbit_id;
      uint32_t m_bc_id;
      uint8_t  m_input_bits_next_clk;
      uint8_t  m_input_bits;
      uint8_t  m_tbp;
      uint8_t  m_tap;
      uint32_t m_checksum;
    }  __attribute__((__packed__)) event;
    size_t m_size;
    uint8_t m_version;
    bool m_debug;
};

inline std::ostream &operator<<(std::ostream &out, const TLBDataFragment &event) {
  try {
    out
    <<std::setw(22)<<" event_id: "<<std::setfill(' ')<<std::setw(32)<<event.event_id()<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" orbit_id: "<<std::setfill(' ')<<std::setw(32)<<event.orbit_id()<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" bc_id: "<<std::setfill(' ')<<std::setw(32)<<event.bc_id()<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" TAP: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<6>(event.tap())<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" TBP: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<6>(event.tbp())<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" input_bits: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<8>(event.input_bits())<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" input_bits_next_clk: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<8>(event.input_bits_next_clk())<<std::setfill(' ')<<std::endl;
  } catch ( TLBDataException& e ) {
    out<<e.what()<<std::endl;
    out<<"Corrupted data for TLB data event "<<event.event_id()<<", bcid "<<event.bc_id()<<std::endl;
    out<<"Fragment size is "<<event.size()<<" bytes total"<<std::endl;
  }

 return out;
}


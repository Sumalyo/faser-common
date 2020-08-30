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

#define TRIGGER_HEADER 0xFEAD000A

class TLBDataException : public Exceptions::BaseException { using Exceptions::BaseException::BaseException; };

struct TLBDataFragment { 
  
  TLBDataFragment( const uint32_t *data, size_t size ) {
    m_size = size;
    m_debug = false;
    event.m_event_id = 0xffffff;
    event.m_orbit_id = 0xffffffff;
    event.m_bc_id = 0xffff;
    event.m_tbp = 0;
    event.m_tap  = 0;
    event.m_input_bits = 0;
    event.m_input_bits_next_clk = 0;
    memcpy(&event, data, std::min(size, sizeof(TLBEvent)));
  }

  bool valid() const {
    if (m_size==sizeof(TLBEvent) && header()== TRIGGER_HEADER ) return true;
    return false;
  }

  public:
    // getters
    uint32_t header() const { return event.m_header; }
    uint32_t event_id() const { return event.m_event_id; }
    uint32_t orbit_id() const {
      if ( valid() || m_debug )  return event.m_orbit_id;
      THROW(TLBDataException, "Data not valid");
    }
    uint32_t bc_id() const { return event.m_bc_id; }
    uint8_t  tap() const {
      if ( valid() || m_debug )  return event.m_tap;
      THROW(TLBDataException, "Data not valid");
    }
    uint8_t  tbp() const {
      if ( valid() || m_debug )  return event.m_tbp;
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
    }  __attribute__((__packed__)) event;
    size_t m_size;
    bool m_debug;
};

inline std::ostream &operator<<(std::ostream &out, const TLBDataFragment &event) {
  try {
    out
    <<std::setw(22)<<" event_id: "<<std::setfill(' ')<<std::setw(32)<<event.event_id()<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" orbit_id: "<<std::setfill(' ')<<std::setw(32)<<event.orbit_id()<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" bc_id: "<<std::setfill(' ')<<std::setw(32)<<event.bc_id()<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" TAP: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<6>(event.tap())<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" TAV: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<6>(event.tbp())<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" input_bits: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<8>(event.input_bits())<<std::setfill(' ')<<std::endl
    <<std::setw(22)<<" input_bits_next_clk: "<<std::setfill(' ')<<std::setw(32)<<std::bitset<8>(event.input_bits_next_clk())<<std::setfill(' ')<<std::endl;
  } catch ( TLBDataException& e ) {
    out<<e.what()<<std::endl;
    out<<"Corrupted data for TLB data event "<<event.event_id()<<", bcid "<<event.bc_id()<<std::endl;
    out<<"Fragment size is "<<event.size()<<" bytes total"<<std::endl;
  }

 return out;
}


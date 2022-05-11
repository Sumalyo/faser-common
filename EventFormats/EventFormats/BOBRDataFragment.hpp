/*
  Copyright (C) 2019-2022 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// BOBRDataFragment.hpp, (c) FASER Detector software
///////////////////////////////////////////////////////////////////

#pragma once
#include <bitset>
#include <cstring> //memcpy
#include "Exceptions/Exceptions.hpp"
#include "FletcherChecksum.hpp"

CREATE_EXCEPTION_TYPE(BOBRDataException,BOBRDataFormat)
namespace BOBRDataFormat {
const uint16_t BOBR_HEADER_V1 = 0xB0B1;

  struct BOBREventV1 {
    uint16_t m_header;
    uint16_t m_status;
    uint32_t m_gpstime_seconds;
    uint32_t m_gpstime_useconds;
    uint32_t m_turncount;
    uint32_t m_fillnumber;
    uint16_t m_machinemode;
    uint16_t m_beam_momentum;
    uint32_t m_beam1_intensity;
    uint32_t m_beam2_intensity;
  }  __attribute__((__packed__));

struct BOBRDataFragment { 
  
  BOBRDataFragment( const uint32_t *data, size_t size ) {
    if (size!=sizeof(BOBREventV1)) 
      THROW(BOBRDataFormat::BOBRDataException, "BOBR fragment size is not correct");
    if (data[0]!=BOBR_HEADER_V1)
      THROW(BOBRDataFormat::BOBRDataException, "Unknown BOBR fragment version");
    m_size = size;
    m_debug = false;
    memcpy(&event, data, size);
  }

  bool valid() const {
    return true;
  }

  public:
    // getters
    uint16_t header() const { return event.m_header; }
    uint16_t status() const { return event.m_status; }
  uint32_t gpstime_seconds() const { return event.m_gpstime_seconds; }
  uint32_t gpstime_useconds() const { return event.m_gpstime_useconds; }
  uint32_t turncount() const { return event.m_turncount; }
  uint32_t fillnumber() const { return event.m_fillnumber; }
  uint16_t machinemode() const { return event.m_machinemode; }
  uint16_t beam_momentum() const { return event.m_beam_momentum; }
  uint32_t beam1_intensity() const { return event.m_beam1_intensity; }
  uint32_t beam2_intensity() const { return event.m_beam2_intensity; }
    size_t size() const { return m_size; }
    //setters
    void set_debug_on( bool debug = true ) { m_debug = debug; }

  private:
    BOBREventV1 event;
    size_t m_size;
    bool m_debug;
};
}

inline std::ostream &operator<<(std::ostream &out, const BOBRDataFormat::BOBRDataFragment &event) {
  try {
    out
      <<std::setw(22)<<" gps_time: "<<std::setfill(' ')<<std::setw(25)<<event.gpstime_seconds()<<"."<<std::setfill('0')<<std::setw(6)<<event.gpstime_useconds()<<std::setfill(' ')<<std::endl
      <<std::setw(22)<<" LHC turn count: "<<std::setfill(' ')<<std::setw(32)<<event.turncount()<<std::setfill(' ')<<std::endl
      <<std::setw(22)<<" LHC fill number: "<<std::setfill(' ')<<std::setw(32)<<event.fillnumber()<<std::setfill(' ')<<std::endl
      <<std::setw(22)<<" LHC machine mode: "<<std::setfill(' ')<<std::setw(32)<<event.machinemode()<<std::setfill(' ')<<std::endl
      <<std::setw(22)<<" Beam momentum [GeV]: "<<std::setfill(' ')<<std::setw(32)<<0.12*event.beam_momentum()<<std::setfill(' ')<<std::endl
      <<std::setw(22)<<" Beam 1 intensity [1e10p]: "<<std::setfill(' ')<<std::setw(32)<<event.beam1_intensity()<<std::setfill(' ')<<std::endl
      <<std::setw(22)<<" Beam 2 intensity [1e10p]: "<<std::setfill(' ')<<std::setw(32)<<event.beam2_intensity()<<std::setfill(' ')<<std::endl;
      
  } catch ( BOBRDataFormat::BOBRDataException& e ) {
    out<<e.what()<<std::endl;
    out<<"Corrupted data for BOBR data event"<<std::endl;
  }

 return out;
}


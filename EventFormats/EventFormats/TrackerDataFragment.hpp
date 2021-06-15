/*
  Copyright (C) 2019-2020 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackerDataFragment.hpp, (c) FASER Detector software
///////////////////////////////////////////////////////////////////

#pragma once
#include <bitset>
#include "Exceptions/Exceptions.hpp"
#include "Logging.hpp"
#include "EventFormats/FletcherChecksum.hpp"
#include <iomanip>
#include <map>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <memory>


CREATE_EXCEPTION_TYPE(TrackerDataException,TrackerData)
struct SCTEvent 
{
public:
    SCTEvent (uint8_t module, unsigned int l1id, unsigned int bcid);
    ~SCTEvent() {}
    
    void AddHeader(unsigned int l1id, unsigned int bcid);
    void AddHit (unsigned int chip, unsigned int strip, unsigned int pattern);
    void AddError (unsigned int chip, unsigned int err);
    void AddUnknownChip (unsigned int chip) {m_UnkownChips.push_back(chip);}
    
    unsigned short                                              GetModuleID() const { return m_moduleID; }
    unsigned int GetNHits() const;
    unsigned int GetNHits(size_t chip) const;
    const std::vector < std::vector < std::pair<uint8_t, uint8_t> > >& 
                                                                GetHits() const {return m_Hits;}
    const std::vector < std::pair<uint8_t, uint8_t> >&          GetHits(size_t chip) const;
    const std::vector < std::vector < uint8_t > >&              GetErrors() const {return m_Errors;}
    const std::vector < uint8_t >&                              GetErrors(size_t chip) const;
    const std::vector < uint8_t >&                              GetUnknownChips() const {return m_UnkownChips;}
    
    bool HasError() const {return m_hasError;}
    bool IsComplete() const {return m_complete;}
    void SetComplete() {m_complete = true;}
    bool MissingData() const {return m_missingData;}
    void SetMissingData() {m_missingData = true;}
    bool BCIDMismatch() const {return m_bcidMismatch;}
    void SetBCIDMismatch() {m_bcidMismatch = true;}
    //void SetChipIsValid() {m_chipIsValid=true;}
    bool ChipIsValid(unsigned int chip) {
          m_chipIsValid=true;
          chip |= 0x20; // adding 2 MSB for chip address. All chips are served by "primary fiber".
          auto it = m_chipIDMap.find(chip);
          if (it == m_chipIDMap.end()) {
            std::stringstream s;
            s << std::hex << chip;
            m_complete = false;
            m_missingData = true;
            m_chipIsValid=false;
            WARNING("SCTEvent::AddHit :: ERROR: AddHit(): passed chipID is not known! chipID = 0x" + s.str());
            //if (m_debug) {WARNING("SCTEvent::AddHit :: ERROR: AddHit(): passed chipID is not known! chipID = 0x" + s.str());}
          }
          return m_chipIsValid;}
    
    unsigned short GetL1ID() const {return m_l1id;}
    unsigned short GetBCID() const {return m_bcid;}
    
private:
    std::vector < std::vector < std::pair<uint8_t, uint8_t> > > m_Hits; // chips[12][nHits] , hit = pair {Strip Nr, Hit pattern}
    std::vector < std::vector < uint8_t > > m_Errors; // chips[12][error]
    std::vector < uint8_t > m_UnkownChips; // chipIDs that were not expected
    unsigned short m_moduleID;
    unsigned short m_bcid;
    unsigned short m_l1id;

    bool m_complete; // sepcifies if data from all chips (i.e. LED and LEDX lines is inserted
    bool m_missingData; // sepcifies if data from all chips (i.e. LED and LEDX lines is inserted
    bool m_bcidMismatch; // bcid mismatch between led and ledX data streams
    bool m_hasError; // bcid mismatch between led and ledX data streams
    bool m_chipIsValid=true; //check if chip exists in map
    std::map<unsigned int, unsigned int> m_chipIDMap; // maps chipID to vector index
};

struct TrackerDataFragment
{
  public:

    const uint32_t MASK_TRBFRAME            = 0xC7000000;
    const uint32_t TRB_HEADER               = 0x00000000;
    const uint32_t TRB_TRAILER              = 0x01000000;
    const uint32_t TRB_END                  = 0x07000eee;
    
    const uint32_t MASK_WORDTYPE            = 0xC0000000;
    const uint32_t WORDTYPE_TRBHEADER       = 0x0;
    const uint32_t WORDTYPE_TRBTRAILER      = 0x0;
    const uint32_t WORDTYPE_TRBDATA         = 0x40000000;
    const uint32_t WORDTYPE_MODULEDATA_LED  = 0x80000000;
    const uint32_t WORDTYPE_MODULEDATA_LEDX = 0xC0000000;
    
    static const uint32_t MASK_TRBDATATYPE             = 0x07000000;
    static const uint32_t TRBDATATYPE_EVENTID          = 0x00000000;
    static const uint32_t TRBDATATYPE_BCID             = 0x00000000;
    static const uint32_t TRBDATATYPE_TRBERROR         = 0x01000000;
    static const uint32_t TRBDATATYPE_MODULEERROR_LED  = 0x02000000;
    static const uint32_t TRBDATATYPE_MODULEERROR_LEDX = 0x03000000;
    static const uint32_t TRBDATATYPE_CRC              = 0x01000000;
    
    const uint32_t MASK_FRAMECNT    = 0x38000000;
    const uint32_t RSHIFT_FRAMECNT  = 27;
    const uint32_t FRAME_COUNTER_CYCLE = 8;
    const uint32_t MASK_BCID     = 0xFFF;
    const uint32_t MASK_ERROR    = 0xF;
    const uint32_t MASK_MODULEDATA_CHANNEL     = 0x40000000;
    const uint32_t RSHIFT_MODULEDATA_CHANNEL   = 30;
    const uint32_t MASK_MODULEDATA_MODULEID    = 0x07000000;
    const uint32_t RSHIFT_MODULEDATA_MODULEID  = 24;
    const uint32_t MASK_MODULEDATA = 0xFFFFFF;
    const uint32_t MASK_TRBDATA_MODULEID       = 0x00700000;
    const uint32_t RSHIFT_TRBDATA_MODULEID       = 20;
    const uint32_t MASK_TRBDATA_ERRORCHANNEL   = 0x01000000;
    const uint32_t RSHIFT_TRBDATA_ERRORCHANNEL   = 24;
    const uint32_t LSHIFT_ERROR_CHANNEL    = 7;
    const uint32_t LSHIFT_ERROR_MODULE     = 4;
    const uint32_t MASK_EVNTCNT  = 0xFFFFFF;
    const uint32_t MASK_CRC      = 0xFFFFFF;
    const uint32_t END_OF_DAQ    = 0x7000EEE; // magic word
    
    const unsigned int usefulBitsModuleData = 32 ; // How many bits are used per word? All MASK / TAG constants rely on this alignment
    // MASKS for module data
    
    const uint32_t MASK_MODULE_HEADER = 0xFC002000;
    const uint32_t TAG_MODULE_HEADER  = 0xE8002000;
    const uint32_t MASK_MODULE_TRAILER= 0xFFFF0000;
    const uint32_t TAG_MODULE_TRAILER = 0x80000000;
    const uint32_t MASK_MODULE_ERROR  = 0xE0200000;
    const uint32_t TAG_MODULE_ERROR   = 0x00200000;
    const uint32_t MASK_MODULE_CONFIG = 0xE1C02010;
    const uint32_t TAG_MODULE_CONFIG  = 0x1C02010;
  //    const uint32_t MASK_MODULE_DATA   = 0xC0040000;
  //    const uint32_t TAG_MODULE_DATA    = 0x40040000;
    const uint32_t MASK_MODULE_DATA   = 0xC0000000; //don't look for first hit bit here as might be missing in buggy module 0
    const uint32_t TAG_MODULE_DATA    = 0x40000000;
    const uint32_t MASK_MODULE_NODATA = 0xE0000000;
    const uint32_t TAG_MODULE_NODATA  = 0x20000000;
    
    // access data modules with bit shift & mask
    const uint32_t RSHIFT_MODULE_BCID = 14;
    const uint32_t MASK_MODULE_BCID   = 0xff;
    const uint32_t RSHIFT_MODULE_L1ID = 22;
    const uint32_t MASK_MODULE_L1ID   = 0xf;
    
    const uint32_t RSHIFT_CHIPADD_CONF = 25;
    const uint32_t MASK_CHIPADD_CONF   = 0xF;
    const uint32_t RSHIFT_CONF1        = 14;
    const uint32_t RSHIFT_CONF2        = 5;
    const uint32_t MASK_CONF           = 0xFF;
    const uint32_t RSHIFT_CHIPADD_ERR  = 25;
    const uint32_t MASK_CHIPADD_ERR    = 0xF;
    const uint32_t RSHIFT_ERR          = 22;
    const uint32_t MASK_ERR            = 0x7;
    const uint32_t RSHIFT_CHIPADD_DATA = 26;
    const uint32_t MASK_CHIPADD_DATA   = 0xF;
    const uint32_t RSHIFT_CHANNEL_DATA = 19;
    const uint32_t MASK_CHANNEL_DATA   = 0x7F;

    static const uint32_t PLANES_PER_STATION = 3;
    static const uint32_t MODULES_PER_PLANE = 8;
    static const uint32_t MODULES_PER_FRAGMENT = 8;
    static const uint32_t CHIPS_PER_MODULE = 12;
    static const uint32_t CHIPS_PER_SIDE = 6;
    static const uint32_t SIDES_PER_MODULE = 2;
    static const uint32_t STRIPS_PER_CHIP = 128;
    static const uint32_t STRIPS_PER_SIDE = STRIPS_PER_CHIP * CHIPS_PER_SIDE;
 
    TrackerDataFragment( const uint32_t *data, size_t size );

    void DecodeModuleData(std::map< std::pair<uint8_t, uint8_t>, std::vector<uint32_t> > dataMap);

    bool valid() const; 

    // getters
    uint32_t event_id() const { return event.m_event_id; }
    uint32_t bc_id() const { return event.m_bc_id; }
    size_t size() const { return m_size; }
    uint8_t trb_error_id() const { return event.m_trb_error_id;}
    std::vector<uint8_t>  module_error_id() const { return event.m_module_error_ids;}
    std::map< std::pair<uint8_t, uint8_t>,std::vector<uint32_t> >  module_modDB() const {return  event.m_modDB;}

    bool hasData(size_t module) const { return (event.GetModule(module) != nullptr); }
    const SCTEvent& operator[](size_t module) const { return *event.GetModule(module); }

    using const_iterator = std::vector<std::shared_ptr<SCTEvent>>::const_iterator;
    using iterator = std::vector<std::shared_ptr<SCTEvent>>::iterator;

    const_iterator cbegin() const { return event.m_hits_per_module.cbegin(); }
    const_iterator cend() const { return event.m_hits_per_module.cend(); }
    iterator begin() { return event.m_hits_per_module.begin(); }
    iterator end() { return event.m_hits_per_module.end(); }

    bool has_trb_error() const { return event.m_has_trb_error; }
    bool has_module_error() const { return event.m_module_error_ids.size() > 0; }
    bool has_crc_error() const { return event.m_crc != event.m_crc_calculated; }
    bool missing_event_id() const { return event.m_event_id_missing; }
    bool missing_bcid() const { return event.m_bc_id_missing; }
    bool missing_crc() const { return event.m_crc_missing; }
    bool missing_frames() const { return event.m_frame_counter_invalid; }
    bool unrecognized_frames() const { return event.m_unrecognized_frames; }

    //setters
    static void set_debug_on( bool debug = true ) { m_debug = debug; }

    // prohibit copy and assign
    TrackerDataFragment(const TrackerDataFragment& other) = delete;
    TrackerDataFragment& operator=(const TrackerDataFragment& other) = delete;

  private:
    size_t m_size;

    struct TRBEvent 
    {
    public:
        TRBEvent() {}
        std::shared_ptr<SCTEvent> GetModule(size_t moduleID) const { return m_hits_per_module[moduleID]; }
        void SetModule(size_t moduleID, std::shared_ptr<SCTEvent> sctEvent) { m_hits_per_module[moduleID] = sctEvent; }

        ~TRBEvent();

        uint32_t m_event_id;
        uint32_t m_bc_id;
        bool m_has_trb_error {false};
        bool m_event_id_missing {true};
        bool m_bc_id_missing {true};
        bool m_crc_missing {true};
        bool m_frame_counter_invalid {false};
        bool m_unrecognized_frames {false};
        uint8_t m_trb_error_id {0};
        uint32_t m_crc;
        uint32_t m_crc_calculated;
        std::vector< uint8_t > m_module_error_ids;
        std::map< std::pair<uint8_t, uint8_t>, std::vector<uint32_t> > m_modDB;
        std::vector < std::shared_ptr<SCTEvent> > m_hits_per_module { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

        // prohibit copy and assign
        TRBEvent(const TRBEvent& other) = delete;
        TRBEvent& operator=(const TRBEvent& other) = delete;

    }  event;
    inline static bool m_debug = false;

};

class Bitstream {
    public:
        Bitstream(std::vector<uint32_t> data);

        void Add(Bitstream stream) {m_bitstreamData.insert(m_bitstreamData.end(), stream.GetDataVector().begin(), stream.GetDataVector().end()); }
        void FillData(std::vector<uint32_t> data){ m_bitstreamData = data;}
        void RemoveBits(unsigned int n);
        uint32_t GetWord32() const {return m_currentBuffer;}
        std::vector<uint32_t> GetDataVector() const {return m_bitstreamData;}
        bool BitsAvailable() const {return (m_bitsAvailable > 0);}
        unsigned int NBitsAvailable() const {return m_bitsAvailable;}
        unsigned int AvailableWords() const {return m_bitstreamData.size();}

        Bitstream(const Bitstream& other) = delete;
        Bitstream& operator=(const Bitstream& other) = delete;

    private:
        uint32_t m_currentBuffer;
        std::vector<uint32_t> m_bitstreamData;
        unsigned int m_bitsUsedOfNextWord;
        long m_bitsAvailable;

        const unsigned int m_usedBitsPerWord = 24;
};


#include "TrackerDataFragment.icc" 



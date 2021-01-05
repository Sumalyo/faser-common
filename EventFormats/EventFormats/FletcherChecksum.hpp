#ifndef __CHECKSUM
#define __CHECKSUM

#include <iostream>
#include <vector>
#include <stdexcept>

/** *******************************************************
 \brief Class used for CRC computation
  ******************************************************** */
  class FletcherChecksum
  {
    public:
      void InitialiseChecksum(){
        m_checksumL = 0;
        m_checksumH = 0;
      }
      void AddData(const uint32_t& data){
        m_checksumL = m_checksumL + data;
        m_checksumH = m_checksumH + m_checksumL;
      }
      uint32_t ReturnChecksum(){
        return ComputeFletcherChecksum(m_checksumH, m_checksumL);
      }
      static uint32_t ReturnFletcherChecksum(const uint32_t* data, size_t size){
        std::vector<uint32_t> words(data, data+size/sizeof(uint32_t));
        size_t wordsTotal = words.size();
        if (wordsTotal <= 1) return 0;
        uint32_t checksumL(0);
        uint32_t checksumH(0);
        for (unsigned i = 0; i<wordsTotal-1; i++) {
          checksumL += words[i];
          checksumH += checksumL;
        }
        return ComputeFletcherChecksum(checksumH, checksumL);
      }

    private:
      uint32_t m_checksumL;
      uint32_t m_checksumH; 
      static uint32_t ComputeFletcherChecksum(const uint32_t& checksumH, const uint32_t& checksumL){
        uint32_t l_assembledChecksumH = ((checksumH>>(20-12)) & (0xFE0<<12)) | (checksumH & (0x1F<<12));
        return (l_assembledChecksumH & (0xFFF<< 12)) | (checksumL & 0xFFF);
      }
  };

#endif /* __CHECKSUM */

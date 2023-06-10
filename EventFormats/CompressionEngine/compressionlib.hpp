#ifndef COMPRESS_H
#include <iostream>  
#include <fstream>  // for file handling 
#include <cstring>  // to use some unsafe C things like memset
#include <sstream>
#include <zlib.h>  // for compression
#include <string.h> //for hadling string operations
#include <chrono>
#include <zstd.h>
#include <vector>
#include <filesystem>
#include "Exceptions/Exceptions.hpp"
#include "Logging.hpp"
#include <nlohmann/json.hpp>
#include "EventFormats/DAQFormats.hpp"
// #define CHUNK_SIZE 32768
#define CHUNK_SIZE 32768
#define ZSTD_LEVEL 15
typedef struct EventHeader{
    std::string Event = "0x00";
    std::string run = "0x00";
    std::string tag = "0x00";
    std::string bc = "0x00";
    std::string trig = "0x00";
    std::string status = "0x00";
    std::string time = "0x00";
    int fragmentCount = 0;
    uint32_t payloadSize = 0;
}EventHeader;

typedef struct EventData {
  EventHeader eventHeader;
  std::string inputSize = "0 bytes";
  std::string outputSize = "0 bytes";
  std::string compressionRatio = "0";
  std::string timeTaken = "0";
}EventData;

typedef struct compressionUtilityLog
{
    std::string Filename = "default";
    std::string Date = "default";
    std::string Compressor = "No Compressor Selected";
    std::string compressorConfig = "No Compressor Selected";
    int eventCount = 0;
    std::vector<EventData> evdata = {};

}compressionUtilityLog;
namespace CompressionUtility{
typedef std::map<std::string, std::string> configMap;
configMap readJsonToMap(const std::string& filename);
class EventCompressor {
public:
    compressionUtilityLog logstruct;
    compressionUtilityLog logstructDecompress;
    bool __isLogging;
    bool __isDecompressing;
    std::map<std:: string,std::string> CompressorConfig;
    virtual ~EventCompressor() {} // TODO To be reviewed
    // std::string __stateTracker ? TODO For state tracking and safety 
    std::string mapToString(const configMap& myMap);
    virtual void configCompression(configMap& config) = 0;
    virtual bool setupCompression() = 0;
    virtual bool setupCompressionAndLogging(std::string Filename) = 0;
    virtual bool Compressevent(DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent) = 0;
    virtual bool deCompressevent(DAQFormats::EventFull& inputEvent,std::vector<uint8_t>& compressedFragments, std::vector<uint8_t>& outputevent) = 0;
    void initializeStruct(std::string Filename,std::string Compressor,std::string config);
    void supportDecompression();
    void addEventData(EventData evData);
    void addEventDataDecompressed(EventData evData);
    void writeToJson(std::string filename);
};
class ZstdCompressor: public EventCompressor {
public:
    ZSTD_CCtx* ctx;
    ZstdCompressor():EventCompressor()
    {
    ctx =  ZSTD_createCCtx();
    };
    void configCompression(configMap& config);
    bool setupCompression();
    bool setupCompressionAndLogging(std::string Filename);
    bool Compressevent(DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent);
    bool deCompressevent(DAQFormats::EventFull& inputEvent,std::vector<uint8_t>& compressedFragments, std::vector<uint8_t>& outputFragments);
    ~ZstdCompressor();
};
}
#endif
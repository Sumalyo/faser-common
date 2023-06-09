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
  std::string timeForCompression = "0";
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
//bool compressFile(const std::string& inputFilename, const std::string& outputFilename,int mode);
double sizeByteMetric(const std::string& filename);
//bool compressString(const std::string& inputString,const std::string& outputFilename, int mode);
//bool zstdCompressor(std::ifstream& ifs,std::ofstream& ofs);
//bool zstdCompressorString(const std::string& input,const std::string& outputFilename);
bool zstdCompressorEvent(const std::vector<uint8_t>* inputevent, std::vector<uint8_t>& outputevent,bool reporting);
bool zstdCompressorEventDAQ(DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent);
//void showHelp(std::string progname);
typedef std::map<std::string, std::string> configMap;
class EventCompressor {
public:
    compressionUtilityLog logstruct;
    bool __isLogging;
    std::map<std:: string,std::string> CompressorConfig;
    // virtual ~EventCompressor() {} TODO To be implemneted 
    std::string mapToString(const configMap& myMap);
    virtual void configCompression(configMap& config) = 0;
    virtual bool setupCompression() = 0;
    virtual bool setupCompressionAndLogging(std::string Filename, std::string date) = 0;
    virtual bool Compressevent(DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent) = 0;
    virtual void closeCompressor() = 0;
    void initializeStruct(std::string Filename,std::string date,std::string Compressor,std::string config);
    void addEventData(EventData evData);
    void displayCompressionUtilityLog();
    void writeToJson(std::string filename);
};
class ZstdCompressor: public EventCompressor {
public:
    ZSTD_CCtx* ctx;
    ZstdCompressor():EventCompressor()
    {
    DEBUG("DEFAULT CONSTRCTOR CALLED");
    ctx =  ZSTD_createCCtx();
    };
    void configCompression(configMap& config);
    bool setupCompression();
    bool setupCompressionAndLogging(std::string Filename, std::string date);
    bool Compressevent(DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent);
    void closeCompressor();
};
}
#endif
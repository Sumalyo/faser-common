/*
FIXME :: There are quite a lot of warnings that come up here. Not strictly adhering to FASER DAQ coding standards. Please review and fix.
TODO :: Currently using std error. Try and use FASER Exception format
        Add logging capability for metrics
*/
# include "compressionlib.hpp"
CREATE_EXCEPTION_TYPE(CompressionEException,CompressionUtility)
typedef struct EventHeader{
    std::string Event = "0x00";
    std::string run = "0x00";
    std::string tag = "0x00";
    std::string bc = "0x00";
    std::string trig = "0x00";
    std::string stattus = "0x00";
    std::string time = "0x00";
    int fragmentCount = 0;
    int payloadSize = 0;
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

namespace nlohmann {

template <>
struct adl_serializer<EventHeader> {
    static void to_json(json& j, const EventHeader& header) {
        j = json{
            {"Event", header.Event},
            {"run", header.run},
            {"tag", header.tag},
            {"bc", header.bc},
            {"trig", header.trig},
            {"status", header.stattus},
            {"time", header.time},
            {"fragmentCount", header.fragmentCount},
            {"payloadSize", header.payloadSize}
        };
    }

    static void from_json(const json& j, EventHeader& header) {
        j.at("Event").get_to(header.Event);
        j.at("run").get_to(header.run);
        j.at("tag").get_to(header.tag);
        j.at("bc").get_to(header.bc);
        j.at("trig").get_to(header.trig);
        j.at("status").get_to(header.stattus);
        j.at("time").get_to(header.time);
        j.at("fragmentCount").get_to(header.fragmentCount);
        j.at("payloadSize").get_to(header.payloadSize);
    }
};

template <>
struct adl_serializer<EventData> {
    static void to_json(json& j, const EventData& eventData) {
        j = json{
            {"eventHeader", eventData.eventHeader},
            {"inputSize", eventData.inputSize},
            {"outputSize", eventData.outputSize},
            {"compressionRatio", eventData.compressionRatio},
            {"timeForCompression", eventData.timeForCompression}
        };
    }

    static void from_json(const json& j, EventData& eventData) {
        j.at("eventHeader").get_to(eventData.eventHeader);
        j.at("inputSize").get_to(eventData.inputSize);
        j.at("outputSize").get_to(eventData.outputSize);
        j.at("compressionRatio").get_to(eventData.compressionRatio);
        j.at("timeForCompression").get_to(eventData.timeForCompression);
    }
};

template <>
struct adl_serializer<compressionUtilityLog> {
    static void to_json(json& j, const compressionUtilityLog& log) {
        j = json{
            {"Filename", log.Filename},
            {"Date", log.Date},
            {"eventCount", log.eventCount},
            {"evdata", log.evdata}
        };
    }

    static void from_json(const json& j, compressionUtilityLog& log) {
        j.at("Filename").get_to(log.Filename);
        j.at("Date").get_to(log.Date);
        j.at("eventCount").get_to(log.eventCount);
        j.at("evdata").get_to(log.evdata);
    }
};

}


namespace CompressionUtility
{
typedef std::map<std::string, std::string> configMap;
using namespace nlohmann;
// Function to get the size of a file in bytes
double sizeByteMetric(const std::string& filename) {
    std::ifstream fileToAnalyze(filename, std::ios::binary);
    fileToAnalyze.seekg(0, std::ios::end);
    double sizeOfFile = static_cast<double>(fileToAnalyze.tellg()); 
    return sizeOfFile;
}

// Demo implementation of Event Compression - TDO Move this to a class based implemenation
bool zstdCompressorEvent(const std::vector<uint8_t>* inputevent, std::vector<uint8_t>& outputevent, bool reporting) {
    if (!inputevent) {
        std::cerr << "Error: input vector pointer is null" << std::endl;
        return false;
    } // Compress Event 

    // Create the zstd compression context
    ZSTD_CCtx* ctx = ZSTD_createCCtx();
    if (!ctx) {
        std::cerr << "Error: could not create zstd compression context" << std::endl;
        return false;
    } // Done

    // Set the compression parameters
    const int compressionLevel = 3;  // 1-22
    if (ZSTD_isError(ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compressionLevel))) {
        ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compressionLevel);
        //std::cout<<ZSTD_isError(ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compressionLevel))<<std::endl;
        std::cerr << "Error: could not set zstd compression level" << std::endl;
        return false;
    } //Done

    // Calculate the maximum output size for compression
    const size_t maxOutputSize = ZSTD_compressBound(inputevent->size());
    outputevent.resize(maxOutputSize); // Compress event

    // Compress the input data and store the compressed data in the output vector
    const size_t compressedSize = ZSTD_compressCCtx(ctx, outputevent.data(), maxOutputSize, inputevent->data(), inputevent->size(), compressionLevel);
    if (ZSTD_isError(compressedSize)) {
        std::cerr << "Error: zstd compression failed: " << ZSTD_getErrorName(compressedSize) << std::endl;
        return false;
    } // Compress Event

    // Resize the output vector to the actual compressed size
    outputevent.resize(compressedSize); // Compress Event

    // Destroy the zstd compression context
    ZSTD_freeCCtx(ctx); // In Destructor Done

    if (reporting){
    std::cout << "Input size: " << inputevent->size() << " bytes" << std::endl;
    std::cout << "Compressed size: " << compressedSize << " bytes" << std::endl;
    const double compressionRatio = static_cast<double>(inputevent->size()) / compressedSize;
    std::cout << "Compression ratio: " << compressionRatio << std::endl;} // Compress Event

    return true;
}

bool zstdCompressorEventDAQ(DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent ){
    outputevent = {1,2,3,4};
    std::cout<<inputEvent<<std::endl;
    return false;
}
/*
Insert Support for zlibcompression
*/

class EventCompressor{
    /*
    Event Compressor Interface
    - Add support for Compressor configuration - In HashMap
    - Add support Logging (On Demand) - Done with Structs and JSON data

    - 
    */

  public:
    compressionUtilityLog logstruct;
    std::map<std:: string,std::string> CompressorConfig;
    std::string mapToString(const configMap& myMap) {
    std::string result;
    for (const auto& pair : myMap) {
        result += pair.first + ":" + pair.second + "\n";
    }
    return result;
}
        //virtual bool Compressevent(const std::vector<uint8_t>* inputevent, std::vector<uint8_t>& outputevent) = 0;
    virtual void configCompression(configMap& config) = 0;
    virtual bool setupCompressionAndLoggingg(std::string Filename,std::string date) = 0;
    virtual bool setupCompression() = 0;
    virtual bool Compressevent(bool logmode = false) =0; // Working
    virtual void closeCompressor() = 0;
    
    void initializeStruct(std::string Filename,std::string date,std::string Compressor,std::string config){
        this->logstruct.Filename=Filename;
        this->logstruct.Date=date;
        this->logstruct.Compressor = Compressor;
        this->logstruct.compressorConfig = config;
        
    }
    void addEvetData(EventData evData)
    {
        this->logstruct.evdata.push_back(evData);
        this->logstruct.eventCount+=1;
    }

    void displayCompressionUtilityLog() {
    std::cout << "Filename: " << this->logstruct.Filename << std::endl;
    std::cout << "Date: " << this->logstruct.Date << std::endl;
    std::cout << "Event Count: " << this->logstruct.eventCount << std::endl;

    for (const EventData& eventData : this->logstruct.evdata) {
        std::cout << "Event Header:" << std::endl;
        std::cout << "  Event: " << eventData.eventHeader.Event << std::endl;
        std::cout << "  Run: " << eventData.eventHeader.run << std::endl;
        std::cout << "  Tag: " << eventData.eventHeader.tag << std::endl;
        std::cout << "  BC: " << eventData.eventHeader.bc << std::endl;
        std::cout << "  Trig: " << eventData.eventHeader.trig << std::endl;
        std::cout << "  Status: " << eventData.eventHeader.stattus << std::endl;
        std::cout << "  Time: " << eventData.eventHeader.time << std::endl;
        std::cout << "  Fragment Count: " << eventData.eventHeader.fragmentCount << std::endl;
        std::cout << "  Payload Size: " << eventData.eventHeader.payloadSize << std::endl;

        std::cout << "Input Size: " << eventData.inputSize << std::endl;
        std::cout << "Output Size: " << eventData.outputSize << std::endl;
        std::cout << "Compression Ratio: " << eventData.compressionRatio << std::endl;
        std::cout << "Time for Compression: " << eventData.timeForCompression << std::endl;

        std::cout << std::endl;
    }

}

    void writeToJson(std::string filename)
    {
        json log = this->logstruct;
        std::ofstream outputFile("/home/osboxes/gsocContributions/faser-common/EventFormats/apps/Logs/"+filename+".json",std::ios::out);
        if (outputFile.is_open()) {
            outputFile << log.dump(4); // The "4" argument adds indentation for better readability
            outputFile.close();
            std::cout << "JSON file created successfully." << std::endl;
        } else {
            std::cout << "Unable to create JSON file." << std::endl;
        }
    }
};


class ZstdCompressor : EventCompressor
{
    public:
    ZSTD_CCtx* ctx;
    void configCompression(configMap& config) override{
        for (const auto& pair : config) {
        this->CompressorConfig[pair.first] = pair.second;  // I can use this to set up enforcemnets later like

    }
        /*
        this->CompressorConfig["compressionLevel"] = config["compressionLevel"]
        and throw exceptions if an invalid config is passed on;
        */
    }
    bool setupCompression() override
    {
        ZSTD_CCtx* ctx = ZSTD_createCCtx();
        if (!ctx) {
        std::cerr << "Error: could not create zstd compression context" << std::endl;
        return false;
        }
        try
        {
            std::string valueOfKey1 = this->CompressorConfig["compressionLevel"];
            const int compressionLevel = std::stoi(valueOfKey1);
            if (ZSTD_isError(ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compressionLevel))) {
            ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compressionLevel);
            std::cerr << "Error: could not set zstd compression level" << std::endl;
            return false;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n'; //TODO Replace with DAQ Exceptions
            return false;
        }
        return true;
    }
    bool setupCompressionAndLoggingg(std::string Filename,std::string date) override
    {
        std::string config = this->mapToString(this->CompressorConfig);
        this->initializeStruct(Filename,date,"ZSTD COmpressor",config);
        bool isSetup = this->setupCompression();
        return isSetup;
    }


    bool Compressevent  ( bool logmode = false) override{
            return false;
        }
    void closeCompressor() override
    {
        ZSTD_freeCCtx(ctx);
    }
};

// class ZlibCompressor : EventCompressor
// {
//     public:
//     bool Compressevent  ( bool logmode = false ) override{
//             this->__loggingMode = logmode;
//             return false;
//         }
//         void ResultMetrics() override{
            
//         }
// };





// }

/*
APIs for Compression Events or Fragments or other Faser Data Formats to be added here
Plan for Logging:

*/
}

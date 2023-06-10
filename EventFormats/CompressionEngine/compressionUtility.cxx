/*
FIXME :: There are quite a lot of warnings that come up here. Not strictly adhering to FASER DAQ coding standards. Please review and fix.
TODO :: Currently using std error. Try and use FASER Exception format
        Add logging capability for metrics - Done
*/
# include "compressionlib.hpp"

CREATE_EXCEPTION_TYPE(CompressionEException,CompressionUtility)

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
            {"status", header.status},
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
        j.at("status").get_to(header.status);
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
            {"timeTaken", eventData.timeTaken}
        };
    }

    static void from_json(const json& j, EventData& eventData) {
        j.at("eventHeader").get_to(eventData.eventHeader);
        j.at("inputSize").get_to(eventData.inputSize);
        j.at("outputSize").get_to(eventData.outputSize);
        j.at("compressionRatio").get_to(eventData.compressionRatio);
        j.at("timeTaken").get_to(eventData.timeTaken);
    }
};

template <>
struct adl_serializer<compressionUtilityLog> {
    static void to_json(json& j, const compressionUtilityLog& log) {
        j = json{
            {"Filename", log.Filename},
            {"Date", log.Date},
            {"Compressor",log.Compressor},
            {"compressorConfig",log.compressorConfig},
            {"eventCount", log.eventCount},
            {"evdata", log.evdata}
        };
    }

    static void from_json(const json& j, compressionUtilityLog& log) {
        j.at("Filename").get_to(log.Filename);
        j.at("Date").get_to(log.Date);
        j.at("Compressor").get_to(log.Compressor);
        j.at("compressorConfig").get_to(log.compressorConfig);
        j.at("eventCount").get_to(log.eventCount);
        j.at("evdata").get_to(log.evdata);
    }
};

}


namespace CompressionUtility
{

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
    } // Compress Event - Omit for now

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

// class EventCompressor{
    /*
    Event Compressor Interface
    - Add support for Compressor configuration - In HashMap
    - Add support Logging (On Demand) - Done with Structs and JSON data

    - 
    */

//   public:
//     compressionUtilityLog logstruct;
//     bool __isLogging;
//     std::map<std:: string,std::string> CompressorConfig;

    std::string EventCompressor::mapToString(const configMap& myMap) 
    {
    std::string result;
    for (const auto& pair : myMap) {
        result += pair.first + ":" + pair.second + "\n";
    }
    return result;
    }
        //virtual bool Compressevent(const std::vector<uint8_t>* inputevent, std::vector<uint8_t>& outputevent) = 0;
    // virtual void EventCompressor::configCompression(configMap& config) = 0;
    // virtual bool EventCompressor::setupCompressionAndLoggingg(std::string Filename,std::string date) = 0;
    // virtual bool EventCompressor::setupCompression() = 0;
    /*
    For now the plan is to only use this to log various metrics for compression and not use the compressed Event fragment 
    To build a new Event. The best way here seems to be by using some constrctor for EventFull that would update the framement 
    of the event and set the appropriate fileds in the Header, return it so that it can be written to a file outout stream.
    
    */
    // virtual bool EventCompressor::Compressevent(DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent) =0;

    // virtual void EventCompressor::closeCompressor() = 0;
    
    void EventCompressor::initializeStruct(std::string Filename,std::string date,std::string Compressor,std::string config){
        this->logstruct.Filename=Filename;
        this->logstruct.Date=date;
        this->logstruct.Compressor = Compressor;
        this->logstruct.compressorConfig = config;
        this->logstructDecompress.Filename=Filename;
        this->logstructDecompress.Date=date;
        this->logstructDecompress.Compressor = Compressor + ":: Decompression Run ";
        this->logstructDecompress.compressorConfig = config;
        
    }
    void EventCompressor::supportDecompression()
    {
        this->__isDecompressing=true;
    }
    void EventCompressor::addEventData(EventData evData)
    {
        this->logstruct.evdata.push_back(evData);
        this->logstruct.eventCount+=1;
    }
    void EventCompressor::addEventDataDecompressed(EventData evData)
    {
        this->logstructDecompress.evdata.push_back(evData);
        this->logstructDecompress.eventCount+=1;
    }
    void EventCompressor::writeToJson(std::string filename)
    {
        json log = this->logstruct;
        std::ofstream outputFile("../EventFormats/apps/Logs/"+filename+".json",std::ios::out);
        if (outputFile.is_open()) {
            outputFile << log.dump(4);
            outputFile.close();
            std::cout << "JSON file created successfully." << std::endl;
        } else {
            std::cout << "Unable to create JSON file." << std::endl;
        }
        if(this->__isDecompressing)
        {
            json log = this->logstructDecompress;
            std::ofstream outputFile("../EventFormats/apps/Logs/"+filename+"_decompression.json",std::ios::out);
            if (outputFile.is_open()) {
                outputFile << log.dump(4);
                outputFile.close();
                std::cout << "JSON file created successfully." << std::endl;
            } else {
                std::cout << "Unable to create JSON file." << std::endl;
            }
        }
    }
// };


// class ZstdCompressor : EventCompressor
// {
//     public:
//     ZSTD_CCtx* ctx;
    void ZstdCompressor::configCompression(configMap& config) {
        for (const auto& pair : config) {
        this->CompressorConfig[pair.first] = pair.second;  // I can use this to set up enforcemnets later like

    }
        /*
        this->CompressorConfig["compressionLevel"] = config["compressionLevel"]
        and throw exceptions if an invalid config is passed on;
        */
    }
    bool ZstdCompressor::setupCompression()
    {
        ZSTD_CCtx* ctx = ZSTD_createCCtx();
        if (!ctx) {
        ERROR("Error: could not create zstd compression context");
        //std::cerr << "Error: could not create zstd compression context" << std::endl;
        return false;
        }
        try
        {
            std::string valueOfKey1 = this->CompressorConfig["compressionLevel"];
            const int compressionLevel = std::stoi(valueOfKey1);
            if (ZSTD_isError(ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compressionLevel))) {
            ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compressionLevel);
            ERROR( "Error: could not set zstd compression level" );
            return false;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n'; //TODO Replace with DAQ Exceptions
            return false;
        }
        INFO("Log :: ZSTD Compressor is set up");  
        this->__isLogging=false;
        return true;
    }
    bool ZstdCompressor::setupCompressionAndLogging(std::string Filename,std::string date){
        
        std::string config = this->mapToString(this->CompressorConfig);
        this->initializeStruct(Filename,date,"ZSTD COmpressor",config);
        bool isSetup = this->setupCompression();
        if(isSetup)
        {
            this->__isLogging=true; // Indicate that the Logging is set up
            INFO("Log:: Logging Enabled in Compressor");
        }
        return isSetup;
    }

    bool ZstdCompressor::Compressevent  ( DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent) {
            /*
            Steps TO DO
            - [ ] Extract the Event Header information and populate the Event Data in the Logging Struct
            - [ ] Load the Compressor Configuration appropriately
            - [ ] Do Compression and store result in a stream of bytes
            - [ ] Record metrics and populate struct accordingly
            */
           
           std::vector<uint8_t>* eventFragments = inputEvent.raw_fragments();


           const size_t maxOutputSize = ZSTD_compressBound(eventFragments->size());
    
           outputevent.resize(maxOutputSize);
    
            // Compress the input data and store the compressed data in the output vector
            int compressionLevel = std::stoi(this->CompressorConfig["compressionLevel"]);
            auto start = std::chrono::high_resolution_clock::now();
            const size_t compressedSize = ZSTD_compressCCtx(ctx, outputevent.data(), maxOutputSize, eventFragments->data(), eventFragments->size(), compressionLevel);
            if (ZSTD_isError(compressedSize)) {

                std::cerr << "Error: zstd compression failed: " << ZSTD_getErrorName(compressedSize) << std::endl;
                return false;
            } // Compress Event
            // Resize the output vector to the actual compressed size
            outputevent.resize(compressedSize); // Compress Event
            inputEvent.updateStatus(1<<11);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            if (this->__isLogging)
            {
                EventData evData;
                evData.eventHeader.Event = std::to_string(inputEvent.event_counter());
                evData.eventHeader.run = std::to_string(inputEvent.run_number());
                evData.eventHeader.tag = std::to_string(static_cast<int>(inputEvent.event_tag()));
                evData.eventHeader.status = std::to_string(static_cast<int>(inputEvent.status()));
                evData.eventHeader.bc = std::to_string(inputEvent.bc_id());
                evData.eventHeader.fragmentCount = inputEvent.fragment_count();
                evData.eventHeader.payloadSize = inputEvent.payload_size();
                evData.eventHeader.trig = std::to_string(inputEvent.trigger_bits());
                evData.eventHeader.time = std::to_string(inputEvent.timestamp());
                evData.inputSize = std::to_string(eventFragments->size()); // TODO See Best Implementation
                evData.outputSize = std::to_string(compressedSize);
                evData.compressionRatio = std::to_string(static_cast<double>(eventFragments->size()) / compressedSize);
                evData.timeTaken = std::to_string(duration.count()); // Time in microseconds
                this->addEventData(evData);
            }
            

            return true;
        }

    bool ZstdCompressor::deCompressevent(DAQFormats::EventFull& inputEvent,std::vector<uint8_t>& compressedFragments, std::vector<uint8_t>& outputFragments)
    {
        auto start = std::chrono::high_resolution_clock::now();
        // Time Sensitive Code
        const size_t maxDecompressedSize = ZSTD_getFrameContentSize(compressedFragments.data(), compressedFragments.size());
        if (maxDecompressedSize == ZSTD_CONTENTSIZE_ERROR || maxDecompressedSize == ZSTD_CONTENTSIZE_UNKNOWN) {
            std::cerr << "Error: Invalid compressed data" << std::endl;
            return false;
        }

        outputFragments.resize(maxDecompressedSize);

        const size_t decompressedSize = ZSTD_decompress(outputFragments.data(), maxDecompressedSize,
                                                        compressedFragments.data(), compressedFragments.size());
        if (ZSTD_isError(decompressedSize)) {
            std::cerr << "Error: Failed to decompress data: " << ZSTD_getErrorName(decompressedSize) << std::endl;
            return false;
        }

        outputFragments.resize(decompressedSize);
    
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        if (this->__isLogging)
        {
            EventData evData;
            evData.eventHeader.Event = std::to_string(inputEvent.event_counter());
            evData.eventHeader.run = std::to_string(inputEvent.run_number());
            evData.eventHeader.tag = std::to_string(static_cast<int>(inputEvent.event_tag()));
            evData.eventHeader.status = std::to_string(static_cast<int>(inputEvent.status()));
            evData.eventHeader.bc = std::to_string(inputEvent.bc_id());
            evData.eventHeader.fragmentCount = inputEvent.fragment_count();
            evData.eventHeader.payloadSize = inputEvent.payload_size();
            evData.eventHeader.trig = std::to_string(inputEvent.trigger_bits());
            evData.eventHeader.time = std::to_string(inputEvent.timestamp());
            evData.inputSize = std::to_string(decompressedSize); // TODO See Best Implementation
            evData.outputSize =std::to_string(compressedFragments.size());
            evData.compressionRatio =std::to_string(static_cast<double>(decompressedSize/compressedFragments.size()));
            evData.timeTaken = std::to_string(duration.count()); // Time in microseconds
            this->addEventDataDecompressed(evData);
        }
        return true;
    }

    ZstdCompressor::~ZstdCompressor(){
        ZSTD_freeCCtx(ctx);
    }
// };

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

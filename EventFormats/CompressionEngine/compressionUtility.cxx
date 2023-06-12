/*
TODO :: There are quite a lot of warnings that come up here. Not strictly adhering to FASER DAQ coding standards. Please review and fix.
TODO :: Currently using std error. Try and use FASER Exception format
    - Add logging capability for metrics - Done
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

configMap readJsonToMap(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file." << std::endl;
        return {};
    }

    json jsonData;
    try {
        file >> jsonData;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        return {};
    }

    configMap dataMap;
    for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
        if (it.value().is_string()) {
            dataMap[it.key()] = it.value();
        }
    }

    return dataMap;
}
std::string generateUniqueIdentifier() {
    // Get the current date and time
    std::time_t currentTime = std::time(nullptr);
    std::tm* localTime = std::localtime(&currentTime);

    // Format the date and time components as strings
    std::stringstream ss;
    ss << std::put_time(localTime, "%Y%m%d%H%M%S");
    std::string timestamp = ss.str();

    // Generate a random number to ensure uniqueness in case of rapid successive runs
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9999);
    int randomNum = dis(gen);

    // Combine the timestamp and random number to form the unique identifier
    std::string uniqueIdentifier = timestamp + "_" + std::to_string(randomNum);
    return uniqueIdentifier;
}
/*
Event Compressor Interface
- Add support for Compressor configuration - In HashMap
- Add support Logging (On Demand) - Done with Structs and JSON data
*/

std::string EventCompressor::mapToString(const configMap& myMap) 
{
    std::string result;
    for (const auto& pair : myMap) {
    result += pair.first + ":" + pair.second + "\n";
    }
    return result;
}
/*
- For now the plan is to only use this to log various metrics for compression and not use the compressed Event fragment 
- To build a new Event. The best way here seems to be by using some constrctor for EventFull that would update the framement 
- of the event and set the appropriate fileds in the Header, return it so that it can be written to a file outout stream.

*/    
void EventCompressor::initializeStruct(std::string Filename,std::string Compressor,std::string config){
    std::time_t currentTime = std::time(nullptr);
    std::tm* localTime = std::localtime(&currentTime);
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << localTime->tm_mday << '/'
        << std::setw(2) << localTime->tm_mon + 1 << '/'
        << localTime->tm_year + 1900;
    std::string currentDate = oss.str();
    this->logstruct.Filename=Filename;
    this->logstruct.Date=currentDate;
    this->logstruct.Compressor = Compressor;
    this->logstruct.compressorConfig = config;
    this->logstructDecompress.Filename=Filename;
    this->logstructDecompress.Date=currentDate;
    this->logstructDecompress.Compressor = Compressor + ":: Decompression Run ";
    this->logstructDecompress.compressorConfig = config;
    
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
void ZstdCompressor::supportDecompression()
{
    this->__isDecompressing=true;
}
bool ZstdCompressor::setupCompressionAndLogging(std::string Filename){
    
    std::string config = this->mapToString(this->CompressorConfig);
    this->initializeStruct(Filename,"ZSTD COmpressor",config);
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
    ToDo Add dictionary based compression support
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
    inputEvent.toggleCompression();
    inputEvent.updatePayloadSize(compressedSize);
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
    //Todo optimize this if possible using a common decompression context
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
    inputEvent.toggleCompression();
    inputEvent.updatePayloadSize(decompressedSize);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    if (this->__isLogging && this->__isDecompressing)
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
        evData.inputSize = std::to_string(compressedFragments.size()); // TODO See Best Implementation
        evData.outputSize =std::to_string(decompressedSize);
        evData.compressionRatio =std::to_string(static_cast<double>(decompressedSize/compressedFragments.size()));
        evData.timeTaken = std::to_string(duration.count()); // Time in microseconds
        this->addEventDataDecompressed(evData);
    }
    return true;
}

ZstdCompressor::~ZstdCompressor(){
        ZSTD_freeCCtx(ctx);
    }


// ---- Adding zlib support
void ZlibCompressor::configCompression(configMap& config) {
    for (const auto& pair : config) {
    this->CompressorConfig[pair.first] = pair.second;  // I can use this to set up enforcemnets later like

}
    /*
    ?this->CompressorConfig["compressionLevel"] = config["compressionLevel"]
    ?this->CompressorConfig["compressionLevel"] = config["compressionLevel"]
    ?and throw exceptions if an invalid config is passed on;
    */
}
bool ZlibCompressor::setupCompression()
{
    std::memset(&stream, 0, sizeof(stream));
    try
    {
        std::string valueOfKey1 = this->CompressorConfig["compressionLevel"];
        this->compressionLevel = std::stoi(valueOfKey1);
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        
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
void ZlibCompressor::supportDecompression()
{
    this->__isDecompressing=true;
    decompressstream.zalloc = Z_NULL;
    decompressstream.zfree = Z_NULL;
    decompressstream.opaque = Z_NULL;
}
bool ZlibCompressor::setupCompressionAndLogging(std::string Filename){
    
    std::string config = this->mapToString(this->CompressorConfig);
    this->initializeStruct(Filename,"Zlib COmpressor",config);
    bool isSetup = this->setupCompression();
    if(isSetup)
    {
        this->__isLogging=true; // Indicate that the Logging is set up
        INFO("Log:: Logging Enabled in Compressor");
    }
    return isSetup;
}
bool ZlibCompressor::Compressevent( DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent) {
    /*
    Steps TO DO
    - [ ] Extract the Event Header information and populate the Event Data in the Logging Struct
    - [ ] Load the Compressor Configuration appropriately
    - [ ] Do Compression and store result in a stream of bytes
    - [ ] Record metrics and populate struct accordingly
    ToDo Add dictionary based compression support
    */
   
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<uint8_t>* eventFragments = inputEvent.raw_fragments();
    const uint8_t* input = eventFragments->data();
    size_t inputSize = eventFragments->size();
    uLongf compressedSize = compressBound(inputSize);
    outputevent.resize(compressedSize);
    stream.avail_in = inputSize;
    stream.next_in = const_cast<Bytef*>(input);
    stream.avail_out = compressedSize;
    stream.next_out = outputevent.data();

    // Initialize the compression
    if (deflateInit(&stream, compressionLevel) != Z_OK)
    {
        std::cerr << "Compression initialization failed." << std::endl;
        return false;
    }
    // Compress the data
    if (deflate(&stream, Z_FINISH) != Z_STREAM_END)
    {
        deflateEnd(&stream);
        std::cerr << "Compression failed." << std::endl;
        return false;
    }
    outputevent.resize(stream.total_out);
    
    
    //const size_t compressedSize = 0;
    inputEvent.toggleCompression();
    inputEvent.updatePayloadSize(compressedSize);
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

bool ZlibCompressor::deCompressevent(DAQFormats::EventFull& inputEvent,std::vector<uint8_t>& compressedFragments, std::vector<uint8_t>& outputFragments)
{
    //
    auto start = std::chrono::high_resolution_clock::now();
    const uint8_t* input = compressedFragments.data();
    size_t inputSize = compressedFragments.size();
    uLongf decompressedSize = 0;
    decompressstream.avail_in = inputSize;
    decompressstream.next_in = const_cast<Bytef*>(input);
    if (inflateInit(&decompressstream) != Z_OK)
    {
        std::cerr << "Decompression initialization failed." << std::endl;
        return false;
    }
    int result = -1;
    // Decompress the data
    do {
        // Determine the size of the output buffer needed
        decompressedSize += 1024; // Increase buffer size as needed
        outputFragments.resize(decompressedSize);

        decompressstream.avail_out = decompressedSize - decompressstream.total_out;
        decompressstream.next_out = outputFragments.data() + decompressstream.total_out;

        result = inflate(&decompressstream, Z_NO_FLUSH);

        if (result == Z_STREAM_ERROR)
        {
            inflateEnd(&decompressstream);
            std::cerr << "Decompression failed." << std::endl;
            return false;
        }

    } while (result != Z_STREAM_END);
    outputFragments.resize(stream.total_out);
    inputEvent.toggleCompression();
    inputEvent.updatePayloadSize(decompressedSize);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    if (this->__isLogging && this->__isDecompressing)
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
        evData.inputSize = std::to_string(compressedFragments.size()); // TODO See Best Implementation
        evData.outputSize =std::to_string(decompressedSize);
        evData.compressionRatio =std::to_string(static_cast<double>(decompressedSize/compressedFragments.size()));
        evData.timeTaken = std::to_string(duration.count()); // Time in microseconds
        this->addEventDataDecompressed(evData);
    }


    return true;
}

ZlibCompressor::~ZlibCompressor(){
        deflateEnd(&stream);
        if (this->__isDecompressing)
        inflateEnd(&decompressstream);
    }
}

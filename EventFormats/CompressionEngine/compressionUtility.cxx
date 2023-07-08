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
    // ZSTD_CCtx* ctx = ZSTD_createCCtx();
    if (!ctx) {
    ERROR("Error: could not create zstd compression context");
    //std::cerr << "Error: could not create zstd compression context" << std::endl;
    return false;
    }
    try
    {
        //std::string valueOfKey1 = this->CompressorConfig["compressionLevel"];
        std::string valueOfKey1 = this->CompressorConfig.at("compressionLevel");
        this->compressionLevel = std::stoi(valueOfKey1);
    }
    catch (const std::out_of_range& e) {
        INFO("Using Default Compression level 7");
        this->compressionLevel = 7;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n'; //TODO Replace with DAQ Exceptions
        return false;
    }
    try
    {
        //std::string valueOfKey2 = this->CompressorConfig["useDictionary"];
        std::string valueOfKey2 = this->CompressorConfig.at("useDictionary");
        this->dictionarySupport = std::stoi(valueOfKey2);
    }
    catch (const std::out_of_range& e) {
        //INFO("Not using dictionary Compression ");
        this->dictionarySupport = 0;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n'; //TODO Replace with DAQ Exceptions
        return false;
    }

    int dictionarySupport = this->dictionarySupport;
    if (dictionarySupport)
    {

        INFO("Using Dictionary for compression");
        std::vector<uint8_t> dictionary; // FIXME BUG Memory Manage this
        if (!this->dictionaryPath.empty()) {
        if (!loadDictionaryFromFile(this->dictionaryPath, dictionary)) {
            // Failed to load the dictionary
            ZSTD_freeCCtx(ctx);
            return false;
        }
        ZSTD_CCtx_loadDictionary(ctx, dictionary.data(), dictionary.size());
    }

    }
    else{
        INFO("Not using Dictionary compression here");
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
    this->initializeStruct(Filename,"ZSTD Compressor",config);
    bool isSetup = this->setupCompression();
    if(isSetup)
    {
        this->__isLogging=true; // Indicate that the Logging is set up
        INFO("Log:: Logging Enabled in Compressor");
    }
    return isSetup;
}

bool ZstdCompressor::loadDictionaryFromFile(const std::string& filePath, std::vector<uint8_t>& dictionary)
{
    // Open the file in binary mode
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        // Failed to open the file
        return false;
    }

    // Get the size of the file
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read the file contents into the dictionary vector
    dictionary.resize(fileSize);
    file.read(reinterpret_cast<char*>(dictionary.data()), fileSize);

    // Close the file
    file.close();

    return true;
}

bool ZstdCompressor::Compressevent  ( DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent_vector) {
    /*
    Steps TO DO
    - [ ] Extract the Event Header information and populate the Event Data in the Logging Struct
    - [ ] Load the Compressor Configuration appropriately
    - [ ] Do Compression and store result in a stream of bytes
    - [ ] Record metrics and populate struct accordingly
    ToDo Add dictionary based compression support
    */
    
    std::vector<uint8_t>* eventFragments = inputEvent.raw_fragments();
    std::vector<uint8_t> outputevent;



    const size_t maxOutputSize = ZSTD_compressBound(eventFragments->size());

    outputevent.resize(maxOutputSize);
    outputevent_vector.resize(maxOutputSize);

    // Compress the input data and store the compressed data in the output vector
    //int compressionLevel = std::stoi(this->CompressorConfig["compressionLevel"]); // // FIXME Bug here Please use a data member compression level and populate from there
    int compressionLevel = this->compressionLevel;
    auto start = std::chrono::high_resolution_clock::now();
    size_t compressedSize = ZSTD_compressCCtx(ctx, outputevent.data(), maxOutputSize, eventFragments->data(), eventFragments->size(), compressionLevel);
    if (ZSTD_isError(compressedSize)) {

        std::cerr << "Error: zstd compression failed: " << ZSTD_getErrorName(compressedSize) << std::endl;
        return false;
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    
    outputevent.resize(compressedSize);
    compressedSize = ZSTD_compressCCtx(ctx, outputevent_vector.data(), maxOutputSize, eventFragments->data(), eventFragments->size(), compressionLevel);
    if (ZSTD_isError(compressedSize)) {

        std::cerr << "Error: zstd compression failed: " << ZSTD_getErrorName(compressedSize) << std::endl;
        return false;
    }
     // Compress Event
    // Resize the output vector to the actual compressed size
     // Compress Event
    outputevent_vector.resize(compressedSize);
    inputEvent.loadCompressedData(outputevent);
    //inputEvent.setCompressionAlgo(0x0100);
    if (this->dictionarySupport)
    {
        inputEvent.setCompressionAlgo(0x0A);
        INFO("Using Dictionary for compression");
    }
    else{
        inputEvent.setCompressionAlgo(0x01);
        DEBUG("Not using Dictionary compression here");
    }
    
    // Event -> Raw Fragments -> outputevent
    // 
    // inputEvent.toggleCompression();
    //inputEvent.updatePayloadSize(compressedSize);
    //inputEvent.loadCompressedData(outputevent);

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
//
bool ZstdCompressor::deCompressevent(DAQFormats::EventFull& inputEvent,std::vector<uint8_t>& compressedFragments, std::vector<uint8_t>& outputFragments)
{
    if (!this->dictionarySupport)
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
    //inputEvent.toggleCompression();
    //inputEvent.updatePayloadSize(decompressedSize);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    if (this->__isLogging && this->__isDecompressing)
    {
        EventData evData;
        evData.eventHeader.Event = std::to_string(inputEvent.event_counter());
        evData.eventHeader.run = std::to_string(inputEvent.run_number());
        evData.eventHeader.tag = std::to_string(static_cast<int>(inputEvent.event_tag()));
        evData.eventHeader.status = std::to_string(static_cast<int>(inputEvent.status()^1<<11)); // dummy for compression
        evData.eventHeader.bc = std::to_string(inputEvent.bc_id());
        evData.eventHeader.fragmentCount = inputEvent.fragment_count();
        evData.eventHeader.payloadSize = decompressedSize; // dummy for logging
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
    else
    {
        //auto start = std::chrono::high_resolution_clock::now(); // ? Should I start the clock here
        ZSTD_DCtx* dctx = ZSTD_createDCtx();
        std::vector<uint8_t> dictionary;
        std::string dictionaryFilePath = this->dictionaryPath;
        if (!dictionaryFilePath.empty()) {
            if (!loadDictionaryFromFile(dictionaryFilePath, dictionary)) {
                // Failed to load the dictionary
                ZSTD_freeDCtx(dctx);
                return false;
            }
            ZSTD_DCtx_loadDictionary(dctx, dictionary.data(), dictionary.size());
        }
        auto start = std::chrono::high_resolution_clock::now();
        size_t maxDecompressedSize = ZSTD_getFrameContentSize(compressedFragments.data(), compressedFragments.size());
        if (maxDecompressedSize == ZSTD_CONTENTSIZE_ERROR || maxDecompressedSize == ZSTD_CONTENTSIZE_UNKNOWN) {
            ZSTD_freeDCtx(dctx);
            return false;
        }

        // Resize the output vector to accommodate the decompressed data
        outputFragments.resize(maxDecompressedSize);

        // Decompress the input vector
        size_t decompressedSize = ZSTD_decompressDCtx(dctx, outputFragments.data(), outputFragments.size(), compressedFragments.data(), compressedFragments.size());
        

        // Check if decompression was successful
        if (ZSTD_isError(decompressedSize)) {
            // Decompression failed
            ZSTD_freeDCtx(dctx);
            return false;
        }

        // Resize the output vector to the actual decompressed size
        outputFragments.resize(decompressedSize);

        // Cleanup and return success
        ZSTD_freeDCtx(dctx);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    if (this->__isLogging && this->__isDecompressing)
    {
        EventData evData;
        evData.eventHeader.Event = std::to_string(inputEvent.event_counter());
        evData.eventHeader.run = std::to_string(inputEvent.run_number());
        evData.eventHeader.tag = std::to_string(static_cast<int>(inputEvent.event_tag()));
        evData.eventHeader.status = std::to_string(static_cast<int>(inputEvent.status()^1<<11)); // dummy for compression
        evData.eventHeader.bc = std::to_string(inputEvent.bc_id());
        evData.eventHeader.fragmentCount = inputEvent.fragment_count();
        evData.eventHeader.payloadSize = decompressedSize; // dummy for logging
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
    return false;
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
        std::string valueOfKey1 = this->CompressorConfig.at("compressionLevel");
        this->compressionLevel = std::stoi(valueOfKey1);
    }
    catch (const std::out_of_range& e) {
        INFO("Using Default Compression level 3");
        this->compressionLevel = 3;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n'; //TODO Replace with DAQ Exceptions
        return false;
    }
    try
    {
    std::string valueOfKey2 = this->CompressorConfig.at("bufferSize");
    this->bufferSize = std::stoi(valueOfKey2);
    }catch (const std::out_of_range& e) {
        INFO("Using Default Buffer Size 1024");
        this->bufferSize = 1024;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n'; //TODO Replace with DAQ Exceptions
        return false;
    }
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    INFO("Log :: Zlib Compressor is set up");  
    this->__isLogging=false;
    return true;
}
void ZlibCompressor::supportDecompression()
{
    this->__isDecompressing=true; //FIXME Add support for decompression here - Test
    // decompressstream.zalloc = Z_NULL;
    // decompressstream.zfree = Z_NULL;
    // decompressstream.opaque = Z_NULL;
}
bool ZlibCompressor::setupCompressionAndLogging(std::string Filename){
    
    std::string config = this->mapToString(this->CompressorConfig);
    this->initializeStruct(Filename,"Zlib Compressor",config);
    bool isSetup = this->setupCompression();
    if(isSetup)
    {
        this->__isLogging=true; // Indicate that the Logging is set up
        INFO("Log:: Logging Enabled in Compressor");
    }
    return isSetup;
}
bool ZlibCompressor::Compressevent( DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent_vector) {
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
    size_t compressedSize = 0;
    std::vector<uint8_t> outputevent;
    stream.next_in = eventFragments->data();
    stream.avail_in = eventFragments->size();
    if (deflateInit(&stream, this->compressionLevel) != Z_OK)
        return false;
    std::vector<uint8_t> compressedBuffer(static_cast<size_t>(this->bufferSize));
    do {
        stream.next_out = compressedBuffer.data();
        stream.avail_out = compressedBuffer.size();
        int result = deflate(&stream, Z_FINISH);

        if (result == Z_STREAM_ERROR) {
            deflateEnd(&stream);
            return false;
        }
        compressedSize = compressedBuffer.size() - stream.avail_out;

        // Append the compressed data to the result vector
        outputevent.insert(outputevent.end(), compressedBuffer.begin(), compressedBuffer.begin() + static_cast<long int>(compressedSize));
        outputevent_vector.insert(outputevent_vector.end(), compressedBuffer.begin(), compressedBuffer.begin() + static_cast<long int>(compressedSize));
    } while (stream.avail_out == 0);
    inputEvent.loadCompressedData(outputevent);
    //inputEvent.setCompressionAlgo(0x0200); // Internal Code for zlib compression
    inputEvent.setCompressionAlgo(0x02); // Internal Code for zlib compression
    //inputEvent.updatePayloadSize(compressedSize);
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
        evData.outputSize = std::to_string(outputevent.size());
        evData.compressionRatio = std::to_string(static_cast<double>(eventFragments->size()) / outputevent.size());
        evData.timeTaken = std::to_string(duration.count()); // Time in microseconds
        this->addEventData(evData);
    }
    
    return true;

}

bool ZlibCompressor::deCompressevent(DAQFormats::EventFull& inputEvent,std::vector<uint8_t>& compressedFragments, std::vector<uint8_t>& outputFragments)
{
    
    auto start = std::chrono::high_resolution_clock::now();
    uLongf decompressedSize = compressedFragments.size() * 3;
    outputFragments.clear();
    outputFragments.resize(decompressedSize);
      int result = uncompress(outputFragments.data(), &decompressedSize, compressedFragments.data(), compressedFragments.size());

      if (result != Z_OK) {
          return false;
      }
      outputFragments.resize(decompressedSize);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    if (this->__isLogging && this->__isDecompressing)
    {
        EventData evData;
        evData.eventHeader.Event = std::to_string(inputEvent.event_counter());
        evData.eventHeader.run = std::to_string(inputEvent.run_number());
        evData.eventHeader.tag = std::to_string(static_cast<int>(inputEvent.event_tag()));
        evData.eventHeader.status = std::to_string(static_cast<int>(inputEvent.status()^1<<11)); // dummy for compression
        evData.eventHeader.bc = std::to_string(inputEvent.bc_id());
        evData.eventHeader.fragmentCount = inputEvent.fragment_count();
        evData.eventHeader.payloadSize = decompressedSize; // dummy for logging
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
        //delete &stream;
        // if (this->__isDecompressing)
        // inflateEnd(&decompressstream);
    }

void lz4Compressor::configCompression(configMap& config) {
    for (const auto& pair : config) {
    this->CompressorConfig[pair.first] = pair.second;  // I can use this to set up enforcemnets later like
    }
}

bool lz4Compressor::setupCompression()
{
    try
    {
        std::string valueOfKey1 = this->CompressorConfig.at("compressionLevel");
        this->compressionLevel = std::stoi(valueOfKey1);
    }
    catch (const std::out_of_range& e) {
        INFO("Using Default Compression level 7");
        this->compressionLevel = 7;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n'; //TODO Replace with DAQ Exceptions
        return false;
    }
    INFO("Log :: LZ4 Compressor is set up");  
    this->__isLogging=false;
    return true;
}
void lz4Compressor::supportDecompression()
{
    this->__isDecompressing=true; 
}
bool lz4Compressor::setupCompressionAndLogging(std::string Filename){
    
    std::string config = this->mapToString(this->CompressorConfig);
    this->initializeStruct(Filename,"LZ4 Compressor",config);
    bool isSetup = this->setupCompression();
    if(isSetup)
    {
        this->__isLogging=true; // Indicate that the Logging is set up
        INFO("Log:: Logging Enabled in Compressor");
    }
    return isSetup;
}
bool lz4Compressor::Compressevent( DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent_vector) {
    /*
    Steps TO DO
    - [ ] Extract the Event Header information and populate the Event Data in the Logging Struct
    - [ ] Load the Compressor Configuration appropriately
    - [ ] Do Compression and store result in a stream of bytes
    - [ ] Record metrics and populate struct accordingly
    */
   
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<uint8_t>* eventFragments = inputEvent.raw_fragments();
    //size_t compressedSize = 0;
    std::vector<uint8_t> outputevent;
    size_t maxCompressedSize = LZ4_compressBound(eventFragments->size());
    outputevent_vector.resize(maxCompressedSize);
    outputevent.resize(maxCompressedSize);

    // Compress the input vector
    int compressedSize = LZ4_compress_HC(reinterpret_cast<const char*>(eventFragments->data()),
                                         reinterpret_cast<char*>(outputevent.data()),
                                         eventFragments->size(),
                                         maxCompressedSize,
                                         this->compressionLevel);
    if (compressedSize <= 0) {
        return false;  // Compression failed
    }
    auto stop = std::chrono::high_resolution_clock::now();
    int compressedSize2 = LZ4_compress_HC(reinterpret_cast<const char*>(eventFragments->data()),
                                         reinterpret_cast<char*>(outputevent_vector.data()),
                                         eventFragments->size(),
                                         maxCompressedSize,
                                         this->compressionLevel);
    if (compressedSize2 <= 0) {
        return false;  // Compression failed
    }
    // Resize the compressed vector to the actual compressed size
    outputevent.resize(compressedSize);
    outputevent_vector.resize(compressedSize);
    inputEvent.loadCompressedData(outputevent);
    inputEvent.setCompressionAlgo(0x03); // Internal Code for lz4 compression
   
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
        evData.outputSize = std::to_string(outputevent.size());
        evData.compressionRatio = std::to_string(static_cast<double>(eventFragments->size()) / outputevent.size());
        evData.timeTaken = std::to_string(duration.count()); // Time in microseconds
        this->addEventData(evData);
    }
    
    return true;

}
bool lz4Compressor::deCompressevent(DAQFormats::EventFull& inputEvent,std::vector<uint8_t>& compressedFragments, std::vector<uint8_t>& outputFragments)
{
    
    auto start = std::chrono::high_resolution_clock::now();
    // Prepare input and output parameters
    const char* inputBuffer = reinterpret_cast<const char*>(compressedFragments.data());
    size_t inputSize = compressedFragments.size();
    size_t estimatedDecompressedSize = compressedFragments.size() * 3;
    int decompressedSize = 0;

    // Loop until the decompressed size is not larger than the estimated size
    while (decompressedSize <= estimatedDecompressedSize) {
        outputFragments.resize(estimatedDecompressedSize);

        // Decompress the input vector
        decompressedSize = LZ4_decompress_safe(inputBuffer,
                                               reinterpret_cast<char*>(outputFragments.data()),
                                               inputSize,
                                               estimatedDecompressedSize);

        if (decompressedSize >= 0) {
            // Decompression successful
            outputFragments.resize(decompressedSize);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            if (this->__isLogging && this->__isDecompressing)
            {
                EventData evData;
                evData.eventHeader.Event = std::to_string(inputEvent.event_counter());
                evData.eventHeader.run = std::to_string(inputEvent.run_number());
                evData.eventHeader.tag = std::to_string(static_cast<int>(inputEvent.event_tag()));
                evData.eventHeader.status = std::to_string(static_cast<int>(inputEvent.status()^1<<11)); // dummy for compression
                evData.eventHeader.bc = std::to_string(inputEvent.bc_id());
                evData.eventHeader.fragmentCount = inputEvent.fragment_count();
                evData.eventHeader.payloadSize = decompressedSize; // dummy for logging
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

        // Increase the estimated decompressed size and try again
        estimatedDecompressedSize *= 2;
    }

    return false;  // Decompression failed

}

lz4Compressor::~lz4Compressor()
{}

bool brotliCompressor::setupCompression()
{
    
    if (!this->state) {
    ERROR("Error: could not create Brotli compression context");
    //std::cerr << "Error: could not create zstd compression context" << std::endl;
    return false;
    }
    try
    {
        std::string valueOfKey1 = this->CompressorConfig["compressionLevel"];
        this->compressionLevel = std::stoi(valueOfKey1);
    }
    catch (const std::out_of_range& e) {
        INFO("Using Default Compression level 7");
        this->compressionLevel = 7;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n'; //TODO Replace with DAQ Exceptions
        return false;
    }
    try
    {
        BrotliEncoderSetParameter(this->state, BROTLI_PARAM_QUALITY, this->compressionLevel);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    INFO("Log :: Brotli Compressor is set up");  
    this->__isLogging=false;
    return true;
}
bool brotliCompressor::setupCompressionAndLogging(std::string Filename){
    
    std::string config = this->mapToString(this->CompressorConfig);
    this->initializeStruct(Filename,"Brotli Compressor",config);
    bool isSetup = this->setupCompression();
    if(isSetup)
    {
        this->__isLogging=true; // Indicate that the Logging is set up
        INFO("Log:: Logging Enabled in Compressor");
    }
    return isSetup;
}
void brotliCompressor::configCompression(configMap& config) {
    for (const auto& pair : config) {
    this->CompressorConfig[pair.first] = pair.second;  // I can use this to set up enforcemnets later like
    }
}
void brotliCompressor::supportDecompression()
{
    this->__isDecompressing=true; 
}
bool brotliCompressor::Compressevent( DAQFormats::EventFull& inputEvent, std::vector<uint8_t>& outputevent_vector) {
    /*
    Steps TO DO
    - [ ] Extract the Event Header information and populate the Event Data in the Logging Struct
    - [ ] Load the Compressor Configuration appropriately
    - [ ] Do Compression and store result in a stream of bytes
    - [ ] Record metrics and populate struct accordingly
    */
   
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<uint8_t>* eventFragments = inputEvent.raw_fragments();
    //size_t compressedSize = 0;
    std::vector<uint8_t> outputevent;
    this->state = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
    size_t compressedSize = BrotliEncoderMaxCompressedSize(eventFragments->size());
    outputevent.resize(compressedSize);
    outputevent_vector.resize(compressedSize);
    size_t availableIn = eventFragments->size();
    const uint8_t* nextIn = eventFragments->data();
    size_t availableOut = outputevent.size();
    uint8_t* nextOut = outputevent.data();
    while (true) {
        if (!BrotliEncoderCompressStream(state, BROTLI_OPERATION_PROCESS,
                                         &availableIn, &nextIn,
                                         &availableOut, &nextOut, nullptr)) {
            BrotliEncoderDestroyInstance(state);
            return false;
        }

        if (BrotliEncoderHasMoreOutput(state))
            compressedSize -= availableOut;

        if (availableIn == 0 && !BrotliEncoderCompressStream(state, BROTLI_OPERATION_FINISH,
                                                             &availableIn, &nextIn,
                                                             &availableOut, &nextOut, nullptr)) {
            BrotliEncoderDestroyInstance(state);
            return false;
        }

        if (!BrotliEncoderHasMoreOutput(state))
            break;
    }
    outputevent.resize(compressedSize);
    BrotliEncoderDestroyInstance(state);
    auto stop = std::chrono::high_resolution_clock::now();
    outputevent_vector.assign(outputevent.begin(),outputevent.end());
    inputEvent.loadCompressedData(outputevent);
    inputEvent.setCompressionAlgo(0x04); // Internal Code for lz4 compression
    // Populate output event vector with the same contents
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
        evData.outputSize = std::to_string(outputevent.size());
        evData.compressionRatio = std::to_string(static_cast<double>(eventFragments->size()) / outputevent.size());
        evData.timeTaken = std::to_string(duration.count()); // Time in microseconds
        this->addEventData(evData);
    }
    
    return true;

}
bool brotliCompressor::deCompressevent(DAQFormats::EventFull& inputEvent,std::vector<uint8_t>& compressedFragments, std::vector<uint8_t>& outputFragments)
{
    
    auto start = std::chrono::high_resolution_clock::now();
    if (decstate == nullptr) {
        ERROR("Can not create brotli decmpression state");
        return false;  // Failed to create Brotli decoder instance
    }
    const uint8_t* inputBuffer = compressedFragments.data();
    size_t inputSize = compressedFragments.size();
    size_t availableInput = inputSize;
    size_t totalOutput = 0;
    outputFragments.resize(inputSize * 10);  // Initial guess for the decompressed size
    while (true) {
        size_t outputBufferLength = outputFragments.size() - totalOutput;
        uint8_t* outputBuffer = outputFragments.data() + totalOutput;

        BrotliDecoderResult result = BrotliDecoderDecompressStream(decstate, &availableInput, &inputBuffer, &outputBufferLength, &outputBuffer, &totalOutput);

        if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) {
            // Not enough input data to continue decompression
            break;
        }

        if (result == BROTLI_DECODER_RESULT_SUCCESS) {
            // Decompression successful
            break;
        }

        if (result == BROTLI_DECODER_RESULT_ERROR) {
            // Error occurred during decompression
            return false;
        }

        // Resize the output vector to accommodate more data
        outputFragments.resize(outputFragments.size() * 2);
    }

    // Resize the output vector to the actual decompressed size
    outputFragments.resize(totalOutput);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    if (this->__isLogging && this->__isDecompressing)
    {
        EventData evData;
        evData.eventHeader.Event = std::to_string(inputEvent.event_counter());
        evData.eventHeader.run = std::to_string(inputEvent.run_number());
        evData.eventHeader.tag = std::to_string(static_cast<int>(inputEvent.event_tag()));
        evData.eventHeader.status = std::to_string(static_cast<int>(inputEvent.status()^1<<11)); // dummy for compression
        evData.eventHeader.bc = std::to_string(inputEvent.bc_id());
        evData.eventHeader.fragmentCount = inputEvent.fragment_count();
        evData.eventHeader.payloadSize = totalOutput; // dummy for logging
        evData.eventHeader.trig = std::to_string(inputEvent.trigger_bits());
        evData.eventHeader.time = std::to_string(inputEvent.timestamp());
        evData.inputSize = std::to_string(compressedFragments.size()); // TODO See Best Implementation
        evData.outputSize =std::to_string(totalOutput);
        evData.compressionRatio =std::to_string(static_cast<double>(totalOutput/compressedFragments.size()));
        evData.timeTaken = std::to_string(duration.count()); // Time in microseconds
        this->addEventDataDecompressed(evData);
    }


    return true;
}
brotliCompressor::~brotliCompressor()
{
    //BrotliEncoderDestroyInstance(this->state);
    BrotliDecoderDestroyInstance(this->decstate);
}


}


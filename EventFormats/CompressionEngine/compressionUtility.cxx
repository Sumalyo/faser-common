/*
FIXME :: There are quite a lot of warnings that come up here. Not strictly adhering to FASER DAQ coding standards. Please review and fix.
TODO :: Currently using std error. Try and use FASER Exception format
        Add logging capability for metrics
*/
# include "compressionlib.hpp"
#include <nlohmann/json.hpp>
CREATE_EXCEPTION_TYPE(CompressionEException,CompressionUtility)
namespace CompressionUtility{

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
    }

    // Create the zstd compression context
    ZSTD_CCtx* ctx = ZSTD_createCCtx();
    if (!ctx) {
        std::cerr << "Error: could not create zstd compression context" << std::endl;
        return false;
    }

    // Set the compression parameters
    const int compressionLevel = 3;  // 1-22
    if (ZSTD_isError(ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compressionLevel))) {
        ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compressionLevel);
        //std::cout<<ZSTD_isError(ZSTD_CCtx_setParameter(ctx, ZSTD_c_compressionLevel, compressionLevel))<<std::endl;
        std::cerr << "Error: could not set zstd compression level" << std::endl;
        return false;
    }

    // Calculate the maximum output size for compression
    const size_t maxOutputSize = ZSTD_compressBound(inputevent->size());
    outputevent.resize(maxOutputSize);

    // Compress the input data and store the compressed data in the output vector
    const size_t compressedSize = ZSTD_compressCCtx(ctx, outputevent.data(), maxOutputSize, inputevent->data(), inputevent->size(), compressionLevel);
    if (ZSTD_isError(compressedSize)) {
        std::cerr << "Error: zstd compression failed: " << ZSTD_getErrorName(compressedSize) << std::endl;
        return false;
    }

    // Resize the output vector to the actual compressed size
    outputevent.resize(compressedSize);

    // Destroy the zstd compression context
    ZSTD_freeCCtx(ctx);

    if (reporting){
    std::cout << "Input size: " << inputevent->size() << " bytes" << std::endl;
    std::cout << "Compressed size: " << compressedSize << " bytes" << std::endl;
    const double compressionRatio = static_cast<double>(inputevent->size()) / compressedSize;
    std::cout << "Compression ratio: " << compressionRatio << std::endl;}

    return true;
}

/*
Insert Support for zlibcompression
*/

class EventCompressor{
    /*
    Event Compressor Interface
    - Add support for Compressor configuration
    - Add support Logging (On Demand)
    - Add support to track the file stream being used

    - 
    */
protected:
    std::string __loggingJson;
    bool __loggingMode;
  public:
        //virtual bool Compressevent(const std::vector<uint8_t>* inputevent, std::vector<uint8_t>& outputevent) = 0;
        virtual bool Compressevent(bool logmode = false) =0;
        virtual void ResultMetrics() =0;
        EventCompressor()
        {
            __loggingJson= "Logs/default.json";
            __loggingMode=false;
        }


};

class ZstdCompressor : EventCompressor
{
    public:
    bool Compressevent  ( bool logmode = false) override{
            this->__loggingMode = logmode;
            return false;
        }
        void ResultMetrics() override{
            
        }
};

class ZlibCompressor : EventCompressor
{
    public:
    bool Compressevent  ( bool logmode = false ) override{
            this->__loggingMode = logmode;
            return false;
        }
        void ResultMetrics() override{
            
        }
};





}

/*
APIs for Compression Events or Fragments or other Faser Data Formats to be added here
*/

/*
  Copyright (C) 2023 CERN for the benefit of the FASER collaboration
*/

///////////////////////////////////////////////////////////////////
// eventCompress.cxx, (c) FASER Detector software
///////////////////////////////////////////////////////////////////
#include "EventFormats/DAQFormats.hpp"
#include <getopt.h>
#include "EventFormats/TLBDataFragment.hpp"
#include "EventFormats/TLBMonitoringFragment.hpp"
#include "EventFormats/DigitizerDataFragment.hpp"
#include "EventFormats/TrackerDataFragment.hpp"
#include "EventFormats/BOBRDataFragment.hpp"
#include "CompressionEngine/compressionlib.hpp"

using namespace DAQFormats;
using namespace TLBDataFormat;
using namespace TLBMonFormat;
using namespace BOBRDataFormat;
using namespace TrackerData;
static void usage() {
  std::cout<<"Usage: eventCompress [-n nEventsMax -l -d -s -o <output_file> -c <config_file> ] <input_raw_filename> \n"
            "   -n <no. events>:   Run Compression for n events only (optional)\n"
            "   -l --enableLog:    Enable logging for metrics and benchmarks (optional) \n"
            "   -d --decomress:    Run Decompression tests (optional)\n"
            "   -s --silent:       suupress logs (optional)\n"
            "   -o --write:        specify file to write out to\n"
            "   -c --config:       specify file to read compressor config\n";
   exit(1);
}

int main(int argc, char **argv) {

  // argument parsing
  if(argc<2) usage();

  bool enableLogging = false;
  bool runDecompressionTests = false;
  bool suppressLogs = false;
  int nEventsMax = -1;
  std::string outputFilename = "default";
  std::string configFile = "../EventFormats/CompressionEngine/compressionConfig.json";
  
  int opt;
  static struct option long_options[] = {
        {"enableLog", no_argument, nullptr, 'l'},
        {"decompress", no_argument, nullptr, 'd'},
        {"silent", no_argument, nullptr, 's'},
        {"write", required_argument, nullptr, 'o'},
        {"config", required_argument, nullptr, 'c'},
        {nullptr, 0, nullptr, 0} // Required at the end of the array
    };

  while (true) {
    opt = getopt_long(argc, argv, "n:ldso:c:", long_options, nullptr);
    if (opt == -1) break;
    switch ( opt ) {
            case 'n':
                nEventsMax = std::stoi(optarg);
                break;
            case 'l':
            std::cout<<"Logging is enabled\n";
                enableLogging = true;
                break;
            case 'd':
            std::cout<<"Running decompression tests\n";
                runDecompressionTests = true;
                break;
            case 's':
            std::cout<<"Silent mode enabled\n";
                suppressLogs = true;
                break;
            case 'o':
                outputFilename = optarg;
                break;
            case 'c':
                configFile = optarg;
                break;
            case ':':
                std::cout<<"Missing optopt : "<<optopt<<std::endl;
                break;
            case '?':  // unknown option...
                usage();
                break;
            default:
                usage();
    }
  }

  if (optind >= argc) {
    std::cerr<<"ERROR: too few arguments given."<<std::endl;
    std::cerr <<"ERROR: input_raw_filename is required"<<std::endl;
    usage();
  }


  std::string filename(argv[optind]);
  
  std::string outfilename;
  if (outputFilename == "default")
  {
  outfilename = filename;
  size_t dotPos = outfilename.find_last_of(".");
  if (dotPos != std::string::npos) {
      std::string nameWithoutExtension = outfilename.substr(0, dotPos);
      outfilename = nameWithoutExtension + "_compressed.raw";
  }
  }
  else
  {
    outfilename = outputFilename;
  }
  std::cout<<"Writing data to file : "<<outfilename<<std::endl;
  std::ifstream in(filename, std::ios::binary);
  if (!in.is_open()){
    std::cerr << "ERROR: can't open input file "<<filename<<std::endl;
    return 1;
  }
  std::ios_base::openmode mode;
  mode = std::ios::trunc;
  std::ofstream out(outfilename, std::ios::out | std::ios::binary | mode);
  if (!out.is_open()){
    std::cerr << "ERROR: can't open output file "<<outfilename<<std::endl;
    return 1;
  }
  // The file input
  int nEventsRead=0;
  CompressionUtility::configMap CompConfig = CompressionUtility::readJsonToMap(configFile);
  CompressionUtility::EventCompressor* usedCompressor;
  std::string compressor = CompConfig["Compressor"];
  if(compressor=="ZSTD")
  {
  usedCompressor = new CompressionUtility::ZstdCompressor();
  }else if(compressor=="Zlib")
  {
    usedCompressor = new CompressionUtility::ZlibCompressor();
  }else if(compressor=="LZ4")
  {
    usedCompressor = new CompressionUtility::lz4Compressor();
  }else if(compressor=="Brotli")
  {
    usedCompressor = new CompressionUtility::brotliCompressor();
  }
  usedCompressor->configCompression(CompConfig);
  if (enableLogging)
  {
    usedCompressor->setupCompressionAndLogging(filename); 
  }
  else{
     usedCompressor->setupCompression();
  }
  if(runDecompressionTests)
  {
    usedCompressor->supportDecompression();
  }
  while(in.good() and in.peek()!=EOF) {
    try {
      EventFull event(in);
      /*
      * The target workflow would be:
      - Extract the event fragments from the Event aand compress it using the compression utility
      - Update the Compression Flag in Event Status
        TODO:: Replace the event fragments with the new compressed fragment (and update the payload size at header appropriately) 
        TODO:: Write the compressed event out to a new file
      - Record all relevant metrices Investigation and analysis
      */
      std::vector<uint8_t> compressedData;
    if (usedCompressor->Compressevent(event,compressedData))
       {
        if(!suppressLogs)
        std::cout << "Compression successful" << std::endl;
        //event.updateStatus(1<<11);
        } 
        else 
        {
        std::cerr << "Compression failed" << std::endl;
        }
      std::vector<uint8_t> decompressedData;
      byteVector* raw = event.raw();
      out.write(reinterpret_cast<char*>(&(*raw)[0]), event.size());
      if(usedCompressor->__isDecompressing){
        //if (usedCompressor->deCompressevent(event,compressedData,decompressedData))
        if (usedCompressor->deCompressevent(event,event.compressedData,decompressedData))
        {
        if(!suppressLogs)
        std::cout << "Decompression successful" << std::endl;
        } 
        else 
        {
        std::cerr << "Decompression failed" << std::endl;
        compressedData.clear();
        }
      }
      else
      {
        compressedData.clear();
      }
    if(!suppressLogs)
    std::cout<<event<<std::endl;

    decompressedData.clear();
    raw->clear();
    } catch (EFormatException &e) {
      std::cout<<"Problem while reading file - "<<e.what()<<std::endl;
      return 1;
    }
    catch(...)
    {
      std::cout<<"An exception occurred"<<std::endl;
    }
    
    
    // read up to nEventsMax if specified
    nEventsRead++;
    if(nEventsMax!=-1 && nEventsRead>=nEventsMax){
      std::cout<<"Finished reading specified number of events : "<<nEventsMax<<std::endl;
      break;
    }

  }
  if(enableLogging)
  {
    if(usedCompressor->__isLogging)
    {
      usedCompressor->writeToJson("logTest"+CompressionUtility::generateUniqueIdentifier());
    }
    else
    {
      INFO("Logging was not setup for compressor hence no JSON log created");
    }
  }
  delete usedCompressor;
}
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

using namespace DAQFormats;
using namespace TLBDataFormat;
using namespace TLBMonFormat;
using namespace BOBRDataFormat;
using namespace TrackerData;
static void usage() {
   std::cout<<"EXPERIMENTAL FEATURE: Use to genrate event files to train dictionary compressor\n"
            "Usage: eventDump [--startIndex -i <sequence_number> -o --outputdir <directory_for_files> ] [-n nEventsMax] --debug <filename>\n"
              "   -n <no. events>:   Run Compression for n events only (optional)\n"
              "   -o --outputdir:    specify directory path to write out event files\n"
              "   -i --startIndex:   Use if event files are in directory to overload index\n"; 
   exit(1);
}

int main(int argc, char **argv) {

  // argument parsing
  if(argc<3) usage();
  int nEventsMax = -1;
  int startIndex = 0;
  std::string outputDirname = "../EventFormats/CompressionEngine/trainDictionary/";
  int opt;
  static struct option long_options[] = {
    {"outputdir", required_argument, nullptr, 'o'},
    {"startIndex", required_argument, nullptr, 'i'},
    {nullptr, no_argument, nullptr, 0}
  };
  

  while (true) {
    opt = getopt_long(argc, argv, "n:o:i:", long_options, nullptr);
    if (opt == -1) break;
    switch ( opt ) {
    case 'o':
      std::cout<<"Specifying output Directory : "<<optarg<<std::endl;
      outputDirname = optarg;
      break;
    case 'n':
      std::cout<<"Specifying Nvents : "<<optarg<<std::endl;
      nEventsMax = std::atoi(optarg);
      break;
    case 'i':
      std::cout<<"Specifying Nvents : "<<optarg<<std::endl;
      startIndex = std::atoi(optarg);
      break;
    case ':':
      std::cout<<"Missing optopt : "<<optopt<<std::endl;
      break;
    case '?':  // unknown option...
      usage();
      break;
    }
  }

  
  if (optind >= argc) {
    std::cout<<"ERROR: too few arguments given."<<std::endl;
    usage();
  }
  std::string filename(argv[optind]);
  std::ifstream in(filename, std::ios::binary);
  if (!in.is_open()){
    std::cout << "ERROR: can't open file "<<filename<<std::endl;
    return 1;
  }
  
  int nEventsRead=0;
  
  while(in.good() and in.peek()!=EOF) {
    try {
      EventFull event(in);
      std::string outputFile = outputDirname+"event" + std::to_string(nEventsRead+1+startIndex) + ".raw";
        std::ofstream outFile(outputFile);
        if (!outFile.is_open()) {
            std::cout << "Failed to create output file: " << outputFile << std::endl;
        }
      else
      {
      byteVector* raw = event.raw();
      outFile.write(reinterpret_cast<char*>(&(*raw)[0]), event.size());
      outFile.close();
      std::cout << "Created file: " << outputFile << std::endl;
      }
    // read up to nEventsMax if specified
    }catch (EFormatException &e) {
      std::cout<<"Problem while reading file - "<<e.what()<<std::endl;
      return 1;
    }
    catch(...)
    {
      std::cout<<"An exception occurred"<<std::endl;
    }
    nEventsRead++;
    if(nEventsMax!=-1 && nEventsRead>=nEventsMax){
      std::cout<<"Finished reading specified number of events : "<<nEventsMax<<std::endl;
      break;
    }   
}
}
// After writing the event 

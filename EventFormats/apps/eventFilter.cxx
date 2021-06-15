#include "EventFormats/DAQFormats.hpp"
#include <getopt.h>
#include "EventFormats/TLBDataFragment.hpp"
#include "EventFormats/TLBMonitoringFragment.hpp"
#include "EventFormats/DigitizerDataFragment.hpp"
#include "EventFormats/TrackerDataFragment.hpp"
#include <list>
#include <algorithm>

using namespace DAQFormats;
using namespace TLBDataFormat;
using namespace TLBMonFormat;

static void usage() {
   std::cout<<"Usage: eventFilter [-n nEvents] [-e evnum] <infile> <outfile>\n"
              "   -a                  append (rather than overwrite) output file\n"
              "   -n <no. events>:    write only first n events\n"
              "   -e <events>:        write specific event numbers, can specify\n"
              "                       comma-separated list of events, or ranges\n"
              "   -t <mask>:          only write events satisfying (mask | trigger)\n"
              "                       specify mask in hex format: 0xFF, \n"
     ;
   exit(1);
}

int main(int argc, char **argv) {

  // argument parsing
  if(argc<3) usage();

  int nEventsMax = -1;
  std::list<int> event_list;

  int opt;
  char* token;
  bool append = false;
  unsigned short mask = 0;

  while (true) {
    opt = getopt(argc, argv, "ad:n:e:t:");
    if (opt == -1) break;
    switch ( opt ) {

    case 'a':
      append = true;
      break;

    case 'n':
      //std::cout<<"Specifying Nvents : "<<optarg<<std::endl;
      nEventsMax = std::atoi(optarg);
      break;

    case 'e':
      if(optarg==NULL){
        std::cout<<"No event list specified!"<<std::endl;
	break;
      } 

      std::cout<<"Specifying Event Numbers : " << optarg << std::endl;
      // Allow specification like 1,3,5-7
      // First split string by commas

      token = strtok(optarg, ",");

      while (token != NULL) {
	std::string tstr = std::string(token);

	// Is there a hyphen?
	std::size_t loc = tstr.find('-');
	if (loc == std::string::npos) {
	  // No hyphen, save run number
	  event_list.push_back(std::stoi(tstr));
	} else {
	  // Yes hyphen, extract two run numbers and fill range to list
	  int start = std::stoi(tstr.substr(0,loc));
	  int end = std::stoi(tstr.substr(loc+1,tstr.size()-loc));
	  std::cout<<"Found range from " << start << " to " << end << std::endl;
	  for (int run=start; run <= end; run++)
	    event_list.push_back(run);
	}

	token = strtok(NULL, ",");
      }
      break;

    case 't':
      sscanf(optarg, "%hx", &mask);
      break;

    case ':':
      std::cout<<"Missing optarg : "<<optopt<<std::endl;
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
  std::string infilename(argv[optind]);

  if (++optind >= argc) {
    std::cout<<"ERROR: too few arguments given."<<std::endl;
    usage();
  }
  std::string outfilename(argv[optind]);

  // Explain what we are doing
  if (nEventsMax > 0)
    std::cout<<"Writing Max Events    : "<<nEventsMax<<std::endl;
  if (event_list.size() > 0) {
    std::cout<<"Writing Events        : ";
    for (auto const &i: event_list) {
      std::cout << i << " ";
    }
    std::cout<<std::endl;
  }
  if (mask > 0)
    std::cout<<"Using trigger mask    : " << std::hex << mask << std::dec << std::endl;

  std::cout<<"Reading from file     : "<<infilename<<std::endl;
  if (append) {
    std::cout<<"Appending to file     : "<<outfilename<<std::endl;
  } else {
    std::cout<<"Writing to file       : "<<outfilename<<std::endl;
  }

  // Open input and output files
  std::ifstream in(infilename, std::ios::binary);
  if (!in.is_open()){
    std::cout << "ERROR: can't open file "<<infilename<<std::endl;
    return 1;
  }

  std::ios_base::openmode mode;
  if (append) {
    mode = std::ios::app;
  } else {
    mode = std::ios::trunc;
  }
  std::ofstream out(outfilename, std::ios::out | std::ios::binary | mode);
  if (!out.is_open()){
    std::cout << "ERROR: can't open file "<<outfilename<<std::endl;
    return 1;
  }
  
  int nEventsWritten=0;
  
  while(in.good() and in.peek()!=EOF) {
    try {
      EventFull event(in);

      // Skip events not in our event list
      if (!event_list.empty() && (std::find(event_list.begin(), event_list.end(), event.event_counter()) == event_list.end()))
	  continue;

      // Skip events that don't pass trigger
      if ((mask>0) && (event.trigger_bits() & mask) == 0) continue;

      // Have an event to write out
      std::cout<<event<<std::endl;

      byteVector* raw = event.raw();
      out.write(reinterpret_cast<char*>(&(*raw)[0]), event.size());

      //std::cout << "Wrote Run: " << event.run_number()
      //		<< " Event: " << event.event_counter() << std::endl;
    } catch (EFormatException &e) {
      std::cout<<"Problem while reading file - "<<e.what()<<std::endl;
      return 1;
    }
    
    // write up to nEventsMax if specified
    nEventsWritten++;
    if(nEventsMax>0 && nEventsWritten>=nEventsMax){
      std::cout<<"Finished reading specified number of events : "<<nEventsMax<<std::endl;
      break;
    }
    
  }
}



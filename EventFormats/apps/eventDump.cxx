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
   std::cout<<"Usage: eventDump [-f] [-d TLB/TRB/Digitizer/BOBR/all] [-n nEventsMax] --debug <filename>\n"
              "   -f:                 print fragment header information\n"
              "   -d <subdetector>:   print full event information for subdetector\n"
              "   -n <no. events>:    print only first n events\n"
              "   --debug:            set TLB and tracker decoders to debug mode\n"; 
   exit(1);
}

int main(int argc, char **argv) {

  // argument parsing
  if(argc<2) usage();

  bool showFragments=false;
  bool showData=false;
  bool showTLB=false;
  bool showTRB=false;
  bool showDigitizer=false;
  bool showBOBR=false;
  int nEventsMax = -1;
  static int debug_mode;
  int opt;
  static struct option long_options[] = {
    {"debug", no_argument, &debug_mode, 1},
    {nullptr, no_argument, nullptr, 0}
  };

  while (true) {
    opt = getopt_long(argc, argv, "fd:n:", long_options, nullptr);
    if (opt == -1) break;
    switch ( opt ) {
    case 'f':
      std::cout<<"DumpingFragments ... "<<std::endl;
      showFragments = true;
      break;
    case 'd':
      std::cout<<"DumpingData ..."<<std::endl;
      if(optarg==NULL)
        std::cout<<"DumpingData : NULL"<<std::endl;
      else
        std::cout<<"DumpingData : "<<optarg<<std::endl;
      showFragments = true;
      showData = true;
      if(optarg==NULL){
        std::cout<<"No systems specified - printing all"<<std::endl;
        showBOBR=true;
        showTLB=true;
        showTRB=true;
        showDigitizer=true;
      }
      else{
        std::string systems = optarg;
        std::cout<<"DumpingData select systems : "<<systems<<std::endl;
        if(systems.find("TLB")==0){
          showTLB=true;
        }
        else if(systems.find("TRB")==0){
          showTRB=true;
        }
        else if(systems.find("BOBR")==0){
          showBOBR=true;
        }
        else if(systems.find("Digitizer")==0){
          showDigitizer=true;
        }
        else if(systems.find("all")==0){
	  showBOBR=true;
          showTLB=true;
          showTRB=true;
          showDigitizer=true;
        }
        else {
          std::cout<<"ERROR: Argument for -d is invalid "<<std::endl;
          usage();
        }
      }
      std::cout<<"DumpingData TLB       : "<<showTLB<<std::endl;
      std::cout<<"DumpingData TRB       : "<<showTRB<<std::endl;
      std::cout<<"DumpingData Digitizer : "<<showDigitizer<<std::endl;
      std::cout<<"DumpingData BOBR      : "<<showBOBR<<std::endl;
      break;
    case 'n':
      std::cout<<"Specifying Nvents : "<<optarg<<std::endl;
      nEventsMax = std::atoi(optarg);
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
      std::cout<<event<<std::endl;
      if (showFragments) {
      for(const auto &id :event.getFragmentIDs()) {
        const EventFragment* frag=event.find_fragment(id);
        std::cout<<*frag<<std::endl;
        if (showData) {
          switch (frag->source_id()&0xFFFF0000) {
          case TriggerSourceID:
            if(showData && showTLB){
              if (event.event_tag() == PhysicsTag ) {
                TLBDataFragment tlb_data_frag = TLBDataFragment(frag->payload<const uint32_t*>(), frag->payload_size());
                if (debug_mode) tlb_data_frag.set_debug_on();
                std::cout<<"TLB data fragment:"<<std::endl;
                std::cout<<tlb_data_frag<<std::endl;
              }
              else if (event.event_tag() == TLBMonitoringTag ) {
                TLBMonitoringFragment tlb_mon_frag = TLBMonitoringFragment(frag->payload<const uint32_t*>(), frag->payload_size());
                if (debug_mode) tlb_mon_frag.set_debug_on();
                std::cout<<"TLB monitoring fragment:"<<std::endl;
                std::cout<<tlb_mon_frag<<std::endl;
              }
            }
            break;
          case TrackerSourceID:
            if(showData && showTRB){
              try {
                TrackerDataFragment tracker_data_frag(frag->payload<const uint32_t*>(), frag->payload_size());
                if (debug_mode) tracker_data_frag.set_debug_on();
                std::cout<<"Tracker data fragment:"<<std::endl;
                std::cout<<tracker_data_frag<<std::endl;
                if (!tracker_data_frag.valid()){
                  std::cout<<" WARNING corrupted tracker fragment!"<<std::dec<<std::endl;
                  if (tracker_data_frag.has_trb_error()) std::cout<<"TRB error found with id = "<<static_cast<int>(tracker_data_frag.trb_error_id())<<std::endl;
                  if (tracker_data_frag.has_module_error()) {
                    for (auto module_error : tracker_data_frag.module_error_id()) std::cout<<"Module error found with id = "<<static_cast<int>(module_error)<<std::endl;
                  }
                  if (tracker_data_frag.has_crc_error()) std::cout<<"CRC msimatch!"<<std::endl;
                }
              }
              catch (TrackerDataException &e ){
                std::cout<<"WARNING Crashed TrackerDataFragment! Skipping... "<<std::endl; 
                std::cout<<e.what()<<std::endl;
              }
            }
            break;
          case PMTSourceID:
            if(showData && showDigitizer){
              if (event.event_tag() == PhysicsTag ) {
                DigitizerDataFragment digitizer_data_frag = DigitizerDataFragment(frag->payload<const uint32_t*>(), frag->payload_size());
                std::cout<<"Digitizer data fragment:"<<std::endl;
                std::cout<<digitizer_data_frag<<std::endl;
              }
            }
            break;
          case BOBRSourceID:
            if(showData && showBOBR){
	      BOBRDataFragment bobr_data_frag = BOBRDataFragment(frag->payload<const uint32_t*>(), frag->payload_size());
	      std::cout<<"BOBR data fragment:"<<std::endl;
	      std::cout<<bobr_data_frag<<std::endl;
            }
            break;
          default:
            const uint32_t* payload=frag->payload<const uint32_t *>();
            unsigned int ii=0;
            for(;ii<frag->payload_size()/4;ii++) {
          if (ii%8==0) std::cout<<" ";
          std::cout<<" 0x"<<std::setw(8)<<std::hex<<std::setfill('0')<<payload[ii];
          if (ii%8==7) std::cout<<std::endl;
              }
              if (ii%8!=0) std::cout<<std::endl;
              std::cout<<std::dec<<std::setfill(' ');
              break;
            }
          }
        }	
      }
    } catch (EFormatException &e) {
      std::cout<<"Problem while reading file - "<<e.what()<<std::endl;
      return 1;
    }
    
    // read up to nEventsMax if specified
    nEventsRead++;
    if(nEventsMax!=-1 && nEventsRead>=nEventsMax){
      std::cout<<"Finished reading specified number of events : "<<nEventsMax<<std::endl;
      break;
    }
    
  }
}

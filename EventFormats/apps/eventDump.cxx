#include "EventFormats/DAQFormats.hpp"
#include <unistd.h>

using namespace DAQFormats;

void usage() {
   std::cout<<"Usage: eventDump [-f] [-d] <filename>"<<std::endl;
   exit(1);
}

int main(int argc, char **argv) {
  if(argc<2) usage();
  bool showFragments=false;
  bool showData=false;
  int opt;
  while ( (opt = getopt(argc, argv, "fd")) != -1 ) {  
    switch ( opt ) {
    case 'f':
      showFragments = true;
      break;
    case 'd':
      showFragments = true;
      showData = true;
      break;
    case '?':  // unknown option...
      usage();
      break;
    }
  }
  
  std::string filename(argv[optind]);
  std::ifstream in(filename, std::ios::binary);
  if (!in.is_open()){
    std::cout << "ERROR: can't open file "<<filename<<std::endl;
    return 1;
  }
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
	    case TriggerSourceID: //FIXME put in specific 
	    case TrackerSourceID:
	    case PMTSourceID:
	    default:
	      const uint32_t* payload=frag->payload<const uint32_t *>();
	      int ii=0;
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
  }
}

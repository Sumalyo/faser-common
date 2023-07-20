# faser-common Description

This houses software common to detector, online and offline faser code.  It includes
some utilities/functions as well as the event decoders that convert binary data
to useable data stored in memory for offline reconstruction.  This can be included
in other code bases as well as built and run standalone.

# Code Bases
There are different bits of code here that serve separate purposes.  Note that this
documentation is also used by doxygen and when viewing that page, this provides the 
links to those places.

## EventFormats
DAQFormats ([Link To Source](EventFormats/EventFormats/DAQFormats.hpp)): 
This is the base of all raw data formats.

DigitizerDataFragment ([Link To Source](EventFormats/EventFormats/DigitizerDataFragment.hpp)): 
This is the digitizer specific data format and event decoder.

TLBDataFragment ([Link To Source](EventFormats/EventFormats/TLBDataFragment.hpp)): 
This is the trigger logic board specific data format and event decoder for *data* fragments.

TLBMonitoringFragment ([Link To Source](EventFormats/EventFormats/TLBMonitoringFragment.hpp)): 
This is the trigger logic board specific data format and event decoder for *monitoring* fragments.

## Exceptions
Exceptions ([Link To Source](Exceptions/Exceptions/Exceptions.hpp)): 
This houses the functionality for exceptions handling.

## Logging
Logging ([Link To Source](Logging/include/Logging.hpp)): 
This houses a set of utilities that allow one to mimic DAQ-ling logging in their
hardware specific code to avoid writing std::cout statements in a controlled way.





# Standalone Build Instructions
The code can be built standalone on machines that have the right software installed.
One example of this is the __faser-daq-001.cern.ch__ machine housed in the CaloScint
lab at CERN.  If you need credentials for this machine, contact [Brian Petersen](mailto:Brian.Petersen@cern.ch).  

The other options is to use [docker](https://www.docker.com/) ([Tutorial Here](https://matthewfeickert.github.io/intro-to-docker/))
for which we have an image configured to mimic the environment necessary for uses such as this.  The image
__gitlab-registry.cern.ch/faser/docker/daq__ is built and housed at [faser/docker](https://gitlab.cern.ch/faser/docker)
and used for the CI tests as well.  If you boot into that image, then it will work as well.

Once you have configured one of these methods, you can build the code as :

```
source setup.sh
mkdir -p build
cd build
cmake3 ..
make
```

# Standalone Running/Testing

## Event Decoders
Once you build the code, there is a primary executable [eventDump.cxx](EventFormats/apps/eventDump.cxx)
which is compiled into the executable in your build directory at `build/EventFormats/eventDump` which 
can be run on test binary data to develop or understand the functionality of the event/fragment decoders.

Test data were recorded on April 16 which can be used to work with the standalone event decoders. These test runs
were done with the TLB + Digitizer and were recorded with the scintillator lab lockdown 
setup using the branch - [TLB-Digi-testing](https://gitlab.cern.ch/faser/daq/-/tree/TLB-Digi-testing).
The data can be found on the faserdaq service account EOS space at `/eos/user/f/faserdaq/TestData/2020_April16`
with readme.txt contained within.  It should be noted that the data has small (Trigger receiver) 
bug in it and there are corrupted events in physics stream.  Two configurations exist:
 - [Run 1]
   - 200 Hz input signal into channel 0 & 2 of the Digitizer
   - Coincidence signal, prescale 1 (~100 Hz)
   - single signal input 1, prescale 2 (~50 Hz)
   - ~25 Hz random trigger
   - 1 Hz monitoring data from the TLB
   - Only channel 1 is enabled for data readout from the Digitizer
 - [Run 2]
   - 200 Hz input signal into channel 0 & 2 of the Digitizer
   - Coincidence signal, prescale 1 (~100 Hz)
   - single signal input 1, prescale 3 (~30 Hz)
   - single signal input 2, prescale 10 (~10 Hz)
   - ~25 Hz random trigger, prescale 3 (~10 Hz)
   - 1 Hz monitoring data from the TLB
   - Only channel 1 is enabled for data readout from the Digitizer
   
 ## Event Filtering
A second executable [eventFilter.cxx](EventFormats/apps/eventFilter.cxx) is also compiled in the build directory at `build/EventFormats/eventFilter`.  This application reads in a raw data file and can write out a subset of the events to a new raw data file.  Currently, this application can filter on event number, trigger type, or just some total number of events.  The options can be seen with `eventFilter -h`.

# Data Compression

## Event Compression
A executable [eventCompress.cxx](EventFormats/apps/eventCompress.cxx) is compiled at the build directory at `build/EventFormats/eventCompress`.
This application adds support for data compression (using various compressors) of raw files produced during the run.  It reads in a raw data file and can write out compressed events to a file as directed.The options can be seen with `eventCompress -h`. The compressors needs to be configured with the JSON file at [compressionConfig.json](EventFormats/CompressionEngine/compressionConfig.json). This application can log performance benchmarks for analysis <br>
**Supported Compressors**<br>
Currently support using [ZSTD](https://github.com/facebook/zstd), [Zlib Deflate](https://github.com/madler/zlib) and [LZ4](https://github.com/lz4/lz4) are provided. 
<br>
**Compressor Config**
```JSON
{
    "Compressor":"ZSTD", // Supports ZSTD, LZ4 and Zlib Compression
    "compressionLevel":"5" // A Higer compression level means smaller file sizes but at the cost of Computing Resources and time
}
```
**To Run**
```bash
cd build
EventFormats/eventCompress -s -o compressed.raw input_raw_file.raw
```
### Experimental Dictionary Compression
Currently experimnetal support for dictionary compression is provided which can be used with ZSTD and LZ4 compressors. To enable it add `"useDictionary":"1"` to the configuration JSON. To create a dictionary   
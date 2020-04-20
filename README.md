# faser-common

Software common to detector, online and offline faser code


# Build Instructions
```
source setup.sh
mkdir -p build
cd build
cmake3 ..
make
```

# Event Decoders

## Executables
Once you build the code, there is a primary executable [eventDump.cxx](EventFormats/app/eventDump.cxx)
which is compiled into the executable in your build directory at `build/EventFormats/eventDump` which 
can be run on test binary data to develop or understand the functionality of the event/fragment decoders.


## Test Input
The following are test data recorded which can be used to work with the standalone event decoders.

### April 16
TLB + Digitizer data were recorded with the scintillator lab lockdown 
setup using the branch - [TLB-Digi-testing](https://gitlab.cern.ch/faser/daq/-/tree/TLB-Digi-testing).
The data can be found on the faserdaq service account EOS space at `/eos/user/f/faserdaq/TestData/2020_April16`
with readme.txt contained within.  It should be noted that the data has small (Trigger receiver) 
bug in it and there are corrupted events in physics stream.  Two configurations exist:
 - 1)
   - 200 Hz input signal into channel 0 & 2
   - Coincidence signal, prescale 1 (~100 Hz)
   - single signal input 1, prescale 2 (~50 Hz)
   - ~25 Hz random trigger
   - 1 Hz monitoring data
 - 2)
   - 200 Hz input signal into channel 0 & 2
   - Coincidence signal, prescale 1 (~100 Hz)
   - single signal input 1, prescale 3 (~30 Hz)
   - single signal input 2, prescale 10 (~10 Hz)
   - ~25 Hz random trigger, prescale 3 (~10 Hz)
   - 1 Hz monitoring data

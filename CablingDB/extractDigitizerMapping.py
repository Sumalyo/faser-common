#!/usr/bin/env python3

import json
import copy
import pprint
import sys
import requests
import argparse

# Parse any command-line options
parser = argparse.ArgumentParser(description="Digitizer Mapping Checker")

parser.add_argument("-r", "--firstRun", default=0,
                    help="Specify first run to consider (default: all)")

args = parser.parse_args()
try:
    first_run = int(args.firstRun)
except Exception as e:
    print("Error parsing first run:")
    print(e)
    sys.exit(-1)

# This only works on lxplus
response = requests.get("http://faser-runnumber.web.cern.ch/RunList")
runs = response.json()
runs.reverse()

digitizer_map={}

# Calo: (row, mod, pmt)
# Veto/Preshower/Trigger: (station, plate, pmt)

# Orignal TI12 detector, without FASERnu veto
TI12_map = [
    ('calo', 0, 1, 0),  # Digitizer 0
    ('calo', 0, 0, 0),
    ('calo', 1, 1, 0),
    ('calo', 1, 0, 0),
    ('veto', 0, 0, 0),  # Digitizer 4
    ('veto', 0, 1, 0),
    ('veto', 1, 0, 0),
    ('veto', 1, 1, 0),
    ('trigger', 0, 0, 1),  # Digitizer 8
    ('trigger', 0, 0, 0), 
    ('trigger', 0, 1, 1), 
    ('trigger', 0, 1, 0), 
    ('preshower', 0, 0, 0), # Digitizer 12
    ('preshower', 0, 1, 0),
    ('none', 0),  # Add dummy number or else tuple is flattened
    ('clock', 0) # Digitizer 15
] 

# TI12 map as of run 6520, when the FASERnu veto is installed
# Installed 3/15/2022, first non-test run 6525
# Also, rename trigger to timing
TI12_6520_map = [
    ('calo', 0, 1, 0),  # Digitizer 0
    ('calo', 0, 0, 0),
    ('calo', 1, 1, 0),
    ('calo', 1, 0, 0),
    ('vetonu', 0, 0, 0),  # Digitizer 4
    ('vetonu', 0, 1, 0),
    ('veto', 1, 0, 0),     # Digitizer 6
    ('veto', 1, 1, 0),
    ('trigger', 0, 0, 1),  # Digitizer 8
    ('trigger', 0, 0, 0), 
    ('trigger', 0, 1, 1), 
    ('trigger', 0, 1, 0), 
    ('preshower', 0, 0, 0), # Digitizer 12
    ('preshower', 0, 1, 0),
    ('veto', 0, 1, 0), # Digitizer 14
    ('clock', 0) # Digitizer 15
] 

TB_map = [
    ('calo', 1, 2, 0),  # Digitizer 0
    ('calo', 0, 2, 0),
    ('calo', 1, 1, 0),
    ('calo', 0, 1, 0),
    ('calo', 1, 0, 0),
    ('calo', 0, 0, 0),
    ('preshower', 0, 0, 0),  # Digitizer 6
    ('preshower', 0, 1, 0),
    ('trigger', 0, 0, 0), # Digitizer 8
    ('trigger', 0, 1, 0), 
    ('none', 0),
    ('none', 0),
    ('none', 0),
    ('none', 0),
    ('none', 0),
    ('clock', 0) # Digitizer 15
]

for info in runs:

    runno=info["runnumber"]
    if runno < first_run: continue

    # Skip runtypes we don't want to reconstruct
    runtype = info['type']
    if "Calibration" in runtype: continue
    if runtype == "Test": continue

    detectors=info["detectors"]
    digs=[det for det in detectors if det.startswith("DIG")]
    if not digs: continue
    trbs=[det for det in detectors if det.startswith("TRB")]
    if not trbs: continue
    trbIDs=[int(trb[3:]) for trb in trbs]
    trbIDs.sort()
    host=info["host"].split(".")[0]

    # Check if host makes sense
    if not host in ["faser-daq-002","faser-daq-006","faser-daq-009","faser-daq-010"]:
        #if runno in [423,441,444,448,2802]: continue # known test runs to be ignored
        print(f"ERROR: run {runno} is running on unusual host {host} - not on list of runs to ignore")
        sys.exit(1)

    # Find the readout channels
    query = f"https://faser-runinfo.app.cern.ch/cgibin/getRunInfo.py?runno={runno}"
    response = requests.get(query)
    config = response.json().get('configuration', None)

    if not config:
        print(f"ERROR: run {runno} has no configuration!")
        sys.exit(1)

    comps = config['components']
    digi_channels = []
    for comp in comps:
        if comp['name'] != "digitizer01": continue

        if not comp.get('modules', None):

            if not comp.get('settings', None):
                print(f"ERROR run {runno} has no modules or settings!")
                sys.exit(1)

            else:
                dig = comp['settings']
                readout = dig['channel_readout']

        else:
            dig = comp['modules'][0]
            if dig['name'] != "digitizer01": continue
            settings = dig['settings']
            readout = settings['channel_readout']

        for chan in readout:
            chid = int(chan['channel'])
            if chan['enable']:
                digi_channels.append(chid)

        break

    if host=="faser-daq-002": 
        #EHN1 data or test beam data
        if len(trbIDs)!=3:
            #if runno in [1002,1804,1806]: continue
            #if runno in range(1219,1238): continue
            print(f"ERROR: EHN1 run {runno} with unusual number of TRBs: {trbIDs}")
            sys.exit(1)

        if len(digi_channels) == 11: # Normal testbeam data
            digitizer_map[runno] = copy.copy(TB_map)

        else:
            # Other EHN1 test data, just call everything 'trigger' for now
            digitizer_map[runno] = [('none', 0)]*16

            layer_count = 0
            for chan in digi_channels:
                if chan == 15:
                    digitizer_map[runno][chan] = ('clock', 0)
                else:
                    digitizer_map[runno][chan] = ('trigger', 0, layer_count, 0)
                    layer_count += 1

            print(f"{runno} ({host}): {digi_channels}")

 
    else:
        #TI12 data

        if len(trbIDs)!=6 and len(trbIDs)!=9 and len(trbIDs)!=12:

            print(f"ERROR: TI12 run {runno} with unusal number of TRBs: {trbIDs}")
            sys.exit(1)

        if runno < 6520:
            digitizer_map[runno] = copy.copy(TI12_map)
        else:
            digitizer_map[runno] = copy.copy(TI12_6520_map)

    # Remove any channels not enabled in readout
    first = True
    for chan in range(len(digitizer_map[runno])):
        if digitizer_map[runno][chan][0] == 'none': continue
        if chan not in digi_channels:
            if first:
                print(f"{runno} ({host}): {digi_channels}")
                first = False
            print(f"Run {runno} removing channel {chan}: {digitizer_map[runno][chan]}")
            digitizer_map[runno][chan] = ('none', 0)

#remove duplicates from continuous runs
prevMap=[('none', 0)]*16
runList=sorted(digitizer_map)
for run in runList:
    if digitizer_map[run]==prevMap:
        del digitizer_map[run]
    else:
        prevMap = digitizer_map[run]

pprint.pprint(digitizer_map)
#print(digitizer_map)
print(f"Entries: {len(digitizer_map)} runs")

#FIXME: should add check that last run always have full TI12 detector map future runs...


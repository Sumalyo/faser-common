#!/usr/bin/env python3

import json
import copy
import pprint
import sys
import requests
import argparse

# Parse any command-line options
parser = argparse.ArgumentParser(description="Digitizer Scale Checker")

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

# Keyed by run number, contains map of channels => ranges
range_map = dict()
prev_map = None

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
    dynamic_range = dict()

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
            digi_channels.append(chid)
            dynamic_range[chid] = float(chan['dynamic_range'])

        break

    # Done filling dynamic_range map

    if prev_map is None or prev_map != dynamic_range:
        range_map[runno] = copy.copy(dynamic_range)
        prev_map = range_map[runno]

pprint.pprint(range_map)
#print(digitizer_map)
print(f"Entries: {len(range_map)} runs")

#FIXME: should add check that last run always have full TI12 detector map future runs...


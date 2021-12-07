#!/usr/bin/env python3

import json
import pprint
import sys
import urllib.request

f = urllib.request.urlopen("http://faser-runnumber.web.cern.ch/RunList")
runs=json.loads(f.read().decode('utf-8'))
runs.reverse()

trbmap={}

for info in runs:
    runno=info["runnumber"]
    detectors=info["detectors"]
    trbs=[det for det in detectors if det.startswith("TRB")]
    if not trbs: continue
    trbIDs=[int(trb[3:]) for trb in trbs]
    trbIDs.sort()
    host=info["host"].split(".")[0]
    if not host in ["faser-daq-002","faser-daq-006","faser-daq-009","faser-daq-010"]:
        if runno in [423,441,444,448,2802]: continue # known test runs to be ignored
        print(f"ERROR: run {runno} is running on unusual host {host} - not on list of runs to ignore")
        sys.exit(1)
    chMapping=[(-1,-1)]*16
    if host=="faser-daq-002": 
        #EHN1 data or test beam data
        if len(trbIDs)!=3:
            if runno in [1002,1804,1806]: continue
            if runno in range(1219,1238): continue
            print(f"ERROR: EHN1 run {runno} with unusual number of TRBs: {trbIDs}")
            sys.exit(1)
        if (trbIDs[0]%3)!=0 and trbIDs[0]!=11:
            print("ERROR: Unusual first TRB: {trbIDs}")
            sys.exit(1)
        station=0
        for layer,trbid in enumerate(trbIDs):
            chMapping[trbid]=(station,layer)
        trbmap[runno]=chMapping
    else:
        #TI12 data
        for trbid in trbIDs:
            if trbid not in range(0,9) and trbid not in range(11,14):
                print("ERROR: illegal TRB in run {runno}: {trbIDs}")
                sys.exit(1)
            layerid=trbid
            if layerid>10: layerid=layerid-14
            layerid+=3
            station=layerid//3
            layer=layerid%3
            chMapping[trbid]=(station,layer)
        trbmap[runno]=chMapping    

#remove duplicates from continuous runs
prevMap=[None]*16
runList=sorted(trbmap)
for run in runList:
    if trbmap[run]==prevMap:
        del trbmap[run]
    else:
        # check if it just some layers that got removed temporarily
        diff=[ old for old,new in zip(prevMap,trbmap[run]) if old!=new and new!=(-1,-1)] 
        if not diff:
            del trbmap[run]
        else:
            prevMap=trbmap[run]

pprint.pprint(trbmap)
print(f"Entries: {len(trbmap)} runs")

#FIXME: should add check that last run always have full TI12 detector map future runs...


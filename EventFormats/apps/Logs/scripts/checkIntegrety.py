'''
Given a folder conataining a Experiment - check the integrety of the run between
logxyz.json <-------------> logxyz_decompression.json
The jsons conatain events which can be read and verified for decompression integrety verifying Input and output payload sizes
Usage:
    python checkIntegrety.py -dir <directory_name>
Returns
    - If any anomalies are present in runs then what
    = If no anomalies are present a PASS result
Anomalies
    - compressionjsonFile["evdata"][index]["inputSize"] == decompressionjsonFile["evdata"][index]["outputSize"]
    AND
    - compressionjsonFile["evdata"][index]["outputSize"] == decompressionjsonFile["evdata"][index]["inputSize"]
    - compressionjsonFile["evdata"][index]["eventHeader"]["payloadSize"] = compressionjsonFile["evdata"][index]["outputSize"]
    - decompressionjsonFile["evdata"][index]["eventHeader"]["payloadSize"] = decompressionjsonFile["evdata"][index]["outputSize"]
    
'''
import argparse
import glob
import json
if __name__=="__main__":
    parser = argparse.ArgumentParser(
                        prog='checkIntegrety.py',
                        description='Given a folder conataining a Experiment - check the integrety of the run between the compression and decompression logs',
                        epilog='Use this only for validation of logs ')
    parser.add_argument("-d","--dir",help="<directory_name> containing json logs",default="../")
    args  = parser.parse_args()
    directory = args.dir
    print(f"Checking in {directory}")
    jsonFiles = glob.glob(directory + '/*_decompression.json')
    if len(jsonFiles) == 0:
        print("No Decompression logs here")
        exit(1)
    for jsonfile in jsonFiles:
        compressionLog = jsonfile.replace("_decompression", "")
        with open(jsonfile) as f:
            with open(compressionLog) as fc:
                print(f"Testing {f.name} <---------------------> {fc.name}")
                jsonParse = json.load(f)
                jsonParseCompress = json.load(fc)
                eventDataLisDecompressed = jsonParse["evdata"]
                eventDataListCompressed = jsonParseCompress["evdata"]
                for i,ed in enumerate(eventDataLisDecompressed):
                    if ed["outputSize"]==eventDataListCompressed[i]["inputSize"]:
                        print("PASS I/O Test 1")
                    else:
                        print("FAIL I/O Test 1")
                        exit()
                    if ed["inputSize"]==eventDataListCompressed[i]["outputSize"]:
                        print("PASS I/O Test 2")
                    else:
                        print("FAIL I/O Test 2")
                        exit()
                    if str(ed["eventHeader"]["payloadSize"])==ed["outputSize"]:
                        print("PASS Payload set Correctly in Decompression log")
                    else:
                        print("FAIL Payload not set Correctly in Decompression log")
                        exit()
                    if str(eventDataListCompressed[i]["eventHeader"]["payloadSize"])==eventDataListCompressed[i]["outputSize"]:
                        print("PASS Payload set Correctly in Compression log")
                    else:
                        print("FAIL Payload not set Correctly in Compression Log")
                        exit()  
    print("All PASS")

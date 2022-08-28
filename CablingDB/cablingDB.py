#!/bin/env python

# Requires python 3.8 or higher

description = '<timeStamp>run-lumi</timeStamp><addrHeader><address_header clid="1238547719" service_type="71" /></addrHeader><typeName>CondAttrListCollection</typeName>'

# Station, plane pairs for hardware ID running from 0 to 15.
# - Extracted with "extracteTRBMapping.py" script from run info database
mappingData = {
      429: [(0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
      586: [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     1003: [(0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     1040: [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     1099: [(0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     1217: [(-1, -1), (-1, -1), (-1, -1), (0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     1305: [(-1, -1), (-1, -1), (-1, -1), (2, 0), (2, 1), (2, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     1320: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     1342: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     3284: [(0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     3287: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     3288: [(0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4272: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4359: [(0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4399: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4404: [(0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4411: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4418: [(0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4439: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4857: [(0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4865: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4875: [(0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4892: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4900: [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1)],
     4912: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4954: [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1)],
     4989: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4991: [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1)],
     4993: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     4996: [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1)],
     4997: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     5042: [(-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1)],
     5050: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1), (-1, -1)],
     5303: [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0), (3, 1), (3, 2), (-1, -1), (-1, -1), (0, 0), (0, 1), (0, 2), (-1, -1), (-1, -1)],
    }

maxInt32 = 0xFFFFFFFF

# PMT type, followed by station, plate, pmt for scintillators
# or row, module, pmt for the calorimeter
# Extracted from DAQ configuration using extractDigitizerMapping.py
digi_map = {
429: [('trigger', 0, 0, 0),
       ('trigger', 0, 1, 0),
       ('trigger', 0, 2, 0),
       ('trigger', 0, 3, 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0)],
 466: [('trigger', 0, 0, 0),
       ('none', 0),
       ('trigger', 0, 1, 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0)],
 639: [('trigger', 0, 0, 0),
       ('none', 0),
       ('trigger', 0, 1, 0),
       ('none', 0),
       ('trigger', 0, 2, 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0)],
 660: [('trigger', 0, 0, 0),
       ('none', 0),
       ('trigger', 0, 1, 0),
       ('none', 0),
       ('trigger', 0, 2, 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('none', 0),
       ('clock', 0)],
 1324: [('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 1345: [('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('none', 0),
        ('clock', 0)],
 1386: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('none', 0),
        ('clock', 0)],
 3243: [('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 3247: [('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 3395: [('calo', 1, 2, 0),
        ('calo', 0, 2, 0),
        ('calo', 1, 1, 0),
        ('calo', 0, 1, 0),
        ('calo', 1, 0, 0),
        ('calo', 0, 0, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4272: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4360: [('trigger', 0, 0, 0),
        ('none', 0),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('trigger', 0, 2, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4399: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4409: [('trigger', 0, 0, 0),
        ('none', 0),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('trigger', 0, 2, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4411: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4429: [('trigger', 0, 0, 0),
        ('none', 0),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('trigger', 0, 2, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4439: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4452: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('none', 0),
        ('clock', 0)],
 4876: [('trigger', 0, 0, 0),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4892: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('none', 0),
        ('clock', 0)],
 4904: [('trigger', 0, 0, 0),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4912: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('none', 0),
        ('clock', 0)],
 4954: [('trigger', 0, 0, 0),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4989: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('none', 0),
        ('clock', 0)],
 4991: [('trigger', 0, 0, 0),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4993: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('none', 0),
        ('clock', 0)],
 4996: [('trigger', 0, 0, 0),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 4997: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('none', 0),
        ('clock', 0)],
 5042: [('trigger', 0, 0, 0),
        ('trigger', 0, 1, 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('none', 0),
        ('clock', 0)],
 5050: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('veto', 0, 0, 0),
        ('veto', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('none', 0),
        ('clock', 0)],
 6525: [('calo', 0, 1, 0),
        ('calo', 0, 0, 0),
        ('calo', 1, 1, 0),
        ('calo', 1, 0, 0),
        ('vetonu', 0, 0, 0),
        ('vetonu', 0, 1, 0),
        ('veto', 1, 0, 0),
        ('veto', 1, 1, 0),
        ('trigger', 0, 0, 1),
        ('trigger', 0, 0, 0),
        ('trigger', 0, 1, 1),
        ('trigger', 0, 1, 0),
        ('preshower', 0, 0, 0),
        ('preshower', 0, 1, 0),
        ('veto', 0, 1, 0),
        ('clock', 0)]
}


# Range map from extractDigitizerScale.py
range_map = {
 432: {0: 2.0,
       1: 2.0,
       2: 2.0,
       3: 2.0,
       4: 2.0,
       5: 2.0,
       6: 2.0,
       7: 2.0,
       8: 2.0,
       9: 2.0,
       10: 2.0,
       11: 2.0,
       12: 2.0,
       13: 2.0,
       14: 2.0,
       15: 2.0},
7932: {0: 0.5,
        1: 0.5,
        2: 0.5,
        3: 0.5,
        4: 2.0,
        5: 2.0,
        6: 2.0,
        7: 2.0,
        8: 2.0,
        9: 2.0,
        10: 2.0,
        11: 2.0,
        12: 2.0,
        13: 2.0,
        14: 2.0,
        15: 2.0},
 7965: {0: 2.0,
        1: 2.0,
        2: 2.0,
        3: 2.0,
        4: 2.0,
        5: 2.0,
        6: 2.0,
        7: 2.0,
        8: 2.0,
        9: 2.0,
        10: 2.0,
        11: 2.0,
        12: 2.0,
        13: 2.0,
        14: 2.0,
        15: 2.0},
 8251: {0: 0.5,
        1: 0.5,
        2: 0.5,
        3: 0.5,
        4: 2.0,
        5: 2.0,
        6: 2.0,
        7: 2.0,
        8: 2.0,
        9: 2.0,
        10: 2.0,
        11: 2.0,
        12: 2.0,
        13: 2.0,
        14: 2.0,
        15: 2.0},
 8252: {0: 2.0,
        1: 2.0,
        2: 2.0,
        3: 2.0,
        4: 2.0,
        5: 2.0,
        6: 2.0,
        7: 2.0,
        8: 2.0,
        9: 2.0,
        10: 2.0,
        11: 2.0,
        12: 2.0,
        13: 2.0,
        14: 2.0,
        15: 2.0}
}

# Look for data entry errors

print('Validating TRB data')

lastRun = 0
for run, data in mappingData.items():
    assert isinstance(run, int), 'Run number is not integer'
    assert run > lastRun, 'Run numbers out of order'
    assert run <= maxInt32, 'Run number out of range'
    lastRun = run
    assert len(data) == 16, 'Run does not have 16 TRB entries'
    for i in range(16):
        assert len(data[i]) == 2, 'TRB entry does not have two values'
        assert isinstance(data[i][0], int), 'Station number is not integer'
        assert isinstance(data[i][1], int), 'Plane number is not integer'
        assert data[i][0] >= -1 and data[i][0] <= 3, 'Invalid station number'
        assert data[i][1] >= -1 and data[i][1] <= 2, 'Invalid plane number'
        assert (data[i][0] == -1 and data[i][1] == -1) or (data[i][0] != -1 and data[i][1] != -1), 'Invalid station/plane combination'
    for i in range(15):
        if data[i][0] != -1:
            for j in range(15-i):
                k = i + j + 1
                assert (data[i][0] != data[k][0] or data[i][1] != data[k][1]), 'Duplicated (station, plane) pair'

print('Validating digitizer data')

lastRun = 0
for run, data in digi_map.items():
    assert isinstance(run, int), 'Run number is not integer'
    assert run > lastRun, 'Run numbers out of order'
    assert run <= maxInt32, 'Run number out of range'
    lastRun = run
    assert len(data) == 16, 'Run does not have 16 digitizer channels'
    for i in range(16):
        if data[i][0] == 'none': continue  # Skip empty channels
        if data[i][0] == 'clock': continue
        assert len(data[i]) == 4, 'Digitizer entry does not have four values'
        assert isinstance(data[i][0], str), 'Digitizer type is not string'
        assert isinstance(data[i][1], int), 'Digitizer station is not int'
        assert isinstance(data[i][2], int), 'Digitizer plane is not int'
        assert isinstance(data[i][3], int), 'Digitizer pmt is not int'
        assert data[i][0] in ['trigger', 'veto', 'vetonu', 'preshower', 'calo'], 'Invalid type'
        assert data[i][1] >= -1 and data[i][1] <= 1, 'Invalid station number'
        assert data[i][2] >= -1 and data[i][2] <= 3, 'Invalid plate number'
        assert data[i][3] >= -1 and data[i][3] <= 1, 'Invalid pmt number'
    
# Data looks OK


from PyCool import cool

# Use CONDBR3 for real data
dbSvc = cool.DatabaseSvcFactory.databaseService()
connectString = 'sqlite://;schema=CABP200.db;dbname=CONDBR3'

print('Recreating database')

dbSvc.dropDatabase( connectString )
db = dbSvc.createDatabase( connectString )

cableSpec = cool.RecordSpecification()
cableSpec.extend( 'station', cool.StorageType.Int32 )
cableSpec.extend( 'plane'  , cool.StorageType.Int32 )

cableFolderSpec = cool.FolderSpecification(cool.FolderVersioning.SINGLE_VERSION, cableSpec)
cableFolder = db.createFolder('/SCT/DAQ/CableMapping', cableFolderSpec, description, True)

# There should be one record entered per TRB per IOV
lastValid = cool.ValidityKeyMax
for firstValidRun, mapping in reversed(mappingData.items()):
    firstValid = (firstValidRun << 32)
    for hardwareID in range(16):
        cableRecord = cool.Record(cableSpec)
        cableRecord[ 'station' ] = mapping[hardwareID][0]
        cableRecord[ 'plane' ] = mapping[hardwareID][1]
        cableFolder.storeObject( firstValid, lastValid, cableRecord, int(hardwareID) )
    lastValid = ((firstValidRun - 1) << 32) | (cool.ValidityKeyMax & 0x00000000FFFFFFFF)


# Add digitizer mapping
print('Recreating digitizer database')

cableSpec = cool.RecordSpecification()
cableSpec.extend( 'type', cool.StorageType.String255 )
cableSpec.extend( 'station', cool.StorageType.Int32 )  # Used for scintillators
cableSpec.extend( 'plate', cool.StorageType.Int32 )
cableSpec.extend( 'row', cool.StorageType.Int32 ) # Used for calorimeters
cableSpec.extend( 'module', cool.StorageType.Int32 )
cableSpec.extend( 'pmt', cool.StorageType.Int32 )

digiFolderSpec = cool.FolderSpecification(cool.FolderVersioning.SINGLE_VERSION, cableSpec)
digiFolder = db.createFolder('/WAVE/DAQ/CableMapping', digiFolderSpec, description, True)

# There should be one record entered per IOV
lastValid = cool.ValidityKeyMax
for firstValidRun, mapping in reversed(digi_map.items()):
    firstValid = (firstValidRun << 32)
    for channel in range(16):
        cableRecord = cool.Record(cableSpec)
        cableRecord[ 'type' ] = mapping[channel][0]
        if cableRecord[ 'type' ] == 'calo':
            cableRecord['row'] = mapping[channel][1]
            cableRecord['module'] = mapping[channel][2]
            cableRecord['pmt'] = mapping[channel][3]
        # Already asserted allowable types above
        elif len(mapping[channel]) == 4:
            cableRecord['station'] = mapping[channel][1]
            cableRecord['plate'] = mapping[channel][2]
            cableRecord['pmt'] = mapping[channel][3]

        digiFolder.storeObject( firstValid, lastValid, cableRecord, int(channel) )

    lastValid = ((firstValidRun - 1) << 32) | (cool.ValidityKeyMax & 0x00000000FFFFFFFF)

# Add digitizer scale
print('Recreating digitizer scale')

digiSpec = cool.RecordSpecification()
digiSpec.extend('range', cool.StorageType.Float)

digiFolderSpec = cool.FolderSpecification(cool.FolderVersioning.SINGLE_VERSION, digiSpec)
digiFolder = db.createFolder('/WAVE/DAQ/Range', digiFolderSpec, description, True)

# There should be one record entered per IOV
lastValid = cool.ValidityKeyMax
for firstValidRun, mapping in reversed(range_map.items()):
    firstValid = (firstValidRun << 32)
    for channel in range(16):
        digiRecord = cool.Record(digiSpec)
        digiRecord['range'] = mapping[channel]
        digiFolder.storeObject( firstValid, lastValid, digiRecord, int(channel) )

    lastValid = ((firstValidRun - 1) << 32) | (cool.ValidityKeyMax & 0x00000000FFFFFFFF)

# Done
db.closeDatabase()

print('Database completed')

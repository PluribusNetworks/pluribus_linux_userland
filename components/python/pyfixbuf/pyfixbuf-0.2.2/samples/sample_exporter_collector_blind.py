#!/usr/bin/python
## ------------------------------------------------------------------------
## sample_exporter_collector_blind.py
##
## sample IPFIX collector & exporter (aka mediator) using pyfixbuf.
## created to be used with the sample_exporter.py script.
## Processes the ipfix file created by sample_exporter.py and writes
## to a text file.  This is different from sample_exporter_collector.py
## in that it has no idea what to expect.
## ------------------------------------------------------------------------
## Copyright (C) 2013-2015 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## ------------------------------------------------------------------------
## See license information in LICENSE-OPENSOURCE.txt

import sys
import pyfixbuf

# Test that the argument number is correct
if (len(sys.argv) < 2):
    print "Must supply an IPFIX file to read and (optional) text file "
    "to write to."
    sys.exit()


#create the information model with the standard IPFIX elements
infomodel = pyfixbuf.InfoModel()

# add YAF's HTTP IPFIX elements for a sub template
infomodel.add_element_list(pyfixbuf.YAF_HTTP_LIST)

# create the collector
collector = pyfixbuf.Collector()

#create the IPFIX file to read from
collector.init_file(sys.argv[1])

#create the session
session = pyfixbuf.Session(infomodel)

#create the buffer for the collector, set auto to True to 
#automatically generate templates
buf = pyfixbuf.Buffer(auto=True)

# make the buffer an export buffer
buf.init_collection(session, collector)

# set auto insert on buffer in case we receive any Info Element Options Recs
buf.auto_insert()

if (len(sys.argv) > 2):
    outFile = open(sys.argv[2], "w")
else:
    outFile = sys.stdout

flowcount = 0

dontprintlist=["subTemplateMultiList", "subTemplateList", "paddingOctets"]

for data in buf:

    # convert the rec to a dictionary
    data = data.as_dict()
    for key,value in data.items():
        if (key not in dontprintlist):
            outFile.write(str(key) + ": " + str(value) + "\n")

    flowcount += 1
    # get data out of STML
    if "subTemplateMultiList" in data:
        stml = data["subTemplateMultiList"]
        subs = 0
        for entry in stml:
            print "--- STML %d ---" % subs
            recs = 0
            for record in entry:
                x = 0
                print "-- ENTRY %d --" % recs
                for elem in record:
                    if type(elem) is pyfixbuf.STL:
                        for item in elem:
                            item = item.as_dict()
                            for key,value in item.items():
                                outFile.write(key + ": " + str(value) + "\n")
                        elem.clear()

                    elif type(elem) is pyfixbuf.BL:
                        count = 0
                        for item in elem:
                            outFile.write(elem.element.name + " " + str(count) + " is " + item + "\n")
                            count += 1
                        elem.clear()
                    else:
                        outFile.write("Item " + elem.element.name + ": " + str(value) + "\n")
                    x += 1
                recs += 1
            subs += 1

        stml.clear()
    outFile.write("------------------------------\n")

sys.stdout.write("Processed " + str(flowcount) + " flows \n")



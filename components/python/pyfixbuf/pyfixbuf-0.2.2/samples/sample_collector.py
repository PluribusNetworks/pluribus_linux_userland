#!/usr/bin/env python
## ------------------------------------------------------------------------
## sample_collector.py
## sample IPFIX collector using pyfixbuf.
## Reads an IPFIX file and writes to a text file.
## ------------------------------------------------------------------------
## Copyright (C) 2013-2015 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## See license information in LICENSE-OPENSOURCE.txt 

import sys
import pyfixbuf as p
from netsa.data.times import *

# Test that the argument number is correct

if (len(sys.argv) != 3):
    print "Must supply an IPFIX file to read and a text file to write."
    print "Usage: sample_collector.py file.ipfix file.txt"
    sys.exit()

# Create an InfoModel

infomodel = p.InfoModel()

# Add basic YAF & stats elements to the infomodel

infomodel.add_element_list(p.YAF_LIST)
infomodel.add_element_list(p.YAF_STATS_LIST)
infomodel.add_element_list(p.YAF_DNS_LIST)
infomodel.add_element_list(p.YAF_HTTP_LIST)

# Create a Template

tmpl = p.Template(infomodel)

# Create a Stats Template

stats_tmpl = p.Template(infomodel)

# Add some elements to the data template

data_list = [
    p.InfoElementSpec("flowStartMilliseconds"),
    p.InfoElementSpec("flowEndMilliseconds"),
    p.InfoElementSpec("octetTotalCount"),
    p.InfoElementSpec("reverseOctetTotalCount"),
    p.InfoElementSpec("packetTotalCount"),
    p.InfoElementSpec("reversePacketTotalCount"),
    p.InfoElementSpec("sourceIPv4Address"),
    p.InfoElementSpec("destinationIPv4Address"),
    p.InfoElementSpec("sourceTransportPort"),
    p.InfoElementSpec("destinationTransportPort"),
    p.InfoElementSpec("flowAttributes"),
    p.InfoElementSpec("reverseFlowAttributes"),
    p.InfoElementSpec("protocolIdentifier"),
    p.InfoElementSpec("flowEndReason"),
    p.InfoElementSpec("silkAppLabel"),
    p.InfoElementSpec("subTemplateMultiList")]

# Add elements to stats template (this is only a subset of stats that YAF exports)
stats_list = [
    p.InfoElementSpec("exportedFlowRecordTotalCount"),
    p.InfoElementSpec("packetTotalCount"),
    p.InfoElementSpec("droppedPacketTotalCount"),
    p.InfoElementSpec("ignoredPacketTotalCount")]

# Add the lists to their respective template

tmpl.add_spec_list(data_list)
stats_tmpl.add_spec_list(stats_list)

# Create a collector

collector = p.Collector()

# Give the collector your input file to read from

collector.init_file(sys.argv[1])

# Create a session

session = p.Session(infomodel)

# Add your template to the session

session.add_internal_template(tmpl, 999)

# Add the Stats Template

session.add_internal_template(stats_tmpl, 911)

# create a Record for the main template and stats template

rec = p.Record(infomodel, tmpl)

statsrec = p.Record(infomodel, stats_tmpl)

# Create a buffer to read from
buf = p.Buffer()

# Add the session and collector the buffer

buf.init_collection(session, collector)

# Set your internal template to the data template

buf.set_internal_template(999)

# open our output file

outFile = open(sys.argv[2], "w")

flowcount = 0

noprintlist=["paddingOctets", "subTemplateList", "subTemplateMultiList"]

# Now we are ready to get the elements from the buffer!
for data in buf:
    flowcount += 1
    data = data.as_dict()
    for key,value in data.items():
        if (key == "flowStartMilliseconds" or key == "flowEndMilliseconds"):
            outFile.write(key + ": " + str(make_datetime(value/1000)) + "\n")
        elif key not in noprintlist:
            outFile.write(key + ": " + str(value) + "\n")

    stml = data["subTemplateMultiList"]
    for entry in stml:
        if "tcpSequenceNumber" in entry:
            for record in entry:
                record = record.as_dict()
                for key,value in record.items():
                    outFile.write(key + ": " + str(value) + "\n")
        elif entry.template_id == 0xCE00:
            for record in entry:
                stl = record[0]
                for dnsrec in stl:
                    dnsrec = dnsrec.as_dict()
                    for key,value in dnsrec.items():
                        if (key not in noprintlist):
                            outFile.write(key + ": " + str(value) + '\n')

            stl.clear()

    stml.clear()
    outFile.write("----------------------------\n")

# check to see if the next template is a stats template
    tmpl_next = buf.next_template()

    if ( tmpl_next.scope ):
        outFile.write("STATS Record:\n")
        # set the interal template to the stats template
        buf.set_internal_template(911)
        # get the record
        stats = buf.next_record(statsrec)
        if (stats != None):
            stats = stats.as_dict()
            # print all the items in stats
            for key,value in stats.items():
                outFile.write(key + ": " + str(value) + "\n")
        # set the template back to the data template
        buf.set_internal_template(999)

sys.stdout.write("Finished. Processed " + str(flowcount) + " flows.\n")


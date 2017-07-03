#!/usr/bin/python
## ------------------------------------------------------------------------
## sample_listener.py
##
## sample IPFIX listener using pyfixbuf.
## This script listens for connections from an exporter and writes
## to a text file.  
## Usage: hostname port {tcp,udp} file
## ------------------------------------------------------------------------
## Copyright (C) 2013-2015 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## ------------------------------------------------------------------------
## See license information in LICENSE-OPENSOURCE.txt

import sys
import pyfixbuf
import netsa_silk as silk

# Test that the argument number is correct

if (len(sys.argv) < 4):
    print "Usage: sample_listener.py hostname port transport [out_file]."
    print "This script runs forever"
    sys.exit()

# Create an InfoModel

infomodel = pyfixbuf.InfoModel()

# Add basic YAF & stats elements to the infomodel

infomodel.add_element_list(pyfixbuf.YAF_LIST)
infomodel.add_element_list(pyfixbuf.YAF_STATS_LIST)
infomodel.add_element_list(pyfixbuf.YAF_DNS_LIST)
infomodel.add_element_list(pyfixbuf.YAF_HTTP_LIST)

# Create a Template

tmpl = pyfixbuf.Template(infomodel)

# Create a Stats Template

stats_tmpl = pyfixbuf.Template(infomodel)

# Add some elements to the data template

data_list = [pyfixbuf.InfoElementSpec("flowStartMilliseconds"),
             pyfixbuf.InfoElementSpec("flowEndMilliseconds"),
             pyfixbuf.InfoElementSpec("octetTotalCount"),
             pyfixbuf.InfoElementSpec("reverseOctetTotalCount"),
             pyfixbuf.InfoElementSpec("packetTotalCount"),
             pyfixbuf.InfoElementSpec("reversePacketTotalCount"),
             pyfixbuf.InfoElementSpec("sourceIPv4Address"),
             pyfixbuf.InfoElementSpec("destinationIPv4Address"),
             pyfixbuf.InfoElementSpec("sourceTransportPort"),
             pyfixbuf.InfoElementSpec("destinationTransportPort"),
             pyfixbuf.InfoElementSpec("flowAttributes"),
             pyfixbuf.InfoElementSpec("reverseFlowAttributes"),
             pyfixbuf.InfoElementSpec("protocolIdentifier"),
             pyfixbuf.InfoElementSpec("flowEndReason"),
             pyfixbuf.InfoElementSpec("silkAppLabel"),
             pyfixbuf.InfoElementSpec("subTemplateMultiList")]

# Add elements to stats template (this is only a subset of stats that YAF exports)
stats_list = [pyfixbuf.InfoElementSpec("exportedFlowRecordTotalCount"),
              pyfixbuf.InfoElementSpec("packetTotalCount"),
              pyfixbuf.InfoElementSpec("droppedPacketTotalCount"),
              pyfixbuf.InfoElementSpec("ignoredPacketTotalCount")]

# Add the lists to their respective template

tmpl.add_spec_list(data_list)
stats_tmpl.add_spec_list(stats_list)

# Create a session

session = pyfixbuf.Session(infomodel)

# Add your template to the session

session.add_internal_template(tmpl, 999)

# Add the Stats Template

session.add_internal_template(stats_tmpl, 911)

# create a Rec for each template or subtemplate to give the buffer

rec = pyfixbuf.Record(infomodel, tmpl)

statsrec = pyfixbuf.Record(infomodel, stats_tmpl)

transport = sys.argv[3]
# Create a listener

listener = pyfixbuf.Listener(session, hostname=sys.argv[1], transport=transport.lower(), port=sys.argv[2])

# open our output file
if len(sys.argv) > 4:
    outFile = open(sys.argv[4], "w")
else:
    outFile = sys.stdout

flowcount = 0

# Start listening
while (1):
    if (flowcount):
        print("Received %d flows" % flowcount)
    buf = listener.wait()
    buf.set_record(rec)
    # Set your internal template to the data template
    buf.set_internal_template(999)

    for record in buf:
        flowcount += 1
        data = record.as_dict()
        for key,value in data.items():
            if key == "sourceIPv4Address" or key == "destinationIPv4Address":
                addr = silk.IPAddr(value)
                outFile.write(key + ": " + str(addr) + "\n")
            elif key != "subTemplateMultiList":
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
                    stl = record["subTemplateList"]
                    for dnsrec in stl:
                        dnsrec = dnsrec.as_dict()
                        for key,value in dnsrec.items():
                            if (key != "subTemplateList"):
                                outFile.write(key + ": " + str(value) + '\n')

                stl.clear()

        stml.clear()
        outFile.write("----------------------------\n")

        # check to see if the next template is a stats template
        tmpl_next = buf.next_template()

        if ( tmpl_next.scope ):
            sys.stdout.write("STATS Record:\n")
            # set the interal template to the stats template
            buf.set_internal_template(911)
            # get the record
            stats = buf.next_record(statsrec)
            if (stats != None):
                stats = stats.as_dict()
            # print all the items in stats
                for key,value in stats.items():
                    sys.stdout.write(key + ": " + str(value) + "\n")
            sys.stdout.write("-------------------------\n")
            # set the template back to the data template
            buf.set_internal_template(999)



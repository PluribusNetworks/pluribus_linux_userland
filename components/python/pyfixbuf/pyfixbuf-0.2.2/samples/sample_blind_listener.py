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

if (len(sys.argv) < 4):
    print "Usage: sample_listener.py hostname port transport [out_file]."
    print "This script runs forever"
    sys.exit()


#create the information model with the standard IPFIX elements
infomodel = pyfixbuf.InfoModel()
# add YAF's HTTP IPFIX elements for a sub template
infomodel.add_element_list(pyfixbuf.YAF_LIST)
infomodel.add_element_list(pyfixbuf.YAF_HTTP_LIST)
infomodel.add_element_list(pyfixbuf.YAF_DNS_LIST)
infomodel.add_element_list(pyfixbuf.YAF_FLOW_STATS_LIST)
tmpl = pyfixbuf.Template(infomodel)

data_list = data_list = [pyfixbuf.InfoElementSpec("flowStartMilliseconds"),
             pyfixbuf.InfoElementSpec("flowEndMilliseconds"),
             pyfixbuf.InfoElementSpec("octetTotalCount"),
             pyfixbuf.InfoElementSpec("reverseOctetTotalCount"),
             pyfixbuf.InfoElementSpec("packetTotalCount"),
             pyfixbuf.InfoElementSpec("reversePacketTotalCount"),
             pyfixbuf.InfoElementSpec("sourceIPv4Address"),
             pyfixbuf.InfoElementSpec("destinationIPv4Address"),
             pyfixbuf.InfoElementSpec("sourceTransportPort"),
             pyfixbuf.InfoElementSpec("destinationTransportPort"),
             pyfixbuf.InfoElementSpec("protocolIdentifier"),
                         pyfixbuf.InfoElementSpec("subTemplateMultiList")]

tmpl.add_spec_list(data_list)

#create the session
session = pyfixbuf.Session(infomodel)

session.add_internal_template(tmpl, 999)

transport = sys.argv[3]
# Create a listener 

listener = pyfixbuf.Listener(session, hostname=sys.argv[1], transport=transport.lower(), port=sys.argv[2])

if (len(sys.argv) > 4):
    outFile = open(sys.argv[4], "w")
else:
    outFile = sys.stdout

flowcount = 0

dontprintlist=["subTemplateMultiList", "subTemplateList", "paddingOctets"]
tmpl_list=[]
while (1):

    buf = listener.wait()

    tmpl_next = buf.next_template()
    print tmpl_next.template_id
    if tmpl_next.template_id == 0:
        try:
            buf.set_internal_template(999)
            data = buf.next()
        except StopIteration:
            continue
    tmpl = session.get_template(tmpl_next.template_id)
    for i in range(0, len(tmpl)):
        ie = tmpl.getIndexedIE(i)
        if (ie.length != 65535):
            tmpl.specs.append(pyfixbuf.InfoElementSpec(ie.tname, ie.length))
        else:
            tmpl.specs.append(pyfixbuf.InfoElementSpec(ie.tname))
    session.add_template(tmpl, tmpl_next.template_id)
    buf.set_internal_template(tmpl_next.template_id)

    try:
        data = buf.next()
    except StopIteration:
        if (not(listener)):
            break
        else:
            continue

#    for data in buf:
    if data:
        # convert the rec to a dictionary
        data = data.as_dict()
        for key,value in data.items():
            if (key not in dontprintlist):
                outFile.write(str(key) + ": " + str(value) + "\n")

        if "subTemplateMultiList" in data:
            stml = data["subTemplateMultiList"]
            subs = 0
            for entry in stml:
                print "--- STML %d ---" % subs
                recs = 0
                for record in entry:
                    x = 0
                    print "-- ENTRY %d --" % recs
                    record = record.as_dict()
                    for r, t in record.items():
                        outFile.write(str(r) + ": " + str(t) + "\n")
#                    for elem in record:
#                        if type(elem) is pyfixbuf.STL:
#                            for item in elem:
#                                item = item.as_dict()
#                                for key,value in item.items():
#                                    outFile.write(key + ": " + str(value) + "\n")
#                            elem.clear()
#                        elif type(elem) is pyfixbuf.BL:
#                            count = 0
#                            for item in elem:
#                                outFile.write(elem.element.name + " " + str(count) + " is " + item + "\n")
#                                count += 1
#                            elem.clear()
#                        else:
#                            outFile.write("Item " + str(x) + ": " + str(elem) + "\n")
                        x += 1
                recs += 1
            subs += 1

        stml.clear()


        flowcount += 1
    # get data out of STML
        outFile.write("------------------------------\n")

sys.stdout.write("Processed " + str(flowcount) + " flows \n")



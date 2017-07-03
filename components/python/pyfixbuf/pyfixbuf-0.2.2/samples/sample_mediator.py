#!/usr/bin/python
## ------------------------------------------------------------------------
## sample_mediator.py
##
## Example of a true yaf mediator.  Reads YAF IPFIX files and exports
## a modified version (subset) to an IPFIX file.  
## See exporter_list for the modified template.
## ------------------------------------------------------------------------
## Copyright (C) 2013 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## ------------------------------------------------------------------------
## See license information in LICENSE-OPENSOURCE.txt
## ------------------------------------------------------------------------


import sys
import pyfixbuf

# Test that the argument number is correct
if (len(sys.argv) != 3):
    print "Must supply an IPFIX file to read and to write to."
    print "Usage: sample_mediator.py infile.ipfix outfile.ipfix"
    sys.exit()

infomodel = pyfixbuf.InfoModel()

infomodel.add_element_list(pyfixbuf.YAF_LIST)
infomodel.add_element_list(pyfixbuf.YAF_HTTP_LIST)
infomodel.add_element_list(pyfixbuf.YAF_DNS_LIST)

tmpl = pyfixbuf.Template(infomodel)

statstmpl = pyfixbuf.Template(infomodel)

exportertmpl = pyfixbuf.Template(infomodel)

data_list = [
    pyfixbuf.InfoElementSpec("flowStartMilliseconds"),
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

stats_list = [
    pyfixbuf.InfoElementSpec("exportedFlowRecordTotalCount"),
    pyfixbuf.InfoElementSpec("packetTotalCount"),
    pyfixbuf.InfoElementSpec("droppedPacketTotalCount"),
    pyfixbuf.InfoElementSpec("ignoredPacketTotalCount")]

exporter_list = [
    pyfixbuf.InfoElementSpec("sourceIPv4Address"),
    pyfixbuf.InfoElementSpec("destinationIPv4Address"),
    pyfixbuf.InfoElementSpec("sourceTransportPort"),
    pyfixbuf.InfoElementSpec("destinationTransportPort"),
    pyfixbuf.InfoElementSpec("packetTotalCount"),
    pyfixbuf.InfoElementSpec("reversePacketTotalCount"),
    pyfixbuf.InfoElementSpec("payload"),
    pyfixbuf.InfoElementSpec("reversePayload")]

# add the elements we want in collection template
tmpl.add_spec_list(data_list)

# add stats elements we want from the incoming stats template
statstmpl.add_spec_list(stats_list)

# add the items we want to export to our export template
exportertmpl.add_spec_list(exporter_list)

# create the collector
collector = pyfixbuf.Collector()

# create the exporter
exporter = pyfixbuf.Exporter()

# get the file from the command line arguments to read
collector.init_file(sys.argv[1])

# get the file to write to from the command line arguments
exporter.init_file(sys.argv[2])

# create the sessions for the collector & exporter
session = pyfixbuf.Session(infomodel)
exsession = pyfixbuf.Session(infomodel)

# add the appropriate templates to each session
session.add_internal_template(tmpl, template_id=876)

session.add_internal_template(statstmpl, template_id=900)

exsession.add_template(exportertmpl, template_id=987)

# create the input record from the input template since there are no duplicate elements
rec = pyfixbuf.Record(infomodel, tmpl)

# we can create the export rec from the export template since there are no duplicate elements
exportrec = pyfixbuf.Record(infomodel, exportertmpl)

# create buffers for input and export
buf = pyfixbuf.Buffer(rec)
buf.init_collection(session, collector)

exbuf = pyfixbuf.Buffer(exportrec)
exbuf.init_export(exsession, exporter)

# set the internal template on the input buffer
buf.set_internal_template(876)

# export the templates
exsession.export_templates()


# set the internal and external templates on the export buffer
exbuf.set_internal_template(987)
exbuf.set_export_template(987)

# this record is found in the subtemplatemultilist coming from YAF
payrec = pyfixbuf.Record(infomodel)
payrec.add_element("payload")
payrec.add_element("reversePayload")

# this record is also found in the subtemplatemultilist coming from YAF
httprec = pyfixbuf.Record(infomodel)
httprec.add_element("httpServerString", pyfixbuf.BASICLIST)
httprec.add_element("httpUserAgent", pyfixbuf.BASICLIST)
httprec.add_element("httpGet", pyfixbuf.BASICLIST)
httprec.add_element("httpConnection", pyfixbuf.BASICLIST)
httprec.add_element("httpReferer", pyfixbuf.BASICLIST)

# this record is also found in the STML coming from YAF
dnsouter = pyfixbuf.Record(infomodel)
dnsouter.add_element("dnsQRList", pyfixbuf.SUBTEMPLATELIST)

# this record will be contained in the subtemplatelist, "dnsQRList"
dnsinner = pyfixbuf.Record(infomodel)
dnsinner.add_element("dnsList", pyfixbuf.SUBTEMPLATELIST)
dnsinner.add_element("dnsQName")
dnsinner.add_element("dnsTTL")
dnsinner.add_element("dnsQRType")
dnsinner.add_element("dnsQueryResponse")
dnsinner.add_element("dnsAuthoritative")
dnsinner.add_element("dnsNXDomain")
dnsinner.add_element("dnsRRSection")

# the following record is found in the subtemplatelist "dnsList" depending on the DNS Response type
dnsA = pyfixbuf.Record(infomodel)
dnsA.add_element("dnsNSDName")

# create the stats rec
statrec = pyfixbuf.Record(infomodel, statstmpl)

flowcount = 0

for data in buf:

    flowcount += 1
    # this will copy all the same elements from the incoming rec to the outgoing
    # we still need to deal with any items in the list that we want to copy
    exportrec.copy(data)

    # get items from STML
    stml = data["subTemplateMultiList"]
    for entry in stml:
        if "payload" in entry:
            # this is the payload template from YAF
            entry.set_record(payrec)
            for pay in entry:
                exportrec['payload'] = pay['payload']
                exportrec['reversePayload'] = pay['reversePayload']
        elif entry.template_id == 0xC600 or entry.template_id == 0xC610:
            # this is the HTTP Template from YAF
            entry.set_record(httprec)
            for http in entry:
                # we could get the elements out of here - right now we can just clear them
                http.clear_all_lists()
                # this is the same as doing all of this...
                #http.clear_basic_list("httpUserAgent")
                #http.clear_basic_list("httpServerString")
                #http.clear_basic_list("httpGet")
                #http.clear_basic_list("httpConnection")
                #http.clear_basic_list("httpReferer")
        elif entry.template_id == 0xCE00:
            # this is the DNS template from YAF
            entry.set_record(dnsouter)
            for record in entry:
                stl = dnsouter["dnsQRList"]
                stl.set_record(dnsinner)
                for dnsrec in stl:
                    # we can get all the elements out of here - just clear them right now
                    if (dnsrec['dnsQRType'] == 2):
                        stl2 = dnsrec["dnsList"]
                        # this is the inner STL
                        stl2.set_record(dnsA)
                        for arec in stl2:
                            print "A Record!"
                        stl2.clear()
                stl.clear()

    # append the exportrec
    exbuf.append(exportrec)
    # clear the stml
    stml.clear()
    # clears the fields in the exportrec
    exportrec.clear()

# check to see if the next template is a stats template
    tmpl_next = buf.next_template()

    if ( tmpl_next.scope ):
        # set the interal template to the stats template
        buf.set_internal_template(900)
        # get the record
        stats = buf.next_record(statrec)
        #if (stats != None):
            # print all the items in stats
        # set the template back to the data template
        buf.set_internal_template(876)


exbuf.emit()
sys.stdout.write("Processed " + str(flowcount) + " flows \n")

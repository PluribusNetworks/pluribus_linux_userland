#!/usr/bin/python
## ------------------------------------------------------------------------
## sample_exporter_collector.py
##
## sample IPFIX collector & exporter (aka mediator) using pyfixbuf.
## created to be used with the sample_exporter.py script.
## Processes the ipfix file created by sample_exporter.py and writes
## to a text file.
## ------------------------------------------------------------------------
## Copyright (C) 2013-2015 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## ------------------------------------------------------------------------
## See license information in LICENSE-OPENSOURCE.txt

import sys
import pyfixbuf

# Test that the argument number is correct
if (len(sys.argv) != 3):
    print "Must supply an IPFIX file to read and text file to write to."
    sys.exit()


#create the information model with the standard IPFIX elements
infomodel = pyfixbuf.InfoModel()

# add YAF's HTTP IPFIX elements for a sub template
infomodel.add_element_list(pyfixbuf.YAF_HTTP_LIST)

# create the "outer" template
tmpl = pyfixbuf.Template(infomodel)

# add elements we want in our "outer" template
import_list = [
    pyfixbuf.InfoElementSpec("flowStartMilliseconds"),
    pyfixbuf.InfoElementSpec("packetTotalCount"),
    pyfixbuf.InfoElementSpec("reversePacketTotalCount"),
    pyfixbuf.InfoElementSpec("sourceIPv4Address"),
    pyfixbuf.InfoElementSpec("destinationIPv4Address"),
    pyfixbuf.InfoElementSpec("sourceTransportPort"),
    pyfixbuf.InfoElementSpec("destinationTransportPort"),
    pyfixbuf.InfoElementSpec("protocolIdentifier"),
    pyfixbuf.InfoElementSpec("subTemplateMultiList")]

# add elements to template
tmpl.add_spec_list(import_list)

otmpl = pyfixbuf.Template(infomodel, True)

# create the collector

collector = pyfixbuf.Collector()

#create the IPFIX file to write to

collector.init_file(sys.argv[1])

#create the session

session = pyfixbuf.Session(infomodel)

# add the template as our internal template

session.add_internal_template(tmpl, 999)

session.add_internal_template(otmpl, otmpl.template_id)

# create the record to fill to export
# we can just give it the template since we don't have duplicate elements
#rec = pyfixbuf.Record(infomodel, tmpl)

rec = pyfixbuf.Record(infomodel)
rec.add_element("flowStartMilliseconds")
rec.add_element("packetTotalCount")
rec.add_element("reversePacketTotalCount")
rec.add_element_list(["sourceIPv4Address", "destinationIPv4Address", "sourceTransportPort", "destinationTransportPort", "protocolIdentifier", "subTemplateMultiList"])


#create the buffer for the collector
buf = pyfixbuf.Buffer(rec)

# make the buffer an export buffer
buf.init_collection(session, collector)

# set the internal template on the buffer
buf.set_internal_template(999)

# create a rec for our sub template multilist
# we use 2 basicLists so add elements separately
sub_rec = pyfixbuf.Record(infomodel)
sub_rec.add_element("subTemplateList")
sub_rec.add_element("httpUserAgent", pyfixbuf.BASICLIST)
sub_rec.add_element("httpServerString", pyfixbuf.BASICLIST)

# create a rec for our subtemplatelist

stl_rec = pyfixbuf.Record(infomodel)
stl_rec.add_element_list(["octetTotalCount", "reverseOctetTotalCount"])
stl_rec.add_element("sourceMacAddress", pyfixbuf.BASICLIST)

# open output file to write to

outFile = open(sys.argv[2], "w")

# use auto_insert setting or do the following to get information elements
# that are sent in IPFIX.

#buf.auto_insert()
tmpl_next = buf.next_template()

#set auto_insert or do this:
if ( tmpl_next.scope ):
    if (tmpl_next.type):
        srec = pyfixbuf.Record(infomodel, otmpl)
        buf.set_internal_template(otmpl.template_id)
        infotype = buf.next_record(srec)
        if (infotype != None):
            infotype = infotype.as_dict()
            infomodel.add_options_element(infotype)
            ie = infomodel.get_element("weird3byteElement")
            #print information about element?
#            print ie.id
#            print ie.ent
#            print ie.type
#            print ie.length
#            for key,value in infotype.items():
#                outFile.write(key + ": " + str(value) + "\n")
        # set the template back to the data template

buf.set_internal_template(999)

flowcount = 0

for data in buf:

    # convert the rec to a dictionary
    data = data.as_dict()
    for key,value in data.items():
        # don't print STML since it's just an object - see below
        if (key != "subTemplateMultiList"):
            outFile.write(key + ": " + str(value) + "\n")

    flowcount += 1
    # get data out of STML
    stml = data["subTemplateMultiList"]


    for entry in stml:
        # initialize this entry to the sub_rec we created above
        entry.set_record(sub_rec)
        for record in entry:
            #first item is a STL
            if "subTemplateList" in entry:
                stl = record["subTemplateList"]
                # set it to the stl_rec we created above then get entries
                if "octetTotalCount" in stl:
                    stl.set_record(stl_rec)
                    for item in stl:
                        item = item.as_dict()
                        for key,value in item.items():
                            outFile.write(key + ": " + str(value) + "\n")
                stl.clear()

            if "basicList" in entry:
                # now get the 2 basiclists - just printing the lists
                useragentlist = sub_rec["httpUserAgent"]
                count = 0
                for item in useragentlist:
                    outFile.write("httpUserAgent " + str(count) + " is " + item + "\n")
                    count += 1
                count = 0
                for item in sub_rec["httpServerString"]:
                    outFile.write("httpServerString " + str(count) + " is " + item + "\n")
                    count += 1
                # clear all fields in this record
                # clearing can also be done separately -
                # by calling stl.clear() and sub_rec.clear_basic_list on each BL
                sub_rec.clear_basic_list("httpUserAgent")
                sub_rec.clear_basic_list("httpServerString")

    stml.clear()
    outFile.write("------------------------------\n")

sys.stdout.write("Processed " + str(flowcount) + " flows \n")



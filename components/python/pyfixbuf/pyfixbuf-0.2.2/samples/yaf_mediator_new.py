#!/usr/bin/python
## ------------------------------------------------------------------------
## yaf_mediator_new.py
##
## Example of a true yaf mediator.  Can act as an IPFIX listener or file
## reader.  This will print to stdout or the provided text file.
## This script can handle any field that YAF exports.
## This script is different than yaf_mediator.py in that it doesn't define
## any of the templates or records before reading.
## ------------------------------------------------------------------------
## Copyright (C) 2014-2015 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## ------------------------------------------------------------------------
## See license information in LICENSE-OPENSOURCE.txt
## ------------------------------------------------------------------------


import sys
import os.path
import pyfixbuf
from netsa.data.times import *

# Test that the argument number is correct
if (len(sys.argv) < 2):
    print "Example Usage: yaf_mediator_new.py hostname [port] [protocol] [outfile]"
    print "OR: yaf_mediator_new.py infile.ipfix [outfile]"
    sys.exit()

infomodel = pyfixbuf.InfoModel()
infomodel.add_element_list(pyfixbuf.YAF_LIST)
infomodel.add_element_list(pyfixbuf.YAF_HTTP_LIST)
infomodel.add_element_list(pyfixbuf.YAF_DNS_LIST)
infomodel.add_element_list(pyfixbuf.YAF_SLP_LIST)
infomodel.add_element_list(pyfixbuf.YAF_SIP_LIST)
infomodel.add_element_list(pyfixbuf.YAF_SMTP_LIST)
infomodel.add_element_list(pyfixbuf.YAF_FTP_LIST)
infomodel.add_element_list(pyfixbuf.YAF_SSL_LIST)
infomodel.add_element_list(pyfixbuf.YAF_IMAP_LIST)
infomodel.add_element_list(pyfixbuf.YAF_RTSP_LIST)
infomodel.add_element_list(pyfixbuf.YAF_DPI_LIST)

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
    pyfixbuf.InfoElementSpec("sourceIPv6Address"),
    pyfixbuf.InfoElementSpec("destinationIPv6Address"),
    pyfixbuf.InfoElementSpec("sourceTransportPort"),
    pyfixbuf.InfoElementSpec("destinationTransportPort"),
    pyfixbuf.InfoElementSpec("flowAttributes"),
    pyfixbuf.InfoElementSpec("reverseFlowAttributes"),
    pyfixbuf.InfoElementSpec("protocolIdentifier"),
    pyfixbuf.InfoElementSpec("initialTCPFlags"),
    pyfixbuf.InfoElementSpec("reverseInitialTCPFlags"),
    pyfixbuf.InfoElementSpec("unionTCPFlags"),
    pyfixbuf.InfoElementSpec("reverseUnionTCPFlags"),
    pyfixbuf.InfoElementSpec("tcpSequenceNumber"),
    pyfixbuf.InfoElementSpec("vlanId"),
    pyfixbuf.InfoElementSpec("flowEndReason"),
    pyfixbuf.InfoElementSpec("silkAppLabel"),
    pyfixbuf.InfoElementSpec("subTemplateMultiList")]

stats_list = [
    pyfixbuf.InfoElementSpec("exportedFlowRecordTotalCount"),
    pyfixbuf.InfoElementSpec("packetTotalCount"),
    pyfixbuf.InfoElementSpec("droppedPacketTotalCount"),
    pyfixbuf.InfoElementSpec("ignoredPacketTotalCount")]


# add the elements we want in collection template
tmpl.add_spec_list(data_list)

# add stats elements we want from the incoming stats template
statstmpl.add_spec_list(stats_list)

# create the sessions for the collector & exporter
session = pyfixbuf.Session(infomodel)

# add the appropriate templates to each session
session.add_internal_template(tmpl, template_id=876)

session.add_internal_template(statstmpl, template_id=900)

collector = None
listener = None
transport = "tcp"
port = "18000"

# get the file from the command line arguments to read
if (os.path.isfile(sys.argv[1])):
    # create the collector
    collector = pyfixbuf.Collector()
    collector.init_file(sys.argv[1])
    if len(sys.argv) > 2:
        outFile = open(sys.argv[2], 'w')
    else:
        outFile = sys.stdout
else:
    if (len(sys.argv) > 3):
        transport = sys.argv[3]
    if (len(sys.argv) > 2):
        port = sys.argv[2]
    if (len(sys.argv) > 4):
        outFile = open(sys.argv[4], 'w')
    else:
        outFile = sys.stdout
    listener = pyfixbuf.Listener(session, hostname=sys.argv[1], 
                                 transport=transport.lower(), 
                                 port=sys.argv[2])


# create the input record from the input template since there are no duplicate elements
rec = pyfixbuf.Record(infomodel, tmpl)

# create buffers for input and export
if (collector):
    buf = pyfixbuf.Buffer(rec)

    buf.init_collection(session, collector)

# set the internal template on the input buffer
    buf.set_internal_template(876)

flowcount = 0

while (1):
    if (listener):
        buf = listener.wait()
        buf.set_record(rec)
        buf.set_internal_template(876)

    try:
        data = buf.next()
    except StopIteration:
        if (not(listener)):
            break
        else:
            continue

    flowcount += 1

    outFile.write("----flow: %d ----\n" % flowcount)

    for key,value in data.as_dict().items():
        if (key == "flowStartMilliseconds" or key == "flowEndMilliseconds"):
            outFile.write(key + ": " + str(make_datetime(value/1000)) + "\n")
        elif key != "subTemplateMultiList" and value:
            outFile.write(key + ": " + str(value) + "\n")
    # get items from STML
    stml = data["subTemplateMultiList"]
    for entry in stml:
        if "payload" in entry:
            # this is the payload template from YAF
            result = pyfixbuf.pyfix_hex_dump(entry["payload"], length=16)
            outFile.write(result + "\n")
            if "reversePayload" in entry:
                outFile.write("reverse payload: ")
                result = pyfixbuf.pyfix_hex_dump(entry["reversePayload"], length=16)
                outFile.write(result + "\n")
        elif "tcpSequenceNumber" in entry:
            for record in entry:
                record = record.as_dict()
                for key,value in record.items():
                    outFile.write(key + ": " + str(value) + "\n")
        elif "sourceMacAddress" in entry:
            outFile.write( "sourceMacAddress: " + entry["sourceMacAddress"] + "\n")
            outFile.write( "destinationMacAddress: " + entry["destinationMacAddress"] + "\n")
        elif entry.template_id == 0xC600 or entry.template_id == 0xC610:
            # this is the HTTP Template from YAF
            for http in entry:
                i = 0
                while i < len(http):
                    # http[i] is a BASICLIST - could iterate through it
                    if (len(http[i])):
                        outFile.write("\t" + http[i].element.name + ": " + str(http[i]) + "\n")
                    i += 1
                http.clear_all_lists()
        elif entry.template_id == 0xCA0A:
            numcert = 0
            for ssl in entry:
                for key,value in ssl.as_dict().items():
                    if (key != "subTemplateList" and key != "basicList"):
                        outFile.write("\t" + key + ": " + str(value) + "\n")
                bl = ssl["basicList"]
                outFile.write("\t" + str(bl) + "\n")
                stl = ssl["subTemplateList"]
                for cert in stl:
                    numcert += 1
                    outFile.write("\t == cert: %d ==\n" % numcert)
                    issuer = cert[0]
                    subject = cert[1]
                    outFile.write("\tsslcertvaliditynotbefore: " + cert["sslCertValidityNotBefore"] + "\n")
                    outFile.write("\tsslcertvaliditynotafter: " + cert["sslCertValidityNotAfter"] + "\n")
                    for issue in issuer:
                        outFile.write("\t" + str(issue["sslObjectType"]) + ": " + issue["sslObjectValue"] + "\n")
                    issue.clear()
                    for sub in subject:
                        outFile.write( "\t" +  str(sub["sslObjectType"]) + ": " + sub["sslObjectValue"] + "\n")
                    subject.clear()
                stl.clear()
        elif entry.template_id == 0xCE00:
            # this is the DNS template from YAF
            for record in entry:
                stl = record["subTemplateList"]
                for dnsrec in stl:
                    dnsdict = dnsrec.as_dict()
                    for key,value in dnsdict.items():
                        if (key != "subTemplateList"):
                            outFile.write("\t" +  key + ": " + str(value) + "\n")
                        else:
                            stl2 = dnsrec["subTemplateList"]
                            if (dnsrec['dnsQRType'] == 1):
                                for arec in stl2:
                                    outFile.write("\tA record: " + arec["sourceIPv4Address"] + "\n")
                            elif (dnsrec['dnsQRType'] == 2):
                                for ns in stl2:
                                    outFile.write( "\tNS Record: " + ns["dnsNSDName"] + "\n")
                            elif (dnsrec['dnsQRType'] == 5):
                                for cn in stl2:
                                    outFile.write( "\tCNAME rec: " + cn['dnsCName'] + "\n")
                            elif (dnsrec['dnsQRType'] == 12):
                                for cn in stl2:
                                    outFile.write( "\tPTR rec: " + cn['dnsPTRDName'] + "\n")
                            elif (dnsrec['dnsQRType'] == 15):
                                for mx in stl2:
                                    mxdict = mx.as_dict()
                                    for key, value in mxdict.items():
                                        outFile.write( "\t" +  key + ": " + str(value) + "\n")
                            elif (dnsrec['dnsQRType'] == 16):
                                for cn in stl2:
                                    outFile.write( "\tTXT rec: " +cn['dnsTXTData'] + "\n")
                            elif (dnsrec['dnsQRType'] == 28):
                                for cn in stl2:
                                    outFile.write( "\tAAAA rec: " + cn['sourceIPv6Address'] + "\n")
                            elif (dnsrec['dnsQRType'] == 6):
                                for soa in stl2:
                                    soadict = soa.as_dict()
                                    for (key, value) in soadict.items():
                                        outFile.write("\t" +  key + ": " + str(value) + "\n")
                            stl2.clear()
                    outFile.write("\t----\n")
                stl.clear()
        elif entry.template_id == 0xC200:
             # this is IRC
             for irc in entry:
                 for (key,value) in irc.as_dict().items():
                     outFile.write( "\t" +  key + ": " + str(value) + "\n")
                 irc.clear_all_lists()
        elif entry.template_id == 0xC300:
            # pop3
            for pop in entry:
                for (key,value) in pop.as_dict().items():
                    outFile.write( "\t" +  key + ": " + str(value) + "\n")
                pop.clear_all_lists()
        elif entry.template_id == 0xC500:
            #slp
            for slp in entry:
                for (key,value) in slp.as_dict().items():
                    outFile.write( "\t" +  key + ": " + str(value) + "\n")
        elif entry.template_id == 0xC700:
            #ftp
            for ftp in entry:
                for (key, value) in ftp.as_dict().items():
                    outFile.write( "\t" + str(key) + ": " + str(value) + "\n")
                ftp.clear_all_lists() 
        elif entry.template_id == 0xC800:
            #imap
            for imap in entry:
                for (key, value) in imap.as_dict().items():
                    outFile.write( "\t" + str(key) + ": " + str(value) + "\n")
                imap.clear_all_lists()
        elif entry.template_id == 0xC900:
            #rtsp
            for rtsp in entry:
                for (key, value) in rtsp.as_dict().items():
                    outFile.write( "\t" + key + ": " + str(value) + "\n")
                rtsp.clear_all_lists()
        elif entry.template_id == 0xCA00:
            #sip                     
            for sip in entry:
                for (key, value) in sip.as_dict().items():
                    outFile.write( "\t" + key + ": " + str(value) + "\n")
                sip.clear_all_lists()
        elif entry.template_id == 0xCB00:
            #smtp 
            for smtp in entry:
                for (key, value) in smtp.as_dict().items():
                    outFile.write( "\t" + str(key) + ": " + str(value) + "\n")
                smtp.clear_all_lists()
        elif entry.template_id == 0xCC00:
            #ssh
            for ssh in entry:
                for (key, value) in ssh.as_dict().items():
                    outFile.write( "\t" + key + ": " + str(value) + "\n")
                ssh.clear_all_lists()
        elif "tftpFilename" in entry:
            for tftp in entry:
                for (key, value) in tftp.as_dict().items():
                    outFile.write( "\t" + key + ": " + str(value) + "\n")
        elif entry.template_id == 0xC202:
            stl = entry["subTemplateList"]
            for ent in stl:
                dnp = ent.as_dict()
                outFile.write("\t DNP3 srcAddress: %d\n" % dnp["dnp3SourceAddress"])
                outFile.write("\t DNP3 dstAddress: %d\n" % dnp["dnp3DestinationAddress"])
                outFile.write("\t DNP3 function: %d\n" % dnp["dnp3Function"])
                outFile.write("\t DNP3 object: \n")
                result = pyfixbuf.pyfix_hex_dump(dnp["dnp3ObjectData"], length=16)
                outFile.write( result + "\n")
            stl.clear()
        elif entry.template_id == 0xC204:
            md = entry["modbusData"]
            for i in md:
                outFile.write("modbusData:\n")
                result = pyfixbuf.pyfix_hex_dump(i, length=16)
                outFile.write( result + "\n")
            md.clear()
        elif entry.template_id == 0xC205:
            for enip in entry["ethernetIPData"]:
                outFile.write("Ethernet/IP Data:" + "\n")
                result = pyfixbuf.pyfix_hex_dump(enip, length=16)
                outFile.write(result + "\n")
            entry["ethernetIPData"].clear()
        elif entry.template_id == 0xC206:
            for rtp in entry:
                for (key, value) in rtp.as_dict().items():
                    outFile.write("\t" + key + ": " + str(value) + "\n")
        elif entry.template_id == 0xCD00:
            for nntp in entry:
                for (key, value) in nntp.as_dict().items():
                    outFile.write("\t" + key + ": " + str(value) + "\n")
        elif entry.template_id == 0xCE0C:
            stl = entry["subTemplateList"] 
            stl.set_record(mysqlinner)
            outFile.write("\t MySQL user name: %s\n" % entry["mysqlUsername"])
            for ent in stl:
                my = ent.as_dict()
                outFile.write("\t MySQL code: %d: %s\n" % (my["mysqlCommandCode"], my["mysqlCommandText"]))
            stl.clear()

    # clear the stml
    stml.clear()

# check to see if the next template is a stats template
    tmpl_next = buf.next_template()

    if ( tmpl_next.scope ):
        # set the interal template to the stats template
        buf.set_internal_template(900)
        # get the record
        stats = buf.next_record()
        #if (stats != None):
        stats = stats.as_dict()
        # print all the items in stats
        for key,value in stats.items():
            outFile.write(key + ": " + str(value) + "\n")        
            # print all the items in stats
        # set the template back to the data template
        buf.set_internal_template(876)


sys.stdout.write("Processed " + str(flowcount) + " flows \n")

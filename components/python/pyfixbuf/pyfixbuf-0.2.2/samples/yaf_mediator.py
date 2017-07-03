#!/usr/bin/python
## ------------------------------------------------------------------------
## yaf_mediator.py
##
## Example of a true yaf mediator.  Reads YAF IPFIX files and exports
## a modified version (subset) to an IPFIX file.  
## See exporter_list for the modified template.
## ------------------------------------------------------------------------
## Copyright (C) 2014 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## ------------------------------------------------------------------------
## See license information in LICENSE-OPENSOURCE.txt
## ------------------------------------------------------------------------


import sys
import pyfixbuf
from netsa.data.times import *

# Test that the argument number is correct
if (len(sys.argv) != 2):
    print "Must supply an IPFIX file to read"
    print "Usage: sample_mediator.py infile.ipfix"
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

# create the collector
collector = pyfixbuf.Collector()

# get the file from the command line arguments to read
collector.init_file(sys.argv[1])

# create the sessions for the collector & exporter
session = pyfixbuf.Session(infomodel)

# add the appropriate templates to each session
session.add_internal_template(tmpl, template_id=876)

session.add_internal_template(statstmpl, template_id=900)

# create the input record from the input template since there are no duplicate elements
rec = pyfixbuf.Record(infomodel, tmpl)

# create buffers for input and export
buf = pyfixbuf.Buffer(rec)
buf.init_collection(session, collector)

# set the internal template on the input buffer
buf.set_internal_template(876)

# this record is found in the subtemplatemultilist coming from YAF
payrec = pyfixbuf.Record(infomodel)
payrec.add_element("payload")
payrec.add_element("reversePayload")

macrec = pyfixbuf.Record(infomodel)
macrec.add_element("sourceMacAddress")
macrec.add_element("destinationMacAddress")

tcprec = pyfixbuf.Record(infomodel)
tcp_elements = ["tcpSequenceNumber", "initialTCPFlags", "unionTCPFlags",
                "reverseInitialTCPFlags", "reverseUnionTCPFlags", 
                "reverseTcpSequenceNumber"]
tcprec.add_element_list(tcp_elements)


# this record is also found in the subtemplatemultilist coming from YAF
httprec = pyfixbuf.Record(infomodel)
httprec.add_element("httpServerString", pyfixbuf.BASICLIST)
httprec.add_element("httpUserAgent", pyfixbuf.BASICLIST)
httprec.add_element("httpGet", pyfixbuf.BASICLIST)
httprec.add_element("httpConnection", pyfixbuf.BASICLIST)
httprec.add_element("httpReferer", pyfixbuf.BASICLIST)
httprec.add_element("httpLocation", pyfixbuf.BASICLIST)
httprec.add_element("httpHost", pyfixbuf.BASICLIST)
httprec.add_element("httpContentLength", pyfixbuf.BASICLIST)
httprec.add_element("httpAge", pyfixbuf.BASICLIST)
httprec.add_element("httpResponse", pyfixbuf.BASICLIST)
httprec.add_element("httpAcceptLanguage", pyfixbuf.BASICLIST)
httprec.add_element("httpAccept", pyfixbuf.BASICLIST)
httprec.add_element("httpContentType", pyfixbuf.BASICLIST)
httprec.add_element("httpVersion", pyfixbuf.BASICLIST)
httprec.add_element("httpCookie", pyfixbuf.BASICLIST)
httprec.add_element("httpSetCookie", pyfixbuf.BASICLIST)
httprec.add_element("httpAuthorization", pyfixbuf.BASICLIST)
httprec.add_element("httpVia", pyfixbuf.BASICLIST)
httprec.add_element("httpX-Forwarded-For", pyfixbuf.BASICLIST)
httprec.add_element("httpRefresh", pyfixbuf.BASICLIST)

#imap record
imaprec = pyfixbuf.Record(infomodel)
imaprec.add_element("imapCapability", pyfixbuf.BASICLIST)
imaprec.add_element("imapLogin", pyfixbuf.BASICLIST)
imaprec.add_element("imapStartTLS", pyfixbuf.BASICLIST)
imaprec.add_element("imapAuthenticate", pyfixbuf.BASICLIST)
imaprec.add_element("imapCommand", pyfixbuf.BASICLIST)
imaprec.add_element("imapExists", pyfixbuf.BASICLIST)
imaprec.add_element("imapRecent", pyfixbuf.BASICLIST)

#rtsp record
rtsprec = pyfixbuf.Record(infomodel)
rtsprec.add_element("rtspURL", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspVersion", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspReturnCode", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspContentLength", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspCommand", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspContentType", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspTransport", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspCSeq", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspLocation", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspPacketsReceived", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspUserAgent", pyfixbuf.BASICLIST)
rtsprec.add_element("rtspJitter", pyfixbuf.BASICLIST)

#siprec
siprec = pyfixbuf.Record(infomodel)
siprec.add_element("sipInvite", pyfixbuf.BASICLIST)
siprec.add_element("sipCommand", pyfixbuf.BASICLIST)
siprec.add_element("sipVia", pyfixbuf.BASICLIST)
siprec.add_element("sipMaxForwards", pyfixbuf.BASICLIST)
siprec.add_element("sipAddress", pyfixbuf.BASICLIST)
siprec.add_element("sipContentLength", pyfixbuf.BASICLIST)
siprec.add_element("sipUserAgent", pyfixbuf.BASICLIST)

#smtprec
smtprec = pyfixbuf.Record(infomodel)
smtprec.add_element("smtpHello", pyfixbuf.BASICLIST)
smtprec.add_element("smtpFrom", pyfixbuf.BASICLIST)
smtprec.add_element("smtpTo", pyfixbuf.BASICLIST)
smtprec.add_element("smtpContentType", pyfixbuf.BASICLIST)
smtprec.add_element("smtpSubject", pyfixbuf.BASICLIST)
smtprec.add_element("smtpFilename", pyfixbuf.BASICLIST)
smtprec.add_element("smtpContentDisposition", pyfixbuf.BASICLIST)
smtprec.add_element("smtpResponse", pyfixbuf.BASICLIST)
smtprec.add_element("smtpEnhanced", pyfixbuf.BASICLIST)
smtprec.add_element("smtpSize", pyfixbuf.BASICLIST)
smtprec.add_element("smtpDate", pyfixbuf.BASICLIST)

#ftprec
ftprec = pyfixbuf.Record(infomodel)
ftprec.add_element("ftpReturn", pyfixbuf.BASICLIST)
ftprec.add_element("ftpUser", pyfixbuf.BASICLIST)
ftprec.add_element("ftpPass", pyfixbuf.BASICLIST)
ftprec.add_element("ftpType", pyfixbuf.BASICLIST)
ftprec.add_element("ftpRespCode", pyfixbuf.BASICLIST)

#sshrec
sshrec = pyfixbuf.Record(infomodel)
sshrec.add_element("sshVersion", pyfixbuf.BASICLIST)

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
dnsA.add_element("sourceIPv4Address")

dnsNS = pyfixbuf.Record(infomodel)
dnsNS.add_element("dnsNSDName")

dnsCN = pyfixbuf.Record(infomodel)
dnsCN.add_element("dnsCName")

dnsSOA = pyfixbuf.Record(infomodel)
dnsSOA.add_element("dnsSOAMName")
dnsSOA.add_element("dnsSOARName")
dnsSOA.add_element("dnsSOASerial")
dnsSOA.add_element("dnsSOARefresh")
dnsSOA.add_element("dnsSOARetry")
dnsSOA.add_element("dnsSOAExpire")
dnsSOA.add_element("dnsSOAMinimum")

dnsMX = pyfixbuf.Record(infomodel)
dnsMX.add_element("dnsMXExchange")
dnsMX.add_element("dnsMXPreference")

dnsPTR = pyfixbuf.Record(infomodel)
dnsPTR.add_element("dnsPTRDName")

dnsTXT = pyfixbuf.Record(infomodel)
dnsTXT.add_element("dnsTXTData")

dnsAAAA = pyfixbuf.Record(infomodel)
dnsAAAA.add_element("sourceIPv6Address")

dnsSRV = pyfixbuf.Record(infomodel)
dnsSRV.add_element("dnsSRVTarget")
dnsSRV.add_element("dnsSRVPriority")
dnsSRV.add_element("dnsSRVWeight")
dnsSRV.add_element("dnsSRVPort")

sslrec = pyfixbuf.Record(infomodel)
sslrec.add_element("sslCipher", pyfixbuf.BASICLIST)
sslrec.add_element("sslServerCipher")
sslrec.add_element("sslClientVersion")
sslrec.add_element("sslCompressionMethod")
sslrec.add_element("sslRecordVersion")
sslrec.add_element("sslCertList", pyfixbuf.SUBTEMPLATELIST)

sslcertrec = pyfixbuf.Record(infomodel)
sslcertrec.add_element("issuerList", pyfixbuf.SUBTEMPLATELIST)
sslcertrec.add_element("subjectList", pyfixbuf.SUBTEMPLATELIST)
sslcertrec.add_element("extensions", pyfixbuf.SUBTEMPLATELIST)
sslcertrec.add_element("sslCertSignature")
sslcertrec.add_element("sslCertSerialNumber")
sslcertrec.add_element("start", 0, "sslCertValidityNotBefore")
sslcertrec.add_element("end", 0, "sslCertValidityNotAfter")
sslcertrec.add_element("sslPublicKeyAlgorithm")
sslcertrec.add_element("sslPublicKeyLength")
sslcertrec.add_element("sslCertVersion")

issuerlist = pyfixbuf.Record(infomodel)
issuerlist.add_element("sslObjectValue")
issuerlist.add_element("sslObjectType")

subjectlist = pyfixbuf.Record(infomodel)
subjectlist.add_element("sslObjectValue")
subjectlist.add_element("sslObjectType")

ircrec = pyfixbuf.Record(infomodel)
ircrec.add_element("ircTextMessage", pyfixbuf.BASICLIST)

pop3rec = pyfixbuf.Record(infomodel)
pop3rec.add_element("pop3TextMessage", pyfixbuf.BASICLIST)

slprec = pyfixbuf.Record(infomodel)
slprec.add_element("slpString", pyfixbuf.BASICLIST)
slprec.add_element("slpVersion")
slprec.add_element("slpMessageType")

tftprec = pyfixbuf.Record(infomodel)
tftprec.add_element("tftpFilename")
tftprec.add_element("tftpMode")

nntprec = pyfixbuf.Record(infomodel)
nntprec.add_element("nntpResponse", pyfixbuf.BASICLIST)
nntprec.add_element("nntpCommand", pyfixbuf.BASICLIST)

# create the stats rec
statrec = pyfixbuf.Record(infomodel, statstmpl)

modbusrec = pyfixbuf.Record(infomodel)
modbusrec.add_element("modbusData", pyfixbuf.BASICLIST)

dnprec = pyfixbuf.Record(infomodel)
dnprec.add_element("subTemplateList")

dnpinner = pyfixbuf.Record(infomodel)
dnpinner.add_element("dnp3SourceAddress")
dnpinner.add_element("dnp3DestinationAddress")
dnpinner.add_element("dnp3Function")
dnpinner.add_element("paddingOctets", length=3)
dnpinner.add_element("dnp3ObjectData")

ethip = pyfixbuf.Record(infomodel)
ethip.add_element("ethernetIPData", pyfixbuf.BASICLIST)

rtprec = pyfixbuf.Record(infomodel)
rtprec.add_element("rtpPayloadType")
rtprec.add_element("reverseRtpPayloadType")

mysqlrec = pyfixbuf.Record(infomodel)
mysqlrec.add_element("subTemplateList")
mysqlrec.add_element("mysqlUsername")
mysqlinner = pyfixbuf.Record(infomodel)
mysqlinner.add_element("mysqlCommandText")
mysqlinner.add_element("mysqlCommandCode")

flowcount = 0

for data in buf:

    flowcount += 1

    print "----flow: %d ----" % flowcount
    
    for key,value in data.as_dict().items():
        if (key == "flowStartMilliseconds" or key == "flowEndMilliseconds"):
            print key + ": " + str(make_datetime(value/1000))
        elif key != "subTemplateMultiList" and value:
            print key + ": " + str(value)
    # get items from STML
    stml = data["subTemplateMultiList"]
    for entry in stml:
        if "payload" in entry:
            # this is the payload template from YAF
            entry.set_record(payrec)
            print "payload: "
            result = pyfixbuf.pyfix_hex_dump(entry["payload"], length=16)
            print result
            if "reversePayload" in entry:
                print "reverse payload: "
                result = pyfixbuf.pyfix_hex_dump(entry["reversePayload"], length=16)
                print result
        elif "tcpSequenceNumber" in entry:
            entry.set_record(tcprec)
            for record in entry:
                record = record.as_dict()
                for key,value in record.items():
                    print key + ": " + str(value)
        elif "sourceMacAddress" in entry:
            entry.set_record(macrec)
            print "sourceMacAddress: " + entry["sourceMacAddress"]
            print "destinationMacAddress: " + entry["destinationMacAddress"]
        elif entry.template_id == 0xC600 or entry.template_id == 0xC610:
            # this is the HTTP Template from YAF
            entry.set_record(httprec)
            for http in entry:
                httpish = http.as_dict()
                for key,value in http.as_dict().items():
                    if (len(value)):
                        print "\t" + key + ": " + str(value)
                http.clear_all_lists()
        elif entry.template_id == 0xCA0A:
            entry.set_record(sslrec)
            numcert = 0
            for ssl in entry:
                for key,value in ssl.as_dict().items():
                    if (key != "sslCertList" and key != "sslCipher"):
                        print "\t" + key + ": " + str(value)
                bl = ssl["sslCipher"]
                print "\t" + str(bl)
                stl = ssl["sslCertList"]
                stl.set_record(sslcertrec)
                for cert in stl:
                    numcert += 1
                    print "\t == cert: %d ==" % numcert
                    issuer = sslcertrec["issuerList"]
                    issuer.set_record(issuerlist)
                    subject = sslcertrec["subjectList"]
                    subject.set_record(subjectlist)
                    print "\t" +  "sslcertvaliditynotbefore: " + cert["start"]
                    print "\t" +  "sslcertvaliditynotafter: " + cert["end"]
                    for issue in issuer:
                        print "\t" +  str(issue["sslObjectType"]) + ": " + issue["sslObjectValue"]
                    issue.clear()
                    for sub in subject:
                        print "\t" +  str(sub["sslObjectType"]) + ": " + sub["sslObjectValue"]
                    subject.clear()
                stl.clear()
        elif entry.template_id == 0xCE00:
            # this is the DNS template from YAF
            entry.set_record(dnsouter)
            for record in entry:
                stl = dnsouter["dnsQRList"]
                stl.set_record(dnsinner)
                for dnsrec in stl:
                    dnsdict = dnsrec.as_dict()
                    for key,value in dnsdict.items():
                        if (key != "dnsList"):
                            print "\t" +  key + ": " + str(value)
                        else:
                            stl2 = dnsrec['dnsList']
                            if (dnsrec['dnsQRType'] == 1):
                                stl2.set_record(dnsA)
                                for arec in stl2:
                                    print "\t" +  "A record: " + arec["sourceIPv4Address"]
                            elif (dnsrec['dnsQRType'] == 2):
                                stl2.set_record(dnsNS)
                                for ns in stl2:
                                    print "\t" +  "NS Record: " + ns["dnsNSDName"]
                            elif (dnsrec['dnsQRType'] == 5):
                                stl2.set_record(dnsCN)
                                for cn in stl2:
                                    print "\t" +  "CNAME rec: " + cn['dnsCName']
                            elif (dnsrec['dnsQRType'] == 12):
                                stl2.set_record(dnsPTR)
                                for cn in stl2:
                                    print "\t" +  "PTR rec: " + cn['dnsPTRDName']
                            elif (dnsrec['dnsQRType'] == 15):
                                stl2.set_record(dnsMX)
                                for mx in stl2:
                                    mxdict = mx.as_dict()
                                    for key, value in mxdict.items():
                                        print "\t" +  key + ": " + str(value)
                            elif (dnsrec['dnsQRType'] == 16):
                                stl2.set_record(dnsTXT)
                                for cn in stl2:
                                    print "\t" +  "TXT rec: " +cn['dnsTXTName']
                            elif (dnsrec['dnsQRType'] == 28):
                                stl2.set_record(dnsAAAA)
                                for cn in stl2:
                                    print "\t" +  "AAAA rec: " + cn['sourceIPv6Address']
                            elif (dnsrec['dnsQRType'] == 6):
                                stl2.set_record(dnsSOA)
                                for soa in stl2:
                                    soadict = soa.as_dict()
                                    for (key, value) in soadict.items():
                                        print "\t" +  key + ": " + str(value)
                            stl2.clear()
                    print "\t" +  "----"
                stl.clear()
        elif entry.template_id == 0xC200:
             # this is IRC
             entry.set_record(ircrec)
             for irc in entry:
                 for (key,value) in irc.as_dict().items():
                     print "\t" +  key + ": " + str(value)
                 irc.clear_all_lists()
        elif entry.template_id == 0xC300:
            # pop3
            entry.set_record(pop3rec)
            for pop in entry:
                for (key,value) in pop.as_dict().items():
                    print "\t" +  key + ": " + str(value)
                pop.clear_all_lists()
        elif entry.template_id == 0xC500:
            #slp
            entry.set_record(slprec)
            for slp in entry:
                for (key,value) in slp.as_dict().items():
                    print "\t" +  key + ": " + str(value)
        elif entry.template_id == 0xC700:
            #ftp
            entry.set_record(ftprec)
            for ftp in entry:
                for (key, value) in ftp.as_dict().items():
                    print "\t" + key + ": " + str(value)
                ftp.clear_all_lists() 
        elif entry.template_id == 0xC800:
            #imap
            entry.set_record(imaprec)
            for imap in entry:
                for (key, value) in imap.as_dict().items():
                    print "\t" + key + ": " + str(value)
                imap.clear_all_lists()
        elif entry.template_id == 0xC900:
            #rtsp
            entry.set_record(rtsprec)
            for rtsp in entry:
                for (key, value) in rtsp.as_dict().items():
                    print "\t" + key + ": " + str(value)
                rtsp.clear_all_lists()
        elif entry.template_id == 0xCA00:
            #sip                     
            entry.set_record(siprec)
            for sip in entry:
                for (key, value) in sip.as_dict().items():
                    print "\t" + key + ": " + str(value)
                sip.clear_all_lists()
        elif entry.template_id == 0xCB00:
            #smtp 
            entry.set_record(smtprec)
            for smtp in entry:
                for (key, value) in smtp.as_dict().items():
                    print "\t" + key + ": " + str(value)
                smtp.clear_all_lists()
        elif entry.template_id == 0xCC00:
            #ssh
            entry.set_record(sshrec)
            for ssh in entry:
                for (key, value) in ssh.as_dict().items():
                    print "\t" + key + ": " + str(value)
                ssh.clear_all_lists()
        elif "tftpFilename" in entry:
            entry.set_record(tftprec)
            for tftp in entry:
                for (key, value) in tftp.as_dict().items():
                    print "\t" + key + ": " + str(value)
        elif entry.template_id == 0xC202:
            entry.set_record(dnprec)
            stl = entry["subTemplateList"]
            stl.set_record(dnpinner)
            for ent in stl:
                dnp = ent.as_dict()
                print "\t DNP3 srcAddress: %d" % dnp["dnp3SourceAddress"]
                print "\t DNP3 dstAddress: %d" % dnp["dnp3DestinationAddress"]
                print "\t DNP3 function: %d" % dnp["dnp3Function"]
                print "\t DNP3 object: "
                result = pyfixbuf.pyfix_hex_dump(dnp["dnp3ObjectData"], length=16)
                print result
            stl.clear()
        elif entry.template_id == 0xC204:
            entry.set_record(modbusrec)
            md = entry["modbusData"]
            for i in md:
                print "modbusData:"
                result = pyfixbuf.pyfix_hex_dump(i, length=16)
                print result
            md.clear()
        elif entry.template_id == 0xC205:
            entry.set_record(ethip)
            for enip in entry["ethernetIPData"]:
                print "Ethernet/IP Data:"
                result = pyfixbuf.pyfix_hex_dump(enip, length=16)
                print result
            entry["ethernetIPData"].clear()
        elif entry.template_id == 0xC206:
            entry.set_record(rtprec)
            for rtp in entry:
                for (key, value) in rtp.as_dict().items():
                    print "\t" + key + ": " + str(value)
        elif entry.template_id == 0xCD00:
            entry.set_record(nntprec)
            for nntp in entry:
                for (key, value) in nntp.as_dict().items():
                    print "\t" + key + ": " + str(value)
        elif entry.template_id == 0xCE0C:
            entry.set_record(mysqlrec)
            stl = entry["subTemplateList"] 
            stl.set_record(mysqlinner)
            print "\t MySQL user name: %s" % entry["mysqlUsername"]
            for ent in stl:
                my = ent.as_dict()
                print "\t MySQL code: %d: %s" % (my["mysqlCommandCode"], my["mysqlCommandText"])
            stl.clear()

    # clear the stml
    stml.clear()

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


sys.stdout.write("Processed " + str(flowcount) + " flows \n")

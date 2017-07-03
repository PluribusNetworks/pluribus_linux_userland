#########################################################################
# Copyright 2013-2015 by Carnegie Mellon University
#
# @OPENSOURCE_HEADER_START@
# Use of the Network Situational Awareness Python support library and
# related source code is subject to the terms of the following licenses:
#
# GNU Lesser Public License (LGPL) Rights pursuant to Version 2.1, February 1999
# Government Purpose License Rights (GPLR) pursuant to DFARS 252.227.7013
#
# NO WARRANTY
#
# ANY INFORMATION, MATERIALS, SERVICES, INTELLECTUAL PROPERTY OR OTHER
# PROPERTY OR RIGHTS GRANTED OR PROVIDED BY CARNEGIE MELLON UNIVERSITY
# PURSUANT TO THIS LICENSE (HEREINAFTER THE "DELIVERABLES") ARE ON AN
# "AS-IS" BASIS. CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY
# KIND, EITHER EXPRESS OR IMPLIED AS TO ANY MATTER INCLUDING, BUT NOT
# LIMITED TO, WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE,
# MERCHANTABILITY, INFORMATIONAL CONTENT, NONINFRINGEMENT, OR ERROR-FREE
# OPERATION. CARNEGIE MELLON UNIVERSITY SHALL NOT BE LIABLE FOR INDIRECT,
# SPECIAL OR CONSEQUENTIAL DAMAGES, SUCH AS LOSS OF PROFITS OR INABILITY
# TO USE SAID INTELLECTUAL PROPERTY, UNDER THIS LICENSE, REGARDLESS OF
# WHETHER SUCH PARTY WAS AWARE OF THE POSSIBILITY OF SUCH DAMAGES.
# LICENSEE AGREES THAT IT WILL NOT MAKE ANY WARRANTY ON BEHALF OF
# CARNEGIE MELLON UNIVERSITY, EXPRESS OR IMPLIED, TO ANY PERSON
# CONCERNING THE APPLICATION OF OR THE RESULTS TO BE OBTAINED WITH THE
# DELIVERABLES UNDER THIS LICENSE.
#
# Licensee hereby agrees to defend, indemnify, and hold harmless Carnegie
# Mellon University, its trustees, officers, employees, and agents from
# all claims or demands made against them (and any related losses,
# expenses, or attorney's fees) arising out of, or relating to Licensee's
# and/or its sub licensees' negligent use or willful misuse of or
# negligent conduct or willful misconduct regarding the Software,
# facilities, or other rights or assistance granted by Carnegie Mellon
# University under this License, including, but not limited to, any
# claims of product liability, personal injury, death, damage to
# property, or violation of any laws or regulations.
#
# Carnegie Mellon University Software Engineering Institute authored
# documents are sponsored by the U.S. Department of Defense under
# Contract FA8721-05-C-0003. Carnegie Mellon University retains
# copyrights in all material produced under this contract. The U.S.
# Government retains a non-exclusive, royalty-free license to publish or
# reproduce these documents, or allow others to do so, for U.S.
# Government purposes only pursuant to the copyright license under the
# contract clause at 252.227.7013.
#
# @OPENSOURCE_HEADER_END@
#
#######################################################################
import warnings
import _pyfixbuf
import ipaddress
import struct
import sys

__all__ = ['InfoElement', 'InfoModel', 'Buffer',
           'Session', 'Exporter', 'Collector',
           'Template', 'InfoElementSpec', 'Record',
           'STML', 'STL', 'BL', 'CERT_PEN', 'VARLEN',
           'BASICLIST', 'Listener',
           'SUBTEMPLATELIST',
           'SUBTEMPLATEMULTILIST', 'ENDIAN',
           'REVERSIBLE', 'DataType', 'Units',
           'Semantic']

CERT_PEN = 6871
VARLEN = 65535
ENDIAN = 1
REVERSIBLE = 64
IPV6 = 6

def enum(*seq, **named):
    enums = dict(zip(seq, range(len(seq))), **named)
    return type('Enum', (), enums)


DataType = enum("OCTET_ARRAY", "UINT8", "UINT16", "UINT32", "UINT64", "INT8",
                 "INT16", "INT32", "INT64", "FLOAT32", "FLOAT64", "BOOL",
                 "MAC_ADDR", "STRING", "SECONDS", "MILLISECONDS",
                 "MICROSECONDS", "NANOSECONDS", "IP4ADDR", "IP6ADDR",
                 "BASIC_LIST", "SUB_TMPL_LIST", "SUB_TMPL_MULTI_LIST")

Units = enum("NONE", "BITS", "OCTETS", "PACKETS", "FLOWS", "SECONDS",
             "MILLISECONDS", "MICROSECONDS", "NANOSECONDS", "WORDS",
             "MESSAGES", "HOPS", "ENTRIES")

Semantic = enum("DEFAULT", "QUANTITY", "TOTALCOUNTER", "DELTACOUNTER",
                "IDENTIFIER", "FLAGS", "LIST")

BASICLIST = DataType.BASIC_LIST
SUBTEMPLATELIST = DataType.SUB_TMPL_LIST
SUBTEMPLATEMULTILIST = DataType.SUB_TMPL_MULTI_LIST

def typeToLength(dt):
    if ((dt == DataType.OCTET_ARRAY) or (dt == DataType.STRING) or
        (dt == DataType.BASIC_LIST) or (dt == DataType.SUB_TMPL_LIST) or
        (dt == DataType.SUB_TMPL_MULTI_LIST)):
        return VARLEN
    elif ((dt == DataType.UINT8) or (dt == DataType.INT8) or
          (dt == DataType.BOOL)):
        return 1
    elif ((dt == DataType.UINT16) or (dt == DataType.INT16)):
        return 2
    elif ((dt == DataType.UINT32) or (dt ==DataType.INT32) or
          (dt == DataType.SECONDS) or (dt == DataType.FLOAT32) or
          (dt == DataType.IP4ADDR)):
        return 4
    elif (dt == DataType.MAC_ADDR):
        return 6
    elif ((dt == DataType.UINT64) or (dt == DataType.INT64) or
          (dt == DataType.FLOAT64) or (dt == DataType.MILLISECONDS)or
          (dt == DataType.MICROSECONDS) or (dt == DataType.NANOSECONDS)):
        return 8
    elif (dt == DataType.IP6ADDR):
        return 16
    else:
        return VARLEN


def lenToType(len, dt):
    signed = False
    float = False
    if dt in [DataType.INT8, DataType.INT16, DataType.INT32, DataType.INT64]:
        signed = True
    elif ((dt == DataType.FLOAT32) or (dt == DataType.FLOAT64)):
        float = True

    if (len == 1):
        if signed:
            return DataType.INT8
        else:
            return DataType.UINT8
    elif (len == 2):
        if signed:
            return DataType.INT16
        else:
            return DataType.UINT16
    elif (len <= 4):
        if signed:
            return DataType.INT32
        elif float:
            return DataType.FLOAT32
        else:
            return DataType.UINT32
    elif (len <= 8):
        if signed:
            return DataType.INT64
        elif float:
            return DataType.FLOAT64
        else:
            return DataType.UINT64


def verifyModifiedLength(dt):

    if ((dt == DataType.BOOL) or (dt == DataType.IP6ADDR) or
        (dt == DataType.SECONDS) or (dt == DataType.MILLISECONDS) or
        (dt == DataType.IP4ADDR) or (dt == DataType.MICROSECONDS) or
        (dt == DataType.NANOSECONDS) or (dt == DataType.UINT8) or
        (dt == DataType.INT8) or (dt == DataType.BASIC_LIST) or
        (dt == DataType.MAC_ADDR) or (dt == DataType.FLOAT32) or
        (dt == DataType.SUB_TMPL_LIST) or (dt == DataType.SUB_TMPL_MULTI_LIST)):
        return False
    else:
        return True

def macToHex(x):
    try:
        x = x.replace(':', '').decode('hex')
    except:
        pass
    x = bytearray(x)
    return x

class InfoElement(_pyfixbuf.fbInfoElementBase):

    """
    Creates a new Information Element (IE) using the given *name*,
    *enterprise_number*, and *id*, and optional *length*, *reversible* flag,
    *endian* flags, *datatype*, *units*, *min*, *max*, *semantic*, and
    *description*.  An Information Element identifies
    a type of data to be stored and transmitted via IPFIX.

    If no *length* is provided, the IE is defined as having a variable
    length.  All Strings should be variable length.

    If *endian* is set, the IE is assumed to be an integer and will be
    converted to and from network byte order upon transcoding.

    If *reversible* is set, a second IE is created for the same information
    in the reverse direction. [The reversed IE's name is the same *name*, but
    with ``reverse`` prepended.]

    If *type* is set, pyfixbuf will know how to print values
    of this type.  Otherwise the value of the element will be a byte array.
    See the above table for a list of types.

    *units* optionally defines the units of an Information Element.
    See the above table for a list of units.

    *min* optionally defines the minimum value of an Information Element.

    *max* optionally defines the maximum value of an Information Element.

    *semantic* optionally defines the semantics of an Information Element.
    See the above table for a list of semantics.

    *description* optionally contains a human-readable description of an
    Information Element.

    """

    def __init__(self, *args, **kwds):

        _pyfixbuf.fbInfoElementBase.__init__(self, *args, **kwds)


YAF_LIST = [InfoElement("initialTCPFlags", CERT_PEN, 14, 1, True, True,
                        type=DataType.UINT8, semantic=Semantic.FLAGS),
            InfoElement("unionTCPFlags", CERT_PEN, 15, 1, True, True,
                        type=DataType.UINT8, semantic=Semantic.FLAGS),
            InfoElement("reverseFlowDeltaMilliseconds", CERT_PEN, 21, 4,
                          endian=True, type=DataType.MILLISECONDS,
                        units=Units.MILLISECONDS),
            InfoElement("silkAppLabel", CERT_PEN, 33, 2, endian=True,
                        type=DataType.UINT16),
            InfoElement("osName", CERT_PEN, 36, VARLEN, True,
                        type=DataType.STRING),
            InfoElement("payload", CERT_PEN, 18, VARLEN, True,
                        type=DataType.OCTET_ARRAY),
            InfoElement("osVersion", CERT_PEN, 37, VARLEN, True,
                        type=DataType.STRING),
            InfoElement("firstPacketBanner", CERT_PEN, 38, VARLEN,
                        reversible=True),
            InfoElement("secondPacketBanner", CERT_PEN, 39, VARLEN,
                        reversible=True),
            InfoElement("flowAttributes", CERT_PEN, 40, 2, True, True,
                        DataType.UINT16),
            InfoElement("osFingerPrint", CERT_PEN, 107, VARLEN, True,
                        type=DataType.STRING),
            InfoElement("yafFlowKeyHash", CERT_PEN, 106, 4, True, True,
                        type=DataType.UINT32)]

YAF_STATS_LIST = [InfoElement("expiredFragmentCount", CERT_PEN, 100, 4,
                              endian=True, type=DataType.UINT32,
                              units=Units.PACKETS,
                              semantic=Semantic.TOTALCOUNTER),
                  InfoElement("assembledFragmentCount", CERT_PEN, 101, 4,
                              endian=True, type=DataType.UINT32,
                              units=Units.PACKETS,
                              semantic=Semantic.TOTALCOUNTER),
                  InfoElement("meanFlowRate", CERT_PEN, 102, 4, endian=True,
                              type=DataType.UINT32),
                  InfoElement("meanPacketRate", CERT_PEN, 103, 4,
                              endian=True, type=DataType.UINT32),
                  InfoElement("flowTableFlushEventCount", CERT_PEN, 104, 4,
                              endian=True, type=DataType.UINT32,
                              semantic=Semantic.TOTALCOUNTER),
                  InfoElement("flowTablePeakCount", CERT_PEN, 105, 4,
                              endian=True, type=DataType.UINT32,
                              semantic=Semantic.TOTALCOUNTER)]
YAF_FLOW_STATS_LIST= [InfoElement("smallPacketCount", CERT_PEN, 500, 4,
                                  endian=True, reversible=True,
                                  type=DataType.UINT32,
                                  semantic=Semantic.TOTALCOUNTER),
                      InfoElement("nonEmptyPacketCount", CERT_PEN, 501, 4,
                                  endian=True, reversible=True,
                                  type=DataType.UINT32,
                                  semantic=Semantic.TOTALCOUNTER),
                      InfoElement("dataByteCount", CERT_PEN, 502, 8,
                                  endian=True, reversible=True,
                                  type=DataType.UINT64,
                                  semantic=Semantic.TOTALCOUNTER),
                      InfoElement("averageInterarrivalTime", CERT_PEN, 503, 8,
                                  endian=True, reversible=True,
                                  type=DataType.UINT64),
                      InfoElement("standardDeviationInterarrivalTime",
                                  CERT_PEN, 504, 8, endian=True,
                                  reversible=True, type=DataType.UINT64),
                      InfoElement("firstNonEmptyPacketSize", CERT_PEN, 505, 2,
                                  endian=True, reversible=True,
                                  type=DataType.UINT16,
                                  semantic=Semantic.QUANTITY),
                      InfoElement("maxPacketSize", CERT_PEN, 506, 2,
                                  endian=True, reversible=True,
                                  type=DataType.UINT16,
                                  semantic=Semantic.QUANTITY),
                      InfoElement("firstEightNonEmptyPacketDirections",
                                  CERT_PEN, 507, 1, endian=True,
                                  reversible=True, type=DataType.UINT8),
                      InfoElement("standardDeviationPayloadLength", CERT_PEN,
                                  508, 2, endian=True, reversible=True,
                                  type=DataType.UINT16),
                      InfoElement("tcpUrgCount", CERT_PEN, 509, 4,
                                  endian=True, reversible=True,
                                  type=DataType.UINT32),
                      InfoElement("largePacketCount", CERT_PEN, 510, 4,
                                  endian=True, reversible=True,
                                  type=DataType.UINT32)]

YAF_HTTP_LIST = [InfoElement("httpServerString", CERT_PEN, 110),
                 InfoElement("httpUserAgent", CERT_PEN, 111),
                 InfoElement("httpGet", CERT_PEN, 112),
                 InfoElement("httpConnection", CERT_PEN, 113),
                 InfoElement("httpVersion", CERT_PEN, 114),
                 InfoElement("httpReferer", CERT_PEN, 115),
                 InfoElement("httpLocation", CERT_PEN, 116),
                 InfoElement("httpHost", CERT_PEN, 117),
                 InfoElement("httpContentLength", CERT_PEN, 118),
                 InfoElement("httpAge", CERT_PEN, 119),
                 InfoElement("httpAccept", CERT_PEN, 120),
                 InfoElement("httpAcceptLanguage", CERT_PEN, 121),
                 InfoElement("httpContentType", CERT_PEN, 122),
                 InfoElement("httpResponse", CERT_PEN, 123),
                 InfoElement("httpCookie", CERT_PEN, 220),
                 InfoElement("httpSetCookie", CERT_PEN, 221),
                 InfoElement("httpAuthorization", CERT_PEN, 252),
                 InfoElement("httpVia", CERT_PEN, 253),
                 InfoElement("httpX-Forwarded-For", CERT_PEN, 254),
                 InfoElement("httpRefresh", CERT_PEN, 256),
                 InfoElement("httpIMEI", CERT_PEN, 257),
                 InfoElement("httpIMSI", CERT_PEN, 258),
                 InfoElement("httpMSISDN", CERT_PEN, 259),
                 InfoElement("httpSubscriber", CERT_PEN, 260),
                 InfoElement("httpExpires", CERT_PEN, 255),
                 InfoElement("httpAcceptCharset", CERT_PEN, 261),
                 InfoElement("httpAcceptEncoding", CERT_PEN, 262),
                 InfoElement("httpAllow", CERT_PEN, 263),
                 InfoElement("httpDate", CERT_PEN, 264),
                 InfoElement("httpExpect", CERT_PEN, 265),
                 InfoElement("httpFrom", CERT_PEN, 266),
                 InfoElement("httpProxyAuthentication", CERT_PEN, 267),
                 InfoElement("httpUpgrade", CERT_PEN, 268),
                 InfoElement("httpWarning", CERT_PEN, 269),
                 InfoElement("httpDNT", CERT_PEN, 270),
                 InfoElement("httpX-Forwarded-Proto", CERT_PEN, 271),
                 InfoElement("httpX-Forwarded-Host", CERT_PEN, 272),
                 InfoElement("httpX-Forwarded-Server", CERT_PEN, 273),
                 InfoElement("httpX-DeviceID", CERT_PEN, 274),
                 InfoElement("httpX-Profile", CERT_PEN, 275),
                 InfoElement("httpLastModified", CERT_PEN, 276),
                 InfoElement("httpContentEncoding", CERT_PEN, 277),
                 InfoElement("httpContentLanguage", CERT_PEN, 278),
                 InfoElement("httpContentLocation", CERT_PEN, 279),
                 InfoElement("httpX-UA-Compatible", CERT_PEN, 280)]


YAF_SLP_LIST = [InfoElement("slpVersion", CERT_PEN, 128, 1, endian=True,
                            type=DataType.UINT8),
                InfoElement("slpMessageType", CERT_PEN, 129, 1, endian=True,
                            type=DataType.UINT8),
                InfoElement("slpString", CERT_PEN, 130)]

YAF_FTP_LIST = [InfoElement("ftpReturn", CERT_PEN, 131),
                InfoElement("ftpUser", CERT_PEN, 132),
                InfoElement("ftpPass", CERT_PEN, 133),
                InfoElement("ftpType", CERT_PEN, 134),
                InfoElement("ftpRespCode", CERT_PEN, 135)]

YAF_IMAP_LIST = [InfoElement("imapCapability", CERT_PEN, 136),
                 InfoElement("imapLogin", CERT_PEN, 137),
                 InfoElement("imapStartTLS", CERT_PEN, 138),
                 InfoElement("imapAuthenticate", CERT_PEN, 139),
                 InfoElement("imapCommand", CERT_PEN, 140),
                 InfoElement("imapExists", CERT_PEN, 141),
                 InfoElement("imapRecent", CERT_PEN, 142)]

YAF_RTSP_LIST = [InfoElement("rtspURL", CERT_PEN, 143),
                 InfoElement("rtspVersion", CERT_PEN, 144),
                 InfoElement("rtspReturnCode", CERT_PEN, 145),
                 InfoElement("rtspContentLength", CERT_PEN, 146),
                 InfoElement("rtspCommand", CERT_PEN, 147),
                 InfoElement("rtspContentType", CERT_PEN, 148),
                 InfoElement("rtspTransport", CERT_PEN, 149),
                 InfoElement("rtspCSeq", CERT_PEN, 150),
                 InfoElement("rtspLocation", CERT_PEN, 151),
                 InfoElement("rtspPacketsReceived", CERT_PEN, 152),
                 InfoElement("rtspUserAgent", CERT_PEN, 153),
                 InfoElement("rtspJitter", CERT_PEN, 154)]

YAF_SIP_LIST = [InfoElement("sipInvite", CERT_PEN, 155),
                InfoElement("sipCommand", CERT_PEN, 156),
                InfoElement("sipVia", CERT_PEN, 157),
                InfoElement("sipMaxForwards", CERT_PEN, 158),
                InfoElement("sipAddress", CERT_PEN, 159),
                InfoElement("sipContentLength", CERT_PEN, 160),
                InfoElement("sipUserAgent", CERT_PEN, 161)]

YAF_SMTP_LIST = [InfoElement("smtpHello", CERT_PEN, 162),
                 InfoElement("smtpFrom", CERT_PEN, 163),
                 InfoElement("smtpTo", CERT_PEN, 164),
                 InfoElement("smtpContentType", CERT_PEN, 165),
                 InfoElement("smtpSubject", CERT_PEN, 166),
                 InfoElement("smtpFilename", CERT_PEN, 167),
                 InfoElement("smtpContentDisposition", CERT_PEN, 168),
                 InfoElement("smtpResponse", CERT_PEN, 169),
                 InfoElement("smtpEnhanced", CERT_PEN, 170),
                 InfoElement("smtpSize", CERT_PEN, 222),
                 InfoElement("smtpDate", CERT_PEN, 251)]


YAF_DNS_LIST = [InfoElement("dnsQueryResponse", CERT_PEN, 174, 1,
                              endian=True, type=DataType.UINT8),
                InfoElement("dnsQRType", CERT_PEN, 175, 2, endian=True,
                            type=DataType.UINT16),
                InfoElement("dnsAuthoritative", CERT_PEN, 176, 1,
                              endian=True, type=DataType.UINT8),
                InfoElement("dnsNXDomain", CERT_PEN, 177, 1, endian=True,
                            type=DataType.UINT8),
                InfoElement("dnsRRSection", CERT_PEN, 178, 1, endian=True,
                            type=DataType.UINT8),
                InfoElement("dnsQName", CERT_PEN, 179),
                InfoElement("dnsCName", CERT_PEN, 180),
                InfoElement("dnsMXPreference", CERT_PEN, 181, 2,endian=True,
                            type=DataType.UINT16),
                InfoElement("dnsMXExchange", CERT_PEN, 182),
                InfoElement("dnsNSDName", CERT_PEN, 183),
                InfoElement("dnsPTRDName", CERT_PEN, 184),
                InfoElement("dnsTTL", CERT_PEN, 199, 4, endian=True,
                            type=DataType.UINT32),
                InfoElement("dnsTXTData", CERT_PEN, 208),
                InfoElement("dnsSOASerial", CERT_PEN, 209, 4, endian=True,
                            type=DataType.UINT32),
                InfoElement("dnsSOARefresh", CERT_PEN, 210, 4, endian=True,
                            type=DataType.UINT32),
                InfoElement("dnsSOARetry", CERT_PEN, 211, 4, endian=True,
                            type=DataType.UINT32),
                InfoElement("dnsSOAExpire", CERT_PEN, 212, 4, endian=True,
                            type=DataType.UINT32),
                InfoElement("dnsSOAMinimum", CERT_PEN, 213, 4, endian=True,
                            type=DataType.UINT32),
                InfoElement("dnsSOAMName", CERT_PEN, 214),
                InfoElement("dnsSOARName", CERT_PEN, 215),
                InfoElement("dnsSRVPriority", CERT_PEN, 216, 2, endian=True,
                            type=DataType.UINT16),
                InfoElement("dnsSRVWeight", CERT_PEN, 217, 2, endian=True,
                            type=DataType.UINT16),
                InfoElement("dnsSRVPort", CERT_PEN, 218, 2, endian=True,
                            type=DataType.UINT16),
                InfoElement("dnsSRVTarget", CERT_PEN, 219),
                InfoElement("dnsID", CERT_PEN, 226, 2, endian=True,
                            type=DataType.UINT16),
                InfoElement("dnsAlgorithm", CERT_PEN, 227, 1, endian=True,
                            type=DataType.UINT8),
                InfoElement("dnsKeyTag", CERT_PEN, 228, 2, endian=True,
                            type=DataType.UINT16),
                InfoElement("dnsSigner", CERT_PEN, 229),
                InfoElement("dnsSignature", CERT_PEN, 230,
                            type=DataType.OCTET_ARRAY),
                InfoElement("dnsDigest", CERT_PEN, 231,
                            type=DataType.OCTET_ARRAY),
                InfoElement("dnsPublicKey", CERT_PEN, 232,
                            type=DataType.OCTET_ARRAY),
                InfoElement("dnsSalt", CERT_PEN, 233,
                            type=DataType.OCTET_ARRAY),
                InfoElement("dnsHashData", CERT_PEN, 234,
                            type=DataType.OCTET_ARRAY),
                InfoElement("dnsIterations", CERT_PEN, 235, 2, endian=True,
                            type=DataType.UINT16),
                InfoElement("dnsSignatureExpiration", CERT_PEN, 236, 4,
                            endian=True, type=DataType.UINT32),
                InfoElement("dnsSignatureInception", CERT_PEN, 237, 4,
                            endian=True, type=DataType.UINT32),
                InfoElement("dnsDigestType", CERT_PEN, 238, 1, endian=True,
                            type=DataType.UINT8),
                InfoElement("dnsLabels", CERT_PEN, 239, 1, endian=True,
                            type=DataType.UINT8),
                InfoElement("dnsTypeCovered", CERT_PEN, 240, 2, endian=True,
                            type=DataType.UINT16),
                InfoElement("dnsFlags", CERT_PEN, 241, 2, endian=True,
                            type=DataType.UINT16, semantic=Semantic.FLAGS)]

YAF_SSL_LIST = [InfoElement("sslCipher", CERT_PEN, 185, 4, endian=True,
                            type=DataType.UINT32),
                InfoElement("sslClientVersion", CERT_PEN, 186, 1,
                            endian=True, type=DataType.UINT8),
                InfoElement("sslServerCipher", CERT_PEN, 187, 4,endian=True,
                            type=DataType.UINT32),
                InfoElement("sslCompressionMethod", CERT_PEN, 188, 1,
                            endian=True, type=DataType.UINT8),
                InfoElement("sslCertVersion", CERT_PEN, 189, 1, endian=True,
                            type=DataType.UINT8),
                InfoElement("sslCertSignature", CERT_PEN, 190),
                InfoElement("sslCertIssuerCountryName", CERT_PEN, 191),
                InfoElement("sslCertIssuerOrgName", CERT_PEN, 192),
                InfoElement("sslCertIssuerOrgUnitName", CERT_PEN, 193),
                InfoElement("sslCertIssuerZipCode", CERT_PEN, 194),
                InfoElement("sslCertIssuerState", CERT_PEN, 195),
                InfoElement("sslCertIssuerCommonName", CERT_PEN, 196),
                InfoElement("sslCertIssuerLocalityName", CERT_PEN, 197),
                InfoElement("sslCertIssuerStreetAddress", CERT_PEN, 198),
                InfoElement("sslCertSubCountryName", CERT_PEN, 200),
                InfoElement("sslCertSubOrgName", CERT_PEN, 201),
                InfoElement("sslCertSubOrgUnitName", CERT_PEN, 202),
                InfoElement("sslCertSubZipCode", CERT_PEN, 203),
                InfoElement("sslCertSubState", CERT_PEN, 204),
                InfoElement("sslCertSubCommonName", CERT_PEN, 205),
                InfoElement("sslCertSubLocalityName", CERT_PEN, 206),
                InfoElement("sslCertSubStreetAddress", CERT_PEN, 207),
                InfoElement("sslCertSerialNumber", CERT_PEN, 244),
                InfoElement("sslObjectType", CERT_PEN, 245, 1,
                            endian=True, type=DataType.UINT8),
                InfoElement("sslObjectValue", CERT_PEN, 246),
                InfoElement("sslCertValidityNotBefore", CERT_PEN, 247),
                InfoElement("sslCertValidityNotAfter", CERT_PEN, 248),
                InfoElement("sslPublicKeyAlgorithm", CERT_PEN, 249),
                InfoElement("sslPublicKeyLength", CERT_PEN, 250, 2,
                            endian=True, type=DataType.UINT16),
                InfoElement("sslRecordVersion", CERT_PEN, 288, 2,
                            endian=True, type=DataType.UINT16)]

YAF_DPI_LIST = [InfoElement("mysqlUsername", CERT_PEN, 223),
                InfoElement("mysqlCommandCode", CERT_PEN, 224, 1,
                            endian=True, type=DataType.UINT8),
                InfoElement("mysqlCommandText", CERT_PEN, 225),
                InfoElement("pop3TextMessage", CERT_PEN, 124),
                InfoElement("ircTextMessage", CERT_PEN, 125),
                InfoElement("tftpFilename", CERT_PEN, 126),
                InfoElement("tftpMode", CERT_PEN, 127),
                InfoElement("sshVersion", CERT_PEN, 171),
                InfoElement("nntpResponse", CERT_PEN, 172),
                InfoElement("nntpCommand", CERT_PEN, 173),
                InfoElement("dhcpFingerPrint", CERT_PEN, 242, reversible=True),
                InfoElement("dhcpVendorCode", CERT_PEN, 243, reversible=True),
                InfoElement("dnp3SourceAddress", CERT_PEN, 281, 2, endian=True,
                            type=DataType.UINT16),
                InfoElement("dnp3DestinationAddress", CERT_PEN, 282, 2, 
                            endian=True, type=DataType.UINT16),
                InfoElement("dnp3Function", CERT_PEN, 283, 1, endian=True,
                            type=DataType.UINT8),
                InfoElement("dnp3ObjectData", CERT_PEN, 284, 
                            type=DataType.OCTET_ARRAY),
                InfoElement("modbusData", CERT_PEN, 285, 
                            type=DataType.OCTET_ARRAY),
                InfoElement("ethernetIPData", CERT_PEN, 286, 
                            type=DataType.OCTET_ARRAY),
                InfoElement("rtpPayloadType", CERT_PEN, 287, 1, 
                            endian=True, reversible=True, type=DataType.UINT8)]

class InfoElementSpec(_pyfixbuf.fbInfoElementSpecBase):
    """
    Creates a new Information Element Specification using the given *name*,
    and optional override *length*.  An IPFIX Template is made up of one or
    more :class:`InfoElementSpec`.

    The given *name* must be a defined Information Element in the
    Information Model before adding the :class:`InfoElementSpec` to a
    class:`Template`.

    If *length* is nonzero, it will replace the default length of
    this Information Element (often used for reduced-length encoding).

    Examples::

    >>> spec1 = pyfixbuf.InfoElementSpec("fooname")
    >>> spec2 = pyfixbuf.InfoElementSpec("sourceTransportPort")
    >>> spec3 = pyfixbuf.InfoElementSpec("flo_element", 4)
    """
    def __init__(self, *args, **kwds):

        _pyfixbuf.fbInfoElementSpecBase.__init__(self, *args, **kwds)

class Session(_pyfixbuf.fbSessionBase):
    """
    Creates an empty :class:`Session` given an Information Model,
    *InfoModel*.  A Session stores and manages all of the IPFIX Templates.
    """
    def __init__(self, *args, **kwds):
        self.model = args[0]
        _pyfixbuf.fbSessionBase.__init__(self, *args, **kwds)
        self.internal_templates = dict()
        self.external_templates = dict()

    def add_template(self, template, template_id=0):
        """
        Adds the given *template* to the session with the optional
        *template_id*.  This template will be added to both the
        internal and external templates.  Use add_internal_template
        or add_external_template to be more selective on template
        usage.

        If a *template_id* is not given or 0, libfixbuf will
        automatically choose one. *template_id* will be used for
        representing both the internal and external template.

        Returns the *template_id* of the added template.

        """
        if (template_id > 65535):
            raise Exception("Invalid ID: " + str(template_id) +
                            " Template ID must be a 16 bit integer")
        tid = _pyfixbuf.fbSessionBase.addTemplate(self, template, template_id,
                                                  True)
        _pyfixbuf.fbSessionBase.addTemplate(self, template, tid, False)
        self.internal_templates[tid] = template
        self.external_templates[tid] = template
        template.added = True
        return tid


    def add_internal_template(self, template, template_id=0):
        """
        Adds the given *template* as an internal template to the session with
        the optionally given *template_id*. An internal template determines
        how the data will be presented when transcoded.

        If *template_id* is not set or 0, libfixbuf will automatically
        choose one.

        Returns the *template_id* of the added template.

        """

        if (template_id > 65535):
            raise Exception("Invalid ID: " + str(template_id) +
                            " Template ID must be a 16 bit integer")
        tid = _pyfixbuf.fbSessionBase.addTemplate(self, template, template_id,
                                                  True)
        self.internal_templates[tid] = template
        template.added = True
        return tid

    def add_external_template(self, template, template_id=0):
        """
        Adds the given *template* as an external template to the session with
        the optionally given *template_id*.

        If *template_id* is not set or 0, libfixbuf will automatically
        choose one.

        Returns the *template_id* of the added template.
        """
        if (template_id > 65535):
            raise Exception("Invalid ID: " + str(template_id) +
                            " Template ID must be a 16 bit integer")
        tid = _pyfixbuf.fbSessionBase.addTemplate(self, template, template_id,
                                                  False)
        self.external_templates[tid] = template
        template.added = True
        return tid

    def decode_only(self, id_list):
        """
        This method is for IPFIX Collectors only.  Only decode
        records in a list that have template IDs in the given *list*.

        This does not apply for all incoming templates.  This only
        applies for nested templates found in a SubTemplateMultiList
        or SubTemplateList.

        """
        for item in id_list:
            if ( not (isinstance(item, int)) ):
                raise Exception("Invalid Template ID: " + str(item) +
                                    ". Template ID in List must be an Integer");
        _pyfixbuf.fbSessionBase.addDecodeList(self, id_list)

    def ignore_templates(self, id_list):
        """
        This method is for IPFIX Collectors only.  Ignore all
        templates with the template IDs in the given *list*.

        This does not apply for all incoming templates.  This only
        applies for nested templates found in a SubTemplateMultiList
        or SubTemplateList.


        """
        for item in id_list:
            if ( not (isinstance(item, int)) ):
                raise Exception("Invalid Template ID: " + str(item) +
                                ". Template ID in List must be an Integer");

        _pyfixbuf.fbSessionBase.addIgnoreList(self, id_list)

    def get_template(self, template_id, internal=False):
        """
        Return the template with the given *template_id*.  By default,
        the external template is returned.  Set *internal* to True,
        to retrieve the internal template with the given *template_id*.

        This template cannot be modified.
        """
        template = _pyfixbuf.fbSessionBase.getTemplate(self, template_id, internal)
        if (template == None):
            if internal:
                raise Exception("Internal template with ID: " + str(template_id) +
                                " does not belong to this Session.")
            else:
                raise Exception("External template with ID: " + str(template_id) +
                                " has not been added to this Session.")
        new_template = Template(self.model, 0, template)
        return new_template

    def add_template_pair(self, external_template_id, internal_template_id):
        """
        This method is for IPFIX Collectors.  This gives the collector
        control over how to transcode incoming templates in a
        SubTemplateMultiList (STML) or SubTemplateList (STL).

        By default, libfixbuf transcodes each entry in the STML or STL with
        the external template it received, requiring the collector to free or
        clear any
        memory allocated for the list elements. The collector can change this
        behavior by adding a "template pair."  For each entry in the STL
        or STML, if the entry has the given *external_template_id*, it will
        use the given *internal_template_id* to transcode the record.
        The *internal_template_id* must reference an internal template
        that was previously added to the session.  If *internal_template_id*
        is 0, the entry will not be transcoded and will be ignored by
        libfixbuf.

        Once a template pair has been added - the default is to ONLY decode
        entries that have an external_template_id in this template pair table.
        Therefore, any entries in a STML or STL that reference a template id
        not in this table will be dropped.
        """
        if (self.internal_templates.has_key(internal_template_id) or
            internal_template_id == 0):
            _pyfixbuf.fbSessionBase.addTemplatePair(self, external_template_id,
                                                    internal_template_id)
        else:
            raise Exception("An internal template with id: " +
                            str(internal_template_id)
                            + " has not been added to this session")


class InfoModel(_pyfixbuf.fbInfoModelBase):
    """
    An IPFIX Information Model stores all of the Information Elements that
    can be collected or exported by a Collecting or Exporting Process.

    The :class:`InfoModel` constructor creates a new Information Model and
    adds the default IANA-managed Information Elements.

    """
    def __init__(self):

        _pyfixbuf.fbInfoModelBase.__init__(self)

    def add_element(self, element):
        """ Adds the given :class:`InfoElement`, *element*, to the
        :class:`InfoModel`.
        """
        return _pyfixbuf.fbInfoModelBase.addElement(self, element)

    def add_element_list(self, list):
        """Adds a *list* of :class:`InfoElement` to the :class:`InfoModel`."""
        for item in list:
            if item != None:
                _pyfixbuf.fbInfoModelBase.addElement(self, item)

    def get_element_length(self, *args, **kwds):
        """
        Returns the default length of the Information Element with the
        given *name* in the Information Model.
        If *type* (BASICLIST, SUBTEMPLATELIST,
        SUBTEMPLATEMULTILIST) is given, it assumes this element will be used
        in a list and the length of the list structure in libfixbuf will be
        returned.
        If the Information Element is a variable length (VARLEN) element, the length
        that will be returned is the length of the fbVarfield_t structure
        in libfixbuf.  To get the length of the Information Element as it
        is defined in the Information Model, use :meth:`get_element` to return
        the :class:`InfoElement` and the length attribute.
        """
        return _pyfixbuf.fbInfoModelBase.getElementLength(self, *args, **kwds)

    def get_element_type(self, name):
        """
        Returns the type of the Information Element as defined in the
        :class:`InfoModel` given the Information Element *name*.
        """
        return _pyfixbuf.fbInfoModelBase.getElementTrueType(self, name)

    def get_element(self, *args, **kwds):
        """
        Returns the :class:`InfoElement` given the *name* or *id* and *ent*.
        """
        return _pyfixbuf.fbInfoModelBase.getElement(self, *args, **kwds)

    def add_options_element(self, rec):
        """
        Add the information element contained in the Options :class:`Record`.
        Use this method for incoming Options Records that contain Information
        Element Type Information.
        """
        if (not isinstance(rec, dict)):
            rec.as_dict()
        dt = rec["informationElementDataType"]
        len = typeToLength(dt)
        return self.add_element(InfoElement(rec["informationElementName"],
                              rec["privateEnterpriseNumber"],
                              rec["informationElementId"], len, type=dt,
                              min=rec["informationElementRangeBegin"],
                              max=rec["informationElementRangeEnd"],
                              units=rec["informationElementUnits"],
                              semantic=rec["informationElementSemantics"],
                             description=rec["informationElementDescription"]))


class Exporter(_pyfixbuf.fbExporterBase):
    """
    Creates an empty :class:`Exporter`.  Initialize the exporter using
    :meth:`init_file` or :meth:`init_net`.
    """

    def __init__(self):
        self.initialized = 0
        _pyfixbuf.fbExporterBase.__init__(self)

    def init_file(self, filename):
        """
        Initializes the :class:`Exporter` to write to the given *filename*.
        """
        self.initialized = 1
        _pyfixbuf.fbExporterBase.allocFile(self, filename)

    def init_net(self, hostname, transport="tcp", port=4739):
        """
        Initializes the :class:`Exporter` to write to the given *hostname*,
        *port* over the given *transport*.

        Given *hostname* may be a hostname or IP address.

        Acceptable values for *transport* are "tcp" and "udp". Default is
        "tcp."

        Given *port* must be greater than 1024. Default is 4739.
        """
        if (port < 1024 or port == 0):
            raise Exception("Invalid Port. Port must be greater than 1024.")
        self.initialized = 1
        _pyfixbuf.fbExporterBase.allocNet(self, host=hostname,
                                          transport=transport, port=str(port))


class Collector(_pyfixbuf.fbCollectorBase):
    """
    Creates an uninitialized :class:`Collector`.  An IPFIX Collector manages
    the file it is reading from.
    Initialize the collector using :meth:`init_file`.
    """
    def __init__(self):
        self.initialized = 0
        _pyfixbuf.fbCollectorBase.__init__(self)

    def init_file(self, filename):
        """
        Initialize the :class:`Collector` to read from the given *filename*.
        *filename* should be the path to a valid IPFIX File.
        """
        self.initialized = 1
        return _pyfixbuf.fbCollectorBase.allocCollectorFile(self, filename)

class Template(_pyfixbuf.fbTemplateBase):
    """
    Creates a new Template using the given *model*.  An IPFIX Template
    is an ordered list of the Information Elements that are to be
    collected or exported.  For export, the order of Information Elements
    in the Templates determines how the data will be exported.

    If *type* is given, an Information Element Type Information Options
    Template will be created.  The appropriate elements will automatically
    be added to the template and the scope will be sent.  See :rfc:`5610`
    for more information.

    Once a Template has been added to the session, it can not be altered.

    A :class:`Template` can be accessed like a dictionary or a list to
    retrieve a specific :class:`InfoElementSpec.`'

    """

    def __init__(self, *args, **kwds):
        _pyfixbuf.fbTemplateBase.__init__(self, *args, **kwds)
        self.infomodel = args[0]
        self.specs = []
        self.added = False
        self.type_template = False
        if len(args) == 2:
            self.type_template = args[1]
            self.specs.extend([InfoElementSpec("informationElementRangeBegin"),
                              InfoElementSpec("informationElementRangeEnd"),
                              InfoElementSpec("privateEnterpriseNumber"),
                              InfoElementSpec("informationElementUnits"),
                              InfoElementSpec("informationElementId"),
                              InfoElementSpec("informationElementDataType"),
                              InfoElementSpec("informationElementSemantics"),
                              InfoElementSpec("paddingOctets", 6),
                              InfoElementSpec("informationElementName"),
                              InfoElementSpec("informationElementDescription")])

    def add_spec(self, spec):
        """ Adds a given InfoElementSpec *spec* to the Template.

        Once the Template has been added to the session, it can not be altered.

        """
        if (self.added):
            raise Exception("Invalid add: Template has already been added "
                            "to the session.")
        if (not (isinstance(spec, InfoElementSpec))):
            raise Exception("Not a valid InfoElementSpec.")

        if spec.length:
            if (spec.length > self.infomodel.get_element_length(spec.name)):
                raise Exception("Invalid Reduced-length.  Must be smaller than "
                                "the default InfoElement length.")
            if (not(verifyModifiedLength(self.infomodel.get_element_type(spec.name)))):
                raise Exception("Information Element Data Type does not support "
                                "reduced-length encoding.")
        self.specs.append(spec)
        return _pyfixbuf.fbTemplateBase.addSpec(self, spec)


    def add_spec_list(self, list):
        """ Adds the given *list* of InfoElementSpec items to the 
        :class:`Template`.

        Once the :class:`Template` has been added to the :class:`Session`, 
        it can not be altered.
        """
        if (self.added):
            raise Exception("Invalid add: Template has already been added"
                            " to the session.")
        for item in list:
            if (not (isinstance(item, InfoElementSpec))):
                raise Exception("List item is not  a valid InfoElementSpec.")
            if item.length:
                if (item.length > self.infomodel.get_element_length(item.name)):
                    raise Exception("Invalid Reduced-length.  Must be smaller than "
                                    "the default InfoElement length.")
                if (not(verifyModifiedLength(self.infomodel.get_element_type(item.name)))):
                    raise Exception("Information Element Data Type does not support "
                                    "reduced-length encoding.")

            self.specs.append(item)
            _pyfixbuf.fbTemplateBase.addSpec(self, item)


    def add_element(self, name):
        """ Adds an Information Element with the given *name* to the Template.
        This function can be used as an alternative to add_spec.

        This function creates an :class:`InfoElementSpec` with the given element
        *name* and default length and adds it to the template.
        """
        self.add_spec(InfoElementSpec(name))


    def __contains__(self, element):
        """ Determine if the given element is in the Template.

        If *element* is a name, return True if an Information Element with
        the given name is included in the Template.

        If *element* is an :class:`InfoElement` or :class:`InfoElementSpec`,
        return True if the element exists in the Template, False otherwise.
        """
        return _pyfixbuf.fbTemplateBase.containsElement(self, element)

    def __len__(self):
        """ Returns the number of elements in the :class:`Template`.
        """
        return _pyfixbuf.fbTemplateBase.__len__(self)

    def __getitem__(self, key):
        """ Returns the :class:`InfoElementSpec` with the given 
        *name* or index 
        """
        if (isinstance(key, str)):
            for item in self.specs:
                if item.name == key:
                    return item
            raise KeyError("No InfoElementSpec with name " + key)
        elif (isinstance(key, int)):
            if key < len(self):
                return self.specs[key]
            else:
                raise IndexError("No element at index " + str(key))
        else:
            raise TypeError

    def build_spec_list(self):
        """ Wallk through the :class:`Template` to build the 
        list of class:`InfoElementSpecs` contained in the :class:`Template`.
  
        This is typically used by a :class:`Collector` or :class:`Listener` 
        for external templates that it has received from the :class:`Exporter`
        or in a file. 
        """
        for i in range(0, len(self)):
            ie = self.getIndexedIE(i)
            if (ie.length != VARLEN):
                self.specs.append(InfoElementSpec(ie.tname, ie.length))
            else:
                self.specs.append(InfoElementSpec(ie.tname))

class Record(_pyfixbuf.fbRecordBase):
    """
    Creates an empty Record given an :class:`InfoModel`, *model*, and
    optionally a *template* and *record*.

    The :class:`Record` is returned from a collection :class:`Buffer` or
    is added to an exporting :class:`Buffer`.

    When adding elements to a :class:`Record`, the :class:`Record` 
    should match a :class:`Template`.  If the process is
    collecting, the :class:`Record` should match the Internal Template.
    For an Exporting process, the :class:`Record` should match the External
    Template, and there should be one :class:`Record` for each External
    Template.  A :class:`Record` can not contain more Information Elements
    than it's associated *template*.  Information Elements should be added to the
    :class:`Record` in the same order as the :class:`Template`.

    If a *template* is given to the constructor,, all Information 
    Elements that exist in the
    *template* will be added to the :class:`Record` in the same order as they
    exist in the Template.

    If a *record* is given, all Information Elements that exist in the
    *record* will be added to the :class:`Record` in the same order as
    they exist in the *record.*

    One element must exist in the :class:`Record` before 
    exporting any data.

    A :class:`Record` maintains internal dictionaries for the elements
    that it contains.  For this reason, if a template contains more than 1
    of the same Information Element,
    elements must be added using the :meth:`add_element` method in order
    to give alternate key names to elements that are the same.

    A :class:`Record` may also be accessed similar to a list.

    """
    def __init__(self, model, template=None, record=None):
        self.off_dict = dict()
        self.len_dict = dict()
        self.type_dict = dict()
        self.basic_list = dict()
        self.index_to_name_list = []
        self.list_init = False
        self.template = None
        self.session = None
        self.current = 0
        if (isinstance(model, InfoModel)):
            self.model = model
        else:
            raise Exception("Invalid InfoModel. Argument 1 must be of "
                            "type InfoModel")
        count = 0
        i = 0
        if template:
            if (isinstance(template, Template)):
                self.template = template
            else:
                raise Exception("Invalid Template. "
                                "Argument 2 must be of type Template")
            for item in template.specs:
                new_key = (item.name,0)
                while new_key in self.off_dict:
                    new_key=(item.name,(new_key[1]+1))
                self.off_dict[new_key] = count
                self.index_to_name_list.append(new_key)
                if (item.length):
                    # use length override and get actual type
                    self.len_dict[new_key] = item.length
                    self.type_dict[new_key]=model.get_element_type(item.name)
                else:
                    # otherwise get it from the infomodel
                    self.len_dict[new_key] =model.getElementLength(item.name)
                    self.type_dict[new_key] = model.getElementType(item.name)
                count = count + self.len_dict[new_key]
                if self.type_dict[new_key] == BASICLIST:
                    self.basic_list[new_key] = None
        elif record:
            count = 0
            if (isinstance(record, Record)):
                self.template = record.template
                for key,value in record.off_dict.items():
                    self.off_dict[key] = value
                    self.index_to_name_list.append(key)
                for key,value in record.len_dict.items():
                    self.len_dict[key] = value
                    count = count + value
                    self.index_to_name_list.append(key)
                for key, value in record.type_dict.items():
                    self.type_dict[key] = value
                    self.index_to_name_list.append(key)
                for key, value in record.basic_list.items():
                    self.basic_list[key] = value
        _pyfixbuf.fbRecordBase.__init__(self, count)
        self.length = count

    def add_element(self, key_name, type=0, element_name=None, length=0):
        """
        Adds an Information Element with the given *key_name*, optional *type*,
        optional *element_name*, and optional reduced-length *length*
        to the :class:`Record`.

        If *key_name* is the same name as the defined :class:`InfoElementSpec`,
        then a *type* is not necessary.

        If the template contains more than one of the same Information Element,
        you must give an alternate *key_name*, *type*, and *element_name* to
        describe the other Information Elements.

        A *type* 0 is a regular, fixed-length Information Element.
        Other valid types are VARLEN, BASICLIST, SUBTEMPLATELIST, and
        SUBTEMPLATEMULTILIST.

        *element_name* is the defined Information Element name.  This is
        only needed if the *key_name* is NOT a valid Information Element name
        and *type* = 0.

        *length* is the reduced-length value for the Information Element.  This
        can only be applied to certain data types and must be a smaller length
        than the default Information Element length. If set to 0, the length
        will default to the length provided by the InfoModel.

        Elements must be added in the same order as they exist in the template.

        Examples::

        >>> my_rec = pyfixbuf.Record(model)
        >>> my_rec.add_element("sourceTransportPort")
        >>> my_rec.add_element("sourceTransportPort2", 0, "sourceTransportPort")
        >>> my_rec.add_element("basicList")
        >>> my_rec.add_element("basicList2", BASICLIST)
        >>> my_rec.add_element("octetTotalCount", length=4)

        In the above example, an empty :class:`Record` was created.
        The corresponding template
        to the above :class:`Record` would look something like:

        >>> tmpl = Template(model)
        >>> tmpl.add_spec_list([pyfixbuf.InfoElementSpec("sourceTransportPort"),
        ...                   pyfixbuf.InfoElementSpec("sourceTransportPort"),
        ...                   pyfixbuf.InfoElementSpec("basicList"),
        ...                   pyfixbuf.InfoElementSpec("basicList"),
        ...                   pyfixbuf.InfoElementSpec("octetTotalCount", 4)])

        As you can see, we have two sourceTransportPort elements and
        two basicList elements.  A basicList is a list
        of one or more of the same Information Element.
        The Information Element in the basicList does not have to be
        initialized until data is added to the :class:`Record`.

        Since we have two sourceTransportPort fields, we must give a *key_name*
        to one of the elements, in this case, sourceTransport2.
        Since sourceTransportPort2 is not a defined Information Element in the
        Information Model, the *element_name* must be given to the method.

        Similarly, in order to access the dictionary of elements
        in the :class:`Record`, we had to give the second basicList a
        *key_name*, basicList2. Since basicList2 is not a defined Information
        Element, it needs to be given the *type*, BASICLIST.
        Since *type* is not 0, it does not need an *element_name*.
        """
        if (self.list_init == True):
            raise Exception("Information Elements may not be added to the "
                            "Record once the Record's lists have been "
                            "initialized.")
        key = (key_name, 0)
        while key in self.off_dict:
            key = (key_name, key[1]+1)
        if (type > 0):
            if (type != BASICLIST and type != SUBTEMPLATELIST and
                type != SUBTEMPLATEMULTILIST and type != VARLEN):
                raise Exception("Invalid Type " + str(type))
        self.off_dict[key] = self.length
        self.index_to_name_list.append(key)
        if element_name == None:
            element_name = key_name
        try:
            self.len_dict[key] = self.model.get_element_length(element_name, type)
        except StandardError:
                raise Exception("Information Element " + element_name +
                                " does Not Exist in Information Model")


        if type:
            self.type_dict[key] = type
        else:
            self.type_dict[key] = self.model.getElementType(element_name)

        # use modified length if requested and type is not a list/varlen
        if (length and (type == 0)):
            if (verifyModifiedLength(self.type_dict[key])):
                if (length <= self.len_dict[key]):
                    self.type_dict[key] = lenToType(length, self.type_dict[key])
                    self.len_dict[key] = length
                else:
                    raise Exception("Length value must be smaller than default "
                                    "Information Element Length (default: " +
                                    str(self.len_dict[key]) + ").")
            else:
                raise Exception("Information Element " + element_name +
                                " does not support reduced-length encoding.")


        self.length = self.length + self.len_dict[key]
        if (self.type_dict[key] == DataType.BASIC_LIST):
            ie = self.model.get_element(element_name)
            if (ie.name):
                self.basic_list[key] = element_name
            else:
                self.basic_list[key] = None


    def clear_all_lists(self):
        """
        Clears all the lists in the top level of the :class:`Record`.

        Any nested lists must be accessed and cleared manually.

        This is useful for a :class:`Record`
        that contains mostly one level list items, such as YAF_HTTP_LIST.
        """
        for key,value in self.type_dict.items():
            if value == DataType.BASIC_LIST:
                self.clear_basic_list(key)
            elif value == DataType.SUB_TMPL_LIST:
                stl = self[key]
                stl.clear()
            elif value == DataType.SUB_TMPL_MULTI_LIST:
                stml = self[key]
                stml._clear_stml(self, self.off_dict[key])

    def clear(self):
        """
        Clears any memory allocated for the :class:`Record`.

        """
        _pyfixbuf.fbRecordBase.clear(self)

    def add_element_list(self, element_list):
        """
        Adds the given *element_list*, a list of Information Element names
        to the :class:`Record`.  See above method :meth:`addElement`.
        """
        for item in element_list:
            self.add_element(item)

    def init_basic_list(self, basic_list_key, count=0, element_name=None):
        """
        Initializes a basicList for export with the given *basic_list_key*
        name to a list of  *count* elements.  If a name is not given to
        the *element_name* keyword, it assumes
        the *basic_list_key* is a valid Information Element Name.

        Examples::

            >>> my_rec.add_element("bL", BASICLIST, "octetTotalCount")
            >>> my_rec.add_element("basicList")
            >>> my_rec.add_element("basicList2", BASICLIST)
            >>> my_rec.init_basic_list("bL", 4)
            >>> my_rec.init_basic_list("basicList", 3, "destinationTransportPort")
            >>> my_rec.init_basic_list("basicList2", 2, "souceIPv4Address")

        In the above example, we have initialized three basicLists.  The first
        initializes a basicList of octetTotalCounts by adding the element as
        as basicList to the record.  Later we initialize the basicList to 4 items.
        The second does the initialization of the type, destintationTransportPort,
        when calling :meth:`init_basic_list` as opposed to the first, which is done
        when the basicList is added to the record. The third,
        basicList2, is initialized to two sourceIPv4Adresses.

        It is perfectly acceptable to initialize a list to 0 elements.
        All basicLists in the :class:`Record` must be initialized before
        appending the :class:`Record` to the :class:`Buffer`.

        A basicList may be initialized via this method, or by using the
        :class:`BL` and setting the basicList element in the 
        :class:`Record` to the :class:`BL`.
        """
        if (isinstance(basic_list_key, str)):
            basic_list_key = (basic_list_key, 0)
        if element_name:
            ie = element_name
        elif (self.basic_list[basic_list_key] == "basicList"):
            raise Exception("Nested basicLists are not supported")
        elif (self.basic_list[basic_list_key] != None):
            ie = self.basic_list[basic_list_key]
        else:
            raise Exception(basic_list_key[0] + " must be initialized to an "
                            "information element.")
        self.basic_list[basic_list_key] = BL(self.model, ie, count)


    def clear_basic_list(self, basic_list_key):
        """
        Clears the basicList.  Frees any memory allocated for the list and
        should be called after the :class:`Record` has been appended to
        the :class:`Buffer`.
        """
        if (isinstance(basic_list_key, str)):
            basic_list_key = (basic_list_key, 0)
        _pyfixbuf.fbRecordBase.basicListClear(self,
                                              self.off_dict[basic_list_key])

    def normalize_key(self, key):
        """
        Convert a string or integer key into canonical form.
        """
        if (isinstance(key, str)):
            return (key, 0)
        if (isinstance(key, int)):
            return self.index_to_name_list[key]
        return key

    def __getitem__(self, key):
        """
        Returns the value of the element with the given key.

        *key* may be the name of an InfoElement, the *key_name* given to
        add_element() when the element was added to the :class:`Record`, or
        an index (integer) into the :class:`Record`.

        """

        key = self.normalize_key(key)
        list = []
        # Check to make sure this element exists
        if (False == self.type_dict.has_key(key)):
            raise Exception("Element " + key[0] +
                            " does not exist in this Record")

        if (self.type_dict[key] == DataType.BASIC_LIST):
            bl = BL(self.model, self.basic_list[key])
            _pyfixbuf.fbRecordBase.getBL(self, bl, self.off_dict[key])
            bl._fill_bl_list()
            return bl
        elif (self.type_dict[key] == VARLEN):
            var = _pyfixbuf.fbRecordBase.getOffset(self, self.off_dict[key],
                                                   self.len_dict[key],
                                                   self.type_dict[key])
            return var.string
        elif (self.type_dict[key] == DataType.SUB_TMPL_MULTI_LIST):
            return self.get_stml_list_entry(key)
        elif (self.type_dict[key] == DataType.SUB_TMPL_LIST):
            return self.get_stl_list_entry(key)
        elif (self.type_dict[key] == DataType.IP4ADDR):
            return str(ipaddress.IPv4Address(_pyfixbuf.fbRecordBase.getOffset(
                                                         self,
                                                         self.off_dict[key],
                                                         self.len_dict[key],
                                                         self.type_dict[key])))
        elif (self.type_dict[key] == DataType.IP6ADDR):
            v6 = _pyfixbuf.fbRecordBase.getOffset(self, self.off_dict[key],
                                                  self.len_dict[key],
                                                  self.type_dict[key])
            return bytes_to_v6(v6)
        elif (self.type_dict[key] == DataType.MAC_ADDR):
            mac = _pyfixbuf.fbRecordBase.getOffset(self, self.off_dict[key],
                                                   self.len_dict[key],
                                                   self.type_dict[key])
            return ':'.join('%02x' % ord(b) for b in mac)
        else:
            return _pyfixbuf.fbRecordBase.getOffset(self, self.off_dict[key],
                                                    self.len_dict[key],
                                                    self.type_dict[key])

    def __setitem__(self, key, value):
        """
        Set the given element with name *key* to the given *value*.
        If the *value* is an IP Address, it will convert the
        String representation to an ``int``.
        The *key* may be a string which represents either the :class:`InfoElement`
        name or *key_name* given to add_element. The *key* may also be an integer,
        which is an index into the :class:`Record`.
        """
        key = self.normalize_key(key)
        if (key not in self.type_dict):
            raise Exception(key + " does not exist in Record.")
        if (self.type_dict[key] == BASICLIST):
            if (isinstance(value, list)):
                if (self.basic_list[key] == None):
                    raise Exception("Basic List \"" + key + "\"  has not "
                                    "been initialized, call init_basic_list()");
                elif (isinstance(self.basic_list[key], BL)):
                    self.basic_list[key].copy(value)
                    bl = self.basic_list[key]
                else:
                    self.basic_list[key] = BL(self.model, self.basic_list[key],
                                              len(value))
                    self.basic_list[key].copy(value)
                    bl = self.basic_list[key]
            elif (not(isinstance(value, BL))):
                if (self.basic_list[key] == None):
                    raise Exception("Basic List \"" + key + "\" has not "
                                    "been initialized, call init_basic_list()");
                elif (isinstance(self.basic_list[key], BL)):
                    self.basic_list[key].copy([value])
                    bl = self.basic_list[key]
                else:
                    self.basic_list[key] = BL(self.model, self.basic_list[key],1)
                    self.basic_list[key].copy([value])
                    bl = self.basic_list[key]
            else:
                bl = value
            _pyfixbuf.fbRecordBase.setOffset(self, bl, self.off_dict[key],
                                             self.len_dict[key],
                                             self.type_dict[key])
        else:
            if ((self.type_dict[key] == DataType.IP4ADDR)
                and isinstance(value, str)):
                value = int(ipaddress.IPv4Address(value))
            elif (self.type_dict[key] == DataType.IP6ADDR):
                value = v6_to_bytes(value)
            elif (self.type_dict[key] == DataType.MAC_ADDR):
                value = macToHex(value)
            elif (self.type_dict[key] == SUBTEMPLATELIST):
                self.list_init = True
                if (isinstance(value, list)):
                    stl = STL(self, key)
                    template = stl_template(value)
                    if (template == None):
                        raise Exception("At least one Record in list must "
                                        "be associated with template.")
                    stl.entry_init(value[0], template, len(value))
                    i = 0
                    while (i < len(value)):
                        stl[i] = value[i]
                        i += 1
                    value = stl
                elif (isinstance(value, STL)):
                    value.inrec=self
            elif (self.type_dict[key] == SUBTEMPLATEMULTILIST):
                self.list_init = True
                if (isinstance(value, list)):
                    stml = create_stml_from_list(value)
                    value = stml
                elif (isinstance(value, STML)):
                    value._put_stml_rec(self, self.off_dict[key])
            else:
                rt = check_type(value, self.type_dict[key])
                if (rt == False):
                    raise Exception("Value is type of " + str(type(value)) +
                                    " but key is of different type")

            _pyfixbuf.fbRecordBase.setOffset(self, value, self.off_dict[key],
                                             self.len_dict[key],
                                             self.type_dict[key])

    def copy(self, other):
        """
        Copies all the matching elements in this :class:`Record` to the
        *other* :class:`Record`.
        """
        if (not(isinstance(other, Record))):
            raise Exception("Copying from non-Record Object")
        for key,value in self.off_dict.items():
            if other.off_dict.has_key(key):
                newval = other[key]
                self[key] = newval

    def is_list(self, key):
        """
        Returns ``True`` or ``False`` depending on the type of the given
        *key*.
        """
        key = self.normalize_key(key)
        if (self.type_dict[key] in [BASICLIST, SUBTEMPLATELIST, SUBTEMPLATEMULTILIST]):
            return True
        else:
            return False

    def get_stl_list_entry(self, key):
        """
        Gets the subTemplateList from the :class:`Record` with the given
        *key* and returns a newly allocated :class:`STL`.

        A :class:`STL` may also be accessed by using :meth:`__getitem__`.
        """
        key = self.normalize_key(key)
        if self.is_list(key):
            stl = STL(self, key)
            stl.session = self.session
            return stl
        else:
            return None

    def get_stml_list_entry(self, key):
        """
        Gets the subTemplateMultiList with the given *key* and returns
        a newly allocated :class:`STML`.
        
        A :class:`STML` may also be retrieved by using __getitem__().

        """
        key = self.normalize_key(key)
        if (self.is_list(key)):
            stml = STML(self, key)
            stml.session = self.session
            return stml

    def as_dict(self):
        """ Returns the :class:`Record` as a dictionary. """
        recdict = dict()
        for key,value in self.off_dict.items():
            num = key[1]
            if (num == 0):
                recdict[key[0]] = self.__getitem__(key)
            else:
                recdict[key] = self.__getitem__(key)
        return recdict

    def __len__(self):
        """ Returns the number of elements in the :class:`Record`. """
        return len(self.off_dict)

    def __contains__(self, item):
        """ Returns True if item is in the :class:`Record`, False otherwise """
        if (isinstance(item, str)):
            item = (item, 0)
        if item in self.off_dict:
            return True
        else:
            return False

    def __iter__(self):
        """ Iterate through the Record """
        return self

    def next(self):
        """ Returns the next item in the :class:`Record` """
        if self.current < len(self.index_to_name_list):
            key = self.index_to_name_list[self.current]
            self.current += 1
            return self[key]
        else:
            raise StopIteration

    def matches_template(self, template):
        """ Returns True if the entries in the :class:`Template` match the
        information element entries in the :class:`Record`. """
        i = 0
        while i < len(template):
            ie = template.getIndexedIE(i)
            if (i >= len(self.index_to_name_list)):
                break
            x = self.index_to_name_list[i]
            ie2 = self.model.get_element(x[0])
            if ie.tname == ie2.name:
                i += 1
                continue
            elif ie.type in [BASICLIST, SUBTEMPLATELIST, SUBTEMPLATEMULTILIST]:
                i += 1
                continue
            else:
                return False
        return True

    def set_template(self, template):
        """ 
        If the :class:`Record` was not initialized with a :class:`Template`,
        this method is used to set the corresponding :class:`Template` with the
        :class:`Record`.  A :class:`Record` must have a :class:`Template` associated
        with it when assigning a :class:`Record` to a subTemplateList element.

        Examples:

        >>> tmpl = pyfixbuf.Template(model)
        >>> tmpl.add_spec_list([pyfixbuf.InfoElementSpec("sourceTransportPort"),
        ...                   pyfixbuf.InfoElementSpec("destinationTransportPort")]
        >>> my_rec = pyfixbuf.Record(model)
        >>> my_rec.add_element("sourceTransportPort", "destinationTransportPort")
        >>> my_rec["sourceTransportPort"] = 13
        >>> my_rec["destinationTransportPort"] = 15
        >>> my_rec.set_template(tmpl)
        >>> other_rec["subTemplateList"] = [my_rec]
        """

        if (len(template) == len(self)):
            self.template = template
        else:
            raise Exception("Number of items in template does not "
                            "match number of items in Record.")

    def count(self, element_name):
        """ Counts the occurrence of the *element_name* in the 
        :class:`Record`.

        Examples:
        
        >>> rec.add_element_list(["basicList", "basicList", "basicList"])
        >>> rec.count("basicList")
        3        
        >>> rec.count("sourceTransportPort")
        0
        """
        counter = 0
        for key,value in self.off_dict.items():
            if (key[0] == element_name):
                counter += 1

        return counter

class Buffer(_pyfixbuf.fBufBase):
    """
    Creates an uninitialized :class:`Buffer` given a :class:`Record`, *record*.
    A :class:`Record` must be associated with the :class:`Buffer` before
    retrieving or appending data to the Buffer.  If *auto* is set, 
    a :class:`Template` will be auto-generated from the external template that
    is set on the :class:`Buffer`. A :class:`Record` will then be auto-generated
    to match the new :class:`Template` that was set on the :class:`Buffer`.
    
    The :class:`Buffer` must also be initialized for collection using 
    :meth:`init_collection` or exporting :meth:`init_export` prior to calling next().
    """
    def __init__(self, record=None, auto=False):
        self.session = None
        self.int_tmpl = None
        self.ext_tmpl = None
        self.model = None
        #set 1 for collection, 2 for export
        self.mode = 0
        self.auto = auto
        if record:
            if (not (isinstance(record, Record))):
                raise Exception("Not a valid Record.")
        self.rec = record
        _pyfixbuf.fBufBase.__init__(self)

    def init_collection(self, session, collector):
        """
        Initialize the :class:`Buffer` for collection given the
        :class:`Session`, *session*, and :class:`Collector`, *collector*.
        """
        self.session = session
        self.model = session.model
        self.mode = 1
        if (collector.initialized == 0):
            raise Exception("Collector has not been initialized for file "
                            "collection")
        return _pyfixbuf.fBufBase.allocForCollection(self, session, collector)

    def init_export(self, session, exporter):
        """
        Initialize the :class:`Buffer` for Export given the :class:`Session`,
        *session*, and :class:`Exporter`, *exporter*.
        """
        self.session = session
        if (exporter.initialized == 0):
            raise Exception("Exporter has not been initialized for file "
                            " or transport.")
        self.mode = 2
        return _pyfixbuf.fBufBase.allocForExport(self, session, exporter)

    def _init_listener(self, session):
        """
        Internal function to set session on buffer.
        """
        self.session = session
        self.model = session.model
        self.mode = 1

    def set_internal_template(self, template_id):
        """
        The :class:`Buffer` must have an internal template set on it before
        collecting or exporting.  Set the internal template with the given
        template ID.
        """
        if (self.session == None):
            raise Exception("Buffer needs to be initialized for collection "
                            "or export before setting templates.")
        if template_id == 0:
            raise Exception("Invalid Template Id [0]")
        if (template_id not in self.session.internal_templates):
            raise Exception("Template was not added to Session's Internal"
                            " Templates.")
        self.int_tmpl = template_id
        return _pyfixbuf.fBufBase.setInternalTemplate(self, template_id)

    def set_export_template(self, template_id):
        """
        The :class:`Buffer` must have an export template set before appending
        any :class:`Record` to the :class:`Buffer`.  This is how fixbuf will
        transcode the given :class:`Record`.  Set the external template with
        the given template ID.
        """
        if (self.session == None):
            raise Exception("Buffer needs to be initialized for collection "
                            "or export before setting templates.")
        if template_id == 0:
            raise Exception("Invalid Template Id [0]")
        if (self.session.external_templates.get(template_id) == None):
            raise Exception("Template was not added to Session's External "
                            "Templates.")
        self.ext_tmpl = template_id
        return _pyfixbuf.fBufBase.setExportTemplate(self, template_id)

    def __iter__(self):
        """ Iterate through the buffer """
        return self

    def next(self):
        """
        Returns the next :class:`Record` in the buffer.
        Raises :exc:`StopIteration` Exception when done.
        """
        if (self.auto):
            self._auto_generate_template()

        if self.int_tmpl == None:
            raise Exception("No Internal Template Set on Buffer")
        if self.rec == None:
            tmpl = self.session.get_template(self.int_tmpl, True)
            tmpl.build_spec_list()
            self.rec = Record(self.model, tmpl)
        if (self.rec.length == 0):
            raise Exception("No Information Elements in Record")
        self.rec.session = self.session
        _pyfixbuf.fBufBase.nextRecord(self, self.rec)
        return self.rec

    def next_record(self, record=None):
        """
        Get the next record on the buffer in the form of the given
        :class:`Record`, *record*.
        """
        if (self.auto):
            self._auto_generate_template()

        if self.int_tmpl == None:
            raise Exception("No internal template set on buffer")
        if record == None:
            tmpl = self.session.get_template(self.int_tmpl, True)
            tmpl.build_spec_list()
            record = Record(self.model, tmpl)
        if (record.length == 0):
            raise Exception("No Information Elements in Record")
        try:
            _pyfixbuf.fBufBase.nextRecord(self, record)
            record.session = self.session
            return record
        except StopIteration:
            return None


    def set_record(self, record):
        """ Set the given *record* on the buffer """
        self.rec = record
        if (record.template):
            if (record.template.template_id):
                self.set_internal_template(record.template.template_id)
            

    def next_template(self):
        """
        Retrieves the external template that will be used to read the
        next record from the buffer.  If no next record is available,
        returns ``None``.
        """
        template = _pyfixbuf.fBufBase.nextTemplate(self)
        new_template = Template(self.session.model, 0, template)
        return new_template

    def get_template(self):
        """
        Retrieves the external template that was used to read
        the last record from the buffer.  If no record has been read,
        returns ``None``.
        """
        template = _pyfixbuf.fBufBase.getTemplate(self)
        new_template = Template(self.session.model, 0, template)
        return new_template

    def append(self, *args):
        """
        Appends the given *record* on the buffer.  If a second argument,
        *length*, is given, append only the first *length* number  of bytes to
        the buffer.

        An internal and external template must be set on the buffer
        prior to appending an :class:`Record`.
        """
        if self.ext_tmpl == None:
            raise Exception("No External Template Set on Buffer")
        if self.int_tmpl == None:
            raise Exception("No Internal Template Set on Buffer")
        return _pyfixbuf.fBufBase.append(self, *args)

    def write_ie_options_record(self, name, tmpl):
        """
        Appends an Information Element Type Information Record
        on the Buffer.  An Options Record will be written with
        information about the Information Element with the given
        *name*.  *template* is the Information
        Element Type Options Template that was created by giving
        "type=1" to the Template constructor.
        """
        inttid = self.int_tmpl
        exttid = self.ext_tmpl
        if (self.mode == 0):
            raise Exception("This buffer has not been initialized for export.")
        elif (self.mode == 1):
            raise Exception("This buffer is for collection only. It cannot "
                            "write options records.")
        if (not(tmpl.scope)):
            raise Exception("Given Template is not an Options Template.")
        if (tmpl.template_id not in self.session.internal_templates):
            tid = self.session.add_template(tmpl)
        _pyfixbuf.fBufBase.writeOptionsRecord(self, tmpl.infomodel,
                                              name, tmpl.template_id)
        #set templates back to where they were
        if inttid:
            self.set_internal_template(inttid)
        if exttid:
            self.set_export_template(exttid)


    def auto_insert(self):
        """
        Automatically insert any Information Elements that it
        receives Information Element Option Records for.  It will only insert
        information elements that do not have private enterprise number of 0.
        """
        return _pyfixbuf.fBufBase.setAutoInsert(self)


    def ignore_options(self, ignore):
        """
        If *ignore* is set to True, the Buffer will ignore Options
        Templates and Records.  By default, *ignore* is False, and the Buffer will return
        Options Records and the application must use next_template()
        to retrieve the :class:`Template` set on the Buffer and
        determine if it is an Options Template.
        """
        if (ignore < 0):
            raise Exception("Invalid value given to ignore_options.  "
                            "Should be True/False")
        _pyfixbuf.fBufBase.ignoreOptions(self, ignore)


    def _auto_generate_template(self):
        """
        If *auto* is set to True, the buffer will auto-generate internal
        templates from the external templates it receives and set them
        on the buffer before decoding the next IPFIX message.  The 
        :class:`Record` returned from the :class:`Buffer` will match
        the external template retrieved from the exporting message.
        """
        if (self.session == None):
            raise Exception("Buffer must be initialized for collection "
                            "before template can be received.")
        if (self.mode != 1):
            raise Exception("Auto generation for templates only applies "
                            "to Buffers in collection mode.")
        tmpl_next = self.next_template()
        if tmpl_next.template_id == 0:
            # no template on buffer
            return
        if tmpl_next.template_id != self.int_tmpl:
            self.rec = None
        try:
            tmpl = self.session.get_template(tmpl_next.template_id, True)
        except:
            tmpl = self.session.get_template(tmpl_next.template_id)
            tmpl.build_spec_list()
            self.session.add_template(tmpl, tmpl_next.template_id)

        self.set_internal_template(tmpl_next.template_id)
        

class STML(_pyfixbuf.fbSTMLBase):
    """
    A :class:`STML` object represents a subTemplateMultiList.

    If *record*, a :class:`Record` object, and *key_name*, a string, are provided
    the :class:`STML` object with *key_name* will be initialized in the given
    :class:`Record`.  It is only necessary to initialize and give a *type_count* if the
    subTemplateMultiList will be exported.  All subTemplateMultiLists in an
    exported :class:`Record` must be initialized.  It is acceptable to
    initialize an STML to 0 list entries.

    A :class:`STML` must be initialized with *record* and *key_name* OR a *type_count*.
    This object can be used to set a subTemplateMultiList element in a :class:`Record`.

    The subTemplateMultiList is initialized to ``None`` unless it is given
    a *type_count*, in which case it will
    intialize the list and allocate memory in the given record.

    *type_count* is the amount of different templates that the :class:`STML`
    will contain.  For example, if you plan to have an STML with entries of
    type Template ID 999 and 888, *type_count* would be 2.  *type_count* would
    also be 2 even if both instances will use Template ID 999.



    Examples::

    >>> stml = my_rec["subTemplateMultiList"]   # sufficient for collection
    >>> stml = pyfixbuf.STML(rec, "subTemplateMultiList", 3)  # STML with 3 entries for export
    >>> stml = pyfixbuf.STML(type_count=2)
    >>> stml = [record1, record2]
    >>> stml2 = pyfixbuf.STML(type_count=3)
    >>> stml2[0] = [record1, record2]
    >>> stml2[1][0] = record3
    >>> stml2[2].entry_init(record3, tmpl3, 0) #all entries must be init'd - even to 0.
    >>> rec["subTemplateMultiList"] = stml
    """
    def __init__(self, record=None, key_name=None, type_count=-1):
        self.info_model = None
        self.offset = 0
        self.entries = []
        self.rec = None
        self.session = None
        if ((record == None) and (type_count >= 0)):
            _pyfixbuf.fbSTMLBase.__init__(self, None, 0, type_count)
        elif record:
            key_name = record.normalize_key(key_name)
            record.list_init = True
            self.info_model = record.model
            if (key_name not in record.off_dict):
                raise Exception(key_name + " does not exist in record.")
            self.offset = record.off_dict[key_name]
            self.rec = record
            _pyfixbuf.fbSTMLBase.__init__(self, record, self.offset,
                                          type_count)
        else:
            raise Exception("STML must be intialized with either a Record or count")

    def __iter__(self):
        """ Iterator for the SubTemplateMultiList """
        return self

    def next(self):
        """ Returns the next SubTemplateMultiList Entry in the List
        """
        _pyfixbuf.fbSTMLBase.getNextEntry(self, self.rec, self.offset)
        stmlentry = STMLEntry(self)
        return stmlentry

    def clear(self):
        """
        Clear the entries in the subTemplateMultiList and frees any
        memory allocated.
        """
        _pyfixbuf.fbSTMLBase.clear(self, self.rec, self.offset)

    def _clear_stml(self, record, offset):
        _pyfixbuf.fbSTMLBase.clear(self, record, offset)

    def _put_stml_rec(self, record, offset):
        self.rec = record
        self.offset = offset

    def __len__(self):
        """
        Return the number of elements in the :class:`STML`.
        """
        return self.count

    def __contains__(self, name):
        """
        Determine if item with element *name* is in the first entry in
        the :class:`STML`.
        """
        _pyfixbuf.fbSTMLBase.getFirstEntry(self, self.rec, self.offset)
        stmlentry = STMLEntry(self)
        if name in stmlentry:
            rv = True
        else:
            rv = False
        _pyfixbuf.fbSTMLBase.rewind(self)
        return rv

    def __getitem__(self, index):
        """
        Returns the :class:`STMLEntry` at the *index*

        Examples::

        >>> entry = stml[0]
        >>> stml[0].entry_init[record, template, 3]
        """

        if (not(isinstance(index, int))):
            raise TypeError
        if (len(self.entries) == 0):
            i = 0
            while (i < len(self)):
                _pyfixbuf.fbSTMLBase.getIndex(self, i)
                stmlentry = STMLEntry(self)
                i += 1
                self.entries.append(stmlentry)
        return self.entries[index]

    def __setitem__(self, key, value):
        """
        This sets an entry in the STML to the given list of :class:`Record` objects.
        *value* must be a list. All :class:`Records` in the list should have the
        same :class:Template.

        Examples:

        >>> stml[0] = [rec1, rec2, rec3, rec4]

        """
        if (not(isinstance(value, list))):
            raise TypeError
        if (len(value) == 0):
            raise Exception("List must not be empty. Use entry_init to "
                            "initialze STMLEntry to empty list.")
        entry = self[key]
        if (value[0].template):
            entry.entry_init(value[0], value[0].template, len(value))
        else:
            raise Exception("Records in list must have Template "
                            "associated with them, or STMLEntry must "
                            "be initialized with entry_init()")
        i = 0
        while (i < len(value)):
            entry[i] = value[i]
            i += 1



class STMLEntry(_pyfixbuf.fbSTMLEntryBase):
    """
    Creates an empty :class:`STMLEntry` and associates it to the
    given :class:`STML`, *stml*.  There should be one
    :class:`STMLEntry` for each different Template in the
    :class:`STML`.

    Each :class:`STMLEntry` should be initialized using :meth:`entry_init`
    to associate a :class:`Record` and :class:`Template` with the entry.
    """

    def __init__(self, stml):
        """
        Creates an empty subTemplateMultiList Entry and associates it to
        it's outer subTemplateMultiList.
        """
        _pyfixbuf.fbSTMLEntryBase.__init__(self, stml)
        self.info_model = stml.info_model
        self.stml = stml
        self.rec = None
        self.initialized = False
        self.items = 0
        self.tlen = 0
        # this variable keeps track if getNextDatPtr or getNextEntry was called
        # to set the record to the appropriate data buffer for __getitem__
        self.recset = False

    def entry_init(self, record, template, count=0):
        """
        Initializes the :class:`STMLEntry` to the given :class:`Record`,
        *record*, *template*, and *count* instances of the *record* it
        will contain.

        This should only be used for exporting a subTemplateMultiList.
        Entries in the :class:`STML` must all be initialized, even if it
        is initialized to 0.  This method is not necessary if a :class:`Record`
        has a template associated with it. The application can simply set
        the :class:`STMLEntry` to a list of :class:`Records` and the
        :class:`STMLEntry` will automatically be initialized.

        Examples::

        >>> stml = pyfixbuf.STML(my_rec, "subTemplateMultiList", 1)
        >>> stml[0].entry_init(my_rec, template, 2)
        >>> my_rec["sourceTransportPort"] = 3
        >>> stml[0][0] = my_rec
        >>> my_rec["sourceTransportPort"] = 5
        >>> stml[0][1] = my_rec

        """
        if ( len(record.off_dict) != len(template)):
            raise Exception("Invalid Record for Template: Number of Elements"
                            " are Not Equal")
        self.set_record(record)
        self.initialized = True
        self.items = count
        self.template = template.template_id
        self.tlen = len(template)
        self.info_model = record.model
        _pyfixbuf.fbSTMLEntryBase.entryInit(self, record, template,
                                            template.template_id, count)

    def set_record(self, record):
        """
        Set the :class:`Record`, *record*, on the :class:`STMLEntry` to
        access its elements.
        """
        if (not (isinstance(record, Record))):
            raise Exception("Not a valid Record.")
        tid = self.template_id
        if (tid and self.stml.session):
            tmpl = self.stml.session.get_template(tid, False)
            if (not(record.matches_template(tmpl))):
                raise Exception("Invalid Record to Template: Number of Element"
                                "s do not match.")
        self.rec = record

    def set_template(self, template):
        """
        Assign a template to the :class:`STMLEntry`.  The given template
        must be a valid :class:`Template`.

        Use this method as an alternative to entry_init. This is only
        required if the Record that will be assigned to the :class:`STMLEntry`
        was not created with a template.  Using this method instead of
        entry_init will result in only allocating 1 item for the
        :class:`STMLEntry`.
        """
        if (not (isinstance(template, Template))):
            raise Exception("Not a valid Template.")
        self.initialized = True
        self.items = 1
        self.template = template.template_id
        self.tlen = len(template)
        _pyfixbuf.fbSTMLEntryBase.entryInit(self, None, template,
                                            template.template_id, 1)

    def __iter__(self):
        """ Iterator for the STML Entry """
        return self

    def next(self):
        """ Retrieves the next :class:`Record` in the :class:`STMLEntry`.
        
        If a :class:`Record` has not been associated with this :class:`STMLEntry`
        a :class:`Record` will be auto generated using the current :class:`Template`
        set on this :class:`STMLEntry`.
        """
        if (self.rec == None):
            tid = self.template_id
            tmpl = self.stml.session.get_template(tid)
            tmpl.build_spec_list()
            self.rec = Record(self.info_model, tmpl) 
            self.rec.session = self.stml.session
            #raise Exception("No record set on STML Entry")
        _pyfixbuf.fbSTMLEntryBase.getNextRecord(self, self.rec)
        self.recset = True
        self.rec.current = 0
        return self.rec

    def __contains__(self, name):
        """
        Determine if the template associated with this STMLEntry contains
        the Information Element with the given *name*.

        Alternatively, you can access the template ID associated with the
        STMLEntry to determine the type of :class:`Record` that should be used
        to access the elements.
        """
        return _pyfixbuf.fbSTMLEntryBase.containsElement(self, self.info_model,
                                                         name)
    def __len__(self):
        """
        Return the number of elements in the :class:`STMLEntry`.
        """
        return self.count

    def __getitem__(self, item):
        """
        Get the :class:`Record` at the given index.  If item is a name of
        an information element, it will retrieve the value of that element
        for the first :class:`Record` in the :class:`STMLEntry`
        """
        if (self.rec == None):
            if (self.info_model):
                tid = self.template_id
                tmpl = self.stml.session.get_template(tid)
                tmpl.build_spec_list()
                self.rec = Record(self.info_model, tmpl)
            else:
                raise Exception("No record set on STMLEntry")
        elif (self.info_model == None):
            self.info_model = self.rec.model

        newRecord = Record(self.info_model, record=self.rec)
        newRecord.session=self.stml.session
        if (isinstance(item, str) or isinstance(item, tuple)):
            _pyfixbuf.fbSTMLEntryBase.getIndexedEntry(self, newRecord, 0)
            return newRecord[item]
        elif (isinstance(item, int)):
            _pyfixbuf.fbSTMLEntryBase.getIndexedEntry(self, newRecord, item)
            self.recset=True
            return newRecord

    def __setitem__(self, key, value):
        """
        Set the entry item with the given key (index) to the given
        value.  The value should a valid :class:`Record`.

        If the :class:`STMLEntry` was not intialized, it will be initialized to
        1 entry of the :class:`Record`'s :class:`Template`.

        """
        if (not (isinstance(value, Record))):
            raise Exception("Value is not a valid Record.")
        if (self.initialized == False):
            if (self.items == 0):
                if (value.template):
                    self.entry_init(value, value.template, 1)
            else:
               raise Exception("STMLEntry needs initialized for number of items.")

        if (len(value) != self.tlen):
            raise Exception("All Records in STMLEntry must have same Template "
                            "(lengths do not match)")
        self.rec = value
        _pyfixbuf.fbSTMLEntryBase.setIndexedEntry(self, key, value)


class STL(_pyfixbuf.fbSTLBase):
    """
    A :class:`STL` represents a subTemplateList.

    If *record*, a :class:`Record` object, and *key_name*, a string,  are provided,
    the subTemplateList for *key_name* in the given *record* are initialized, 
    otherwise a generic :class:`STL` will be initialized. 
    Eventually a :class:`Template` must be associated with the 
    :class:`STL` for encoding.  

    For decoding, a :class:`Record` must be associated with the :class:`STL`.

    """

    def __init__(self, record=None, key_name=None):
        self.tlen = 0
        self.init = False
        self.session = None
        if (record != None and key_name != None):
            key_name = record.normalize_key(key_name)
            self.info_model = record.model
            if (record.type_dict[key_name] != SUBTEMPLATELIST):
                raise Exception(key_name + " is not a SUBTEMPLATELIST")
            # this is the record that matches the STL
            self.rec = None
            if (not (isinstance(record, Record))):
                raise Exception("First argument must be a valid Record.")
            self.inrec = record
            record.list_init = True
            _pyfixbuf.fbSTLBase.__init__(self, record, record.off_dict[key_name])
        else:
            _pyfixbuf.fbSTLBase.__init__(self)

    def set_record(self, record):
        """
        Set the given *record* on the STL.
        """
        if (not (isinstance(record, Record))):
            raise Exception("Not a valid Record.")
        tid = self.template_id
        if (tid and self.session):
            tmpl = self.session.get_template(tid, False)
            if (not(record.matches_template(tmpl))):
                raise Exception("Invalid Record to Template: Number of "
                                "Elements do not match.")
        self.rec = record
 
    def __contains__(self, name):
        """
        Returns ``True`` if the element with the given *name* exists in the
        :class:`Template` associated with the subTemplateList.
        Returns ``False`` if not present.
        """
        return _pyfixbuf.fbSTLBase.containsElement(self, self.info_model, name)

    def entry_init(self, record, template, count=0):
        """
        Initialize the STL to the given :class:`Record`, *record*, and
        *template* to *count* entries.

        This method should only be used to export a :class:`STL`.

        Each :class:`STL` should be initialized before appending
        the :class:`Record` to the :class:`Buffer` even if it
        is initialized to 0.

        The record that contains the :class:`STL` should not be modified
        after calling entry_init().

        """
        if ( len(record.off_dict) != len(template)):
            raise Exception("Invalid Record for Template: Number of Elements"
                            " are Not Equal")
        self.set_record(record)
        self.tlen = len(template)
        self.init = True
        _pyfixbuf.fbSTLBase.entryInit(self, template, template.template_id, count)

    def __iter__(self):
        """ Iterator for a :class:`STL` """
        return self

    def next(self):
        """ This returns the next record in the :class:`STL` """
        if self.rec == None:
            if self.session:
                tid = self.template_id
                tmpl = self.session.get_template(tid)
                tmpl.build_spec_list()
                self.rec = Record(self.info_model, tmpl)
                self.rec.session = self.session
            else:
                raise Exception("No Record or Session set for STL")
        _pyfixbuf.fbSTLBase.getNext(self, self.rec)
        return self.rec

    def clear(self):
        """
        Clear all entries in the list.  Nested elements should be accessed
        and freed before calling :meth:`clear`.  Frees any memory previously
        allocated for the list.
        """
        _pyfixbuf.fbSTLBase.clear(self)

    def __len__(self):
        """
        Return the number of elements in the :class:`STL`.
        """
        return self.count

    def __getitem__(self, item):
        """
        Get the :class:`Record` at the given index.  If item is a name of
        an information element, it will retrieve the value of that element
        for the last :class:`Record` accessed in the :class:`STL`
        """
        if self.rec == None:
            if self.session:
                tid = self.template_id
                tmpl = self.session.get_template(tid)
                tmpl.build_spec_list()
                self.rec = Record(self.info_model, tmpl)
                self.rec.session = self.session
            else:
                raise Exception("No Record or Session set for STL")

        if (isinstance(item, str) or isinstance(item, tuple)):
            return self.rec[item]
        elif (isinstance(item, int)):
            newRecord = Record(self.rec.model, record=self.rec)
            newRecord.session=self.session
            _pyfixbuf.fbSTLBase.getIndexedEntry(self, newRecord, item)
            return newRecord

    def __setitem__(self, key, value):
        """
        Set the entry item with the given key (index) to the given
        value.  The value should a valid :class:`Record`. If the :class:`STL`
        was not initialized via entry_init(), the :class:`STL` will be
        initialized with the given :class:`Record`'s template and a count of 1.

        """
        if (not (isinstance(value, Record))):
            raise Exception("Value is not a valid Record.")

        if (self.init == False):
            if (value.template):
                self.entry_init(value, value.template, 1)
            else:
                raise Exception("STL has not been initialized. Use entry_init().")

        if (len(value) != self.tlen):
            raise Exception("Record must match Template for this "
                            "STL (lengths do not match)")
        self.rec = value
        _pyfixbuf.fbSTLBase.setIndexedEntry(self, key, value)

class BL(_pyfixbuf.fbBLBase):
    """
    A :class:`BL` represents a basicList.

    A basicList is a list of zero or more instances of an Information Element.

    A basicList can be initialized through a :class:`Record` via init_basic_list(),
    or by creating a :class:`BL` object.

    The constructor requires an :class:`InfoModel` *model*, and a
    :class:`InfoElementSpec`, :class:`InfoElement`, or string *element*.  Additionally,
    it takes an optional integer *count* which represents the number of elements
    in the list, and an optional integer *semantic* to express the relationship among
    the list items.

    All basicLists in a :class:`Record` must be initialized (even to 0) before
    appending a :class:`Record` to a :class:`Buffer`.

    Examples::

    >>> rec.add_element("basicList", BASICLIST)
    >>> rec.add_element("basicList2", BASICLIST)
    >>> bl = BL(model, "sourceTransportPort", 2)
    >>> bl[0] = 80
    >>> bl[1] = 23
    >>> rec["basicList"] = bl
    >>> rec.init_basic_list("basicList2", 4, "octetTotalCount")
    >>> rec["basicList2"] = [99, 101, 104, 23]
    """

    def __init__(self, model, element, count=0, semantic=0):
        self.model = model
        self.list = []
        self.current = 0
        if (not(isinstance(model, InfoModel))):
            raise TypeError("model must be valid InfoModel")
        if (isinstance(element, InfoElement)):
            _pyfixbuf.fbBLBase.__init__(self, element, count, semantic)

        elif (isinstance(element, str)):
            ie = model.get_element(element)
            _pyfixbuf.fbBLBase.__init__(self, ie, count, semantic)

        elif (isinstance(element, InfoElementSpec)):
            ie = model.get_element(element.name)
            _pyfixbuf.fbBLBase.__init__(self, ie, count, semantic)

        else:
            _pyfixbuf.fbBLBase.__init__(self)

    def _fill_bl_list(self):
        if (len(self) and len(self.list) == 0):
            blist = _pyfixbuf.fbBLBase.getitems(self)
            type = self.element.type
            tlist = []
            if type == DataType.IP4ADDR:
                for item in blist:
                    tlist.append(str(ipaddress.IPv4Address(item)))
            if type == DataType.IP6ADDR:
                for item in blist:
                    tlist.append(bytes_to_v6(item))
            if type == DataType.MAC_ADDR:
                for item in blist:
                    tlist.append(':'.join('%02x' % b for b in item))
            else:
                self.list = blist

            if (len(tlist)):
                self.list = tlist

    def __len__(self):
        """ Returns the number of entries in the basicList"""
        return self.count

    def __iter__(self):
        """ Iterate through the basicList """
        return iter(self.list)

    def next(self):
        """ Returns the next item in the basicList """
        self._fill_bl_list()
        if self.current > len(self):
            raise StopIteration
        else:
            item = self.list[self.current]
            self.current += 1
            return item

    def __getitem__(self, index):
        """ Returns the value for the *index* in the basicList """
        self._fill_bl_list()
        return self.list[index]

    def __setitem__(self, key, value):
        if (key > len(self)):
            raise IndexError
        if (self.element == None):
            raise Exception("BL must be initialized with InfoElement"
                            " before setting items.")
        type = self.element.type
        if type == DataType.IP6ADDR:
            value = v6_to_bytes(value)
        if (type == DataType.IP4ADDR and isinstance(value, str)):
            value = int(ipaddress.IPv4Address(value))
        if (type == DataType.MAC_ADDR and isinstance(value, str)):
            value = macToHex(value)

        _pyfixbuf.fbBLBase.setitems(self, key, value)

    def copy(self, other):
        """
        Copy all the items in the list to the :class:`BL`.
        This will only copy up to the length of the :class:`BL`.
        """
        if (not (isinstance(other, list))):
            raise TypeError
        i = 0
        while ((i < len(self)) and (i < len(other))):
            self[i] = other[i]
            i += 1


    def __contains__(self, item):
        """ Returns True if item is in the :class:`BL`, False otherwise """
        self._fill_bl_list()
        if item in self.list:
            return True
        else:
            return False

    def __str__(self):
        self._fill_bl_list()
        return str(self.list)

    def __eq__(self, other):
        if (other == None):
            return False
        if (not(isinstance(other, list))):
            raise TypeError
        if (len(self) != len(other)):
            return False
        self._fill_bl_list()
        i = 0
        for item in self.list:
            if (item != other[i]):
                return False
            i += 1
        return True

class Listener(_pyfixbuf.fbListenerBase):
    """
    Create a :class:`Listener` given a *session*, transport protocol,
    *transport*, *hostname*, and *port* to listen on.

    *session* must be a valid instance of :class:`Session`.

    *hostname* may be a hostname or IP Address.

    *transport* may contain "tcp" or "udp". Default is "tcp."

    *port* should be greater than 1024.  Default is 4739.

    Examples::

    >>> listener = Listener(session, hostname="localhost", port=18000)
    """
    def __init__(self, session, hostname, transport="tcp", port=4739):
        self.session = session
        if (port < 1024 or port == 0):
            raise Exception("Invalid Port: Port should be greater than 1024")
        _pyfixbuf.fbListenerBase.__init__(self)
        _pyfixbuf.fbListenerBase.allocListener(self, transport, hostname,
                                               str(port), session)

    def wait(self, record=None):
        """
        Wait for a connection on the set host and port.
        Returns a newly allocated :class:`Buffer`.

        If a :class:`Record` is given to :meth:`wait` then the returned
        :class:`Buffer` will already be associated with an :class:`Record`.

        If no :class:`Record` is given, you must use :meth:`set_record` on
        the :class:`Buffer` before
        accessing the elements.

        After receiving the :class:`Buffer` you must set the internal template
        on the returned :class:`Buffer` using :meth:`set_internal_template`
        before accessing the data.

        Examples::

        >>> buf = listener.wait()
        >>> buf.set_record(my_rec)
        >>> buf.set_internal_template(999)
        >>> for data in buf:
        >>> ...
        """
        try:
            buf = Buffer(record)
            buf._init_listener(self.session)
            buf = _pyfixbuf.fbListenerBase.listenerWait(self, buf, self.session)
            return buf
        except (KeyboardInterrupt, SystemExit):
            raise Exception("Stopped By User")

###################
# Misc Functions
###################

def check_type(value, dt):

    if type(value) is int:
        if ((dt == DataType.UINT8) or (dt == DataType.UINT16) or
            (dt== DataType.UINT32) or (dt == DataType.UINT64) or
            (dt== DataType.INT8) or (dt == DataType.INT16) or
            (dt == DataType.INT32) or (dt == DataType.INT64) or
            (dt == DataType.SECONDS) or (dt == DataType.MILLISECONDS)
            or (dt == DataType.MICROSECONDS) or
            (dt == DataType.NANOSECONDS)):
            return True
    if type(value) is long:
        if ((dt == DataType.UINT32) or (dt == DataType.UINT64) or
            (dt== DataType.INT32) or (dt == DataType.INT64) or
            (dt == DataType.SECONDS) or (dt == DataType.MICROSECONDS) or
                (dt == DataType.NANOSECONDS) or (dt == DataType.MILLISECONDS)):
                return True
    if type(value) is str:
        if ((dt == DataType.OCTET_ARRAY) or (dt == DataType.STRING)
            or (dt == VARLEN)):
            return True
    if type(value) is float:
        if ((dt == DataType.FLOAT32) or (dt == DataType.FLOAT64)):
            return True

    return False


def bytes_to_v6(bytes):
    return str(ipaddress.IPv6Address(bytes))


def v6_to_bytes(v6):
    return bytes(ipaddress.IPv6Address(v6).packed)


def create_stml_from_list(list):
    listlen = len(list)
    tid = dict()
    tid2list=[]
    i = 0
    difflist = 0
    for item in list:
        if not(isinstance(item, Record)):
            raise TypeError("List must be a list of Records.")
        template = item.template
        if template:
            if template not in tid:
                newlist = []
                newlist.append(item)
                tid[template] = difflist
                tid2list.append(newlist)
                difflist += 1
            else:
                elist = tid2list[tid[template]]
                elist.append(item)
                tid2list[tid[template]] = elist
        else:
            raise Exception("No template associated with item " + str(i+1))

        i += 1

    stml = STML(type_count=len(tid))
    i = 0
    for lt in tid2list:
        stml[i] = lt
        i += 1

    return stml


def stl_template(list):
    for item in list:
        if item.template != None:
            return item.template

    return None

FILTER = ''.join([(len(repr(chr(x)))==3) and chr(x) or '.' for x in range(256)])    
def pyfix_hex_dump(src, length=8):
    newstr=[]
    for i in xrange(0, len(src), length):
        s = src[i:i+length]
        hex = ' '.join(["%02X" % ord(x) for x in s])
        printchar = s.translate(FILTER)
        newstr.append("%04X   %-*s   %s\n" % (i, length*3, hex, printchar))
    return ''.join(newstr)

/*
 * COPYRIGHT 2015 Pluribus Networks Inc.
 *
 * All rights reserved. This copyright notice is Copyright Management
 * Information under 17 USC 1202 and is included to protect this work and
 * deter copyright infringement.  Removal or alteration of this Copyright
 * Management Information without the express written permission from
 * Pluribus Networks Inc is prohibited, and any such unauthorized removal
 * or alteration will be a violation of federal law.
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 */

#ifndef	__NV_NETINET_H
#define	__NV_NETINET_H

#include <netinet/in.h>
#include "nv_types.h"

#ifndef sun
#include <nv_endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define	_BIT_FIELDS_LTOH
#endif

#ifndef _IPADDR_T
#define	_IPADDR_T
typedef uint32_t ipaddr_t;
#endif

#define	ntohll be64toh
#define	htonll htobe64

#endif

#define	NV_IP_SIMPLE_HDR_LENGTH_IN_WORDS   5
#define	NV_IP_SIMPLE_HDR_LENGTH	(NV_IP_SIMPLE_HDR_LENGTH_IN_WORDS << 2)

/*
 * IP Protocols
 */
#define	NV_IPPROTO_ICMP		1
#define	NV_IPPROTO_IGMP		2
#define	NV_IPPROTO_TCP		6
#define	NV_IPPROTO_UDP		17
#define	NV_IPPROTO_OSPF		89
#define	NV_IPPROTO_PIM		103
#define	NV_IPPROTO_VRRP		112

/*
 * Well known port numbers (e.g. BGP)
 */
#define	NV_IPPORT_BGP	179

/*
 * Well known Wake-on-Lan ports
 */
#define NV_WOL_UDP_PORT1 7
#define NV_WOL_UDP_PORT2 9

#ifdef LINUX
/*
 * XXX: not sure this is correct...
 */
#define	IFF_DHCPRUNNING IFF_DYNAMIC
#endif

#define	NV_IPV4_VERSION			4
#define	NV_IP_VERSION			NV_IPV4_VERSION

/* ICMP types */
#define	NV_ICMP_ECHO_REPLY		0
#define	NV_ICMP_DEST_UNREACHABLE	3
#define	NV_ICMP_SOURCE_QUENCH		4
#define	NV_ICMP_REDIRECT		5
#define	NV_ICMP_ECHO_REQUEST		8
#define	NV_ICMP_ROUTER_ADVERTISEMENT	9
#define	NV_ICMP_ROUTER_SOLICITATION	10
#define	NV_ICMP_TIME_EXCEEDED		11
#define	NV_ICMP_PARAM_PROBLEM		12
#define	NV_ICMP_TIME_STAMP_REQUEST	13
#define	NV_ICMP_TIME_STAMP_REPLY	14
#define	NV_ICMP_INFO_REQUEST		15
#define	NV_ICMP_INFO_REPLY		16
#define	NV_ICMP_ADDRESS_MASK_REQUEST	17
#define	NV_ICMP_ADDRESS_MASK_REPLY	18

/* Codes for UNREACH. */
#define	NV_ICMP_NET_UNREACH    0	  /* Network Unreachable	  */
#define	NV_ICMP_HOST_UNREACH	1    /* Host Unreachable	*/
#define	NV_ICMP_PROT_UNREACH	2	/* Protocol Unreachable */
#define	NV_ICMP_PORT_UNREACH	3    /* Port Unreachable	*/
#define	NV_ICMP_FRAG_NEEDED    4	  /* Fragmentation Needed/DF set    */
#define	NV_ICMP_SR_FAILED	 5    /* Source Route failed	    */
#define	NV_ICMP_NET_UNKNOWN    6
#define	NV_ICMP_HOST_UNKNOWN	7
#define	NV_ICMP_HOST_ISOLATED	 8
#define	NV_ICMP_NET_ANO		9
#define	NV_ICMP_HOST_ANO	10
#define	NV_ICMP_NET_UNR_TOS    11
#define	NV_ICMP_HOST_UNR_TOS	12
#define	NV_ICMP_PKT_FILTERED	13    /* Packet filtered */
#define	NV_ICMP_PREC_VIOLATION	  14	/* Precedence violation */
#define	NV_ICMP_PREC_CUTOFF    15	  /* Precedence cut off */

/*
 * Definition of code field values.
 */
#define	NV_ICMP_UNREACH_PORT	3	/* bad port */

#define	NV_IPPORT_DOMAIN	53
#define	NV_IPPORT_BOOTPS	67
#define	NV_IPPORT_BOOTPC	68
#define	NV_IPPORT_DHCPV6C	546
#define	NV_IPPORT_DHCPV6S	547

struct nv_ether_addr {
	uint8_t			ether_addr_octet[6];
};

typedef struct nv_ether_header_s {
	struct nv_ether_addr	ether_dhost;
	struct nv_ether_addr	ether_shost;
	uint16_t		ether_type;
} nv_ether_header_t;

typedef struct nv_ether_vlan_header_s {
	struct nv_ether_addr	ether_dhost;
	struct nv_ether_addr	ether_shost;
	uint16_t		ether_tpid;
	uint16_t		ether_tci;
	uint16_t		ether_type;
} nv_ether_vlan_header_t;

#define	NV_ETHERTYPE_802_MIN	(0x0600)	/* Min valid ethernet type */
						/* under IEEE 802.3 rules */
#define	NV_ETHERTYPE_IP		(0x0800)
#define	NV_ETHERTYPE_ARP	(0x0806)
#define NV_ETHERTYPE_WOL        (0x0842)
#define	NV_ETHERTYPE_REVARP	(0x8035)
#define	NV_ETHERTYPE_VLAN	(0x8100)
#define	NV_ETHERTYPE_IPV6	(0x86dd)	/* IPv6 */
#define	NV_ETHERTYPE_SLOW	(0x8809)	/* Slow Protocol */
#define	NV_ETHERTYPE_QINQ	(0x88a8)
#define	NV_ETHERTYPE_LLDP	(0x88cc)
#define	NV_ETHERTYPE_ECP	(0x8940)
#define	NV_ETHERTYPE_QINQ_OLD	(0x9100)

#define	NV_ETHERMIN		(60)	/* min frame w/header w/o fcs */
#define	NV_ETHERMAX		(1514)	/* max frame w/header w/o fcs */

#define	NV_RTRALERT_LEN		   4

#define	NV_IPV6_DSCP_MASK	(0x0fc00000)

#define	nv_ether_cmp(a, b) (bcmp((caddr_t)a, (caddr_t)b, 6))
#define	nv_ether_copy(a, b) (bcopy((caddr_t)a, (caddr_t)b, 6))

#define	NV_MAC_STP {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00}
#define	NV_MAC_PVST {0x01, 0x00, 0x0C, 0xCC, 0xCC, 0xCD}

typedef uint32_t tcp_seq;

typedef struct nv_tcphdr_s {
	uint16_t	th_sport;	/* source port */
	uint16_t	th_dport;	/* destination port */
	tcp_seq		th_seq;		/* sequence number */
	tcp_seq		th_ack;		/* acknowledgement number */
#ifdef _BIT_FIELDS_LTOH
	uint32_t	th_x2:4,		/* (unused) */
			th_off:4;		/* data offset */
#else
	uint32_t	th_off:4,		/* data offset */
			th_x2:4;		/* (unused) */
#endif
	uchar_t	th_flags;
#define	NV_TH_FIN  0x01
#define	NV_TH_SYN  0x02
#define	NV_TH_RST  0x04
#define	NV_TH_PUSH 0x08
#define	NV_TH_ACK  0x10
#define	NV_TH_URG  0x20
#define	NV_TH_ECE  0x40
#define	NV_TH_CWR  0x80
	uint16_t	th_win;		/* window */
	uint16_t	th_sum;		/* checksum */
	uint16_t	th_urp;		/* urgent pointer */
} nv_tcphdr_t;

typedef struct nv_udphdr_s {
	uint16_t uh_sport;		/* source port */
	uint16_t uh_dport;		/* destination port */
	uint16_t uh_ulen;		/* udp length */
	uint16_t uh_sum;		/* udp checksum */
} nv_udphdr_t;

/* Aligned IP header */
typedef struct nv_ipha_s {
	uint8_t		ipha_version_and_hdr_length;
	uint8_t		ipha_type_of_service;
	uint16_t	ipha_length;
	uint16_t	ipha_ident;
	uint16_t	ipha_fragment_offset_and_flags;
	uint8_t		ipha_ttl;
	uint8_t		ipha_protocol;
	uint16_t	ipha_hdr_checksum;
	ipaddr_t	ipha_src;
	ipaddr_t	ipha_dst;
} nv_ipha_t;

/* ICMP Header Structure */
typedef struct nv_icmph_s {
	uint8_t		icmph_type;
	uint8_t		icmph_code;
	uint16_t	icmph_checksum;
	union {
		struct { /* ECHO request/response structure */
			uint16_t	u_echo_ident;
			uint16_t	u_echo_seqnum;
		} u_echo;
		struct { /* Destination unreachable structure */
			uint16_t	u_du_zero;
			uint16_t	u_du_mtu;
		} u_du;
		struct { /* Parameter problem structure */
			uint8_t		u_pp_ptr;
			uint8_t		u_pp_rsvd[3];
		} u_pp;
		struct { /* Redirect structure */
			ipaddr_t	u_rd_gateway;
		} u_rd;
	} icmph_u;
} nv_icmph_t;

#define	nv_icmph_echo_ident	   icmph_u.u_echo.u_echo_ident
#define	nv_icmph_echo_seqnum	   icmph_u.u_echo.u_echo_seqnum
#define	nv_icmph_du_zero	   icmph_u.u_du.u_du_zero
#define	nv_icmph_du_mtu		   icmph_u.u_du.u_du_mtu
#define	nv_icmph_pp_ptr		   icmph_u.u_pp.u_pp_ptr
#define	nv_icmph_rd_gateway	   icmph_u.u_rd.u_rd_gateway

#define	NV_IP_NETMASK_SLASH32	32
#define	NV_IP_NETMASK_SLASH128	128

/*
 * 802.2 specific declarations
 */
typedef struct nv_llchdr_s {
	unsigned char	llc_dsap;
	unsigned char	llc_ssap;
	unsigned char	llc_ctl;
} nv_llchdr_t;

typedef struct nv_llchdr_xid_s {
	unsigned char	llcx_format;
	unsigned char	llcx_class;
	unsigned char	llcx_window;
} nv_llchdr_xid_t;

typedef struct nv_snaphdr {
	uchar_t		snap_oid[3];
	uchar_t		snap_type[2];
} nv_snaphdr_t;

/*
 * Group Record Types.	The values of these enums match the Record Type
 * field values defined in RFCs 3376 and 3810 for IGMPv3 and MLDv2 reports.
 */
typedef enum {
	NV_MODE_IS_INCLUDE = 1,
	NV_MODE_IS_EXCLUDE,
	NV_CHANGE_TO_INCLUDE,
	NV_CHANGE_TO_EXCLUDE,
	NV_ALLOW_NEW_SOURCES,
	NV_BLOCK_OLD_SOURCES
} nv_mcast_record_t;

#define	NV_IPTOS_PREC_INTERNETCONTROL	   0xc0

/*
 * Definitions for options.
 */

/* Bits in the option value */
#define	NV_IPOPT_COPY		   0x80

#define	NV_IPOPT_COPIED(o)	   ((o)&0x80)
#define	NV_IPOPT_CLASS(o)	   ((o)&0x60)
#define	NV_IPOPT_NUMBER(o)	   ((o)&0x1f)

#define	NV_IPOPT_CONTROL	   0x00
#define	NV_IPOPT_RESERVED1	   0x20
#define	NV_IPOPT_DEBMEAS	   0x40
#define	NV_IPOPT_RESERVED2	   0x60

#define	NV_IPOPT_EOL		   0x00		   /* end of option list */
#define	NV_IPOPT_NOP		   0x01		   /* no operation */

#define	NV_IPOPT_RR		   0x07		   /* record packet route */
#define	NV_IPOPT_RTRALERT	   0x14		   /* router alert */
#define	NV_IPOPT_TS		   0x44		   /* timestamp */
#define	NV_IPOPT_SECURITY	   0x82		   /* provide s,c,h,tcc */
#define	NV_IPOPT_LSRR		   0x83		   /* loose source route */
#define	NV_IPOPT_EXTSEC		   0x85
#define	NV_IPOPT_COMSEC		   0x86
#define	NV_IPOPT_SATID		   0x88		   /* satnet id */
#define	NV_IPOPT_SSRR		   0x89		   /* strict source route */
#define	NV_IPOPT_RA		   0x94
#define	NV_IPOPT_SDMDD		   0x95

/*
 * Message types, including version number.
 */

#define	NV_IGMP_MEMBERSHIP_QUERY	0x11	/* membership query    */
#define	NV_IGMP_V1_MEMBERSHIP_REPORT	0x12	/* Vers.1 membership report */
#define	NV_IGMP_V2_MEMBERSHIP_REPORT	0x16	/* Vers.2 membership report */
#define	NV_IGMP_V3_MEMBERSHIP_REPORT	0x22	/* Vers.3 membership report */
#define	NV_IGMP_V2_LEAVE_GROUP		0x17	/* Leave-group message	    */
#define	NV_IGMP_DVMRP			0x13	/* DVMRP routing message    */
#define	NV_IGMP_PIM			0x14	/* PIM routing message	    */

#define	NV_IGMP_MTRACE_RESP		0x1e	/* traceroute resp to sender */
#define	NV_IGMP_MTRACE			0x1f	/* mcast traceroute messages */

#define	NV_IGMP_MAX_HOST_REPORT_DELAY	10	/* max delay for response to */
						/* query (in seconds)	*/
						/* according to RFC1112 */

#define	NV_IGMP_V3_MAXRT_FPMIN		0x80	/* max resp code fp format */
#define	NV_IGMP_V3_MAXRT_MANT_MASK	0x0f
#define	NV_IGMP_V3_MAXRT_EXP_MASK	0x70

#define	NV_IGMP_V3_SFLAG_MASK		0x8	/* mask off s part of sqrv */
#define	NV_IGMP_V3_RV_MASK		0x7	/* mask off qrv part of sqrv */

#define	NV_IGMP_V3_QQI_FPMIN		0x80	/* qqi code fp format */
#define	NV_IGMP_V3_QQI_MANT_MASK	0x0f
#define	NV_IGMP_V3_QQI_EXP_MASK		0x70

#define	NV_NO_VIF	0xFFFF

/*
 * Internet Group Management Protocol (IGMP) definitions.
 *
 * Written by Steve Deering, Stanford, May 1988.
 * Modified by Rosen Sharma, Stanford, Aug 1994
 * Modified by Bill Fenner, Xerox PARC, April 1995
 *
 * MULTICAST 3.5.1.1
 */

/*
 * IGMP packet format.
 */
typedef struct nv_igmp_s {
	uchar_t		igmp_type;	/* version & type of IGMP message  */
	uchar_t		igmp_code;	/* code for routing sub-msgs	   */
	ushort_t	igmp_cksum;	/* IP-style checksum		   */
	struct in_addr	igmp_group;	/* group address being reported	   */
					/*  (zero for queries)		   */
} nv_igmp_t;

/* IGMPv3 Membership Report common header */
typedef struct nv_igmp3r_s {
	uchar_t		igmp3r_type;	/* version & type of IGMP message  */
	uchar_t		igmp3r_code;	/* code for routing sub-msgs	   */
	ushort_t	igmp3r_cksum;	/* IP-style checksum		   */
	ushort_t	igmp3r_res;	/* Reserved			   */
	ushort_t	igmp3r_numrec;	/* Number of group records	   */
} nv_igmp3r_t;

/* IGMPv3 Group Record header */
typedef struct nv_grphdr_s {
	uchar_t		grphdr_type;	/* type of record		   */
	uchar_t		grphdr_auxlen;	/* auxiliary data length	   */
	ushort_t	grphdr_numsrc;	/* number of sources		   */
	struct in_addr	grphdr_group;	/* group address being reported	   */
} nv_grphdr_t;

/* IGMPv3 Membership Query header */
typedef struct nv_igmp3q_s {
	uchar_t		igmp3q_type;	/* type of IGMP message		   */
	uchar_t		igmp3q_mxrt;	/* maximum response time	   */
	ushort_t	igmp3q_cksum;	/* IP-style checksum		   */
	struct in_addr	igmp3q_group;	/* group address being queried	   */
	uint8_t		igmp3q_sqrv;
	uint8_t		igmp3q_qqic;
	ushort_t	igmp3q_numsrc;	/* number of sources		   */
} nv_igmp3q_t;

#define	NV_IGMP_ROBUSTNESS_DEFAULT	2
#define	NV_IGMP_QQIC_DEFAULT		60	/* 60 Seconds for QQIC */

/*
 * PIM packet format.
 */
typedef struct nv_pim_s {
#ifdef _BIT_FIELDS_LTOH
	uint8_t	pim_type:4,	/* type of PIM message */
		pim_vers:4;	/* PIM version */
#else
	uint8_t	pim_vers:4,	/* PIM version */
		pim_type:4;	/* type of PIM message */
#endif
	uint8_t	pim_reserved;	/* Reserved */
	uint16_t	pim_cksum;	/* IP-style checksum */
} nv_pim_t;

#define	NV_PIM_VERSION	2
#define	NV_PIM_MINLEN	8	/* The header min. length is 8 */

/* Register message + inner IPheader */
#define	NV_PIM_REG_MINLEN	(NV_PIM_MINLEN + NV_IP_SIMPLE_HDR_LENGTH)

/*
 * From the PIM protocol spec (RFC 2362), the following PIM message types
 * are defined.
 */
#define	NV_PIM_HELLO		0x0
#define	NV_PIM_REGISTER		0x1
#define	NV_PIM_REGISTER_STOP	0x2
#define	NV_PIM_JOIN_PRUNE	0x3
#define	NV_PIM_BOOTSTRAP	0x4
#define	NV_PIM_ASSERT		0x5
#define	NV_PIM_GRAFT		0x6
#define	NV_PIM_GRAFT_ACK	0x7
#define	NV_PIM_CAND_RP_ADV	0x8

#define	NV_LLC_XID	   0xAF
#define	NV_LLC_TEST	   0xE3
#define	NV_LLC_P	   0x10	/* P bit for use with XID/TEST */
#define	NV_LLC_XID_FMTID   0x81	/* XID format identifier */
#define	NV_LLC_SERVICES	   0x01	/* Services supported */
#define	NV_LLC_GLOBAL_SAP  0XFF	 /* Global SAP address */
#define	NV_LLC_NULL_SAP	   0x00
#define	NV_LLC_SNAP_SAP	   0x42	/* IEEE Standard SNAP SAP */
#define	NV_LLC_UI	   0x03	/* IEEE Standard UI */
#define	NV_LLC_GROUP_ADDR  0x01 /* indication in DSAP of a group address */
#define	NV_LLC_RESPONSE	   0x01	/* indication in SSAP of a response */
#define	NV_LLC_NOVELL_SAP  -1	/* indicator that Novell 802.3 mode is used */

#define	NV_LLC_XID_INFO_SIZE	   3	   /* length of the INFO field */
#define	NV_LLC_XID_CLASS_I	   (0x01)  /* Class I */
#define	NV_LLC_XID_CLASS_II	   (0x03)  /* Class II */
#define	NV_LLC_XID_CLASS_III	   (0x05)  /* Class III */
#define	NV_LLC_XID_CLASS_IV	   (0x07)  /* Class IV */

/* Types can be or'd together */
#define	NV_LLC_XID_TYPE_1	   (0x01)  /* Type 1 */
#define	NV_LLC_XID_TYPE_2	   (0x02)  /* Type 2 */
#define	NV_LLC_XID_TYPE_3	   (0x04)  /* Type 3 */

typedef struct nv_arphdr_s {
	ushort_t ar_hrd;	/* format of hardware address */
#define	NV_ARPHRD_ETHER	   1	   /* ethernet hardware address */
#define	NV_ARPHRD_IEEE802  6	   /* IEEE 802 hardware address */
#define	NV_ARPHRD_FRAME	   15	   /* Frame relay */
#define	NV_ARPHRD_ATM	   16	   /* ATM */
#define	NV_ARPHRD_HDLC	   17	   /* HDLC */
#define	NV_ARPHRD_FC	   18	   /* Fibre Channel RFC 4338 */
#define	NV_ARPHRD_IPATM	   19	   /* ATM RFC 2225 */
#define	NV_ARPHRD_TUNNEL   31	   /* IPsec Tunnel RFC 3456 */
#define	NV_ARPHRD_IB	   32	   /* IPoIB hardware address */
	ushort_t ar_pro;	/* format of protocol address */
	uchar_t ar_hln;		/* length of hardware address */
	uchar_t ar_pln;		/* length of protocol address */
	ushort_t ar_op;		/* one of: */
#define	NV_ARPOP_REQUEST   1	   /* request to resolve address */
#define	NV_ARPOP_REPLY	   2	   /* response to previous request */
#define	NV_REVARP_REQUEST  3	   /* Reverse ARP request */
#define	NV_REVARP_REPLY	   4	   /* Reverse ARP reply */
	/*
	 * The remaining fields are variable in size,
	 * according to the sizes above, and are defined
	 * as appropriate for specific hardware/protocol
	 * combinations.  (E.g., see <netinet/if_ether.h>.)
	 */
#ifdef	notdef
	uchar_t ar_sha[];	/* sender hardware address */
	uchar_t ar_spa[];	/* sender protocol address */
	uchar_t ar_tha[];	/* target hardware address */
	uchar_t ar_tpa[];	/* target protocol address */
#endif	/* notdef */
} nv_arphdr_t;

typedef struct nv_ether_arp_s {
	nv_arphdr_t ea_hdr;		/* fixed-size header */
	struct nv_ether_addr arp_sha;	/* sender hardware address */
	uchar_t arp_spa[4];		/* sender protocol address */
	struct nv_ether_addr arp_tha;	/* target hardware address */
	uchar_t arp_tpa[4];		/* target protocol address */
} nv_ether_arp_t;

#define	IPV6_HAS_EXT_HDR(nxthdr) \
	((nxthdr == IPPROTO_HOPOPTS) || \
	(nxthdr == IPPROTO_DSTOPTS) || \
	(nxthdr == IPPROTO_ROUTING) || \
	(nxthdr == IPPROTO_FRAGMENT) || \
	(nxthdr == IPPROTO_AH) || \
	(nxthdr == IPPROTO_ESP))

#define	nv_arp_hrd ea_hdr.ar_hrd
#define	nv_arp_pro ea_hdr.ar_pro
#define	nv_arp_hln ea_hdr.ar_hln
#define	nv_arp_pln ea_hdr.ar_pln
#define	nv_arp_op  ea_hdr.ar_op

#define	NV_IPV6_VERSION			6
#define	NV_IPV6_VFC			(NV_IPV6_VERSION << 4)
/*
 * NV_IPV6_VCF:
 * 4 bits version = 6
 * 8 bits tclass = 0
 * 20 bits flow-id = 0
 */
#define	NV_IPV6_VCF			(NV_IPV6_VERSION << 28)

/* Definitions for IPv6 */
typedef struct	nv_ip6_hdr {
	union {
		struct nv_ip6_hdrctl {
			uint32_t	ip6_un1_flow;   /* 4 bits version, */
							/* 8 bits tclass, and */
							/* 20 bits flow-ID */
			uint16_t	ip6_un1_plen;   /* payload length */
			uint8_t		ip6_un1_nxt;    /* next header */
			uint8_t		ip6_un1_hlim;   /* hop limit */
		} ip6_un1;
		uint8_t	ip6_un2_vfc;	/* 4 bits version and */
					/* top 4 bits of tclass */
	} nv_ip6_ctlun;
	struct in6_addr ip6_src;	/* source address */
	struct in6_addr ip6_dst;	/* destination address */
} nv_ip6ha_t;

#define	nv_ip6_vfc	nv_ip6_ctlun.ip6_un2_vfc /* 4 bits version and */
						/* top 4 bits of tclass */
#define	nv_ip6_flow	nv_ip6_ctlun.ip6_un1.ip6_un1_flow
#define	nv_ip6_vcf	nv_ip6_flow		/* Version, tclass, flow-ID */
#define	nv_ip6_plen	nv_ip6_ctlun.ip6_un1.ip6_un1_plen
#define	nv_ip6_nxt	nv_ip6_ctlun.ip6_un1.ip6_un1_nxt
#define	nv_ip6_hlim	nv_ip6_ctlun.ip6_un1.ip6_un1_hlim
#define	nv_ip6_hops	nv_ip6_ctlun.ip6_un1.ip6_un1_hlim

/* Hop-by-Hop options header */
typedef struct nv_ip6_hbh {
	uint8_t	ip6h_nxt;	/* next header */
	uint8_t	ip6h_len;	/* length in units of 8 octets */
		/* followed by options */
} nv_ip6_hbh_t;

/* Destination options header */
typedef struct nv_ip6_dest {
	uint8_t	ip6d_nxt;	/* next header */
	uint8_t	ip6d_len;	/* length in units of 8 octets */
		/* followed by options */
} nv_ip6_dest_t;

/* Routing header */
typedef struct nv_ip6_rthdr {
	uint8_t	ip6r_nxt;	/* next header */
	uint8_t	ip6r_len;	/* length in units of 8 octets */
	uint8_t	ip6r_type;	/* routing type */
	uint8_t	ip6r_segleft;	/* segments left */
		/* followed by routing type specific data */
} nv_ip6_rthdr_t;

/* Type 0 Routing header */
typedef struct nv_ip6_rthdr0 {
	uint8_t	ip6r0_nxt;		/* next header */
	uint8_t	ip6r0_len;		/* length in units of 8 octets */
	uint8_t	ip6r0_type;		/* always zero */
	uint8_t	ip6r0_segleft;		/* segments left */
	uint32_t ip6r0_reserved;	/* reserved field */
} nv_ip6_rthdr0_t;

/* Fragment header */
typedef struct nv_ip6_frag {
	uint8_t		ip6f_nxt;	/* next header */
	uint8_t		ip6f_reserved;	/* reserved field */
	uint16_t	ip6f_offlg;	/* offset, reserved, and flag */
	uint32_t	ip6f_ident;	/* identification */
} nv_ip6_frag_t;

/* ip6f_offlg field related constants (in network byte order) */
#ifdef _BIG_ENDIAN
#define	NV_IP6F_OFF_MASK	0xfff8	/* mask out offset from _offlg */
#define	NV_IP6F_RESERVED_MASK	0x0006	/* reserved bits in ip6f_offlg */
#define	NV_IP6F_MORE_FRAG		0x0001	/* more-fragments flag */
#else
#define	NV_IP6F_OFF_MASK	0xf8ff	/* mask out offset from _offlg */
#define	NV_IP6F_RESERVED_MASK	0x0600	/* reserved bits in ip6f_offlg */
#define	NV_IP6F_MORE_FRAG	0x0100	/* more-fragments flag */
#endif

/* IPv6 options */
typedef struct	nv_ip6_opt {
	uint8_t	ip6o_type;
	uint8_t	ip6o_len;
} nv_ip6_opt_t;

/*
 * Definitions for ICMPv6
 * This includes definitions for ICMPv6 subtype MLD
 */

/*
 * Type and code definitions for ICMPv6.
 * Based on RFC2292.
 */

#define	ICMP6_INFOMSG_MASK		0x80 /* all informational messages */

/* Minimum ICMPv6 header length. */
#define	ICMP6_MINLEN	8

typedef struct nv_icmp6_hdr {
	uint8_t	 icmp6_type;	/* type field */
	uint8_t	 icmp6_code;	/* code field */
	uint16_t icmp6_cksum;	/* checksum field */
	union {
		uint32_t icmp6_un_data32[1];	/* type-specific field */
		uint16_t icmp6_un_data16[2];	/* type-specific field */
		uint8_t	 icmp6_un_data8[4];	/* type-specific field */
	} icmp6_dataun;
} nv_icmp6_t;

#define	icmp6_data32	icmp6_dataun.icmp6_un_data32
#define	icmp6_data16	icmp6_dataun.icmp6_un_data16
#define	icmp6_data8	icmp6_dataun.icmp6_un_data8
#define	icmp6_pptr	icmp6_data32[0]	/* parameter prob */
#define	icmp6_mtu	icmp6_data32[0]	/* packet too big */
#define	icmp6_id	icmp6_data16[0]	/* echo request/reply */
#define	icmp6_seq	icmp6_data16[1]	/* echo request/reply */
#define	icmp6_maxdelay	icmp6_data16[0]	/* mcast group membership */

#define	NV_ICMPV6_DEST_UNREACHABLE	1
#define	NV_ICMPV6_ECHO_REQ	128
#define	NV_ICMPV6_ECHO_REPLY	129

/*
 * Codes for Destination Unreachable
 */
#define	NV_ICMPV6_NOROUTE	0
#define	NV_ICMPV6_ADM_PROHIBITED	1
#define	NV_ICMPV6_NOT_NEIGHBOUR	2
#define	NV_ICMPV6_ADDR_UNREACH	3
#define	NV_ICMPV6_PORT_UNREACH	4
#define	NV_ICMPV6_POLICY_FAIL	5
#define	NV_ICMPV6_REJECT_ROUTE	6

/*
 * Defines related to IPv6 ND (Neighbor Discovery)
 * Note ND packets are ICMPv6 packets
 */
#define	NV_IPV6_ND_RS		133 /* Router Solicitation */
#define	NV_IPV6_ND_RA		134 /* Router Advertisement */
#define	NV_IPV6_ND_NS		135 /* Neighbor Solicitation */
#define	NV_IPV6_ND_NA		136 /* Neighbor Advertisement */
#define	NV_IPV6_ND_RDT		137 /* Redirect */
/*
 * Link addr type in optional hdr
 */
#define	NV_OPT_ND_TARGET_LINKADDR	2

/* ICMPv6 common */
typedef struct nv_icmp6_pkt {
	nv_icmp6_t		hdr;
	struct in6_addr		addr;
} nv_icmp6_pkt_t;

/* IPv6 ND packets */
typedef nv_icmp6_pkt_t nv_nd_pkt_t;

#define	nv_nd_rso	icmp6_data8[0]

#define	NV_ND_ICMP6_HDR_R_BIT		0x80 /* Router flag */
#define	NV_ND_ICMP6_HDR_S_BIT		0x40 /* Solicited flag */
#define	NV_ND_ICMP6_HDR_O_BIT		0x20 /* Override flag */

typedef struct nv_nd_option {
	uint8_t			type;
	uint8_t			length;
	uint8_t			value[0];
} nv_nd_option_t;

typedef struct nv_nd_source_ll_opt {
	nv_nd_option_t		hdr;
	uint8_t			source_ll_addr[6];
} nv_nd_source_ll_opt_t;

typedef struct nv_nd_target_ll_opt {
	nv_nd_option_t		hdr;
	uint8_t			target_ll_addr[6];
} nv_nd_target_ll_opt_t;

/*
 * IPv6 ND messages
 * Only interested in NA/NS now. Ignore RDT etc.
 */
#define		IPV6_ND_PACKET(type)	\
	((type == NV_IPV6_ND_NS) || \
	(type == NV_IPV6_ND_NA))

#define		IPV6_RS_PACKET(type)	\
	(type == NV_IPV6_ND_RS)

#define		IPV6_RA_PACKET(type)	\
	(type == NV_IPV6_ND_RA)

#define		IPV6_ND_OPTION_SOURCE_LL_ADDR		1
#define		IPV6_ND_OPTION_TARGET_LL_ADDR		2
#define		IPV6_ND_OPTION_PREFIX_INFO		3
#define		IPV6_ND_OPTION_REDIRECTED_HDR		4
#define		IPV6_ND_OPTION_MTU			5

/* Multicast Listener Discovery messages (RFC 3542 (v1), RFC 3810 (v2)). */
#define		IPV6_MLD_PACKET(type)    \
	((type == NV_MLD_LISTENER_QUERY) || \
	(type == NV_MLD_LISTENER_REPORT) || \
	(type == NV_MLD_LISTENER_REDUCTION) || \
	(type == NV_MLD_V2_LISTENER_REPORT))

#define	NV_MLD_MINLEN		24
#define	NV_MLD_V2_QUERY_MINLEN	28

/* Query Header, common to v1 and v2 */
typedef nv_icmp6_pkt_t nv_mld_hdr_t;

#define	nv_mld_type	hdr.icmp6_type
#define	nv_mld_code	hdr.icmp6_code
#define	nv_mld_cksum	hdr.icmp6_cksum
#define	nv_mld_maxdelay	hdr.icmp6_data16[0]
#define	nv_mld_reserved	hdr.icmp6_data16[1]

/* MLDv2 query */
typedef struct nv_mld2q {
	nv_mld_hdr_t	mld2q_hdr;
	uint8_t		mld2q_sqrv;	/* S Flag, Q's Robustness Variable  */
	uint8_t		mld2q_qqic;	/* Querier's Query Interval Code    */
	uint16_t	mld2q_numsrc;	/* number of sources		    */
} nv_mld2q_t;

#define	mld2q_type	mld2q_hdr.hdr.icmp6_type
#define	mld2q_code	mld2q_hdr.hdr.icmp6_code
#define	mld2q_cksum	mld2q_hdr.hdr.icmp6_cksum
#define	mld2q_mxrc	mld2q_hdr.hdr.icmp6_data16[0]
#define	mld2q_addr	mld2q_hdr.addr

#define	NV_MLD_V2_SFLAG_MASK		0x8	/* mask off s part of sqrv */
#define	NV_MLD_V2_RV_MASK		0x7	/* mask off qrv part of sqrv */

/* definitions used to extract max response delay from mrc field */
#define	NV_MLD_V2_MAXRT_FPMIN   	0x8000
#define	NV_MLD_V2_MAXRT_MANT_MASK	0x0fff
#define	NV_MLD_V2_MAXRT_EXP_MASK	0x7000

/* definitions used to extract querier's query interval from qqic field */
#define	NV_MLD_V2_QQI_FPMIN	0x80
#define	NV_MLD_V2_QQI_MANT_MASK	0x0f
#define	NV_MLD_V2_QQI_EXP_MASK	0x70

/* MLDv2 response */
typedef nv_icmp6_t		nv_mld2r_t;

#define	mld2r_type	icmp6_type
#define	mld2r_res	icmp6_code
#define	mld2r_cksum	icmp6_cksum
#define	mld2r_res1	icmp6_data16[0]
#define	mld2r_nummar	icmp6_data16[1]

/* MLDv2 multicast address record */
typedef struct nv_mld2mar {
	uint8_t		mld2mar_type;	/* type of record		    */
	uint8_t		mld2mar_auxlen;	/* auxiliary data length	    */
	uint16_t	mld2mar_numsrc;	/* number of sources		    */
	struct in6_addr	mld2mar_group;	/* group address being reported	    */
} nv_mld2mar_t;

/*
 * ICMPv6 group membership types
 */
#define	NV_MLD_LISTENER_QUERY		130
#define	NV_MLD_LISTENER_REPORT		131
#define	NV_MLD_LISTENER_REDUCTION	132
#define	NV_MLD_V2_LISTENER_REPORT	143	/* V2 membership report */
						/* This has includes and */
						/* excludes list which takes */
						/* care of Leave/Done for V2 */
#define	NV_MLD_ROBUSTNESS_DEFAULT	2
#define	NV_MLD_QQIC_DEFAULT		60	/* 60 Seconds for QQIC */


#define	NV_IP6OPT_ROUTER_ALERT		0x05	/* 00 0 00101 */
#define	NV_IP6_ALERT_MLD		0x0000

/* Router Alert Option */
typedef struct	nv_ip6_opt_router {
	nv_ip6_hbh_t  hbh_op_hdr;
	uint8_t	ip6or_type;
	uint8_t	ip6or_len;
	uint16_t ralert_value; /* 2 bytes for data */
	uint16_t ralert_pad; /* + 2 bytes padding */
} nv_ip6_opt_router_t;

#define	NV_ICMPV6_RA_FLAG_M_BIT	0x80 /* Managed address configuration flag */
#define	NV_ICMPV6_RA_FLAG_O_BIT	0x40 /* Other configuration flag */

typedef struct nv_icmp6_ra_pkt_s {
	uint8_t		ra_type;
	uint8_t		ra_code;
	uint16_t	ra_chksum;
	uint8_t		ra_curhop;
	uint8_t		ra_flags;
	uint16_t	ra_lifetime;
	uint32_t	ra_rtime;
	uint32_t	ra_retrasmit;
} nv_icmp6_ra_pkt_t;

#define	NV_ICMPV6_RA_PRF_FLAG_RIGHT	(1 << 3)
#define	NV_ICMPV6_RA_PRF_FLAG_LEFT	(1 << 4)
#define	NV_ICMPV6_RA_PREF_FLAG_L_BIT	0x80 /* on-link flag */
#define	NV_ICMPV6_RA_PREF_FLAG_A_BIT	0x40 /* auto-conf flag */
#define	NV_ICMPV6_RA_PREFIX_TYPE	3
#define	NV_ICMPV6_RA_VALID_LIFETIME	2592000 /* as per quagga defaults */
#define	NV_ICMPV6_RA_PREFERRED_LIFETIME	604800 /* as per quagga defaults */

typedef struct nv_icmp6_ra_prefix_opt_s {
	uint8_t		ra_pref_type;
	uint8_t		ra_pref_len;
	uint8_t		ra_pref_plen;
	uint8_t		ra_pref_flags;
	uint32_t	ra_pref_lifetime;
	uint32_t	ra_pref_plifetime;
	uint32_t	ra_pref_reserved;
	struct in6_addr	ra_pref_prefix;
} nv_icmp6_ra_prefix_opt_t;


#endif	/* __NV_NETINET_H */

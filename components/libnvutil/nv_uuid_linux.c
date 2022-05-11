/*
 * COPYRIGHT 2012 Pluribus Networks Inc.
 *
 * All rights reserved. This copyright notice is Copyright Management
 * Information under 17 USC 1202 and is included to protect this work and
 * deter copyright infringement.  Removal or alteration of this Copyright
 * Management Information without the express written permission from
 * Pluribus Networks Inc is prohibited, and any such unauthorized removal
 * or alteration will be a violation of federal law.
 */

/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
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
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * The copyright in this file is taken from the original Leach & Salz
 * UUID specification, from which this implementation is derived.
 */

/*
 * Copyright (c) 1990- 1993, 1996 Open Software Foundation, Inc.
 * Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, Ca. &
 * Digital Equipment Corporation, Maynard, Mass.  Copyright (c) 1998
 * Microsoft.  To anyone who acknowledges that this file is provided
 * "AS IS" without any express or implied warranty: permission to use,
 * copy, modify, and distribute this file for any purpose is hereby
 * granted without fee, provided that the above copyright notices and
 * this notice appears in all source code copies, and that none of the
 * names of Open Software Foundation, Inc., Hewlett-Packard Company,
 * or Digital Equipment Corporation be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.  Neither Open Software
 * Foundation, Inc., Hewlett-Packard Company, Microsoft, nor Digital
 * Equipment Corporation makes any representations about the
 * suitability of this software for any purpose.
 */

#include <inttypes.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nv_types.h"
#include "nv_string.h"
#include "libnvutil.h"
#include "nv_uuid.h"

#define	URANDOM	"/dev/urandom"

struct uuid {
	uint32_t	time_low;
	uint16_t	time_mid;
	uint16_t	time_hi_and_version;
	uint8_t		clock_seq_hi_and_reserved;
	uint8_t		clock_seq_low;
	uint8_t		node_addr[6];
};

/*
 * Unpacks the structure members in "struct uuid" to a char string "uuid_t".
 */
void
struct_to_string(nv_uuid_t ptr, struct uuid *uu)
{
	uint_t		tmp;
	uchar_t		*out = ptr;

	tmp = uu->time_low;
	out[3] = (uchar_t)tmp;
	tmp >>= 8;
	out[2] = (uchar_t)tmp;
	tmp >>= 8;
	out[1] = (uchar_t)tmp;
	tmp >>= 8;
	out[0] = (uchar_t)tmp;

	tmp = uu->time_mid;
	out[5] = (uchar_t)tmp;
	tmp >>= 8;
	out[4] = (uchar_t)tmp;

	tmp = uu->time_hi_and_version;
	out[7] = (uchar_t)tmp;
	tmp >>= 8;
	out[6] = (uchar_t)tmp;

	tmp = uu->clock_seq_hi_and_reserved;
	out[8] = (uchar_t)tmp;
	tmp = uu->clock_seq_low;
	out[9] = (uchar_t)tmp;

	(void) memcpy(out+10, uu->node_addr, 6);
}

/*
 * Packs the values in the "nv_uuid_t" string into "struct uuid".
 */
void
string_to_struct(struct uuid *uuid, nv_uuid_t in)
{
	uchar_t	*ptr;
	uint_t	tmp;

	ptr = in;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uuid->time_low = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uuid->time_mid = tmp;

	tmp = *ptr++;
	tmp = (tmp << 8) | *ptr++;
	uuid->time_hi_and_version = tmp;

	tmp = *ptr++;
	uuid->clock_seq_hi_and_reserved = tmp;

	tmp = *ptr++;
	uuid->clock_seq_low = tmp;

	(void) memcpy(uuid->node_addr, ptr, 6);
}

/*
 * This function converts the supplied UUID uu from the internal
 * binary format into a 36-byte string (plus trailing null char)
 * and stores this value in the character string pointed to by out.
 */
void
nv_uuid_unparse(nv_uuid_t uu, char *out)
{
	struct uuid 	uuid;
	uint16_t	clock_seq;
	char		etheraddr[13];
	int		index = 0, i;

	/* basic sanity checking */
	if (uu == NULL) {
		return;
	}

	/* XXX user should have allocated enough memory */
	/*
	 * if (strlen(out) < NV_UUID_PRINTABLE_STRING_LENGTH) {
	 * return;
	 * }
	 */
	string_to_struct(&uuid, uu);
	clock_seq = uuid.clock_seq_hi_and_reserved;
	clock_seq = (clock_seq  << 8) | uuid.clock_seq_low;
	for (i = 0; i < 6; i++) {
		(void) sprintf(&etheraddr[index++], "%.2x", uuid.node_addr[i]);
		index++;
	}
	etheraddr[index] = '\0';

	(void) snprintf(out, 25, "%08x-%04x-%04x-%04x-",
	    uuid.time_low, uuid.time_mid, uuid.time_hi_and_version, clock_seq);
	(void) strlcat(out, etheraddr, NV_UUID_PRINTABLE_STRING_LENGTH);
}

/*
 * uuid_parse converts the UUID string given by 'in' into the
 * internal uuid_t format. The input UUID is a string of the form
 * cefa7a9c-1dd2-11b2-8350-880020adbeef in printf(3C) format.
 * Upon successfully parsing the input string, UUID is stored
 * in the location pointed to by uu
 */
int
nv_uuid_parse(char *in, nv_uuid_t uu)
{
	char		*ptr, buf[3];
	int		i;
	struct uuid	uuid;
	uint16_t	clock_seq;

	/* do some sanity checking */
	if ((strlen(in) != 36) || (uu == NULL) || (in[36] != '\0')) {
		return (-1);
	}

	ptr = in;
	for (i = 0; i < 36; i++, ptr++) {
		if ((i == 8) || (i == 13) || (i == 18) || (i == 23)) {
			if (*ptr != '-') {
				return (-1);
			}
		} else {
			if (!isxdigit(*ptr)) {
				return (-1);
			}
		}
	}

	uuid.time_low = strtoul(in, NULL, 16);
	uuid.time_mid = strtoul(in+9, NULL, 16);
	uuid.time_hi_and_version = strtoul(in+14, NULL, 16);
	clock_seq = strtoul(in+19, NULL, 16);
	uuid.clock_seq_hi_and_reserved = (clock_seq & 0xFF00) >> 8;
	uuid.clock_seq_low = (clock_seq & 0xFF);

	ptr = in+24;
	buf[2] = '\0';
	for (i = 0; i < 6; i++) {
		buf[0] = *ptr++;
		buf[1] = *ptr++;
		uuid.node_addr[i] = strtoul(buf, NULL, 16);
	}
	struct_to_string(uu, &uuid);
	return (0);
}

void
nv_uuid_generate(nv_uuid_t nv_uuid)
{
	struct uuid n_uuid;
	int fd, i;
	int nbytes = 0;
	time_t	t;
	uchar_t *buf = nv_uuid;
	(void) memset(nv_uuid, 0, sizeof (nv_uuid_t));
	(void) memset(&n_uuid, 0, sizeof (struct uuid));
	fd = open(URANDOM, O_RDONLY);
	if (fd >= 0) {
		while (nbytes < sizeof (nv_uuid_t)) {
			i = read(fd, buf, sizeof (nv_uuid_t));
			if (i == 0 && errno == EINTR) {
				break;
			}
			nbytes += i;
			buf += i;
		}
		(void) close(fd);
	}
	if (nbytes < sizeof (nv_uuid_t)) {
		srand((unsigned)time(&t));
		for (i = 0; i < sizeof (nv_uuid_t); i++) {
			*buf++ = rand() & 0xFF;
		}
	}
	string_to_struct(&n_uuid, nv_uuid);
	/*
	 * version 4: Randomly generated version.
	 */
	n_uuid.time_hi_and_version |= (1 << 14);
	/*
	 * Apply Version 1 Mask.
	 */
	n_uuid.time_hi_and_version &= 0xefff;
	/*
	 * Varient set to 1(MSB0) 0 (MSB1) specified in spec rfc4122
	 */
	n_uuid.clock_seq_hi_and_reserved |= 0x80;
	/*
	 * Set MSB of Ethernet address to 1 to indicate it was generated
	 * randomly
	 */
	n_uuid.node_addr[0] |= 0x80;
	struct_to_string(nv_uuid, &n_uuid);
}

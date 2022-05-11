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
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#include "nv_memory.h"
#include "nv_getpass.h"

static int intrupt;

static void
catch(int x)
{
	intrupt = 1;
}

char *
nv_getpassphrase(const char *prompt, char *buf, int size)
{
	struct termios oflags, nflags;
	char *p;
	int c;
	FILE	*fi;
	struct sigaction act, osigint, osigtstp, osigwinch;

	if ((fi = fopen("/dev/tty", "r+F")) == NULL)
		return (NULL);
	setbuf(fi, NULL);

	intrupt = 0;
	act.sa_flags = 0;
	act.sa_handler = catch;
	(void) sigemptyset(&act.sa_mask);
	(void) sigaction(SIGINT, &act, &osigint);	/* trap interrupt */
	act.sa_handler = SIG_IGN;
	(void) sigaction(SIGTSTP, &act, &osigtstp);	/* ignore stop */
	(void) sigaction(SIGWINCH, &act, &osigwinch);	/* ignore window change */
	tcgetattr(fileno(stdin), &oflags);
	nflags = oflags;
	nflags.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
		return (NULL);
	}

	(void) fputs(prompt, fi);
	p = buf;
	while (!intrupt &&
	    (c = getc(fi)) != '\n' && c != '\r' && c != EOF) {
		if (p < &buf[ size ])
			*p++ = (char)c;
	}
	*p = '\0';
	(void) putc('\n', fi);

	if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
		printf("whoops\n");
	}
	(void) sigaction(SIGINT, &osigint, NULL);
	(void) sigaction(SIGTSTP, &osigtstp, NULL);
	(void) sigaction(SIGWINCH, &osigwinch, NULL);
	(void) fclose(fi);
	if (intrupt) {		/* if interrupted erase the input */
		buf[0] = '\0';
		(void) kill(getpid(), SIGINT);
	}
	return (buf);
}

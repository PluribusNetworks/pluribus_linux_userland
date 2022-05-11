/*
 * COPYRIGHT 2015 Pluribus Networks Inc.
 *
 * All rights reserved. This copyright notice is Copyright Management
 * Information under 17 USC 1202 and is included to protect this work and
 * deter copyright infringement.  Removal or alteration of this Copyright
 * Management Information without the express written permission from
 * Pluribus Networks Inc is prohibited, and any such unauthorized removal
 * or alteration will be a violation of federal law.
 */

#ifndef NSS_NVOS_H
#define	NSS_NVOS_H

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdarg.h>

#include "nvOS.h"
#include "nv_slist.h"
#include "nvc_client.h"

typedef void (*make_ent_func_t)(vuser_info_t *user, char *buf, int size);

extern boolean_t nvos_nss_g_debug;
extern boolean_t nvos_nss_g_enabled;

void nvos_nss_set_debug(void);
void nvos_nss_dbg_print(
	const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void nvos_nss_dbg_print(const char *fmt, ...);

vuser_info_t *nvos_nss_userop(vuser_info_t *, int);
vuser_info_t *nvos_nss_vuser_getbyname(const char *, vuser_info_t *);
void nvos_nss_make_passwd_ent(vuser_info_t *, char *, int);
void nvos_nss_make_shadow_ent(vuser_info_t *, char *, int);

void nvos_nss_get_groups(nv_slist_t *groups);
int nvos_nss_get_group_by_name(const char *name, nvc_nss_group_t *group);
int nvos_nss_get_group_by_gid(gid_t gid, nvc_nss_group_t *group);

#endif /* NSS_NVOS_H */

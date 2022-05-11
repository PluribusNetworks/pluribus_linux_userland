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
#include "nss_nvos.h"
#include "nvc_client.h"

boolean_t nvos_nss_g_debug = B_FALSE;
boolean_t nvos_nss_g_enabled = B_TRUE;

void
nvos_nss_set_debug(void)
{
	nvos_nss_g_debug = (access("/tmp/nss_nvos.debug", F_OK) == 0);
}

void
nvos_nss_dbg_print(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

void
nvos_nss_dbg_print(const char *fmt, ...)
{
	va_list alist;

	if (!nvos_nss_g_debug) {
		return;
	}

	va_start(alist, fmt);
	(void) vfprintf(stderr, fmt, alist);
	va_end(alist);
}

vuser_info_t *
nvos_nss_vuser_getbyname(const char *name, vuser_info_t *filter)
{
	uint64_t	uid;

	nvos_nss_dbg_print("%s\n", __func__);

	assert(filter != NULL);
	if (nv_strtoul((char *)name, &uid, 0) == 0) {
		nvos_nss_dbg_print("improperly formatted user name:%s\n",
		    name);
		return (NULL);
	}

	bzero(filter, sizeof (*filter));
	strncpy(filter->vuser_name, name, MAX_USER_NAME);
	return (nvos_nss_userop(filter, NVOS_USER_GETBYNAME));
}

vuser_info_t *
nvos_nss_userop(vuser_info_t *arg, int op)
{
	nvOS_connection_t	*conn;
	int			err;
	vuser_info_t		*tmp;

	conn = nvOS_connect();
	if (conn == NULL) {
		return (NULL);
	}

	if (op == NVOS_USER_GETINFO) {
		err = nvOS_user_list(conn, &arg);
	} else {
		arg->vuser_op.user_opCode = op;
		arg->vuser_op.user_opSize = sizeof (*arg);
		err = nvOS_user_op(conn, arg);
	}

	if (err == 0 || err == EACCES) {
		if (op == NVOS_USER_GETINFO) {
			tmp = malloc(arg->vuser_op.user_opSize);
			if (tmp == NULL) {
				nvOS_disconnect(conn);
				return (NULL);
			}
			bcopy(arg, tmp, arg->vuser_op.user_opSize);
			arg = tmp;
		}
	} else {
		arg = NULL;
	}

	nvOS_disconnect(conn);
	return (arg);
}

void
nvos_nss_make_passwd_ent(vuser_info_t *user, char *buf, int size)
{
	char	*shell = "/usr/bin/nvOS_cli";

	if (user->vuser_type & nvc_USER_TYPE_MFG) {
		if (strcmp(user->vuser_name, nvOS_SYSTEST_USER) == 0) {
			shell = "/usr/bin/systest";
		} else if (strcmp(user->vuser_name, nvOS_SYSCONFIG_USER) == 0) {
			shell = "/usr/bin/sysconfig";
		} else if (strcmp(user->vuser_name, nvOS_SERIAL_USER) == 0) {
			shell = "/usr/bin/serial";
		}
	}
	snprintf(buf, size, "%s:x:%d:%d:%s:" NVOS_USER_HOME_DIR ":%s",
	    user->vuser_name, user->vuser_uid, user->vuser_gid,
	    user->vuser_name, shell);
}

void
nvos_nss_make_shadow_ent(vuser_info_t *user, char *buf, int size)
{
	snprintf(buf, size, "%s:%s:%u::::::", user->vuser_name,
	    user->vuser_passwdHash, user->vuser_uid);
}

static int
nvos_nss_copy_group(uint64_t fields, nvc_nss_group_t *group_in,
    nvc_nss_group_t *group_out)
{
	if (!nvc_FIELD_FLAG_TEST(fields, nvc_nss_group_name) ||
	    !nvc_FIELD_FLAG_TEST(fields, nvc_nss_group_gid)) {
		return (-1);
	}

	bzero(group_out, sizeof (*group_out));

	(void) strlcpy(group_out->name, group_in->name,
	    sizeof (group_out->name));
	group_out->gid = group_in->gid;
	if (nvc_FIELD_FLAG_TEST(fields, nvc_nss_group_members)) {
		strlcpy(group_out->members, group_in->members,
		    sizeof (group_out->members));
	}
	return (0);
}

static nvc_nss_group_t *
nvos_nss_dup_group(uint64_t fields, nvc_nss_group_t *group_in)
{
	nvc_nss_group_t	*group;

	group = malloc(sizeof (*group));
	if (nvos_nss_copy_group(fields, group_in, group) != 0) {
		free(group);
		return (NULL);
	}
	return (group);
}

static int
nvos_nss_show_group(uint64_t fields, nvc_nss_group_t *filter,
    nvc_show_nss_group_func_t show_func, void *arg)
{
	nvOS_io_t	io;
	int		err;
	nvOS_result_t	result;

	nvc_init(&io);
	err = nvc_connect(&io);
	if (err != 0) {
		nvos_nss_dbg_print("nvc_connect: %s\n", strerror(err));
		return (-1);
	}
	err = nvc_show_nss_group(&io, fields, filter, show_func, arg, &result);
	nvc_disconnect(&io);
	nvc_done(&io);
	if (err != 0) {
		nvos_nss_dbg_print("nvc_show_group: %s\n", strerror(err));
		return (-1);
	}
	if (result.res_status != nvOS_SUCCESS) {
		nvos_nss_dbg_print("nvc_show_group: %s\n", result.res_msg);
		return (-1);
	}
	return (0);
}

static int
nvos_nss_get_groups_cb(void *arg, uint64_t fields, nvc_nss_group_t *group_in)
{
	nv_slist_t	*groups = arg;
	nvc_nss_group_t	*group;

	group = nvos_nss_dup_group(fields, group_in);
	if (group != NULL) {
		nv_slist_append(groups, group);
	}
	return (0);
}

void
nvos_nss_get_groups(nv_slist_t *groups)
{
	nvc_nss_group_t	filter = { { 0 } };

	(void) nvos_nss_show_group(0, &filter, nvos_nss_get_groups_cb, groups);
}

typedef struct group_arg_s {
	boolean_t	ok;
	nvc_nss_group_t	*group;
} group_arg_t;

static int
nvos_nss_get_group_cb(void *arg, uint64_t fields, nvc_nss_group_t *group_in)
{
	group_arg_t	*group_arg = arg;

	if (nvos_nss_copy_group(fields, group_in, group_arg->group) == 0) {
		group_arg->ok = B_TRUE;
	}
	return (0);
}

int
nvos_nss_get_group_by_name(const char *name, nvc_nss_group_t *group)
{
	nvc_nss_group_t	filter = { { 0 } };
	group_arg_t	arg = { 0 };

	arg.group = group;
	strlcpy(filter.name, name, sizeof (filter.name));
	if (nvos_nss_show_group(nvc_nss_group_name, &filter,
	    nvos_nss_get_group_cb, &arg) != 0) {
		return (-1);
	}

	if (!arg.ok) {
		return (-1);
	}
	return (0);
}

int
nvos_nss_get_group_by_gid(gid_t gid, nvc_nss_group_t *group)
{
	nvc_nss_group_t	filter = { { 0 } };
	group_arg_t	arg = { 0 };

	arg.group = group;
	filter.gid = gid;
	if (nvos_nss_show_group(nvc_nss_group_gid, &filter,
	    nvos_nss_get_group_cb, &arg) != 0) {
		return (-1);
	}

	if (!arg.ok) {
		return (-1);
	}
	return (0);
}

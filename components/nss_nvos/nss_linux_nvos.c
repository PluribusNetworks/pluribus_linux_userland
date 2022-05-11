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
#include <nss.h>
#include <shadow.h>
#include <grp.h>
#include "nss_nvos.h"

typedef enum nss_status nss_status;

typedef struct nvos_backend_s {
	vuser_info_t	*info;
	int		index;
	nv_slist_t	groups;
	nv_slist_iter_t	group_iter;
	nvc_nss_group_t	*group;
} nvos_backend_t;

static nvos_backend_t g_backend;

/*
 * given a vuser_info_t make a shadow entry
 *   - build a shadow string
 *   - parse that string
 */
static nss_status
user_to_shadow_ent(vuser_info_t *user, struct spwd **result, char *buffer,
    size_t buflen, int *errnop)
{
	char		*tok, *ptr;
	char		*delim = ":";

	bzero(buffer, buflen);

	nvos_nss_make_shadow_ent(user, buffer, buflen);
	nvos_nss_dbg_print("%s: buf: %s, len: %d\n", __func__, buffer,
	    (int)buflen);

	/* User name */
	tok = nv_strtok_r(buffer, delim, &ptr);
	if (tok == NULL) {
		nvos_nss_dbg_print("%s: Could not parse ent username\n",
		    __func__);
		return (NSS_STATUS_UNAVAIL);
	}
	(*result)->sp_namp = tok;
	nvos_nss_dbg_print("%s: name %s\n", __func__, tok);

	/* Password Hash */
	tok = nv_strtok_r(NULL, delim, &ptr);
	if (tok == NULL) {
		nvos_nss_dbg_print("%s: Could not parse ent passwd\n",
		    __func__);
		return (NSS_STATUS_UNAVAIL);
	}
	(*result)->sp_pwdp = tok;
	nvos_nss_dbg_print("%s: passwd %s\n", __func__, tok);

	*errnop = errno;

	return (NSS_STATUS_SUCCESS);

}

/*
 * given a vuser_info_t make a passwd entry
 *   - build a passwd string
 *   - parse that string
 */
static nss_status
user_to_passwd_ent(vuser_info_t *user, struct passwd **result, char *buffer,
    size_t buflen, int *errnop)
{
	char		*tok, *endptr, *ptr;
	char		*delim = ":";

	bzero(buffer, buflen);

	nvos_nss_make_passwd_ent(user, buffer, buflen);
	nvos_nss_dbg_print("%s: buf: %s, len: %d\n", __func__, buffer,
	    (int)buflen);

	/* User name */
	tok = nv_strtok_r(buffer, delim, &ptr);
	if (tok == NULL) {
		nvos_nss_dbg_print("%s: Could not parse ent username\n",
		    __func__);
		return (NSS_STATUS_UNAVAIL);
	}
	(*result)->pw_name = tok;
	nvos_nss_dbg_print("%s: name %s\n", __func__, tok);

	/* Passwd */
	tok = nv_strtok_r(NULL, delim, &ptr);
	if (tok == NULL) {
		nvos_nss_dbg_print("%s: Could not parse ent passwd\n",
		    __func__);
		return (NSS_STATUS_UNAVAIL);
	}
	(*result)->pw_passwd = tok;
	nvos_nss_dbg_print("%s: passwd %s\n", __func__, tok);

	/* UID */
	tok = nv_strtok_r(NULL, delim, &ptr);
	if (tok == NULL) {
		nvos_nss_dbg_print("%s: Could not parse ent uid\n", __func__);
		return (NSS_STATUS_UNAVAIL);
	}
	(*result)->pw_uid = (uid_t)strtoul(tok, &endptr, 0);
	if (tok == endptr) {
		nvos_nss_dbg_print("%s: Failed to convert uid - %s\n", __func__,
		    tok);
		return (NSS_STATUS_UNAVAIL);
	}
	nvos_nss_dbg_print("%s: uid %d\n", __func__, (*result)->pw_uid);

	/* GID */
	tok = nv_strtok_r(NULL, delim, &ptr);
	if (tok == NULL) {
		nvos_nss_dbg_print("%s: Could not parse ent gid\n", __func__);
		return (NSS_STATUS_UNAVAIL);
	}
	(*result)->pw_gid = (gid_t)strtoul(tok, &endptr, 0);
	if (tok == endptr) {
		nvos_nss_dbg_print("%s: Failed to convert gid - %s\n",
		    __func__, tok);
		return (NSS_STATUS_UNAVAIL);
	}
	nvos_nss_dbg_print("%s: gid %d\n", __func__, (*result)->pw_gid);

	/* Real username */
	tok = nv_strtok_r(NULL, delim, &ptr);
	(*result)->pw_gecos = tok;
	nvos_nss_dbg_print("%s: gecos %s\n", __func__, tok);

	/* Home dir */
	tok = nv_strtok_r(NULL, delim, &ptr);
	if (tok == NULL) {
		nvos_nss_dbg_print("%s: Could not parse ent homedir\n",
		    __func__);
		return (NSS_STATUS_UNAVAIL);
	}
	(*result)->pw_dir = tok;
	nvos_nss_dbg_print("%s: homedir %s\n", __func__, tok);

	/* Shell */
	tok = nv_strtok_r(NULL, delim, &ptr);
	if (tok == NULL) {
		nvos_nss_dbg_print("%s: Could not parse ent shell\n",
		    __func__);
		return (NSS_STATUS_UNAVAIL);
	}
	(*result)->pw_shell = tok;
	nvos_nss_dbg_print("%s: shell %s\n", __func__, tok);

	*errnop = errno;

	return (NSS_STATUS_SUCCESS);
}

static nss_status
nvos_passwd_getent(struct passwd **result, char *buffer, size_t buflen,
    int *errnop)
{
	nvos_nss_dbg_print("%s\n", __func__);

	if (g_backend.info == NULL) {
		return (NSS_STATUS_NOTFOUND);
	}
	if (g_backend.index >= (g_backend.info->vuser_op.user_opSize) /
	    sizeof (vuser_info_t)) {
		return (NSS_STATUS_NOTFOUND);
	}

	nvos_nss_dbg_print("%s: %s - %d\n", __func__,
	    g_backend.info->vuser_name, g_backend.index);
	return (user_to_passwd_ent(g_backend.info + g_backend.index++, result,
	    buffer, buflen, errnop));
}

static nss_status
nvos_passwd_getbyname(const char *name, struct passwd **result, char *buffer,
    size_t buflen, int *errnop)
{
	vuser_info_t	*user;
	vuser_info_t	filter;

	nvos_nss_dbg_print("%s\n", __func__);

	if ((user = nvos_nss_vuser_getbyname(name, &filter)) == NULL) {
		nvos_nss_dbg_print("%s: User %s not found\n", __func__, name);
		return (NSS_STATUS_NOTFOUND);
	}
	nvos_nss_dbg_print("%s: calling user_to_passwd_ent\n", __func__);
	return (user_to_passwd_ent(user, result, buffer, buflen, errnop));
}

static nss_status
nvos_shadow_getbyname(const char *name, struct spwd **result, char *buffer,
    size_t buflen, int *errnop)
{
	vuser_info_t	*user;
	vuser_info_t	filter;

	nvos_nss_dbg_print("%s\n", __func__);

	if ((user = nvos_nss_vuser_getbyname(name, &filter)) == NULL) {
		nvos_nss_dbg_print("%s: User %s not found\n", __func__, name);
		return (NSS_STATUS_NOTFOUND);
	}
	return (user_to_shadow_ent(user, result, buffer, buflen, errnop));
}

nss_status
_nss_nvos_setpwent(void)
{
	vuser_info_t	filter;

	nvos_nss_set_debug();
	nvos_nss_dbg_print("%s\n", __func__);

	bzero(&g_backend, sizeof (g_backend));
	bzero(&filter, sizeof (filter));
	g_backend.info = nvos_nss_userop(&filter, NVOS_USER_GETINFO);
	if (g_backend.info == NULL) {
		nvos_nss_dbg_print("%s: no entries\n", __func__);
		return (NSS_STATUS_NOTFOUND);
	}
	return (NSS_STATUS_SUCCESS);
}

nss_status
_nss_nvos_endpwent(void)
{
	nvos_nss_dbg_print("%s\n", __func__);

	bzero(&g_backend.info, sizeof (g_backend.info));
	return (NSS_STATUS_SUCCESS);
}

nss_status
_nss_nvos_getpwent_r(struct passwd *result, char *buffer, size_t buflen,
    int *errnop)
{
	nvos_nss_dbg_print("%s\n", __func__);
	return (nvos_passwd_getent(&result, buffer, buflen, errnop));
}

nss_status
_nss_nvos_getspnam_r(const char *name, struct spwd *pwd,
    char *buffer, size_t buflen, int *errnop)
{
	nvos_nss_dbg_print("%s\n", __func__);
	return (nvos_shadow_getbyname(name, &pwd, buffer, buflen, errnop));
}

nss_status
_nss_nvos_getpwnam_r(const char *name, struct passwd *result, char *buffer,
    size_t buflen, int *errnop)
{
	nvos_nss_dbg_print("%s\n", __func__);
	return (nvos_passwd_getbyname(name, &result, buffer, buflen,
	    errnop));
}

nss_status
_nss_nvos_getpwuid_r(uid_t uid, struct passwd *result, char *buffer,
    size_t buflen, int *errnop)
{
	vuser_info_t		filter;
	vuser_info_t		*user;

	nvos_nss_dbg_print("%s\n", __func__);

	bzero(&filter, sizeof (filter));
	filter.vuser_uid = uid;
	user = nvos_nss_userop(&filter, NVOS_USER_GETBYUID);
	if (user == NULL) {
		return (NSS_STATUS_NOTFOUND);
	}

	return (user_to_passwd_ent(user, &result, buffer, buflen, errnop));
}

typedef struct buf_s {
	char	*buffer;
	size_t	buflen;
	char	*ptr;
} buf_t;

static void
buf_init(buf_t *buf, char *buffer, size_t buflen)
{
	bzero(buf, sizeof (*buf));
	buf->buffer = buffer;
	buf->buflen = buflen;
	buf->ptr = buf->buffer;
}

static void *
buf_alloc(buf_t *buf, size_t size)
{
	void	*ptr;

	if (buf->buflen - (buf->ptr - buf->buffer) < size) {
		return (NULL);
	}

	ptr = buf->ptr;
	buf->ptr += size;
	bzero(ptr, size);
	return (ptr);
}

static char *
buf_strdup(buf_t *buf, char *str)
{
	size_t	len;
	char	*dup;

	len = strlen(str) + 1;
	dup = buf_alloc(buf, len);
	if (dup == NULL) {
		return (NULL);
	}
	strlcpy(dup, str, len);
	return (dup);
}

static int
copy_group(nvc_nss_group_t *group, struct group *result, char *buffer,
    size_t buflen)
{
	buf_t		buf;
	char		members[nvc_PCL_LONG_STRING];
	char		*mem;
	int		num_mem;
	char		*lasts;
	int		ii;

	buf_init(&buf, buffer, buflen);

	result->gr_name = buf_strdup(&buf, group->name);
	if (result->gr_name == NULL) {
		return (-1);
	}
	result->gr_passwd = "x";
	result->gr_gid = group->gid;

	num_mem = 0;
	strlcpy(members, group->members, sizeof (members));
	for (mem = strtok_r(members, ",", &lasts); mem != NULL;
	    mem = strtok_r(NULL, ",", &lasts)) {
		num_mem++;
	}
	result->gr_mem = buf_alloc(&buf, sizeof (*result->gr_mem) *
	    (num_mem + 1));
	if (result->gr_mem == NULL) {
		return (-1);
	}
	ii = 0;
	strlcpy(members, group->members, sizeof (members));
	for (mem = strtok_r(members, ",", &lasts); mem != NULL;
	    mem = strtok_r(NULL, ",", &lasts)) {
		assert(ii < num_mem);
		result->gr_mem[ii] = buf_strdup(&buf, mem);
		if (result->gr_mem[ii] == NULL) {
			return (-1);
		}
		ii++;
	}
	assert(ii == num_mem);
	result->gr_mem[ii] = NULL;

	return (0);
}

nss_status
_nss_nvos_getgrnam_r(const char *name, struct group *result, char *buffer,
    size_t buflen, int *errnop)
{
	nvc_nss_group_t	group = { { 0 } };

	if (nvos_nss_get_group_by_name(name, &group) != 0) {
		*errnop = ENOENT;
		return (NSS_STATUS_NOTFOUND);
	}

	if (copy_group(&group, result, buffer, buflen) != 0) {
		*errnop = ERANGE;
		return (NSS_STATUS_TRYAGAIN);
	}

	return (NSS_STATUS_SUCCESS);
}

nss_status
_nss_nvos_getgrgid_r(gid_t gid, struct group *result, char *buffer,
    size_t buflen, int *errnop)
{
	nvc_nss_group_t	group = { { 0 } };

	if (nvos_nss_get_group_by_gid(gid, &group) != 0) {
		*errnop = ENOENT;
		return (NSS_STATUS_NOTFOUND);
	}

	if (copy_group(&group, result, buffer, buflen) != 0) {
		*errnop = ERANGE;
		return (NSS_STATUS_TRYAGAIN);
	}

	return (NSS_STATUS_SUCCESS);
}

nss_status
_nss_nvos_setgrent(void)
{
	nv_slist_init_clear_func(&g_backend.groups, free);
	nvos_nss_get_groups(&g_backend.groups);
	nv_slist_iter_init(&g_backend.group_iter, &g_backend.groups);
	g_backend.group = nv_slist_iter_first(&g_backend.group_iter);
	return (NSS_STATUS_SUCCESS);
}

nss_status
_nss_nvos_endgrent(void)
{
	g_backend.group = NULL;
	nv_slist_done(&g_backend.groups);
	bzero(&g_backend, sizeof (g_backend));
	return (NSS_STATUS_SUCCESS);
}

nss_status
_nss_nvos_getgrent_r(struct group *result, char *buffer, size_t buflen,
    int *errnop)
{
	nvc_nss_group_t	*group;

	group = g_backend.group;
	if (group == NULL) {
		*errnop = ENOENT;
		return (NSS_STATUS_NOTFOUND);
	}

	if (copy_group(group, result, buffer, buflen) != 0) {
		*errnop = ERANGE;
		return (NSS_STATUS_TRYAGAIN);
	}
	g_backend.group = nv_slist_iter_next(&g_backend.group_iter);

	return (NSS_STATUS_SUCCESS);
}

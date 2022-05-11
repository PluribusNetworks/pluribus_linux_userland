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
#include <nss_dbdefs.h>
#include "nss_nvos.h"

#define	LINE_SIZE 4096

typedef struct nvos_backend_s {
	nss_backend_t	nss;
	vuser_info_t	*info;
	int		index;
	nv_slist_t	groups;
	nv_slist_iter_t	group_iter;
	nvc_nss_group_t	*group;
} nvos_backend_t;

static nss_status_t
user_to_ent(vuser_info_t *user, nss_XbyY_args_t *xbyy, make_ent_func_t func)
{
	char		buf[LINE_SIZE];
	int		parse_status;

	(*func)(user, buf, sizeof (buf));
	nvos_nss_dbg_print("%s: buf: %s\n", __func__, buf);
	parse_status = (*xbyy->str2ent)(buf, strlen(buf), xbyy->buf.result,
	    xbyy->buf.buffer, xbyy->buf.buflen);
	if (parse_status == NSS_STR_PARSE_SUCCESS) {
		nvos_nss_dbg_print("%s: success!\n", __func__);
		xbyy->returnval = xbyy->buf.result != NULL ? xbyy->buf.result :
		    xbyy->buf.buffer;
		xbyy->returnlen = strlen(buf); /* XXX huh? */
		return (NSS_SUCCESS);
	}
	nvos_nss_dbg_print("%s: parse error: %d\n", __func__, parse_status);
	return (NSS_ERROR);
}

static nss_status_t
nvos_destr(nss_backend_t *be, void *arg)
{
	nvos_backend_t	*nvos_be = (nvos_backend_t *)be;

	nvos_nss_dbg_print("%s\n", __func__);
	free(nvos_be->info);
	nvos_be->info = NULL;
	nvos_be->index = 0;
	free(nvos_be);
	return (NSS_SUCCESS);
}

static nss_status_t
nvos_endent(nss_backend_t *be, void *arg)
{
	nvos_backend_t	*nvos_be = (nvos_backend_t *)be;

	nvos_nss_dbg_print("%s\n", __func__);

	free(nvos_be->info);
	nvos_be->info = NULL;
	nvos_be->index = 0;
	return (NSS_SUCCESS);
}

static nss_status_t
nvos_setent(nss_backend_t *be, void *arg)
{
	nvos_backend_t	*nvos_be = (nvos_backend_t *)be;
	vuser_info_t	filter;

	nvos_nss_dbg_print("%s\n", __func__);

	free(nvos_be->info);
	nvos_be->info = NULL;
	nvos_be->index = 0;

	bzero(&filter, sizeof (filter));
	nvos_be->info = nvos_nss_userop(&filter, NVOS_USER_GETINFO);
	if (nvos_be->info == NULL) {
		return (NSS_UNAVAIL);
	}
	return (NSS_SUCCESS);
}

static nss_status_t
nvos_getent(nss_backend_t *be, void *arg, make_ent_func_t func)
{
	nvos_backend_t	*nvos_be = (nvos_backend_t *)be;
	nss_XbyY_args_t	*xbyy = (nss_XbyY_args_t *)arg;

	nvos_nss_dbg_print("%s\n", __func__);

	if (nvos_be->info == NULL) {
		return (NSS_NOTFOUND);
	}
	if (nvos_be->index >= nvos_be->info->vuser_op.user_opSize /
	    sizeof (*nvos_be->info)) {
		return (NSS_NOTFOUND);
	}

	return (user_to_ent(nvos_be->info + nvos_be->index++, xbyy, func));
}

static nss_status_t
nvos_getbyname(nss_backend_t *be, void *arg, make_ent_func_t func)
{
	nss_XbyY_args_t	*xbyy = (nss_XbyY_args_t *)arg;
	vuser_info_t	filter;
	vuser_info_t	*user;

	nvos_nss_dbg_print("%s\n", __func__);

	if ((user = nvos_nss_vuser_getbyname(xbyy->key.name, &filter))
	    == NULL) {
		nvos_nss_dbg_print("%s: User %s not found\n", __func__,
		    xbyy->key.name);
		return (NSS_NOTFOUND);
	}

	nvos_nss_dbg_print("calling user_to_ent\n");
	return (user_to_ent(user, xbyy, func));
}

static nss_status_t
nvos_passwd_getent(nss_backend_t *be, void *arg)
{
	nvos_nss_dbg_print("%s\n", __func__);
	return (nvos_getent(be, arg, nvos_nss_make_passwd_ent));
}

static nss_status_t
nvos_passwd_getbyname(nss_backend_t *be, void *arg)
{
	nvos_nss_dbg_print("%s\n", __func__);
	return (nvos_getbyname(be, arg, nvos_nss_make_passwd_ent));
}

static nss_status_t
nvos_passwd_getbyuid(nss_backend_t *be, void *arg)
{
	nss_XbyY_args_t	*xbyy = (nss_XbyY_args_t *)arg;
	vuser_info_t	filter;
	vuser_info_t	*user;

	nvos_nss_dbg_print("%s\n", __func__);

	bzero(&filter, sizeof (filter));
	filter.vuser_uid = xbyy->key.uid;
	user = nvos_nss_userop(&filter, NVOS_USER_GETBYUID);
	if (user == NULL) {
		return (NSS_NOTFOUND);
	}

	return (user_to_ent(user, xbyy, nvos_nss_make_passwd_ent));
}

static nss_backend_op_t g_nvos_passwd_ops[] = {
	nvos_destr,
	nvos_endent,
	nvos_setent,
	nvos_passwd_getent,
	nvos_passwd_getbyname,
	nvos_passwd_getbyuid
};

nss_backend_t *
_nss_nvos_passwd_constr(const char *db_name, const char *src_name,
    const char *cfg_args)
{
	nvos_backend_t	*be;

	if (!nvos_nss_g_enabled) {
		return (NULL);
	}

	nvos_nss_set_debug();
	nvos_nss_dbg_print("%s\n", __func__);
	be = malloc(sizeof (*be));
	if (be == NULL) {
		return (NULL);
	}
	bzero(be, sizeof (*be));
	be->nss.ops = g_nvos_passwd_ops;
	be->nss.n_ops = sizeof (g_nvos_passwd_ops) /
	    sizeof (*g_nvos_passwd_ops);
	return ((nss_backend_t *)be);
}

static nss_status_t
nvos_shadow_getent(nss_backend_t *be, void *arg)
{
	nvos_nss_dbg_print("%s\n", __func__);
	return (nvos_getent(be, arg, nvos_nss_make_shadow_ent));
}

static nss_status_t
nvos_shadow_getbyname(nss_backend_t *be, void *arg)
{
	nvos_nss_dbg_print("%s\n", __func__);
	return (nvos_getbyname(be, arg, nvos_nss_make_shadow_ent));
}

static nss_backend_op_t g_nvos_shadow_ops[] = {
	nvos_destr,
	nvos_endent,
	nvos_setent,
	nvos_shadow_getent,
	nvos_shadow_getbyname
};

nss_backend_t *
_nss_nvos_shadow_constr(const char *db_name, const char *src_name,
    const char *cfg_args)
{
	nvos_backend_t	*be;

	if (!nvos_nss_g_enabled) {
		return (NULL);
	}

	nvos_nss_set_debug();
	nvos_nss_dbg_print("%s\n", __func__);
	be = malloc(sizeof (*be));
	if (be == NULL) {
		return (NULL);
	}
	bzero(be, sizeof (*be));
	be->nss.ops = g_nvos_shadow_ops;
	be->nss.n_ops = sizeof (g_nvos_shadow_ops) /
	    sizeof (*g_nvos_shadow_ops);
	return ((nss_backend_t *)be);
}

void
_nss_nvos_disable(void)
{
	nvos_nss_g_enabled = B_FALSE;
}


static nss_status_t
nvos_group_endent(nss_backend_t *be, void *arg)
{
	nvos_backend_t	*nvos_be = (nvos_backend_t *)be;

	nv_slist_done(&nvos_be->groups);
	nvos_be->group = NULL;
	return (NSS_SUCCESS);
}

static nss_status_t
nvos_group_destr(nss_backend_t *be, void *arg)
{
	nvos_group_endent(be, arg);
	free(be);
	return (NSS_SUCCESS);
}

static nss_status_t
nvos_group_setent(nss_backend_t *be, void *arg)
{
	nvos_backend_t	*nvos_be = (nvos_backend_t *)be;

	nv_slist_init_clear_func(&nvos_be->groups, free);
	nvos_nss_get_groups(&nvos_be->groups);
	nv_slist_iter_init(&nvos_be->group_iter, &nvos_be->groups);
	nvos_be->group = nv_slist_iter_first(&nvos_be->group_iter);
	return (NSS_SUCCESS);
}


static void
nvos_nss_make_group_ent(nvc_nss_group_t *group, char *buf, int size)
{
	snprintf(buf, size, "%s::%d:%s", group->name,
	    group->gid, group->members);
}

static nss_status_t
group_to_ent(nvc_nss_group_t *group, nss_XbyY_args_t *xbyy)
{
	char	buf[LINE_SIZE];
	int	parse_status;

	nvos_nss_make_group_ent(group, buf, sizeof (buf));
	nvos_nss_dbg_print("%s: buf: %s\n", __func__, buf);
	parse_status = (*xbyy->str2ent)(buf, strlen(buf), xbyy->buf.result,
	    xbyy->buf.buffer, xbyy->buf.buflen);
	if (parse_status == NSS_STR_PARSE_SUCCESS) {
		nvos_nss_dbg_print("%s: success!\n", __func__);
		xbyy->returnval = xbyy->buf.result != NULL ? xbyy->buf.result :
		    xbyy->buf.buffer;
		xbyy->returnlen = strlen(buf); /* XXX huh? */
		return (NSS_SUCCESS);
	}

	nvos_nss_dbg_print("%s: parse error: %d\n", __func__, parse_status);
	return (NSS_ERROR);
}

static nss_status_t
nvos_group_getent(nss_backend_t *be, void *arg)
{
	nvos_backend_t	*nvos_be = (nvos_backend_t *)be;
	nvc_nss_group_t	*group;
	nss_XbyY_args_t	*xbyy = (nss_XbyY_args_t *)arg;

	group = nvos_be->group;
	if (group == NULL) {
		return (NSS_NOTFOUND);
	}
	nvos_be->group = nv_slist_iter_next(&nvos_be->group_iter);

	return (group_to_ent(group, xbyy));
}

static nss_status_t
nvos_group_getbyname(nss_backend_t *be, void *arg)
{
	nss_XbyY_args_t	*xbyy = (nss_XbyY_args_t *)arg;
	nvc_nss_group_t	group = { { 0 } };

	if (nvos_nss_get_group_by_name(xbyy->key.name, &group) != 0) {
		nvos_nss_dbg_print("%s: group %s not found\n", __func__,
		    xbyy->key.name);
		return (NSS_NOTFOUND);
	}

	return (group_to_ent(&group, xbyy));
}

static nss_status_t
nvos_group_getbygid(nss_backend_t *be, void *arg)
{
	nss_XbyY_args_t	*xbyy = (nss_XbyY_args_t *)arg;
	nvc_nss_group_t	group = { { 0 } };

	if (nvos_nss_get_group_by_gid(xbyy->key.gid, &group) != 0) {
		nvos_nss_dbg_print("%s: group %s not found\n", __func__,
		    xbyy->key.name);
		return (NSS_NOTFOUND);
	}

	return (group_to_ent(&group, xbyy));
}

static nss_status_t
nvos_group_getbymember(nss_backend_t *be, void *arg)
{
	struct nss_groupsbymem	*argp = arg;
	nv_slist_t		groups;
	nv_slist_iter_t		iter;
	char			buf[LINE_SIZE];
	nvc_nss_group_t		*group;
	nss_status_t		status = NSS_NOTFOUND;

	nv_slist_init_clear_func(&groups, free);
	nvos_nss_get_groups(&groups);
	nv_slist_iter_init(&iter, &groups);
	for (group = nv_slist_iter_first(&iter); group != NULL;
	    group = nv_slist_iter_next(&iter)) {
		nvos_nss_make_group_ent(group, buf, sizeof (buf));
		status = argp->process_cstr(buf, strlen(buf), argp);
		if (status != NSS_NOTFOUND) {
			break;
		}
	}

	nv_slist_done(&groups);
	return (NSS_SUCCESS);
}

static nss_backend_op_t g_nvos_group_ops[] = {
	nvos_group_destr,
	nvos_group_endent,
	nvos_group_setent,
	nvos_group_getent,
	nvos_group_getbyname,
	nvos_group_getbygid,
	nvos_group_getbymember
};

nss_backend_t *
_nss_nvos_group_constr(const char *db_name, const char *src_name,
    const char *cfg_args)
{
	nvos_backend_t	*be;

	if (!nvos_nss_g_enabled) {
		return (NULL);
	}

	nvos_nss_set_debug();
	nvos_nss_dbg_print("%s\n", __func__);
	be = malloc(sizeof (*be));
	if (be == NULL) {
		return (NULL);
	}
	bzero(be, sizeof (*be));
	be->nss.ops = g_nvos_group_ops;
	be->nss.n_ops = sizeof (g_nvos_group_ops) /
	    sizeof (*g_nvos_group_ops);
	return ((nss_backend_t *)be);
}

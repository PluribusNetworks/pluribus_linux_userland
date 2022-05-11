/*
 * External password backend
 * Copyright (c) 2020 Pluribus Networks
 *
 */

#include "includes.h"

#include "common.h"
#include "ext_password_i.h"

struct ext_password_nvos_data {
	char *params;
};
#define B64_INVALID (0xffffffff)

static uint32_t
decode64_sextet(char ch)
{
        if ('A' <= ch && ch <= 'Z') {
                return (ch - 'A');
        }
        if ('a' <= ch && ch <= 'z') {
                return (26 + ch - 'a');
        }
        if ('0' <= ch && ch <= '9') {
                return (52 + ch - '0');
        }
        if (ch == '+') {
                return (62);
        }
        if (ch == '/') {
                return (63);
        }
        return (B64_INVALID);
}

int
nv_base64_decode(char *input, void *output, int out_size)
{
        char            *in;
        uint8_t         *out;
        uint32_t        tmp, val;
        int             equals;

        in = input;
        out = output;

        while (*in != '\0') {
                tmp = decode64_sextet(*in++);
                if (tmp == B64_INVALID) {
                        return (-1);
                }
                val = tmp << 18;
                if (*in != '\0') {
                        tmp = decode64_sextet(*in++);
                        if (tmp == B64_INVALID) {
                                return (-1);
                        }
                        val |= tmp << 12;
                }
                equals = 0;
                if (*in == '=') {
                        equals++;
                        in++;
                } else if (*in != '\0') {
                        tmp = decode64_sextet(*in++);
                        if (tmp == B64_INVALID) {
                                return (-1);
                        }
                        val |= tmp << 6;
                }
                if (*in == '=') {
                        equals++;
                        in++;
                } else if (*in != '\0') {
                        if (equals) {
                                return (-1);
                        }
                        tmp = decode64_sextet(*in++);
                        if (tmp == B64_INVALID) {
                                return (-1);
                        }
                        val |= tmp;
                }
                if (out - (uint8_t *)output >= out_size) {
                        return (-1);
                }
                *out++ = (val >> 16) & 0xff;
                if (equals < 2) {
                        if (out - (uint8_t *)output >= out_size) {
                                return (-1);
                        }
                        *out++ = (val >> 8) & 0xff;
                }
                if (equals == 0) {
                        if (out - (uint8_t *)output >= out_size) {
                                return (-1);
                        }
                        *out++ = val & 0xff;
                }
                if (equals != 0 && *in != '\0') {
                        return (-1);
                }
        }
        return (out - (uint8_t *)output);
}

static void * ext_password_nvos_init(const char *params)
{
	struct ext_password_nvos_data *data;

	data = os_zalloc(sizeof(*data));
	if (data == NULL)
		return NULL;

	if (params)
		data->params = os_strdup(params);

	return data;
}


static void ext_password_nvos_deinit(void *ctx)
{
	struct ext_password_nvos_data *data = ctx;

	str_clear_free(data->params);
	os_free(data);
}

#define PASSWD_ENCODED_LEN	512
static struct wpabuf * ext_password_nvos_get(void *ctx, const char *name)
{
	struct wpabuf	*buf;
	wpa_printf(MSG_DEBUG, "EXT PW TEST: get(%s)", name);
	buf = ext_password_alloc(PASSWD_ENCODED_LEN*sizeof(char)+sizeof(struct wpabuf));
	if (buf == NULL)
		return NULL;
	nv_base64_decode(name, buf->buf, PASSWD_ENCODED_LEN);
	buf->size = PASSWD_ENCODED_LEN*sizeof(char)+sizeof(struct wpabuf);
	buf->used  = strlen(buf->buf);
	wpa_hexdump_ascii_key(MSG_DEBUG, "EXT PW TEST: value",
			wpabuf_head(buf),
			wpabuf_len(buf));
	return buf;
}


const struct ext_password_backend ext_password_nvos = {
	.name = "nvos",
	.init = ext_password_nvos_init,
	.deinit = ext_password_nvos_deinit,
	.get = ext_password_nvos_get,
};

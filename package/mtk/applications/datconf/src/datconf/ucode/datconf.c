/*
 * Copyright (C) 2025  chasey-dev <ellenyoung0912@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "ucode/module.h"
#include "ucode/platform.h"
#include "libdatconf.h"

static uc_resource_type_t *datconf_type;

#define err_return(err) do { \
	uc_vm_registry_set(vm, "datconf.last_error", ucv_int64_new(err)); \
	return NULL; \
} while(0)

/* --- Helper: Get Error --- */

static uc_value_t *
uc_datconf_error(uc_vm_t *vm, size_t nargs)
{
	int last_error = ucv_int64_get(uc_vm_registry_get(vm, "datconf.last_error"));

	if (last_error == 0)
		return NULL;

	uc_vm_registry_set(vm, "datconf.last_error", ucv_int64_new(0));
	return ucv_string_new(strerror(last_error));
}

/* --- Resource Destructor --- */

static void close_ctx(void *ud)
{
	struct kvc_context *ctx = ud;

	if (ctx)
		kvc_unload(ctx);
}

/* --- Context Methods --- */

static uc_value_t *
uc_datconf_ctx_get(uc_vm_t *vm, size_t nargs)
{
	struct kvc_context **ctx = uc_fn_this("datconf.context");
	uc_value_t *key = uc_fn_arg(0);
	const char *val;

	if (!ctx || !*ctx)
		err_return(EBADF);

	if (ucv_type(key) != UC_STRING)
		err_return(EINVAL);

	val = kvc_get(*ctx, ucv_string_get(key));

	if (!val)
		return NULL;

	return ucv_string_new(val);
}

static uc_value_t *
uc_datconf_ctx_set(uc_vm_t *vm, size_t nargs)
{
	struct kvc_context **ctx = uc_fn_this("datconf.context");
	uc_value_t *key = uc_fn_arg(0);
	uc_value_t *val = uc_fn_arg(1);
	int ret;
	char *s_val;

	if (!ctx || !*ctx)
		err_return(EBADF);

	if (ucv_type(key) != UC_STRING)
		err_return(EINVAL);

	/* Convert value to string if necessary, handle non-strings gracefully */
	if (ucv_type(val) == UC_STRING) {
		ret = kvc_set(*ctx, ucv_string_get(key), ucv_string_get(val));
	} else {
		s_val = ucv_to_string(vm, val);
		ret = kvc_set(*ctx, ucv_string_get(key), s_val ? s_val : "");
		free(s_val);
	}

	if (ret != 0)
		err_return(ret);

	return ucv_boolean_new(true);
}

static uc_value_t *
uc_datconf_ctx_unset(uc_vm_t *vm, size_t nargs)
{
	struct kvc_context **ctx = uc_fn_this("datconf.context");
	uc_value_t *key = uc_fn_arg(0);

	if (!ctx || !*ctx)
		err_return(EBADF);

	if (ucv_type(key) != UC_STRING)
		err_return(EINVAL);

	kvc_unset(*ctx, ucv_string_get(key));

	return ucv_boolean_new(true);
}

static uc_value_t *
uc_datconf_ctx_commit(uc_vm_t *vm, size_t nargs)
{
	struct kvc_context **ctx = uc_fn_this("datconf.context");
	int ret;

	if (!ctx || !*ctx)
		err_return(EBADF);

	ret = kvc_commit(*ctx);

	if (ret != 0)
		err_return(-ret); // kvc_commit returns negative errno

	return ucv_boolean_new(true);
}

static uc_value_t *
uc_datconf_ctx_count(uc_vm_t *vm, size_t nargs)
{
	struct kvc_context **ctx = uc_fn_this("datconf.context");

	if (!ctx || !*ctx)
		err_return(EBADF);

	return ucv_int64_new(kvc_get_count(*ctx));
}

static uc_value_t *
uc_datconf_ctx_getall(uc_vm_t *vm, size_t nargs)
{
	struct kvc_context **ctx = uc_fn_this("datconf.context");
	struct kvc_enum_context *ectx;
	const union conf_item *item;
	uc_value_t *obj;

	if (!ctx || !*ctx)
		err_return(EBADF);

	obj = ucv_object_new(vm);
	ectx = kvc_enum_init(*ctx);

	while (!kvc_enum_next(ectx, &item)) {
		ucv_object_add(obj, item->key, ucv_string_new(item->value));
	}

	kvc_enum_end(ectx);

	return obj;
}

static uc_value_t *
uc_datconf_ctx_merge(uc_vm_t *vm, size_t nargs)
{
	struct kvc_context **ctx = uc_fn_this("datconf.context");
	uc_value_t *obj = uc_fn_arg(0);
	char *s_val;

	if (!ctx || !*ctx)
		err_return(EBADF);

	if (ucv_type(obj) != UC_OBJECT)
		err_return(EINVAL);

	ucv_object_foreach(obj, k, v) {
		if (ucv_type(v) == UC_STRING) {
			kvc_set(*ctx, k, ucv_string_get(v));
		} else {
			s_val = ucv_to_string(vm, v);
			kvc_set(*ctx, k, s_val ? s_val : "");
			free(s_val);
		}
	}

	return ucv_boolean_new(true);
}

static uc_value_t *
uc_datconf_ctx_close(uc_vm_t *vm, size_t nargs)
{
	struct kvc_context **ctx = uc_fn_this("datconf.context");
	uc_value_t *commit = uc_fn_arg(0);

	if (!ctx || !*ctx)
		return ucv_boolean_new(true); /* Already closed */

	if (ucv_is_truish(commit))
		kvc_commit(*ctx);

	kvc_unload(*ctx);
	*ctx = NULL;

	return ucv_boolean_new(true);
}

/* --- Global Functions --- */

static uc_value_t *
uc_datconf_open(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *path = uc_fn_arg(0);
	struct kvc_context *ctx;

	if (ucv_type(path) != UC_STRING)
		err_return(EINVAL);

	ctx = dat_load(ucv_string_get(path));
	if (!ctx)
		err_return(errno ? errno : ENOENT);

	return ucv_resource_new(datconf_type, ctx);
}

static uc_value_t *
uc_datconf_open_by_name(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *name = uc_fn_arg(0);
	struct kvc_context *ctx;

	if (ucv_type(name) != UC_STRING)
		err_return(EINVAL);

	ctx = dat_load_by_name(ucv_string_get(name));
	if (!ctx)
		err_return(errno ? errno : ENOENT);

	return ucv_resource_new(datconf_type, ctx);
}

static uc_value_t *
uc_datconf_open_by_index(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *idx = uc_fn_arg(0);
	struct kvc_context *ctx;

	if (ucv_type(idx) != UC_INTEGER)
		err_return(EINVAL);

	ctx = dat_load_by_index((uint32_t)ucv_int64_get(idx));
	if (!ctx)
		err_return(errno ? errno : ENOENT);

	return ucv_resource_new(datconf_type, ctx);
}

/* 
 * parse(content):
 * Parses raw string content and returns a ucode object with all key-values.
 * Does not return a context handle (one-shot).
 */
static uc_value_t *
uc_datconf_parse(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *str = uc_fn_arg(0);
	struct kvc_context *ctx;
	uc_value_t *res;
	
	if (ucv_type(str) != UC_STRING)
		err_return(EINVAL);

	ctx = dat_load_raw(ucv_string_get(str), ucv_string_length(str));
	if (!ctx)
		err_return(ENOMEM);

	/* Reuse getall logic inline */
	res = ucv_object_new(vm);
	struct kvc_enum_context *ectx = kvc_enum_init(ctx);
	const union conf_item *item;

	while (!kvc_enum_next(ectx, &item)) {
		ucv_object_add(res, item->key, ucv_string_new(item->value));
	}
	kvc_enum_end(ectx);
	
	kvc_unload(ctx);
	return res;
}

/* Helpers for semicolon separated values */

static uc_value_t *
uc_datconf_get_indexed_value(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *str = uc_fn_arg(0);
	uc_value_t *idx = uc_fn_arg(1);
	const char *res_str;
	uc_value_t *rv;

	if (ucv_type(str) != UC_STRING || ucv_type(idx) != UC_INTEGER)
		err_return(EINVAL);

	res_str = dat_get_indexed_value(ucv_string_get(str), (size_t)ucv_int64_get(idx));
	
	if (!res_str)
		return NULL;

	rv = ucv_string_new(res_str);
	free((void *)res_str); /* dat_get_indexed_value allocates memory */
	
	return rv;
}

static uc_value_t *
uc_datconf_set_indexed_value(uc_vm_t *vm, size_t nargs)
{
	uc_value_t *str = uc_fn_arg(0);
	uc_value_t *idx = uc_fn_arg(1);
	uc_value_t *val = uc_fn_arg(2);
	const char *res_str;
	uc_value_t *rv;
	char *s_val;

	if (ucv_type(str) != UC_STRING || ucv_type(idx) != UC_INTEGER)
		err_return(EINVAL);

	if (ucv_type(val) == UC_STRING) {
		res_str = dat_set_indexed_value(ucv_string_get(str), (size_t)ucv_int64_get(idx), ucv_string_get(val));
	} else {
		s_val = ucv_to_string(vm, val);
		res_str = dat_set_indexed_value(ucv_string_get(str), (size_t)ucv_int64_get(idx), s_val ? s_val : "");
		free(s_val);
	}

	if (!res_str)
		err_return(ENOMEM);

	rv = ucv_string_new(res_str);
	free((void *)res_str); 
	
	return rv;
}


static const uc_function_list_t ctx_fns[] = {
	{ "get",		uc_datconf_ctx_get },
	{ "set",		uc_datconf_ctx_set },
	{ "unset",		uc_datconf_ctx_unset },
	{ "commit",		uc_datconf_ctx_commit },
	{ "count",		uc_datconf_ctx_count },
	{ "getall",		uc_datconf_ctx_getall },
	{ "merge",		uc_datconf_ctx_merge },
	{ "close",		uc_datconf_ctx_close },
};

static const uc_function_list_t global_fns[] = {
	{ "open",			uc_datconf_open },
	{ "open_by_name",	uc_datconf_open_by_name },
	{ "open_by_index",	uc_datconf_open_by_index },
	{ "parse",			uc_datconf_parse },
	{ "error",			uc_datconf_error },
	{ "get_indexed_value",	uc_datconf_get_indexed_value },
	{ "set_indexed_value",	uc_datconf_set_indexed_value },
};

void uc_module_init(uc_vm_t *vm, uc_value_t *scope)
{
	uc_function_list_register(scope, global_fns);

	datconf_type = uc_type_declare(vm, "datconf.context", ctx_fns, close_ctx);
}
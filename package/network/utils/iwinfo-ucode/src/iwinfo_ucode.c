/*
 * iwinfo_ucode.c - Dynamic Ucode Bindings for libiwinfo
 * Copyright (C) 2025  chasey-dev <ellenyoung0912@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "ucode/module.h"
#include "iwinfo.h"

/* 
 * Define a large buffer size specifically for list operations (scan/assoc).
 * 64KB is sufficient to handle MTK's large MAC tables (up to 544 entries).
 * Standard IWINFO_BUFSIZE (usually 4KB) is kept for simple string ops.
 */
#define IWINFO_BIG_BUFSIZE (64 * 1024)

/* Resource type definition */
static uc_resource_type_t *iwinfo_backend_type;

/* --- Helpers --- */

/* Validate argument is a string and return it */
#define CHECK_STRING(vm, nargs, var_name) \
    uc_value_t *arg = uc_fn_arg(0); \
    if (!arg || ucv_type(arg) != UC_STRING) { \
        uc_vm_raise_exception(vm, EXCEPTION_TYPE, "string required"); \
        return NULL; \
    } \
    const char *var_name = ucv_string_get(arg)

/* Retrieve the iwinfo_ops pointer from the 'this' context */
static const struct iwinfo_ops *
iwinfo_uc_get_ops(uc_vm_t *vm)
{
	const struct iwinfo_ops **ops_ptr = uc_fn_this("iwinfo.backend");

	if (!ops_ptr || !*ops_ptr) {
		uc_vm_raise_exception(vm, EXCEPTION_TYPE, "Invalid backend context");
		return NULL;
	}

	return *ops_ptr;
}

/* Helper to convert bitmask to string array */
static void
iwinfo_uc_add_array_from_bitmask(uc_vm_t *vm, uc_value_t *obj, const char *key, 
                                 int mask, const char * const names[], int count)
{
	uc_value_t *arr = ucv_array_new(vm);
	int i;

	for (i = 0; i < count; i++) {
		if (mask & (1 << i))
			ucv_array_push(arr, ucv_string_new(names[i]));
	}
	ucv_object_add(obj, key, arr);
}

/* Helper to convert crypto entry to ucode object */
static uc_value_t *
iwinfo_uc_cryptotable(uc_vm_t *vm, struct iwinfo_crypto_entry *c)
{
	uc_value_t *obj = ucv_object_new(vm);

	ucv_object_add(obj, "enabled", ucv_boolean_new(c->enabled));
	ucv_object_add(obj, "wep", ucv_boolean_new(c->enabled && !c->wpa_version));
	ucv_object_add(obj, "wpa", ucv_int64_new(c->wpa_version));

	iwinfo_uc_add_array_from_bitmask(vm, obj, "pair_ciphers", c->pair_ciphers, 
		IWINFO_CIPHER_NAMES, IWINFO_CIPHER_COUNT);
	iwinfo_uc_add_array_from_bitmask(vm, obj, "group_ciphers", c->group_ciphers, 
		IWINFO_CIPHER_NAMES, IWINFO_CIPHER_COUNT);
	iwinfo_uc_add_array_from_bitmask(vm, obj, "auth_suites", c->auth_suites, 
		IWINFO_KMGMT_NAMES, IWINFO_KMGMT_COUNT);
	iwinfo_uc_add_array_from_bitmask(vm, obj, "auth_algs", c->auth_algs, 
		IWINFO_AUTH_NAMES, IWINFO_AUTH_COUNT);

	return obj;
}

/* Helper to convert scanlist_*_chan_entry to ucode object */
static void
iwinfo_uc_set_chaninfo(uc_vm_t *vm, uc_value_t *obj, 
                       struct iwinfo_scanlist_ht_chan_entry *ht, 
                       struct iwinfo_scanlist_vht_chan_entry *vht,
                       struct iwinfo_scanlist_vht_chan_entry *he,
                       struct iwinfo_scanlist_vht_chan_entry *eht)
{
	uc_value_t *obj_chaninfo;

	/* HT Operation (802.11n) */
	if (ht && (ht->primary_chan || ht->secondary_chan_off || ht->chan_width)) {
		obj_chaninfo = ucv_object_new(vm);
		ucv_object_add(obj_chaninfo, "primary_chan", ucv_int64_new(ht->primary_chan));
		ucv_object_add(obj_chaninfo, "secondary_chan_off", ucv_int64_new(ht->secondary_chan_off));
		ucv_object_add(obj_chaninfo, "chan_width", ucv_int64_new(ht->chan_width));
		ucv_object_add(obj, "ht_chan_info", obj_chaninfo);
	}

	/* VHT Operation (802.11ac) */
	if (vht && (vht->center_chan_1 || vht->center_chan_2 || vht->chan_width)) {
		obj_chaninfo = ucv_object_new(vm);
		ucv_object_add(obj_chaninfo, "center_chan_1", ucv_int64_new(vht->center_chan_1));
		ucv_object_add(obj_chaninfo, "center_chan_2", ucv_int64_new(vht->center_chan_2));
		ucv_object_add(obj_chaninfo, "chan_width", ucv_int64_new(vht->chan_width));
		ucv_object_add(obj, "vht_chan_info", obj_chaninfo);
	}

	/* HE Operation (802.11ax) */
	if (he && (he->center_chan_1 || he->center_chan_2 || he->chan_width)) {
		obj_chaninfo = ucv_object_new(vm);
		ucv_object_add(obj_chaninfo, "center_chan_1", ucv_int64_new(he->center_chan_1));
		ucv_object_add(obj_chaninfo, "center_chan_2", ucv_int64_new(he->center_chan_2));
		ucv_object_add(obj_chaninfo, "chan_width", ucv_int64_new(he->chan_width));
		ucv_object_add(obj, "he_chan_info", obj_chaninfo);
	}

	/* EHT Operation (802.11be) */
	if (eht && (eht->center_chan_1 || eht->center_chan_2 || eht->chan_width)) {
		obj_chaninfo = ucv_object_new(vm);
		ucv_object_add(obj_chaninfo, "center_chan_1", ucv_int64_new(eht->center_chan_1));
		ucv_object_add(obj_chaninfo, "center_chan_2", ucv_int64_new(eht->center_chan_2));
		ucv_object_add(obj_chaninfo, "chan_width", ucv_int64_new(eht->chan_width));
		ucv_object_add(obj, "eht_chan_info", obj_chaninfo);
	}
}

/* Helper to convert rate entry to ucode object */
static void
iwinfo_uc_set_rateinfo(uc_vm_t *vm, uc_value_t *obj, struct iwinfo_rate_entry *r)
{
	ucv_object_add(obj, "rate", ucv_int64_new(r->rate));
	ucv_object_add(obj, "ht", ucv_boolean_new(r->is_ht));
	ucv_object_add(obj, "vht", ucv_boolean_new(r->is_vht));
	ucv_object_add(obj, "he", ucv_boolean_new(r->is_he));
	ucv_object_add(obj, "eht", ucv_boolean_new(r->is_eht));
	ucv_object_add(obj, "mhz", ucv_int64_new(r->mhz_hi * 256 + r->mhz));

	if (r->is_ht) {
		ucv_object_add(obj, "40mhz", ucv_boolean_new(r->is_40mhz));
		ucv_object_add(obj, "mcs", ucv_int64_new(r->mcs));
		ucv_object_add(obj, "short_gi", ucv_boolean_new(r->is_short_gi));
	} else if (r->is_vht || r->is_he || r->is_eht) {
		ucv_object_add(obj, "mcs", ucv_int64_new(r->mcs));
		ucv_object_add(obj, "nss", ucv_int64_new(r->nss));
		
		if (r->is_he) {
			ucv_object_add(obj, "he_gi", ucv_int64_new(r->he_gi));
			ucv_object_add(obj, "he_dcm", ucv_int64_new(r->he_dcm));
		}
		if (r->is_eht) {
			ucv_object_add(obj, "eht_gi", ucv_int64_new(r->eht_gi));
		}
		if (r->is_vht) {
			ucv_object_add(obj, "short_gi", ucv_boolean_new(r->is_short_gi));
		}
	}
}

/* --- Generic Method Implementations --- */

/* Template for Int return */
#define GENERIC_INT_OP(op_name) \
static uc_value_t * \
iwinfo_uc_##op_name(uc_vm_t *vm, size_t nargs) { \
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm); \
	int val; \
	if (!ifname || !ops) return NULL; \
	if (!ops->op_name(ifname, &val)) return ucv_int64_new(val); \
	return NULL; \
}

/* Template for String return */
#define GENERIC_STR_OP(op_name) \
static uc_value_t * \
iwinfo_uc_##op_name(uc_vm_t *vm, size_t nargs) { \
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm); \
	char buf[IWINFO_BUFSIZE] = { 0 }; \
	if (!ifname || !ops) return NULL; \
	if (!ops->op_name(ifname, buf)) return ucv_string_new(buf); \
	return NULL; \
}

GENERIC_INT_OP(channel)
GENERIC_INT_OP(frequency)
GENERIC_INT_OP(frequency_offset)
GENERIC_INT_OP(txpower)
GENERIC_INT_OP(txpower_offset)
GENERIC_INT_OP(bitrate)
GENERIC_INT_OP(signal)
GENERIC_INT_OP(noise)
GENERIC_INT_OP(quality)
GENERIC_INT_OP(quality_max)
GENERIC_STR_OP(ssid)
GENERIC_STR_OP(bssid)
GENERIC_STR_OP(country)
GENERIC_STR_OP(hardware_name)
GENERIC_STR_OP(phyname)

static uc_value_t *
iwinfo_uc_mode(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	int mode;

	if (!ifname || !ops) return NULL;

	if (ops->mode(ifname, &mode))
		mode = IWINFO_OPMODE_UNKNOWN;

	return ucv_string_new(IWINFO_OPMODE_NAMES[mode]);
}

static uc_value_t *
iwinfo_uc_htmode(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	int mode;
	int i;

	if (!ifname || !ops) return NULL;

	if (ops->htmode(ifname, &mode))
		return NULL;

	for (i = 0; i < IWINFO_HTMODE_COUNT; i++) {
		// op->htmode returns bitmask, we have find bit set to 1
		if (mode & (1 << i)) {
			return ucv_string_new(IWINFO_HTMODE_NAMES[i]);
		}
	}

	return NULL;
}

static uc_value_t *
iwinfo_uc_encryption(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	struct iwinfo_crypto_entry c = { 0 };

	if (!ifname || !ops) return NULL;

	if (!ops->encryption(ifname, (char *)&c))
		return iwinfo_uc_cryptotable(vm, &c);

	return NULL;
}

static uc_value_t *
iwinfo_uc_hwmodelist(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	int modes = 0;
	uc_value_t *obj;

	if (!ifname || !ops) return NULL;
	if (ops->hwmodelist(ifname, &modes)) return NULL;

	obj = ucv_object_new(vm);
	ucv_object_add(obj, "a", ucv_boolean_new(modes & IWINFO_80211_A));
	ucv_object_add(obj, "b", ucv_boolean_new(modes & IWINFO_80211_B));
	ucv_object_add(obj, "g", ucv_boolean_new(modes & IWINFO_80211_G));
	ucv_object_add(obj, "n", ucv_boolean_new(modes & IWINFO_80211_N));
	ucv_object_add(obj, "ac", ucv_boolean_new(modes & IWINFO_80211_AC));
	ucv_object_add(obj, "ad", ucv_boolean_new(modes & IWINFO_80211_AD));
	ucv_object_add(obj, "ax", ucv_boolean_new(modes & IWINFO_80211_AX));
	ucv_object_add(obj, "be", ucv_boolean_new(modes & IWINFO_80211_BE));

	return obj;
}

static uc_value_t *
iwinfo_uc_htmodelist(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	int modes = 0;
	int i;
	uc_value_t *obj;

	if (!ifname || !ops) return NULL;
	if (ops->htmodelist(ifname, &modes)) return NULL;

	obj = ucv_object_new(vm);
	for (i = 0; i < IWINFO_HTMODE_COUNT; i++)
		ucv_object_add(obj, IWINFO_HTMODE_NAMES[i], ucv_boolean_new(modes & (1 << i)));
	
	return obj;
}

static uc_value_t *
iwinfo_uc_mbssid_support(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	int val = 0;

	if (!ifname || !ops) return NULL;

	if (!ops->mbssid_support(ifname, &val))
		return ucv_boolean_new(val);
	return NULL;
}

static uc_value_t *
iwinfo_uc_hardware_id(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	struct iwinfo_hardware_id ids;
	uc_value_t *obj;

	if (!ifname || !ops) return NULL;

	if (!ops->hardware_id(ifname, (char *)&ids)) {
		obj = ucv_object_new(vm);
		ucv_object_add(obj, "vendor_id", ucv_int64_new(ids.vendor_id));
		ucv_object_add(obj, "device_id", ucv_int64_new(ids.device_id));
		ucv_object_add(obj, "subsystem_vendor_id", ucv_int64_new(ids.subsystem_vendor_id));
		ucv_object_add(obj, "subsystem_device_id", ucv_int64_new(ids.subsystem_device_id));
		return obj;
	}
	return NULL;
}

static uc_value_t *
iwinfo_uc_assoclist(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	
	/* 
	 * NOTE: We allocate a specific large buffer here (64KB).
	 * Even if MTK backend doesn't usually return this much, the driver 
	 * supports up to 544 clients (MAX_NUMBER_OF_MAC in mtwifi.h).
	 * 544 * ~150 bytes > 4KB (standard IWINFO_BUFSIZE).
	 * We use heap (calloc) to avoid stack overflow.
	 */
	int i, len = IWINFO_BIG_BUFSIZE;
	char *buf;
	char macstr[18];
	struct iwinfo_assoclist_entry *e;
	uc_value_t *res, *obj, *obj_rx, *obj_tx;

	if (!ifname || !ops) return NULL;

	buf = calloc(1, len);
	if (!buf) {
		uc_vm_raise_exception(vm, EXCEPTION_TYPE, "Out of memory");
		return NULL;
	}

	if (ops->assoclist(ifname, buf, &len) || len <= 0) {
		free(buf);
		return ucv_object_new(vm);
	}

	res = ucv_object_new(vm);
	for (i = 0; i < len; i += sizeof(struct iwinfo_assoclist_entry)) {
		e = (struct iwinfo_assoclist_entry *) &buf[i];
		snprintf(macstr, sizeof(macstr), "%02X:%02X:%02X:%02X:%02X:%02X",
			e->mac[0], e->mac[1], e->mac[2], e->mac[3], e->mac[4], e->mac[5]);

		obj = ucv_object_new(vm);
		ucv_object_add(obj, "signal", ucv_int64_new(e->signal));
		ucv_object_add(obj, "noise", ucv_int64_new(e->noise));
		ucv_object_add(obj, "inactive", ucv_int64_new(e->inactive));
		ucv_object_add(obj, "rx_packets", ucv_uint64_new(e->rx_packets));
		ucv_object_add(obj, "tx_packets", ucv_uint64_new(e->tx_packets));
		ucv_object_add(obj, "rx_bytes", ucv_uint64_new(e->rx_bytes));
		ucv_object_add(obj, "tx_bytes", ucv_uint64_new(e->tx_bytes));

		obj_rx = ucv_object_new(vm);
		iwinfo_uc_set_rateinfo(vm, obj_rx, &e->rx_rate);
		ucv_object_add(obj, "rx_rate", obj_rx);

		obj_tx = ucv_object_new(vm);
		iwinfo_uc_set_rateinfo(vm, obj_tx, &e->tx_rate);
		ucv_object_add(obj, "tx_rate", obj_tx);

		if (e->thr)
			ucv_object_add(obj, "expected_throughput", ucv_int64_new(e->thr));

		ucv_object_add(res, macstr, obj);
	}

	free(buf);
	return res;
}

static uc_value_t *
iwinfo_uc_scanlist(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	/* 
	 * NOTE: Large buffer for scanlist as well. 
	 * Dense environments can return many APs.
	 */
	int i, len = IWINFO_BIG_BUFSIZE;
	char *buf;
	char macstr[18];
	struct iwinfo_scanlist_entry *e;
	uc_value_t *res, *obj;

	if (!ifname || !ops) return NULL;

	buf = calloc(1, len);
	if (!buf) {
		uc_vm_raise_exception(vm, EXCEPTION_TYPE, "Out of memory");
		return NULL;
	}

	if (ops->scanlist(ifname, buf, &len) || len <= 0) {
		free(buf);
		return ucv_array_new(vm);
	}

	res = ucv_array_new(vm);
	for (i = 0; i < len; i += sizeof(struct iwinfo_scanlist_entry)) {
		e = (struct iwinfo_scanlist_entry *) &buf[i];
		obj = ucv_object_new(vm);

		snprintf(macstr, sizeof(macstr), "%02X:%02X:%02X:%02X:%02X:%02X",
			e->mac[0], e->mac[1], e->mac[2], e->mac[3], e->mac[4], e->mac[5]);

		ucv_object_add(obj, "bssid", ucv_string_new(macstr));
		if (e->ssid[0])
			ucv_object_add(obj, "ssid", ucv_string_new((char *)e->ssid));

		ucv_object_add(obj, "channel", ucv_int64_new(e->channel));
		ucv_object_add(obj, "frequency", ucv_int64_new(e->mhz));
		ucv_object_add(obj, "mode", ucv_string_new(IWINFO_OPMODE_NAMES[e->mode]));
		ucv_object_add(obj, "quality", ucv_int64_new(e->quality));
		ucv_object_add(obj, "quality_max", ucv_int64_new(e->quality_max));
		ucv_object_add(obj, "signal", ucv_int64_new(e->signal - 0x100));
		
		ucv_object_add(obj, "encryption", iwinfo_uc_cryptotable(vm, &e->crypto));

		iwinfo_uc_set_chaninfo(vm, obj, 
			&e->ht_chan_info,
			&e->vht_chan_info,
			/* only in newer version */
			NULL, // &e->he_chan_info,
			NULL // &e->eht_chan_info
		);

		ucv_array_push(res, obj);
	}

	free(buf);
	return res;
}

static uc_value_t *
iwinfo_uc_freqlist(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	int i, len = IWINFO_BUFSIZE;
	char *buf;
	struct iwinfo_freqlist_entry *e;
	uc_value_t *res, *obj;

	if (!ifname || !ops) return NULL;

	/* freqlist is usually small (< 1KB), but we use calloc to be safe */
	buf = calloc(1, IWINFO_BUFSIZE);
	if (!buf) {
		uc_vm_raise_exception(vm, EXCEPTION_TYPE, "Out of memory");
		return NULL;
	}

	if (ops->freqlist(ifname, buf, &len) || len <= 0) {
		free(buf);
		return ucv_array_new(vm);
	}

	res = ucv_array_new(vm);
	for (i = 0; i < len; i += sizeof(struct iwinfo_freqlist_entry)) {
		e = (struct iwinfo_freqlist_entry *) &buf[i];
		obj = ucv_object_new(vm);
		ucv_object_add(obj, "mhz", ucv_int64_new(e->mhz));
		ucv_object_add(obj, "channel", ucv_int64_new(e->channel));
		ucv_object_add(obj, "restricted", ucv_boolean_new(e->restricted));
		ucv_array_push(res, obj);
	}

	free(buf);
	return res;
}

static uc_value_t *
iwinfo_uc_txpwrlist(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	int i, len = IWINFO_BUFSIZE;
	char *buf;
	struct iwinfo_txpwrlist_entry *e;
	uc_value_t *res, *obj;

	if (!ifname || !ops) return NULL;

	buf = calloc(1, IWINFO_BUFSIZE);
	if (!buf) {
		uc_vm_raise_exception(vm, EXCEPTION_TYPE, "Out of memory");
		return NULL;
	}

	if (ops->txpwrlist(ifname, buf, &len) || len <= 0) {
		free(buf);
		return ucv_array_new(vm);
	}

	res = ucv_array_new(vm);
	for (i = 0; i < len; i += sizeof(struct iwinfo_txpwrlist_entry)) {
		e = (struct iwinfo_txpwrlist_entry *) &buf[i];
		obj = ucv_object_new(vm);
		ucv_object_add(obj, "dbm", ucv_int64_new(e->dbm));
		ucv_object_add(obj, "mw", ucv_int64_new(e->mw));
		ucv_array_push(res, obj);
	}

	free(buf);
	return res;
}

static uc_value_t *
iwinfo_uc_countrylist(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	int len = IWINFO_BUFSIZE, i;
	char *buf;
	char alpha2[3];
	struct iwinfo_country_entry *c;
	const struct iwinfo_iso3166_label *l;
	uc_value_t *res, *obj;

	if (!ifname || !ops) return NULL;

	buf = calloc(1, IWINFO_BUFSIZE);
	if (!buf) {
		uc_vm_raise_exception(vm, EXCEPTION_TYPE, "Out of memory");
		return NULL;
	}

	if (ops->countrylist(ifname, buf, &len) || len <= 0) {
		free(buf);
		return ucv_array_new(vm);
	}

	res = ucv_array_new(vm);
	for (l = IWINFO_ISO3166_NAMES; l->iso3166; l++) {
		const char *ccode_str = NULL;
		
		for (i = 0; i < len; i += sizeof(struct iwinfo_country_entry)) {
			c = (struct iwinfo_country_entry *) &buf[i];
			if (c->iso3166 == l->iso3166) {
				ccode_str = c->ccode;
				break;
			}
		}

		if (ccode_str) {
			snprintf(alpha2, sizeof(alpha2), "%c%c", 
				(l->iso3166 / 256), (l->iso3166 % 256));
			obj = ucv_object_new(vm);
			ucv_object_add(obj, "alpha2", ucv_string_new(alpha2));
			ucv_object_add(obj, "ccode", ucv_string_new(ccode_str));
			ucv_object_add(obj, "name", ucv_string_new(l->name));
			ucv_array_push(res, obj);
		}
	}

	free(buf);
	return res;
}

static uc_value_t *
iwinfo_uc_lookup_phy(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, section); \
	const struct iwinfo_ops *ops = iwinfo_uc_get_ops(vm);
	char buf[IWINFO_BUFSIZE] = { 0 };

	if (!section || !ops) return NULL;

	if (ops->lookup_phy && !ops->lookup_phy(section, buf))
		return ucv_string_new(buf);

	return NULL;
}

/* Method list for the backend object */
static const uc_function_list_t backend_methods[] = {
	{ "channel",		iwinfo_uc_channel },
	{ "frequency",		iwinfo_uc_frequency },
	{ "frequency_offset", iwinfo_uc_frequency_offset },
	{ "txpower",		iwinfo_uc_txpower },
	{ "txpower_offset",	iwinfo_uc_txpower_offset },
	{ "bitrate",		iwinfo_uc_bitrate },
	{ "signal",			iwinfo_uc_signal },
	{ "noise",			iwinfo_uc_noise },
	{ "quality",		iwinfo_uc_quality },
	{ "quality_max",	iwinfo_uc_quality_max },
	{ "ssid",			iwinfo_uc_ssid },
	{ "bssid",			iwinfo_uc_bssid },
	{ "country",		iwinfo_uc_country },
	{ "hardware_name",	iwinfo_uc_hardware_name },
	{ "phyname",		iwinfo_uc_phyname },
	{ "mode",			iwinfo_uc_mode },
	{ "htmode",         iwinfo_uc_htmode },
	{ "assoclist",		iwinfo_uc_assoclist },
	{ "txpwrlist",		iwinfo_uc_txpwrlist },
	{ "scanlist",		iwinfo_uc_scanlist },
	{ "freqlist",		iwinfo_uc_freqlist },
	{ "countrylist",	iwinfo_uc_countrylist },
	{ "hwmodelist",		iwinfo_uc_hwmodelist },
	{ "htmodelist",		iwinfo_uc_htmodelist },
	{ "encryption",		iwinfo_uc_encryption },
	{ "mbssid_support",	iwinfo_uc_mbssid_support },
	{ "hardware_id",	iwinfo_uc_hardware_id },
	{ "lookup_phy",		iwinfo_uc_lookup_phy },
};

/* --- Top Level Functions --- */

/* Get backend type string for an interface */
static uc_value_t *
iwinfo_uc_type(uc_vm_t *vm, size_t nargs)
{
	CHECK_STRING(vm, nargs, ifname);
	const char *type;

	if (!ifname) return NULL;

	type = iwinfo_type(ifname);
	return type ? ucv_string_new(type) : NULL;
}

static uc_value_t *
iwinfo_uc__gc(uc_vm_t *vm, size_t nargs)
{
	iwinfo_finish();
	return NULL;
}

static const uc_function_list_t global_fns[] = {
	{ "type", iwinfo_uc_type },
	{ "__gc", iwinfo_uc__gc },
};

static void
register_constants(uc_vm_t *vm, uc_value_t *scope)
{
	uc_value_t *c = ucv_object_new(vm);

#define ADD_CONST(x) ucv_object_add(c, #x, ucv_int64_new(x))

	/* Operation modes */
	ADD_CONST(IWINFO_OPMODE_UNKNOWN);
	ADD_CONST(IWINFO_OPMODE_MASTER);
	ADD_CONST(IWINFO_OPMODE_ADHOC);
	ADD_CONST(IWINFO_OPMODE_CLIENT);
	ADD_CONST(IWINFO_OPMODE_MONITOR);
	ADD_CONST(IWINFO_OPMODE_AP_VLAN);
	ADD_CONST(IWINFO_OPMODE_WDS);
	ADD_CONST(IWINFO_OPMODE_MESHPOINT);
	ADD_CONST(IWINFO_OPMODE_P2P_CLIENT);
	ADD_CONST(IWINFO_OPMODE_P2P_GO);

	/* 802.11 capabilities */
	ADD_CONST(IWINFO_80211_A);
	ADD_CONST(IWINFO_80211_B);
	ADD_CONST(IWINFO_80211_G);
	ADD_CONST(IWINFO_80211_N);
	ADD_CONST(IWINFO_80211_AC);
	ADD_CONST(IWINFO_80211_AD);
	ADD_CONST(IWINFO_80211_AX);
	ADD_CONST(IWINFO_80211_BE);

	/* HT Modes */
	ADD_CONST(IWINFO_HTMODE_HT20);
	ADD_CONST(IWINFO_HTMODE_HT40);
	ADD_CONST(IWINFO_HTMODE_VHT20);
	ADD_CONST(IWINFO_HTMODE_VHT40);
	ADD_CONST(IWINFO_HTMODE_VHT80);
	ADD_CONST(IWINFO_HTMODE_VHT80_80);
	ADD_CONST(IWINFO_HTMODE_VHT160);
	ADD_CONST(IWINFO_HTMODE_NOHT);
	ADD_CONST(IWINFO_HTMODE_HE20);
	ADD_CONST(IWINFO_HTMODE_HE40);
	ADD_CONST(IWINFO_HTMODE_HE80);
	ADD_CONST(IWINFO_HTMODE_HE80_80);
	ADD_CONST(IWINFO_HTMODE_HE160);
	ADD_CONST(IWINFO_HTMODE_EHT20);
	ADD_CONST(IWINFO_HTMODE_EHT40);
	ADD_CONST(IWINFO_HTMODE_EHT80);
	ADD_CONST(IWINFO_HTMODE_EHT80_80);
	ADD_CONST(IWINFO_HTMODE_EHT160);
	ADD_CONST(IWINFO_HTMODE_EHT320);

	ucv_object_add(scope, "const", c);
}

/* 
 * Initialize the module.
 * This is where we dynamically look up backends.
 */
void uc_module_init(uc_vm_t *vm, uc_value_t *scope)
{
	/* Register global functions */
	uc_function_list_register(scope, global_fns);

	/* Register constants */
	register_constants(vm, scope);

	/* Define the 'iwinfo.backend' resource type.
	   This type carries the 'struct iwinfo_ops*' pointer.
	   We register the method table directly to the type, making resources callable. */
	iwinfo_backend_type = uc_type_declare(vm, "iwinfo.backend", backend_methods, NULL);

	/* List of possible backends to probe */
	const char *known_backends[] = {
		"nl80211", "mtk", "wext", "wl", "madwifi", NULL
	};

	for (const char **name = known_backends; *name; name++) {
		const struct iwinfo_ops *ops = iwinfo_backend_by_name(*name);

		/* If the backend exists in libiwinfo.so, export it to ucode */
		if (ops) {
			/* Create a resource holding the ops pointer */
			uc_value_t *res = uc_resource_new(iwinfo_backend_type, (void *)ops);
			
			/* Add it to the module scope (e.g. iwinfo.nl80211) */
			ucv_object_add(scope, *name, res);
		}
	}
}
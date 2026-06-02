/* Copyright (C) 2025 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

int handle_set_cbf_self_cfg(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0;
	char *token;
	void *data;
	char *data_str;
	u32 cbf_param[7];

	if (argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	data_str = argv[0];

	token = strtok(data_str, ",");

	if (token == NULL)
		return -EINVAL;

	i = 0;
	while (i < ARRAY_SIZE(cbf_param)) {
		errno = 0;
		cbf_param[i] = strtol(token, NULL, 10);

		if (errno == ERANGE || errno == EINVAL)
			return -EINVAL;

		i++;
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(cbf_param))
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_CBF_SELF_CFG, sizeof(cbf_param), &cbf_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}


COMMAND(set, cbf_self_cfg,
		"<CBF Enable:0/1>,<Bypass RNR:0/1>,<Enable DFBK:0/1>,<Bypass SCR:0/1>,<ICF Padding:0-6>,<ICF Rate:0-2>,<PPDU Type:0/1>",
		MTK_NL80211_VENDOR_SUBCMD_SET_CBF,
		0, CIB_NETDEV, handle_set_cbf_self_cfg,
		"Set CBF SELF CFG.\n");

int handle_set_cbf_mbss_sta_cfg(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0, ret;
	char *token, *mac_addr = NULL;
	void *data;
	char *data_str;
	u32 cbf_param[8];

	if (argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	data_str = argv[0];

	token = strtok(data_str, ",");

	if (token == NULL)
		return -EINVAL;

	i = 0;
	while (i < ARRAY_SIZE(cbf_param)) {
		if (i == 2) {
			mac_addr = token;
			i += 6;
		}
		else {
			errno = 0;
			cbf_param[i] = strtol(token, NULL, 10);

			if (errno == ERANGE || errno == EINVAL)
				return -EINVAL;

			i++;
		}

		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(cbf_param) || mac_addr == NULL)
		return -EINVAL;

	ret = sscanf(mac_addr, "%x:%x:%x:%x:%x:%x",
		&cbf_param[2], &cbf_param[3], &cbf_param[4],
		&cbf_param[5], &cbf_param[6], &cbf_param[7]);

	if (ret != 6)
		return -EINVAL;

	for (i = 0; i < MAC_ADDR_LEN; i++) {
		if (cbf_param[i+2] > 0xff) {
			return -EINVAL;
		}
	}

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_CBF_MBSS_STA_CFG, sizeof(cbf_param), &cbf_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, cbf_mbss_sta_cfg,
		"<CBF Enable:0/1>,<Wlan Index>,<STA MAC>",
		MTK_NL80211_VENDOR_SUBCMD_SET_CBF,
		0, CIB_NETDEV, handle_set_cbf_mbss_sta_cfg,
		"Set CBF MBSS STA CFG.\n");

int handle_set_cbf_obss_ap_cfg(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0, ret;
	char *token, *mac_addr = NULL, *bssid = NULL;
	void *data;
	char *data_str;
	u32 cbf_param[14];

	if (argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	data_str = argv[0];

	token = strtok(data_str, ",");

	if (token == NULL)
		return -EINVAL;

	i = 0;
	while (i < ARRAY_SIZE(cbf_param)) {
		if (i == 2) {
			mac_addr = token;
			i += 6;
		}
		else if (i == 8) {
			bssid = token;
			i += 6;
		}
		else {
			errno = 0;
			cbf_param[i] = strtol(token, NULL, 10);

			if (errno == ERANGE || errno == EINVAL)
				return -EINVAL;

			i++;
		}
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(cbf_param) || mac_addr == NULL || bssid == NULL)
		return -EINVAL;

	ret = sscanf(mac_addr, "%x:%x:%x:%x:%x:%x",
		&cbf_param[2], &cbf_param[3], &cbf_param[4],
		&cbf_param[5], &cbf_param[6], &cbf_param[7]);

	if (ret != 6)
		return -EINVAL;

	for (i = 0; i < MAC_ADDR_LEN; i++) {
		if (cbf_param[i+2] > 0xff) {
			return -EINVAL;
		}
	}

	ret = sscanf(bssid, "%x:%x:%x:%x:%x:%x",
		&cbf_param[8], &cbf_param[9], &cbf_param[10],
		&cbf_param[11], &cbf_param[12], &cbf_param[13]);

	if (ret != 6)
		return -EINVAL;

	for (i = 0; i < MAC_ADDR_LEN; i++) {
		if (cbf_param[i+8] > 0xff) {
			return -EINVAL;
		}
	}

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_CBF_OBSS_AP_CFG, sizeof(cbf_param), &cbf_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, cbf_obss_ap_cfg,
		"<CBF Enable:0/1>,<AP AID>,<AP MAC>,<AP BSSID>",
		MTK_NL80211_VENDOR_SUBCMD_SET_CBF,
		0, CIB_NETDEV, handle_set_cbf_obss_ap_cfg,
		"Set CBF OBSS AP CFG.\n");

int handle_set_cfg_obss_sta_cfg(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0, ret;
	char *token, *mac_addr = NULL;
	void *data;
	char *data_str;
	u32 cbf_param[8];

	if (argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	data_str = argv[0];

	token = strtok(data_str, ",");

	if (token == NULL)
		return -EINVAL;

	i = 0;
	while (i < ARRAY_SIZE(cbf_param)) {
		if (i == 2) {
			mac_addr = token;
			i += 6;
		}
		else {
			errno = 0;
			cbf_param[i] = strtol(token, NULL, 10);

			if (errno == ERANGE || errno == EINVAL)
				return -EINVAL;

			i++;
		}
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(cbf_param) || mac_addr == NULL)
		return -EINVAL;

	ret = sscanf(mac_addr, "%x:%x:%x:%x:%x:%x",
		&cbf_param[2], &cbf_param[3], &cbf_param[4],
		&cbf_param[5], &cbf_param[6], &cbf_param[7]);

	if (ret != 6)
		return -EINVAL;

	for (i = 0; i < MAC_ADDR_LEN; i++) {
		if (cbf_param[i+2] > 0xff) {
			return -EINVAL;
		}
	}

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_CBF_OBSS_STA_CFG, sizeof(cbf_param), &cbf_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, cbf_obss_sta_cfg,
		"<CBF Enable:0/1>,<STA PFMUID>,<STA MAC>",
		MTK_NL80211_VENDOR_SUBCMD_SET_CBF,
		0, CIB_NETDEV, handle_set_cfg_obss_sta_cfg,
		"Set CBF OBSS STA CFG.\n");


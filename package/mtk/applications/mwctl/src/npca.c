/* Copyright (C) 2025 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

int handle_set_npca_glb_mode(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0;
	char *token;
	void *data;
	char *data_str;
	u32 npca_param[3];

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
	while (i < ARRAY_SIZE(npca_param)) {
		errno = 0;
		npca_param[i] = strtol(token, NULL, 10);

		if (errno == ERANGE || errno == EINVAL)
			return -EINVAL;

		i++;
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(npca_param))
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_NPCA_GLB_MODE, sizeof(npca_param), &npca_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, npca_glb_mode,
		"<txop_en:0/1/2>,<ppdu_en:0/1>,<txop_en:0/1/2>",
		MTK_NL80211_VENDOR_SUBCMD_SET_NPCA,
		0, CIB_NETDEV, handle_set_npca_glb_mode,
		"Set NPCA GLB MODE.\n");

int handle_set_npca_glb_threshold(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0;
	char *token;
	void *data;
	char *data_str;
	u32 npca_param[5];

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
	while (i < ARRAY_SIZE(npca_param)) {
		errno = 0;
		npca_param[i] = strtol(token, NULL, 10);

		if (errno == ERANGE || errno == EINVAL)
			return -EINVAL;

		i++;
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(npca_param))
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_NPCA_GLB_THRES, sizeof(npca_param), &npca_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, npca_glb_threshold,
		"<obss_pwr_ppdu:RCPI threshold for PPDU>,<obss_pwr_ctrl:RCPI threshold for Ctrl frame>,<obss_ppdu_len:Len threshold>,<obss_ctrl_duration:Duration threshold>,<rts_ind_timeout_thres>",
		MTK_NL80211_VENDOR_SUBCMD_SET_NPCA,
		0, CIB_NETDEV, handle_set_npca_glb_threshold,
		"Set NPCA GLB THRESHOLD.\n");

int handle_set_npca_glb_ch_swt(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0;
	char *token;
	void *data;
	char *data_str;
	u32 npca_param[5];

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
	while (i < ARRAY_SIZE(npca_param)) {
		errno = 0;
		npca_param[i] = strtol(token, NULL, 10);

		if (errno == ERANGE || errno == EINVAL)
			return -EINVAL;

		i++;
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(npca_param))
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_NPCA_GLB_CH_SWT, sizeof(npca_param), &npca_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, npca_glb_ch_swt,
		"<pd_idx1:0-15>,<pd_idx2:0-15>,<bw_bitmap_switch_en:0/1>,<bw_bitmap_switch_type:0/1>,<bw_bitmap_switch_pd_sel:0/1>",
		MTK_NL80211_VENDOR_SUBCMD_SET_NPCA,
		0, CIB_NETDEV, handle_set_npca_glb_ch_swt,
		"Set NPCA GLB CH SWT.\n");

int handle_set_npca_glb_bypass(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0;
	char *token;
	void *data;
	char *data_str;
	u32 npca_param[7];

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
	while (i < ARRAY_SIZE(npca_param)) {
		errno = 0;
		npca_param[i] = strtol(token, NULL, 10);

		if (errno == ERANGE || errno == EINVAL)
			return -EINVAL;

		i++;
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(npca_param))
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_NPCA_GLB_BYPASS, sizeof(npca_param), &npca_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, npca_glb_bypass,
		"<not_mycolor:0/1>,<not2me:0/1>,<dl:0/1>,<power:0/1>,<insuf_snr:0/1>,<obss_color:0/1>,<obss_addr:0/1>",
		MTK_NL80211_VENDOR_SUBCMD_SET_NPCA,
		0, CIB_NETDEV, handle_set_npca_glb_bypass,
		"Set NPCA GLB BYPASS.\n");

int handle_set_npca_glb_sram_addr(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0, ret;
	char *token, *mac_addr = NULL;
	void *data;
	char *data_str;
	u32 npca_param[8];

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
	while (i < ARRAY_SIZE(npca_param)) {
		if (i == 2) {
			mac_addr = token;
			i += 6;
		}
		else {
			errno = 0;
			npca_param[i] = strtol(token, NULL, 10);

			if (errno == ERANGE || errno == EINVAL)
				return -EINVAL;
			i++;
		}

		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(npca_param) || mac_addr == NULL)
		return -EINVAL;

	ret = sscanf(mac_addr, "%x:%x:%x:%x:%x:%x",
		&npca_param[2], &npca_param[3], &npca_param[4],
		&npca_param[5], &npca_param[6], &npca_param[7]);

	if (ret != 6)
		return -EINVAL;

	for (i = 0; i < MAC_ADDR_LEN; i++) {
		if (npca_param[i+2] > 0xff) {
			return -EINVAL;
		}
	}

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_NPCA_GLB_SRAM_ADDR, sizeof(npca_param), &npca_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, npca_glb_sram_addr,
		"<idx:0-7>,<enable:0/1>,<mac_addr>",
		MTK_NL80211_VENDOR_SUBCMD_SET_NPCA,
		0, CIB_NETDEV, handle_set_npca_glb_sram_addr,
		"Set NPCA GLB SRAM ADDR.\n");

int handle_set_npca_glb_sram_color(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0;
	char *token;
	void *data;
	char *data_str;
	u32 npca_param[3];

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
	while (i < ARRAY_SIZE(npca_param)) {
		errno = 0;
		npca_param[i] = strtol(token, NULL, 10);

		if (errno == ERANGE || errno == EINVAL)
				return -EINVAL;

		i++;
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(npca_param))
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_NPCA_GLB_SRAM_COLOR, sizeof(npca_param), &npca_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, npca_glb_sram_color,
		"<idx:0-7>,<enable:0/1>,<color:0-63>",
		MTK_NL80211_VENDOR_SUBCMD_SET_NPCA,
		0, CIB_NETDEV, handle_set_npca_glb_sram_color,
		"Set NPCA GLB SRAM COLOR.\n");

int handle_set_npca_sta_color(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0;
	char *token;
	void *data;
	char *data_str;
	u32 npca_param[3];

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
	while (i < ARRAY_SIZE(npca_param)) {
		errno = 0;
		npca_param[i] = strtol(token, NULL, 10);

		if (errno == ERANGE || errno == EINVAL)
				return -EINVAL;

		i++;
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(npca_param))
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_NPCA_STA_COLOR, sizeof(npca_param), &npca_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, npca_sta_color,
		"<add:0-delete/1-add>,<wlan_id>,<color:0-63>",
		MTK_NL80211_VENDOR_SUBCMD_SET_NPCA,
		0, CIB_NETDEV, handle_set_npca_sta_color,
		"Set NPCA STA COLOR.\n");

int handle_set_npca_sta_addr(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0, ret;
	char *token, *mac_addr = NULL;
	void *data;
	char *data_str;
	u32 npca_param[8];

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
	while (i < ARRAY_SIZE(npca_param)) {
		if (i == 2) {
			mac_addr = token;
			i += 6;
		}
		else {
			errno = 0;
			npca_param[i] = strtol(token, NULL, 10);

			if (errno == ERANGE || errno == EINVAL)
				return -EINVAL;

			i++;
		}
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(npca_param) || mac_addr == NULL)
		return -EINVAL;

	ret = sscanf(mac_addr, "%x:%x:%x:%x:%x:%x",
		&npca_param[2], &npca_param[3], &npca_param[4],
		&npca_param[5], &npca_param[6], &npca_param[7]);

	if (ret != 6)
		return -EINVAL;

	for (i = 0; i < MAC_ADDR_LEN; i++) {
		if (npca_param[i+2] > 0xff) {
			return -EINVAL;
		}
	}

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_NPCA_STA_ADDR, sizeof(npca_param), &npca_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, npca_sta_addr,
		"<add:0-delete/1-add>,<wlan_id>,<mac_addr>",
		MTK_NL80211_VENDOR_SUBCMD_SET_NPCA,
		0, CIB_NETDEV, handle_set_npca_sta_addr,
		"Set NPCA STA ADDR.\n");

int handle_set_npca_manual_dbg(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int i = 0;
	char *token;
	void *data;
	char *data_str;
	u32 npca_param[2];

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
	while (i < ARRAY_SIZE(npca_param)) {
		errno = 0;
		npca_param[i] = strtol(token, NULL, 10);

		if (errno == ERANGE || errno == EINVAL)
				return -EINVAL;

		i++;
		token = strtok(NULL, ",");

		if (token == NULL)
			break;
	}

	if (i != ARRAY_SIZE(npca_param))
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_NPCA_MANUAL_DBG, sizeof(npca_param), &npca_param))
			return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, npca_manual_dbg,
		"<item:setting item>,<value:setting value>",
		MTK_NL80211_VENDOR_SUBCMD_SET_NPCA,
		0, CIB_NETDEV, handle_set_npca_manual_dbg,
		"Set NPCA MANUAL DEBUG.\n");


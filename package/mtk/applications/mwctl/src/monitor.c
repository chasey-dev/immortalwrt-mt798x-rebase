/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

int handle_air_monitor_enable(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char monitor_enable;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		monitor_enable = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		monitor_enable = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_MONITOR_ENABLE, monitor_enable))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, mnt_en,
	"<0 or 1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_MONITOR, 0, CIB_NETDEV, handle_air_monitor_enable,
	"This command is used to configure air monitor en/disable");

int handle_air_monitor_rule(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned int val_tmp[3] = {0};
	unsigned char monitor_rule[3] = {0};
	int ret;
	unsigned char i;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%d:%d:%d", &val_tmp[0], &val_tmp[1], &val_tmp[2]);
	if (ret != 3)
		return -EINVAL;

	for (i = 0; i < 3; i++) {
		if (val_tmp[i] > 1)
			return -EINVAL;
		monitor_rule[i] = (unsigned char)val_tmp[i];
	}

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_MONITOR_RULE, sizeof(monitor_rule), monitor_rule))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, mnt_rule,
	"<data_mnt_en>:<mgmt_mnt_en>:<ctrl_mnt_en>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_MONITOR, 0, CIB_NETDEV, handle_air_monitor_rule,
	"This command is used to configure air monitor rule");

int handle_air_monitor_sta(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned int val_tmp[6] = {0};
	unsigned char monitor_mac_addr[6] = {0};
	int ret;
	unsigned char i;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%x:%x:%x:%x:%x:%x",
		&val_tmp[0], &val_tmp[1], &val_tmp[2],
		&val_tmp[3], &val_tmp[4], &val_tmp[5]);

	if (ret != 6)
		return -EINVAL;

	for (i = 0; i < 6; i++) {
		if (val_tmp[i] > 0xff) {
			return -EINVAL;
		}
		monitor_mac_addr[i] = (unsigned char)val_tmp[i];
	}

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA, sizeof(monitor_mac_addr), monitor_mac_addr))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, mnt_sta,
	"<monitor_mac_addr>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_MONITOR, 0, CIB_NETDEV, handle_air_monitor_sta,
	"This command is used to configure air monitor sta");

#define MAX_NUM_OF_MONITOR_STA 16
int handle_air_monitor_idx(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	int idx_tmp;
	unsigned char monitor_idx;
	int ret;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%d", &idx_tmp);

	if (ret != 1)
		return -EINVAL;

	monitor_idx = (u8)idx_tmp;
	if (monitor_idx >= MAX_NUM_OF_MONITOR_STA)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_MONITOR_IDX, monitor_idx))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, mnt_idx,
	"<idx>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_MONITOR, 0, CIB_NETDEV, handle_air_monitor_idx,
	"This command is used to configure air monitor idx");


int handle_air_monitor_clr(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_MONITOR_CLR, 1))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, mnt_clr,
	"None",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_MONITOR, 0, CIB_NETDEV, handle_air_monitor_clr,
	"This command is used to clear all air monitor counter");

int handle_air_monitor_sta0(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned int val_tmp[6] = {0};
	unsigned char monitor_mac_addr[6] = {0};
	int ret;
	unsigned char i;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%x:%x:%x:%x:%x:%x",
		&val_tmp[0], &val_tmp[1], &val_tmp[2],
		&val_tmp[3], &val_tmp[4], &val_tmp[5]);

	if (ret != 6)
		return -EINVAL;

	for (i = 0; i < 6; i++) {
		if (val_tmp[i] > 0xff) {
			return -EINVAL;
		}
		monitor_mac_addr[i] = (unsigned char)val_tmp[i];
	}

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_MONITOR_STA0, sizeof(monitor_mac_addr), monitor_mac_addr))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, mnt_sta0,
	"<monitor_mac_addr>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_MONITOR, 0, CIB_NETDEV, handle_air_monitor_sta0,
	"This command is used to configure air monitor sta with idx0");


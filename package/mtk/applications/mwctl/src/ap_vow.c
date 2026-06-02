/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

#define AP_VOW_OPTIONS "[atf_en=<0 or 1>] [atc_en=<group-0 or 1>] [bw_en=<0 or 1>] [bw_ctl_en=<group-0 or 1>]\n"	\
	"[min_rate=<group-value>] [max_rate=<group-value>] [min_ratio=<group-value>] [max_ratio=<group-value>]"

#define MAX_VOW_PARAM_LEN 128

struct vow_option {
	char option_name[MAX_VOW_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char *value);
};

int vow_atf_en_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char vow_atf_en;

	if (!value)
		return -EINVAL;

	if (*value == '0')
		vow_atf_en = 0;
	else if (*value == '1')
		vow_atf_en = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_VOW_ATF_EN_INFO, vow_atf_en))
		return -EMSGSIZE;

	return 0;
}

int vow_bw_en_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char vow_bw_en;

	if (!value)
		return -EINVAL;

	if (*value == '0')
		vow_bw_en = 0;
	else if (*value == '1')
		vow_bw_en = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_EN_INFO, vow_bw_en))
		return -EMSGSIZE;

	return 0;
}

#define VOW_MAX_GROUP_NUM 16
int vow_atc_en_attr_put(struct nl_msg *msg, char *value)
{
	struct vow_group_en_param param;
	int ret;

	if (!value)
		return -EINVAL;

	ret = sscanf(value, "%d-%d", &param.group, &param.en);
	if (ret > 1) {
		if (param.group >= VOW_MAX_GROUP_NUM)
			return -EINVAL;
		if (param.en != 0 && param.en != 1)
			return -EINVAL;

		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_VOW_ATC_EN_INFO, sizeof(param), &param))
			return -EMSGSIZE;
	} else
		return -EINVAL;
	return 0;
}

int vow_bw_ctl_en_attr_put(struct nl_msg *msg, char *value)
{
	struct vow_group_en_param param;
	int ret;

	if (!value)
		return -EINVAL;

	ret = sscanf(value, "%d-%d", &param.group, &param.en);
	if (ret > 1) {
		if (param.group >= VOW_MAX_GROUP_NUM)
			return -EINVAL;
		if (param.en != 0 && param.en != 1)
			return -EINVAL;

		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_VOW_BW_CTL_EN_INFO, sizeof(param), &param))
			return -EMSGSIZE;
	} else
		return -EINVAL;
	return 0;
}

int vow_min_rate_attr_put(struct nl_msg *msg, char *value)
{
	struct vow_rate_param param;
	int ret;

	if (!value)
		return -EINVAL;

	ret = sscanf(value, "%d-%d", &param.group, &param.rate);
	if (ret > 1) {
		if (param.group >= VOW_MAX_GROUP_NUM)
			return -EINVAL;

		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATE_INFO, sizeof(param), &param))
			return -EMSGSIZE;
	} else
		return -EINVAL;
	return 0;
}

int vow_max_rate_attr_put(struct nl_msg *msg, char *value)
{
	struct vow_rate_param param;
	int ret;

	if (!value)
		return -EINVAL;

	ret = sscanf(value, "%d-%d", &param.group, &param.rate);
	if (ret > 1) {
		if (param.group >= VOW_MAX_GROUP_NUM)
			return -EINVAL;

		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATE_INFO, sizeof(param), &param))
			return -EMSGSIZE;
	} else
		return -EINVAL;
	return 0;
}

int vow_min_ratio_attr_put(struct nl_msg *msg, char *value)
{
	struct vow_ratio_param param;
	int ret;

	if (!value)
		return -EINVAL;

	ret = sscanf(value, "%d-%d", &param.group, &param.ratio);
	if (ret > 1) {
		if (param.group >= VOW_MAX_GROUP_NUM)
			return -EINVAL;

		if (param.ratio > 100)
			return -EINVAL;

		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_VOW_MIN_RATIO_INFO, sizeof(param), &param))
			return -EMSGSIZE;
	} else
		return -EINVAL;
	return 0;
}

int vow_max_ratio_attr_put(struct nl_msg *msg, char *value)
{
	struct vow_ratio_param param;
	int ret;

	if (!value)
		return -EINVAL;

	ret = sscanf(value, "%d-%d", &param.group, &param.ratio);
	if (ret > 1) {
		if (param.group >= VOW_MAX_GROUP_NUM)
			return -EINVAL;

		if (param.ratio > 100)
			return -EINVAL;

		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_VOW_MAX_RATIO_INFO, sizeof(param), &param))
			return -EMSGSIZE;
	} else
		return -EINVAL;
	return 0;
}

struct vow_option vow_opt[] = {
	{"atf_en", vow_atf_en_attr_put},
	{"bw_en", vow_bw_en_attr_put},
	{"atc_en", vow_atc_en_attr_put},
	{"bw_ctl_en", vow_bw_ctl_en_attr_put},
	{"min_rate", vow_min_rate_attr_put},
	{"max_rate", vow_max_rate_attr_put},
	{"min_ratio", vow_min_ratio_attr_put},
	{"max_ratio", vow_max_ratio_attr_put},
};

int handle_ap_vow_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr, *param_str, *val_str, invalide = 0;
	int i, j;

	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	for (i = 0; i < argc; i++) {
		ptr = argv[i];
		param_str = ptr;
		val_str = strchr(ptr, '=');

		if (!val_str)
			continue;

		*val_str++ = 0;

		for (j = 0; j < (sizeof(vow_opt) / sizeof(vow_opt[0])); j++) {
			if (strlen(vow_opt[j].option_name) == strlen(param_str) &&
				!strncmp(vow_opt[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(vow_opt) / sizeof(vow_opt[0]))) {
			if (vow_opt[j].attr_put(msg, val_str) < 0)
				printf("invalide argument %s=%s, ignore it\n", param_str, val_str);
			else
				invalide = 1;
		}
	}
	nla_nest_end(msg, data);

	if (!invalide)
		return -EINVAL;

	return 0;
}

COMMAND(set, vow,
	AP_VOW_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_VOW, 0, CIB_NETDEV, handle_ap_vow_set,
	"This command is used to set vow information");

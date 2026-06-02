/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

#define CHANNEL_OPTIONS "[{num=<channel>|freq=<freq>}] [bw=<20|40|80|160|320>] [ext_chan=<below|above>] [ht_coex=<0|1>]"

#define MAX_CHAN_PARAM_LEN 128

struct chan_option {
	char option_name[MAX_CHAN_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char *value);
};

enum mwctl_chan_width {
	MWCTL_CHAN_WIDTH_20,
	MWCTL_CHAN_WIDTH_40,
	MWCTL_CHAN_WIDTH_80,
	MWCTL_CHAN_WIDTH_160,
	MWCTL_CHAN_WIDTH_320,
};

struct bw_option  {
	char option_name[MAX_CHAN_PARAM_LEN];
	enum mwctl_chan_width mode;
};

struct bw_option bw_opt[] = {
	{"20", MWCTL_CHAN_WIDTH_20},
	{"40", MWCTL_CHAN_WIDTH_40},
	{"80", MWCTL_CHAN_WIDTH_80},
	{"160", MWCTL_CHAN_WIDTH_160},
	{"320", MWCTL_CHAN_WIDTH_320},
};

int chan_num_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char chan_num;

	if (!value)
		return -EINVAL;

	chan_num = strtoul(value, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_CHAN_SET_NUM, chan_num))
		return -EMSGSIZE;

	return 0;
}

int freq_attr_put(struct nl_msg *msg, char *value)
{
	unsigned int freq;

	if (!value)
		return -EINVAL;

	freq = strtoul(value, NULL, 10);

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_CHAN_SET_FREQ, freq))
		return -EMSGSIZE;

	return 0;
}

int bw_attr_put(struct nl_msg *msg, char *value)
{
	int i;
	bool b_match = false;

	for (i = 0; i < (sizeof(bw_opt)/sizeof(bw_opt[0])); i++) {
		if (strlen(bw_opt[i].option_name) == strlen(value) &&
			!strncmp(bw_opt[i].option_name, value, strlen(value))) {
			b_match = true;
			if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_CHAN_SET_BW, bw_opt[i].mode))
				return -EMSGSIZE;
			break;
		}
	}

	if (!b_match)
		return -EINVAL;

	return 0;
}

struct ext_chan_option  {
	char option_name[MAX_CHAN_PARAM_LEN];
	unsigned char value;
};

struct ext_chan_option ext_chan_opt[] = {
	{"below", 0},
	{"above", 1},
};

int ext_chan_attr_put(struct nl_msg *msg, char *value)
{
	int i;
	bool b_match = false;

	for (i = 0; i < (sizeof(ext_chan_opt)/sizeof(ext_chan_opt[0])); i++) {
		if (strlen(ext_chan_opt[i].option_name) == strlen(value) &&
			!strncmp(ext_chan_opt[i].option_name, value, strlen(value))) {
			b_match = true;
			if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_EXTCHAN, ext_chan_opt[i].value))
				return -EMSGSIZE;
			break;
		}
	}

	if (!b_match)
		return -EINVAL;

	return 0;
}

int ht_coex_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char ht_coex;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		ht_coex = 1;
	else if (*value == '0')
		ht_coex = 0;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_CHAN_SET_HT_COEX, ht_coex))
		return -EMSGSIZE;

	return 0;
}

struct chan_option channel_opt[] = {
	{"num", chan_num_attr_put},
	{"freq", freq_attr_put},
	{"bw", bw_attr_put},
	{"ext_chan", ext_chan_attr_put},
	{"ht_coex", ht_coex_attr_put},
};

int handle_channel_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr, *param_str, *val_str, valid = 0;
	int i, j, ret = 0;

	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	ret = add_msg_band_link_id(msg, get_band_link_id(),
		MTK_NL80211_VENDOR_ATTR_CHAN_SET_BAND_LINK_ID);
	if (ret < 0)
		return ret;

	for (i = 0; i < argc; i++) {
		ptr = argv[i];
		param_str = ptr;
		val_str = strchr(ptr, '=');

		if (!val_str)
			continue;

		*val_str++ = 0;

		for (j = 0; j < (sizeof(channel_opt) / sizeof(channel_opt[0])); j++) {
			if (strlen(channel_opt[j].option_name) == strlen(param_str) &&
				!strncmp(channel_opt[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(channel_opt) / sizeof(channel_opt[0]))) {
			if (channel_opt[j].attr_put(msg, val_str) < 0)
				printf("Invalid argument %s=%s, ignore it\n", param_str, val_str);
			else
				valid = 1;
		}
	}
	nla_nest_end(msg, data);

	if (!valid)
		return -EINVAL;

	return 0;
}

COMMAND(set, channel,
	CHANNEL_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_CHANNEL, 0, CIB_PHY, handle_channel_set,
	"Set the channel number/frequency or bandwidth.\n");

/** Country command **/
#define COUNTRY_OPTIONS "[region=<country_region>]|[code=<country_code>]|[name=<country_name>]"

int country_code_attr_put(struct nl_msg *msg, char *value)
{
	int len;

	len = strlen(value);

	if (len != 2)
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_CODE, len, value))
		return -EMSGSIZE;

	return 0;
}

int country_region_attr_put(struct nl_msg *msg, char *value)
{
	u32 region;

	if (!value)
		return -EINVAL;

	region = strtoul(value, NULL, 10);

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_REGION, region))
		return -EMSGSIZE;

	return 0;
}

int country_name_attr_put(struct nl_msg *msg, char *value)
{
	int len;

	len = strlen(value);

	if (len >= 38)
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_COUNTRY_SET_NAME, len, value)) {
		printf("country_name invalid!\n");
		return -EMSGSIZE;
	}

	return 0;
}


struct chan_option country_opt[] = {
	{"region", country_region_attr_put},
	{"code", country_code_attr_put},
	{"name", country_name_attr_put},
};

int handle_country_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr, *param_str, *val_str, valid = 0;
	int i, j;

	if (!argc) {
		printf("Null argument!\n");
		return -EINVAL;
	}

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data) {
		printf("nla_nest_start check fail!\n");
		return -ENOMEM;
	}

	for (i = 0; i < argc; i++) {
		ptr = argv[i];
		param_str = ptr;
		val_str = strchr(ptr, '=');

		if (!val_str)
			continue;

		*val_str++ = 0;

		for (j = 0; j < (sizeof(country_opt) / sizeof(country_opt[0])); j++) {
			if (strlen(country_opt[j].option_name) == strlen(param_str) &&
				!strncmp(country_opt[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(country_opt) / sizeof(country_opt[0]))) {
			if (country_opt[j].attr_put(msg, val_str) < 0)
				printf("Invalid argument %s=%s, ignore it\n", param_str, val_str);
			else
				valid = 1;
		}
	}
	nla_nest_end(msg, data);

	if (!valid)
		return -EINVAL;

	return 0;
}

COMMAND(set, country,
	COUNTRY_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_COUNTRY, 0, CIB_PHY, handle_country_set,
	"Set the country code, region and string.\n");


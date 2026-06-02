/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

#define MAX_FTM_PARAM_LEN 128

struct ftm_option {
	char option_name[MAX_FTM_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char *value);
};


int range_req_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char range_req;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		range_req = 1;
	else if (*value == '0')
		range_req = 0;
	else
		return -EINVAL;


	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_RANG_REQ, range_req))
		return -EMSGSIZE;

	return 0;
}

int enable_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char role;

	if (!value)
		return -EINVAL;

	role = strtoul(value, NULL, 10);

	if (role > 0b1111)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_ENABLE, role))
		return -EMSGSIZE;

	return 0;
}

/* Burst Exponent */
int burst_exp_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char burst_exp;

	if (!value)
		return -EINVAL;

	burst_exp = strtoul(value, NULL, 10);

	if (burst_exp > 15)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_BURST_EXP, burst_exp))
		return -EMSGSIZE;

	return 0;
}

/* Burst Duration */
int burst_dur_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char burst_dur;

	if (!value)
		return -EINVAL;

	burst_dur = strtoul(value, NULL, 10);

	if (burst_dur > 15)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_BURST_DUR, burst_dur))
		return -EMSGSIZE;

	return 0;
}

int min_delta_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char min_delta;

	if (!value)
		return -EINVAL;

	min_delta = strtoul(value, NULL, 10);

	if (min_delta < 1 || min_delta > 255)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_MIN_DELTA, min_delta))
		return -EMSGSIZE;

	return 0;
}

/* Patial Timing Synchronization Function */
int ptsf_attr_put(struct nl_msg *msg, char *value)
{
	unsigned int ptsf;

	if (!value)
		return -EINVAL;

	ptsf = strtoul(value, NULL, 10);

	if (ptsf > 65535)
		return -EINVAL;

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_FTM_PARTIAL_TSF, ptsf))
		return -EMSGSIZE;

	return 0;
}

/* Patial TSF No Perference */
int ptsf_no_perfer_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char no_perfer;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		no_perfer = 1;
	else if (*value == '0')
		no_perfer = 0;
	else
		return -EINVAL;


	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_PTSF_NO_PREFERENCE, no_perfer))
		return -EMSGSIZE;

	return 0;
}

/* As Soon As Possible */
int asap_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char asap;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		asap = 1;
	else if (*value == '0')
		asap = 0;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_ASAP, asap))
		return -EMSGSIZE;

	return 0;
}

int ftm_num_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char ftm_num;

	if (!value)
		return -EINVAL;

	ftm_num = strtoul(value, NULL, 10);

	if (ftm_num == 1 || ftm_num > 31)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_NUM, ftm_num))
		return -EMSGSIZE;

	return 0;
}

int fmt_and_bw_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char fmt_and_bw;

	if (!value)
		return -EINVAL;

	fmt_and_bw = strtoul(value, NULL, 10);

	if (fmt_and_bw > 63)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_FMT_AND_BW, fmt_and_bw))
		return -EMSGSIZE;

	return 0;
}


int burst_period_attr_put(struct nl_msg *msg, char *value)
{
	unsigned int burst_period;

	if (!value)
		return -EINVAL;

	burst_period = strtoul(value, NULL, 10);

	if (burst_period > 65535)
		return -EINVAL;

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_FTM_BURST_PERIOD, burst_period))
		return -EMSGSIZE;

	return 0;
}

int target_mac_attr_put(struct nl_msg *msg, char *value)
{
	u8 Addr[ETH_ALEN];
	int matches;

	if (!value)
		return -EINVAL;

	matches = sscanf(value, "%5hhx:%5hhx:%5hhx:%5hhx:%5hhx:%5hhx",
		Addr, Addr+1, Addr+2, Addr+3, Addr+4, Addr+5);

	if (matches != ETH_ALEN)
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_FTM_TARGET_MAC, ETH_ALEN, Addr))
		return -EMSGSIZE;

	return 0;
}

int debug_attr_put(struct nl_msg *msg, char *value)
{
	if (!value)
		return -EINVAL;

	if (nla_put_string(msg, MTK_NL80211_VENDOR_ATTR_FTM_DEBUG, value))
		return -EMSGSIZE;

	return 0;
}

int role_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char role;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		role = 1;
	else if (*value == '2')
		role = 2;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_ROLE, role))
		return -EMSGSIZE;

	return 0;
}

int toae_attr_put(struct nl_msg *msg, char *value)
{
	if (!value)
		return -EINVAL;

	if (nla_put_string(msg, MTK_NL80211_VENDOR_ATTR_FTM_TOAE_CFG, value))
		return -EMSGSIZE;

	return 0;
}

int test_mode_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char enable;

	if (!value)
		return -EINVAL;

	if (*value == '0')
		enable = 0;
	else if (*value == '1')
		enable = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_TESTMODE, enable))
		return -EMSGSIZE;

	return 0;
}

int burst_timeout_attr_put(struct nl_msg *msg, char *value)
{
	unsigned int timeout;

	if (!value)
		return -EINVAL;

	timeout = strtoul(value, NULL, 10);

	if (timeout > 65535)
		return -EINVAL;

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_FTM_FTMR_TIMEOUT, timeout))
		return -EMSGSIZE;

	return 0;
}

int delay_time_attr_put(struct nl_msg *msg, char *value)
{
	unsigned int delay_time;

	if (!value)
		return -EINVAL;

	delay_time = strtoul(value, NULL, 10);

	if (delay_time > 65535)
		return -EINVAL;

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_FTM_NON_ASAP_DELAY_TIME, delay_time))
		return -EMSGSIZE;

	return 0;
}

int ntb_ranging_attr_put(struct nl_msg *msg, char *value)
{
	if (!value)
		return -EINVAL;

	if (nla_put_string(msg, MTK_NL80211_VENDOR_ATTR_FTM_NTB_RANGING_PARAMS, value))
		return -EMSGSIZE;

	return 0;
}

int ntb_meas_time_attr_put(struct nl_msg *msg, char *value)
{
	if (!value)
		return -EINVAL;

	if (nla_put_string(msg, MTK_NL80211_VENDOR_ATTR_FTM_NTB_MEAS_EXP, value))
		return -EMSGSIZE;

	return 0;
}

int ntb_req_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char range_req;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		range_req = 1;
	else if (*value == '0')
		range_req = 0;
	else
		return -EINVAL;


	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_NTB_RANG_REQ, range_req))
		return -EMSGSIZE;

	return 0;
}

struct ftm_option ftm_opt[] = {
	{"range_req", range_req_attr_put},
	{"enable", enable_attr_put},
	{"burst_exp", burst_exp_attr_put},
	{"burst_dur", burst_dur_attr_put},
	{"ptsf", ptsf_attr_put},
	{"ptsf_no_perfer", ptsf_no_perfer_attr_put},
	{"min_delta", min_delta_attr_put},
	{"asap", asap_attr_put},
	{"ftm_num", ftm_num_attr_put},
	{"fmt_bw", fmt_and_bw_attr_put},
	{"burst_period", burst_period_attr_put},
	{"target", target_mac_attr_put},
	{"debug", debug_attr_put},
	{"role", role_attr_put},
	{"toae", toae_attr_put},
	{"test_mode", test_mode_attr_put},
	{"burst_timeout", burst_timeout_attr_put},
	{"delay_time", delay_time_attr_put},
	{"ntb_ranging", ntb_ranging_attr_put},
	{"ntb_meas_time", ntb_meas_time_attr_put},
	{"ntb_req", ntb_req_attr_put},
};


int handle_ftm_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr, *param_str, *val_str, valid = 0;
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

		for (j = 0; j < (sizeof(ftm_opt) / sizeof(ftm_opt[0])); j++) {
			if (strlen(ftm_opt[j].option_name) == strlen(param_str) &&
				!strncmp(ftm_opt[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(ftm_opt) / sizeof(ftm_opt[0]))) {
			if (ftm_opt[j].attr_put(msg, val_str) < 0)
				printf("Invalid argument %s=%s, ignore it\n", param_str, val_str);
			else
				valid = 1;
		}

	}

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_DUMP_PARAMS, 1)) {
		printf("MTK_NL80211_VENDOR_ATTR_FTM_DUMP_PARAMS return error\n");
		return -EMSGSIZE;
	}

	nla_nest_end(msg, data);

	if (!valid)
		return -EINVAL;

	return 0;
}


COMMAND(set, ftm,
	"[range_req=<val:0-disable, 1-enable>]\n"
	"[enable=<val:0-disable, b0-mc_resp, b1-mc_init, b2-ntb_resp, b3-ntb_init>]\n"
	"[role=<val:1-responder, 2-initiator]\n"
	"[burst_exp=<val:0~15>][burst_dur=<val:0~15>]\n"
	"[ptsf=<val:0-65536>][ptsf_no_perfer=<val:0=indicated_ptsf, 1-no_perfer>]\n"
	"[min_delta=<val:1~255>][asap=<val:0-disable, 1-enable>]\n"
	"[delay_time=<val:1~65535>]\n"
	"[ftm_num=<val:2~31>][fmt_bw=<val:0-63>]\n"
	"[burst_period=<val:0-65535>][target=<mac_addr>]\n"
	"[toae=<bias>-<ant>-<speFtm>-<speFtmAck>-<chain>]\n"
	"[test_mode=<1>][burst_timeout=<val:0-65535>]\n"
	"[ntb_ranging=<I2rLmrFdbk>:<ImmR2IFdbk>:<ImmI2RFdbk>:<MaxR2iRep>:<MaxI2rRep>:<MaxR2iLtf>:<MaxI2rLtf>]\n"
	"[ntb_meas_time=<min_exp>:<max_exp>][ntb_req=<val:0-disable, 1-enable>]",
	MTK_NL80211_VENDOR_SUBCMD_FTM, 0, CIB_NETDEV, handle_ftm_set,
	"This command is used to set FTM (Fine Time Measurement) parameters");


int ftm_stat_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_FTM_STAT_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *show_str = NULL;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_FTM_STAT_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_FTM_STAT_STR]) {
			show_str = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_FTM_STAT_STR]);
			printf("%s\n", show_str);
		}
	} else
		printf("no any ftm stat string from driver\n");

	return 0;
}

int handle_ftm_stat(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *cmd_str;
	int value;

	register_handler(ftm_stat_callback, NULL);

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (!argc) {
		printf("Lack of argument\n");
		return -EINVAL;
	}

	cmd_str = argv[0];
	value = strtoul(cmd_str, NULL, 10);

	if (value < 1 || value > 2) {
		printf("Invalid argument: %s, ignore it\n", cmd_str);
		return -EINVAL;
	}

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_FTM_STAT_STR, value))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

DECLARE_SECTION(dump);

COMMAND(dump, ftm_stat,
	"[1-all, 2-as iSTA]",
	MTK_NL80211_VENDOR_SUBCMD_FTM_STAT, 0, CIB_NETDEV, handle_ftm_stat,
	"This command is used to query FTM (Fine Time Measurement) result and parameters");

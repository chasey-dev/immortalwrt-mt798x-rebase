/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

#define ACS_OPTIONS "[trigger=<3|7|8|9>] [psc=<1|0>(valid for 6G)]\n"\
	"[partial_scan =<1|0>] [scan_dwell=<milliseconds>] [restore_dwell =<milliseconds>] [ch_num=<num>]\n"\
	"[skip_ch=<null|ch1,ch2,...> | pref_ch=<ch1,ch2,...>] | [sta_num_thr=<num>] | [check_time=<seconds>]\n"\
	"[ch_util_thr =<percentage>] | [ice_ch_util_thr=<percentage>] | [data_rate_wt=<weight>] | [prio_wt=<weight>]\n"\
	"[tx_power_cons=<1|0>] | [max_acs_times=<num>] | [sw_ch_thr=<percentage>] | [obss_wt=<weight>]\n"

#define ACS_HELP "trigger acs operation\n"\
	"[trigger=3]: acs alg=ChannelAlgBusyTime\n"\
	"[trigger=7]: acs alg=ChannelAlgBusyTime and not switch channel\n"\
	"[trigger=8]: acs alg=Intel Minmax\n"\
	"[trigger=9]: acs alg=Intel Avg\n"\
	"[psc=1]: only scan and select 6G PSC channels\n"\
	"[partial_scan]: use partial scan or full scan in ACS\n"\
	"[scan_dwell]: dwell time on a scanning channel\n"\
	"[restore_dwell]: dwell time on working channel in partial scan mode\n"\
	"[ch_num]: continuous scan channel number in partial scan mode\n"\
	"[skip_ch]: skip channels in ACS measurement\n"\
	"[pref_ch]: prefer channels in ACS measurement\n"\
	"[sta_num_thr]: the connected station number threshold for periodic ACS check\n"\
	"[check_time]: the time interval between each check in normal case\n"\
	"[ch_util_thr]: If the percentage of busy time in the measurement exceeds the threshold,\n"\
	"				the channel is considered busy in normal case\n"\
	"[ice_ch_util_thr]: If the percentage of busy time in the measurement exceeds the threshold,\n"\
	"				the channel is considered busy in emergency case\n"\
	"[data_rate_wt]: The weight of data rate in calculating scores\n"\
	"[prio_wt]: The weight of channel priority in calculating scores\n"\
	"[tx_power_cons]: whether consider TX power when calculating the data rate\n"\
	"[max_acs_times]: maximum times of execute ACS in every 8 hours\n"\
	"[sw_ch_thr]: If the score of the channel selected by ACS does not exceed the threshold compared\n"\
				"to the score of the original working channel, it will not switch\n"\
	"[obss_wt]: The weight of other BSS in calculating scores\n"\
	"[bgnd_scan]: whether use background scan or not\n"

#define SCAN_OPTIONS "[type=<full|partial|offch|overlap>] [clear] [psc=<1|0>(valid for 6G)] [ssid=<ssid>(valid for full scan)]\n"\
	"[ch=<target_ch>(valid for offch)] [active=<1|0>(valid for offch)] [scan_dwell=<milliseconds>(valid for offch)]\n"\
	"[ch_num=<num>(valid for partial)] [dump] [dump=<bss_start_idx>]\n"

#define MAX_ACS_PARAM_LEN 64
#define MAX_SCAN_PARAM_LEN 64
#define MAX_LEN_OF_SSID 32
#define MAX_SCAN_DUMP_LEN 4096
#define MAX_6G_CH 233


struct autoChSel_option {
	char option_name[MAX_ACS_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char *value);
};

struct scan_option_1 {
	char option_name[MAX_SCAN_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg);
};

struct scan_option_2 {
	char option_name[MAX_SCAN_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char *value);
};

int acs_trigger_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char sel;

	if (!value)
		return -EINVAL;

	switch (*value) {
	case '3':
		sel = 3;
		break;
	case '7':
		sel = 7;
		break;
	case '8':
		sel = 8;
		break;
	case '9':
		sel = 9;
		break;
	default:
		sel = 0;
	}

	if (sel == 0)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_SEL, sel))
		return -EMSGSIZE;

	return 0;
}

int acs_check_time_attr_put(struct nl_msg *msg, char *value)
{
	u32 check_time;

	if (!value)
		return -EINVAL;

	check_time = strtoul(value, NULL, 10);

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_CHECK_TIME, check_time))
		return -EMSGSIZE;

	return 0;
}

int acs_6g_psc_attr_put(struct nl_msg *msg, char *value)
{
	u8 psc_enable;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		psc_enable = 1;
	else if (*value == '0')
		psc_enable = 0;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_6G_PSC, psc_enable))
		return -EMSGSIZE;

	return 0;
}

int acs_partial_scan_attr_put(struct nl_msg *msg, char *value)
{
	u8 partial_scan_enable;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		partial_scan_enable = 1;
	else if (*value == '0')
		partial_scan_enable = 0;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_PARTIAL, partial_scan_enable))
		return -EMSGSIZE;

	return 0;
}

int acs_scan_dwell_attr_put(struct nl_msg *msg, char *value)
{
	u16 scan_dwell;

	if (!value)
		return -EINVAL;
	scan_dwell = strtoul(value, NULL, 10);

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_SCANNING_DWELL, scan_dwell))
		return -EMSGSIZE;

	return 0;
}

int acs_restore_dwell_attr_put(struct nl_msg *msg, char *value)
{
	u16 restore_dwell;

	if (!value)
		return -EINVAL;
	restore_dwell = strtoul(value, NULL, 10);

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_RESTORE_DWELL, restore_dwell))
		return -EMSGSIZE;

	return 0;
}

int acs_ch_num_attr_put(struct nl_msg *msg, char *value)
{
	u8 ch_num;

	if (!value)
		return -EINVAL;
	ch_num = strtoul(value, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_NUM, ch_num))
		return -EMSGSIZE;

	return 0;
}

int acs_skip_ch_attr_put(struct nl_msg *msg, char *value)
{
	struct ch_list_info skip_list = {0};
	unsigned char i;
	char *ptr;
	unsigned char skip_ch;

	if (!value)
		return -EINVAL;

	if (strlen("null") == strlen(value) &&
		!strncmp("null", value, strlen(value))) {
		printf("clear auto ch skip list\n");
		goto  put_skip_list_attr;
	}

	for (i = 0, ptr = strsep(&value, ","); (ptr != NULL) && (i < MAX_NUM_OF_CHANNELS);
		ptr = strsep(&value, ","), i++) {
		skip_ch = strtoul(ptr, NULL, 10);
		if (skip_ch == 0 || skip_ch > MAX_6G_CH) {
			printf("invalid skip_ch %d, return\n", skip_ch);
			return -EINVAL;
		}
		skip_list.ch_list[i] = skip_ch;
	}

	if (i == 0) {
		printf("invalid skip_ch list, return\n");
		return -EINVAL;
	}

	skip_list.num_of_ch = (i > MAX_NUM_OF_CHANNELS) ? MAX_NUM_OF_CHANNELS : i;

put_skip_list_attr:
	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_SKIP_LIST, sizeof(struct ch_list_info), &skip_list))
		return -EMSGSIZE;

	return 0;
}

int acs_pref_ch_attr_put(struct nl_msg *msg, char *value)
{
	struct ch_list_info pref_list = {0};
	unsigned char i;
	char *ptr;
	unsigned char pref_ch;

	if (!value)
		return -EINVAL;

	for (i = 0, ptr = strsep(&value, ","); (ptr != NULL) && (i < MAX_NUM_OF_CHANNELS);
		ptr = strsep(&value, ","), i++) {
		pref_ch = strtoul(ptr, NULL, 10);
		if (pref_ch == 0 || pref_ch > MAX_6G_CH) {
			printf("invalid skip_ch %d, return\n", pref_ch);
			return -EINVAL;
		}
		pref_list.ch_list[i] = pref_ch;
	}

	if (i == 0) {
		printf("invalid skip_ch list, return\n");
		return -EINVAL;
	}

	pref_list.num_of_ch = (i > MAX_NUM_OF_CHANNELS) ? MAX_NUM_OF_CHANNELS : i;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_PREFER_LIST, sizeof(struct ch_list_info), &pref_list))
		return -EMSGSIZE;

	return 0;
}

int acs_sta_num_thr_attr_put(struct nl_msg *msg, char *value)
{
	u16 sta_num_thr;

	if (!value)
		return -EINVAL;
	sta_num_thr = strtoul(value, NULL, 10);

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_STA_NUM_THR, sta_num_thr))
		return -EMSGSIZE;

	return 0;
}

int acs_data_rate_wt_attr_put(struct nl_msg *msg, char *value)
{
	u16 data_rate_wt;

	if (!value)
		return -EINVAL;
	data_rate_wt = strtoul(value, NULL, 10);

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_DATA_RATE_WT, data_rate_wt))
		return -EMSGSIZE;

	return 0;
}

int acs_prio_wt_attr_put(struct nl_msg *msg, char *value)
{
	u16 prio_wt;

	if (!value)
		return -EINVAL;
	prio_wt = strtoul(value, NULL, 10);

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_PRIO_WT, prio_wt))
		return -EMSGSIZE;

	return 0;
}

int acs_tx_power_cons_attr_put(struct nl_msg *msg, char *value)
{
	u8 tx_power_cons;

	if (!value)
		return -EINVAL;
	tx_power_cons = strtoul(value, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_TX_POWER_CONS, tx_power_cons))
		return -EMSGSIZE;

	return 0;
}

int acs_ch_util_thr_attr_put(struct nl_msg *msg, char *value)
{
	u8 ch_util_thr;

	if (!value)
		return -EINVAL;
	ch_util_thr = strtoul(value, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_CH_UTIL_THR, ch_util_thr))
		return -EMSGSIZE;

	return 0;
}

int acs_ice_ch_util_thr_attr_put(struct nl_msg *msg, char *value)
{
	u8 ice_ch_util_thr;

	if (!value)
		return -EINVAL;
	ice_ch_util_thr = strtoul(value, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_ICE_CH_UTIL_THR, ice_ch_util_thr))
		return -EMSGSIZE;

	return 0;
}

int acs_max_acs_times_attr_put(struct nl_msg *msg, char *value)
{
	u8 max_acs_times;

	if (!value)
		return -EINVAL;
	max_acs_times = strtoul(value, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_MAX_ACS_TIMES, max_acs_times))
		return -EMSGSIZE;

	return 0;
}

int sw_ch_thr_attr_put(struct nl_msg *msg, char *value)
{
	u8 sw_ch_thr;

	if (!value)
		return -EINVAL;
	sw_ch_thr = strtoul(value, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_SWITCH_THR, sw_ch_thr))
		return -EMSGSIZE;

	return 0;
}

int acs_obss_wt_attr_put(struct nl_msg *msg, char *value)
{
	u16 obss_wt;

	if (!value)
		return -EINVAL;
	obss_wt = strtoul(value, NULL, 10);

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_OBSS_WT, obss_wt))
		return -EMSGSIZE;

	return 0;
}

int acs_bgnd_scan_attr_put(struct nl_msg *msg, char *value)
{
	u8 bgnd_scan;

	if (!value)
		return -EINVAL;
	bgnd_scan = strtoul(value, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AUTO_CH_BGND_SCAN, bgnd_scan))
		return -EMSGSIZE;

	return 0;
}

struct autoChSel_option acs_opt[] = {
	{"trigger", acs_trigger_attr_put},
	{"check_time", acs_check_time_attr_put},
	{"psc", acs_6g_psc_attr_put},
	{"partial_scan", acs_partial_scan_attr_put},
	{"scan_dwell", acs_scan_dwell_attr_put},
	{"restore_dwell", acs_restore_dwell_attr_put},
	{"ch_num", acs_ch_num_attr_put},
	{"skip_ch", acs_skip_ch_attr_put},
	{"pref_ch", acs_pref_ch_attr_put},
	{"sta_num_thr", acs_sta_num_thr_attr_put},
	{"data_rate_wt", acs_data_rate_wt_attr_put},
	{"prio_wt", acs_prio_wt_attr_put},
	{"tx_power_cons", acs_tx_power_cons_attr_put},
	{"ch_util_thr", acs_ch_util_thr_attr_put},
	{"ice_ch_util_thr", acs_ice_ch_util_thr_attr_put},
	{"max_acs_times", acs_max_acs_times_attr_put},
	{"sw_ch_thr", sw_ch_thr_attr_put},
	{"obss_wt", acs_obss_wt_attr_put},
	{"bgnd_scan", acs_bgnd_scan_attr_put},

};

struct scan_type_option  {
	char type_name[MAX_SCAN_PARAM_LEN];
	enum mtk_vendor_attr_scantype type;
};

struct scan_type_option type_opt[] = {
	{"full", NL80211_FULL_SCAN},
	{"partial", NL80211_PARTIAL_SCAN},
	{"offch", NL80211_OFF_CH_SCAN},
	{"overlap", NL80211_2040_OVERLAP_SCAN},
};

int scan_type_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char i;

	if (!value)
		return -EINVAL;

	for (i = 0; i < (sizeof(type_opt)/sizeof(type_opt[0])); i++) {
		if (strlen(type_opt[i].type_name) == strlen(value) &&
			!strncmp(type_opt[i].type_name, value, strlen(value))) {
			if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_SCAN_TYPE, type_opt[i].type))
				return -EMSGSIZE;
			else
				return 0;
		}
	}

	if (i == sizeof(type_opt)/sizeof(type_opt[0]))
		return -EINVAL;

	return 0;
}

int scan_clear_attr_put(struct nl_msg *msg)
{
	if (nla_put_flag(msg, MTK_NL80211_VENDOR_ATTR_SCAN_CLEAR))
		return -EMSGSIZE;

	return 0;
}

int scan_ssid_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char len;

	len = strlen(value);

	if (len > MAX_LEN_OF_SSID)
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_SCAN_SSID, len, value))
		return -EMSGSIZE;

	return 0;
}

int partial_scan_ch_num_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char num_of_ch;

	if (!value)
		return -EINVAL;

	num_of_ch = strtoul(value, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_PARTIAL_SCAN_NUM_OF_CH, num_of_ch))
		return -EMSGSIZE;

	return 0;
}

int offch_scan_target_ch_attr_put(struct nl_msg *msg, char *value)
{
	u32 ch;

	if (!value)
		return -EINVAL;

	ch = strtoul(value, NULL, 10);

	if (ch == 0)
		return -EINVAL;

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_TARGET_CH, ch))
		return -EMSGSIZE;

	return 0;
}

int offch_scan_active_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char active;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		active = 1;
	else if (*value == '0')
		active = 0;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_ACTIVE, active))
		return -EMSGSIZE;

	return 0;
}

int offch_scan_duration_attr_put(struct nl_msg *msg, char *value)
{
	u32 duration;

	if (!value)
		return -EINVAL;

	duration = strtoul(value, NULL, 10);

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_OFFCH_SCAN_DURATION, duration))
		return -EMSGSIZE;

	return 0;
}

int scan_dump_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_SCAN_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *show_str = NULL;
	int err = 0;
	u32 scan_result_len = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_SCAN_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT]) {
			scan_result_len = nla_len(vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT]);
			show_str = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_SCAN_RESULT]);
			if (scan_result_len > MAX_SCAN_DUMP_LEN || scan_result_len <= 0) {
				printf("the scan result len is invalid !!!\n");
				return -EINVAL;
			} else if (*(show_str + scan_result_len - 1) != '\0') {
				printf("the result string is not ended with right terminator, handle it!!!\n");
				*(show_str + scan_result_len - 1) = '\0';
			}
			printf("%s\n", show_str);
		} else
			printf("no scan result attr\n");
	} else
		printf("no any scan result from driver\n");

	return 0;
}

int scan_dump_attr_put(struct nl_msg *msg)
{
	u32 bss_start_idx = 0;

	register_handler(scan_dump_callback, NULL);
	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_SCAN_DUMP_BSS_START_INDEX, bss_start_idx))
		return -EMSGSIZE;

	return 0;
}

int scan_dump_with_bss_idx_attr_put(struct nl_msg *msg, char *value)
{
	u32 bss_start_idx;

	if (!value)
		bss_start_idx = 0;
	else
		bss_start_idx = strtoul(value, NULL, 10);

	register_handler(scan_dump_callback, NULL);
	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_SCAN_DUMP_BSS_START_INDEX, bss_start_idx))
		return -EMSGSIZE;

	return 0;
}

int scan_6G_psc_attr_put(struct nl_msg *msg, char *value)
{
	u8 psc_en;

	if (!value)
		return -EINVAL;

	if (*value == '1')
		psc_en = 1;
	else if (*value == '0')
		psc_en = 0;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_6G_PSC_SCAN_EN, psc_en))
		return -EMSGSIZE;

	return 0;
}

/*scan_opt1, defines the scan commands that trigger without parameters
**example: scan clear
*/
struct scan_option_1 scan_opt1[] = {
	{"clear", scan_clear_attr_put},
	{"dump", scan_dump_attr_put},
};

/*scan_opt2, defines the scan commands that trigger with parameters
**example: scan type=full
*/
struct scan_option_2 scan_opt2[] = {
	{"type", scan_type_attr_put},
	{"ssid", scan_ssid_attr_put},
	{"ch_num", partial_scan_ch_num_attr_put},
	{"ch", offch_scan_target_ch_attr_put},
	{"active", offch_scan_active_attr_put},
	{"scan_dwell", offch_scan_duration_attr_put},
	{"dump", scan_dump_with_bss_idx_attr_put},
	{"psc", scan_6G_psc_attr_put},
};

int handle_auto_ch_sel_set(struct nl_msg *msg, int argc,
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

		for (j = 0; j < (sizeof(acs_opt) / sizeof(acs_opt[0])); j++) {
			if (strlen(acs_opt[j].option_name) == strlen(param_str) &&
				!strncmp(acs_opt[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(acs_opt) / sizeof(acs_opt[0]))) {
			if (acs_opt[j].attr_put(msg, val_str) < 0)
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

int handle_scan_set(struct nl_msg *msg, int argc,
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

		/*scan_opt1 parse*/
		for (j = 0; j < (sizeof(scan_opt1) / sizeof(scan_opt1[0])); j++) {
			if (strlen(scan_opt1[j].option_name) == strlen(param_str) &&
				!strncmp(scan_opt1[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(scan_opt1) / sizeof(scan_opt1[0]))) {
			if (scan_opt1[j].attr_put(msg) < 0)
				printf("param_str %s att_put fail\n", param_str);
			else
				invalide = 1;
			continue;
		}

		/*scan_opt2 parse*/
		val_str = strchr(ptr, '=');

		if (!val_str)
			continue;

		*val_str++ = 0;

		for (j = 0; j < (sizeof(scan_opt2) / sizeof(scan_opt2[0])); j++) {
			if (strlen(scan_opt2[j].option_name) == strlen(param_str) &&
				!strncmp(scan_opt2[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(scan_opt2) / sizeof(scan_opt2[0]))) {
			if (scan_opt2[j].attr_put(msg, val_str) < 0)
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

TOPLEVEL(acs, ACS_OPTIONS, MTK_NL80211_VENDOR_SUBCMD_SET_AUTO_CH_SEL, 0, CIB_NETDEV, handle_auto_ch_sel_set,
	ACS_HELP);

TOPLEVEL(scan, SCAN_OPTIONS, MTK_NL80211_VENDOR_SUBCMD_SET_SCAN, 0, CIB_NETDEV, handle_scan_set,
	"trigger scan operation\n");


/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

#define WMM_NUM_OF_AC     4	/* AC0, AC1, AC2, and AC3 */

int handle_wmm_ap_aifsn(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char aifsn_value[WMM_NUM_OF_AC] = {0};
	unsigned int tmp[WMM_NUM_OF_AC] = {0};
	char *token;
	char *endptr;
	int i = 0;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	token = strtok(val_str, ":");
	while (token != NULL && i < WMM_NUM_OF_AC) {
		tmp[i] = strtol(token, &endptr, 10);
		if (*endptr != '\0') {
			return -EINVAL;
		}
		i++;
		token = strtok(NULL, ":");
	}

	if (i != WMM_NUM_OF_AC)
		return -EINVAL;

	aifsn_value[0] = (unsigned char)tmp[0];
	aifsn_value[1] = (unsigned char)tmp[1];
	aifsn_value[2] = (unsigned char)tmp[2];
	aifsn_value[3] = (unsigned char)tmp[3];

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_WMM_AP_AIFSN_INFO, sizeof(aifsn_value), aifsn_value))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ap_aifsn,
	"<be:bk:vi:vo>",
	MTK_NL80211_VENDOR_SUBCMD_SET_WMM, 0, CIB_NETDEV, handle_wmm_ap_aifsn,
	"This command is used to configure ap aifsn");

int handle_wmm_ap_cwmin(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char cwmin_value[WMM_NUM_OF_AC] = {0};
	unsigned int tmp[WMM_NUM_OF_AC] = {0};
	char *token;
	char *endptr;
	int i = 0;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	token = strtok(val_str, ":");
	while (token != NULL && i < WMM_NUM_OF_AC) {
		tmp[i] = strtol(token, &endptr, 10);
		if (*endptr != '\0') {  // Check for non-numeric characters
			return -EINVAL;
		}
		token = strtok(NULL, ":");
		i++;
	}

	if (i != WMM_NUM_OF_AC) {
		return -EINVAL;
	}

	cwmin_value[0] = (unsigned char)tmp[0];
	cwmin_value[1] = (unsigned char)tmp[1];
	cwmin_value[2] = (unsigned char)tmp[2];
	cwmin_value[3] = (unsigned char)tmp[3];

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMIN_INFO, sizeof(cwmin_value), cwmin_value))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ap_cwmin,
	"<be:bk:vi:vo>",
	MTK_NL80211_VENDOR_SUBCMD_SET_WMM, 0, CIB_NETDEV, handle_wmm_ap_cwmin,
	"This command is used to configure ap cwmin");

int handle_wmm_ap_cwmax(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char cwmax_value[WMM_NUM_OF_AC] = {0};
	unsigned int tmp[WMM_NUM_OF_AC] = {0};
	char *token;
	char *endptr;
	int i = 0;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	token = strtok(val_str, ":");
	while (token != NULL && i < WMM_NUM_OF_AC) {
		tmp[i] = strtol(token, &endptr, 10);
		if (*endptr != '\0') {  // Check for non-numeric characters
			return -EINVAL;
		}
		token = strtok(NULL, ":");
		i++;
	}

	if (i != WMM_NUM_OF_AC)
		return -EINVAL;

	cwmax_value[0] = (unsigned char)tmp[0];
	cwmax_value[1] = (unsigned char)tmp[1];
	cwmax_value[2] = (unsigned char)tmp[2];
	cwmax_value[3] = (unsigned char)tmp[3];

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_WMM_AP_CWMAX_INFO, sizeof(cwmax_value), cwmax_value))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ap_cwmax,
	"<be:bk:vi:vo>",
	MTK_NL80211_VENDOR_SUBCMD_SET_WMM, 0, CIB_NETDEV, handle_wmm_ap_cwmax,
	"This command is used to configure ap cwmax");

int handle_wmm_ap_txop(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char txop_value[WMM_NUM_OF_AC] = {0};
	unsigned int tmp[WMM_NUM_OF_AC] = {0};
	char *token;
	char *endptr;
	int i = 0;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	token = strtok(val_str, ":");

	while (token != NULL && i < WMM_NUM_OF_AC) {
		tmp[i] = strtol(token, &endptr, 10);

		if (*endptr != '\0') {
			return -EINVAL; // Invalid integer conversion
		}

		token = strtok(NULL, ":");
		i++;
	}

	if (i != WMM_NUM_OF_AC) {
		return -EINVAL;
	}

	txop_value[0] = (unsigned char)tmp[0];
	txop_value[1] = (unsigned char)tmp[1];
	txop_value[2] = (unsigned char)tmp[2];
	txop_value[3] = (unsigned char)tmp[3];

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_WMM_AP_TXOP_INFO, sizeof(txop_value), txop_value))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ap_txop,
	"<be:bk:vi:vo>",
	MTK_NL80211_VENDOR_SUBCMD_SET_WMM, 0, CIB_NETDEV, handle_wmm_ap_txop,
	"This command is used to configure ap txop");

int get_wmm_cap_status_callback(struct nl_msg *msg, void *data)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_WMM_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	u8 igmp_status;
	int err = 0;
	//u16 acl_result_len = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_WMM_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_CAP_INFO]) {
			//acl_result_len = nla_len(vndr_tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE]);
			igmp_status = nla_get_u8(vndr_tb[MTK_NL80211_VENDOR_ATTR_WMM_AP_CAP_INFO]);
			if (igmp_status == 0) {
				printf("disabled\n");
			} else {
				printf("enabled\n");
			}
		}
	}

	return 0;
}

int handle_wmm_ap_cap(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char cap;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		cap = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		cap = 1;
	else if (strncmp("s", val_str ,strlen(val_str)) == 0) {
		register_handler(get_wmm_cap_status_callback, NULL);
		cap = 0xf;
	} else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_WMM_AP_CAP_INFO, cap))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, wmm_cap,
	"<be:bk:vi:vo>",
	MTK_NL80211_VENDOR_SUBCMD_SET_WMM, 0, CIB_NETDEV, handle_wmm_ap_cap,
	"This command is used to configure ap txop");

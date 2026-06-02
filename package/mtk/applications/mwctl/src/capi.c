/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

static int ap_rfeature_set_attr(struct nl_msg *msg, int argc, char **argv)
{
	char *val, *data;
	int ret = 0;
	u32 tmp = 0;

	val = strchr(argv[0], '=');
	if (!val)
		return -EINVAL;

	*(val++) = 0;

	errno = 0;
	tmp = strtoul(val, NULL, 0);
	if (errno == ERANGE)
		return -EINVAL;

	if (!strncmp(argv[0], "he_gi", 5)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_GI, (u8)tmp);
	} else if (!strncmp(argv[0], "he_ltf", 6)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_LTF, (u8)tmp);
	} else if (!strncmp(argv[0], "trig_type", 9)) {
		data = strdup(val);
		ret = nla_put_string(msg, MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_TRIG_TYPE, data);
		free(data);
	} else if (!strncmp(argv[0], "ack_policy", 10)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_ACK_PLCY, (u8)tmp);
	} else if (!strncmp(argv[0], "ppdu_type", 9)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_PPDU_TYPE, (u8)tmp);
	} else if (!strncmp(argv[0], "he_txop_rts_thld", 16)) {
		ret = nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_HE_TXOP_RTS_THLD, (u16)tmp);
	} else if (!strncmp(argv[0], "extra_ltf_disable", 17)) {
		ret = nla_put_u8(
			msg, MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_EXTRA_LTF_DISABLE, (u8)tmp);
	} else if (!strncmp(argv[0], "extra_ltf", 9)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_RFEATURE_EXTRA_LTF, (u8)tmp);
	}

	if (ret != 0)
		return -EMSGSIZE;

	return 0;
}

int ap_rfeature_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	int ret = 0;

	if (argc < 1)
		return 1;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	ap_rfeature_set_attr(msg, argc, argv);

	nla_nest_end(msg, data);

	return ret;
}

static int ap_wireless_set_attr(struct nl_msg *msg, int argc, char **argv)
{
	char *val, *data;
	u32 tmp = 0;
	int ret = 0;

	val = strchr(argv[0], '=');
	if (!val)
		return -EINVAL;

	*(val++) = 0;

	errno = 0;
	tmp = strtoul(val, NULL, 0);
	if (errno == ERANGE)
		return -EINVAL;
	if (!strncmp(argv[0], "fixed_mcs", 9)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_MCS, (u8)tmp);
	} else if (!strncmp(argv[0], "ofdma", 5)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_FIXED_OFDMA, (u8)tmp);
	} else if (!strncmp(argv[0], "ppdu_type", 9)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_PPDU_TX_TYPE, (u8)tmp);
	} else if (!strncmp(argv[0], "nusers_ofdma", 12)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_NUSERS_OFDMA, (u8)tmp);
	} else if (!strncmp(argv[0], "add_ba_req_bufsize", 18)) {
		ret = nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_BA_BUFFER_SIZE, (u8)tmp);
	} else if (!strncmp(argv[0], "mimo", 4)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MIMO, (u8)tmp);
	} else if (!strncmp(argv[0], "ampdu", 5)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMPDU, (u8)tmp);
	} else if (!strncmp(argv[0], "amsdu", 5)) {
		ret = nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_AMSDU, (u8)tmp);
	} else if (!strncmp(argv[0], "cert", 4)) {
		data = strdup(val);
		ret = nla_put_string(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_CERT, data);
		free(data);
	} else if (!strncmp(argv[0], "he_txop_rts_thld", 16)) {
		ret = nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_HE_TXOP_RTS_THLD, (u16)tmp);
	}

	if (ret != 0)
		return -EMSGSIZE;

	return 0;
}

int ap_wireless_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	int ret = 0;

	if (argc < 1)
		return 1;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	ap_wireless_set_attr(msg, argc, argv);

	nla_nest_end(msg, data);

	return ret;
}

DECLARE_SECTION(set);

COMMAND(set, ap_rfeatures,
	"[he_gi=<val>(val:0-0.8, 1-1.6, 2-3.2)]|\n"
	"[he_ltf=<val>(val:0-3.2, 1-6.4, 2-12.8)]|\n"
	"[trig_type=<val>(val:0-disable, 1-enable), <val>(val:0-7)]|\n"
	"[ack_policy=<val>(val:0-4)]"
	"[ppdu_type=<val>(val:0-SU, 1-MU, 2-ER, 3-TB , 4-LEGACY)]|\n"
	"[extra_ltf_disable=<val>(val:0-disable, 1-enable)|\n"
	"[extra_ltf=<val>(val:4 or 8]|\n",
	MTK_NL80211_VENDOR_SUBCMD_AP_RFEATURE, 0, CIB_NETDEV, ap_rfeature_set,
	"This command is used to set radio information while Cert");

DECLARE_SECTION(set);

COMMAND(set, ap_wireless,
	"[fixed_mcs=<val>]|\n"
	"[ofdma=<val>(val:0-disable, 1-DL, 2-UL)]|\n"
	"[nusers_ofdma=<val>]|\n"
	"[ppdu_type=<val>(val:0-SU, 1-MU, 2-ER, 3-TB , 4-LEGACY)]|\n"
	"[add_ba_req_bufsize=<val>]|\n"
	"[mimo=<val>(val:0-DL, 1-UL)]|\n"
	"[ampdu=<val>(val:0-disable, 1-enable)]|\n"
	"[amsdu=<val>(val:0-disable, 1-enable)]|\n"
	"[cert=<driver val>(val:0-disable, 1-enable), <fw val>(val:0-disable, 1-enable)]",
	MTK_NL80211_VENDOR_SUBCMD_AP_WIRELESS, 0, CIB_NETDEV, ap_wireless_set,
	"This command is used to set wireless information while Cert");

/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

int handle_cosr_set_rssi_th(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *data_str;
	char rssi_th;

	if (argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	data_str = argv[0];
	rssi_th = (char)strtol(data_str, NULL, 10);

	if (nla_put_s8(msg, MTK_NL80211_VENDOR_ATTR_COSR_SET_RSSI_TH, rssi_th))
		return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}


COMMAND(set, cosr_rssi_th,
		"cosr_rssi_th=<val:-128 ~127>",
		MTK_NL80211_VENDOR_SUBCMD_SET_COSR_INFO,
		0, CIB_NETDEV, handle_cosr_set_rssi_th,
		"Set Cosr ap beacon report rssi threshold.\n");


/* Copyright (C) 2022 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

#define MBO_NPC_MAX_LEN     5	/* req_type, ch, pref, reason_code, and reg_class */

int handle_mbo_npc(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char npc_value[MBO_NPC_MAX_LEN] = {0};
	unsigned int tmp[MBO_NPC_MAX_LEN] = {0};
	int ret;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	ret = sscanf(val_str, "%d:%d:%d:%d:%d",
			&tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4]);

	if (ret != MBO_NPC_MAX_LEN)
		return -EINVAL;

	npc_value[0] = (unsigned char)tmp[0];
	npc_value[1] = (unsigned char)tmp[1];
	npc_value[2] = (unsigned char)tmp[2];
	npc_value[3] = (unsigned char)tmp[3];
	npc_value[4] = (unsigned char)tmp[4];

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_SET_MBO_NPC, sizeof(npc_value), npc_value))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, mbo_ch_pref,
	"req_type, channel, pref, reason_code, reg_class",
	MTK_NL80211_VENDOR_SUBCMD_SET_MBO, 0, CIB_NETDEV, handle_mbo_npc,
	"This command is used to configure mbo parameters");

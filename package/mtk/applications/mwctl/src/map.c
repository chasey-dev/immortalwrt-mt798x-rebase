/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

int handle_map_bhbss(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char bhbss_enable;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		bhbss_enable = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		bhbss_enable = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_BHBSS, bhbss_enable))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, bhbss,
	"<0 or 1> (0=disable 1=enable)",
	MTK_NL80211_VENDOR_SUBCMD_EASYMESH, 0, CIB_NETDEV, handle_map_bhbss,
	"This command is used to configure map bhbss enable/disable");

int handle_map_fhbss(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char fhbss_enable;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		fhbss_enable = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		fhbss_enable = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_EASYMESH_SET_FHBSS, fhbss_enable))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, fhbss,
	"<0 or 1> (0=disable 1=enable)",
	MTK_NL80211_VENDOR_SUBCMD_EASYMESH, 0, CIB_NETDEV, handle_map_fhbss,
	"This command is used to configure map fhbss enable/disable");

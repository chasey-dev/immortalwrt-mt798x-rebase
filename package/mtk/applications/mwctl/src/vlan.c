/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

int handle_vlan_en(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char vlan_en;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	if (strncmp("0", val_str, strlen(val_str)) == 0)
		vlan_en = 0;
	else if (strncmp("1", val_str, strlen(val_str)) == 0)
		vlan_en = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_VLAN_EN_INFO, vlan_en))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, vlan_en,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_VLAN, 0, CIB_NETDEV, handle_vlan_en,
	"This command is used to configure vlan enable");

#define MAX_VID 0x0FFF
int handle_vlan_id(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned short vlan_id;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	vlan_id = (unsigned short)strtoul(val_str, NULL, 10);
	if (vlan_id > MAX_VID)
		return -EINVAL;

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_VLAN_ID_INFO, vlan_id))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, vlan_id,
	"<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_VLAN, 0, CIB_NETDEV, handle_vlan_id,
	"This command is used to configure vlan id");

#define MAX_PCP 7
int handle_vlan_priority(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char vlan_priority;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	vlan_priority = (unsigned char)strtoul(val_str, NULL, 10);
	if (vlan_priority > MAX_PCP)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_VLAN_PRIORITY_INFO, vlan_priority))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, vlan_priority,
	"<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_VLAN, 0, CIB_NETDEV, handle_vlan_priority,
	"This command is used to configure vlan priority");

int handle_vlan_tag(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char vlan_tag;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	if (strncmp("0", val_str, strlen(val_str)) == 0)
		vlan_tag = 0;
	else if (strncmp("1", val_str, strlen(val_str)) == 0)
		vlan_tag = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_VLAN_TAG_INFO, vlan_tag))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, vlan_tag,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_VLAN, 0, CIB_NETDEV, handle_vlan_tag,
	"This command is used to configure vlan tag");

#define TX_VLAN 0
#define RX_VLAN 1
#define VLAN_TX_POLICY_NUM 5
#define VLAN_RX_POLICY_NUM 5
int handle_vlan_policy(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	struct vlan_policy_param param;
	int ret;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	ret = sscanf(val_str, "%d:%d", &param.direction, &param.policy);

	if (ret != 2)
		return -EINVAL;
	if ((param.direction > RX_VLAN)
		|| (param.direction == TX_VLAN && param.policy >= VLAN_TX_POLICY_NUM)
		|| (param.direction == RX_VLAN && param.policy >= VLAN_RX_POLICY_NUM))
		return -EINVAL;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_VLAN_POLICY_INFO, sizeof(param), &param))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, vlan_policy,
	"<0/1:value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_VLAN, 0, CIB_NETDEV, handle_vlan_policy,
	"This command is used to configure vlan policy");

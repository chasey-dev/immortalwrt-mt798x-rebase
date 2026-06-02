/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

#define MULTICAST_SNOOPING_OPTIONS \
	"[enable=<0 or 1>][unknown_policy=<flood|drop>]"\
	"[add=<group id as IP or MAC addr>[-<member MAC addr>]]"\
	"[deny=<group IP addr>][floodingcidr=<option>-<IP addr>/<prefix>]"

#define MAX_MULTICAST_SNOOPING_PARAM_LEN 32
struct multicast_snooping_option {
	char option_name[MAX_MULTICAST_SNOOPING_PARAM_LEN];
	int (*attr_put)(struct nl_msg *msg, char *value);
};

int get_igmp_status_callback(struct nl_msg *msg, void *data)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_MCAST_SNOOP_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	u8 igmp_status;
	int err = 0;
	//u16 acl_result_len = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_MCAST_SNOOP_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE]) {
			igmp_status = nla_get_u8(vndr_tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE]);
			if (igmp_status == 0) {
				printf("disabled\n");
			} else {
				printf("enabled\n");
			}
		}
	}

	return 0;
}


static int mcast_snoop_enable_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char mcsnoop_enable;

	if (!value)
		return -EINVAL;

	if (*value == '0')
		mcsnoop_enable = 0;
	else if (*value == '1')
		mcsnoop_enable = 1;
	else if (*value == 's') {
		register_handler(get_igmp_status_callback, NULL);
		mcsnoop_enable = 0xf;
	} else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE, mcsnoop_enable))
		return -EMSGSIZE;

	return 0;
}

static int mcast_snoop_unknown_policy_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char mcsnoop_plcy;

	if (!value)
		return -EINVAL;

	if (strlen(value) == strlen("drop") && !strncmp(value, "drop", strlen("drop")))
		mcsnoop_plcy = 0;
	else if (strlen(value) == strlen("flood") && !strncmp(value, "flood", strlen("flood")))
		mcsnoop_plcy = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_UNKNOWN_PLCY, mcsnoop_plcy))
		return -EMSGSIZE;

	return 0;
}

static int mcast_snoop_entry_add_attr_put(struct nl_msg *msg, char *value)
{
	int len = strlen(value);

	if (len >= 60)
		return -EINVAL;

	if (nla_put_string(msg, MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_ADD, value))
		return -EMSGSIZE;

	return 0;
}

static int mcast_snoop_entry_del_attr_put(struct nl_msg *msg, char *value)
{
	int len = strlen(value);

	if (len >= 60)
		return -EINVAL;

	if (nla_put_string(msg, MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENTRY_DEL, value))
		return -EMSGSIZE;

	return 0;
}

static int mcast_snoop_deny_list_attr_put(struct nl_msg *msg, char *value)
{
	int len = strlen(value);

	if (len > 64)
		return -EINVAL;

	if (nla_put_string(msg, MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_DENY_LIST, value))
		return -EMSGSIZE;

	return 0;
}

static int mcast_snoop_floodingcidr_attr_put(struct nl_msg *msg, char *value)
{
	int len = strlen(value);

	if (len > 21)
		return -EINVAL;

	if (nla_put_string(msg, MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_FLOODINGCIDR, value))
		return -EMSGSIZE;

	return 0;
}

struct multicast_snooping_option mcsnoop_ops[] = {
	{"enable",			mcast_snoop_enable_attr_put},
	{"policy",			mcast_snoop_unknown_policy_attr_put},
	{"add",				mcast_snoop_entry_add_attr_put},
	{"del",				mcast_snoop_entry_del_attr_put},
	{"deny",			mcast_snoop_deny_list_attr_put},
	{"floodingcidr",		mcast_snoop_floodingcidr_attr_put},
};

int handle_multicast_snooping_set(struct nl_msg *msg,
				  int argc,
				  char **argv,
				  void *ctx)
{
	void *data;
	char *ptr, *param_str, *val_str, invalid = 0;
	int i, j, ret = 0;

	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	/*handler link_id*/
	ret = add_msg_band_link_id(msg, get_band_link_id(), MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_BAND_LINK_ID);
	if (ret < 0)
		return ret;

	for (i = 0; i < argc; i++) {
		ptr = argv[i];
		param_str = ptr;
		val_str = strchr(ptr, '=');

		if (!val_str)
			continue;

		*val_str++ = 0;

		for (j = 0; j < (sizeof(mcsnoop_ops) / sizeof(mcsnoop_ops[0])); j++) {
			if (strlen(mcsnoop_ops[j].option_name) == strlen(param_str) &&
				!strncmp(mcsnoop_ops[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != sizeof(mcsnoop_ops) / sizeof(mcsnoop_ops[0])) {
			if (mcsnoop_ops[j].attr_put(msg, val_str) < 0)
				printf("invalid argument %s=%s, ignore it\n", param_str, val_str);
			else
				invalid = 1;
		}
	}

	nla_nest_end(msg, data);

	if(!invalid)
		return -EINVAL;

	return 0;
}

COMMAND(set, multicast_snooping, MULTICAST_SNOOPING_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_MULTICAST_SNOOPING, 0,
	CIB_NETDEV, handle_multicast_snooping_set,
	"Set the multicast snooping related configurations to specific bss\n");


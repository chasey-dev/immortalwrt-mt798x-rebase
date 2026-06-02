/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

#define ACL_HELP "plicy (0:disable 1:black list 2:white list)\n"\
	"add=xx:xx:xx:xx:xx:xx;xx:xx:xx:xx:xx:xx;...\n"\
	"add=xx:xx:xx:xx:xx:xx-y, y is valid MAC length, range in '1~9' or 'a~c'\n"\
	"del=xx:xx:xx:xx:xx:xx;xx:xx:xx:xx:xx:xx;...\n"\
	"del=xx:xx:xx:xx:xx:xx-y, y is valid MAC length, range in '1~9' or 'a~c'\n"\
	"show_all:show all stas in list\n"\
	"clear_all:clear all stas in list\n"

#define ACL_OPTIONS "[policy=<0|1|2>]\n"\
	"[add=<mac_addr>,<mac_addr>,...]\n"\
	"[add=<mac_addr>-<y>, <y> is valid MAC length, range in '1~9' or 'a~c']\n"\
	"[del=<mac_addr>,<mac_addr>,...]\n"\
	"[del=<mac_addr>-<y>, <y> is valid MAC length, range in '1~9' or 'a~c']\n"\
	"[show_all]\n"\
	"[clear_all]\n"

#define MAX_ACL_PARAM_LEN 128
#define MAX_ACL_DUMP_LEN 4096

int acl_policy_attr_put(struct nl_msg *msg, char *value);
int acl_add_attr_put(struct nl_msg *msg, char *value);
int acl_del_attr_put(struct nl_msg *msg, char *value);
int acl_show_attr_put(struct nl_msg *msg);
int acl_clear_attr_put(struct nl_msg *msg);

struct acl_option1 {
	char option_name[MAX_ACL_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg);
};

struct acl_option2 {
	char option_name[MAX_ACL_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char *value);
};

struct acl_option1 acl_opt1[] = {
	{"show_all", acl_show_attr_put},
	{"clear_all", acl_clear_attr_put},
};

struct acl_option2 acl_opt2[] = {
	{"policy", acl_policy_attr_put},
	{"add", acl_add_attr_put},
	{"del", acl_del_attr_put},
};

struct acl_policy_option  {
	char option_name[MAX_ACL_PARAM_LEN];
	enum mtk_nl80211_vendor_attr_acl_policy mode;
};

struct acl_policy_option policy_opt[] = {
	{"0", MTK_NL80211_VENDOR_ATTR_ACL_DISABLE},
	{"1", MTK_NL80211_VENDOR_ATTR_ACL_ENABLE_WHITE_LIST},
	{"2", MTK_NL80211_VENDOR_ATTR_ACL_ENABLE_BLACK_LIST},
};

int acl_policy_attr_put(struct nl_msg *msg, char *value)
{
	int i;

	for (i = 0; i < (sizeof(policy_opt)/sizeof(policy_opt[0])); i++) {
		if (strlen(policy_opt[i].option_name) == strlen(value) &&
			!strncmp(policy_opt[i].option_name, value, strlen(value))) {
			if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_ACL_POLICY, policy_opt[i].mode))
				return -EMSGSIZE;
		}
	}

	return 0;
}

int acl_add_attr_put(struct nl_msg *msg, char *value)
{
	char *token;
	int i = 0, n = 0, valid = 1;
	u8 *addr_list;
	char *p = value;
	int validMAClength = 0;
	char validMAClengthMode = 0;

	if (!value)
		return -EINVAL;

	if ((p = strchr(p, '-')) != NULL && strlen(value) == 19) {
		validMAClengthMode = 1;
		p++;
		validMAClength = *p;
	}

	if (validMAClengthMode) {
		addr_list = (u8 *)malloc((ETH_ALEN + 1) * sizeof(u8));
	}
	else {
		n = (strlen(value) + 1) / 18;
		addr_list = (u8 *)malloc(n * ETH_ALEN * sizeof(u8));
	}


	if (!addr_list)
		return -EINVAL;

	for (token = strtok(value, ":,-"); token != NULL; token = strtok(NULL, ":,-")) {
		if (validMAClengthMode) {
			if (strlen(token) != 2 || i == ETH_ALEN + 1) {
				if (strlen(token) == 1 && i == ETH_ALEN)
					break;
				valid = 0;
				break;
			}
		}
		else {
			if (strlen(token) != 2 || i == (n * ETH_ALEN)) {
				valid = 0;
				break;
			}
		}
		addr_list[i++] = strtol(token, NULL, 16);
	}

	if (!valid) {
		free(addr_list);
		return -EINVAL;
	}

	if (validMAClengthMode) {
		addr_list[i] = validMAClength;
		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_ACL_ADD_MAC, ETH_ALEN + 1, addr_list)) {
			free(addr_list);
			return -EMSGSIZE;
		}
	}
	else {
		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_ACL_ADD_MAC, n * ETH_ALEN, addr_list)) {
			free(addr_list);
			return -EMSGSIZE;
		}
	}
	free(addr_list);
	return 0;
}

int acl_del_attr_put(struct nl_msg *msg, char *value)
{
	char *token;
	int i = 0, n = 0, valid = 1;
	u8 *addr_list;
	char *p = value;
	int validMAClength = 0;
	char validMAClengthMode = 0;

	if (!value)
		return -EINVAL;

	if ((p = strchr(p, '-')) != NULL && strlen(value) == 19) {
		validMAClengthMode = 1;
		p++;
		validMAClength = *p;
	}

	if (validMAClengthMode) {
		addr_list = (u8 *)malloc((ETH_ALEN + 1) * sizeof(u8));
	}
	else {
		n = (strlen(value) + 1) / 18;
		addr_list = (u8 *)malloc(n * ETH_ALEN * sizeof(u8));
	}


	if (!addr_list)
		return -EINVAL;

	for (token = strtok(value, ":,-"); token != NULL; token = strtok(NULL, ":,-")) {
		if (validMAClengthMode) {
			if (strlen(token) != 2 || i == ETH_ALEN + 1) {
				if (strlen(token) == 1 && i == ETH_ALEN)
					break;
				valid = 0;
				break;
			}
		}
		else {
			if (strlen(token) != 2 || i == (n * ETH_ALEN)) {
				valid = 0;
				break;
			}
		}
		addr_list[i++] = strtol(token, NULL, 16);
	}

	if (!valid) {
		free(addr_list);
		return -EINVAL;
	}

	if (validMAClengthMode) {
		addr_list[i] = validMAClength;
		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_ACL_DEL_MAC, ETH_ALEN + 1, addr_list)) {
			free(addr_list);
			return -EMSGSIZE;
		}
	}
	else {
		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_ACL_DEL_MAC, n * ETH_ALEN, addr_list)) {
			free(addr_list);
			return -EMSGSIZE;
		}
	}
	free(addr_list);
	return 0;
}

int acl_list_dump_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_AP_ACL_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *show_str = NULL;
	int err = 0;
	u16 acl_result_len = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_AP_ACL_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_ACL_LIST_INFO]) {
			acl_result_len = nla_len(vndr_tb[MTK_NL80211_VENDOR_ATTR_ACL_LIST_INFO]);
			show_str = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_ACL_LIST_INFO]);
			if (acl_result_len > MAX_ACL_DUMP_LEN) {
				printf("the scan result len is invalid !!!\n");
				return -EINVAL;
			} else if (*(show_str + acl_result_len - 1) != '\0') {
				printf("the result string is not ended with right terminator, handle it!!!\n");
				*(show_str + acl_result_len - 1) = '\0';
			}
			printf("%s\n", show_str);
		} else
			printf("no acl result attr\n");
	} else
		printf("no any acl result from driver\n");

	return 0;
}

int acl_show_attr_put(struct nl_msg *msg)
{
	register_handler(acl_list_dump_callback, NULL);
	return nla_put_flag(msg, MTK_NL80211_VENDOR_ATTR_ACL_SHOW_ALL);
}

int acl_clear_attr_put(struct nl_msg *msg)
{
	return nla_put_flag(msg, MTK_NL80211_VENDOR_ATTR_ACL_CLEAR_ALL);
}

int handle_acl_set(struct nl_msg *msg, int argc,
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

		/*acl_opt1 parse*/
		for (j = 0; j < (sizeof(acl_opt1) / sizeof(acl_opt1[0])); j++) {
			if (strlen(acl_opt1[j].option_name) == strlen(param_str) &&
				!strncmp(acl_opt1[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(acl_opt1) / sizeof(acl_opt1[0]))) {
			if (acl_opt1[j].attr_put(msg) < 0)
				printf("opt1 param_str %s att_put fail\n", param_str);
			else
				invalide = 1;
			continue;
		}

		/*acl_opt2 parse*/
		val_str = strchr(ptr, '=');

		if (!val_str)
			continue;

		*val_str++ = 0;

		for (j = 0; j < (sizeof(acl_opt2) / sizeof(acl_opt2[0])); j++) {
			if (strlen(acl_opt2[j].option_name) == strlen(param_str) &&
				!strncmp(acl_opt2[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(acl_opt2) / sizeof(acl_opt2[0]))) {
			if (acl_opt2[j].attr_put(msg, val_str) < 0)
				printf("opt2 invalide argument %s=%s, ignore it\n", param_str, val_str);
			else
				invalide = 1;
		}
	}

	nla_nest_end(msg, data);

	if (!invalide)
		return -EINVAL;

	return 0;
}

TOPLEVEL(acl,
	ACL_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_ACL, 0, CIB_NETDEV, handle_acl_set,
	ACL_HELP);

/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

static struct nla_policy
amnt_ctrl_policy[NUM_MTK_VENDOR_ATTRS_AMNT_CTRL] = {
	[MTK_VENDOR_ATTR_AMNT_CTRL_SET] = {.type = NLA_NESTED },
	[MTK_VENDOR_ATTR_AMNT_CTRL_DUMP] = { .type = NLA_NESTED },
};

static struct nla_policy
amnt_dump_policy[NUM_MTK_VENDOR_ATTRS_AMNT_DUMP] = {
	[MTK_VENDOR_ATTR_AMNT_DUMP_INDEX] = {.type = NLA_U8 },
	[MTK_VENDOR_ATTR_AMNT_DUMP_LEN] = { .type = NLA_U8 },
	[MTK_VENDOR_ATTR_AMNT_DUMP_RESULT] = { .type = NLA_NESTED },
};

static int mt76_amnt_set_attr(struct nl_msg *msg, int argc, char **argv)
{
	void *tb1, *tb2;
	u8 a[ETH_ALEN], idx;
	int i = 0, matches;

	errno = 0;
	idx = strtoul(argv[0], NULL, 0);
	if (errno == ERANGE)
		return -EINVAL;
	matches = sscanf(argv[1], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			a, a+1, a+2, a+3, a+4, a+5);

	if (matches != ETH_ALEN)
		return -EINVAL;

	tb1 = nla_nest_start(msg, MTK_VENDOR_ATTR_AMNT_CTRL_SET | NLA_F_NESTED);
	if (!tb1)
		return -ENOMEM;

	if (nla_put_u8(msg, MTK_VENDOR_ATTR_AMNT_SET_INDEX, idx))
		return -EMSGSIZE;

	tb2 = nla_nest_start(msg, MTK_VENDOR_ATTR_AMNT_SET_MACADDR | NLA_F_NESTED);
	if (!tb2) {
		nla_nest_end(msg, tb1);
		return -ENOMEM;
	}

	for (i = 0; i < ETH_ALEN; i++) {
		if (nla_put_u8(msg, i, a[i]))
			return -EMSGSIZE;
	}

	nla_nest_end(msg, tb2);
	nla_nest_end(msg, tb1);

	return 0;
}

int mt76_amnt_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	int ret = 0;

	if (argc < 1)
		return 1;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA | NLA_F_NESTED);
	if (!data)
		return -ENOMEM;

	mt76_amnt_set_attr(msg, argc, argv);

	nla_nest_end(msg, data);

	return ret;
}

static int mt76_amnt_dump_cb(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb1[NUM_MTK_VENDOR_ATTRS_AMNT_CTRL];
	struct nlattr *tb2[NUM_MTK_VENDOR_ATTRS_AMNT_DUMP];
	struct nlattr *attr;
	struct nlattr *data;
	struct nlattr *cur;
	struct amnt_data *res;
	int len = 0, rem;

	attr = unl_find_attr(&unl, msg, NL80211_ATTR_VENDOR_DATA);
	if (!attr) {
		(void)fprintf(stderr, "Testdata attribute not found\n");
		return NL_SKIP;
	}

	if (nla_parse_nested(tb1, MTK_VENDOR_ATTR_AMNT_CTRL_MAX,
		attr, amnt_ctrl_policy))
		return NL_SKIP;

	if (!tb1[MTK_VENDOR_ATTR_AMNT_CTRL_DUMP])
		return NL_SKIP;

	if (nla_parse_nested(tb2, MTK_VENDOR_ATTR_AMNT_DUMP_MAX,
		tb1[MTK_VENDOR_ATTR_AMNT_CTRL_DUMP], amnt_dump_policy))
		return NL_SKIP;

	if (!tb2[MTK_VENDOR_ATTR_AMNT_DUMP_LEN])
		return NL_SKIP;

	len = nla_get_u8(tb2[MTK_VENDOR_ATTR_AMNT_DUMP_LEN]);
	if (!len)
		return 0;

	if (!tb2[MTK_VENDOR_ATTR_AMNT_DUMP_RESULT])
		return NL_SKIP;

	data = tb2[MTK_VENDOR_ATTR_AMNT_DUMP_RESULT];
	nla_for_each_nested(cur,data, rem) {
		res = (struct amnt_data *) nla_data(cur);
		printf("[vendor] amnt_idx: %d, addr=%x:%x:%x:%x:%x:%x, rssi=%d/%d/%d/%d, last_seen=%u\n",
			res->idx,
			res->addr[0], res->addr[1], res->addr[2],
			res->addr[3], res->addr[4], res->addr[5],
			res->rssi[0], res->rssi[1], res->rssi[2],
			res->rssi[3], res->last_seen);
	}
	return 0;
}

int mt76_amnt_dump	(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data, *tb1;
	u8 amnt_idx;

	if (argc < 1)
		return 1;

	register_handler(mt76_amnt_dump_cb, NULL);
	
	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA | NLA_F_NESTED);
	if (!data)
		return -EINVAL;

	tb1 = nla_nest_start(msg, MTK_VENDOR_ATTR_AMNT_CTRL_DUMP | NLA_F_NESTED);
	if (!tb1)
		return -EINVAL;

	errno = 0;
	amnt_idx = strtoul(argv[0], NULL, 0);
	if (errno == ERANGE)
		return -EINVAL;
	if (nla_put_u8(msg, MTK_VENDOR_ATTR_AMNT_DUMP_INDEX, amnt_idx))
		return -EMSGSIZE;

	nla_nest_end(msg, tb1);

	nla_nest_end(msg, data);

	return 0;
}

DECLARE_SECTION(dump);

COMMAND(dump, amnt, "<index> (0x0~0xf or 0xff)",
	MTK_NL80211_VENDOR_SUBCMD_CSI_CTRL, 0, CIB_NETDEV, mt76_amnt_dump, "");

DECLARE_SECTION(set);

COMMAND(set, amnt, "<index>(0x0~0xf) <mac addr>(xx:xx:xx:xx:xx:xx)",
	MTK_NL80211_VENDOR_SUBCMD_AMNT_CTRL, 0, CIB_NETDEV, mt76_amnt_set,
	"");



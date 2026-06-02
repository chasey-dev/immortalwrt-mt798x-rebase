/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

int handle_ieee80211h(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char ieee80211h;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		ieee80211h = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		ieee80211h = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_IEEE80211H_INFO, ieee80211h))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ieee80211h,
	"<0 or 1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO, 0, CIB_PHY, handle_ieee80211h,
	"This command is used to configure ieee802.11h Enable/Disable");

int handle_ackcts_tout_en(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned char ackcts_tout_en;
	void *data;
	char *val_str;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		ackcts_tout_en = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		ackcts_tout_en = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_ACKCTS_TOUT_EN_INFO, ackcts_tout_en))
		return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, ackcts_tout_en,
	"<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO, 0, CIB_PHY, handle_ackcts_tout_en,
	"This command is used to enable ack/cts timeout configure function");

int handle_distance(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int distance;
	void *data;
	char *val_str;
	int ret;
	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%10u", &distance);

	if (ret != 1)
		return -EINVAL;

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_AP_DISTANCE_INFO, distance))
		return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, distance,
	"<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO, 0, CIB_PHY, handle_distance,
	"This command is used to configure ack timeout based on distance between AP and STA(unit: meter)");

int handle_cck_ack_tout(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int cck_ack_tout;
	void *data;
	char *val_str;
	int ret;
	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%10u", &cck_ack_tout);

	if (ret != 1)
		return -EINVAL;

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_AP_CCK_ACK_TOUT_INFO, cck_ack_tout))
		return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, cck_ack_tout,
	"<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO, 0, CIB_PHY, handle_cck_ack_tout,
	"This command is used to configure cck ack timeout value");

int handle_ofdm_ack_tout(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int ofdm_ack_tout;
	void *data;
	char *val_str;
	int ret;
	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%10u", &ofdm_ack_tout);

	if (ret != 1)
		return -EINVAL;

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_AP_OFDM_ACK_TOUT_INFO, ofdm_ack_tout))
		return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, ofdm_ack_tout,
	"<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO, 0, CIB_PHY, handle_ofdm_ack_tout,
	"This command is used to configure ofdm ack timeout value");

int handle_ofdma_ack_tout(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	unsigned int ofdma_ack_tout;
	void *data;
	char *val_str;
	int ret;
	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%10u", &ofdma_ack_tout);

	if (ret != 1)
		return -EINVAL;

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_AP_OFDMA_ACK_TOUT_INFO, ofdma_ack_tout))
		return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 0;
}

COMMAND(set, ofdma_ack_tout,
	"<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO, 0, CIB_PHY, handle_ofdma_ack_tout,
	"This command is used to configure ofdma ack timeout value");

int handle_csa_enable(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char csa;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		csa = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		csa = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_CSA_SUPPORT, csa))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, csa,
	"<0 or 1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO, 0, CIB_PHY, handle_csa_enable,
	"This command is used to configure CSA Enable/Disable");

int handle_csa_2g_enable(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char csa_2g;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		csa_2g = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		csa_2g = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_2G_CSA_SUPPORT, csa_2g))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, csa_2g,
	"<0 or 1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO, 0, CIB_PHY, handle_csa_2g_enable,
	"This command is used to configure 2G CSA Enable/Disable");

DECLARE_SECTION(dump);

#define RADIO_OPTIONS "[phymode]"
int print_radio_handler(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	u16 *phymode;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WMODE]) {
			phymode = (u16 *)nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WMODE]);
			printf("phymode: %d\n", *phymode);
		}
	} else
		printf("No Stats from driver\n");

	return 0;
}

int handle_radio_info_get(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *cmd_str;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data) {
		printf("nla_nest_start check fail!\n");
		return -ENOMEM;
	}

	if (argc > 0) {
		cmd_str = argv[0];
		printf("Invalid argument:%s, ignore it\n", cmd_str);
		return -EINVAL;
	}

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_WMODE, 0))
		return -EMSGSIZE;

	nla_nest_end(msg, data);

	register_handler(print_radio_handler, NULL);
	return 0;
}

COMMAND(dump, radio_info,
	RADIO_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_GET_RUNTIME_INFO, 0, CIB_NETDEV, handle_radio_info_get,
	"Get radio infomation.\n");

int print_bandinfo_handler(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_MAX+ 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char *show_str = NULL;
	int err = 0, reply_len = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BAND]) {
			reply_len = nla_len(vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BAND]);
			show_str = (char *)nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BAND]);

			if (reply_len <= 0)
				return err;

			if (*(show_str + reply_len - 1) != '\0')
                                *(show_str + reply_len - 1) = '\0';

			printf("Band : %s\n", show_str);
		}

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_FREQLIST]) {
			reply_len = nla_len(vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_FREQLIST]);
			show_str = (char *)nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_FREQLIST]);

			if (reply_len <= 0)
				return err;

			if (*(show_str + reply_len - 1) != '\0')
                                *(show_str + reply_len - 1) = '\0';

			printf("Freqlist : %s\n", show_str);
		}
	} else
		printf("No Data from driver\n");

	return 0;
}

int handle_band_info_get(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);

	if (!data) {
		printf("nla_nest_start check fail!\n");
		return -ENOMEM;
	}

	if (argc != 1)
		return -EINVAL;

	val_str = argv[0];

	if (strncmp("band", val_str ,strlen(val_str)) == 0) {
		register_handler(print_bandinfo_handler, NULL);
		if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_BAND, 0))
			return -EMSGSIZE;
	}

	if (strncmp("freqlist", val_str ,strlen(val_str)) == 0) {
		register_handler(print_bandinfo_handler, NULL);
		if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_GET_BAND_INFO_FREQLIST, 0))
			return -EMSGSIZE;
	}

	nla_nest_end(msg, data);

	return 0;
}

#define BAND_INFO_OPTIONS "[band | freqlist]"
COMMAND(dump, band_info,
	BAND_INFO_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_GET_BAND_INFO, 0, CIB_PHY, handle_band_info_get,
	"Get radio band infomation.\n");


int handle_ai_antinoise(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned short ai_antinoise = 0xffff;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		ai_antinoise = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		ai_antinoise = 1;
	else
		return -EINVAL;

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AP_AI_ANTINOISE, ai_antinoise))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ai_antinoise,
	"<0 or 1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_RADIO, 0, CIB_PHY, handle_ai_antinoise,
	"This command is used to configure AI antinoise Enable/Disable");


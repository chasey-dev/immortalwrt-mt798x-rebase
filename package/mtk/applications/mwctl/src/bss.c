
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

DECLARE_SECTION(set);

int get_ba_auto_status_callback(struct nl_msg *msg, void *data)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_AP_BA_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	u8 igmp_status;
	int err = 0;
	//u16 acl_result_len = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_AP_BA_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_BA_EN_INFO]) {
			igmp_status = nla_get_u8(vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_BA_EN_INFO]);
			if (igmp_status == 0) {
				printf("disabled\n");
			} else {
				printf("enabled\n");
			}
		}
	}

	return 0;
}

int handle_ba_auto(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char ba_auto;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		ba_auto = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		ba_auto = 1;
	else if (strncmp("s", val_str ,strlen(val_str)) == 0) {
		register_handler(get_ba_auto_status_callback, NULL);
		ba_auto = 0xf;
	} else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_BA_EN_INFO, ba_auto))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ba_auto,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_BA, 0, CIB_NETDEV, handle_ba_auto,
	"This command is used to configure ba enable");

int get_ba_decline_status_callback(struct nl_msg *msg, void *data)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_AP_BA_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	u8 igmp_status;
	int err = 0;
	//u16 acl_result_len = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_AP_BA_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_BA_DECLINE_INFO]) {
			//acl_result_len = nla_len(vndr_tb[MTK_NL80211_VENDOR_ATTR_MCAST_SNOOP_ENABLE]);
			igmp_status = nla_get_u8(vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_BA_DECLINE_INFO]);
			if (igmp_status == 0) {
				printf("disabled\n");
			} else {
				printf("enabled\n");
			}
		}
	}

	return 0;
}

int handle_ba_decline(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char ba_decline;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (strncmp("0", val_str ,strlen(val_str)) == 0)
		ba_decline = 0;
	else if (strncmp("1", val_str ,strlen(val_str)) == 0)
		ba_decline = 1;
	else if (strncmp("s", val_str ,strlen(val_str)) == 0) {
		register_handler(get_ba_decline_status_callback, NULL);
		ba_decline = 0xf;

	} else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_BA_DECLINE_INFO, ba_decline))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ba_decline,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_BA, 0, CIB_NETDEV, handle_ba_decline,
	"This command is used to configure ba decline");

#define BA_WIN_SZ_1024 1024
int handle_ba_wsize(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned short ba_wsize;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	ba_wsize = (unsigned short)strtoul(val_str, NULL, 10);
	if (ba_wsize ==0 || ba_wsize > BA_WIN_SZ_1024)
		return -EINVAL;

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AP_HT_BA_WSIZE_INFO, ba_wsize))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ba_wsize,
	"<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_BA, 0, CIB_NETDEV, handle_ba_wsize,
	"This command is used to configure ba tx&rx window size");

#define NUM_OF_TID	8
int handle_ba_setup(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned int mac_addr_tmp[6] = {0};
	unsigned int tid_tmp;
	struct ba_mactid_param ba_setup;
	int ret, i;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%x:%x:%x:%x:%x:%x-%d",
		&mac_addr_tmp[0], &mac_addr_tmp[1], &mac_addr_tmp[2],
		&mac_addr_tmp[3], &mac_addr_tmp[4], &mac_addr_tmp[5], &tid_tmp);

	if (ret != 7)
		return -EINVAL;

	for (i = 0; i < 6; i++) {
		if (mac_addr_tmp[i] > 0xff) {
			return -EINVAL;
		}
		ba_setup.mac_addr[i] = (unsigned char)mac_addr_tmp[i];
	}

	if (tid_tmp > NUM_OF_TID - 1)
		return -EINVAL;

	ba_setup.tid = (unsigned char)tid_tmp;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_BA_SETUP_INFO, sizeof(ba_setup), &ba_setup))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ba_setup,
	"<mac_addr>-<tid>",
	MTK_NL80211_VENDOR_SUBCMD_SET_BA, 0, CIB_NETDEV, handle_ba_setup,
	"This command is used to add ori ba entry");

int handle_ba_ori_teardown(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned int mac_addr_tmp[6] = {0};
	unsigned int tid_tmp;
	struct ba_mactid_param ba_ori_teardown;
	int ret, i;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%x:%x:%x:%x:%x:%x-%d",
		&mac_addr_tmp[0], &mac_addr_tmp[1], &mac_addr_tmp[2],
		&mac_addr_tmp[3], &mac_addr_tmp[4], &mac_addr_tmp[5], &tid_tmp);

	if (ret != 7)
		return -EINVAL;

	for (i = 0; i < 6; i++) {
		if (mac_addr_tmp[i] > 0xff) {
			return -EINVAL;
		}
		ba_ori_teardown.mac_addr[i] = (unsigned char)mac_addr_tmp[i];
	}

	if (tid_tmp > NUM_OF_TID - 1)
		return -EINVAL;

	ba_ori_teardown.tid = (unsigned char)tid_tmp;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_BA_ORITEARDOWN_INFO, sizeof(ba_ori_teardown), &ba_ori_teardown))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ba_ori_teardown,
	"<mac_addr>-<tid>",
	MTK_NL80211_VENDOR_SUBCMD_SET_BA, 0, CIB_NETDEV, handle_ba_ori_teardown,
	"This command is used to remove ori ba entry");

int handle_ba_rec_teardown(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned int mac_addr_tmp[6] = {0};
	unsigned int tid_tmp;
	struct ba_mactid_param ba_rec_teardown;
	int ret, i;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%x:%x:%x:%x:%x:%x-%d",
		&mac_addr_tmp[0], &mac_addr_tmp[1], &mac_addr_tmp[2],
		&mac_addr_tmp[3], &mac_addr_tmp[4], &mac_addr_tmp[5], &tid_tmp);

	if (ret != 7)
		return -EINVAL;

	for (i = 0; i < 6; i++) {
		if (mac_addr_tmp[i] > 0xff) {
			return -EINVAL;
		}
		ba_rec_teardown.mac_addr[i] = (unsigned char)mac_addr_tmp[i];
	}

	if (tid_tmp > NUM_OF_TID - 1)
		return -EINVAL;

	ba_rec_teardown.tid = (unsigned char)tid_tmp;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_BA_RECTEARDOWN_INFO, sizeof(ba_rec_teardown), &ba_rec_teardown))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}
COMMAND(set, ba_rec_teardown,
	"<mac_addr>-<tid>",
	MTK_NL80211_VENDOR_SUBCMD_SET_BA, 0, CIB_NETDEV, handle_ba_rec_teardown,
	"This command is used to remove rec ba entry");

#define MAX_MWDS_PARAM_LEN 32
struct mwds_option {
	char option_name[MAX_MWDS_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char value);
};
int mwds_enable_attr_put(struct nl_msg *msg, char value)
{
	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MWDS_ENABLE, value))
		return -EMSGSIZE;
	return 0;
}

int mwds_info_attr_put(struct nl_msg *msg, char value)
{
	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MWDS_INFO, value))
		return -EMSGSIZE;
	return 0;
}

struct mwds_option mwds_opt[] = {
	{"enable", mwds_enable_attr_put},
	{"info", mwds_info_attr_put},
};

static int handle_mwds_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr, *ptr_tmp, *param_str, *val_str, invalide = 0;
	int i, j;
	char Enable;
	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	for (i = 0; i < argc; i++) {
		ptr = argv[i];
		param_str = ptr;
		val_str = strchr(ptr, '=');

		if (!val_str)
			continue;

		*val_str++ = 0;

		for (j = 0; j < (sizeof(mwds_opt) / sizeof(mwds_opt[0])); j++) {
			if (strlen(mwds_opt[j].option_name) == strlen(param_str) &&
				!strncmp(mwds_opt[j].option_name, param_str, strlen(param_str)))
				break;
		}

		errno = 0;
		Enable = strtol(val_str, &ptr_tmp, 10);
		if (errno == ERANGE)
			return -EINVAL;
		if (j != (sizeof(mwds_opt) / sizeof(mwds_opt[0]))) {
			if (mwds_opt[j].attr_put(msg, Enable) < 0)
				printf("invalide argument %s=%d, ignore it\n", param_str, Enable);
			else
				invalide = 1;
		}
	}

	nla_nest_end(msg, data);

	if (!invalide)
		return -EINVAL;

	return 0;
}

COMMAND(set, mwds,
	"enable=<value>/info=<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_MWDS, 0, CIB_NETDEV, handle_mwds_set,
	"This cmd is used to set mwds enable and info");

#define PHYMODE_HELP "###WIFI 4/5###\n" \
	"0 => B/G mixed\n" \
	"1 => B only\n" \
	"2 => A only\n" \
	"3 => A/B/G mixed\n" \
	"4 => G only\n" \
	"5 => A/B/G/GN/AN mixed\n" \
	"6 => N in 2.4G band only\n" \
	"7 => G/GN, i.e., no CCK mode\n" \
	"8 => A/N in 5 band\n" \
	"9 => B/G/GN mode\n" \
	"10 => A/AN/G/GN mode, not support B mode\n" \
	"11 => only N in 5G band\n" \
	"12 => B/G/GN/A/AN/AC mixed\n" \
	"13 => G/GN/A/AN/AC mixed, no B mode\n" \
	"14 => A/AC/AN mixed\n" \
	"15 => AC/AN mixed, but no A mode\n" \
	"###WIFI 6###\n" \
	"16 => B/G/GN/AX_24G\n" \
	"17 => A/AC/AN/AX_5G\n" \
	"###WIFI 6E###\n" \
	"18 => AC/AN/AX_5G/AX_6G\n" \
	"19 => G/GN/AX_24G/AX_6G\n" \
	"20 => A/AC/AN/AX_5G/AX_6G\n" \
	"21 => G/GN/AX_24G/A/AC/AN/AX_5G/AX_6G\n" \
	"###WIFI 7###\n" \
	"22 => B/G/GN/AX_24G/BE_24G\n" \
	"23 => B/G/GN/AX_5G/BE_5G\n" \
	"24 => AC/AN/AX_5G/AX_6G/BE_6G\n" \
	"25 => G/GN/AX_24G/AX_6G/BE_24G/BE_6G\n" \
	"26 => A/AC/AN/AX_5G/AX_6G/BE_5G/BE_6G\n" \
	"27 => G/GN/AX_24G/A/AC/AN/AX_5G/AX_6G/BE_24G/BE_5G/BE_6G\n"

#define PHYMODE_OPTIONS "<phymode_idx>"

int handle_phymode(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char phymode;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	phymode = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_WIRELESS_MODE, phymode))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, phymode,
	PHYMODE_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_phymode,
	PHYMODE_HELP);

int handle_ht_tx_stream(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned long value;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	value = (unsigned long)strtoul(val_str, NULL, 10);

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_HT_TX_STREAM_INFO, value))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ht_tx_stream,
	"<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_ht_tx_stream,
	"This command is used to configure ht tx stream");

int handle_assocreq_rssi_thres(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	char rssi;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	rssi = (char)strtol(val_str, NULL, 10);

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_ASSOCREQ_RSSI_THRES_INFO, sizeof(char), &rssi))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, assocreq_rssi_thres,
	"<value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_assocreq_rssi_thres,
	"This command is used to configure assocreq rssi thres");
#define MAX_POWER_PARAM_LEN 32
#define AP_TXPWR_OPTIONS "[TxPower=<100|75|50|25|12|0>]\n"\
	"[MaxTxPwr=<23>]\n"\
	"[TxPowerInfo=<0|1|2>]\n"\
	"[PercentageCtrl=<0|1>]\n"\
	"[PowerDropCtrl=<100|75|50|25|12|0>]\n"\
	"[DecreasePower=<20>]\n"\
	"[SKUCtrl=<0|1>]\n"\
	"[SKUInfo=<0|1|2>]\n"\
	"[MUTxPower=<0|1:23>]\n"\
	"[mgmt_frame_pwr=<23>]\n"\
	"[PsuSkuCtrl=<0|1>]\n"\
	"[PsuSkuTableIdx=<0:19>]\n"
#define AP_TXPWR_HELP "TxPower:Drop Target TX power by a percentage, work after set channel\n"\
	"MaxTxPwr:set maximum tx power\n"\
	"TxPowerInfo:show power info\n"\
	"PercentageCtrl:enable/disable power percentage\n"\
	"PowerDropCtrl:Drop Target TX power by a percentage, directly to FW\n"\
	"DecreasePower:Drop Target TX power by a specific value(value*0.5dB), directly to FW\n"\
	"SKUCtrl:enable/disable SKU function\n"\
	"SKUInfo:show SKU information\n"\
	"MUTxPower:set MU mannual Tx Power\n"\
	"mgmt_frame_pwr:specific management tx power\n"\
	"PsuSkuCtrl:enable/disable PSU SKU\n"\
	"PsuSkuTableIdx:PSU SKU table index\n"

struct power_option {
	char option_name[MAX_POWER_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char *value);
};
int tx_power_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char pwr_percentage;

	if (!value)
		return -EINVAL;
	pwr_percentage = strtoul(value, NULL, 10);
	if (pwr_percentage > 100)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE, pwr_percentage))
		return -EMSGSIZE;

	return 0;
}
int tx_maxpower_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char pwr_max;

	if (!value)
		return -EINVAL;
	pwr_max = strtoul(value, NULL, 10);
	if (pwr_max > 63)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_MAX, pwr_max))
		return -EMSGSIZE;

	return 0;
}
int tx_power_info_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char pwr_info;

	if (!value)
		return -EINVAL;
	pwr_info = strtoul(value, NULL, 10);
	if (pwr_info > 2)/*Now, the FW not support bigger than 2*/
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_INFO, pwr_info))
		return -EMSGSIZE;

	return 0;
}
int tx_power_percentage_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char pwr_drop_en;

	if (!value)
		return -EINVAL;
	pwr_drop_en = strtoul(value, NULL, 10);
	if ((pwr_drop_en != 1) && (pwr_drop_en != 0))
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_PERCENTAGE_EN, pwr_drop_en))
		return -EMSGSIZE;

	return 0;
}
int tx_power_drop_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char pwr_drop;

	if (!value)
		return -EINVAL;
	pwr_drop = strtoul(value, NULL, 10);
	if (pwr_drop > 100)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_DROP_CTRL, pwr_drop))
		return -EMSGSIZE;

	return 0;
}
int tx_power_decrease_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char pwr_dec;

	if (!value)
		return -EINVAL;
	pwr_dec = strtoul(value, NULL, 10);
	if (pwr_dec > 63)/*power decrease value 0 ~ 31dBm*/
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_DECREASE, pwr_dec))
		return -EMSGSIZE;

	return 0;
}
int tx_skuctrl_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char sku_en;

	if (!value)
		return -EINVAL;
	sku_en = strtoul(value, NULL, 10);
	if ((sku_en != 1) && (sku_en != 0))
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_CTRL, sku_en))
		return -EMSGSIZE;

	return 0;
}
int tx_skuinfo_attr_put(struct nl_msg *msg, char *value)
{
	if (!value)
		return -EINVAL;
	if (nla_put_flag(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_SKU_INFO))
		return -EMSGSIZE;
	return 0;
}

int tx_mu_power_attr_put(struct nl_msg *msg, char *value)
{
	struct mu_power_param mupower_param = {.en = 0, .value = 0};
	int ret = 0;

	if (!value)
		return -EINVAL;
	ret = sscanf(value, "%d:%d", &mupower_param.en, &mupower_param.value);
	if (ret > 1) {

		if (mupower_param.en != 0 && mupower_param.en != 1)
			return -EINVAL;
		if (mupower_param.value >= 63)
			return -EINVAL;
		if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_MU, sizeof(mupower_param), &mupower_param))
			return -EMSGSIZE;
	} else
		return -EINVAL;
	return 0;
}
int tx_mgmt_power_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char len;

	if (!value)
		return -EINVAL;
	len = strlen(value);
	if (len > 4)
		return -EINVAL;
	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_MGMT, len, value))
		return -EMSGSIZE;
	return 0;
}
int tx_psu_skuctrl_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char sku_en;

	if (!value)
		return -EINVAL;
	sku_en = strtoul(value, NULL, 10);
	if ((sku_en != 1) && (sku_en != 0))
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_PSU_SKU_CTRL, sku_en))
		return -EMSGSIZE;

	return 0;
}
int tx_psu_sku_table_idx_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char idx;

	if (!value)
		return -EINVAL;
	idx = strtoul(value, NULL, 10);
	if (idx >= 21)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_TXPWR_PSU_SKU_TABLE_IDX, idx))
		return -EMSGSIZE;

	return 0;
}

struct power_option pwr_opt[] = {
	{"TxPower", tx_power_attr_put},
	{"MaxTxPwr", tx_maxpower_attr_put},
	{"TxPowerInfo", tx_power_info_attr_put},
	{"PercentageCtrl", tx_power_percentage_attr_put},
	{"PowerDropCtrl", tx_power_drop_attr_put},
	{"DecreasePower", tx_power_decrease_attr_put},
	{"SKUCtrl", tx_skuctrl_attr_put},
	{"SKUInfo", tx_skuinfo_attr_put},
	{"MUTxPower",tx_mu_power_attr_put},
	{"mgmt_frame_pwr",tx_mgmt_power_attr_put},
	{"PsuSkuCtrl", tx_psu_skuctrl_attr_put},
	{"PsuSkuTableIdx", tx_psu_sku_table_idx_attr_put},
};
static int handle_TXPower_set(struct nl_msg *msg, int argc,
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
		val_str = strchr(ptr, '=');

		if (!val_str)
			continue;

		*val_str++ = 0;

		for (j = 0; j < (sizeof(pwr_opt) / sizeof(pwr_opt[0])); j++) {
			if (strlen(pwr_opt[j].option_name) == strlen(param_str) &&
				!strncmp(pwr_opt[j].option_name, param_str, strlen(param_str)))
				break;
		}
		if (j != (sizeof(pwr_opt) / sizeof(pwr_opt[0]))) {
			if (pwr_opt[j].attr_put(msg, val_str) < 0)
				printf("invalide argument %s=%s, ignore it\n", param_str, val_str);
			else
				invalide = 1;
		}

	}

	nla_nest_end(msg, data);

	if (!invalide)
		return -EINVAL;

	return 0;
}

COMMAND(set, pwr,
	AP_TXPWR_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_TXPOWER, 0, CIB_NETDEV, handle_TXPower_set,
	AP_TXPWR_HELP);

#define AP_EDCA_OPTIONS "[TxBurst=<0|1>]\n"
#define AP_EDCA_HELP "control txburst on/off."
static int handle_edca_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char txburst = 0;

	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;
	val_str = argv[0];
	if (!val_str)
		return -EINVAL;
	txburst = strtoul(val_str, NULL, 10);
	if ((txburst != 1) && (txburst != 0))
		return -EINVAL;
	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_TX_BURST, txburst))
		return -EMSGSIZE;
	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, txburst,
	AP_EDCA_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_EDCA, 0, CIB_NETDEV, handle_edca_set,
	AP_EDCA_HELP);

static int set_ht_mpdu_density(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char mpdu_density;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	mpdu_density = strtol(val_str, NULL, 10);
	if (mpdu_density > 7)
		mpdu_density = 0;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_MPDU_DENSITY, mpdu_density))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, mpdu_density,
	"<density_value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, set_ht_mpdu_density,
	"This cmd is used to config mpdu density\n");

static int set_ht_amsdu(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char amsdu_en;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	amsdu_en = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_AMSDU_EN, amsdu_en))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ht_amsdu,
	"<amsdu_en>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, set_ht_amsdu,
	"This cmd is used to config amsdu enable/disable\n");

static int set_bss_coex_ap_cnt_thr(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char cnt_thr;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	cnt_thr = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_CNT_THR, cnt_thr))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, bss_coex_ap_thr,
	"<ap_thr_value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, set_bss_coex_ap_cnt_thr,
	"This cmd is used to config coex ap count threshold\n");

static int set_bss_ht_ext_cha(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char value;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	value = strtol(val_str, NULL, 10);

	if (value != 0 && value != 1)
		return -ENOMEM;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_HT_EXT_CHA, value))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ht_ext_cha,
	"<ht_ext_cha_value>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, set_bss_ht_ext_cha,
	"This cmd is used to config ht_ext_cha\n");

static int set_ht_protect(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char ht_protect;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	ht_protect = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_HT_PROTECT, ht_protect))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ht_protect,
	"<ht_protect>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, set_ht_protect,
	"This cmd is used to config ht protect\n");

static int set_vht_disallow_non_vht(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char disallow_not_vht;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	disallow_not_vht = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_DISALLOW_NON_VHT, disallow_not_vht))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, disallow_non_vht,
	"<disallow_non_vht>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, set_vht_disallow_non_vht,
	"This cmd is used to config disallow non vht\n");

static int set_etxbf_en_cond(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char etxbf_en_cond;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	etxbf_en_cond = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_ETXBF_EN_COND, etxbf_en_cond))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, etxbf_en_cond,
	"<etxbf_en_cond>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, set_etxbf_en_cond,
	"This cmd is used to config etxbf en\n");

static int set_pmf_sha256(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char pmf_sha256;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	pmf_sha256 = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_PMF_SHA256, pmf_sha256))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, pmf_sha256,
	"<pmf_sha256>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, set_pmf_sha256,
	"This cmd is used to config pmf sha256\n");

#define BCN_INT_HELP "set bcn interval 20~1024 ms\n"
#define BCN_INT_OPTIONS "<bcn_interval>"

int handle_beacon_int(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned short beacon_int;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if ((strtol(val_str, NULL, 10) < 20) || (strtol(val_str, NULL, 10) >1024)) {
		printf("bcn interval need set to 20~1024\n");
		return -EINVAL;
	}

	beacon_int = strtol(val_str, NULL, 10);

	if (nla_put_u16(msg, MTK_NL80211_VENDOR_ATTR_AP_BCN_INT, beacon_int))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, beacon_int,
	BCN_INT_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_beacon_int,
	BCN_INT_HELP);

#define DTIM_INT_HELP "set dtim interval 1~255\n"
#define DTIM_INT_OPTIONS "<dtim_interval>"

int handle_dtim_int(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char dtim_int;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if ((strtol(val_str, NULL, 10) < 1) || (strtol(val_str, NULL, 10) >255)) {
		printf("dtim interval need set to 1~255\n");
		return -EINVAL;
	}

	dtim_int = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_DTIM_INT, dtim_int))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, dtim_int,
	DTIM_INT_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_dtim_int,
	DTIM_INT_HELP);

#define HIDE_SSID_HELP "set to hide some bss, 1:on 0:off\n"
#define HIDE_SSID_OPTIONS "<hide_en>"

int handle_hide_ssid(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char hide_en;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if ((strtol(val_str, NULL, 10) != 0) && (strtol(val_str, NULL, 10) != 1)) {
		printf("hide_en need set to 1 or 0\n");
		return -EINVAL;
	}

	hide_en = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_HIDDEN_SSID, hide_en))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, hide_ssid,
	HIDE_SSID_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_hide_ssid,
	HIDE_SSID_HELP);

#define HT_OP_MODE_HELP "set bss ht op mode, 1:green field,0:mix mode\n"
#define HT_OP_MODE_OPTIONS "<ht_op_mode>"

int handle_ht_op_mode(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char ht_op_mode;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if ((strtol(val_str, NULL, 10) != 0) && (strtol(val_str, NULL, 10) != 1)) {
		printf("ht_op_mode need set to 1 or 0\n");
		return -EINVAL;
	}

	ht_op_mode = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_HT_OP_MODE, ht_op_mode))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, ht_op_mode,
	HT_OP_MODE_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_ht_op_mode,
	HT_OP_MODE_HELP);

#define BSS_MAX_IDLE_TIMEOUT_OPTIONS "<value> (seconds)"
#define BSS_MAX_IDLE_TIMEOUT_HELP "This command is used to configure ap idle timeout"
int handle_max_bss_idle_timeout(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned int idle_period;
	int ret;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	ret = sscanf(val_str, "%u", &idle_period);

	if (ret != 1)
		return -EINVAL;

	if (nla_put_u32(msg, MTK_NL80211_VENDOR_ATTR_AP_BSS_MAX_IDLE, idle_period))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, bss_max_idle_timeout,
	BSS_MAX_IDLE_TIMEOUT_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_max_bss_idle_timeout,
	BSS_MAX_IDLE_TIMEOUT_HELP);

int handle_muru_ofdma_dl_en(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char muru_ofdma_dl_en;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	if (strncmp("0", val_str, strlen(val_str)) == 0)
		muru_ofdma_dl_en = 0;
	else if (strncmp("1", val_str, strlen(val_str)) == 0)
		muru_ofdma_dl_en = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_DL_EN, muru_ofdma_dl_en))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, muru_dl_en,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_muru_ofdma_dl_en,
	"This command is used to configure muru ofdma dl enable");

int handle_muru_ofdma_ul_en(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char muru_ofdma_ul_en;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	if (strncmp("0", val_str, strlen(val_str)) == 0)
		muru_ofdma_ul_en = 0;
	else if (strncmp("1", val_str, strlen(val_str)) == 0)
		muru_ofdma_ul_en = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MURU_OFDMA_UL_EN, muru_ofdma_ul_en))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, muru_ul_en,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_muru_ofdma_ul_en,
	"This command is used to configure muru ofdma ul enable");

int handle_mu_mimo_dl_en(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char mu_mimo_dl_en;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	if (strncmp("0", val_str, strlen(val_str)) == 0)
		mu_mimo_dl_en = 0;
	else if (strncmp("1", val_str, strlen(val_str)) == 0)
		mu_mimo_dl_en = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MU_MIMO_DL_EN, mu_mimo_dl_en))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, mu_dl_en,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_mu_mimo_dl_en,
	"This command is used to configure mu-mimo dl enable");

int handle_mu_mimo_ul_en(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char mu_mimo_ul_en;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	if (!val_str)
		return -EINVAL;

	if (strncmp("0", val_str, strlen(val_str)) == 0)
		mu_mimo_ul_en = 0;
	else if (strncmp("1", val_str, strlen(val_str)) == 0)
		mu_mimo_ul_en = 1;
	else
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MU_MIMO_UL_EN, mu_mimo_ul_en))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, mu_ul_en,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_mu_mimo_ul_en,
	"This command is used to configure mu-mimo ul enable");

int handle_rts_bw_signaling(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char rts_bw_signaling;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	rts_bw_signaling = strtol(val_str, NULL, 10);
	if (rts_bw_signaling > 2)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_RTS_BW_SIGNALING, rts_bw_signaling))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;

}

COMMAND(set, bw_signaling,
	"<0 0r 1 0r 2>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_rts_bw_signaling,
	"This command config RTS BW singaling,0:disable/1:static/2:dynamic\n");


int handle_mlo_switch(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *cmd_str;
	unsigned char enable;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (!argc || argc != 1) {
		printf("Invalid argument\n");
		return -EINVAL;
	}

	cmd_str = argv[0];
	enable = strtol(cmd_str, NULL, 10);
	printf("Enable = %d\n",enable);
	if (enable > 1) {
		printf("[%s - %d] wrong input value\n",__func__,__LINE__);
		return -EINVAL;
	}

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MLO_SWITCH_SET, enable))
				return -EMSGSIZE;
	nla_nest_end(msg, data);

	return 0;
}


COMMAND(set, mlo_switch,
	"<0 0r 1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_MLO_SWITCH, 0, CIB_NETDEV, handle_mlo_switch,
	"Set sta mlo link .\n");


int handle_mgmt_rx(struct nl_msg *msg, int argc,
		char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char en_reject;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	en_reject = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_MGMT_RX, en_reject))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

#define MGMT_RX_HELP "set mgmt rx action, 2:reject any mgmt rx 1: accept beacon/probe_rsp 0:normal mgmt rx\n"
#define MGMT_RX_OPTIONS "<0/1/2>\n 2:reject any mgmt rx\n 1:accept beacon/probe_rsp\n 0:normal mgmt rx"
COMMAND(set, mgmt_rx,
	MGMT_RX_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_mgmt_rx,
	MGMT_RX_HELP);

int handle_no_bcn(struct nl_msg *msg, int argc,
			char **argv, void *ctx)
{
	void *data;
	char *val_str;
	unsigned char no_bcn;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];

	no_bcn = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_NO_BCN, no_bcn))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

#define NO_BCN_HELP "set ap no beacon action, 1:stop to send beacon 0:start to send beacon\n"
#define NO_BCN_OPTIONS "<1/0>\n 1:no beacon\n 0:has beacon"
COMMAND(set, no_bcn,
	NO_BCN_OPTIONS,
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_no_bcn,
	NO_BCN_HELP);

int handle_set_11axonly(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	char Enable;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
	return -ENOMEM;

	val_str = argv[0];
	Enable = strtol(val_str, NULL, 10);
	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_SET_11AXONLY, Enable))
	return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, set_11axonly,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_set_11axonly,
	"This command is used to configure 11ax only make sure wirelessmod 16~21");

int handle_set_11beonly(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	char Enable;

	if (!argc || argc != 1)
	return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	Enable = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_SET_11BEONLY, Enable))
	return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, set_11beonly,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_set_11beonly,
	"This command is used to configure 11be only make sure wirelessmod 22~27");


int handle_set_no_agmode(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	char Enable;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	Enable = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_SET_NO_AGMODE, Enable))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, set_no_agmode,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_set_no_agmode,
	"This command is used to configure 11be only make sure wirelessmod 12~27");

int handle_set_no_nmode(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *val_str;
	char Enable;

	if (!argc || argc != 1)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	val_str = argv[0];
	Enable = strtol(val_str, NULL, 10);

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_SET_NO_NMODE, Enable))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(set, set_no_nmode,
	"<0/1>",
	MTK_NL80211_VENDOR_SUBCMD_SET_AP_BSS, 0, CIB_NETDEV, handle_set_no_nmode,
	"This command is used to configure 11be only make sure wirelessmod 12~27");

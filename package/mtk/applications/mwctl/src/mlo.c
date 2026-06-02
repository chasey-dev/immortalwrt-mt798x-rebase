/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include <stdio.h>
#include <limits.h>
#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

extern inline int hwaddr_aton2(const char *txt, u8 *addr);
extern inline int hex2num(char c);
DECLARE_SECTION(dump);
#define MAX_MLD_PARAM_LEN 128

struct mld_option {
	char option_name[MAX_MLD_PARAM_LEN];
	int (* attr_put)(struct nl_msg *msg, char *value);
};

int handle_mlo_info_show(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *cmd_str;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (argc > 0) {
		cmd_str = argv[0];
		printf("Invalid argument:%s, ignore it\n", cmd_str);
		return -EINVAL;
	}

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_MLO_INFO_SHOW_CMD_STR, 1))
				return -EMSGSIZE;
	nla_nest_end(msg, data);

	return 0;
}

int ap_mld_dump_callback(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_ATTR_MAX + 1];
	struct nlattr *affiliated_ap_attr, *affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int err = 0, rem;
	unsigned char *mac, mld_index;
	unsigned char *ap_bssid, ap_linkid;
	unsigned short ap_disabled_subchan;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_AP_MLD_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		printf("AP MLD dump start================>\n");
		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_ADDRESS]) {
			mac = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_ADDRESS]);
			printf("AP MLD MAC:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX]) {
			mld_index = nla_get_u8(vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX]);
			printf("AP MLD group id:%u\n", mld_index);
		}

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_APS]) {
			int i = 0;
			nla_for_each_nested(affiliated_ap_attr, vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_APS], rem) {
				if (nla_parse_nested(affiliated_aps, MTK_NL80211_VENDOR_ATTR_AP_MLD_ATTR_MAX, affiliated_ap_attr, NULL) != 0)
					continue;
				if (affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_BSSID]) {
					ap_bssid = nla_data(affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_BSSID]);
					printf("Affiliated AP[%d] BSSID %02x:%02x:%02x:%02x:%02x:%02x\n", i,
						ap_bssid[0], ap_bssid[1], ap_bssid[2], ap_bssid[3], ap_bssid[4], ap_bssid[5]);
				}

				if (affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_LINKID]) {
					ap_linkid = nla_get_u8(affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_LINKID]);
					printf("Affiliated AP[%d] Link ID %d\n", i, ap_linkid);
				}

				if (affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_DISABLED_SUBCHAN]) {
					ap_disabled_subchan = nla_get_u16(affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_AFFILIATED_AP_DISABLED_SUBCHAN]);
					printf("Affiliated AP[%d] Disabled Subchannel %d\n", i, ap_disabled_subchan);
				}

				if (affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLMR])
					printf("Affiliated AP[%d] EMLMR enabled\n", i);

				if (affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLSR])
					printf("Affiliated AP[%d] EMLSR enabled\n", i);

				if (affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_STR])
					printf("Affiliated AP[%d] STR enabled\n", i);

				if (affiliated_aps[MTK_NL80211_VENDOR_ATTR_AP_MLD_NSTR])
					printf("Affiliated AP[%d] NSTR enabled\n", i);
				i++;
			}
		}
	} else {
		printf("no ap mld get from driver\n");
	}
	printf("AP MLD dump end<================\n");
	return 0;
}

int mld_query_all_attr_put(struct nl_msg *msg, char *value)
{
	if (value) {
		printf("value string should be empty for dumping all mlds!\n");
		return -EINVAL;
	}

	if (nla_put_flag(msg, MTK_NL80211_VENDOR_ATTR_AP_MLD_DUMP))
		return -EMSGSIZE;

	return 0;
}

int mld_query_mld_by_mac_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char mld_mac[ETH_ALEN];

	if (!value) {
		printf("value string should be not be NULL for dumping mld by mac!\n");
		return -EINVAL;
	}

	if (hwaddr_aton2(value, mld_mac) < 0) {
		printf("invalide mld mac address string %s!\n", value);
		return -EINVAL;
	}

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_MLD_ADDRESS, ETH_ALEN, mld_mac))
		return -EMSGSIZE;

	return 0;
}

int mld_query_mld_by_index_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char mld_index;
	long retl;
	char *endptr;

	if (!value) {
		printf("value string should be not be NULL for dumping mld by mld index!\n");
		return -EINVAL;
	}
	retl = strtol(value, &endptr, 10);

	if (retl < 0 || retl > 255)
		return -EINVAL;

	if (*endptr != '\0' || endptr == value)
		return -EINVAL;

	mld_index = retl;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX, mld_index))
		return -EMSGSIZE;

	return 0;
}

struct mld_option mld_options[] = {
	{"all", mld_query_all_attr_put},
	{"mac", mld_query_mld_by_mac_attr_put},
	{"index", mld_query_mld_by_index_attr_put},
};

int handle_ap_mlo_dump(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr = NULL, *param_str = NULL, *val_str = NULL, invalide = 0;
	int i, j;

	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	for (i = 0; i < argc; i++) {
		ptr = argv[i];
		param_str = strtok(ptr, "=");

		if (param_str == NULL)
			return -EINVAL;

		val_str = strtok(NULL, "=");

		for (j = 0; j < (sizeof(mld_options) / sizeof(mld_options[0])); j++) {
			if (strlen(mld_options[j].option_name) == strlen(param_str) &&
				!strncmp(mld_options[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(mld_options) / sizeof(mld_options[0]))) {
			if (mld_options[j].attr_put(msg, val_str) < 0)
				printf("param_str %s att_put fail\n", param_str);
			else
				invalide = 1;
			continue;
		}
	}

	nla_nest_end(msg, data);

	if (!invalide)
		return -EINVAL;
	else
		register_handler(ap_mld_dump_callback, NULL);

	return 0;
}


int apcli_mld_dump_callback(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_ATTR_MAX + 1];
	struct nlattr *affiliated_apcli_attr, *affiliated_apclis[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int err = 0, rem;
	unsigned char *mac, *affiliated_apcli;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_APCLI_MLD_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		printf("APCLI MLD dump start================>\n");
		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_MAC]) {
			mac = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_MAC]);
			printf("APCLI MLD MAC:%02x:%02x:%02x:%02x:%02x:%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_AP_MLD_MAC]) {
			mac = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_AP_MLD_MAC]);
			printf("APCLI connected AP MLD MAC:%02x:%02x:%02x:%02x:%02x:%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STAS]) {
			int i = 0;
			nla_for_each_nested(affiliated_apcli_attr, vndr_tb[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STAS], rem) {
				if (nla_parse_nested(affiliated_apclis, MTK_NL80211_VENDOR_ATTR_APCLI_MLD_ATTR_MAX, affiliated_apcli_attr, NULL) != 0)
					continue;
				if (affiliated_apclis[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STA_MAC]) {
					affiliated_apcli = nla_data(affiliated_apclis[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_AFFILIATED_STA_MAC]);
					printf("Affiliated APCLI[%d] MAC %02x:%02x:%02x:%02x:%02x:%02x\n", i,
						affiliated_apcli[0], affiliated_apcli[1], affiliated_apcli[2], affiliated_apcli[3], affiliated_apcli[4], affiliated_apcli[5]);
				}

				if (affiliated_apclis[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_STR_ENABLE]) {
					printf("Affiliated APCLI[%d] STR enabled\n", i);
				}

				if (affiliated_apclis[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_NSTR_ENABLE]) {
					printf("Affiliated APCLI[%d] NSTR enabled\n", i);
				}

				if (affiliated_apclis[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_EMLMR_ENABLE]) {
					printf("Affiliated APCLI[%d] EMLMR enabled\n", i);
				}

				if (affiliated_apclis[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_EMLSR_ENABLE]) {
					printf("Affiliated APCLI[%d] EMLSR enabled\n", i);
				}

				if (affiliated_apclis[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_LINK_MAC]) {
					mac = nla_data(affiliated_apclis[MTK_NL80211_VENDOR_ATTR_APCLI_MLD_LINK_MAC]);
					printf("Affiliated APCLI[%d] LINK BSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
						i, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				}
				i++;
			}
		}
	} else {
		printf("no apcli mld get from driver\n");
	}
	printf("APCLI MLD dump end<================\n");
	return 0;
}

int handle_apcli_mlo_dump(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;

	if (argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (nla_put_flag(msg, MTK_NL80211_VENDOR_ATTR_DUMP_APCLI_MLD))
		return -EMSGSIZE;

	nla_nest_end(msg, data);

	register_handler(apcli_mld_dump_callback, NULL);

	return 0;
}


int connected_sta_mld_query_all_attr_put(struct nl_msg *msg, char *value)
{
	return 0;
}

int connected_sta_mld_query_mld_by_index_attr_put(struct nl_msg *msg, char *value)
{
	unsigned char mld_index;
	long retl;
	char *endptr;

	if (!value) {
		printf("value string should be not be NULL for dumping mld by mld index!\n");
		return -EINVAL;
	}
	retl = strtol(value, &endptr, 10);

	if (retl < 0 || retl > 255)
		return -EINVAL;

	if (*endptr != '\0' || endptr == value)
		return -EINVAL;

	mld_index = retl;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX_TO_DUMP, mld_index))
		return -EMSGSIZE;

	return 0;
}

struct mld_option con_sta_mld_options[] = {
	{"all", connected_sta_mld_query_all_attr_put},
	{"index",connected_sta_mld_query_mld_by_index_attr_put},
};

int connected_sta_mld_dump_callback(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX + 1];
	struct nlattr *affiliated_sta_attr, *affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int err = 0, rem, tmp_s32;
	unsigned char *mac, mld_index, tmp;
	unsigned char linkid;
	unsigned int time, tmp_u32;
	unsigned long long tmp_u64;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		printf("connected sta MLD dump start================>\n");
		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX_TO_DUMP]) {
			mld_index = nla_get_u8(vndr_tb[MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX_TO_DUMP]);
			printf("connected ap mld index=%u\n", mld_index);
		}

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAC]) {
			mac = nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAC]);
			printf("Connected sta mld macBSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
				mac [0], mac [1], mac [2], mac [3], mac [4], mac [5]);
		}
		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_EMLMR])
			printf("\tEMLMR enabled\n");

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_EMLSR])
			printf("\tEMLSR enabled\n");

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_STR])
			printf("\tSTR enabled\n");

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_NSTR])
			printf("\tNSTR enabled\n");

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_LAST_CONNECT_TIME]) {
			time = nla_get_u32(vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_LAST_CONNECT_TIME]);
			printf("\tconnected time=%u\n", time);
		}

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA]) {
			int i = 0;
			nla_for_each_nested(affiliated_sta_attr, vndr_tb[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA], rem) {
				if (nla_parse_nested(affiliated_stas, MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_MAX, affiliated_sta_attr, NULL) != 0)
					continue;
				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_MAC]) {
					mac = nla_data(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_MAC]);
					printf("affiliated sta[%d] mac %02x:%02x:%02x:%02x:%02x:%02x\n", i,
						mac[0], mac [1], mac [2], mac [3], mac [4], mac [5]);
				}

				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BSSID]) {
					mac = nla_data(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BSSID]);
					printf("affiliated sta[%d] BSSID %02x:%02x:%02x:%02x:%02x:%02x\n", i,
						mac[0], mac [1], mac [2], mac [3], mac [4], mac [5]);
				}

				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LINKID]) {
					linkid = nla_get_u8(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LINKID]);
					printf("affiliated sta[%d] Link ID %d\n", i, linkid);
				}

				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_UL]) {
					tmp = nla_get_u8(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_UL]);
					printf("affiliated sta[%d] TID MAP UL=%02x\n", i, tmp);
				}

				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_DL]) {
					tmp = nla_get_u8(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_TID_MAP_DL]);
					printf("affiliated sta[%d] TID MAP DL=%02x\n", i, tmp);
				}

				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_SENT]) {
					tmp_u64 = nla_get_u64(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_SENT]);
					printf("affiliated sta[%d] bytes send=%llu\n", i, tmp_u64);
				}
				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_RECEIVED]) {
					tmp_u64 = nla_get_u64(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_BYTES_RECEIVED]);
					printf("affiliated sta[%d] bytes received=%llu\n", i, tmp_u64);
				}

				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_SENT]) {
					tmp_u32 = nla_get_u32(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_SENT]);
					printf("affiliated sta[%d] packets send=%u\n", i, tmp_u32);
				}

				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_RECEIVED]) {
					tmp_u32 = nla_get_u32(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_PACKETS_RECEIVED]);
					printf("affiliated sta[%d] packets received=%u\n", i, tmp_u32);
				}

				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_ERRORS_SENT]) {
					tmp_u64 = nla_get_u64(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_ERRORS_SENT]);
					printf("affiliated sta[%d] errors send=%llu\n", i, tmp_u64);
				}
				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_RETRIES]) {
					tmp_u64 = nla_get_u64(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_RETRIES]);
					printf("affiliated sta[%d] retries send=%llu\n", i, tmp_u64);
				}
				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_SIGNAL_STRENGTH]) {
					tmp_s32 = nla_get_s32(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_SIGNAL_STRENGTH]);
					printf("affiliated sta[%d] signal strength=%d\n", i, tmp_s32);
				}

				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_DL_RATE]) {
					tmp_u32 = nla_get_u32(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_DL_RATE]);
					printf("affiliated sta[%d] est dl rate=%u\n", i, tmp_u32);
				}

				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_UL_RATE]) {
					tmp_u32 = nla_get_u32(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_EST_UL_RATE]);
					printf("affiliated sta[%d] est ul rate=%u\n", i, tmp_u32);
				}
				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_DL_RATE]) {
					tmp_u32 = nla_get_u32(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_DL_RATE]);
					printf("affiliated sta[%d] last dl rate=%u\n", i, tmp_u32);
				}
				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_UL_RATE]) {
					tmp_u32 = nla_get_u32(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_LAST_UL_RATE]);
					printf("affiliated sta[%d] last ul rate=%u\n", i, tmp_u32);
				}
				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_RECEIVE]) {
					tmp_u64 = nla_get_u64(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_RECEIVE]);
					printf("affiliated sta[%d] air time receive=%llu\n", i, tmp_u64);
				}
				if (affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_TRANSMIT]) {
					tmp_u64 = nla_get_u64(affiliated_stas[MTK_NL80211_VENDOR_ATTR_CONNECTED_STA_MLD_AFFILIATED_STA_AIR_TIME_TRANSMIT]);
					printf("affiliated sta[%d] air time send=%llu\n", i, tmp_u64);
				}
				i++;
			}
		}
	} else {
		printf("no connected sta mld get from driver\n");
	}
	printf("Connectd STA MLD dump end<================\n");
	return 0;
}

int handle_connected_sta_mld_dump(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	char *ptr = NULL, *param_str = NULL, *val_str = NULL, invalide = 0;
	int i, j;

	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	for (i = 0; i < argc; i++) {
		ptr = argv[i];
		param_str = strtok(ptr, "=");

		if (param_str == NULL)
			return -EINVAL;

		val_str = strtok(NULL, "=");

		for (j = 0; j < (sizeof(con_sta_mld_options) / sizeof(con_sta_mld_options[0])); j++) {
			if (strlen(con_sta_mld_options[j].option_name) == strlen(param_str) &&
				!strncmp(con_sta_mld_options[j].option_name, param_str, strlen(param_str)))
				break;
		}

		if (j != (sizeof(con_sta_mld_options) / sizeof(con_sta_mld_options[0]))) {
			if (con_sta_mld_options[j].attr_put(msg, val_str) < 0)
				printf("param_str %s att_put fail\n", param_str);
			else
				invalide = 1;
			continue;
		}
	}

	nla_nest_end(msg, data);

	if (!invalide)
		return -EINVAL;
	else
		register_handler(connected_sta_mld_dump_callback, NULL);

	return 0;
}


COMMAND(dump, mlo_info, NULL,
		MTK_NL80211_VENDOR_SUBCMD_SHOW_MLO_INFO, 0, CIB_NETDEV, handle_mlo_info_show,
		"Show apcli mlo link info.\n");

#define AP_MLD_DUMP_HELP_STR "[all(dump all mld groups)]\n"\
							 "[mac=xx:xx:xx:xx:xx:xx(dump mld groups of specific mac)]\n"\
							 "[index=<1-16>(dump mld groups of a specific index)]\n"

COMMAND(dump, ap_mld, AP_MLD_DUMP_HELP_STR,
		MTK_NL80211_VENDOR_SUBCMD_GET_AP_MLD, 0, CIB_NETDEV, handle_ap_mlo_dump,
		"dump ap_mld information\n");

COMMAND(dump, apcli_mld, NULL,
		MTK_NL80211_VENDOR_SUBCMD_GET_APCLI_MLD, 0, CIB_NETDEV, handle_apcli_mlo_dump,
		"dump apcli_mld information\n");

#define CONNECTED_STA_MLD_DUMP_HELP_STR "[all(dump all connected sta mld)]\n"\
							 "[index=<1-16>(dump connected sta mld of a ap mld with a specific index)]\n"
COMMAND(dump, con_sta_mld, CONNECTED_STA_MLD_DUMP_HELP_STR,
		MTK_NL80211_VENDOR_SUBCMD_GET_CONNECTED_STA_MLD, 0, CIB_NETDEV, handle_connected_sta_mld_dump,
		"dump connected sta mld information\n");

DECLARE_SECTION(set);
#define MAX_MLD_NUM 32

int
handle_set_ap_mld_index(struct nl_msg *msg,int argc, char** argv)
{
	unsigned long mld_group_idx;

	if (argc < 1)
		return -EINVAL;

	mld_group_idx = strtoul(argv[0], NULL, 10);
	if (mld_group_idx == ULONG_MAX && errno == ERANGE)
		return -EINVAL;

	if (mld_group_idx > MAX_MLD_NUM)
		return -EINVAL;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_INDEX, (u8)(mld_group_idx)))
		return -EMSGSIZE;

	return 1;
}

int
handle_set_ap_mld_addr(struct nl_msg *msg, int argc, char** argv)
{
	unsigned long mld_group_idx;
	u8 mld_addr[ETH_ALEN];
	void *data;

	if (argc < 2)
		return -EINVAL;

	mld_group_idx = strtoul(argv[0], NULL, 10);
	if (mld_group_idx == ULONG_MAX && errno == ERANGE)
		return -EINVAL;

	if (mld_group_idx > MAX_MLD_NUM)
		return -EINVAL;

	if (hwaddr_aton2(argv[1], mld_addr) < 0)
		return -EINVAL;

	data = nla_nest_start(msg, MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_ADDRESS);
	if (!data)
		return -EMSGSIZE;

	if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX, (u8)mld_group_idx))
		return -EMSGSIZE;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_AP_MLD_ADDRESS, ETH_ALEN, mld_addr))
		return -EMSGSIZE;

	nla_nest_end(msg, data);

	return 2;
}

int
handle_set_ap_mld_eml_mode(struct nl_msg *msg, int argc, char** argv)
{
        unsigned long mld_group_idx;
        void *data;

        if (argc < 2)
                return -EINVAL;

        mld_group_idx = strtoul(argv[0], NULL, 10);
        if (mld_group_idx == ULONG_MAX && errno == ERANGE)
                return -EINVAL;

        if (mld_group_idx > MAX_MLD_NUM)
                return -EINVAL;

        data = nla_nest_start(msg, MTK_NL80211_VENDOR_ATTR_SET_AP_MLD_EML_MODE);
        if (!data)
                return -EMSGSIZE;

        if (nla_put_u8(msg, MTK_NL80211_VENDOR_ATTR_AP_MLD_INDEX, (u8)mld_group_idx))
                return -EMSGSIZE;

	if (strcmp(argv[1], "emlsr") == 0) {
		if (nla_put_flag(msg, MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLSR))
                	return -EMSGSIZE;
	} else if (strcmp(argv[1], "emlmr") == 0) {
		if (nla_put_flag(msg, MTK_NL80211_VENDOR_ATTR_AP_MLD_EMLMR))
                	return -EMSGSIZE;
	} else if (strcmp(argv[1], "disable") != 0) {
		return -EINVAL;
	}

        nla_nest_end(msg, data);

        return 2;
}

struct attr_t {
	char *name;
	int (*func)(struct nl_msg *msg, int argc, char** argv);
};

int handle_attrs(struct nl_msg *msg,
		struct attr_t attrs[],
		int attrs_cnt,
		char *attr_name,
		int argc,
		char **argv)
{
	int i;

	for (i = 0; i < attrs_cnt; i++) {
		if (strcmp(attrs[i].name, attr_name) == 0) {
			return (*attrs[i].func)(msg,
						argc,
						argv);
		}
	}

	return -EINVAL;
}

struct attr_t set_ap_mld_attrs[] = {
	{
		.name = "index",
		.func = handle_set_ap_mld_index,
	},

	{
		.name = "addr",
		.func = handle_set_ap_mld_addr,
	},

	{
		.name = "eml_mode",
		.func = handle_set_ap_mld_eml_mode,
	},
};

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

int
handle_set_ap_mld(struct nl_msg *msg, int argc, char **argv, void *ctx)
{
	void *data;
	int i;
	int ret;

	if (!argc)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -EMSGSIZE;

	for (i = 0; i < argc; i++) {
		ret = handle_attrs(msg,
				set_ap_mld_attrs,
				ARRAY_SIZE(set_ap_mld_attrs),
				argv[i],
				argc - (i + 1),
				argv + (i + 1));
		if (ret < 0)
			return ret;

		i += ret;
	}

	nla_nest_end(msg, data);

	return 0;
}

#define SET_AP_MLD_HELP_STR  "[index <0-32>]\n [addr <mld index: 1-32> <mld_addr>]\n [eml_mode <mld index: 1-32> <disable|emlsr|emlmr]"

COMMAND(set, ap_mld, SET_AP_MLD_HELP_STR,
                MTK_NL80211_VENDOR_SUBCMD_SET_AP_MLD, 0, CIB_NETDEV, handle_set_ap_mld,
                "set AP mld\n");

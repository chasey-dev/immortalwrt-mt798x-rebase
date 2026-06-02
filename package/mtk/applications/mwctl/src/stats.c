/* Copyright (C) 2021 Mediatek Inc. */
#define _GNU_SOURCE

#include "mtk_vendor_nl80211.h"
#include "mt76-vendor.h"
#include "mwctl.h"

struct radio_stats {
	unsigned long BytesSent;
	unsigned long BytesReceived;
	unsigned long PacketsSent;
	unsigned long PacketsReceived;
	unsigned long ErrorsSent;
	unsigned long ErrorsReceived;
	unsigned long DiscardPacketsSent;
	unsigned long DiscardPacketsReceived;
	unsigned int PLCPErrorCount;
	unsigned int FCSErrorCount;
	unsigned int InvalidMACCount;
	unsigned int PacketsOtherReceived;
	unsigned long CtsReceived;
	unsigned long NoCtsReceived;
	unsigned long FrameHeaderError;
	unsigned long GoodPLCPReceived;
	unsigned long DPacketOtherMACReceived;
	unsigned long MPacketOtherMACReceived;
	unsigned long CPacketOtherMACReceived;
	unsigned long CtsOtherMACReceived;
	unsigned long RtsOtherMACReceived;
	unsigned int TotalChannelChangeCount;
	unsigned int ManualChannelChangeCount;
	unsigned int AutoStartupChannelChangeCount;
	unsigned int AutoUserChannelChangeCount;
	unsigned int AutoRefreshChannelChangeCount;
	unsigned int AutoDynamicChannelChangeCount;
	unsigned int AutoDFSChannelChangeCount;
	unsigned long UnicastPacketsSent;
	unsigned long UnicastPacketsReceived;
	unsigned long MulticastPacketsSent;
	unsigned long MulticastPacketsReceived;
	unsigned long BroadcastPacketsSent;
	unsigned long BroadcastPacketsReceived;
};

struct bss_stats {
	unsigned long BytesSent;
	unsigned long BytesReceived;
	unsigned long PacketsSent;
	unsigned long PacketsReceived;
	unsigned int ErrorsSent;
	unsigned int RetransCount;
	unsigned int FailedRetransCount;
	unsigned int RetryCount;
	unsigned int MultipleRetryCount;
	unsigned int ACKFailureCount;
	unsigned int AggregatedPacketCount;
	unsigned int ErrorsReceived;
	unsigned long UnicastPacketsSent;
	unsigned long UnicastPacketsReceived;
	unsigned int DiscardPacketsSent;
	unsigned int DiscardPacketsReceived;
	unsigned long MulticastPacketsSent;
	unsigned long MulticastPacketsReceived;
	unsigned long BroadcastPacketsSent;
	unsigned long BroadcastPacketsReceived;
	unsigned int UnknownProtoPacketsReceived;
	unsigned long DiscardPacketsSentBufOverflow;
	unsigned long DiscardPacketsSentNoAssoc;
	unsigned long FragSent;
	unsigned long SentNoAck;
	unsigned long DupReceived;
	unsigned long TooLongReceived;
	unsigned long TooShortReceived;
	unsigned long AckUcastReceived;
};

struct sta_txrx_stats {
	unsigned long BytesSent;
	unsigned long BytesReceived;
	unsigned long PacketsSent;
	unsigned long PacketsReceived;
	unsigned int ErrorsSent;
	unsigned int ErrorsReceived;
	unsigned int RetransCount;
	unsigned int FailedRetransCount;
	unsigned int RetryCount;
	unsigned int MultipleRetryCount;
};

struct sta_txrx_rates {
	unsigned int TxRate;
	unsigned int RxRate;
	unsigned int RxRate_rt;
	unsigned int TxRate_rt;
	unsigned int avg_tx_rate;
	unsigned int avg_rx_rate;
};

struct station {
	unsigned char MacAddr[ETH_ALEN];
	unsigned char VMacAddr[ETH_ALEN];
	unsigned short PhyMode;
	bool AuthenticationState;
	unsigned int LastConnectTime;
	signed short SignalStrength;
	signed short Noise;
	unsigned char Retransmissions;
	unsigned int UtilizationReceive;
	unsigned int UtilizationTransmit;
	struct sta_txrx_stats stats;
	struct sta_txrx_rates rates;
	bool repeater;
	bool valid;
};

struct wifi_sta_info {
        unsigned char MacAddr[MAC_ADDR_LEN];
        unsigned short PhyMode;
        bool AuthenticationState;
        unsigned int LastConnectTime;
        signed short SignalStrength;
        signed short Noise;
        unsigned long TxBytes;
        unsigned long RxBytes;
        unsigned long AvgTxBytes;
        unsigned long AvgRxBytes;
        unsigned long OneSecTxBytes;
        unsigned long OneSecRxBytes;
        unsigned char XtraRange;
        unsigned char BW;
        unsigned char Nss;
        unsigned char McsRate;
        unsigned int tx_latency_max;
        unsigned int tx_latency_min;
        unsigned int tx_latency_max_t2;
        unsigned int tx_latency_min_t2;
        unsigned int air_latency_max;
        unsigned int air_latency_min;
        unsigned int air_latency_max_t2;
        unsigned int air_latency_min_t2;
        bool valid;
};

DECLARE_SECTION(dump);

int radio_stat_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_RADIO_STATS_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct radio_stats *stat;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_RADIO_STATS_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_RADIO_STATS]) {
			stat = (struct radio_stats*)nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_RADIO_STATS]);
			printf("ByteSent %ld ByteReceived %ld\n",stat->BytesSent,stat->BytesReceived);
		}
	} else
		printf("No Stats from driver\n");

	return 0;
}

int radio_stats_handle(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	u8 stats[4];

	if (argc > 1)
		return -EINVAL;

	memset(stats, 0, 4);
	register_handler(radio_stat_callback, NULL);

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_RADIO_STATS, 0, stats))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(dump, radio_stats, "",
	MTK_NL80211_VENDOR_SUBCMD_GET_RADIO_STATS, 0, CIB_NETDEV, radio_stats_handle,
	"This command is used to get radio stats");

int bss_stat_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_BSS_STATS_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct bss_stats *stat;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_BSS_STATS_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_BSS_STATS]) {
			stat = (struct bss_stats*)nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_BSS_STATS]);
			printf("ByteSent %ld ByteReceived %ld\n",stat->BytesSent,stat->BytesReceived);
		}
	} else
		printf("No Stats from driver\n");

	return 0;
}

int bss_stats_handle(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	u8 stats[4];
	int ret_offset = 0;

	if (argc > 2)
		return -EINVAL;

	memset(stats, 0, 4);
	register_handler(bss_stat_callback, NULL);

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	ret_offset = add_msg_band_link_id(msg, get_band_link_id(),
		MTK_NL80211_VENDOR_ATTR_BSS_STATS_BAND_LINK_ID);
	if (ret_offset < 0)
		return ret_offset;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_BSS_STATS, 0, stats))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(dump, bss_stats, "",
	MTK_NL80211_VENDOR_SUBCMD_GET_BSS_STATS, 0, CIB_NETDEV, bss_stats_handle,
	"This command is used to get BSS stats");

int sta_stat_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct station *stat;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_STA]) {
			stat = (struct station*)nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_STA]);
			printf("ByteSent %ld ByteReceived %ld\n",stat->stats.BytesSent,stat->stats.BytesReceived);
		}
	} else
		printf("No Stats from driver\n");

	return 0;
}

int sta_stats_handle(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	u8 Addr[ETH_ALEN];
	int matches;
	char *token;

	if (!argc || argc != 1)
		return -EINVAL;

	register_handler(sta_stat_callback, NULL);

	for (matches = 0, token = strtok(argv[0], ":"); token != NULL && (matches < ETH_ALEN); token = strtok(NULL, ":")) {
		Addr[matches++] = strtol(token, NULL, 16);
	}

	if (matches != ETH_ALEN)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_STA, ETH_ALEN, Addr))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(dump, sta_stats, "<mac_addr>(xx:xx:xx:xx:xx:xx)",
	MTK_NL80211_VENDOR_SUBCMD_GET_STA, 0, CIB_NETDEV, sta_stats_handle,
	"This command is used to get station stats");

static double bytes_to_mbps(unsigned long bytes) {
	unsigned long bits = bytes * 8;

	/* 1024*1024 = 1048576*/
	double mbps = (double)bits / (1048576);
	return mbps;
}

#define BW_ARRAY_MAX 6
static char *bw_to_string(unsigned char bandwidth){
	char *bw_str[BW_ARRAY_MAX] = {"20M", "40M", "80M", "160M", "320M", "N/A"};

	if (bandwidth < BW_ARRAY_MAX)
		return bw_str[bandwidth];

	return bw_str[BW_ARRAY_MAX-1];
}

int gtb_sta_stat_callback(struct nl_msg *msg, void *cb)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *vndr_tb[MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct wifi_sta_info *stat;
	int err = 0;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(vndr_tb, MTK_NL80211_VENDOR_ATTR_STA_ATTR_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (vndr_tb[MTK_NL80211_VENDOR_ATTR_STA_INFO_GTBAPP_FORMAT]) {
			stat = (struct wifi_sta_info*)nla_data(vndr_tb[MTK_NL80211_VENDOR_ATTR_STA_INFO_GTBAPP_FORMAT]);
			printf("[%d][%d] RSSI %d, LAT1 %5u(ms) LAT2 %5u(ms), AIR_LAT1 %5u(ms) AIR_LAT2 %5u(ms), TX %7.2f Mbps, RX %7.2f Mbps, BW %s, %dS-M%d\n",
				stat->valid, stat->XtraRange, stat->SignalStrength,
				stat->tx_latency_max, stat->tx_latency_max_t2,
				stat->air_latency_max, stat->air_latency_max_t2,
				bytes_to_mbps(stat->OneSecTxBytes), bytes_to_mbps(stat->OneSecRxBytes),
				bw_to_string(stat->BW), stat->Nss, stat->McsRate);
		}
	} else
		printf("No Stats from driver\n");

	return 0;
}

int gtb_sta_stats_handle(struct nl_msg *msg, int argc,
	char **argv, void *ctx)
{
	void *data;
	u8 Addr[ETH_ALEN];
	int matches;
	char *token;

	if (!argc || argc != 1)
		return -EINVAL;

	register_handler(gtb_sta_stat_callback, NULL);

	for (matches = 0, token = strtok(argv[0], ":"); token != NULL && (matches < ETH_ALEN); token = strtok(NULL, ":")) {
		Addr[matches++] = strtol(token, NULL, 16);
	}

	if (matches != ETH_ALEN)
		return -EINVAL;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -ENOMEM;

	if (nla_put(msg, MTK_NL80211_VENDOR_ATTR_STA_INFO_GTBAPP_FORMAT, ETH_ALEN, Addr))
		return -EMSGSIZE;

	nla_nest_end(msg, data);
	return 0;
}

COMMAND(dump, gtb_sta_stats, "<mac_addr>(xx:xx:xx:xx:xx:xx)",
	MTK_NL80211_VENDOR_SUBCMD_GET_STA, 0, CIB_NETDEV, gtb_sta_stats_handle,
	"This command is used to get station stats");


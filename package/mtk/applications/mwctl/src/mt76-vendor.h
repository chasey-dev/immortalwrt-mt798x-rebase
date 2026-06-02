// SPDX-License-Identifier: ISC
/* Copyright (C) 2020 Felix Fietkau <nbd@nbd.name> */
#ifndef __MT76_VENDOR_H
#define __MT76_VENDOR_H

#include <errno.h>
#include <linux/nl80211.h>
#include <netlink/attr.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unl.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64, ktime_t;

#define MTK_NL80211_VENDOR_ID	0x0ce7

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

struct nl_msg;
struct nlattr;

enum mtk_vendor_attr_csi_ctrl {
	MTK_VENDOR_ATTR_CSI_CTRL_UNSPEC,

	MTK_VENDOR_ATTR_CSI_CTRL_CFG,
	MTK_VENDOR_ATTR_CSI_CTRL_CFG_MODE,
	MTK_VENDOR_ATTR_CSI_CTRL_CFG_TYPE,
	MTK_VENDOR_ATTR_CSI_CTRL_CFG_VAL1,
	MTK_VENDOR_ATTR_CSI_CTRL_CFG_VAL2,
	MTK_VENDOR_ATTR_CSI_CTRL_MAC_ADDR,
	MTK_VENDOR_ATTR_CSI_CTRL_INTERVAL,

	MTK_VENDOR_ATTR_CSI_CTRL_DUMP_NUM,

	MTK_VENDOR_ATTR_CSI_CTRL_DATA,

	/* keep last */
	NUM_MTK_VENDOR_ATTRS_CSI_CTRL,
	MTK_VENDOR_ATTR_CSI_CTRL_MAX =
		NUM_MTK_VENDOR_ATTRS_CSI_CTRL - 1
};

enum mtk_vendor_attr_csi_data {
	MTK_VENDOR_ATTR_CSI_DATA_UNSPEC,
	MTK_VENDOR_ATTR_CSI_DATA_PAD,

	MTK_VENDOR_ATTR_CSI_DATA_VER,
	MTK_VENDOR_ATTR_CSI_DATA_TS,
	MTK_VENDOR_ATTR_CSI_DATA_RSSI,
	MTK_VENDOR_ATTR_CSI_DATA_SNR,
	MTK_VENDOR_ATTR_CSI_DATA_BW,
	MTK_VENDOR_ATTR_CSI_DATA_CH_IDX,
	MTK_VENDOR_ATTR_CSI_DATA_TA,
	MTK_VENDOR_ATTR_CSI_DATA_I,
	MTK_VENDOR_ATTR_CSI_DATA_Q,
	MTK_VENDOR_ATTR_CSI_DATA_INFO,
	MTK_VENDOR_ATTR_CSI_DATA_RSVD1,
	MTK_VENDOR_ATTR_CSI_DATA_RSVD2,
	MTK_VENDOR_ATTR_CSI_DATA_RSVD3,
	MTK_VENDOR_ATTR_CSI_DATA_RSVD4,
	MTK_VENDOR_ATTR_CSI_DATA_TX_ANT,
	MTK_VENDOR_ATTR_CSI_DATA_RX_ANT,
	MTK_VENDOR_ATTR_CSI_DATA_MODE,
	MTK_VENDOR_ATTR_CSI_DATA_H_IDX,

	/* keep last */
	NUM_MTK_VENDOR_ATTRS_CSI_DATA,
	MTK_VENDOR_ATTR_CSI_DATA_MAX =
		NUM_MTK_VENDOR_ATTRS_CSI_DATA - 1
};

enum mtk_vendor_attr_mnt_ctrl {
	MTK_VENDOR_ATTR_AMNT_CTRL_UNSPEC,

	MTK_VENDOR_ATTR_AMNT_CTRL_SET,
	MTK_VENDOR_ATTR_AMNT_CTRL_DUMP,
	/* keep last */
	NUM_MTK_VENDOR_ATTRS_AMNT_CTRL,
	MTK_VENDOR_ATTR_AMNT_CTRL_MAX =
		NUM_MTK_VENDOR_ATTRS_AMNT_CTRL - 1
};

enum mtk_vendor_attr_mnt_set {
	MTK_VENDOR_ATTR_AMNT_SET_UNSPEC,

	MTK_VENDOR_ATTR_AMNT_SET_INDEX,
	MTK_VENDOR_ATTR_AMNT_SET_MACADDR,

	/* keep last */
	NUM_MTK_VENDOR_ATTRS_AMNT_SET,
	MTK_VENDOR_ATTR_AMNT_SET_MAX =
		NUM_MTK_VENDOR_ATTRS_AMNT_SET - 1
};

enum mtk_vendor_attr_mnt_dump {
	MTK_VENDOR_ATTR_AMNT_DUMP_UNSPEC,

	MTK_VENDOR_ATTR_AMNT_DUMP_INDEX,
	MTK_VENDOR_ATTR_AMNT_DUMP_LEN,
	MTK_VENDOR_ATTR_AMNT_DUMP_RESULT,

	/* keep last */
	NUM_MTK_VENDOR_ATTRS_AMNT_DUMP,
	MTK_VENDOR_ATTR_AMNT_DUMP_MAX =
		NUM_MTK_VENDOR_ATTRS_AMNT_DUMP - 1
};


#define CSI_MAX_COUNT 256
#define ETH_ALEN 6

struct csi_data {
	s16 data_i[CSI_MAX_COUNT];
	s16 data_q[CSI_MAX_COUNT];
	s8 rssi;
	u8 snr;
	u32 ts;
	u8 data_bw;
	u8 pri_ch_idx;
	u8 ta[ETH_ALEN];
	u32 info;
	u8 rx_mode;
	u32 h_idx;
	u16 tx_idx;
	u16 rx_idx;
};

struct amnt_data {
	u8 idx;
	u8 addr[ETH_ALEN];
	s8 rssi[4];
	u32 last_seen;
};

extern struct unl unl;

int mt76_csi_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx);
int mt76_csi_dump(struct nl_msg *nlmsg, int argc,
	char **argv, void *ctx);

int mt76_amnt_set(struct nl_msg *msg, int argc,
	char **argv, void *ctx);
int mt76_amnt_dump(struct nl_msg *msg, int argc,
	char **argv, void *ctx);

int handle_set_command(struct nl_msg *msg, int argc,
	char **argv, void *ctx);

#endif

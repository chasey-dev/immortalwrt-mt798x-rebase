/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2014-2016 Zhiqiang Yang <zhiqiang.yang@mediatek.com>
 */

#ifndef NF_HNAT_MCAST_H
#define NF_HNAT_MCAST_H

#define RTMGRP_IPV4_MROUTE 0x20
#define RTMGRP_MDB 0x2000000

#define MAX_MCAST_ENTRY 64
#define MAX_MCAST_PORT	5

#define INVLD_IDX		(-1)
#define HNAT_NPU_ENTRY_MC	(30)
#define FOE_TS_WRAP_THRESHOLD	(0x3000)
#define FOE_TS_MAX_DIFF		(20)

struct ppe_mcast_list {
	struct list_head list;  /* Protected by rwlock */
	u16 vid; /* vlan id */
	u8 dmac[ETH_ALEN]; /* multicast mac addr */
	u8 mc_port; /* multicast port */
};

struct ppe_mcast_member {
	struct list_head list;
	u32 ifindex;
};

struct ppe_mcast_group {
	u8 mac[ETH_ALEN];
	struct list_head members;
	int mtbl_idx;
	int ppe_id;
	int foe_idx;
	struct list_head list;
	u32 psebmp;
	__be32 dip;	/*stream dip*/
	__be32 sip;	/*server ip*/
	struct in6_addr dip6;
	struct in6_addr sip6;
	bool offload;
	bool is_ipv4;
	u32 npu_grp_idx;
};

struct ppe_mcast_table {
	struct workqueue_struct *queue;
	struct work_struct work;
	struct socket *msock;
	struct list_head groups;
	u8 max_entry;
	rwlock_t mcast_lock; /* Protect list and mc_port field */
};

struct ppe_mcast_h {
	union {
		u32 value;
		struct {
			u32 mc_vid : 12;
			u32 mc_qos_qid64 : 3;
			u32 mc_px_en : 5;
			u32 mc_mpre_sel : 2; /* 0=01:00, 1=33:33 */
			u32 mc_vid_cmp: 1;
			u32 mc_p4_q : 1;
			u32 mc_p0_q : 1;
			u32 mc_p1_q : 1;
			u32 mc_p2_q : 1;
			u32 mc_p3_q : 1;
			u32 mc_qos_qid : 4;
		} info;
	} u;
};

struct ppe_mcast_l {
	u32 addr;
};

enum ppe_mcast_port {
	MCAST_TO_GDMA3,
	MCAST_TO_PDMA,
	MCAST_TO_GDMA1,
	MCAST_TO_GDMA2,
	MCAST_TO_TDMA,
};

enum hnat_mcast_mode {
	HNAT_MCAST_MODE_MULTI = 0,
	HNAT_MCAST_MODE_UNI,
};

struct mcast_offload_info {
	union {
		__be32 ip;
		struct in6_addr ipv6;
	} dst;
	union {
		__be32 ip;
		struct in6_addr ipv6;
	} src;
	u32 dest;
	u8 daddr[ETH_ALEN];
	u8 pqid;
	u8 dsa_port;
	bool is_ipv6;	/* ipv4 or ipv6 */
	bool m2u_en;
	u32 npu_grp_idx;
};

enum hnat_mcast_info_id {
	HNAT_MCAST_INFO_IP = 0,
	HNAT_MCAST_INFO_MAC = 1,
};

struct hnat_mcast_group_ip {
	bool is_ipv4;
	union {
		__be32 ip;
		struct in6_addr ipv6;
	} addr;
};

struct hnat_mcast_group_info {
	int grp_id;
	struct hnat_mcast_group_ip gip;
	u8 grp_mac[ETH_ALEN];
	u8 *extr_info;
};

enum hnat_npu_mcast_dest {
	HNAT_NPU_MCAST_DEST_SWITCH = 0,
	HNAT_NPU_MCAST_DEST_LAN,
	HNAT_NPU_MCAST_DEST_MAX,
};

/* int (*npu_mcast_client_insert)
 * -EINVAL:mac address is zero or not multicst address
 * -EPERM :dip is zero or dip is not multicast ip or dip == sip
 * -ENOMEM: No available multicast group or alloc source fail
 * -EEXIST: client has been added before
 * int (*npu_mcast_client_delete)
 * -EINVAL: parameter is null or ip is zero, or ip is not mc ip or dsa-port > 3
 * -EPERM :dip is zero or dip is not mc ip or dip == sip or npu fw write fail
 * -ENOENT:mc group not found or client not found
 */
struct npu_hnat_mcast_ops {
	int (*npu_mcast_client_insert)(struct mcast_offload_info *hnat_mcast_params);
	int (*npu_mcast_client_delete)(struct mcast_offload_info *hnat_mcast_params);
	int (*npu_mcast_client_update)(struct mcast_offload_info *hnat_mcast_params);
};

#define IS_MCAST_MULTI_MODE (hnat_priv->data->mcast && mcast_mode == HNAT_MCAST_MODE_MULTI)
#define IS_MCAST_UNI_MODE (hnat_priv->data->mcast && mcast_mode == HNAT_MCAST_MODE_UNI)
#define IS_MCAST_PORT_GDM(port)							\
	(port == (1 << MCAST_TO_GDMA1) || port == (1 << MCAST_TO_GDMA2) ||	\
	 port == (1 << MCAST_TO_GDMA3))
#define DMAC_TO_HI16(dmac) ((dmac[0] << 8) | dmac[1])
#define DMAC_TO_LO32(dmac) ((dmac[2] << 24) | (dmac[3] << 16) | (dmac[4] << 8) | dmac[5])

int hnat_mcast_ser_handle(void);
int hnat_mcast_mcport_ppse_map_init(u32 ppe_id);
int hnat_mcast_enable(u32 ppe_id);
int hnat_mcast_disable(void);
void hnat_mcast_ifdown_handle(int ifindex);
int mtk_npu_hnat_mcast_ops_register(struct npu_hnat_mcast_ops *hnat_mcast_ops);
int mtk_npu_hnat_mcast_ops_unregister(void);
void hnat_notify_mcast_sw_path(int group_idx);
void hnat_notify_mcast_hw_path(int group_idx);
int hnat_mcast_blist_handle(u8 *ip, u32 mask, bool add, bool is_ipv4);
int hnat_mcast_offload_handle(bool enable);
int hnat_register_pcie_link_hook(bool (*hook_func)(bool up));
void hnat_unregister_pcie_link_hook(void);
extern bool (*hnat_mcast_eth_ext_link_handle)(bool up);
int hnat_register_wifi_info_hook(int (*hook_func)(u32 id, void *data));
void hnat_unregister_wifi_info_hook(void);

#endif

/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2014-2016 Zhiqiang Yang <zhiqiang.yang@mediatek.com>
 */

#ifndef NF_HNAT_MCAST_H
#define NF_HNAT_MCAST_H

#define RTMGRP_IPV4_MROUTE 0x20
#define RTMGRP_MDB 0x2000000

#define MAX_MCAST_ENTRY 64
#define MAX_MCAST_PORT	5

struct ppe_mcast_list {
	struct list_head list;  /* Protected by rwlock */
	u16 vid; /* vlan id */
	u8 dmac[ETH_ALEN]; /* multicast mac addr */
	u8 mc_port; /* multicast port */
};

struct ppe_mcast_group {
	u16 vid;
	u8 dmac[ETH_ALEN]; /* multicast mac addr */
	u8 if_num[MAX_MCAST_PORT]; /* num of if added to multi group. */
	u8 mc_port; /* 1:forward to cpu,2:forward to GDMA1,3:forward to GDMA2 */
	bool valid;
};

struct ppe_mcast_table {
	struct workqueue_struct *queue;
	struct work_struct work;
	struct socket *msock;
	struct ppe_mcast_group mtbl[MAX_MCAST_ENTRY];
	struct list_head mlist;
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
			u32 mc_px_qos_en : 5;
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
	MCAST_TO_QDMA,
};

enum hnat_mcast_mode {
	HNAT_MCAST_MODE_MULTI = 0,
	HNAT_MCAST_MODE_UNI,
};

#define IS_MCAST_MULTI_MODE (hnat_priv->data->mcast && mcast_mode == HNAT_MCAST_MODE_MULTI)
#define IS_MCAST_UNI_MODE (hnat_priv->data->mcast && mcast_mode == HNAT_MCAST_MODE_UNI)
#define IS_MCAST_PORT_GDM(port)							\
	(port == (1 << MCAST_TO_GDMA1) || port == (1 << MCAST_TO_GDMA2) ||	\
	 port == (1 << MCAST_TO_GDMA3))
#define DMAC_TO_HI16(dmac) ((dmac[0] << 8) | dmac[1])
#define DMAC_TO_LO32(dmac) ((dmac[2] << 24) | (dmac[3] << 16) | (dmac[4] << 8) | dmac[5])

int hnat_mcast_enable(u32 ppe_id);
int hnat_mcast_disable(void);
bool hnat_is_mcast_uni(struct sk_buff *skb);

#endif

/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2024 MediaTek Inc.
 * Author: Chak-kei Lam <chak-kei.lam@mediatek.com>
 */

#ifndef NF_HNAT_API_H
#define NF_HNAT_API_H

#include "hnat.h"

enum hnat_status {
	HNAT_ALREADY_SET = 1,
	HNAT_SUCCESS = 0,
	HNAT_FAIL = -1,
	HNAT_ENTRY_NOT_FOUND = -2,
};

struct hnat_tuple {
	unsigned short ppe_index;
	unsigned short hash_index;
	unsigned short ttl;
	unsigned short psn;
	unsigned short fqos;
	unsigned short qid;
	unsigned short vlan_layer;
	unsigned short dscp;
	unsigned short pkt_type;
	unsigned short is_udp;
	unsigned short fdr_en; /* force mode for DMAD ring index by rxid */
	unsigned short rxid; /* DMA ring index */
	unsigned short fp; /* PSE force port, 0: ADMA(CPU) */
	unsigned short tid;
	unsigned short is_prior;

	/* egress layer2 */
	unsigned char dmac[6];
	unsigned char smac[6];
	unsigned short vlan1;
	unsigned short vlan2;
	unsigned short pppoe_id;

	/* ingress layer3 */
	unsigned int ing_sipv4;
	unsigned int ing_dipv4;

	unsigned int ing_sipv6_0;
	unsigned int ing_sipv6_1;
	unsigned int ing_sipv6_2;
	unsigned int ing_sipv6_3;
	unsigned int ing_dipv6_0;
	unsigned int ing_dipv6_1;
	unsigned int ing_dipv6_2;
	unsigned int ing_dipv6_3;

	/* egress layer3 */
	unsigned int eg_sipv4;
	unsigned int eg_dipv4;

	unsigned int eg_sipv6_0;
	unsigned int eg_sipv6_1;
	unsigned int eg_sipv6_2;
	unsigned int eg_sipv6_3;
	unsigned int eg_dipv6_0;
	unsigned int eg_dipv6_1;
	unsigned int eg_dipv6_2;
	unsigned int eg_dipv6_3;

	/*ingress layer4*/
	unsigned short ing_sp;
	unsigned short ing_dp;

	/*egress layer4*/
	unsigned short eg_sp;
	unsigned short eg_dp;

	unsigned char ing_dev[16]; /* netdev name of ingress */
	unsigned char eg_dev[16]; /* netdev name of egress */
};

extern void (*hnat_bind_callback)(struct hnat_tuple *opt);
extern void (*hnat_fin_callback)(struct hnat_tuple *opt);

int mtk_hnat_get_ppe_num(void);
int mtk_hnat_get_ppe_entry_num(void);
int mtk_hnat_get_ppe_entry_by_index(struct hnat_tuple *opt);
int mtk_hnat_calc_ppe_hash_index_by_tuple(struct hnat_tuple *opt);
int mtk_hnat_delete_entry_by_index(unsigned short ppe_index,
				   unsigned short hash_index);
int mtk_hnat_get_mib_count_by_index(unsigned short ppe_index,
				    unsigned short hash_index,
				    unsigned long long *pkt_cnt,
				    unsigned long long *byte_cnt);
int mtk_hnat_get_all_mib_counts(unsigned long long **pkt_cnts,
			       unsigned long long **byte_cnts);
int mtk_hnat_update_hqos_by_index(unsigned short ppe_index,
				  unsigned short hash_index,
				  unsigned short fqos,
				  unsigned short qid);
int mtk_hnat_update_hqos_by_skb(struct sk_buff *skb,
				unsigned short fqos,
				unsigned short qid);
int mtk_hnat_update_dscp_by_index(unsigned short ppe_index,
				  unsigned short hash_index,
				  int dscp);
int mtk_hnat_update_dscp_by_skb(struct sk_buff *skb, int dscp);
int mtk_hnat_update_vlan_by_index(unsigned short ppe_index,
				  unsigned short hash_index,
				  unsigned short vlan1,
				  unsigned short vlan2);
int mtk_hnat_update_vlan_by_skb(struct sk_buff *skb,
				unsigned short vlan1,
				unsigned short vlan2);
int mtk_hnat_update_pppoe_by_index(unsigned short ppe_index,
				   unsigned short hash_index,
				   unsigned short psn,
				   unsigned short pppoe_id);
int mtk_hnat_update_pppoe_by_skb(struct sk_buff *skb,
				 unsigned short psn,
				 unsigned short pppoe_id);
int mtk_hnat_update_is_prior_by_index(unsigned short ppe_index,
				      unsigned short hash_index,
				      unsigned short is_prior);
int mtk_hnat_update_is_prior_by_skb(struct sk_buff *skb,
				    unsigned short is_prior);
int mtk_hnat_update_tid_by_index(unsigned short ppe_index,
				 unsigned short hash_index,
				 unsigned short tid);
int mtk_hnat_update_tid_by_skb(struct sk_buff *skb, unsigned short tid);
int mtk_hnat_register_bind_callback(void (*func)(struct hnat_tuple *));
int mtk_hnat_register_fin_callback(void (*func)(struct hnat_tuple *));
int mtk_hnat_get_fin_age_config(unsigned short ppe_index);
int mtk_hnat_set_fin_age_config(unsigned short ppe_index, bool enable);

void hnat_trigger_callback(void (*func)(struct hnat_tuple *), struct sk_buff *skb);
void hnat_api_init_debugfs(struct dentry *root);

#define IS_HNAT_API_SUPPORTED(x)	\
	(IS_IPV4_HNAPT(x) || IS_IPV6_5T_ROUTE(x))

int hnat_tuple_detail(struct hnat_tuple *opt);
void hnat_bind_callback_test(struct hnat_tuple *opt);
void hnat_fin_callback_test(struct hnat_tuple *opt);

#endif /* NF_HNAT_API_H */

/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2024 MediaTek Inc.
 * Author: Chak-kei Lam <chak-kei.lam@mediatek.com>
 */

#include <linux/debugfs.h>

#include "hnat.h"
#include "hnat_api.h"
#include "nf_hnat_mtk.h"

void (*hnat_bind_callback)(struct hnat_tuple *opt) = NULL;
void (*hnat_fin_callback)(struct hnat_tuple *opt) = NULL;

/* Fill PPE info1 and info2 from foe entry to hnat_tuple*/
static void mtk_hnat_foe_to_tuple_info_blk(struct foe_entry *entry, struct hnat_tuple *opt)
{
	/* Info 1 */
	opt->is_udp = entry->bfib1.udp;
	opt->pkt_type = entry->bfib1.pkt_type;
	opt->vlan_layer = entry->bfib1.vlan_layer;

	opt->psn = entry->bfib1.psn;
	opt->ttl = entry->bfib1.ttl;

	/* Info 2 */
	switch (entry->bfib1.pkt_type) {
	case IPV4_HNAPT:
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
		opt->fqos = (entry->ipv4_hnapt.tport_id == NR_QDMA_TPORT) ? 1 : 0;
#else
		opt->fqos = entry->ipv4_hnapt.iblk2.fqos;
#endif
		opt->qid = entry->ipv4_hnapt.iblk2.qid;
		opt->dscp = entry->ipv4_hnapt.iblk2.dscp;
		opt->fdr_en = entry->ipv4_hnapt.iblk2.winfoi;
		opt->rxid = entry->ipv4_hnapt.iblk2.rxid;
		opt->fp = entry->ipv4_hnapt.iblk2.dp;
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
		opt->tid = entry->ipv4_hnapt.winfo_pao.tid;
		opt->is_prior = entry->ipv4_hnapt.winfo_pao.is_prior;
#endif
		break;
	case IPV6_5T_ROUTE:
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
		opt->fqos = (entry->ipv6_5t_route.tport_id == NR_QDMA_TPORT) ? 1 : 0;
#else
		opt->fqos = entry->ipv6_5t_route.iblk2.fqos;
#endif
		opt->qid = entry->ipv6_5t_route.iblk2.qid;
		opt->dscp = entry->ipv6_5t_route.iblk2.dscp;
		opt->fdr_en = entry->ipv6_5t_route.iblk2.winfoi;
		opt->rxid = entry->ipv6_5t_route.iblk2.rxid;
		opt->fp = entry->ipv6_5t_route.iblk2.dp;
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
		opt->tid = entry->ipv6_5t_route.winfo_pao.tid;
		opt->is_prior = entry->ipv6_5t_route.winfo_pao.is_prior;
#endif
		break;
	default:
		break;
	}
}

/* Fill ethernet frame header info from foe entry to hnat_tuple */
static void mtk_hnat_foe_to_tuple_l2(struct foe_entry *entry, struct hnat_tuple *opt)
{
	u32 dmac_hi, smac_hi;
	u16 dmac_lo, smac_lo;

	if (IS_IPV4_GRP(entry)) {
		dmac_hi = swab32(entry->ipv4_hnapt.dmac_hi);
		dmac_lo = swab16(entry->ipv4_hnapt.dmac_lo);
		smac_hi = swab32(entry->ipv4_hnapt.smac_hi);
		smac_lo = swab16(entry->ipv4_hnapt.smac_lo);
		opt->vlan1 = entry->ipv4_hnapt.vlan1;
		opt->vlan2 = entry->ipv4_hnapt.vlan2;
		opt->pppoe_id = entry->ipv4_hnapt.pppoe_id;
	} else {
		dmac_hi = swab32(entry->ipv6_5t_route.dmac_hi);
		dmac_lo = swab16(entry->ipv6_5t_route.dmac_lo);
		smac_hi = swab32(entry->ipv6_5t_route.smac_hi);
		smac_lo = swab16(entry->ipv6_5t_route.smac_lo);
		opt->vlan1 = entry->ipv6_5t_route.vlan1;
		opt->vlan2 = entry->ipv6_5t_route.vlan2;
		opt->pppoe_id = entry->ipv6_5t_route.pppoe_id;
	}

	memcpy(opt->dmac, &dmac_hi, 4);
	memcpy(&opt->dmac[4], &dmac_lo, 2);
	memcpy(opt->smac, &smac_hi, 4);
	memcpy(&opt->smac[4], &smac_lo, 2);
}

/* Fill IP/IPv6 header info from foe entry to hnat_tuple */
static void mtk_hnat_foe_to_tuple_l3(struct foe_entry *entry, struct hnat_tuple *opt)
{
	switch (entry->bfib1.pkt_type) {
	case IPV4_HNAPT:
		opt->ing_sipv4 = htonl(entry->ipv4_hnapt.sip);
		opt->ing_dipv4 = htonl(entry->ipv4_hnapt.dip);
		opt->eg_sipv4 = htonl(entry->ipv4_hnapt.new_sip);
		opt->eg_dipv4 = htonl(entry->ipv4_hnapt.new_dip);
		break;
	case IPV6_5T_ROUTE:
		opt->ing_sipv6_0 = entry->ipv6_5t_route.ipv6_sip0;
		opt->ing_sipv6_1 = entry->ipv6_5t_route.ipv6_sip1;
		opt->ing_sipv6_2 = entry->ipv6_5t_route.ipv6_sip2;
		opt->ing_sipv6_3 = entry->ipv6_5t_route.ipv6_sip3;
		opt->ing_dipv6_0 = entry->ipv6_5t_route.ipv6_dip0;
		opt->ing_dipv6_1 = entry->ipv6_5t_route.ipv6_dip1;
		opt->ing_dipv6_2 = entry->ipv6_5t_route.ipv6_dip2;
		opt->ing_dipv6_3 = entry->ipv6_5t_route.ipv6_dip3;

		opt->eg_sipv6_0 = entry->ipv6_5t_route.ipv6_sip0;
		opt->eg_sipv6_1 = entry->ipv6_5t_route.ipv6_sip1;
		opt->eg_sipv6_2 = entry->ipv6_5t_route.ipv6_sip2;
		opt->eg_sipv6_3 = entry->ipv6_5t_route.ipv6_sip3;
		opt->eg_dipv6_0 = entry->ipv6_5t_route.ipv6_dip0;
		opt->eg_dipv6_1 = entry->ipv6_5t_route.ipv6_dip1;
		opt->eg_dipv6_2 = entry->ipv6_5t_route.ipv6_dip2;
		opt->eg_dipv6_3 = entry->ipv6_5t_route.ipv6_dip3;
		break;
	default:
		break;
	}
}

/* Fill TCP/UDP header info from foe entry to hnat_tuple */
static void mtk_hnat_foe_to_tuple_l4(struct foe_entry *entry, struct hnat_tuple *opt)
{
	switch (entry->bfib1.pkt_type) {
	case IPV4_HNAPT:
		opt->ing_sp = entry->ipv4_hnapt.sport;
		opt->ing_dp = entry->ipv4_hnapt.dport;
		opt->eg_sp = entry->ipv4_hnapt.new_sport;
		opt->eg_dp = entry->ipv4_hnapt.new_dport;
		break;
	case IPV6_5T_ROUTE:
		opt->ing_sp = entry->ipv6_5t_route.sport;
		opt->ing_dp = entry->ipv6_5t_route.dport;
		opt->eg_sp = entry->ipv6_5t_route.sport;
		opt->eg_dp = entry->ipv6_5t_route.dport;
		break;
	default:
		break;
	}
}

static int mtk_hnat_foe_to_hnat_tuple(struct foe_entry *hw_entry, struct hnat_tuple *opt)
{
	struct foe_entry entry = {0};

	memcpy(&entry, hw_entry, sizeof(entry));

	/* Transform foe_entry to hnat_tuple */
	mtk_hnat_foe_to_tuple_info_blk(&entry, opt); /* Fill in PPE info1 & info2 */
	/* Fill in ethernet header info */
	mtk_hnat_foe_to_tuple_l2(&entry, opt);
	mtk_hnat_foe_to_tuple_l3(&entry, opt);
	mtk_hnat_foe_to_tuple_l4(&entry, opt);

	return HNAT_SUCCESS;
}

int mtk_hnat_get_ppe_num(void)
{
	return CFG_PPE_NUM;
}
EXPORT_SYMBOL(mtk_hnat_get_ppe_num);

int mtk_hnat_get_ppe_entry_num(void)
{
	return (int)hnat_priv->foe_etry_num;
}
EXPORT_SYMBOL(mtk_hnat_get_ppe_entry_num);

int mtk_hnat_get_ppe_entry_by_index(struct hnat_tuple *opt)
{
	struct foe_entry *entry;
	struct mtk_hnat *h = hnat_priv;
	unsigned short ppe_index = opt->ppe_index;
	unsigned short hash_index = opt->hash_index;

	if (ppe_index >= CFG_PPE_NUM || hash_index >= h->foe_etry_num)
		return HNAT_FAIL;

	entry = &h->foe_table_cpu[ppe_index][hash_index];

	if (!entry_hnat_is_bound(entry))
		return HNAT_ENTRY_NOT_FOUND;

	return mtk_hnat_foe_to_hnat_tuple(entry, opt);
}
EXPORT_SYMBOL(mtk_hnat_get_ppe_entry_by_index);

int mtk_hnat_calc_ppe_hash_index_by_tuple(struct hnat_tuple *opt)
{
	struct foe_entry entry = { 0 };
	struct net_device *dev = NULL;
	struct mtk_mac *mac;

	if (opt)
		dev = dev_get_by_name(&init_net, opt->ing_dev);

	if (!dev) {
		pr_err("hnat ing_dev not found!\n");
		return HNAT_FAIL;
	}

	mac = netdev_priv(dev);

	if (mac->id == MTK_GMAC3_ID && CFG_PPE_NUM >= 3)
		opt->ppe_index = 2;
	else if (mac->id == MTK_GMAC2_ID && CFG_PPE_NUM >= 2)
		opt->ppe_index = 1;
	else
		opt->ppe_index = 0;

	dev_put(dev);

	entry.bfib1.pkt_type = opt->pkt_type;

	switch (opt->pkt_type) {
	case IPV4_HNAPT:
		entry.ipv4_hnapt.sip = opt->ing_sipv4;
		entry.ipv4_hnapt.dip = opt->ing_dipv4;
		entry.ipv4_hnapt.sport = opt->ing_sp;
		entry.ipv4_hnapt.dport = opt->ing_dp;
		break;
	case IPV6_5T_ROUTE:
		/* copy IPv6 source and destination address */
		entry.ipv6_5t_route.ipv6_sip0 = opt->ing_sipv6_0;
		entry.ipv6_5t_route.ipv6_sip1 = opt->ing_sipv6_1;
		entry.ipv6_5t_route.ipv6_sip2 = opt->ing_sipv6_2;
		entry.ipv6_5t_route.ipv6_sip3 = opt->ing_sipv6_3;
		entry.ipv6_5t_route.ipv6_dip0 = opt->ing_dipv6_0;
		entry.ipv6_5t_route.ipv6_dip1 = opt->ing_dipv6_1;
		entry.ipv6_5t_route.ipv6_dip2 = opt->ing_dipv6_2;
		entry.ipv6_5t_route.ipv6_dip3 = opt->ing_dipv6_3;
		entry.ipv6_5t_route.sport = opt->ing_sp;
		entry.ipv6_5t_route.dport = opt->ing_dp;
		break;
	default:
		return HNAT_FAIL;
	}

	opt->hash_index = hnat_get_ppe_hash(&entry);

	return HNAT_SUCCESS;
}
EXPORT_SYMBOL(mtk_hnat_calc_ppe_hash_index_by_tuple);

int mtk_hnat_delete_entry_by_index(unsigned short ppe_index,
				   unsigned short hash_index)
{
	struct foe_entry *entry;
	struct mtk_hnat *h = hnat_priv;

	if (ppe_index >= CFG_PPE_NUM || hash_index >= h->foe_etry_num)
		return HNAT_FAIL;

	entry = h->foe_table_cpu[ppe_index] + hash_index;

	spin_lock(&hnat_priv->entry_lock);
	__entry_delete(entry);
	spin_unlock(&hnat_priv->entry_lock);

	hnat_cache_clr(ppe_index);

	return HNAT_SUCCESS;
}
EXPORT_SYMBOL(mtk_hnat_delete_entry_by_index);

int mtk_hnat_get_mib_count_by_index(unsigned short ppe_index,
				    unsigned short hash_index,
				    unsigned long long *pkt_cnt,
				    unsigned long long *byte_cnt)
{
	struct mtk_hnat *h = hnat_priv;
	struct hnat_accounting diff = {0};
	struct foe_entry *entry;

	if (ppe_index >= CFG_PPE_NUM || hash_index >= h->foe_etry_num)
		return HNAT_FAIL;

	if (!hnat_priv->data->per_flow_accounting)
		return HNAT_FAIL;

	if (!pkt_cnt || !byte_cnt) {
		pr_err("%s: invalid input parameters!\n", __func__);
		return HNAT_FAIL;
	}

	entry = h->foe_table_cpu[ppe_index] + hash_index;

	if (!entry_hnat_is_bound(entry))
		return HNAT_ENTRY_NOT_FOUND;

	hnat_get_count(h, ppe_index, hash_index, &diff);

	*pkt_cnt = diff.packets;
	*byte_cnt = diff.bytes;

	return HNAT_SUCCESS;
}
EXPORT_SYMBOL(mtk_hnat_get_mib_count_by_index);

int mtk_hnat_get_all_mib_counts(unsigned long long **pkt_cnts,
				unsigned long long **byte_cnts)
{
	struct mtk_hnat *h = hnat_priv;
	struct hnat_accounting diff;
	struct foe_entry *entry;
	u32 ppe_index, hash_index;
	u32 bind_cnt = 0;

	if (!h->data->per_flow_accounting)
		return 0;

	if (!pkt_cnts || !byte_cnts) {
		pr_err("%s: invalid input parameters!\n", __func__);
		return 0;
	}

	for (ppe_index = 0; ppe_index < CFG_PPE_NUM; ppe_index++) {
		for (hash_index = 0; hash_index < h->foe_etry_num; hash_index++) {
			entry = h->foe_table_cpu[ppe_index] + hash_index;
			if (!entry_hnat_is_bound(entry))
				continue;

			bind_cnt++;

			if (!hnat_get_count(h, ppe_index, hash_index, &diff))
				continue;

			pkt_cnts[ppe_index][hash_index] = diff.packets;
			byte_cnts[ppe_index][hash_index] = diff.bytes;
		}
	}

	return bind_cnt;
}

/* Helper function: validate skb and extract PPE index and hash */
static int hnat_get_idx_from_skb(struct sk_buff *skb, unsigned short *ppe_idx,
				 unsigned short *hash_idx)
{
	if (!skb || skb_hnat_alg(skb) ||
	    unlikely(!is_magic_tag_valid(skb) || !IS_SPACE_AVAILABLE_HEAD(skb)) ||
	    !skb_hnat_is_hashed(skb))
		return HNAT_FAIL;

	/* Update by skb API would only update entry with Keep-alive packet */
	if (skb_hnat_reason(skb) != HIT_BIND_KEEPALIVE_DUP_OLD_HDR)
		return HNAT_FAIL;

	*ppe_idx = skb_hnat_ppe(skb);
	*hash_idx = skb_hnat_entry(skb);

	return HNAT_SUCCESS;
}

/* Update logic callback type */
typedef int (*hnat_update_fn)(struct foe_entry *entry, void *data);

/* Generic update template */
static int hnat_update_entry_generic(unsigned short ppe_index,
				     unsigned short hash_index,
				     hnat_update_fn update_logic, void *data)
{
	struct mtk_hnat *h = hnat_priv;
	struct foe_entry *hw_entry, entry = { 0 };
	int ret;

	if (ppe_index >= CFG_PPE_NUM || hash_index >= h->foe_etry_num)
		return HNAT_FAIL;

	hw_entry = &h->foe_table_cpu[ppe_index][hash_index];
	spin_lock_bh(&h->entry_lock);

	if (!entry_hnat_is_bound(hw_entry)) {
		spin_unlock_bh(&h->entry_lock);
		return HNAT_ENTRY_NOT_FOUND;
	}

	memcpy(&entry, hw_entry, sizeof(entry));

	/* Execute specific field modification logic */
	ret = update_logic(&entry, data);

	if (ret == HNAT_SUCCESS) {
		/* We must ensure all info has been updated before set to hw */
		wmb();
		memcpy(hw_entry, &entry, sizeof(entry));
		dma_wmb();

		spin_unlock_bh(&h->entry_lock);
		/* clear ppe cache */
		hnat_cache_clr(ppe_index);
	} else {
		spin_unlock_bh(&h->entry_lock);
	}

	return (ret == HNAT_ALREADY_SET) ? HNAT_SUCCESS : ret;
}

struct hqos_data {
	unsigned short fqos;
	unsigned short qid;
};

static int hnat_entry_set_hqos(struct foe_entry *entry, void *data)
{
	struct hqos_data *d = (struct hqos_data *)data;
	unsigned short fqos, qid;

	if (!d || !entry)
		return HNAT_FAIL;

	if (d->qid >= MTK_QDMA_NUM_QUEUES)
		return HNAT_FAIL;

	fqos = (d->fqos) ? 1 : 0;
	qid = d->qid & MTK_QDMA_QUEUE_MASK;

	if (IS_IPV4_GRP(entry)) {
		if (entry->ipv4_hnapt.iblk2.fqos == fqos &&
		    entry->ipv4_hnapt.iblk2.qid == qid)
			return HNAT_ALREADY_SET;

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
		entry->ipv4_hnapt.tport_id = (fqos) ? NR_QDMA_TPORT : 0;
#endif
		entry->ipv4_hnapt.iblk2.fqos = fqos;
		entry->ipv4_hnapt.iblk2.qid = qid;
	} else if (IS_IPV6_GRP(entry)) {
		if (entry->ipv6_5t_route.iblk2.fqos == fqos &&
		    entry->ipv6_5t_route.iblk2.qid == qid)
			return HNAT_ALREADY_SET;

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
		entry->ipv6_5t_route.tport_id = (fqos) ? NR_QDMA_TPORT : 0;
#endif
		entry->ipv6_5t_route.iblk2.fqos = fqos;
		entry->ipv6_5t_route.iblk2.qid = qid;
	} else {
		return HNAT_FAIL;
	}

	return HNAT_SUCCESS;
}

int mtk_hnat_update_hqos_by_index(unsigned short ppe_index,
				  unsigned short hash_index,
				  unsigned short fqos,
				  unsigned short qid)
{
	struct hqos_data data = { .fqos = fqos, .qid = qid };

	return hnat_update_entry_generic(ppe_index, hash_index,
					 hnat_entry_set_hqos, &data);
}
EXPORT_SYMBOL(mtk_hnat_update_hqos_by_index);

int mtk_hnat_update_hqos_by_skb(struct sk_buff *skb,
				unsigned short fqos,
				unsigned short qid)
{
	unsigned short ppe_idx, hash_idx;

	if (hnat_get_idx_from_skb(skb, &ppe_idx, &hash_idx) != HNAT_SUCCESS)
		return HNAT_FAIL;

	return mtk_hnat_update_hqos_by_index(ppe_idx, hash_idx, fqos, qid);
}
EXPORT_SYMBOL(mtk_hnat_update_hqos_by_skb);

static int hnat_entry_set_dscp(struct foe_entry *entry, void *data)
{
	int dscp;

	if (!data || !entry)
		return HNAT_FAIL;

	dscp = *(int *)data;
	dscp = (dscp > 0) ? dscp & 0xff : 0;

	if (IS_IPV4_GRP(entry)) {
		if (entry->ipv4_hnapt.iblk2.dscp == dscp)
			return HNAT_ALREADY_SET;
		entry->ipv4_hnapt.iblk2.dscp = dscp;
	} else if (IS_IPV6_GRP(entry)) {
		if (entry->ipv6_5t_route.iblk2.dscp == dscp)
			return HNAT_ALREADY_SET;
		entry->ipv6_5t_route.iblk2.dscp = dscp;
	} else {
		return HNAT_FAIL;
	}

	return HNAT_SUCCESS;
}

int mtk_hnat_update_dscp_by_index(unsigned short ppe_index,
				  unsigned short hash_index,
				  int dscp)
{
	return hnat_update_entry_generic(ppe_index, hash_index,
					 hnat_entry_set_dscp, &dscp);
}
EXPORT_SYMBOL(mtk_hnat_update_dscp_by_index);

int mtk_hnat_update_dscp_by_skb(struct sk_buff *skb, int dscp)
{
	unsigned short ppe_idx, hash_idx;

	if (hnat_get_idx_from_skb(skb, &ppe_idx, &hash_idx) != HNAT_SUCCESS)
		return HNAT_FAIL;

	return mtk_hnat_update_dscp_by_index(ppe_idx, hash_idx, dscp);
}
EXPORT_SYMBOL(mtk_hnat_update_dscp_by_skb);

struct vlan_data {
	unsigned short vlan1;
	unsigned short vlan2;
};

static int hnat_entry_set_vlan(struct foe_entry *entry, void *data)
{
	struct vlan_data *d = (struct vlan_data *)data;
	unsigned short *vlan1, *vlan2, *sp_tag;
	unsigned short vlan_layer;
	bool is_dsa;

	if (!d || !entry)
		return HNAT_FAIL;

	if (IS_IPV4_GRP(entry)) {
		vlan1 = &entry->ipv4_hnapt.vlan1;
		vlan2 = &entry->ipv4_hnapt.vlan2;
		sp_tag = &entry->ipv4_hnapt.sp_tag;
	} else if (IS_IPV6_GRP(entry)) {
		vlan1 = &entry->ipv6_5t_route.vlan1;
		vlan2 = &entry->ipv6_5t_route.vlan2;
		sp_tag = &entry->ipv6_5t_route.sp_tag;
	} else {
		return HNAT_FAIL;
	}

	vlan_layer = (d->vlan1 != 0) + (d->vlan2 != 0);

	/* Prevent unnecessary update */
	if (entry->bfib1.vlan_layer == vlan_layer && *vlan1 == d->vlan1 && *vlan2 == d->vlan2)
		return HNAT_ALREADY_SET;

	is_dsa = (entry->bfib1.vlan_layer != 0 && *sp_tag != ETH_P_8021Q);

	entry->bfib1.vlan_layer = vlan_layer;
	*vlan1 = d->vlan1 & 0xffff;
	*vlan2 = d->vlan2 & 0xffff;

	if (is_dsa) {
		if (!entry->bfib1.vlan_layer) {
			entry->bfib1.vlan_layer = 1;
			*sp_tag &= ~BIT(8);
		} else {
			*sp_tag |= BIT(8);
		}
	} else if (entry->bfib1.vlan_layer) {
		*sp_tag = ETH_P_8021Q;
	}

	return HNAT_SUCCESS;
}

int mtk_hnat_update_vlan_by_index(unsigned short ppe_index,
				  unsigned short hash_index,
				  unsigned short vlan1,
				  unsigned short vlan2)
{
	struct vlan_data data = { .vlan1 = vlan1, .vlan2 = vlan2 };

	return hnat_update_entry_generic(ppe_index, hash_index,
					 hnat_entry_set_vlan, &data);
}
EXPORT_SYMBOL(mtk_hnat_update_vlan_by_index);

int mtk_hnat_update_vlan_by_skb(struct sk_buff *skb,
				unsigned short vlan1,
				unsigned short vlan2)
{
	unsigned short ppe_idx, hash_idx;

	if (hnat_get_idx_from_skb(skb, &ppe_idx, &hash_idx) != HNAT_SUCCESS)
		return HNAT_FAIL;

	return mtk_hnat_update_vlan_by_index(ppe_idx, hash_idx, vlan1, vlan2);
}
EXPORT_SYMBOL(mtk_hnat_update_vlan_by_skb);

struct pppoe_data {
	unsigned short psn;
	unsigned short pppoe_id;
};

static int hnat_entry_set_pppoe(struct foe_entry *entry, void *data)
{
	struct pppoe_data *d = (struct pppoe_data *)data;
	unsigned short pppoe_id, psn;

	if (!d || !entry)
		return HNAT_FAIL;

	psn = (d->psn) ? 1 : 0;
	pppoe_id = d->pppoe_id & 0xffff;

	if (IS_IPV4_GRP(entry)) {
		if (entry->bfib1.psn == psn &&
		    entry->ipv4_hnapt.pppoe_id == pppoe_id)
			return HNAT_ALREADY_SET;
		entry->ipv4_hnapt.pppoe_id = pppoe_id;
	} else if (IS_IPV6_GRP(entry)) {
		if (entry->bfib1.psn == psn &&
		    entry->ipv6_5t_route.pppoe_id == pppoe_id)
			return HNAT_ALREADY_SET;
		entry->ipv6_5t_route.pppoe_id = pppoe_id;
	} else {
		return HNAT_FAIL;
	}

	entry->bfib1.psn = psn;

	return HNAT_SUCCESS;
}

int mtk_hnat_update_pppoe_by_index(unsigned short ppe_index,
				   unsigned short hash_index,
				   unsigned short psn,
				   unsigned short pppoe_id)
{
	struct pppoe_data data = { .psn = psn, .pppoe_id = pppoe_id };

	return hnat_update_entry_generic(ppe_index, hash_index,
					 hnat_entry_set_pppoe, &data);
}
EXPORT_SYMBOL(mtk_hnat_update_pppoe_by_index);

int mtk_hnat_update_pppoe_by_skb(struct sk_buff *skb,
				 unsigned short psn,
				 unsigned short pppoe_id)
{
	unsigned short ppe_idx, hash_idx;

	if (hnat_get_idx_from_skb(skb, &ppe_idx, &hash_idx) != HNAT_SUCCESS)
		return HNAT_FAIL;

	return mtk_hnat_update_pppoe_by_index(ppe_idx, hash_idx, psn, pppoe_id);
}
EXPORT_SYMBOL(mtk_hnat_update_pppoe_by_skb);

static int hnat_entry_set_is_prior(struct foe_entry *entry, void *data)
{
	unsigned short is_prior;

	if (!data || !entry)
		return HNAT_FAIL;

	is_prior = (*(unsigned short *)data) & 0x1;

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
	if (IS_IPV4_GRP(entry)) {
		if (entry->ipv4_hnapt.winfo_pao.is_prior == is_prior)
			return HNAT_ALREADY_SET;
		entry->ipv4_hnapt.winfo_pao.is_prior = is_prior;
	} else if (IS_IPV6_HNAPT(entry) || IS_IPV6_HNAT(entry)) {
		if (entry->ipv6_hnapt.winfo_pao.is_prior == is_prior)
			return HNAT_ALREADY_SET;
		entry->ipv6_hnapt.winfo_pao.is_prior = is_prior;
	} else if (IS_IPV6_GRP(entry)) {
		if (entry->ipv4_mape.winfo_pao.is_prior == is_prior)
			return HNAT_ALREADY_SET;
		entry->ipv4_mape.winfo_pao.is_prior = is_prior;
	} else {
		return HNAT_FAIL;
	}
#else
	return HNAT_FAIL;
#endif

	return HNAT_SUCCESS;
}

int mtk_hnat_update_is_prior_by_index(unsigned short ppe_index,
				      unsigned short hash_index,
				      unsigned short is_prior)
{
	return hnat_update_entry_generic(ppe_index, hash_index,
					 hnat_entry_set_is_prior, &is_prior);
}
EXPORT_SYMBOL(mtk_hnat_update_is_prior_by_index);

int mtk_hnat_update_is_prior_by_skb(struct sk_buff *skb,
				    unsigned short is_prior)
{
	unsigned short ppe_idx, hash_idx;

	if (hnat_get_idx_from_skb(skb, &ppe_idx, &hash_idx) != HNAT_SUCCESS)
		return HNAT_FAIL;

	return mtk_hnat_update_is_prior_by_index(ppe_idx, hash_idx, is_prior);
}
EXPORT_SYMBOL(mtk_hnat_update_is_prior_by_skb);

static int hnat_entry_set_tid(struct foe_entry *entry, void *data)
{
	unsigned short tid;

	if (!data || !entry)
		return HNAT_FAIL;

	tid = (*(unsigned short *)data) & 0xf;

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
	if (IS_IPV4_GRP(entry)) {
		if (entry->ipv4_hnapt.winfo_pao.tid == tid)
			return HNAT_ALREADY_SET;
		entry->ipv4_hnapt.winfo_pao.tid = tid;
	} else if (IS_IPV6_HNAPT(entry) || IS_IPV6_HNAT(entry)) {
		if (entry->ipv6_hnapt.winfo_pao.tid == tid)
			return HNAT_ALREADY_SET;
		entry->ipv6_hnapt.winfo_pao.tid = tid;
	} else if (IS_IPV6_GRP(entry)) {
		if (entry->ipv4_mape.winfo_pao.tid == tid)
			return HNAT_ALREADY_SET;
		entry->ipv4_mape.winfo_pao.tid = tid;
	} else {
		return HNAT_FAIL;
	}
#else
	return HNAT_FAIL;
#endif

	return HNAT_SUCCESS;
}

int mtk_hnat_update_tid_by_index(unsigned short ppe_index,
				 unsigned short hash_index,
				 unsigned short tid)
{
	return hnat_update_entry_generic(ppe_index, hash_index,
					 hnat_entry_set_tid, &tid);
}
EXPORT_SYMBOL(mtk_hnat_update_tid_by_index);

int mtk_hnat_update_tid_by_skb(struct sk_buff *skb, unsigned short tid)
{
	unsigned short ppe_idx, hash_idx;

	if (hnat_get_idx_from_skb(skb, &ppe_idx, &hash_idx) != HNAT_SUCCESS)
		return HNAT_FAIL;

	return mtk_hnat_update_tid_by_index(ppe_idx, hash_idx, tid);
}
EXPORT_SYMBOL(mtk_hnat_update_tid_by_skb);

int mtk_hnat_register_bind_callback(void (*func)(struct hnat_tuple *))
{
	if (!func) {
		pr_err("%s: callback function is null!\n", __func__);
		return HNAT_FAIL;
	}

	hnat_bind_callback = func;

	return HNAT_SUCCESS;
}
EXPORT_SYMBOL(mtk_hnat_register_bind_callback);

int mtk_hnat_register_fin_callback(void (*func)(struct hnat_tuple *))
{
	if (!func) {
		pr_err("%s: callback function is null!\n", __func__);
		return HNAT_FAIL;
	}

	hnat_fin_callback = func;

	return HNAT_SUCCESS;
}
EXPORT_SYMBOL(mtk_hnat_register_fin_callback);

int mtk_hnat_get_fin_age_config(unsigned short ppe_index)
{
	struct mtk_hnat *h = hnat_priv;
	u32 val;

	if (ppe_index >= CFG_PPE_NUM)
		return -EINVAL;

	val = readl(h->ppe_base[ppe_index] + PPE_TB_CFG);

	return !!(val & FIN_AGE);
}
EXPORT_SYMBOL(mtk_hnat_get_fin_age_config);

int mtk_hnat_set_fin_age_config(unsigned short ppe_index, bool enable)
{
	struct mtk_hnat *h = hnat_priv;

	if (ppe_index >= CFG_PPE_NUM)
		return HNAT_FAIL;

	cr_set_field(h->ppe_base[ppe_index] + PPE_TB_CFG, FIN_AGE, enable);

	return HNAT_SUCCESS;
}
EXPORT_SYMBOL(mtk_hnat_set_fin_age_config);

int hnat_tuple_detail(struct hnat_tuple *opt)
{
	pr_info("==========<PPE_ID=%d, Flow Table Entry=%d>===============\n",
		opt->ppe_index, opt->hash_index);
	pr_info("-----------------<HNAT_TUPLE Info>------------------\n");
	switch (opt->pkt_type) {
	case IPV4_HNAPT:
		pr_info("Create IPV4_HNAPT entry\n");
		pr_info("IPv4 Org IP/Port: %pI4:%d->%pI4:%d\n",
			&opt->ing_sipv4, opt->ing_sp, &opt->ing_dipv4, opt->ing_dp);
		pr_info("IPv4 New IP/Port: %pI4:%d->%pI4:%d\n",
			&opt->eg_sipv4, opt->eg_sp, &opt->eg_dipv4, opt->eg_dp);
		break;
	case IPV6_5T_ROUTE:
		pr_info("Create IPv6 5-Tuple entry\n");
		pr_info("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X:%d-> %08X:%08X:%08X:%08X:%d\n",
			opt->ing_sipv6_0, opt->ing_sipv6_1, opt->ing_sipv6_2, opt->ing_sipv6_3,
			opt->ing_sp,
			opt->ing_dipv6_0, opt->ing_dipv6_1, opt->ing_dipv6_2, opt->ing_dipv6_3,
			opt->ing_dp);
		pr_info("EG  SIPv6->DIPv6: %08X:%08X:%08X:%08X:%d-> %08X:%08X:%08X:%08X:%d\n",
			opt->eg_sipv6_0, opt->eg_sipv6_1, opt->eg_sipv6_2, opt->eg_sipv6_3,
			opt->eg_sp,
			opt->eg_dipv6_0, opt->eg_dipv6_1, opt->eg_dipv6_2, opt->eg_dipv6_3,
			opt->eg_dp);
		break;
	default:
		pr_info("Unknown PPE type!\n");
		break;
	}

	pr_info("SMAC=%pM => DMAC=%pM\n", opt->smac, opt->dmac);
	pr_info("FQOS=%d QID=%d\n", opt->fqos, opt->qid);
	pr_info("Vlan_Layer = %u, Vid1 = 0x%x, Vid2 = 0x%x\n",
		opt->vlan_layer, opt->vlan1, opt->vlan2);
	pr_info("psn = %d, pppoe = %d, proto = %s\n",
		opt->psn, opt->pppoe_id,
		opt->is_udp == 0 ? "TCP" : opt->is_udp == 1 ? "UDP" : "Unknown");
	pr_info("dscp = %d\n", opt->dscp);
	pr_info("fdr_en = %d, rxid = %d, fp = %d\n", opt->fdr_en, opt->rxid, opt->fp);
	pr_info("ing_dev = %s, eg_dev = %s\n", opt->ing_dev, opt->eg_dev);
	pr_info("tid = %d | is_prior = %d\n", opt->tid, opt->is_prior);

	return 0;
}

void hnat_trigger_callback(void (*func)(struct hnat_tuple *),
			   struct sk_buff *skb)
{
	struct hnat_tuple opt = {0};
	struct net_device *dev;

	opt.ppe_index = skb_hnat_ppe(skb);
	opt.hash_index = skb_hnat_entry(skb);

	dev = dev_get_by_index(&init_net, skb->skb_iif);
	if (dev) {
		strscpy(opt.ing_dev, dev->name, sizeof(opt.ing_dev));
		dev_put(dev);
	}

	strscpy(opt.eg_dev, skb->dev->name, sizeof(opt.eg_dev));

	if (func && mtk_hnat_get_ppe_entry_by_index(&opt) == HNAT_SUCCESS)
		func(&opt);

}

void hnat_bind_callback_test(struct hnat_tuple *opt)
{
	pr_info("%s() called.\n", __func__);
	hnat_tuple_detail(opt);
}

void hnat_fin_callback_test(struct hnat_tuple *opt)
{
	pr_info("%s() called.\n", __func__);
	hnat_tuple_detail(opt);
}

static int hnat_manual_api_read(struct seq_file *m, void *private)
{
	seq_puts(m, "===========================Manual_API===========================\n");
	seq_puts(m, "0: show ppe_num and ppe_entry_num\n");
	seq_puts(m, "1: $ppe_index $hash_index : get entry detail by ppe_indx and hash_index\n");
	seq_puts(m, "2: $ppe_index $hash_index : delete entry by ppe_indx and hash_index\n");
	seq_puts(m, "3: $ppe_index $hash_index : ");
	seq_puts(m, "get entry statistics by ppe_indx and hash_index\n");
	seq_puts(m, "4: get all entry statistics\n");
	seq_puts(m, "5: $ppe_index $hash_index $fqos $qid : update HQOS\n");
	seq_puts(m, "6: $ppe_index $hash_index $dscp : update dscp\n");
	seq_puts(m, "7: $ppe_index $hash_index $vlan1 vlan2 : update vlan1 and vlan2\n");
	seq_puts(m, "8: $ppe_index $hash_index $psn $pppoe_id : update psn and pppoe_id\n");
	seq_puts(m, "9: register testing entry bind and fin callback\n");
	seq_puts(m, "10: $ppe_index $enable : get/set fin age config\n");
	seq_puts(m, "11: $pkt_type $ing_dev $sip $dip [$sip1-3 $dip1-3] $sp $dp : ");
	seq_puts(m, "get calculated ppe/hash index\n");
	seq_puts(m, "12: $ppe_index $hash_index $tid : update tid\n");
	seq_puts(m, "13: $ppe_index $hash_index $is_prior : update is_prior\n");

	return 0;
}

static int hnat_manual_action_show_stats(char *p_buf)
{
	unsigned short ppe_index, hash_index;
	struct hnat_tuple opt = { 0 };
	int ret;

	if (sscanf(p_buf, "%*d %1hu %5hu", &ppe_index, &hash_index) != 2)
		return -EFAULT;

	if (ppe_index >= CFG_PPE_NUM || hash_index >= hnat_priv->foe_etry_num)
		return -EINVAL;

	opt.ppe_index = ppe_index;
	opt.hash_index = hash_index;
	ret = mtk_hnat_get_ppe_entry_by_index(&opt);
	if (ret != HNAT_SUCCESS)
		return (ret == HNAT_ENTRY_NOT_FOUND) ? -ENOENT : -EINVAL;

	hnat_tuple_detail(&opt);

	return 0;
}

static int hnat_manual_action_delete(char *p_buf)
{
	unsigned short ppe_index, hash_index;
	int ret;

	if (sscanf(p_buf, "%*d %1hu %5hu", &ppe_index, &hash_index) != 2)
		return -EFAULT;

	ret = mtk_hnat_delete_entry_by_index(ppe_index, hash_index);

	return (ret == HNAT_SUCCESS) ? 0 : -EINVAL;
}

static int hnat_manual_action_mib(char *p_buf)
{
	unsigned short ppe_index, hash_index;
	unsigned long long pkt_cnt, byte_cnt;
	int ret;

	if (sscanf(p_buf, "%*d %1hu %5hu", &ppe_index, &hash_index) != 2)
		return -EFAULT;

	ret = mtk_hnat_get_mib_count_by_index(ppe_index, hash_index, &pkt_cnt, &byte_cnt);
	if (ret == HNAT_SUCCESS) {
		pr_info("get mib count of ppe=%d, entry=%d, pkt=%llu, byte=%llu\n",
			ppe_index, hash_index, pkt_cnt, byte_cnt);
		return 0;
	}

	return (ret == HNAT_ENTRY_NOT_FOUND) ? -ENOENT : -EINVAL;
}

static int hnat_manual_action_all_mib(void)
{
	unsigned long long **pkt_cnts, **byte_cnts;
	int i, j, ret = 0;

	if (!hnat_priv->data->per_flow_accounting)
		return 0;

	pkt_cnts = kvzalloc(CFG_PPE_NUM * sizeof(unsigned long long *),
			    GFP_KERNEL);
	byte_cnts = kvzalloc(CFG_PPE_NUM * sizeof(unsigned long long *),
			     GFP_KERNEL);
	if (!pkt_cnts || !byte_cnts) {
		kvfree(pkt_cnts);
		kvfree(byte_cnts);
		return -ENOMEM;
	}

	for (i = 0; i < CFG_PPE_NUM; i++) {
		pkt_cnts[i] = kvzalloc(hnat_priv->foe_etry_num *
					       sizeof(unsigned long long),
				       GFP_KERNEL);
		byte_cnts[i] = kvzalloc(hnat_priv->foe_etry_num *
						sizeof(unsigned long long),
					GFP_KERNEL);
		if (!pkt_cnts[i] || !byte_cnts[i]) {
			ret = -ENOMEM;
			goto err;
		}
	}

	pr_info("BIND CNT:%d\n",
		mtk_hnat_get_all_mib_counts(pkt_cnts, byte_cnts));
	for (i = 0; i < CFG_PPE_NUM; i++) {
		for (j = 0; j < hnat_priv->foe_etry_num; j++) {
			if (pkt_cnts[i][j] != 0)
				pr_info("ppe=%d, entry=%d, pkt=%llu, byte=%llu\n",
					i, j, pkt_cnts[i][j], byte_cnts[i][j]);
		}
	}

err:
	for (i = 0; i < CFG_PPE_NUM; i++) {
		kvfree(pkt_cnts[i]);
		kvfree(byte_cnts[i]);
	}
	kvfree(pkt_cnts);
	kvfree(byte_cnts);

	return ret;
}

static int hnat_manual_action_update_entry(int action, char *p_buf)
{
	unsigned short ppe_index, hash_index;
	unsigned short val1, val2;
	struct hnat_tuple opt = {0};

	if (sscanf(p_buf, "%*d %1hu %5hu", &ppe_index, &hash_index) != 2)
		return -EFAULT;

	if (ppe_index >= CFG_PPE_NUM || hash_index >= hnat_priv->foe_etry_num)
		return -EINVAL;

	switch (action) {
	case 5:
		if (sscanf(p_buf, "%*d %*d %*d %1hu %3hu", &val1, &val2) != 2)
			return -EFAULT;
		if (mtk_hnat_update_hqos_by_index(ppe_index, hash_index,
						  val1, val2) != HNAT_SUCCESS)
			return -EINVAL;
		break;
	case 6:
		if (sscanf(p_buf, "%*d %*d %*d %3hu", &val1) != 1)
			return -EFAULT;
		if (mtk_hnat_update_dscp_by_index(ppe_index, hash_index, val1) != HNAT_SUCCESS)
			return -EINVAL;
		break;
	case 7:
		if (sscanf(p_buf, "%*d %*d %*d %5hu %5hu", &val1, &val2) != 2)
			return -EFAULT;
		if (mtk_hnat_update_vlan_by_index(ppe_index, hash_index,
						  val1, val2) != HNAT_SUCCESS)
			return -EINVAL;
		break;
	case 8:
		if (sscanf(p_buf, "%*d %*d %*d %1hu %5hu", &val1, &val2) != 2)
			return -EFAULT;
		if (mtk_hnat_update_pppoe_by_index(ppe_index, hash_index,
						   val1, val2) != HNAT_SUCCESS)
			return -EINVAL;
		break;
	case 12:
		if (sscanf(p_buf, "%*d %*d %*d %2hu", &val1) != 1)
			return -EFAULT;
		if (mtk_hnat_update_tid_by_index(ppe_index, hash_index, val1) != HNAT_SUCCESS)
			return -EINVAL;
		break;
	case 13:
		if (sscanf(p_buf, "%*d %*d %*d %1hu", &val1) != 1)
			return -EFAULT;
		if (mtk_hnat_update_is_prior_by_index(ppe_index, hash_index, val1) != HNAT_SUCCESS)
			return -EINVAL;
		break;
	}

	opt.ppe_index = ppe_index;
	opt.hash_index = hash_index;
	if (mtk_hnat_get_ppe_entry_by_index(&opt) == HNAT_SUCCESS)
		hnat_tuple_detail(&opt);
	else
		return -ENOENT;

	return 0;
}

static int hnat_manual_api_open(struct inode *inode, struct file *file)
{
	return single_open(file, hnat_manual_api_read, file->private_data);
}

static ssize_t hnat_manual_api_write(struct file *file,
				     const char __user *buffer,
				     size_t count, loff_t *data)
{
	char buf[256], *p_buf;
	int action, ret = 0, n;

	if (count >= sizeof(buf) || copy_from_user(buf, buffer, count))
		return -EFAULT;

	buf[count] = '\0';
	p_buf = strim(buf);
	/* add dummy variable n to resolve SSCANF_TO_KSTRTO warning */
	if (sscanf(p_buf, "%2d%n", &action, &n) < 1)
		return -EFAULT;

	switch (action) {
	case 0:
		pr_info("Configured PPE_NUM:%d, PPE_ENTRY_NUM:%d\n",
			 mtk_hnat_get_ppe_num(), mtk_hnat_get_ppe_entry_num());
		break;
	case 1:
		ret = hnat_manual_action_show_stats(p_buf);
		break;
	case 2:
		ret = hnat_manual_action_delete(p_buf);
		break;
	case 3:
		ret = hnat_manual_action_mib(p_buf);
		break;
	case 4:
		ret = hnat_manual_action_all_mib();
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 12:
	case 13:
		ret = hnat_manual_action_update_entry(action, p_buf);
		break;
	case 9:
		if (mtk_hnat_register_bind_callback(hnat_bind_callback_test) != HNAT_SUCCESS ||
		    mtk_hnat_register_fin_callback(hnat_fin_callback_test) != HNAT_SUCCESS) {
			ret = -EINVAL;
		} else {
			pr_info("Registered test callbacks!\n");
		}
		break;
	case 10: {
		unsigned short ppe_idx, en;

		if (sscanf(p_buf, "%*d %1hu %1hu", &ppe_idx, &en) != 2)
			return -EFAULT;

		mtk_hnat_set_fin_age_config(ppe_idx, en);
		pr_info("PPE%d fin age: %d\n", ppe_idx,
			mtk_hnat_get_fin_age_config(ppe_idx));
		break;
	}
	case 11: {
		struct hnat_tuple opt = { 0 };

		if (sscanf(p_buf, "%*d %2hu", &opt.pkt_type) != 1)
			return -EFAULT;

		if (opt.pkt_type == IPV4_HNAPT) {
			if (sscanf(p_buf, "%*d %*d %15s %8x %8x %4hx %4hx",
				   opt.ing_dev, &opt.ing_sipv4,
				   &opt.ing_dipv4, &opt.ing_sp,
				   &opt.ing_dp) != 5)
				return -EFAULT;
		} else if (opt.pkt_type == IPV6_5T_ROUTE) {
			if (sscanf(p_buf, "%*d %*d %15s %8x%8x%8x%8x %8x%8x%8x%8x %4hx %4hx",
				   opt.ing_dev, &opt.ing_sipv6_0,
				   &opt.ing_sipv6_1, &opt.ing_sipv6_2,
				   &opt.ing_sipv6_3, &opt.ing_dipv6_0,
				   &opt.ing_dipv6_1, &opt.ing_dipv6_2,
				   &opt.ing_dipv6_3, &opt.ing_sp,
				   &opt.ing_dp) != 11)
				return -EFAULT;
		} else {
			return -EINVAL;
		}

		mtk_hnat_calc_ppe_hash_index_by_tuple(&opt);
		pr_info("Calculated ppe=%d, hash=%d\n",
			opt.ppe_index, opt.hash_index);
		break;
	}
	default:
		pr_info("Unknown action: %d\n", action);
		return -EINVAL;
	}

	return (ret < 0) ? ret : count;
}

static const struct file_operations hnat_manual_api_fops = {
	.open = hnat_manual_api_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.write = hnat_manual_api_write,
	.release = single_release,
};

void hnat_api_init_debugfs(struct dentry *root)
{
	debugfs_create_file("manual_api", 0444, root, hnat_priv,
			    &hnat_manual_api_fops);
}

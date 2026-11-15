// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2014-2016 Zhiqiang Yang <zhiqiang.yang@mediatek.com>
 */

#include <net/netfilter/nf_flow_table.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_bridge.h>
#include "hnat.h"
#include <net/dsa.h>

struct npu_hnat_mcast_ops *hnat_mc_ops;

bool (*hnat_mcast_eth_ext_link_handle)(bool up) = NULL;
EXPORT_SYMBOL(hnat_mcast_eth_ext_link_handle);

int (*mtk_hnat_mcast_blist)(u8 *ip, u32 mask, bool add, bool is_ipv4) = NULL;
EXPORT_SYMBOL(mtk_hnat_mcast_blist);

int (*mtk_hnat_mcast_offload)(bool enable) = NULL;
EXPORT_SYMBOL(mtk_hnat_mcast_offload);

static int (*mtk_wifi_mcast_get_group_info)(u32 id, void *data);

#define BMP_IS_SINGLE_PORT(pse_bmp)  (hweight32(pse_bmp) == 1)

static int set_hnat_mtbl(u8 *mac, int index, int ppe_id, u32 pse_bmp)
{
	struct ppe_mcast_h mcast_h;
	struct ppe_mcast_l mcast_l;
	u16 mac_hi = DMAC_TO_HI16(mac);
	u32 mac_lo = DMAC_TO_LO32(mac);
	void __iomem *reg;

	mcast_h.u.value = 0;
	mcast_l.addr = 0;
	if (mac_hi == 0x0100)
		mcast_h.u.info.mc_mpre_sel = 0;
	else if (mac_hi == 0x3333)
		mcast_h.u.info.mc_mpre_sel = 1;
	else
		return -EINVAL;

	mcast_h.u.info.mc_px_en = pse_bmp;
	mcast_l.addr = mac_lo;

	if (index < 0x10) {
		reg = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_H_0 + ((index) * 8);
		writel(mcast_h.u.value, reg);
		reg = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_L_0 + ((index) * 8);
		writel(mcast_l.addr, reg);
	} else {
		index = index - 0x10;
		reg = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_H_10 + ((index) * 8);
		writel(mcast_h.u.value, reg);
		reg = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_L_10 + ((index) * 8);
		writel(mcast_l.addr, reg);
	}

	return 0;
}

static void clr_hnat_mtbl(int ppe_id, int index)
{
	void __iomem *reg;

	if (index < 0x10) {
		reg = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_H_0 + ((index) * 8);
		writel(0, reg);
		reg = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_L_0 + ((index) * 8);
		writel(0, reg);
	} else {
		index = index - 0x10;
		reg = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_H_10 + ((index) * 8);
		writel(0, reg);
		reg = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_L_10 + ((index) * 8);
		writel(0, reg);
	}
}

static int hnat_mcast_get_mac_from_mdb(struct br_mdb_entry *entry, u8 *mac)
{
	u32 ip;

	switch (ntohs(entry->addr.proto)) {
	case ETH_P_IP:
		ip = be32_to_cpu(entry->addr.u.ip4);
		mac[0] = 0x01;
		mac[1] = 0x00;
		mac[2] = 0x5e;
		mac[3] = (ip >> 16) & 0x7F;
		mac[4] = (ip >> 8) & 0xFF;
		mac[5] = ip & 0xFF;
		break;
	case ETH_P_IPV6:
		ip = be32_to_cpu(entry->addr.u.ip6.s6_addr32[3]);
		mac[0] = 0x33;
		mac[1] = 0x33;
		mac[2] = (ip >> 24) & 0xFF;
		mac[3] = (ip >> 16) & 0xFF;
		mac[4] = (ip >> 8) & 0xFF;
		mac[5] = ip & 0xFF;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static struct ppe_mcast_group *hnat_mcast_find_group_by_ip(const struct hnat_mcast_group_ip *key)
{
	struct ppe_mcast_group *group, *result = NULL;

	if (!key || !hnat_priv || !hnat_priv->pmcast)
		return NULL;

	list_for_each_entry(group, &hnat_priv->pmcast->groups, list) {
		if (key->is_ipv4) {
			if (group->is_ipv4 && group->dip == key->addr.ip) {
				result = group;
				break;
			}
		} else {
			if (!group->is_ipv4 &&
			    !memcmp(&group->dip6, &key->addr.ipv6, sizeof(struct in6_addr))) {
				result = group;
				break;
			}
		}
	}

	return result;
}

static void hnat_mcast_foe_mc_enable(struct foe_entry *entry, bool *clr_cache, bool to_npu)
{
	if (IS_IPV4_GRP(entry)) {
		entry->ipv4_hnapt.tport_id = 0;
		if (to_npu)
			entry->ipv4_hnapt.tops_entry = HNAT_NPU_ENTRY_MC;
		entry->ipv4_hnapt.iblk2.mcast = 1;
		entry->ipv4_hnapt.iblk2.dp = 0;
	} else if (IS_IPV6_GRP(entry)) {
		entry->ipv6_5t_route.tport_id = 0;
		if (to_npu)
			entry->ipv6_5t_route.tops_entry = HNAT_NPU_ENTRY_MC;
		entry->ipv6_5t_route.iblk2.mcast = 1;
		entry->ipv6_5t_route.iblk2.dp = 0;
	}
	entry->bfib1.vlan_layer = 0;
	entry->bfib1.vpm = 0;
	*clr_cache = true;
}

static void hnat_mcast_foe_stag_update(struct foe_entry *foe, u8 stag, bool *clr_cache)
{
	if (IS_IPV4_GRP(foe))
		foe->ipv4_hnapt.sp_tag = stag;
	else if (IS_IPV6_GRP(foe))
		foe->ipv6_5t_route.sp_tag = stag;

	foe->bfib1.vlan_layer = 1;
	*clr_cache = true;
}

static void hnat_mcast_foe_mc_disable(struct foe_entry *foe, u32 pse_bmp, bool *clr_cache)
{
	int dp;

	switch (__ffs(pse_bmp)) {
	case MCAST_TO_GDMA1:
		dp = NR_GMAC1_PORT;
		break;
	case MCAST_TO_GDMA2:
		dp = NR_GMAC2_PORT;
		break;
	case MCAST_TO_GDMA3:
		dp = NR_GMAC3_PORT;
		break;
	case MCAST_TO_TDMA:
		dp = NR_TDMA_PORT;
		break;
	default:
		if (debug_level >= 5)
			pr_info("Invalid pse_bmp=%x\n", pse_bmp);
		return;
	}

	if (IS_IPV4_GRP(foe)) {
		foe->ipv4_hnapt.iblk2.dp = dp;
		foe->ipv4_hnapt.iblk2.mcast = 0;
		foe->ipv4_hnapt.tport_id = 0;
		if (dp == NR_TDMA_PORT)
			foe->ipv4_hnapt.tops_entry = 30;
		else
			foe->ipv4_hnapt.tops_entry = 0;
	} else if (IS_IPV6_GRP(foe)) {
		foe->ipv6_5t_route.iblk2.dp = dp;
		foe->ipv6_5t_route.iblk2.mcast = 0;
		foe->ipv6_5t_route.tport_id = 0;
		if (dp == NR_TDMA_PORT)
			foe->ipv6_5t_route.tops_entry = 30;
		else
			foe->ipv6_5t_route.tops_entry = 0;
	}

	foe->bfib1.vpm = 0;
	if (dp == NR_GMAC1_PORT)
		foe->bfib1.vlan_layer = 1;
	else
		foe->bfib1.vlan_layer = 0;

	*clr_cache = true;
}

static int hnat_mcast_alloc_mtbl_entry(int ppe_id)
{
	int idx;
	void __iomem *reg_h, *reg_l;
	u32 val_h, val_l;

	for (idx = 0; idx < hnat_priv->pmcast->max_entry; idx++) {
		if (idx < 0x10) {
			reg_h = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_H_0 + (idx * 8);
			reg_l = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_L_0 + (idx * 8);
		} else {
			reg_h = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_H_10 + ((idx - 0x10) * 8);
			reg_l = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_L_10 + ((idx - 0x10) * 8);
		}

		val_h = readl(reg_h);
		val_l = readl(reg_l);

		if (val_h == 0 && val_l == 0)
			return idx;
	}

	return -1;
}

static int hnat_mcast_mtbl_setup(struct ppe_mcast_group *group,
				 u32 pse_bmp, struct foe_entry *foe,
				 bool *clr_cache, bool to_npu)
{
	u32 bmp = 0;

	if (group->mtbl_idx == -1) {
		group->mtbl_idx = hnat_mcast_alloc_mtbl_entry(group->ppe_id);
		if (group->mtbl_idx == -1) {
			memset(foe, 0, sizeof(*foe));
			group->foe_idx = -1;
			group->ppe_id = -1;
			*clr_cache = true;
			return -1;
		}
	}

	if (to_npu) {
		bmp = (pse_bmp & ~(1 << MCAST_TO_GDMA1)) | (1 << MCAST_TO_TDMA);
		set_hnat_mtbl(group->mac, group->mtbl_idx, group->ppe_id, bmp);
	} else
		set_hnat_mtbl(group->mac, group->mtbl_idx, group->ppe_id, pse_bmp);

	return 0;
}

static void hnat_mcast_mtbl_teardown(struct ppe_mcast_group *group)
{
	if (group->mtbl_idx != INVLD_IDX && group->ppe_id != INVLD_IDX) {
		clr_hnat_mtbl(group->ppe_id, group->mtbl_idx);
		group->mtbl_idx = INVLD_IDX;
	}
}

static struct foe_entry *hnat_mcast_get_hw_foe(struct ppe_mcast_group *group)
{
	if (group->ppe_id != -1 && group->foe_idx != -1)
		return &hnat_priv->foe_table_cpu[group->ppe_id][group->foe_idx];
	else
		return NULL;
}

static int hnat_mcast_set_gmac_bit(struct net_device *dev, u32 *pse_bmp)
{
	struct mtk_mac *mac;

	mac = netdev_priv(dev);
	switch (HNAT_GMAC_FP(mac->id)) {
	case NR_GMAC1_PORT:
		*pse_bmp |= (1 << MCAST_TO_GDMA1);
		break;
	case NR_GMAC2_PORT:
		*pse_bmp |= (1 << MCAST_TO_GDMA2);
		break;
	case NR_GMAC3_PORT:
		*pse_bmp |= (1 << MCAST_TO_GDMA3);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int hnat_mcast_handle_dsa(struct net_device *dev, u32 *pse_bmp, u32 *stag)
{
	const unsigned int *port_reg;
	int port_index = -1;

	port_reg = of_get_property(dev->dev.of_node, "reg", NULL);
	if (unlikely(!port_reg))
		return -EINVAL;

	port_index = be32_to_cpup(port_reg);
	*pse_bmp |= (1 << MCAST_TO_GDMA1);
	*stag |= BIT(port_index);

	return 0;
}

static u32 hnat_mcast_get_single_dev_pse_bit(int ifindex, u32 *stag)
{
	struct net_device *dev;
	u32 pse_bmp = 0;

	dev = dev_get_by_index(&init_net, ifindex);
	if (!dev)
		return 0;

	if (IS_DSA_LAN(dev))
		hnat_mcast_handle_dsa(dev, &pse_bmp, stag);
	else if (IS_WHNAT(dev))
		pse_bmp |= (1 << MCAST_TO_TDMA);
	else if (IS_LAN_GRP(dev) || IS_WAN(dev))
		hnat_mcast_set_gmac_bit(dev, &pse_bmp);

	dev_put(dev);
	return pse_bmp;
}

static u32 hnat_mcast_get_pse_tx_bmp(struct ppe_mcast_group *group, u32 *stag)
{
	struct net_device *dev = NULL;
	struct ppe_mcast_member *member;
	u32 pse_bmp = 0;

	list_for_each_entry(member, &group->members, list) {
		dev = dev_get_by_index(&init_net, member->ifindex);
		if (!dev)
			continue;

		if (!netif_running(dev)) {
			dev_put(dev);
			continue;
		}

		if (IS_DSA_LAN(dev)) {
			hnat_mcast_handle_dsa(dev, &pse_bmp, stag);
			dev_put(dev);
			continue;
		}

		if (IS_WHNAT(dev))
			pse_bmp |= (1 << MCAST_TO_TDMA);
		else if (IS_LAN_GRP(dev) || IS_WAN(dev))
			hnat_mcast_set_gmac_bit(dev, &pse_bmp);

		dev_put(dev);
	}

	return pse_bmp;
}

static void hnat_mcast_prepare_offload_info(struct mcast_offload_info *mc_info,
					    struct ppe_mcast_group *group,
					    struct foe_entry *foe)
{
	memset(mc_info, 0, sizeof(*mc_info));

	mc_info->dest = HNAT_NPU_MCAST_DEST_SWITCH;
	mc_info->m2u_en = 0;
	memcpy(mc_info->daddr, group->mac, ETH_ALEN);

	if (IS_IPV4_GRP(foe)) {
		mc_info->is_ipv6 = 0;
		mc_info->dst.ip = htonl(foe->ipv4_hnapt.dip);
	} else if (IS_IPV6_GRP(foe)) {
		mc_info->is_ipv6 = 1;
		mc_info->dst.ipv6.s6_addr32[0] = htonl(foe->ipv6_5t_route.ipv6_dip0);
		mc_info->dst.ipv6.s6_addr32[1] = htonl(foe->ipv6_5t_route.ipv6_dip1);
		mc_info->dst.ipv6.s6_addr32[2] = htonl(foe->ipv6_5t_route.ipv6_dip2);
		mc_info->dst.ipv6.s6_addr32[3] = htonl(foe->ipv6_5t_route.ipv6_dip3);
	}
}

static u32 hnat_mcast_get_dsa_port(int ifindex)
{
	struct net_device *dev = NULL;
	const unsigned int *port_reg;
	int port_index = INVLD_IDX;

	dev = dev_get_by_index(&init_net, ifindex);
	if (!dev)
		return INVLD_IDX;

	if (!IS_DSA_LAN(dev)) {
		dev_put(dev);
		return INVLD_IDX;
	}

	port_reg = of_get_property(dev->dev.of_node, "reg", NULL);
	if (port_reg)
		port_index = be32_to_cpup(port_reg);

	dev_put(dev);
	return port_index;
}

static int hnat_mcast_npu_delete_single(struct mcast_offload_info *mc_info, int port)
{
	int ret;

	if (!hnat_mc_ops || !hnat_mc_ops->npu_mcast_client_delete)
		return 0;

	if (port >= 0)
		mc_info->dsa_port = port;

	if (debug_level >= 5)
		pr_info("Del client dip:%pI4/%pI6c dsaport:%d is6:%d\n",
			&mc_info->dst.ip, &mc_info->dst.ipv6,
			mc_info->dsa_port, mc_info->is_ipv6);
	ret = hnat_mc_ops->npu_mcast_client_delete(mc_info);
	if (ret)
		pr_info("Del client fail ret:%d\n", ret);

	return ret;
}

static int hnat_mcast_npu_delete_multi(struct mcast_offload_info *mc_info, u32 stag)
{
	int port_bit;
	int ret = 0;
	int first_error = 0;

	for (port_bit = 0; port_bit < 8; port_bit++) {
		if (stag & (1 << port_bit)) {
			mc_info->dsa_port = port_bit;
			ret = hnat_mcast_npu_delete_single(mc_info, port_bit);
			if (ret && !first_error)
				first_error = ret;
		}
	}

	return ret;
}

static int hnat_mcast_npu_delete_stations(struct mcast_offload_info *mc_info,
					  u32 stag, bool is_dsa)
{
	int ret = 0;

	if (!hnat_mc_ops || !hnat_mc_ops->npu_mcast_client_delete)
		return 0;

	if (is_dsa)
		ret = hnat_mcast_npu_delete_single(mc_info, -1);
	else
		ret = hnat_mcast_npu_delete_multi(mc_info, stag);

	return ret;
}

static int hnat_mcast_npu_add_multi(struct ppe_mcast_group *group,
				    struct mcast_offload_info *mc_info, u32 stag)
{
	int port_bit;
	int ret = 0;

	for (port_bit = 0; port_bit < 8; port_bit++) {
		if (stag & (1 << port_bit)) {
			mc_info->dsa_port = port_bit;
			ret = hnat_mc_ops->npu_mcast_client_insert(mc_info);
			if (!ret)
				group->npu_grp_idx = mc_info->npu_grp_idx;
			else
				group->offload = false;

			if (debug_level >= 5)
				pr_info("Add client dip:%pI4/%pI6c port:%d grp:%d hw:%d is6:%d\n",
					&mc_info->dst.ip, &mc_info->dst.ipv6,
					mc_info->dsa_port, group->npu_grp_idx,
					group->offload, mc_info->is_ipv6);
		}
	}

	return ret;
}

static int hnat_mcast_npu_add_stations(struct ppe_mcast_group *group,
				       struct mcast_offload_info *mc_info,
				       u32 stag, bool is_dsa, u32 pse_bmp)
{
	int ret = 0;

	if (!hnat_mc_ops || !hnat_mc_ops->npu_mcast_client_insert)
		return 0;

	if (is_dsa) {
		ret = hnat_mc_ops->npu_mcast_client_insert(mc_info);
		/* When the IGMP-controlled Join remains active,if multicast
		 * data transmission stops and then resumes, HNAT will notify
		 * NPU to add the same client again. In this case, the NPU
		 * returns -EEXIST, which is expected behavior.
		 */
		if (ret == -EEXIST)
			ret = 0;

		if (!ret)
			group->npu_grp_idx = mc_info->npu_grp_idx;
		else
			group->offload = false;

		if (debug_level >= 5)
			pr_info("Add client dip:%pI4/%pI6c port:%d grp:%d hw:%d is6:%d\n",
				&mc_info->dst.ip, &mc_info->dst.ipv6,
				mc_info->dsa_port, group->npu_grp_idx,
				group->offload, mc_info->is_ipv6);
	} else {
		if (!BMP_IS_SINGLE_PORT(pse_bmp))
			ret = hnat_mcast_npu_add_multi(group, mc_info, stag);
	}

	return ret;
}

static int hnat_mcast_group_teardown_npu(struct ppe_mcast_group *group)
{
	struct foe_entry *foe;
	struct mcast_offload_info mc_info;
	u32 stag = 0, pse_bmp = 0;
	int ret = 0;

	if (!group) {
		pr_info("Teardown npu Invalid group\n");
		return -EINVAL;
	}

	foe = hnat_mcast_get_hw_foe(group);
	if (!foe) {
		pr_info("Teardown npu No FOE entry in group %pM\n",
			group->mac);
		return -ENOENT;
	}

	hnat_mcast_prepare_offload_info(&mc_info, group, foe);

	pse_bmp = hnat_mcast_get_pse_tx_bmp(group, &stag);

	ret = hnat_mcast_npu_delete_stations(&mc_info, stag, false);

	group->psebmp = 0;

	return ret;
}

static int hnat_mcast_npu_update(struct ppe_mcast_group *group,
				 struct mcast_offload_info *mc_info,
				 u32 stag, u32 pse_bmp,
				 int type, bool is_dsa)
{
	bool gdma1_in_cur = !!(pse_bmp & (1 << MCAST_TO_GDMA1));
	bool gdma1_in_pre = !!(group->psebmp & (1 << MCAST_TO_GDMA1));
	bool pre_bmp_multi = !BMP_IS_SINGLE_PORT(group->psebmp) &&
				(group->psebmp != 0);
	bool cur_bmp_multi = !BMP_IS_SINGLE_PORT(pse_bmp) && (pse_bmp != 0);
	int ret = 0;

	if (type == RTM_NEWMDB) {
		if (gdma1_in_cur && cur_bmp_multi) {
			/* e.g.(gdm1 gdm3) -> tdma join    skip
			 */
			if (gdma1_in_pre && pre_bmp_multi && !is_dsa)
				return 0;
			/* e.g.(gdm1)      -> gdm3 join    add all
			 * e.g.(gdm1 gdm3) -> lanx join    add lanx
			 * e.g.(tdma gdm3) -> lanx join    add lanx
			 */
			ret = hnat_mcast_npu_add_stations(group, mc_info, stag, is_dsa, pse_bmp);
		}
	} else if (type == RTM_DELMDB) {
		if (BMP_IS_SINGLE_PORT(pse_bmp)) {
			/* e.g.(gdm1 gdm3) -> gdm3 Leave    del all
			 * e.g.(gdm1 tdma) -> tdma Leave    del all
			 */
			if (gdma1_in_pre && pre_bmp_multi)
				ret = hnat_mcast_npu_delete_stations(mc_info, stag, is_dsa);
		} else {
			/* e.g.(gdm1 gdm3)      -> lanx Leave    del lanx
			 * e.g.(gdm1 tdma gdm3) -> lanx Leave    del lanx
			 */
			if (is_dsa && (gdma1_in_cur || gdma1_in_pre))
				ret = hnat_mcast_npu_delete_single(mc_info, -1);
		}
	}

	return ret;
}

static int hnat_mcast_foe_mtbl_update(struct ppe_mcast_group *group,
				      struct foe_entry *foe,
				      u32 pse_bmp, bool *clr_cache)
{
	if (BMP_IS_SINGLE_PORT(pse_bmp)) {
		hnat_mcast_foe_mc_disable(foe, pse_bmp, clr_cache);
		hnat_mcast_mtbl_teardown(group);
	} else {
		if (!hnat_mcast_mtbl_setup(group, pse_bmp, foe, clr_cache, 1))
			hnat_mcast_foe_mc_enable(foe, clr_cache, 1);
	}

	if (clr_cache && (foe->bfib1.state == BIND))
		hnat_cache_ebl(1);

	return 0;
}

static void hnat_mcast_no_npu_teardown_group(struct ppe_mcast_group *group,
					     u32 pse_bmp, int *ret)
{
	entry_delete(group->ppe_id, group->foe_idx);
	hnat_mcast_mtbl_teardown(group);
	group->psebmp  = 0;
	group->foe_idx = INVLD_IDX;
	group->ppe_id  = INVLD_IDX;
	*ret = -1;
}

static void hnat_mcast_no_npu_gsw_multi_eth(struct ppe_mcast_group *group,
					    struct foe_entry *foe,
					    u32 pse_bmp,
					    bool *clr_cache, int *ret)
{
	if (!hnat_mcast_mtbl_setup(group, pse_bmp, foe, clr_cache, 0))
		hnat_mcast_foe_mc_enable(foe, clr_cache, 0);

	if (*clr_cache && (foe->bfib1.state == BIND))
		hnat_cache_ebl(1);

	group->psebmp = pse_bmp;
	*ret = 0;
}

static int hnat_mcast_npu_gsw_handle(struct ppe_mcast_group *group,
				     struct foe_entry *foe,
				     u8 stag, u32 pse_bmp)
{
	bool clr_cache = false;

	if (BMP_IS_SINGLE_PORT(pse_bmp)) {
		hnat_mcast_foe_mc_disable(foe, pse_bmp, &clr_cache);
		hnat_mcast_mtbl_teardown(group);
	} else {
		if (!hnat_mcast_mtbl_setup(group, pse_bmp, foe, &clr_cache, 0))
			hnat_mcast_foe_mc_enable(foe, &clr_cache, 0);
	}

	if (clr_cache && (foe->bfib1.state == BIND))
		hnat_cache_ebl(1);

	group->psebmp = pse_bmp;

	return 0;
}

static bool hnat_mcast_is_mtk_dsa(const char *dev_name)
{
	struct net_device *dev;
	bool result = false;

	dev = dev_get_by_name(&init_net, dev_name);
	if (!dev)
		return false;

	if (netdev_uses_dsa(dev) &&
		dev->dsa_ptr->tag_ops->proto == DSA_TAG_PROTO_MTK)
		result = true;

	dev_put(dev);

	return result;
}

static void hnat_mcast_no_npu_single_gdma(struct ppe_mcast_group *group,
					  struct foe_entry *foe,
					  u8 stag, u32 pse_bmp,
					  bool *clr_cache, int *ret)
{
	hnat_mcast_foe_stag_update(foe, stag, clr_cache);
	hnat_mcast_foe_mc_disable(foe, pse_bmp, clr_cache);
	hnat_mcast_mtbl_teardown(group);

	if (*clr_cache && (foe->bfib1.state == BIND))
		hnat_cache_ebl(1);

	group->psebmp = pse_bmp;
	*ret = 0;
}

static void hnat_mcast_no_npu_multi_handle(struct ppe_mcast_group *group,
					  struct foe_entry *foe,
					  u32 pse_bmp, bool is_multi_eth,
					  bool *clr_cache, int *ret)
{
	if (hnat_mcast_is_mtk_dsa("eth0")) {
		hnat_mcast_no_npu_teardown_group(group, pse_bmp, ret);
		return;
	}

	if (is_multi_eth)
		hnat_mcast_no_npu_gsw_multi_eth(group, foe, pse_bmp,
				       clr_cache, ret);
	else
		hnat_mcast_no_npu_teardown_group(group, pse_bmp, ret);
}

static bool hnat_mcast_no_npu_handle(struct ppe_mcast_group *group,
				      struct foe_entry *foe,
				      u8 stag, u32 pse_bmp,
				      bool *clr_cache, int *ret)
{
	bool is_single, is_wifi_only, is_gdma_only, is_multi, is_multi_eth;

	if (hnat_mc_ops)
		return false;

	is_single    = BMP_IS_SINGLE_PORT(pse_bmp);
	is_wifi_only = is_single && (pse_bmp & (1 << MCAST_TO_TDMA));
	is_gdma_only = is_single && !is_wifi_only;
	is_multi     = !is_single && (pse_bmp != 0);
	is_multi_eth = is_multi && !(pse_bmp & (1 << MCAST_TO_TDMA));

	if (is_wifi_only || is_multi)
		hnat_mcast_no_npu_multi_handle(group, foe, pse_bmp, is_multi_eth, clr_cache, ret);
	else if (is_gdma_only)
		hnat_mcast_no_npu_single_gdma(group, foe, stag, pse_bmp, clr_cache, ret);
	else
		*ret = 0;

	return true;
}

static int hnat_mcast_npu_dsa_handle(struct ppe_mcast_group *group,
				    struct foe_entry *foe,
				    u8 stag, u32 pse_bmp,
				    int type, int ifindex)
{
	struct mcast_offload_info mc_info;
	bool clr_cache = false;
	bool is_dsa;
	int ret = 0;
	u32 dsaport = INVLD_IDX;

	hnat_mcast_foe_stag_update(foe, stag, &clr_cache);

	hnat_mcast_prepare_offload_info(&mc_info, group, foe);

	dsaport = hnat_mcast_get_dsa_port(ifindex);
	mc_info.dsa_port = dsaport;
	is_dsa = (dsaport != INVLD_IDX);

	ret = hnat_mcast_npu_update(group, &mc_info, stag, pse_bmp, type, is_dsa);

	hnat_mcast_foe_mtbl_update(group, foe, pse_bmp, &clr_cache);

	group->psebmp = pse_bmp;

	return ret;
}

static int hnat_mcast_hw_update(struct ppe_mcast_group *group,
				struct foe_entry *foe,
				u8 stag, u32 pse_bmp,
				int type, int ifindex)
{
	bool clr_cache = false;
	int ret = 0;

	if (hnat_mcast_no_npu_handle(group, foe, stag, pse_bmp,
				&clr_cache, &ret))
		return ret;

	if (hnat_mcast_is_mtk_dsa("eth0"))
		ret = hnat_mcast_npu_dsa_handle(group, foe, stag, pse_bmp, type, ifindex);
	else
		ret = hnat_mcast_npu_gsw_handle(group, foe, stag, pse_bmp);

	return ret;
}

static int hnat_mcast_event_hw_handle(struct ppe_mcast_group *group, int type, int ifindex)
{
	struct foe_entry *foe = NULL;
	u32 pse_bmp = 0;
	u32 stag = 0;
	int ret = 0;

	foe = hnat_mcast_get_hw_foe(group);
	if (!foe)
		return -ENOENT;

	if ((!IS_IPV4_GRP(foe)) && (!IS_IPV6_GRP(foe)))
		return -EINVAL;

	pse_bmp = hnat_mcast_get_pse_tx_bmp(group, &stag);
	if (debug_level >= 5)
		pr_info("event handle.mac:%pM type:%d ppe:%d foe:%d bmp:%x\n",
			group->mac, type, group->ppe_id, group->foe_idx, pse_bmp);

	if (pse_bmp)
		ret = hnat_mcast_hw_update(group, foe, stag, pse_bmp, type, ifindex);
	else {
		ret = entry_delete(group->ppe_id, group->foe_idx);
		group->psebmp = 0;
		group->ppe_id = INVLD_IDX;
		group->foe_idx = INVLD_IDX;
	}

	return ret;
}

static bool hnat_mcast_no_npu_teardown(struct ppe_mcast_group *group)
{
	u32 pre_bmp = 0, pre_stag = 0;
	bool pre_single, pre_wifi_only, pre_multi;

	if (hnat_mc_ops)
		return false;

	pre_bmp	  = hnat_mcast_get_pse_tx_bmp(group, &pre_stag);
	pre_single	  = BMP_IS_SINGLE_PORT(pre_bmp);
	pre_wifi_only = pre_single && (pre_bmp & (1 << MCAST_TO_TDMA));
	pre_multi	  = !pre_single && (pre_bmp != 0);

	return (pre_wifi_only || pre_multi) &&
		group->foe_idx == INVLD_IDX &&
		group->ppe_id  == INVLD_IDX;
}

int hnat_mcast_foe_bind_handle(const u8 *dmac,
	u32 ppe_id, u32 foe_idx, struct foe_entry *entry, int ifindex)
{
	u32 pse_bmp = 0;
	u32 stag = 0;
	int ret = 0;
	struct ppe_mcast_group *group = NULL;
	struct hnat_mcast_group_ip key;

	memset(&key, 0, sizeof(key));
	if ((!IS_IPV4_GRP(entry)) && (!IS_IPV6_GRP(entry)))
		return -1;

	if (!hnat_priv->pmcast)
		return -1;

	if (IS_IPV4_GRP(entry)) {
		key.is_ipv4 = true;
		key.addr.ip = htonl(entry->ipv4_hnapt.dip);
	} else {
		key.is_ipv4 = false;
		key.addr.ipv6.s6_addr32[0] = htonl(entry->ipv6_5t_route.ipv6_dip0);
		key.addr.ipv6.s6_addr32[1] = htonl(entry->ipv6_5t_route.ipv6_dip1);
		key.addr.ipv6.s6_addr32[2] = htonl(entry->ipv6_5t_route.ipv6_dip2);
		key.addr.ipv6.s6_addr32[3] = htonl(entry->ipv6_5t_route.ipv6_dip3);
	}

	write_lock_bh(&hnat_priv->pmcast->mcast_lock);
	group = hnat_mcast_find_group_by_ip(&key);
	if (!group) {
		write_unlock_bh(&hnat_priv->pmcast->mcast_lock);
		return -1;
	}

	if (!(group->offload)) {
		write_unlock_bh(&hnat_priv->pmcast->mcast_lock);
		return -1;
	}

	if (hnat_mcast_no_npu_teardown(group)) {
		write_unlock_bh(&hnat_priv->pmcast->mcast_lock);
		return -1;
	}

	group->ppe_id = ppe_id;
	group->foe_idx = foe_idx;
	if (IS_IPV4_GRP(entry))
		group->sip = htonl(entry->ipv4_hnapt.sip);
	else if (IS_IPV6_GRP(entry)) {
		group->sip6.s6_addr32[0] = htonl(entry->ipv6_5t_route.ipv6_sip0);
		group->sip6.s6_addr32[1] = htonl(entry->ipv6_5t_route.ipv6_sip1);
		group->sip6.s6_addr32[2] = htonl(entry->ipv6_5t_route.ipv6_sip2);
		group->sip6.s6_addr32[3] = htonl(entry->ipv6_5t_route.ipv6_sip3);
	}

	pse_bmp = hnat_mcast_get_pse_tx_bmp(group, &stag);

	if (debug_level >= 5)
		pr_info("foe bind ifidx:%d mac:%pM ppe:%d foe_idx:%d dip:%pI4 sip:%pI4 dip6:%pI6c sip6:%pI6c\n",
			ifindex, dmac, group->ppe_id, group->foe_idx, &group->dip, &group->sip,
			&group->dip6, &group->sip6);

	if (pse_bmp) {
		ret = hnat_mcast_hw_update(group, entry, stag, pse_bmp, RTM_NEWMDB, ifindex);
		if (ret < 0) {
			write_unlock_bh(&hnat_priv->pmcast->mcast_lock);
			return -1;
		}
	}

	if (!group->offload) {
		write_unlock_bh(&hnat_priv->pmcast->mcast_lock);
		return -1;
	}

	write_unlock_bh(&hnat_priv->pmcast->mcast_lock);

	return 0;
}

void hnat_mcast_ifdown_handle(int ifindex)
{
	struct ppe_mcast_table *pmcast = hnat_priv->pmcast;
	struct ppe_mcast_group *group, *tmp;
	struct ppe_mcast_member *member;
	struct foe_entry *foe;
	u32 down_bmp, new_bmp, new_stag, dummy_stag;
	bool member_found;

	if (!pmcast)
		return;

	write_lock_bh(&pmcast->mcast_lock);
	list_for_each_entry_safe(group, tmp, &pmcast->groups, list) {
		member_found = false;
		list_for_each_entry(member, &group->members, list) {
			if (member->ifindex == ifindex) {
				member_found = true;
				break;
			}
		}
		if (!member_found)
			continue;

		foe = hnat_mcast_get_hw_foe(group);
		if (!foe)
			continue;

		dummy_stag = 0;
		down_bmp = hnat_mcast_get_single_dev_pse_bit(ifindex, &dummy_stag);
		new_bmp = group->psebmp & ~down_bmp;

		if (debug_level >= 5)
			pr_info("ifdwn idx:%d dbmp:0x%x obmp:0x%x nbmp:0x%x\n",
				ifindex, down_bmp, group->psebmp, new_bmp);

		if (new_bmp) {
			new_stag = 0;
			new_bmp = hnat_mcast_get_pse_tx_bmp(group, &new_stag);
			hnat_mcast_hw_update(group, foe, new_stag, new_bmp,
						   RTM_DELMDB, ifindex);
		} else {
			/* No members left: tear down everything */
			entry_delete(group->ppe_id, group->foe_idx);
			hnat_mcast_mtbl_teardown(group);
			group->psebmp = 0;
			group->ppe_id = INVLD_IDX;
			group->foe_idx = INVLD_IDX;
		}
	}
	write_unlock_bh(&pmcast->mcast_lock);
}

static struct ppe_mcast_group *hnat_mcast_create_group(const u8 *dmac, struct br_mdb_entry *entry)
{
	struct ppe_mcast_group *group;

	group = kzalloc(sizeof(*group), GFP_ATOMIC);
	if (!group)
		return NULL;

	memcpy(group->mac, dmac, ETH_ALEN);
	INIT_LIST_HEAD(&group->members);
	group->mtbl_idx = INVLD_IDX;
	group->foe_idx = INVLD_IDX;
	group->ppe_id = INVLD_IDX;
	group->npu_grp_idx = INVLD_IDX;
	group->offload = true;

	if (ntohs(entry->addr.proto) == ETH_P_IP) {
		group->is_ipv4 = true;
		group->dip = entry->addr.u.ip4;
	} else if (ntohs(entry->addr.proto) == ETH_P_IPV6) {
		group->is_ipv4 = false;
		group->dip6 = entry->addr.u.ip6;
	}
	list_add_tail(&group->list, &hnat_priv->pmcast->groups);

	return group;
}

static int hnat_mcast_group_add_station(struct ppe_mcast_group *group, u32 ifindex)
{
	struct ppe_mcast_member *member = NULL;

	member = kzalloc(sizeof(*member), GFP_ATOMIC);
	if (!member)
		return -ENOMEM;

	member->ifindex = ifindex;
	list_add_tail(&member->list, &group->members);

	return 0;
}

static int hnat_mcast_group_del_station(struct ppe_mcast_group *group, u32 ifindex)
{
	struct ppe_mcast_member *member = NULL, *m;

	list_for_each_entry_safe(member, m, &group->members, list) {
		if (ifindex == member->ifindex) {
			list_del(&member->list);
			kfree(member);
			break;
		}
	}

	return 0;
}

static int hnat_mcast_group_del(struct ppe_mcast_group *group)
{
	if (list_empty(&group->members)) {
		list_del(&group->list);
		kfree(group);
	}

	return 0;
}

static int hnat_mcast_group_update(const u8 *dmac, int type, struct br_mdb_entry *entry)
{
	struct ppe_mcast_group *group = NULL;
	struct hnat_mcast_group_ip key;
	int ret = 0;
	bool is_new_group = false;

	memset(&key, 0, sizeof(key));
	if ((ntohs(entry->addr.proto) != ETH_P_IP) &&
		(ntohs(entry->addr.proto) != ETH_P_IPV6))
		return -EPROTO;

	if ((type != RTM_NEWMDB) && (type != RTM_DELMDB))
		return -EINVAL;

	if (!hnat_priv->pmcast)
		return -1;

	if (ntohs(entry->addr.proto) == ETH_P_IP) {
		key.is_ipv4 = true;
		key.addr.ip = entry->addr.u.ip4;
	} else {
		key.is_ipv4 = false;
		key.addr.ipv6 = entry->addr.u.ip6;
	}

	write_lock_bh(&hnat_priv->pmcast->mcast_lock);
	group = hnat_mcast_find_group_by_ip(&key);
	if (!group) {
		group = hnat_mcast_create_group(dmac, entry);
		is_new_group = true;
	}

	if (!group) {
		write_unlock_bh(&hnat_priv->pmcast->mcast_lock);
		return -ENOMEM;
	}

	if (type == RTM_NEWMDB) {
		if (hnat_mcast_group_add_station(group, entry->ifindex)) {
			if (is_new_group) {
				list_del(&group->list);
				kfree(group);
			}
			write_unlock_bh(&hnat_priv->pmcast->mcast_lock);
			return -ENOMEM;
		}
	} else
		hnat_mcast_group_del_station(group, entry->ifindex);

	if (mcast_hook_toggle)
		ret = hnat_mcast_event_hw_handle(group, type, entry->ifindex);

	/* Delete group if no member in group */
	hnat_mcast_group_del(group);

	write_unlock_bh(&hnat_priv->pmcast->mcast_lock);

	return ret;
}

static void hnat_mcast_nlmsg_handler(struct work_struct *work)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;
	struct nlattr *nest, *nest2, *info;
	struct br_port_msg *bpm;
	struct br_mdb_entry *entry;
	struct ppe_mcast_table *pmcast;
	struct sock *sk;
	u8 dmac[ETH_ALEN] = {0};

	pmcast = container_of(work, struct ppe_mcast_table, work);
	sk = pmcast->msock->sk;
	while ((skb = skb_dequeue(&sk->sk_receive_queue))) {
		nlh = nlmsg_hdr(skb);
		if (!nlmsg_ok(nlh, skb->len)) {
			kfree_skb(skb);
			continue;
		}
		bpm = nlmsg_data(nlh);
		nest = nlmsg_find_attr(nlh, sizeof(bpm), MDBA_MDB);
		if (!nest) {
			kfree_skb(skb);
			continue;
		}
		nest2 = nla_find_nested(nest, MDBA_MDB_ENTRY);
		if (nest2) {
			info = nla_find_nested(nest2, MDBA_MDB_ENTRY_INFO);
			if (!info) {
				kfree_skb(skb);
				continue;
			}

			entry = (struct br_mdb_entry *)nla_data(info);
			hnat_mcast_get_mac_from_mdb(entry, dmac);
			if (is_multicast_ether_addr(dmac))
				hnat_mcast_group_update(dmac, nlh->nlmsg_type, entry);
		}
		kfree_skb(skb);
	}
}

static void hnat_mcast_nlmsg_rcv(struct sock *sk)
{
	struct ppe_mcast_table *pmcast = hnat_priv->pmcast;
	struct workqueue_struct *queue = pmcast->queue;
	struct work_struct *work = &pmcast->work;

	queue_work(queue, work);
}

static struct socket *hnat_mcast_netlink_open(struct net *net)
{
	struct socket *sock = NULL;
	int ret;
	struct sockaddr_nl addr;

	ret = sock_create_kern(net, PF_NETLINK, SOCK_RAW, NETLINK_ROUTE, &sock);
	if (ret < 0)
		goto out;

	sock->sk->sk_data_ready = hnat_mcast_nlmsg_rcv;
	addr.nl_family = PF_NETLINK;
	addr.nl_pid = 65536; /*fix me:how to get an unique id?*/
	addr.nl_groups = RTMGRP_MDB;
	ret = sock->ops->bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0)
		goto out;

	return sock;
out:
	if (sock)
		sock_release(sock);

	return NULL;
}

static void hnat_mcast_check_timestamp(struct timer_list *t)
{
	struct foe_entry *entry;
	int i, hash_index;
	u16 e_ts, foe_ts;

	for (i = 0; i < CFG_PPE_NUM; i++) {
		for (hash_index = 0; hash_index < hnat_priv->foe_etry_num; hash_index++) {
			entry = hnat_priv->foe_table_cpu[i] + hash_index;
			if (entry->bfib1.sta == 1) {
				e_ts = (entry->ipv4_hnapt.m_timestamp) & 0xffff;
				foe_ts = foe_timestamp(hnat_priv, true);
				if ((foe_ts - e_ts) > FOE_TS_WRAP_THRESHOLD)
					foe_ts = (~(foe_ts)) & 0xffff;
				if (abs(foe_ts - e_ts) > FOE_TS_MAX_DIFF)
					entry_delete(i, hash_index);
			}
		}
	}
	mod_timer(&hnat_priv->hnat_mcast_check_timer, jiffies + 10 * HZ);
}

static bool hnat_mcast_ip_match_blist(struct ppe_mcast_group *group,
				      struct mcast_blist_data *blist)
{
	bool found = false;

	if (blist->is_ipv4) {
		u32 group_ip = ntohl(group->dip);
		u32 blist_ip = ntohl(blist->ipv4);

		found = (group_ip & blist->mask) == (blist_ip & blist->mask);
	} else {
		u32 remaining = blist->mask;
		int i;

		found = true;
		for (i = 0; i < 4; i++) {
			u32 group_word = ntohl(group->dip6.s6_addr32[i]);
			u32 blist_word = ntohl(blist->ipv6.s6_addr32[i]);
			u32 word_mask;

			if (remaining == 0)
				break;
			else if (remaining >= 32)
				word_mask = 0xffffffff;
			else
				word_mask = 0xffffffff << (32 - remaining);

			if ((group_word & word_mask) != (blist_word & word_mask)) {
				found = false;
				break;
			}

			remaining = (remaining > 32) ? (remaining - 32) : 0;
		}
	}

	return found;
}

static bool hnat_mcast_blist_entry_match(struct mcast_blist_data *blist,
					 u8 *ip, u32 mask, bool is_ipv4)
{
	u32 check_mask = (0xffffffff << (32 - mask));

	if (is_ipv4) {
		return (blist->is_ipv4 &&
			(blist->ipv4 == *(u32 *)ip) &&
			(blist->mask == check_mask));
	} else {
		return (!blist->is_ipv4 &&
			!memcmp(&blist->ipv6, ip, sizeof(struct in6_addr)) &&
			(blist->mask == check_mask));
	}
}

static struct mcast_blist_data *hnat_mcast_blist_create_entry(u8 *ip, u32 mask,
							      bool is_ipv4)
{
	struct mcast_blist_data *mcast;

	mcast = kzalloc(sizeof(struct mcast_blist_data), GFP_ATOMIC);
	if (!mcast)
		return NULL;

	if (is_ipv4) {
		mcast->is_ipv4 = true;
		mcast->ipv4 = *(u32 *)ip;
	} else {
		mcast->is_ipv4 = false;
		memcpy(&mcast->ipv6, ip, sizeof(struct in6_addr));
	}
	mcast->mask = 0xffffffff << (32 - mask);

	return mcast;
}

int hnat_mcast_blist_handle(u8 *ip, u32 mask, bool add, bool is_ipv4)
{
	struct mtk_hnat *h = hnat_priv;
	struct mcast_blist_data *m = NULL, *mcast = NULL, *next = NULL;
	struct ppe_mcast_table *pmcast = hnat_priv->pmcast;
	struct ppe_mcast_group *group, *tmp;
	bool found = false;

	write_lock_bh(&pmcast->mcast_lock);
	if (add) {
		list_for_each_entry(m, &h->mcast_blist_list, list) {
			if (hnat_mcast_blist_entry_match(m, ip, mask, is_ipv4)) {
				write_unlock_bh(&pmcast->mcast_lock);
				return 0;
			}
		}

		mcast = hnat_mcast_blist_create_entry(ip, mask, is_ipv4);
		if (!mcast) {
			write_unlock_bh(&pmcast->mcast_lock);
			return -ENOMEM;
		}

		list_add(&mcast->list, &h->mcast_blist_list);

		list_for_each_entry_safe(group, tmp, &pmcast->groups, list) {
			/* Check if group IP matches the blacklist entry (with mask) */
			if (hnat_mcast_ip_match_blist(group, mcast)) {
				hnat_mcast_group_teardown_npu(group);
				entry_delete(group->ppe_id, group->foe_idx);
				group->psebmp = 0;
				group->ppe_id = INVLD_IDX;
				group->foe_idx = INVLD_IDX;
			}
		}
	} else {
		/* blacklist remove*/
		list_for_each_entry_safe(m, next, &h->mcast_blist_list, list) {
			if (hnat_mcast_blist_entry_match(m, ip, mask, is_ipv4)) {
				list_del(&m->list);
				kfree(m);
				found = true;
				break;
			}
		}
		if (!found)
			pr_info("Blist entry not found\n");
	}
	write_unlock_bh(&pmcast->mcast_lock);

	return 0;
}

static int hnat_mcast_group_handle(struct ppe_mcast_group *group, bool enable)
{
	struct ppe_mcast_table *pmcast = hnat_priv->pmcast;
	struct mcast_offload_info mc_info;
	struct foe_entry *foe = NULL;
	u32 pse_bmp = 0, stag = 0;

	if (!pmcast)
		return -EINVAL;

	foe = hnat_mcast_get_hw_foe(group);
	if (!foe)
		return -ENOENT;

	hnat_mcast_prepare_offload_info(&mc_info, group, foe);

	pse_bmp = hnat_mcast_get_pse_tx_bmp(group, &stag);

	if (!enable) {
		entry_delete(group->ppe_id, group->foe_idx);
		group->psebmp = 0;
		group->ppe_id = INVLD_IDX;
		group->foe_idx = INVLD_IDX;
		hnat_mcast_npu_delete_stations(&mc_info, stag, false);
	}

	return 0;
}

int hnat_mcast_offload_handle(bool enable)
{
	struct ppe_mcast_table *pmcast = hnat_priv->pmcast;
	struct ppe_mcast_group *group, *tmp;

	mcast_hook_toggle = enable ? 1 : 0;

	if (!pmcast)
		return -1;

	write_lock_bh(&pmcast->mcast_lock);
	list_for_each_entry_safe(group, tmp, &pmcast->groups, list) {
		hnat_mcast_group_handle(group, enable);
	}
	write_unlock_bh(&pmcast->mcast_lock);
	return 0;
}

int hnat_mcast_mcport_ppse_map_init(u32 ppe_id)
{
	if (!hnat_priv || ppe_id >= CFG_PPE_NUM || !hnat_priv->ppe_base[ppe_id])
		return -EINVAL;

	/* Enable multicast table lookup */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_GLO_CFG, MCAST_TB_EN, 1);
	/* multicast port0 map to PDMA */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MCAST_PPSE, MC_P0_PPSE, NR_PDMA_PORT);
	/* multicast port1 map to GMAC1 */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MCAST_PPSE, MC_P1_PPSE, NR_GMAC1_PORT);
	/* multicast port2 map to GMAC2 */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MCAST_PPSE, MC_P2_PPSE, NR_GMAC2_PORT);
	/* multicast port3 map to TDMA */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MCAST_PPSE, MC_P3_PPSE, NR_TDMA_PORT);
	/* multicast port4 map to GMAC3 */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MCAST_PPSE, MC_P4_PPSE, NR_GMAC3_PORT);

	return 0;
}

int hnat_mcast_ser_handle(void)
{
	struct ppe_mcast_table *pmcast = NULL;
	struct ppe_mcast_group *group, *tmp;
	int i, ret;

	if (!hnat_priv || !hnat_priv->pmcast)
		return -EINVAL;

	pmcast = hnat_priv->pmcast;
	write_lock_bh(&pmcast->mcast_lock);
	list_for_each_entry_safe(group, tmp, &pmcast->groups, list) {
		group->psebmp = 0;
		group->mtbl_idx = INVLD_IDX;
		group->ppe_id = INVLD_IDX;
		group->foe_idx = INVLD_IDX;
	}
	write_unlock_bh(&pmcast->mcast_lock);

	for (i = 0; i < CFG_PPE_NUM; i++) {
		ret = hnat_mcast_mcport_ppse_map_init(i);
		if (ret) {
			pr_warn("hnat: init mcast map for ppe %d fail\n", i);
			return -EINVAL;
		}
	}

	return 0;
}

int hnat_mcast_enable(u32 ppe_id)
{
	struct ppe_mcast_table *pmcast;

	if (ppe_id >= CFG_PPE_NUM)
		return -EINVAL;

	if (!hnat_priv->pmcast) {
		pmcast = kzalloc(sizeof(*pmcast), GFP_KERNEL);
		if (!pmcast)
			return -1;

		INIT_LIST_HEAD(&pmcast->groups);
		rwlock_init(&pmcast->mcast_lock);

		if (hnat_priv->data->version == MTK_HNAT_V1_1)
			pmcast->max_entry = 0x10;
		else
			pmcast->max_entry = MAX_MCAST_ENTRY;

		INIT_WORK(&pmcast->work, hnat_mcast_nlmsg_handler);
		pmcast->queue = create_singlethread_workqueue("ppe_mcast");
		if (!pmcast->queue)
			goto err1;

		pmcast->msock = hnat_mcast_netlink_open(&init_net);
		if (!pmcast->msock)
			goto err2;

		hnat_priv->pmcast = pmcast;

		/* mt7629 should checkout mcast entry life time manualy */
		if (hnat_priv->data->version == MTK_HNAT_V1_3) {
			timer_setup(&hnat_priv->hnat_mcast_check_timer,
				    hnat_mcast_check_timestamp, 0);
			hnat_priv->hnat_mcast_check_timer.expires = jiffies;
			add_timer(&hnat_priv->hnat_mcast_check_timer);
		}
	}

	hnat_mcast_mcport_ppse_map_init(ppe_id);
	mtk_hnat_mcast_blist = hnat_mcast_blist_handle;
	mtk_hnat_mcast_offload = hnat_mcast_offload_handle;

	return 0;
err2:
	if (pmcast->queue)
		destroy_workqueue(pmcast->queue);
err1:
	kfree(pmcast);

	return -1;
}

int hnat_mcast_disable(void)
{
	struct ppe_mcast_table *pmcast = hnat_priv->pmcast;
	struct ppe_mcast_group *group, *tmp;
	struct ppe_mcast_member *member = NULL, *m;
	struct mcast_blist_data *mb = NULL, *next = NULL;
	int i;

	if (!pmcast)
		return -EINVAL;

	if (hnat_priv->data->version == MTK_HNAT_V1_3)
		del_timer_sync(&hnat_priv->hnat_mcast_check_timer);

	/* Disable multicast table lookup */
	for (i = 0; i < CFG_PPE_NUM; i++)
		cr_set_field(hnat_priv->ppe_base[i] + PPE_GLO_CFG, MCAST_TB_EN, 0);

	flush_work(&pmcast->work);
	destroy_workqueue(pmcast->queue);
	sock_release(pmcast->msock);
	list_for_each_entry_safe(group, tmp, &pmcast->groups, list) {
		list_for_each_entry_safe(member, m, &group->members, list) {
			list_del(&member->list);
			kfree(member);
		}
		list_del(&group->list);
		kfree(group);
	}

	list_for_each_entry_safe(mb, next, &hnat_priv->mcast_blist_list, list) {
		list_del(&mb->list);
		kfree(mb);
	}

	kfree(pmcast);
	hnat_priv->pmcast = NULL;
	mtk_hnat_mcast_blist = NULL;
	mtk_hnat_mcast_offload = NULL;

	return 0;
}

int mtk_npu_hnat_mcast_ops_register(struct npu_hnat_mcast_ops *hnat_mcast_ops)
{
	if (hnat_mc_ops)
		return -EBUSY;

	hnat_mc_ops = hnat_mcast_ops;
	return 0;
}
EXPORT_SYMBOL(mtk_npu_hnat_mcast_ops_register);

int mtk_npu_hnat_mcast_ops_unregister(void)
{
	hnat_mc_ops = NULL;
	return 0;
}
EXPORT_SYMBOL(mtk_npu_hnat_mcast_ops_unregister);

static void hnat_mcast_update_path_from_wifi(int grp_id, bool hw_path)
{
	struct hnat_mcast_group_info grp_info;
	struct hnat_mcast_group_ip key;
	struct ppe_mcast_group *group;
	int ret;

	memset(&key, 0, sizeof(key));
	memset(&grp_info, 0x0, sizeof(grp_info));
	grp_info.grp_id = grp_id;
	if (mtk_wifi_mcast_get_group_info) {
		ret = mtk_wifi_mcast_get_group_info(HNAT_MCAST_INFO_IP, (void *)(&grp_info));
		if (!ret) {
			if (grp_info.gip.is_ipv4) {
				key.is_ipv4 = true;
				key.addr.ip = grp_info.gip.addr.ip;
			} else {
				key.is_ipv4 = false;
				memcpy(&key.addr.ipv6, &grp_info.gip.addr.ipv6,
					sizeof(struct in6_addr));
			}

			group = hnat_mcast_find_group_by_ip(&key);
			if (group) {
				group->offload = hw_path;
				if (!hw_path) {
					entry_delete(group->ppe_id, group->foe_idx);
					group->psebmp = 0;
					group->ppe_id = INVLD_IDX;
					group->foe_idx = INVLD_IDX;
				}
			} else
				pr_info(" Get group:%d from local fail\n", grp_id);
		} else
			pr_info(" Get group:%d from wifi fail\n", grp_id);
	}
}

static void hnat_mcast_group_path_switch(int group_idx, bool hw_path)
{
	struct ppe_mcast_table *pmcast;
	struct ppe_mcast_group *group, *tmp;

	if (!hnat_priv || !hnat_priv->pmcast)
		return;

	pmcast = hnat_priv->pmcast;
	list_for_each_entry_safe(group, tmp, &pmcast->groups, list) {
		if (group->npu_grp_idx == group_idx) {
			group->offload = hw_path;
			if (!hw_path) {
				entry_delete(group->ppe_id, group->foe_idx);
				group->psebmp = 0;
				group->ppe_id = INVLD_IDX;
				group->foe_idx = INVLD_IDX;
			}
			return;
		}
	}

	pr_info("path switch group_idx %d not found\n", group_idx);
	hnat_mcast_update_path_from_wifi(group_idx, hw_path);

	return;
}

void hnat_notify_mcast_sw_path(int group_idx)
{
	hnat_mcast_group_path_switch(group_idx, false);
}
EXPORT_SYMBOL(hnat_notify_mcast_sw_path);

void hnat_notify_mcast_hw_path(int group_idx)
{
	hnat_mcast_group_path_switch(group_idx, true);
}
EXPORT_SYMBOL(hnat_notify_mcast_hw_path);

int hnat_register_pcie_link_hook(bool (*hook_func)(bool up))
{
	if (hnat_mcast_eth_ext_link_handle)
		return -EEXIST;
	hnat_mcast_eth_ext_link_handle = hook_func;
	return 0;
}
EXPORT_SYMBOL(hnat_register_pcie_link_hook);

void hnat_unregister_pcie_link_hook(void)
{
	hnat_mcast_eth_ext_link_handle = NULL;
}
EXPORT_SYMBOL(hnat_unregister_pcie_link_hook);


int hnat_register_wifi_info_hook(int (*hook_func)(u32 id, void *data))
{
	if (mtk_wifi_mcast_get_group_info)
		return -EEXIST;

	mtk_wifi_mcast_get_group_info = hook_func;
	return 0;
}
EXPORT_SYMBOL(hnat_register_wifi_info_hook);

void hnat_unregister_wifi_info_hook(void)
{
	mtk_wifi_mcast_get_group_info = NULL;
}
EXPORT_SYMBOL(hnat_unregister_wifi_info_hook);


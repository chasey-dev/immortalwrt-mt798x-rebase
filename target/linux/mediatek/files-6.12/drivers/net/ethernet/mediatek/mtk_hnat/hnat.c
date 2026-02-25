/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2014-2016 Sean Wang <sean.wang@mediatek.com>
 *   Copyright (C) 2016-2017 John Crispin <blogic@openwrt.org>
 */

#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/if.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/rtnetlink.h>
#include <net/netlink.h>

#include "nf_hnat_mtk.h"
#include "hnat.h"

struct mtk_hnat *hnat_priv;
static struct socket *_hnat_roam_sock;
static struct work_struct _hnat_roam_work;
static struct delayed_work _hnat_flow_entry_teardown_work;

int (*ra_sw_nat_hook_rx)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_rx);
int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no) = NULL;
EXPORT_SYMBOL(ra_sw_nat_hook_tx);
void (*ra_sw_nat_clear_bind_entries)(void) = NULL;
EXPORT_SYMBOL(ra_sw_nat_clear_bind_entries);

int (*hnat_get_wdma_tx_port)(u32 wdma_idx) = NULL;
EXPORT_SYMBOL(hnat_get_wdma_tx_port);
int (*hnat_get_wdma_rx_port)(u32 wdma_idx) = NULL;
EXPORT_SYMBOL(hnat_get_wdma_rx_port);

int (*ppe_del_entry_by_mac)(unsigned char *mac) = NULL;
EXPORT_SYMBOL(ppe_del_entry_by_mac);
int (*ppe_del_entry_by_ip)(bool is_ipv4, void *addr) = NULL;
EXPORT_SYMBOL(ppe_del_entry_by_ip);
int (*ppe_del_entry_by_bssid_wcid)(u32 wdma_idx, u16 bssid, u16 wcid) = NULL;
EXPORT_SYMBOL(ppe_del_entry_by_bssid_wcid);

void (*ppe_dev_register_hook)(struct net_device *dev) = NULL;
EXPORT_SYMBOL(ppe_dev_register_hook);
void (*ppe_dev_unregister_hook)(struct net_device *dev) = NULL;
EXPORT_SYMBOL(ppe_dev_unregister_hook);
int (*mtk_tnl_encap_offload)(struct sk_buff *skb, struct ethhdr *eth) = NULL;
EXPORT_SYMBOL(mtk_tnl_encap_offload);
int (*mtk_tnl_decap_offload)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(mtk_tnl_decap_offload);
bool (*mtk_tnl_decap_offloadable)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(mtk_tnl_decap_offloadable);
bool (*mtk_crypto_offloadable)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(mtk_crypto_offloadable);

int (*hnat_set_wdma_pse_port_state)(u32 wdma_idx, bool up) = NULL;
EXPORT_SYMBOL(hnat_set_wdma_pse_port_state);

static void hnat_sma_build_entry(struct timer_list *t)
{
	int i;

	for (i = 0; i < CFG_PPE_NUM; i++)
		cr_set_field(hnat_priv->ppe_base[i] + PPE_TB_CFG,
			     SMA, SMA_FWD_CPU_BUILD_ENTRY);
}

struct foe_entry *hnat_get_foe_entry(u32 ppe_id, u32 index)
{
	if (index == 0x7fff || index >= hnat_priv->foe_etry_num ||
	    ppe_id >= CFG_PPE_NUM)
		return ERR_PTR(-EINVAL);

	return &hnat_priv->foe_table_cpu[ppe_id][index];
}
EXPORT_SYMBOL(hnat_get_foe_entry);

static void hnat_reset_timestamp(struct timer_list *t)
{
	struct foe_entry *entry;
	int hash_index;

	hnat_cache_ebl(0);
	cr_set_field(hnat_priv->ppe_base[0] + PPE_TB_CFG, TCP_AGE, 0);
	cr_set_field(hnat_priv->ppe_base[0] + PPE_TB_CFG, UDP_AGE, 0);
	writel(0, hnat_priv->fe_base + 0x0010);

	for (hash_index = 0; hash_index < hnat_priv->foe_etry_num; hash_index++) {
		entry = hnat_priv->foe_table_cpu[0] + hash_index;
		if (entry->bfib1.state == BIND)
			entry->bfib1.time_stamp =
				readl(hnat_priv->fe_base + 0x0010) & (0xFFFF);
	}

	cr_set_field(hnat_priv->ppe_base[0] + PPE_TB_CFG, TCP_AGE, 1);
	cr_set_field(hnat_priv->ppe_base[0] + PPE_TB_CFG, UDP_AGE, 1);
	hnat_cache_ebl(1);

	mod_timer(&hnat_priv->hnat_reset_timestamp_timer, jiffies + 14400 * HZ);
}

void cr_set_bits(void __iomem *reg, u32 bs)
{
	u32 val = readl(reg);

	val |= bs;
	writel(val, reg);
}

void cr_clr_bits(void __iomem *reg, u32 bs)
{
	u32 val = readl(reg);

	val &= ~bs;
	writel(val, reg);
}

void cr_set_field(void __iomem *reg, u32 field, u32 val)
{
	unsigned int tv = readl(reg);

	tv &= ~field;
	tv |= ((val) << (ffs((unsigned int)field) - 1));
	writel(tv, reg);
}

/*boundary entry can't be used to accelerate data flow*/
void exclude_boundary_entry(struct foe_entry *foe_table_cpu)
{
	int entry_base = 0;
	int bad_entry, i, j;
	struct foe_entry *foe_entry;
	/*these entries are boundary every 128 entries*/
	int boundary_entry_offset[8] = { 12, 25, 38, 51, 76, 89, 102, 115};

	if (!foe_table_cpu)
		return;

	for (i = 0; entry_base < hnat_priv->foe_etry_num; i++) {
		/* set boundary entries as static*/
		for (j = 0; j < 8; j++) {
			bad_entry = entry_base + boundary_entry_offset[j];
			foe_entry = &foe_table_cpu[bad_entry];
			foe_entry->udib1.sta = 1;
		}
		entry_base = (i + 1) * 128;
	}
}

static int mtk_get_wdma_tx_port(u32 wdma_idx)
{
	if (wdma_idx == 0 || wdma_idx == 1 || wdma_idx == 2)
		return NR_PPE0_PORT;

	return -EINVAL;
}

static int mtk_get_wdma_rx_port(u32 wdma_idx)
{
	if (wdma_idx == 2)
		return NR_WDMA2_PORT;
	else if (wdma_idx == 1)
		return NR_WDMA1_PORT;
	else if (wdma_idx == 0)
		return NR_WDMA0_PORT;

	return -EINVAL;
}

static int mtk_set_wdma_pse_port_state(u32 wdma_idx, bool up)
{
	int port;

	port = mtk_get_wdma_rx_port(wdma_idx);
	if (port < 0)
		return -EINVAL;

	cr_set_field(hnat_priv->fe_base + MTK_FE_GLO_CFG(port),
		     MTK_FE_LINK_DOWN_P((u32)port), !up);

	return 0;
}

static int mtk_set_ppe_pse_port_state(u32 ppe_id, bool up)
{
	u32 port;

	if (ppe_id == 0)
		port = NR_PPE0_PORT;
	else if (ppe_id == 1)
		port = NR_PPE1_PORT;
	else if (ppe_id == 2)
		port = NR_PPE2_PORT;
	else
		return -EINVAL;

	cr_set_field(hnat_priv->fe_base + MTK_FE_GLO_CFG(port),
		     MTK_FE_LINK_DOWN_P(port), !up);

	return 0;
}

void set_gmac_ppe_fwd(int id, int enable)
{
	void __iomem *reg;
	u32 val;

	reg = hnat_priv->fe_base +
		((id == NR_GMAC1_PORT) ? GDMA1_FWD_CFG :
		 (id == NR_GMAC2_PORT) ? GDMA2_FWD_CFG : GDMA3_FWD_CFG);

	if (enable) {
#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
		if (l4s_toggle)
			cr_set_field(reg, GDM_ALL_FRC_MASK, BITS_GDM_ALL_FRC_P_TDMA);
		else if (CFG_PPE_NUM >= 3 && id == NR_GMAC3_PORT)
			cr_set_field(reg, GDM_ALL_FRC_MASK, BITS_GDM_ALL_FRC_P_PPE2);
		else if (CFG_PPE_NUM >= 2 && id == NR_GMAC2_PORT)
			cr_set_field(reg, GDM_ALL_FRC_MASK, BITS_GDM_ALL_FRC_P_PPE1);
		else
			cr_set_field(reg, GDM_ALL_FRC_MASK, BITS_GDM_ALL_FRC_P_PPE);
#else
		cr_set_field(reg, GDM_ALL_FRC_MASK, BITS_GDM_ALL_FRC_P_PPE);
#endif
		return;
	}

	/*disabled */
	val = readl(reg);
#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
	if ((CFG_PPE_NUM >= 2 &&
	    ((val & GDM_ALL_FRC_MASK) == BITS_GDM_ALL_FRC_P_PPE1 ||
	     (val & GDM_ALL_FRC_MASK) == BITS_GDM_ALL_FRC_P_PPE2)))
		cr_set_field(reg, GDM_ALL_FRC_MASK,
			     BITS_GDM_ALL_FRC_P_CPU_PDMA);
#endif

	if ((val & GDM_ALL_FRC_MASK) == BITS_GDM_ALL_FRC_P_PPE)
		cr_set_field(reg, GDM_ALL_FRC_MASK,
				 BITS_GDM_ALL_FRC_P_CPU_PDMA);

}

int entry_mac_cmp(struct foe_entry *entry, u8 *mac, enum entry_cmp_flags flags)
{
	u32 smac_hi, dmac_hi;
	u16 smac_lo, dmac_lo;

	if (IS_IPV4_GRP(entry)) {
		smac_hi = swab32(entry->ipv4_hnapt.smac_hi);
		smac_lo = swab16(entry->ipv4_hnapt.smac_lo);
		dmac_hi = swab32(entry->ipv4_hnapt.dmac_hi);
		dmac_lo = swab16(entry->ipv4_hnapt.dmac_lo);
	} else {
		smac_hi = swab32(entry->ipv6_5t_route.smac_hi);
		smac_lo = swab16(entry->ipv6_5t_route.smac_lo);
		dmac_hi = swab32(entry->ipv6_5t_route.dmac_hi);
		dmac_lo = swab16(entry->ipv6_5t_route.dmac_lo);
	}

	if ((flags & ENTRY_CMP_SRC) &&
	    (smac_hi == *(u32 *)mac) && (smac_lo == *(u16 *)&mac[4]))
		return 1;

	if ((flags & ENTRY_CMP_DST) &&
	    (dmac_hi == *(u32 *)mac) && (dmac_lo == *(u16 *)&mac[4]))
		return 1;

	return 0;
}

int entry_ip_cmp(struct foe_entry *entry,
		 bool is_ipv4,
		 void *addr,
		 enum entry_cmp_flags flags)
{
	struct in6_addr *tmp_ipv6;
	struct in6_addr ipv6 = {0};
	struct in6_addr foe_sipv6 = {0};
	struct in6_addr foe_dipv6 = {0};
	u32 *tmp_ipv4, ipv4;
	u32 foe_sipv4 = {0}, foe_dipv4 = {0};
	u32 *sipv6_0 = NULL;
	u32 *dipv6_0 = NULL;
	int ret = 0;

	if (is_ipv4) {
		tmp_ipv4 = (u32 *)addr;
		ipv4 = ntohl(*tmp_ipv4);

		switch ((int)entry->bfib1.pkt_type) {
		case IPV4_HNAPT:
		case IPV4_HNAT:
			foe_sipv4 = entry->ipv4_hnapt.sip;
			foe_dipv4 = entry->ipv4_hnapt.new_dip;
			break;
		case IPV4_DSLITE:
		case IPV4_MAP_E:
			foe_sipv4 = entry->ipv4_dslite.sip;
			foe_dipv4 = entry->ipv4_dslite.dip;
			break;
		case IPV6_6RD:
			foe_sipv4 = entry->ipv6_6rd.tunnel_sipv4;
			foe_dipv4 = entry->ipv6_6rd.tunnel_dipv4;
			break;
		default:
			return ret;
		}

		if (((flags & ENTRY_CMP_SRC) && (foe_sipv4 == ipv4)) ||
		    ((flags & ENTRY_CMP_DST) && (foe_dipv4 == ipv4)))
			ret = 1;
	} else {
		memset(&foe_sipv6, 0, sizeof(struct in6_addr));
		memset(&foe_dipv6, 0, sizeof(struct in6_addr));
		memset(&ipv6, 0, sizeof(struct in6_addr));

		tmp_ipv6 = (struct in6_addr *)addr;
		ipv6.s6_addr32[0] = ntohl(tmp_ipv6->s6_addr32[0]);
		ipv6.s6_addr32[1] = ntohl(tmp_ipv6->s6_addr32[1]);
		ipv6.s6_addr32[2] = ntohl(tmp_ipv6->s6_addr32[2]);
		ipv6.s6_addr32[3] = ntohl(tmp_ipv6->s6_addr32[3]);

		switch ((int)entry->bfib1.pkt_type) {
		case IPV6_3T_ROUTE:
		case IPV6_5T_ROUTE:
		case IPV6_6RD:
			sipv6_0 = &(entry->ipv6_3t_route.ipv6_sip0);
			dipv6_0 = &(entry->ipv6_3t_route.ipv6_dip0);
			break;
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
		case IPV6_HNAT:
		case IPV6_HNAPT:
			sipv6_0 = &(entry->ipv6_hnapt.ipv6_sip0);
			dipv6_0 = &(entry->ipv6_hnapt.new_ipv6_ip0);
			break;
#endif
		case IPV4_DSLITE:
		case IPV4_MAP_E:
			sipv6_0 = &(entry->ipv4_dslite.tunnel_sipv6_0);
			dipv6_0 = &(entry->ipv4_dslite.tunnel_dipv6_0);
			break;
		default:
			return ret;
		}

		if ((flags & ENTRY_CMP_SRC) && sipv6_0) {
			memcpy(&foe_sipv6, sipv6_0, sizeof(struct in6_addr));
			if (!memcmp(&foe_sipv6, &ipv6, sizeof(struct in6_addr)))
				ret = 1;
		}

		if ((flags & ENTRY_CMP_DST) && dipv6_0) {
			memcpy(&foe_dipv6, dipv6_0, sizeof(struct in6_addr));
			if (!memcmp(&foe_dipv6, &ipv6, sizeof(struct in6_addr)))
				ret = 1;
		}
	}
	return ret;
}

int entry_delete_by_mac(u8 *mac)
{
	struct foe_entry *entry = NULL;
	int index, i, ret = 0;
	int cnt;

	if (!mac) {
		if (debug_level >= 2)
			pr_warn("%s: invalid mac address\n", __func__);
		return 0;
	}

	for (i = 0; i < CFG_PPE_NUM; i++) {
		entry = hnat_priv->foe_table_cpu[i];
		cnt = 0;
		for (index = 0; index < DEF_ETRY_NUM; entry++, index++) {
			if (entry->bfib1.state == BIND &&
			    entry_mac_cmp(entry, mac, ENTRY_CMP_ANY)) {
				spin_lock_bh(&hnat_priv->entry_lock);
				__entry_delete(entry);
				spin_unlock_bh(&hnat_priv->entry_lock);
				if (debug_level >= 2)
					pr_info("[%s]: delete entry idx = %d_%d\n",
						__func__, i, index);
				cnt++;
			}
		}
		/* clear HWNAT cache */
		if (cnt > 0)
			hnat_cache_clr(i);
		ret += cnt;
	}

	if (!ret && debug_level >= 2)
		pr_info("%s: entry not found\n", __func__);

	return ret;
}

int entry_delete_by_ip(bool is_ipv4, void *addr)
{
	struct foe_entry *entry = NULL;
	int index, i, ret = 0;
	int cnt;

	if (!addr) {
		if (debug_level >= 2)
			pr_warn("%s: invalid ip address\n", __func__);
		return 0;
	}

	for (i = 0; i < CFG_PPE_NUM; i++) {
		entry = hnat_priv->foe_table_cpu[i];
		cnt = 0;
		for (index = 0; index < DEF_ETRY_NUM; entry++, index++) {
			if (entry->bfib1.state == BIND &&
			    entry_ip_cmp(entry, is_ipv4, addr, ENTRY_CMP_ANY)) {
				spin_lock_bh(&hnat_priv->entry_lock);
				__entry_delete(entry);
				spin_unlock_bh(&hnat_priv->entry_lock);
				if (debug_level >= 2)
					pr_info("[%s]: delete entry idx = %d_%d\n",
						__func__, i, index);
				cnt++;
			}
		}
		/* clear HWNAT cache */
		if (cnt > 0)
			hnat_cache_clr(i);
		ret += cnt;
	}

	if (!ret && debug_level >= 2)
		pr_info("%s: entry not found\n", __func__);

	return ret;
}

static int entry_delete_by_bssid_wcid(u32 wdma_idx, u16 bssid, u16 wcid)
{
	struct foe_entry *entry = NULL;
	int index, i;
	int ret = 0;
	int port;
	int cnt;

	port = mtk_get_wdma_rx_port(wdma_idx);

	if (port < 0)
		return -EINVAL;

	for (i = 0; i < CFG_PPE_NUM; i++) {
		entry = hnat_priv->foe_table_cpu[i];
		cnt = 0;
		for (index = 0; index < DEF_ETRY_NUM; entry++, index++) {
			if (entry->bfib1.state != BIND)
				continue;

			if (IS_IPV4_GRP(entry)) {
				if (entry->ipv4_hnapt.winfo.bssid != bssid ||
				    entry->ipv4_hnapt.winfo.wcid != wcid ||
				    entry->ipv4_hnapt.iblk2.dp != port)
					continue;
			} else if (IS_IPV4_MAPE(entry) || IS_IPV4_MAPT(entry)) {
				if (entry->ipv4_mape.winfo.bssid != bssid ||
				    entry->ipv4_mape.winfo.wcid != wcid ||
				    entry->ipv4_mape.iblk2.dp != port)
					continue;
			} else if (IS_IPV6_HNAPT(entry) || IS_IPV6_HNAT(entry)) {
				if (entry->ipv6_hnapt.winfo.bssid != bssid ||
				    entry->ipv6_hnapt.winfo.wcid != wcid ||
				    entry->ipv6_hnapt.iblk2.dp != port)
					continue;
			} else {
				if (entry->ipv6_5t_route.winfo.bssid != bssid ||
				    entry->ipv6_5t_route.winfo.wcid != wcid ||
				    entry->ipv6_5t_route.iblk2.dp != port)
					continue;
			}

			spin_lock_bh(&hnat_priv->entry_lock);
			__entry_delete(entry);
			spin_unlock_bh(&hnat_priv->entry_lock);
			if (debug_level >= 2)
				pr_info("[%s]: delete entry idx = %d_%d\n",
					__func__, i, index);
			cnt++;
		}
		/* clear HWNAT cache */
		if (cnt > 0)
			hnat_cache_clr(i);
		ret += cnt;
	}

	if (!ret && debug_level >= 2)
		pr_info("%s: entry not found\n", __func__);

	return ret;
}

static void hnat_roam_handler(struct work_struct *work)
{
	struct kvec iov;
	struct msghdr msg;
	struct nlmsghdr *nlh;
	struct ndmsg *ndm;
	struct nlattr *nla;
	u8 rcv_buf[512];
	int len;

	if (!_hnat_roam_sock)
		return;

	iov.iov_base = rcv_buf;
	iov.iov_len = sizeof(rcv_buf);
	memset(&msg, 0, sizeof(msg));
	msg.msg_namelen = sizeof(struct sockaddr_nl);

	len = kernel_recvmsg(_hnat_roam_sock, &msg, &iov, 1, iov.iov_len, 0);
	if (len <= 0)
		goto out;

	nlh = (struct nlmsghdr *)rcv_buf;
	if (!NLMSG_OK(nlh, len) || nlh->nlmsg_type != RTM_NEWNEIGH)
		goto out;

	len = nlh->nlmsg_len - NLMSG_HDRLEN;
	ndm = (struct ndmsg *)NLMSG_DATA(nlh);
	if (ndm->ndm_family != PF_BRIDGE)
		goto out;

	nla = (struct nlattr *)((u8 *)ndm + sizeof(struct ndmsg));
	len -= NLMSG_LENGTH(sizeof(struct ndmsg));
	while (nla_ok(nla, len)) {
		if (nla_type(nla) == NDA_LLADDR)
			entry_delete_by_mac(nla_data(nla));
		nla = nla_next(nla, &len);
	}

out:
	schedule_work(&_hnat_roam_work);
}

static int hnat_roaming_enable(void)
{
	struct socket *sock = NULL;
	struct sockaddr_nl addr;
	int ret;

	INIT_WORK(&_hnat_roam_work, hnat_roam_handler);

	ret = sock_create_kern(&init_net, AF_NETLINK, SOCK_RAW, NETLINK_ROUTE, &sock);
	if (ret < 0)
		goto out;

	_hnat_roam_sock = sock;

	addr.nl_family = AF_NETLINK;
	addr.nl_pad = 0;
	addr.nl_pid = 65534;
	addr.nl_groups = 1 << (RTNLGRP_NEIGH - 1);
	ret = kernel_bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0)
		goto out;

	schedule_work(&_hnat_roam_work);
	pr_info("hnat roaming work enable\n");

	return 0;
out:
	if (sock)
		sock_release(sock);

	return ret;
}

static void hnat_roaming_disable(void)
{
	if (_hnat_roam_sock)
		sock_release(_hnat_roam_sock);
	_hnat_roam_sock = NULL;
	pr_info("hnat roaming work disable\n");
}

static void hnat_flow_entry_teardown_all(u32 ppe_id)
{
	struct hnat_flow_entry *flow_entry;
	struct hlist_head *head;
	struct hlist_node *n;
	int index;

	spin_lock_bh(&hnat_priv->flow_entry_lock);
	for (index = 0; index < DEF_ETRY_NUM / 4; index++) {
		head = &hnat_priv->foe_flow[ppe_id][index];
		hlist_for_each_entry_safe(flow_entry, n, head, list) {
			hnat_flow_entry_delete(flow_entry);
		}
	}
	spin_unlock_bh(&hnat_priv->flow_entry_lock);
}

static void hnat_flow_entry_teardown_handler(struct work_struct *work)
{
	struct hnat_flow_entry *flow_entry;
	struct hlist_head *head;
	struct hlist_node *n;
	int index, i;
	u32 cnt = 0;

	spin_lock_bh(&hnat_priv->flow_entry_lock);
	for (i = 0; i < CFG_PPE_NUM; i++) {
		for (index = 0; index < DEF_ETRY_NUM / 4; index++) {
			head = &hnat_priv->foe_flow[i][index];
			hlist_for_each_entry_safe(flow_entry, n, head, list) {
				/* If the entry has not been used for 30 seconds, teardown it. */
				if (time_after(jiffies, flow_entry->last_update + 30 * HZ)) {
					hnat_flow_entry_delete(flow_entry);
					cnt++;
				}
			}
		}
	}
	spin_unlock_bh(&hnat_priv->flow_entry_lock);

	if (debug_level >= 2 && cnt > 0)
		pr_info("[%s]: Teardown %d entries\n", __func__, cnt);

	schedule_delayed_work(&_hnat_flow_entry_teardown_work, 1 * HZ);
}

static void hnat_flow_entry_teardown_enable(void)
{
	INIT_DELAYED_WORK(&_hnat_flow_entry_teardown_work, hnat_flow_entry_teardown_handler);
	schedule_delayed_work(&_hnat_flow_entry_teardown_work, 1 * HZ);
}

static void hnat_flow_entry_teardown_disable(void)
{
	cancel_delayed_work(&_hnat_flow_entry_teardown_work);
}

static int is_cah_ctrl_request_done(u32 ppe_id)
{
	int count = 1000;

	if (ppe_id >= CFG_PPE_NUM)
		return 0;

	/* waiting for 1sec to make sure action was finished */
	do {
		if ((readl(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL) & CAH_REQ) == 0)
			return 1;
		udelay(1000);
	} while (--count);

	return 0;
}

int hnat_search_cache_line(u32 ppe_id, u32 tag)
{
	u32 tag_srh = 0;
	int line = 0;

	if (ppe_id >= CFG_PPE_NUM || tag >= hnat_priv->foe_etry_num)
		return -EINVAL;

	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_TAG_SRH, TAG_SRH, tag);
	/* software access cache command = tag search */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_CMD, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_REQ, 1);

	if (is_cah_ctrl_request_done(ppe_id)) {
		tag_srh = readl(hnat_priv->ppe_base[ppe_id] + PPE_CAH_TAG_SRH);
		/* tag search miss */
		if (!((tag_srh >> 31) & 0x1))
			return -1;
		line = (tag_srh >> 16) & 0x7f;
	} else {
		pr_warn("%s line search timeout %d_%d\n", __func__, ppe_id, tag);
		return -EBUSY;
	}

	return line;
}

static void __hnat_write_cache_line(u32 ppe_id, u32 line, u32 tag, u32 state, u32 *data)
{
	int i;

	if (ppe_id >= CFG_PPE_NUM || line >= MAX_PPE_CACHE_NUM) {
		pr_warn("%s: invalid ppe_id or line %d_%d\n", __func__, ppe_id, line);
		return;
	}

	if (state > 3) {
		pr_warn("%s: invalid cache line state %d\n", __func__, state);
		return;
	}

	if (data == NULL)
		goto skip_data_write;

	/* write data filed of the cache line */
	for (i = 0; i < sizeof(struct foe_entry) / 4; i++) {
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_LINE_RW, LINE_RW, line);
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_LINE_RW, OFFSET_RW, i / 4);
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_DATA_SEL, i % 4);
#else
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_LINE_RW, OFFSET_RW, i);
#endif
		writel(data[i], hnat_priv->ppe_base[ppe_id] + PPE_CAH_WDATA);
		/* software access cache command = write */
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_CMD, 3);
		/* trigger software access cache request */
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_REQ, 1);
		if (!is_cah_ctrl_request_done(ppe_id))
			pr_warn("%s write data timeout in line %d_%d\n",
				__func__, ppe_id, line);
	}

skip_data_write:
	/* write tag filed of the cache line */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_LINE_RW, LINE_RW, line);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_LINE_RW, OFFSET_RW, 0x1F);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_DATA_SEL, 0);
	writel((state << 16) | tag, hnat_priv->ppe_base[ppe_id] + PPE_CAH_WDATA);
	/* software access cache command = write */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_CMD, 3);
	/* trigger software access cache request */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_REQ, 1);
	if (!is_cah_ctrl_request_done(ppe_id))
		pr_warn("%s write tag timeout in line %d_%d\n",
			__func__, ppe_id, line);
}

int hnat_write_cache_line(u32 ppe_id, int line, u32 tag, u32 state, u32 *data)
{
	u32 scan_mode;
	u32 flow_cfg;
	u32 cah_en;
	u32 i;

	if (ppe_id >= CFG_PPE_NUM || line >= MAX_PPE_CACHE_NUM) {
		pr_warn("%s: invalid ppe_id or line %d_%d\n", __func__, ppe_id, line);
		return -EINVAL;
	}

	if (state > 3) {
		pr_warn("%s: invalid cache line state %d\n", __func__, state);
		return -EINVAL;
	}

	/* disable table learning */
	flow_cfg = readl(hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG);
	writel(0, hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG);

	/* wait PPE return to idle */
	udelay(1);

	/* disable scan mode */
	scan_mode = FIELD_GET(SCAN_MODE, readl(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG));
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, SCAN_MODE, 0);

	/* disable cache */
	cah_en = readl(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL) & CAH_EN;
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_EN, 0);

	if (line < 0) {
		for (i = 0; i < MAX_PPE_CACHE_NUM; i++)
			__hnat_write_cache_line(ppe_id, i, tag, state, data);
	} else {
		__hnat_write_cache_line(ppe_id, line, tag, state, data);
	}

	/* restore cache enable */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_EN, cah_en);

	/* restore scan mode */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, SCAN_MODE, scan_mode);

	/* restore table learning */
	writel(flow_cfg, hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG);

	return 0;
}

static int __hnat_dump_cache_entry(u32 ppe_id, int line)
{
	static const char * const cache_status[] = { "INVALID", "VALID", "DIRTY", "LOCK" };
	u32 data[32] = {0};
	int tag, status;
	int i;

	if (ppe_id >= CFG_PPE_NUM || line >= MAX_PPE_CACHE_NUM) {
		pr_warn("%s: invalid ppe_id or line %d_%d\n", __func__, ppe_id, line);
		return -EINVAL;
	}

	/* Get the cache line status and tag */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_LINE_RW, LINE_RW, line);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_LINE_RW, OFFSET_RW, 0x1F);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_DATA_SEL, 0);
	/* software access cache command = read */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_CMD, 2);
	/* trigger software access cache request */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_REQ, 1);
	if (!is_cah_ctrl_request_done(ppe_id)) {
		pr_warn("%s: read tag timeout %d_%d\n", __func__, ppe_id, line);
		return -EBUSY;
	}

	tag = readl(hnat_priv->ppe_base[ppe_id] + PPE_CAH_RDATA) & 0xFFFF;
	status = (readl(hnat_priv->ppe_base[ppe_id] + PPE_CAH_RDATA) >> 16) & 0x3;

	pr_info("==========<PPE Table Cache Entry=%d_%d, line:%d, status:%s >===============\n",
		ppe_id, tag, line, cache_status[status]);

	for (i = 0; i < sizeof(struct foe_entry) / 4; i++) {
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_LINE_RW, LINE_RW, line);
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_LINE_RW, OFFSET_RW, i / 4);
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_DATA_SEL, i % 4);
#else
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_LINE_RW, OFFSET_RW, i);
#endif
		/* software access cache command = read */
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_CMD, 2);
		/* trigger software access cache request */
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_REQ, 1);

		if (is_cah_ctrl_request_done(ppe_id))
			data[i] = readl(hnat_priv->ppe_base[ppe_id] + PPE_CAH_RDATA);
		else
			pr_warn("%s: read data timeout %d_%d\n", __func__, ppe_id, line);
	}

	for (i = 0; i < 4; i++) {
		pr_info("%08x %08x %08x %08x %08x %08x %08x %08x\n",
			data[i * 8], data[i * 8 + 1], data[i * 8 + 2], data[i * 8 + 3],
			data[i * 8 + 4], data[i * 8 + 5], data[i * 8 + 6], data[i * 8 + 7]);
	}

	pr_info("=========================================\n");

	return 0;
}

int hnat_dump_cache_entry(u32 ppe_id, int hash)
{
	u32 scan_mode;
	u32 cah_en;
	int line;
	int i;

	if (ppe_id >= CFG_PPE_NUM) {
		pr_warn("%s: invalid ppe_id %d\n", __func__, ppe_id);
		return -EINVAL;
	}

	/* disable scan mode */
	scan_mode = FIELD_GET(SCAN_MODE, readl(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG));
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, SCAN_MODE, 0);
	/* disable ppe cache */
	cah_en = readl(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL) & CAH_EN;
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_EN, 0);

	if (hash < 0) {
		for (i = 0; i < MAX_PPE_CACHE_NUM; i++)
			__hnat_dump_cache_entry(ppe_id, i);
	} else {
		line = hnat_search_cache_line(ppe_id, hash);
		if (line < 0 || __hnat_dump_cache_entry(ppe_id, line) < 0)
			pr_warn("%s: cache line of entry %d_%d not found!\n",
				__func__, ppe_id, hash);
	}

	/* restore cache enable */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_EN, cah_en);
	/* restore scan mode */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, SCAN_MODE, scan_mode);

	return 0;
}

int hnat_dump_ppe_entry(u32 ppe_id, u32 hash)
{
	struct foe_entry *entry;
	int i;

	if (ppe_id >= CFG_PPE_NUM || hash >= hnat_priv->foe_etry_num) {
		pr_warn("%s: invalid ppe_id or hash %d_%d\n", __func__, ppe_id, hash);
		return -1;
	}

	entry = hnat_get_foe_entry(ppe_id, hash);
	pr_info("====================PPE Entry %d_%d HEX DUMP========================\n",
		ppe_id, hash);
	for (i = 0; i < sizeof(*entry) / 4; i += 8) {
		pr_info("%08x %08x %08x %08x %08x %08x %08x %08x\n",
			*((u32 *)entry + i + 0), *((u32 *)entry + i + 1),
			*((u32 *)entry + i + 2), *((u32 *)entry + i + 3),
			*((u32 *)entry + i + 4), *((u32 *)entry + i + 5),
			*((u32 *)entry + i + 6), *((u32 *)entry + i + 7));
	}

	return 0;
}

static irqreturn_t hnat_handle_fe_irq2(int irq, void *priv)
{
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
	struct ppe_flow_chk_status *fcs;
	u32 irq_status, chk_status;
	u32 ppe_id;

	irq_status = readl(hnat_priv->fe_base + MTK_FE_INT_STATUS2);
	if (irq_status & MTK_FE_INT2_PPE0_FLOW_CHK) {
		ppe_id = 0;

		writel(MTK_FE_INT2_PPE0_FLOW_CHK, hnat_priv->fe_base + MTK_FE_INT_STATUS2);
	} else if ((irq_status & MTK_FE_INT2_PPE1_FLOW_CHK) && (CFG_PPE_NUM > 1)) {
		ppe_id = 1;
		writel(MTK_FE_INT2_PPE1_FLOW_CHK, hnat_priv->fe_base + MTK_FE_INT_STATUS2);
	} else {
		return IRQ_NONE;
	}

	chk_status = readl(hnat_priv->ppe_base[ppe_id] - 0x200 + PPE_FLOW_CHK_STATUS);
	fcs = (struct ppe_flow_chk_status *)(&chk_status);
	pr_info("PPE%d_FLOW_CHK_IRQ HIT! status=0x%08x\n", ppe_id, chk_status);
	pr_info("ENTRY=%d|STC=%d|STATE=%d|SP=%d|FP=%d|CAH=%d|RMT=%d|PSN=%d|DRAM=%d|VALID=%d\n",
		fcs->entry, fcs->sta, fcs->state, fcs->sp, fcs->fp, fcs->cah, fcs->rmt,
		fcs->psn, fcs->dram, fcs->valid);

	if (hnat_dump_cache_entry(ppe_id, fcs->entry) < 0)
		pr_warn("Failed to dump cache entry %d_%d!\n", ppe_id, fcs->entry);

	if (hnat_dump_ppe_entry(ppe_id, fcs->entry) < 0)
		pr_warn("Failed to dump ppe entry %d_%d!\n", ppe_id, fcs->entry);

	return IRQ_HANDLED;
#endif
	return IRQ_NONE;
}

void __hnat_cache_clr(u32 ppe_id)
{
	static const u32 mask = BIT_ALERT_TCP_FIN_RST_SYN |
				BIT_MD_TOAP_BYP_CRSN0 |
				BIT_MD_TOAP_BYP_CRSN1 |
				BIT_MD_TOAP_BYP_CRSN2 |
				BIT_IP_PROT_CHK_BLIST |
				BIT_IPV4_NAT_FRAG_EN |
				BIT_IPV4_HASH_GREK |
				BIT_IPV6_HASH_GREK |
				BIT_CS0_RM_ALL_IP6_IP_EN |
				BIT_L2_HASH_ETH |
				BIT_L2_HASH_VID;
	u32 cah_en, flow_cfg, scan_mode;
	u32 i, idle, retry;

	if (ppe_id >= CFG_PPE_NUM)
		return;

	/* disable table learning */
	flow_cfg = readl(hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG);
	writel(flow_cfg & mask, hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG);
	/* wait PPE return to idle */
	udelay(100);

	for (retry = 0; retry < 10; retry++) {
		for (i = 0, idle = 0; i < 3; i++) {
			if ((readl(hnat_priv->ppe_base[ppe_id] + PPE_CAH_DBG) & CAH_DBG_BUSY) == 0)
				idle++;
		}

		if (idle >= 3)
			break;

		udelay(10);
	}

	if (retry >= 10) {
		pr_info("%s: ppe cache idle check timeout!\n", __func__);
		goto out;
	}

	/* disable scan mode */
	scan_mode = FIELD_GET(SCAN_MODE, readl(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG));
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, SCAN_MODE, 0);
	/* disable cache */
	cah_en = readl(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL) & CAH_EN;
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_EN, 0);

	/* invalidate PPE cache lines */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_X_MODE, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_X_MODE, 0);

	/* lock the preserved cache line */
	if (hnat_priv->data->version >= MTK_HNAT_V2)
		__hnat_write_cache_line(ppe_id, 0, 0x7FFF, 3, NULL);
	else
		__hnat_write_cache_line(ppe_id, 0, 0x3FFF, 3, NULL);

	/* restore cache enable */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_EN, cah_en);
	/* restore scan mode */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, SCAN_MODE, scan_mode);
out:
	/* restore table learning */
	writel(flow_cfg, hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG);
}

void hnat_cache_clr(u32 ppe_id)
{
	spin_lock_bh(&hnat_priv->cah_lock);
	__hnat_cache_clr(ppe_id);
	spin_unlock_bh(&hnat_priv->cah_lock);

	if (debug_level >= 2)
		pr_info("%s: Clear cache of PPE%d\n", __func__, ppe_id);
}

void __hnat_cache_ebl(u32 ppe_id, int enable)
{
	if (ppe_id >= CFG_PPE_NUM)
		return;

	spin_lock_bh(&hnat_priv->cah_lock);

	if (enable)
		__hnat_cache_clr(ppe_id);

	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_CAH_CTRL, CAH_EN, enable);

	spin_unlock_bh(&hnat_priv->cah_lock);
}

void hnat_cache_ebl(int enable)
{
	int i;

	for (i = 0; i < CFG_PPE_NUM; i++)
		__hnat_cache_ebl(i, enable);

	if (debug_level >= 2)
		pr_info("%s: %s cache of all PPE\n", __func__, (enable) ? "Enable" : "Disable");
}
EXPORT_SYMBOL(hnat_cache_ebl);

static int hnat_hw_init(u32 ppe_id)
{
	if (ppe_id >= CFG_PPE_NUM)
		return -EINVAL;

	/* setup hashing */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, TB_ETRY_NUM,
		     hnat_priv->etry_num_cfg);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, HASH_MODE, HASH_MODE_1);
	writel(HASH_SEED_KEY, hnat_priv->ppe_base[ppe_id] + PPE_HASH_SEED);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, XMODE, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, TB_ENTRY_SIZE,
		     (hnat_priv->data->version == MTK_HNAT_V3) ? ENTRY_128B :
		     (hnat_priv->data->version == MTK_HNAT_V2) ? ENTRY_96B :
								 ENTRY_80B);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, SMA, SMA_FWD_CPU_BUILD_ENTRY);

	/* set ip proto */
	writel(0xFFFFFFFF, hnat_priv->ppe_base[ppe_id] + PPE_IP_PROT_CHK);

	/* enable FOE */
	cr_set_bits(hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG,
		    BIT_IPV4_NAT_EN | BIT_IPV4_NAPT_EN |
		    BIT_IPV4_NAT_FRAG_EN | BIT_IPV4_HASH_GREK |
		    BIT_IPV4_DSL_EN | BIT_IPV6_6RD_EN |
		    BIT_IPV6_3T_ROUTE_EN | BIT_IPV6_5T_ROUTE_EN);

	if (hnat_priv->data->version == MTK_HNAT_V2 ||
	    hnat_priv->data->version == MTK_HNAT_V3)
		cr_set_bits(hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG,
			    BIT_IPV4_MAPE_EN | BIT_IPV4_MAPT_EN);

	if (hnat_priv->data->version == MTK_HNAT_V3)
		cr_set_bits(hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG,
			    BIT_L2_HASH_VID | BIT_L2_HASH_ETH |
			    BIT_IPV6_NAT_EN | BIT_IPV6_NAPT_EN |
			    BIT_CS0_RM_ALL_IP6_IP_EN);

	/* setup FOE aging */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, NTU_AGE, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, UNBD_AGE, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_UNB_AGE, UNB_MNP, 1000);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_UNB_AGE, UNB_DLTA, 3);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, TCP_AGE, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, UDP_AGE, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, FIN_AGE, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_BND_AGE_0, UDP_DLTA, 12);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_BND_AGE_0, NTU_DLTA, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_BND_AGE_1, FIN_DLTA, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_BND_AGE_1, TCP_DLTA, 7);

	/* setup FOE ka */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, KA_CFG, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_BIND_LMT_1, NTU_KA, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_KA, KA_T, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_KA, TCP_KA, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_KA, UDP_KA, 0);
	mdelay(10);

	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, SCAN_MODE, 2);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, KA_CFG, 3);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, TICK_SEL, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_KA, KA_T, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_KA, TCP_KA, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_KA, UDP_KA, 1);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_BIND_LMT_1, NTU_KA, 1);

	/* setup FOE rate limit */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_BIND_LMT_0, QURT_LMT, 16383);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_BIND_LMT_0, HALF_LMT, 16383);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_BIND_LMT_1, FULL_LMT, 16383);
	/* setup binding threshold as 30 packets per second */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_BNDR, BIND_RATE, 0x1E);

	/* setup FOE cf gen */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_GLO_CFG, PPE_EN, 1);
	writel(0, hnat_priv->ppe_base[ppe_id] + PPE_DFT_CPORT); /* pdma */
	/* writel(0x55555555, hnat_priv->ppe_base[ppe_id] + PPE_DFT_CPORT); */ /* qdma */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_GLO_CFG, TTL0_DRP, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_GLO_CFG, MCAST_TB_EN, 1);

	/* setup caching */
	__hnat_cache_ebl(ppe_id, 1);

	if (hnat_priv->data->version == MTK_HNAT_V2 ||
	    hnat_priv->data->version == MTK_HNAT_V3) {
		writel(0xcb777, hnat_priv->ppe_base[ppe_id] + PPE_DFT_CPORT1);
		writel(0x7f, hnat_priv->ppe_base[ppe_id] + PPE_SBW_CTRL);
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_GLO_CFG, SP_CMP_EN, 1);
	}

	if (hnat_priv->data->version == MTK_HNAT_V3) {
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_SB_FIFO_DBG,
			     SB_MED_FULL_DRP_EN, 1);
		cr_set_bits(hnat_priv->ppe_base[ppe_id] + PPE_GLO_CFG,
			    NEW_IPV4_ID_INC_EN | TSID_EN);
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG,
			    IS_SP_TAG_EN, 1);
		if (ppe_id == 0)
			cr_set_field(hnat_priv->fe_base + MTK_FE_INT_ENABLE2,
				     MTK_FE_INT2_PPE0_FLOW_CHK, 1);
		else if (ppe_id == 1)
			cr_set_field(hnat_priv->fe_base + MTK_FE_INT_ENABLE2,
				     MTK_FE_INT2_PPE1_FLOW_CHK, 1);
	}

	/*enable ppe mib counter*/
	if (hnat_priv->data->per_flow_accounting) {
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MIB_CFG, MIB_EN, 1);
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MIB_CFG, MIB_READ_CLEAR, 1);
		cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MIB_CAH_CTRL, MIB_CAH_EN, 1);
	}

	hnat_priv->g_ppdev = dev_get_by_name(&init_net, hnat_priv->ppd);
	hnat_priv->g_wandev = dev_get_by_name(&init_net, hnat_priv->wan);

	dev_info(hnat_priv->dev, "PPE%d hwnat start\n", ppe_id);

	return 0;
}

static int hnat_start(u32 ppe_id)
{
	u32 foe_table_sz;
	u32 foe_mib_tb_sz;
	u32 foe_flow_sz;
	int etry_num_cfg;
	int i;

	if (ppe_id >= CFG_PPE_NUM)
		return -EINVAL;

	/* mapp the FOE table */
	for (etry_num_cfg = DEF_ETRY_NUM_CFG; etry_num_cfg >= 0;
	     etry_num_cfg--, hnat_priv->foe_etry_num /= 2) {
		foe_table_sz = hnat_priv->foe_etry_num * sizeof(struct foe_entry);
		hnat_priv->foe_table_cpu[ppe_id] = dma_alloc_coherent(
				hnat_priv->dev, foe_table_sz,
				&hnat_priv->foe_table_dev[ppe_id], GFP_KERNEL);
		if (hnat_priv->foe_table_cpu[ppe_id]) {
			foe_flow_sz = (hnat_priv->foe_etry_num / 4) * sizeof(struct hlist_head);
			/* Allocate the foe_flow table for wifi tx binding flow */
			hnat_priv->foe_flow[ppe_id] = devm_kzalloc(hnat_priv->dev, foe_flow_sz,
								   GFP_KERNEL);
			/* If the memory allocation fails, free the memory allocated before */
			if (!hnat_priv->foe_flow[ppe_id]) {
				dma_free_coherent(hnat_priv->dev, foe_table_sz,
						  hnat_priv->foe_table_cpu[ppe_id],
						  hnat_priv->foe_table_dev[ppe_id]);
				hnat_priv->foe_table_cpu[ppe_id] = NULL;
			} else {
				break;
			}
		}
	}

	if (!hnat_priv->foe_table_cpu[ppe_id] || !hnat_priv->foe_flow[ppe_id])
		return -1;
	dev_info(hnat_priv->dev, "PPE%d entry number = %d\n",
		 ppe_id, hnat_priv->foe_etry_num);

	writel(hnat_priv->foe_table_dev[ppe_id], hnat_priv->ppe_base[ppe_id] + PPE_TB_BASE);
	memset(hnat_priv->foe_table_cpu[ppe_id], 0, foe_table_sz);

	for (i = 0; i < hnat_priv->foe_etry_num / 4; i++)
		INIT_HLIST_HEAD(&hnat_priv->foe_flow[ppe_id][i]);

	if (hnat_priv->data->version == MTK_HNAT_V1_1)
		exclude_boundary_entry(hnat_priv->foe_table_cpu[ppe_id]);

	if (hnat_priv->data->per_flow_accounting) {
		foe_mib_tb_sz = hnat_priv->foe_etry_num * sizeof(struct mib_entry);
		hnat_priv->foe_mib_cpu[ppe_id] =
			dma_alloc_coherent(hnat_priv->dev, foe_mib_tb_sz,
					   &hnat_priv->foe_mib_dev[ppe_id], GFP_KERNEL);
		if (!hnat_priv->foe_mib_cpu[ppe_id])
			return -1;
		writel(hnat_priv->foe_mib_dev[ppe_id],
		       hnat_priv->ppe_base[ppe_id] + PPE_MIB_TB_BASE);
		memset(hnat_priv->foe_mib_cpu[ppe_id], 0, foe_mib_tb_sz);

		hnat_priv->acct[ppe_id] =
			kcalloc(hnat_priv->foe_etry_num, sizeof(struct hnat_accounting),
				GFP_KERNEL);
		if (!hnat_priv->acct[ppe_id])
			return -1;
	}

	hnat_priv->etry_num_cfg = etry_num_cfg;
	hnat_hw_init(ppe_id);

	return 0;
}

static int ppe_busy_wait(u32 ppe_id)
{
	unsigned long t_start = jiffies;
	u32 r = 0;

	if (ppe_id >= CFG_PPE_NUM)
		return -EINVAL;

	while (1) {
		r = readl((hnat_priv->ppe_base[ppe_id] + 0x0));
		if (!(r & BIT(31)))
			return 0;
		if (time_after(jiffies, t_start + HZ))
			break;
		mdelay(10);
	}

	dev_notice(hnat_priv->dev, "ppe:%s timeout\n", __func__);

	return -1;
}

static void hnat_stop(u32 ppe_id)
{
	u32 foe_table_sz;
	u32 foe_mib_tb_sz;
	struct foe_entry *entry, *end;

	if (ppe_id >= CFG_PPE_NUM)
		return;

	/* send all traffic back to the DMA engine */
	set_gmac_ppe_fwd(NR_GMAC1_PORT, 0);
	set_gmac_ppe_fwd(NR_GMAC2_PORT, 0);
	set_gmac_ppe_fwd(NR_GMAC3_PORT, 0);

	dev_info(hnat_priv->dev, "hwnat stop\n");

	/* disable caching */
	__hnat_cache_ebl(ppe_id, 0);

	if (hnat_priv->foe_table_cpu[ppe_id]) {
		entry = hnat_priv->foe_table_cpu[ppe_id];
		end = hnat_priv->foe_table_cpu[ppe_id] + hnat_priv->foe_etry_num;
		while (entry < end) {
			__entry_delete(entry);
			entry++;
		}
	}

	/* flush cache has to be ahead of hnat disable --*/
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_GLO_CFG, PPE_EN, 0);

	/* disable scan mode and keep-alive */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, SCAN_MODE, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, KA_CFG, 0);

	ppe_busy_wait(ppe_id);

	/* disable FOE */
	cr_clr_bits(hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG,
		    BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN | BIT_IPV4_NAT_FRAG_EN |
		    BIT_IPV6_HASH_GREK | BIT_IPV4_DSL_EN |
		    BIT_IPV6_6RD_EN | BIT_IPV6_3T_ROUTE_EN |
		    BIT_IPV6_5T_ROUTE_EN | BIT_MD_TOAP_BYP_CRSN1 | BIT_MD_TOAP_BYP_CRSN0);

	if (hnat_priv->data->version == MTK_HNAT_V2 ||
	    hnat_priv->data->version == MTK_HNAT_V3)
		cr_clr_bits(hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG,
			    BIT_IPV4_MAPE_EN | BIT_IPV4_MAPT_EN);

	if (hnat_priv->data->version == MTK_HNAT_V3)
		cr_clr_bits(hnat_priv->ppe_base[ppe_id] + PPE_FLOW_CFG,
			    BIT_L2_HASH_VID | BIT_L2_HASH_ETH |
			    BIT_IPV6_NAT_EN | BIT_IPV6_NAPT_EN |
			    BIT_CS0_RM_ALL_IP6_IP_EN);

	/* disable FOE aging */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, NTU_AGE, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, UNBD_AGE, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, TCP_AGE, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, UDP_AGE, 0);
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_TB_CFG, FIN_AGE, 0);

	/* free the FOE table */
	foe_table_sz = hnat_priv->foe_etry_num * sizeof(struct foe_entry);
	if (hnat_priv->foe_table_cpu[ppe_id])
		dma_free_coherent(hnat_priv->dev, foe_table_sz,
				  hnat_priv->foe_table_cpu[ppe_id],
				  hnat_priv->foe_table_dev[ppe_id]);
	hnat_priv->foe_table_cpu[ppe_id] = NULL;
	writel(0, hnat_priv->ppe_base[ppe_id] + PPE_TB_BASE);

	if (hnat_priv->data->per_flow_accounting) {
		foe_mib_tb_sz = hnat_priv->foe_etry_num * sizeof(struct mib_entry);
		if (hnat_priv->foe_mib_cpu[ppe_id])
			dma_free_coherent(hnat_priv->dev, foe_mib_tb_sz,
					  hnat_priv->foe_mib_cpu[ppe_id],
					  hnat_priv->foe_mib_dev[ppe_id]);
		writel(0, hnat_priv->ppe_base[ppe_id] + PPE_MIB_TB_BASE);
		kfree(hnat_priv->acct[ppe_id]);
	}

	/* Release the allocated hnat_flow_entry nodes */
	if (hnat_priv->foe_flow[ppe_id])
		hnat_flow_entry_teardown_all(ppe_id);
}

static void hnat_release_netdev(void)
{
	int i;
	struct extdev_entry *ext_entry;

	for (i = 0; i < MAX_EXT_DEVS && hnat_priv->ext_if[i]; i++) {
		ext_entry = hnat_priv->ext_if[i];
		if (ext_entry->dev)
			dev_put(ext_entry->dev);
		ext_if_del(ext_entry);
		kfree(ext_entry);
	}

	if (hnat_priv->g_ppdev)
		dev_put(hnat_priv->g_ppdev);

	if (hnat_priv->g_wandev)
		dev_put(hnat_priv->g_wandev);
}

static struct notifier_block nf_hnat_netdevice_nb __read_mostly = {
	.notifier_call = nf_hnat_netdevice_event,
};

static struct notifier_block nf_hnat_netevent_nb __read_mostly = {
	.notifier_call = nf_hnat_netevent_handler,
};

int hnat_enable_hook(void)
{
	/* register hook functions used by WHNAT module.
	 */
	if (hnat_priv->data->whnat) {
		ra_sw_nat_hook_rx =
			(hnat_priv->data->version == MTK_HNAT_V2 ||
			 hnat_priv->data->version == MTK_HNAT_V3) ?
			 mtk_sw_nat_hook_rx : NULL;
		ra_sw_nat_hook_tx = mtk_sw_nat_hook_tx;
		ppe_dev_register_hook = mtk_ppe_dev_register_hook;
		ppe_dev_unregister_hook = mtk_ppe_dev_unregister_hook;
		ra_sw_nat_clear_bind_entries = foe_clear_all_bind_entries;
		hnat_get_wdma_tx_port = mtk_get_wdma_tx_port;
		hnat_get_wdma_rx_port = mtk_get_wdma_rx_port;
		hnat_set_wdma_pse_port_state = mtk_set_wdma_pse_port_state;
	}

	if (hnat_register_nf_hooks())
		return -1;

	ppe_del_entry_by_mac = entry_delete_by_mac;
	ppe_del_entry_by_ip = entry_delete_by_ip;
	ppe_del_entry_by_bssid_wcid = entry_delete_by_bssid_wcid;
	hook_toggle = 1;

	/* register hook function used at linux gso segmentation */
	mtk_skb_headroom_copy = mtk_hnat_skb_headroom_copy;

	return 0;
}

int hnat_disable_hook(void)
{
	struct foe_entry *entry;
	int i, hash_index;
	int cnt;

	ra_sw_nat_hook_tx = NULL;
	ra_sw_nat_hook_rx = NULL;
	ra_sw_nat_clear_bind_entries = NULL;
	hnat_unregister_nf_hooks();

	for (i = 0; i < CFG_PPE_NUM; i++) {
		cr_set_field(hnat_priv->ppe_base[i] + PPE_TB_CFG,
			     SMA, SMA_ONLY_FWD_CPU);
		cnt = 0;
		for (hash_index = 0; hash_index < hnat_priv->foe_etry_num; hash_index++) {
			entry = hnat_priv->foe_table_cpu[i] + hash_index;
			if (entry->bfib1.state == BIND) {
				spin_lock_bh(&hnat_priv->entry_lock);
				__entry_delete(entry);
				spin_unlock_bh(&hnat_priv->entry_lock);
				cnt++;
			}
		}
		/* clear HWNAT cache */
		if (cnt > 0)
			hnat_cache_clr(i);
	}

	mod_timer(&hnat_priv->hnat_sma_build_entry_timer, jiffies + 3 * HZ);
	ppe_del_entry_by_mac = NULL;
	ppe_del_entry_by_ip = NULL;
	ppe_del_entry_by_bssid_wcid = NULL;
	hook_toggle = 0;

	/* unregister hook function used at linux gso segmentation */
	mtk_skb_headroom_copy = NULL;

	return 0;
}

int hnat_warm_init(void)
{
	u32 foe_table_sz, foe_mib_tb_sz, ppe_id = 0;
	int i;

	unregister_netevent_notifier(&nf_hnat_netevent_nb);
	hnat_neigh_update_cleanup();

	for (ppe_id = 0; ppe_id < CFG_PPE_NUM; ppe_id++) {
		foe_table_sz =
			hnat_priv->foe_etry_num * sizeof(struct foe_entry);
		writel(hnat_priv->foe_table_dev[ppe_id],
		       hnat_priv->ppe_base[ppe_id] + PPE_TB_BASE);
		memset(hnat_priv->foe_table_cpu[ppe_id], 0, foe_table_sz);

		if (hnat_priv->data->version == MTK_HNAT_V1_1)
			exclude_boundary_entry(hnat_priv->foe_table_cpu[ppe_id]);

		if (hnat_priv->data->per_flow_accounting) {
			foe_mib_tb_sz =
				hnat_priv->foe_etry_num * sizeof(struct mib_entry);
			writel(hnat_priv->foe_mib_dev[ppe_id],
			       hnat_priv->ppe_base[ppe_id] + PPE_MIB_TB_BASE);
			memset(hnat_priv->foe_mib_cpu[ppe_id], 0,
			       foe_mib_tb_sz);
		}

		hnat_hw_init(ppe_id);
	}

	/* The SER will enable all the PPE ports again,
	 * so we have to manually disable the unused ports one more.
	 */
	for (i = 0; i < MAX_PPE_NUM; i++) {
		if (i >= CFG_PPE_NUM)
			mtk_set_ppe_pse_port_state(i, false);
	}

	set_gmac_ppe_fwd(NR_GMAC1_PORT, 1);
	set_gmac_ppe_fwd(NR_GMAC2_PORT, 1);
	set_gmac_ppe_fwd(NR_GMAC3_PORT, 1);
	register_netevent_notifier(&nf_hnat_netevent_nb);

	return 0;
}

static struct packet_type mtk_pack_type __read_mostly = {
	.type   = HQOS_MAGIC_TAG,
	.func   = mtk_hqos_ptype_cb,
};

static int hnat_probe(struct platform_device *pdev)
{
	int i;
	int err = 0;
	int index = 0;
	struct resource *res;
	const char *name;
	struct device_node *np;
	unsigned int val;
	struct property *prop;
	struct extdev_entry *ext_entry;
	const struct of_device_id *match;

	hnat_priv = devm_kzalloc(&pdev->dev, sizeof(struct mtk_hnat), GFP_KERNEL);
	if (!hnat_priv) {
		err = -ENOMEM;
		goto err_out2;
	}

	hnat_priv->foe_etry_num = DEF_ETRY_NUM;

	match = of_match_device(of_hnat_match, &pdev->dev);
	if (unlikely(!match)) {
		err = -EINVAL;
		goto err_out2;
	}

	hnat_priv->data = (struct mtk_hnat_data *)match->data;

	hnat_priv->dev = &pdev->dev;
	np = hnat_priv->dev->of_node;

	err = of_property_read_string(np, "mtketh-wan", &name);
	if (err < 0)
		strscpy(hnat_priv->wan, "eth1", IFNAMSIZ);
	else
		strncpy(hnat_priv->wan, (char *)name, IFNAMSIZ - 1);
	dev_info(&pdev->dev, "wan = %s\n", hnat_priv->wan);

	err = of_property_read_string(np, "mtketh-lan", &name);
	if (err < 0)
		strscpy(hnat_priv->lan, "eth0", IFNAMSIZ);
	else
		strncpy(hnat_priv->lan, (char *)name, IFNAMSIZ - 1);
	dev_info(&pdev->dev, "lan = %s\n", hnat_priv->lan);

	err = of_property_read_string(np, "mtketh-lan2", &name);
	if (err < 0)
		strscpy(hnat_priv->lan2, "eth2", IFNAMSIZ);
	else
		strncpy(hnat_priv->lan2, (char *)name, IFNAMSIZ - 1);
	dev_info(&pdev->dev, "lan2 = %s\n", hnat_priv->lan2);

	err = of_property_read_string(np, "mtketh-ppd", &name);
	if (err < 0)
		strscpy(hnat_priv->ppd, "eth0", IFNAMSIZ);
	else
		strncpy(hnat_priv->ppd, (char *)name, IFNAMSIZ - 1);
	dev_info(&pdev->dev, "ppd = %s\n", hnat_priv->ppd);

	/*get total gmac num in hnat*/
	err = of_property_read_u32_index(np, "mtketh-max-gmac", 0, &val);

	if (err < 0) {
		err = -EINVAL;
		goto err_out2;
	}

	hnat_priv->gmac_num = val;

	dev_info(&pdev->dev, "gmac num = %d\n", hnat_priv->gmac_num);

	err = of_property_read_u32_index(np, "mtkdsa-wan-port", 0, &val);

	if (err < 0) {
		hnat_priv->wan_dsa_port = NONE_DSA_PORT;
	} else {
		hnat_priv->wan_dsa_port = val;
		dev_info(&pdev->dev, "wan dsa port = %d\n", hnat_priv->wan_dsa_port);
	}

	err = of_property_read_u32_index(np, "mtketh-ppe-num", 0, &val);

	if (err < 0)
		hnat_priv->ppe_num = 1;
	else
		hnat_priv->ppe_num = val;

	dev_info(&pdev->dev, "ppe num = %d\n", hnat_priv->ppe_num);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		err = -ENOENT;
		goto err_out2;
	}

	hnat_priv->fe_base = devm_ioremap(&pdev->dev, res->start,
					  res->end - res->start + 1);
	if (!hnat_priv->fe_base) {
		err = -EADDRNOTAVAIL;
		goto err_out2;
	}

#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
	hnat_priv->ppe_base[0] = hnat_priv->fe_base + 0x2200;

	if (CFG_PPE_NUM > 1)
		hnat_priv->ppe_base[1] = hnat_priv->fe_base + 0x2600;

	if (CFG_PPE_NUM > 2)
		hnat_priv->ppe_base[2] = hnat_priv->fe_base + 0x2e00;
#else
	hnat_priv->ppe_base[0] = hnat_priv->fe_base + 0xe00;
#endif

	if (hnat_priv->data->version == MTK_HNAT_V3) {
		/* PPE flow check interrupt registeration for MT7987 */
		hnat_priv->fe_irq2 = platform_get_irq_optional(pdev, 0);
		if (hnat_priv->fe_irq2 >= 0) {
			err = devm_request_irq(hnat_priv->dev, hnat_priv->fe_irq2,
					       hnat_handle_fe_irq2, IRQF_SHARED,
					       dev_name(hnat_priv->dev), hnat_priv);
			if (err)
				dev_err(&pdev->dev, "Unable to request FE IRQ!\n");
		}
	}

	err = hnat_init_debugfs(hnat_priv);
	if (err)
		goto err_out2;

	prop = of_find_property(np, "ext-devices", NULL);
	for (name = of_prop_next_string(prop, NULL); name;
	     name = of_prop_next_string(prop, name), index++) {
		ext_entry = kzalloc(sizeof(*ext_entry), GFP_KERNEL);
		if (!ext_entry) {
			err = -ENOMEM;
			goto err_out1;
		}
		strncpy(ext_entry->name, (char *)name, IFNAMSIZ - 1);
		ext_if_add(ext_entry);
	}

	for (i = 0; i < MAX_EXT_DEVS && hnat_priv->ext_if[i]; i++) {
		ext_entry = hnat_priv->ext_if[i];
		dev_info(&pdev->dev, "ext devices = %s\n", ext_entry->name);
	}

	hnat_priv->lvid = 1;
	hnat_priv->wvid = 2;

	spin_lock_init(&hnat_priv->cah_lock);
	spin_lock_init(&hnat_priv->entry_lock);
	spin_lock_init(&hnat_priv->flow_entry_lock);

	for (i = 0; i < CFG_PPE_NUM; i++) {
		err = hnat_start(i);
		if (err)
			goto err_out;
	}

	/* The PSE default enables all the PPE ports,
	 * so we need to manually disable the unused ports.
	 */
	for (i = 0; i < MAX_PPE_NUM; i++) {
		if (i >= CFG_PPE_NUM)
			mtk_set_ppe_pse_port_state(i, false);
		else
			mtk_set_ppe_pse_port_state(i, true);
	}

	if (hnat_priv->data->whnat) {
		err = whnat_adjust_nf_hooks();
		if (err)
			goto err_out;
	}

	err = hnat_enable_hook();
	if (err)
		goto err_out;

	register_netdevice_notifier(&nf_hnat_netdevice_nb);
	register_netevent_notifier(&nf_hnat_netevent_nb);
	hnat_neigh_update_init();

	if (hnat_priv->data->mcast) {
		for (i = 0; i < CFG_PPE_NUM; i++)
			hnat_mcast_enable(i);
	}

	timer_setup(&hnat_priv->hnat_sma_build_entry_timer, hnat_sma_build_entry, 0);
	if (hnat_priv->data->version == MTK_HNAT_V1_3) {
		timer_setup(&hnat_priv->hnat_reset_timestamp_timer, hnat_reset_timestamp, 0);
		hnat_priv->hnat_reset_timestamp_timer.expires = jiffies;
		add_timer(&hnat_priv->hnat_reset_timestamp_timer);
	}

	if (IS_HQOS_MODE && IS_GMAC1_MODE)
		dev_add_pack(&mtk_pack_type);

	err = hnat_roaming_enable();
	if (err)
		pr_info("hnat roaming work fail\n");

	hnat_flow_entry_teardown_enable();

	INIT_LIST_HEAD(&hnat_priv->xlat.map_list);

	return 0;

err_out:
	for (i = 0; i < CFG_PPE_NUM; i++)
		hnat_stop(i);
err_out1:
	hnat_deinit_debugfs(hnat_priv);
	for (i = 0; i < MAX_EXT_DEVS && hnat_priv->ext_if[i]; i++) {
		ext_entry = hnat_priv->ext_if[i];
		ext_if_del(ext_entry);
		kfree(ext_entry);
	}
err_out2:
	for (i = 0; i < MAX_PPE_NUM; i++)
		mtk_set_ppe_pse_port_state(i, false);
	return err;
}

static void hnat_remove(struct platform_device *pdev)
{
	int i;

	hnat_roaming_disable();
	hnat_flow_entry_teardown_disable();
	unregister_netdevice_notifier(&nf_hnat_netdevice_nb);
	unregister_netevent_notifier(&nf_hnat_netevent_nb);
	hnat_neigh_update_cleanup();
	hnat_disable_hook();

	if (hnat_priv->data->mcast)
		hnat_mcast_disable();

	for (i = 0; i < CFG_PPE_NUM; i++)
		hnat_stop(i);

	for (i = 0; i < MAX_PPE_NUM; i++)
		mtk_set_ppe_pse_port_state(i, false);

	hnat_deinit_debugfs(hnat_priv);
	hnat_release_netdev();
	del_timer_sync(&hnat_priv->hnat_sma_build_entry_timer);
	if (hnat_priv->data->version == MTK_HNAT_V1_3)
		del_timer_sync(&hnat_priv->hnat_reset_timestamp_timer);

	if (IS_HQOS_MODE && IS_GMAC1_MODE)
		dev_remove_pack(&mtk_pack_type);
}

static const struct mtk_hnat_data hnat_data_v1 = {
	.num_of_sch = 2,
	.whnat = false,
	.per_flow_accounting = false,
	.mcast = false,
	.version = MTK_HNAT_V1_1,
};

static const struct mtk_hnat_data hnat_data_v2 = {
	.num_of_sch = 2,
	.whnat = true,
	.per_flow_accounting = true,
	.mcast = false,
	.version = MTK_HNAT_V1_2,
};

static const struct mtk_hnat_data hnat_data_v3 = {
	.num_of_sch = 4,
	.whnat = false,
	.per_flow_accounting = false,
	.mcast = false,
	.version = MTK_HNAT_V1_3,
};

static const struct mtk_hnat_data hnat_data_v4 = {
	.num_of_sch = 4,
	.whnat = true,
	.per_flow_accounting = true,
	.mcast = false,
	.version = MTK_HNAT_V2,
};

static const struct mtk_hnat_data hnat_data_v5 = {
	.num_of_sch = 4,
	.whnat = true,
	.per_flow_accounting = true,
	.mcast = false,
	.version = MTK_HNAT_V3,
};

const struct of_device_id of_hnat_match[] = {
	{ .compatible = "mediatek,mtk-hnat", .data = &hnat_data_v3 },
	{ .compatible = "mediatek,mtk-hnat_v1", .data = &hnat_data_v1 },
	{ .compatible = "mediatek,mtk-hnat_v2", .data = &hnat_data_v2 },
	{ .compatible = "mediatek,mtk-hnat_v3", .data = &hnat_data_v3 },
	{ .compatible = "mediatek,mtk-hnat_v4", .data = &hnat_data_v4 },
	{ .compatible = "mediatek,mtk-hnat_v5", .data = &hnat_data_v5 },
	{},
};
MODULE_DEVICE_TABLE(of, of_hnat_match);

static struct platform_driver hnat_driver = {
	.probe = hnat_probe,
	.remove = hnat_remove,
	.driver = {
		.name = "mediatek_soc_hnat",
		.of_match_table = of_hnat_match,
	},
};

module_platform_driver(hnat_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Sean Wang <sean.wang@mediatek.com>");
MODULE_AUTHOR("John Crispin <john@phrozen.org>");
MODULE_DESCRIPTION("Mediatek Hardware NAT");

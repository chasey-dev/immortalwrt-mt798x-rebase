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

#include <linux/netfilter_bridge.h>
#include <linux/netfilter_ipv6.h>

#include <net/arp.h>
#include <net/neighbour.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_flow_table.h>
#include <net/ipv6.h>
#include <net/ip6_route.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_acct.h>

#include "nf_hnat_mtk.h"
#include "hnat.h"

#include "../mtk_eth_soc.h"

#define do_ge2ext_fast(dev, skb)                                               \
	((IS_LAN_GRP(dev) || IS_WAN(dev) || IS_PPD(dev)) && \
	 skb_hnat_is_hashed(skb) && \
	 skb_hnat_reason(skb) == HIT_BIND_FORCE_TO_CPU)
#define do_ext2ge_fast_learn(dev, skb)                                         \
	(IS_PPD(dev) &&                                                        \
	 (skb_hnat_sport(skb) == NR_PDMA_PORT ||                           \
	  skb_hnat_sport(skb) == NR_QDMA_PORT) &&                       \
	  ((get_dev_from_index(skb->vlan_tci & VLAN_VID_MASK)) ||   \
		 get_wandev_from_index(skb->vlan_tci & VLAN_VID_MASK)))
#define do_mape_w2l_fast(dev, skb)                                          \
		(mape_toggle && IS_WAN(dev) && (!is_from_mape(skb)))

static struct ipv6hdr mape_l2w_v6h;
static struct ipv6hdr mape_w2l_v6h;
static inline uint8_t get_wifi_hook_if_index_from_dev(const struct net_device *dev)
{
	int i;

	for (i = 1; i < MAX_IF_NUM; i++) {
		if (hnat_priv->wifi_hook_if[i] == dev)
			return i;
	}

	return 0;
}

static inline int get_ext_device_number(void)
{
	int i, number = 0;

	for (i = 0; i < MAX_EXT_DEVS && hnat_priv->ext_if[i]; i++)
		number += 1;
	return number;
}

static inline int find_extif_from_devname(const char *name)
{
	int i;
	struct extdev_entry *ext_entry;

	for (i = 0; i < MAX_EXT_DEVS && hnat_priv->ext_if[i]; i++) {
		ext_entry = hnat_priv->ext_if[i];
		if (!strcmp(name, ext_entry->name))
			return 1;
	}
	return 0;
}

static inline int get_index_from_dev(const struct net_device *dev)
{
	int i;
	struct extdev_entry *ext_entry;

	for (i = 0; i < MAX_EXT_DEVS && hnat_priv->ext_if[i]; i++) {
		ext_entry = hnat_priv->ext_if[i];
		if (dev == ext_entry->dev)
			return ext_entry->dev->ifindex;
	}
	return 0;
}

static inline struct net_device *get_dev_from_index(int index)
{
	int i;
	struct extdev_entry *ext_entry;
	struct net_device *dev = 0;

	for (i = 0; i < MAX_EXT_DEVS && hnat_priv->ext_if[i]; i++) {
		ext_entry = hnat_priv->ext_if[i];
		if (ext_entry->dev && index == ext_entry->dev->ifindex) {
			dev = ext_entry->dev;
			break;
		}
	}
	return dev;
}

static inline struct net_device *get_wandev_from_index(int index)
{
	if (!hnat_priv->g_wandev)
		hnat_priv->g_wandev = dev_get_by_name(&init_net, hnat_priv->wan);

	if (hnat_priv->g_wandev && hnat_priv->g_wandev->ifindex == index)
		return hnat_priv->g_wandev;
	return NULL;
}

static inline int extif_set_dev(struct net_device *dev)
{
	int i;
	struct extdev_entry *ext_entry;

	for (i = 0; i < MAX_EXT_DEVS && hnat_priv->ext_if[i]; i++) {
		ext_entry = hnat_priv->ext_if[i];
		if (!strcmp(dev->name, ext_entry->name) && !ext_entry->dev) {
			dev_hold(dev);
			ext_entry->dev = dev;
			pr_info("%s(%s)\n", __func__, dev->name);

			return ext_entry->dev->ifindex;
		}
	}

	return -1;
}

static inline int extif_put_dev(struct net_device *dev)
{
	int i;
	struct extdev_entry *ext_entry;

	for (i = 0; i < MAX_EXT_DEVS && hnat_priv->ext_if[i]; i++) {
		ext_entry = hnat_priv->ext_if[i];
		if (ext_entry->dev == dev) {
			ext_entry->dev = NULL;
			dev_put(dev);
			pr_info("%s(%s)\n", __func__, dev->name);

			return 0;
		}
	}

	return -1;
}

int ext_if_add(struct extdev_entry *ext_entry)
{
	int len = get_ext_device_number();

	if (len < MAX_EXT_DEVS)
		hnat_priv->ext_if[len++] = ext_entry;

	return len;
}

int ext_if_del(struct extdev_entry *ext_entry)
{
	int i, j;

	for (i = 0; i < MAX_EXT_DEVS; i++) {
		if (hnat_priv->ext_if[i] == ext_entry) {
			for (j = i; hnat_priv->ext_if[j] && j < MAX_EXT_DEVS - 1; j++)
				hnat_priv->ext_if[j] = hnat_priv->ext_if[j + 1];
			hnat_priv->ext_if[j] = NULL;
			break;
		}
	}

	return i;
}

bool hnat_flow_entry_match(struct foe_entry *entry, struct foe_entry *data)
{
	int len;

	if (entry->udib1.udp != data->udib1.udp)
		return false;

	switch (entry->udib1.pkt_type) {
	case IPV4_HNAPT:
	case IPV4_HNAT:
	case IPV4_DSLITE:
	case IPV4_MAP_T:
	case IPV4_MAP_E:
		len = offsetof(struct hnat_ipv4_hnapt, info_blk2);
		break;
	case IPV6_3T_ROUTE:
	case IPV6_5T_ROUTE:
	case IPV6_6RD:
	case IPV6_HNAPT:
	case IPV6_HNAT:
		len = offsetof(struct hnat_ipv6_5t_route, resv1);
		break;
	default:
		return false;
	}

	return !memcmp(&entry->ipv4_hnapt.sip, &data->ipv4_hnapt.sip, len - 4);
}

void hnat_flow_entry_delete(struct hnat_flow_entry *flow_entry)
{
	hlist_del_init(&flow_entry->list);
	kfree(flow_entry);
}

static struct hnat_flow_entry *hnat_flow_entry_search(struct foe_entry *data,
						      u16 ppe_index,
						      u16 hash)
{
	struct hnat_flow_entry *flow_entry;
	struct hlist_head *head = &hnat_priv->foe_flow[ppe_index][hash / 4];
	struct hlist_node *n;

	hlist_for_each_entry_safe(flow_entry, n, head, list) {
		if ((flow_entry->ppe_index == ppe_index) &&
		    (flow_entry->hash == hash) &&
		    hnat_flow_entry_match(&flow_entry->data, data))
			return flow_entry;
	}

	return NULL;
}

static void foe_clear_ethdev_bind_entries(struct net_device *dev)
{
	struct net_device *master_dev = dev;
	const struct dsa_port *dp;
	struct foe_entry *entry;
	struct mtk_mac *mac;
	bool match_dev = false;
	int port_id, gmac;
	int cnt;
	u32 i, hash_index;
	u32 dsa_tag;

	/* Get the master device if the device is slave device */
	port_id = hnat_dsa_get_port(&master_dev);
	mac = netdev_priv(master_dev);
	gmac = HNAT_GMAC_FP(mac->id);

	if (gmac < 0)
		return;

	if (port_id >= 0) {
		dp = dsa_port_from_netdev(dev);
		if (IS_ERR(dp))
			return;

		if (IS_DSA_TAG_PROTO_8021Q(dp))
			dsa_tag = port_id | GENMASK(11, 10);
		else
			dsa_tag = BIT(port_id);
	}

	for (i = 0; i < CFG_PPE_NUM; i++) {
		cnt = 0;
		for (hash_index = 0; hash_index < hnat_priv->foe_etry_num; hash_index++) {
			entry = hnat_priv->foe_table_cpu[i] + hash_index;
			if (!entry_hnat_is_bound(entry))
				continue;

			match_dev = (IS_IPV4_GRP(entry)) ? entry->ipv4_hnapt.iblk2.dp == gmac :
							   entry->ipv6_5t_route.iblk2.dp == gmac;

			if (match_dev && port_id >= 0) {
				if (IS_DSA_TAG_PROTO_8021Q(dp)) {
					match_dev = (IS_IPV4_GRP(entry)) ?
						entry->ipv4_hnapt.vlan1 == dsa_tag :
						entry->ipv6_5t_route.vlan1 == dsa_tag;
				} else {
					match_dev = (IS_IPV4_GRP(entry)) ?
						!!(entry->ipv4_hnapt.sp_tag & dsa_tag) :
						!!(entry->ipv6_5t_route.sp_tag & dsa_tag);
				}
			}

			if (match_dev) {
				spin_lock_bh(&hnat_priv->entry_lock);
				__entry_delete(entry);
				spin_unlock_bh(&hnat_priv->entry_lock);
				if (debug_level >= 2)
					pr_info("[%s]: delete entry idx = %d_%d\n",
						__func__, i, hash_index);
				cnt++;
			}
		}

		/* clear HWNAT cache */
		if (cnt > 0)
			hnat_cache_clr(i);
	}
}

void foe_clear_all_bind_entries(void)
{
	struct foe_entry *entry;
	int i, hash_index;
	int cnt;

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
				if (debug_level >= 2)
					pr_info("[%s]: delete entry idx = %d_%d\n",
						__func__, i, hash_index);
				cnt++;
			}
		}
		/* clear HWNAT cache */
		if (cnt > 0)
			hnat_cache_clr(i);
	}

	mod_timer(&hnat_priv->hnat_sma_build_entry_timer, jiffies + 3 * HZ);
}

static void gmac_ppe_fwd_enable(struct net_device *dev)
{
	struct net_device *master_dev = dev;
	struct mtk_mac *mac;

	if (netdev_uses_dsa(master_dev))
		hnat_dsa_get_port(&master_dev);

	mac = netdev_priv(master_dev);

	if (mac->id == MTK_GMAC1_ID)
		set_gmac_ppe_fwd(NR_GMAC1_PORT, 1);
	else if (mac->id == MTK_GMAC2_ID)
		set_gmac_ppe_fwd(NR_GMAC2_PORT, 1);
	else if (mac->id == MTK_GMAC3_ID)
		set_gmac_ppe_fwd(NR_GMAC3_PORT, 1);
}

int nf_hnat_netdevice_event(struct notifier_block *unused, unsigned long event,
			    void *ptr)
{
	struct net_device *dev;

	dev = netdev_notifier_info_to_dev(ptr);

	switch (event) {
	case NETDEV_UP:
		gmac_ppe_fwd_enable(dev);

		extif_set_dev(dev);

		break;
	case NETDEV_CHANGE:
		/* Clear PPE entries if the slave of bond device physical link down */
		if (!netif_is_bond_slave(dev) ||
		    (!IS_LAN_GRP(dev) && !IS_WAN(dev)))
			break;

		if (netif_carrier_ok(dev))
			break;

		foe_clear_ethdev_bind_entries(dev);
		break;
	case NETDEV_GOING_DOWN:
		if (!get_wifi_hook_if_index_from_dev(dev))
			extif_put_dev(dev);

		if (!IS_LAN_GRP(dev) && !IS_WAN(dev) &&
		    !find_extif_from_devname(dev->name) &&
		    !dev->netdev_ops->ndo_flow_offload_check)
			break;

		foe_clear_all_bind_entries();

		break;
	case NETDEV_UNREGISTER:
		if (hnat_priv->g_ppdev == dev) {
			hnat_priv->g_ppdev = NULL;
			dev_put(dev);
		}
		if (hnat_priv->g_wandev == dev) {
			hnat_priv->g_wandev = NULL;
			dev_put(dev);
		}

		break;
	case NETDEV_REGISTER:
		if (IS_PPD(dev) && !hnat_priv->g_ppdev)
			hnat_priv->g_ppdev = dev_get_by_name(&init_net, hnat_priv->ppd);
		if (IS_WAN(dev) && !hnat_priv->g_wandev)
			hnat_priv->g_wandev = dev_get_by_name(&init_net, hnat_priv->wan);

		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

void foe_clear_crypto_entry(u32 cdrt_idx)
{
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
	struct foe_entry *entry;
	int i, hash_index;
	int cnt;

	for (i = 0; i < CFG_PPE_NUM; i++) {
		if (!hnat_priv->foe_table_cpu[i])
			continue;
		cnt = 0;
		for (hash_index = 0; hash_index < hnat_priv->foe_etry_num; hash_index++) {
			entry = hnat_priv->foe_table_cpu[i] + hash_index;

			if (entry->bfib1.state == BIND && IS_IPV4_HNAPT(entry) &&
			    entry->ipv4_hnapt.cdrt_id == cdrt_idx) {
				spin_lock_bh(&hnat_priv->entry_lock);
				__entry_delete(entry);
				spin_unlock_bh(&hnat_priv->entry_lock);
				if (debug_level >= 2)
					pr_info("[%s]: delete entry idx = %d_%d\n",
						__func__, i, hash_index);
				cnt++;
			}
		}
		/* clear HWNAT cache */
		if (cnt > 0)
			hnat_cache_clr(i);
	}
#endif /* defined(CONFIG_MEDIATEK_NETSYS_V3) */
}
EXPORT_SYMBOL(foe_clear_crypto_entry);

static void foe_clear_entry(struct list_head *head)
{
	struct hnat_neigh_update_event *neigh;
	struct foe_entry *entry;
	int i, hash_index;
	int cnt;
	bool is_ipv4, dip_match, dmac_match;

	for (i = 0; i < CFG_PPE_NUM; i++) {
		if (!hnat_priv->foe_table_cpu[i])
			continue;
		cnt = 0;
		for (hash_index = 0; hash_index < hnat_priv->foe_etry_num; hash_index++) {
			entry = hnat_priv->foe_table_cpu[i] + hash_index;
			if (entry->bfib1.state != BIND)
				continue;

			list_for_each_entry(neigh, head, list) {
				is_ipv4 = (neigh->tbl_family == AF_INET);
				dip_match = entry_ip_cmp(entry, is_ipv4, &neigh->dip,
							 ENTRY_CMP_DST);
				if (!dip_match)
					continue;
				dmac_match = entry_mac_cmp(entry, neigh->ha, ENTRY_CMP_DST);
				/* Delete entry if nud_state is NUD_FAILED or DMAC not match */
				if (!((neigh->nud_state & NUD_FAILED) || !dmac_match))
					continue;

				spin_lock_bh(&hnat_priv->entry_lock);
				__entry_delete(entry);
				spin_unlock_bh(&hnat_priv->entry_lock);

				if (debug_level >= 7) {
					pr_info("%s: state=%d, New mac= %pM\n",
						__func__, neigh->nud_state, neigh->ha);
					if (is_ipv4)
						pr_info("Delete old entry: dip =%pI4\n",
							&neigh->dip);
					else
						pr_info("Delete old entry: dip =%pI6\n",
							&neigh->dip6);
				}
				cnt++;
				break;
			}
		}
		/* clear HWNAT cache */
		if (cnt > 0)
			hnat_cache_clr(i);
	}
}

void hnat_neigh_update_init(void)
{
	INIT_DELAYED_WORK(&hnat_priv->neigh_update.work, hnat_neigh_update_work_handler);
	INIT_LIST_HEAD(&hnat_priv->neigh_update.head);
	spin_lock_init(&hnat_priv->neigh_update.lock);

	spin_lock_bh(&hnat_priv->neigh_update.lock);
	hnat_priv->neigh_update.pending_cnt = 0;
	spin_unlock_bh(&hnat_priv->neigh_update.lock);
}

void hnat_neigh_update_cleanup(void)
{
	struct hnat_neigh_update_event *entry, *tmp;
	int is_pending;

	 /* Take lock first to prevent new entries */
	spin_lock_bh(&hnat_priv->neigh_update.lock);

	/* Check if work is pending while holding lock */
	is_pending = delayed_work_pending(&hnat_priv->neigh_update.work);

	/* Clear the list */
	list_for_each_entry_safe(entry, tmp, &hnat_priv->neigh_update.head, list) {
		list_del(&entry->list);
		kfree(entry);
	}
	hnat_priv->neigh_update.pending_cnt = 0;
	spin_unlock_bh(&hnat_priv->neigh_update.lock);

	/* Cancel work after clearing list */
	if (is_pending)
		cancel_delayed_work_sync(&hnat_priv->neigh_update.work);
}

void hnat_neigh_update_work_handler(struct work_struct *work)
{
	struct hnat_neigh_update_event *entry, *tmp;
	LIST_HEAD(local_list);
	int processed = 0;

	spin_lock_bh(&hnat_priv->neigh_update.lock);
	/* move the list to local_list */
	/* Process only a limited number of entries at once */
	list_for_each_entry_safe(entry, tmp, &hnat_priv->neigh_update.head, list) {
		/* Batch processing budget, default 128 */
		if (processed >= NEIGH_PROCESS_BUDGET)
			break;
		list_move_tail(&entry->list, &local_list);
		processed++;
	}
	hnat_priv->neigh_update.pending_cnt -= processed;
	spin_unlock_bh(&hnat_priv->neigh_update.lock);

	if (!list_empty(&local_list))
		foe_clear_entry(&local_list);

	list_for_each_entry_safe(entry, tmp, &local_list, list) {
		list_del(&entry->list);
		kfree(entry);
	}

	/* reschedule if new update event added during handling */
	spin_lock_bh(&hnat_priv->neigh_update.lock);
	if (!list_empty(&hnat_priv->neigh_update.head))
		schedule_delayed_work(&hnat_priv->neigh_update.work,
				      msecs_to_jiffies(NEIGH_UPDATE_WORK_DELAY_MS));
	spin_unlock_bh(&hnat_priv->neigh_update.lock);
}

int nf_hnat_netevent_handler(struct notifier_block *unused, unsigned long event,
			     void *ptr)
{
	struct net_device *dev = NULL;
	struct neighbour *neigh = NULL;
	struct hnat_neigh_update_event *entry;
	u32 pending_max;

	switch (event) {
	case NETEVENT_NEIGH_UPDATE:
		neigh = ptr;
		dev = neigh->dev;
		if (!dev || !(neigh->nud_state & (NUD_CONNECTED | NUD_FAILED)))
			return NOTIFY_DONE;

		/* gc_thresh3 is the hard limit for the number of ARP/ND entries
		 * (default is 1024). We multiply it by 2 to provide a safety buffer
		 * for handling event bursts.
		 */
		pending_max = max(4096, (arp_tbl.gc_thresh3 + nd_tbl.gc_thresh3) * 2);

		spin_lock_bh(&hnat_priv->neigh_update.lock);

		if (hnat_priv->neigh_update.pending_cnt >= pending_max)
			goto unlock_out;

		entry = kzalloc(sizeof(*entry), GFP_ATOMIC);
		if (!entry)
			goto unlock_out;

		memcpy(entry->ha, neigh->ha, ETH_ALEN);
		if (neigh->tbl->family == AF_INET)
			memcpy(&entry->dip, neigh->primary_key, neigh->tbl->key_len);
		else
			memcpy(&entry->dip6, neigh->primary_key, neigh->tbl->key_len);
		entry->nud_state = neigh->nud_state;
		entry->tbl_family = neigh->tbl->family;

		list_add_tail(&entry->list, &hnat_priv->neigh_update.head);
		hnat_priv->neigh_update.pending_cnt++;

		if (!delayed_work_pending(&hnat_priv->neigh_update.work))
			schedule_delayed_work(&hnat_priv->neigh_update.work,
					      msecs_to_jiffies(NEIGH_UPDATE_WORK_DELAY_MS));

		goto unlock_out;
	}

	return NOTIFY_DONE;
unlock_out:
	spin_unlock_bh(&hnat_priv->neigh_update.lock);
	return NOTIFY_DONE;
}

static unsigned int mape_add_ipv6_hdr(struct sk_buff *skb, struct ipv6hdr mape_ip6h)
{
	struct ethhdr *eth = NULL;
	struct ipv6hdr *ip6h = NULL;
	struct iphdr *iph = NULL;

	if (skb_headroom(skb) < IPV6_HDR_LEN || skb_shared(skb) ||
	    (skb_cloned(skb) && !skb_clone_writable(skb, 0))) {
		return -1;
	}

	/* point to L3 */
	memcpy(skb->data - IPV6_HDR_LEN - ETH_HLEN, skb_push(skb, ETH_HLEN), ETH_HLEN);
	memcpy(skb_push(skb, IPV6_HDR_LEN - ETH_HLEN), &mape_ip6h, IPV6_HDR_LEN);

	eth = (struct ethhdr *)(skb->data - ETH_HLEN);
	eth->h_proto = htons(ETH_P_IPV6);
	skb->protocol = htons(ETH_P_IPV6);

	iph = (struct iphdr *)(skb->data + IPV6_HDR_LEN);
	ip6h = (struct ipv6hdr *)(skb->data);
	ip6h->payload_len = iph->tot_len; /* maybe different with ipv4 */

	skb_set_network_header(skb, 0);
	skb_set_transport_header(skb, iph->ihl * 4 + IPV6_HDR_LEN);
	return 0;
}

static void fix_skb_packet_type(struct sk_buff *skb, struct net_device *dev,
				struct ethhdr *eth)
{
	skb->pkt_type = PACKET_HOST;
	if (unlikely(is_multicast_ether_addr(eth->h_dest))) {
		if (ether_addr_equal_64bits(eth->h_dest, dev->broadcast))
			skb->pkt_type = PACKET_BROADCAST;
		else
			skb->pkt_type = PACKET_MULTICAST;
	}
}

static unsigned int do_hnat_ext_to_ge(struct sk_buff *skb, const struct net_device *in,
				      const char *func)
{
	if (hnat_priv->g_ppdev && hnat_priv->g_ppdev->flags & IFF_UP) {
		u16 vlan_id = 0;

		skb_set_network_header(skb, 0);
		skb_push(skb, ETH_HLEN);
		set_to_ppe(skb);

		vlan_id = skb_vlan_tag_get_id(skb);
		if (vlan_id) {
			skb = vlan_insert_tag(skb, skb->vlan_proto, skb->vlan_tci);
			if (!skb)
				return -1;
		}

		/*set where we come from*/
		skb->vlan_proto = htons(ETH_P_8021Q);
		skb->vlan_tci =
			(VLAN_CFI_MASK | (in->ifindex & VLAN_VID_MASK));
		skb->dev = hnat_priv->g_ppdev;
		dev_queue_xmit(skb);
		if (debug_level >= 7) {
			trace_printk("%s: vlan_prot=0x%x, vlan_tci=%x, in->name=%s, skb->dev->name=%s\n",
				     __func__, ntohs(skb->vlan_proto), skb->vlan_tci,
				     in->name, hnat_priv->g_ppdev->name);
			trace_printk("%s: called from %s successfully\n", __func__, func);
		}
		return 0;
	}

	if (debug_level >= 7)
		trace_printk("%s: called from %s fail\n", __func__, func);
	return -1;
}

static unsigned int do_hnat_ext_to_ge2(struct sk_buff *skb, const char *func)
{
	struct ethhdr *eth = eth_hdr(skb);
	struct mtk_hnat *h = hnat_priv;
	struct net_device *dev;
	struct foe_entry *entry;

	if (debug_level >= 7)
		trace_printk("%s: vlan_prot=0x%x, vlan_tci=%x\n", __func__,
			     ntohs(skb->vlan_proto), skb->vlan_tci);

	if (skb_hnat_entry(skb) >= h->foe_etry_num ||
	    skb_hnat_ppe(skb) >= CFG_PPE_NUM)
		return -1;

	dev = get_dev_from_index(skb->vlan_tci & VLAN_VID_MASK);

	if (dev) {
		/*set where we to go*/
		skb->dev = dev;
		skb->vlan_proto = 0;
		skb->vlan_tci = 0;

		if (ntohs(eth->h_proto) == ETH_P_8021Q) {
			skb = skb_vlan_untag(skb);
			if (unlikely(!skb))
				return -1;
		}

		if (IS_BOND(dev) &&
		    (((h->data->version == MTK_HNAT_V2 ||
		       h->data->version == MTK_HNAT_V3) &&
				(skb_hnat_entry(skb) != 0x7fff)) ||
		     ((h->data->version != MTK_HNAT_V2 &&
		       h->data->version != MTK_HNAT_V3) &&
				(skb_hnat_entry(skb) != 0x3fff))))
			skb_set_hash(skb, skb_hnat_entry(skb) >> 1, PKT_HASH_TYPE_L4);

		set_from_extge(skb);
		fix_skb_packet_type(skb, skb->dev, eth);
		netif_rx(skb);
		if (debug_level >= 7)
			trace_printk("%s: called from %s successfully\n", __func__,
				     func);
	} else {
		/* MapE WAN --> LAN/WLAN PingPong. */
		dev = get_wandev_from_index(skb->vlan_tci & VLAN_VID_MASK);
		if (mape_toggle && dev) {
			if (!mape_add_ipv6_hdr(skb, mape_w2l_v6h)) {
				skb_set_mac_header(skb, -ETH_HLEN);
				skb->dev = dev;
				set_from_mape(skb);
				skb->vlan_proto = 0;
				skb->vlan_tci = 0;
				fix_skb_packet_type(skb, skb->dev, eth_hdr(skb));
				entry = &h->foe_table_cpu[skb_hnat_ppe(skb)][skb_hnat_entry(skb)];
				entry->bfib1.pkt_type = IPV4_HNAPT;
				netif_rx(skb);
				return 0;
			}
		}
		if (debug_level >= 7)
			trace_printk("%s: called from %s fail\n", __func__, func);
		return -1;
	}

	return 0;
}

static unsigned int do_hnat_ge_to_ext(struct sk_buff *skb, const char *func)
{
	/*set where we to go*/
	u8 index;
	struct foe_entry *entry;
	struct net_device *dev;

	if (skb_hnat_entry(skb) >= hnat_priv->foe_etry_num ||
	    skb_hnat_ppe(skb) >= CFG_PPE_NUM)
		return -1;

	entry = &hnat_priv->foe_table_cpu[skb_hnat_ppe(skb)][skb_hnat_entry(skb)];

	if (IS_IPV4_GRP(entry))
		index = entry->ipv4_hnapt.act_dp & UDF_PINGPONG_IFIDX;
	else
		index = entry->ipv6_5t_route.act_dp & UDF_PINGPONG_IFIDX;

	dev = get_dev_from_index(index);
	if (!dev) {
		if (debug_level >= 7)
			trace_printk("%s: called from %s. Get wifi interface fail\n",
				     __func__, func);
		return 0;
	}

	skb->dev = dev;

	if (IS_HQOS_MODE && eth_hdr(skb)->h_proto == HQOS_MAGIC_TAG) {
		skb = skb_unshare(skb, GFP_ATOMIC);
		if (!skb)
			return NF_ACCEPT;

		if (unlikely(!pskb_may_pull(skb, VLAN_HLEN)))
			return NF_ACCEPT;

		skb_pull_rcsum(skb, VLAN_HLEN);

		memmove(skb->data - ETH_HLEN, skb->data - ETH_HLEN - VLAN_HLEN,
			2 * ETH_ALEN);
	}

	if (skb->dev) {
		skb_set_network_header(skb, 0);
		skb_push(skb, ETH_HLEN);
		dev_queue_xmit(skb);
		if (debug_level >= 7)
			trace_printk("%s: called from %s successfully\n", __func__,
				     func);
		return 0;
	}

	if (mape_toggle) {
		/* Add ipv6 header mape for lan/wlan -->wan */
		dev = get_wandev_from_index(index);
		if (dev) {
			if (!mape_add_ipv6_hdr(skb, mape_l2w_v6h)) {
				skb_set_network_header(skb, 0);
				skb_push(skb, ETH_HLEN);
				skb_set_mac_header(skb, 0);
				skb->dev = dev;
				dev_queue_xmit(skb);
				return 0;
			}
			if (debug_level >= 7)
				trace_printk("%s: called from %s fail[MapE]\n", __func__,
					     func);
			return -1;
		}
	}

	/*if external devices is down, invalidate related ppe entry*/
	if (entry_hnat_is_bound(entry)) {
		entry->bfib1.state = INVALID;
		if (IS_IPV4_GRP(entry))
			entry->ipv4_hnapt.act_dp &= ~UDF_PINGPONG_IFIDX;
		else
			entry->ipv6_5t_route.act_dp &= ~UDF_PINGPONG_IFIDX;

		/* clear HWNAT cache */
		hnat_cache_clr(skb_hnat_ppe(skb));
	}
	if (debug_level >= 7)
		trace_printk("%s: called from %s fail, index=%x\n", __func__,
			     func, index);
	return -1;
}

static void pre_routing_print(struct sk_buff *skb, const struct net_device *in,
			      const struct net_device *out, const char *func)
{
	if (debug_level >= 7)
		trace_printk(
			"[%s]: %s(iif=0x%x CB2=0x%x)-->%s (ppe_hash=0x%x) sport=0x%x reason=0x%x alg=0x%x from %s\n",
			__func__, in->name, skb_hnat_iface(skb),
			HNAT_SKB_CB2(skb)->magic, out->name, skb_hnat_entry(skb),
			skb_hnat_sport(skb), skb_hnat_reason(skb), skb_hnat_alg(skb),
			func);
}

static void post_routing_print(struct sk_buff *skb, const struct net_device *in,
			       const struct net_device *out, const char *func)
{
	if (debug_level >= 7)
		trace_printk(
			"[%s]: %s(iif=0x%x, CB2=0x%x)-->%s (ppe_hash=0x%x) sport=0x%x reason=0x%x alg=0x%x from %s\n",
			__func__, in->name, skb_hnat_iface(skb),
			HNAT_SKB_CB2(skb)->magic, out->name, skb_hnat_entry(skb),
			skb_hnat_sport(skb), skb_hnat_reason(skb), skb_hnat_alg(skb),
			func);
}

static inline void hnat_set_iif(const struct nf_hook_state *state,
				struct sk_buff *skb, int val)
{
	if (IS_WHNAT(state->in) && FROM_WED(skb)) {
		return;
	} else if (IS_LAN(state->in)) {
		skb_hnat_iface(skb) = FOE_MAGIC_GE_LAN;
	} else if (IS_LAN2(state->in)) {
		skb_hnat_iface(skb) = FOE_MAGIC_GE_LAN2;
	} else if (IS_PPD(state->in)) {
		skb_hnat_iface(skb) = FOE_MAGIC_GE_PPD;
	} else if (IS_EXT(state->in)) {
		skb_hnat_iface(skb) = FOE_MAGIC_EXT;
	} else if (IS_WAN(state->in)) {
		skb_hnat_iface(skb) = FOE_MAGIC_GE_WAN;
	} else if (!IS_BR(state->in)) {
		if (state->in->netdev_ops->ndo_flow_offload_check) {
			skb_hnat_iface(skb) = FOE_MAGIC_GE_VIRTUAL;
		} else {
			skb_hnat_iface(skb) = FOE_INVALID;

			if (is_magic_tag_valid(skb) &&
			    IS_SPACE_AVAILABLE_HEAD(skb))
				memset(skb_hnat_info(skb), 0, FOE_INFO_LEN);
		}
	}
}

static inline void hnat_set_alg(const struct nf_hook_state *state,
				struct sk_buff *skb, int val)
{
	skb_hnat_alg(skb) = val;
}

static inline void hnat_set_head_frags(const struct nf_hook_state *state,
				       struct sk_buff *head_skb, int val,
				       void (*fn)(const struct nf_hook_state *state,
						  struct sk_buff *skb, int val))
{
	struct sk_buff *segs = skb_shinfo(head_skb)->frag_list;

	fn(state, head_skb, val);
	while (segs) {
		fn(state, segs, val);
		segs = segs->next;
	}
}

static void ppe_fill_flow_lbl(struct foe_entry *entry, struct ipv6hdr *ip6h)
{
	entry->ipv4_dslite.flow_lbl[0] = ip6h->flow_lbl[2];
	entry->ipv4_dslite.flow_lbl[1] = ip6h->flow_lbl[1];
	entry->ipv4_dslite.flow_lbl[2] = ip6h->flow_lbl[0];
}

#if !(defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3))
static unsigned int do_hnat_mape_w2l_fast(struct sk_buff *skb, const struct net_device *in,
					  const char *func)
{
	struct ipv6hdr *ip6h = ipv6_hdr(skb);
	struct iphdr _iphdr;
	struct iphdr *iph;
	struct ethhdr *eth;

	/* WAN -> LAN/WLAN MapE. */
	if (mape_toggle && (ip6h->nexthdr == NEXTHDR_IPIP)) {
		iph = skb_header_pointer(skb, IPV6_HDR_LEN, sizeof(_iphdr), &_iphdr);
		if (unlikely(!iph))
			return -1;

		switch (iph->protocol) {
		case IPPROTO_UDP:
		case IPPROTO_TCP:
			break;
		default:
			return -1;
		}
		mape_w2l_v6h = *ip6h;

		/* Remove ipv6 header. */
		memcpy(skb->data + IPV6_HDR_LEN - ETH_HLEN,
		       skb->data - ETH_HLEN, ETH_HLEN);
		skb_pull(skb, IPV6_HDR_LEN - ETH_HLEN);
		skb_set_mac_header(skb, 0);
		skb_set_network_header(skb, ETH_HLEN);
		skb_set_transport_header(skb, ETH_HLEN + sizeof(_iphdr));

		eth = eth_hdr(skb);
		eth->h_proto = htons(ETH_P_IP);
		set_to_ppe(skb);

		skb->vlan_proto = htons(ETH_P_8021Q);
		skb->vlan_tci =
		(VLAN_CFI_MASK | (in->ifindex & VLAN_VID_MASK));

		if (!hnat_priv->g_ppdev)
			hnat_priv->g_ppdev = dev_get_by_name(&init_net, hnat_priv->ppd);

		skb->dev = hnat_priv->g_ppdev;
		skb->protocol = htons(ETH_P_IP);

		dev_queue_xmit(skb);

		return 0;
	}
	return -1;
}
#endif

static void mtk_464xlat_pre_process(struct sk_buff *skb)
{
	struct foe_entry *foe;

	if (skb_hnat_entry(skb) >= hnat_priv->foe_etry_num ||
	    skb_hnat_ppe(skb) >= CFG_PPE_NUM)
		return;

	foe = &hnat_priv->foe_table_cpu[skb_hnat_ppe(skb)][skb_hnat_entry(skb)];
	if (foe->bfib1.state != BIND &&
	    skb_hnat_reason(skb) == HIT_UNBIND_RATE_REACH)
		memcpy(&headroom[skb_hnat_entry(skb)], skb->head,
		       sizeof(struct hnat_desc));

	if (foe->bfib1.state == BIND)
		memset(&headroom[skb_hnat_entry(skb)], 0,
		       sizeof(struct hnat_desc));
}

static int hnat_ipv6_addr_equal(u32 *foe_ipv6_ptr, const struct in6_addr *target)
{
	struct in6_addr foe_in6_addr;
	int i;

	for (i = 0; i < 4; i++)
		foe_in6_addr.s6_addr32[i] = htonl(foe_ipv6_ptr[i]);

	return ipv6_addr_equal(&foe_in6_addr, target);
}

static int hnat_get_hdr_protocol(struct sk_buff *skb, u16 *h_offset)
{
	u16 h_proto, offset = 0;

	h_proto = ntohs(skb->protocol);

	if (h_proto == ETH_P_PPP_SES) {
		struct {
			struct pppoe_hdr hdr;
			__be16 proto;
		} *hdr, _hdr;

		hdr = skb_header_pointer(skb, offset, sizeof(_hdr), &_hdr);
		if (!hdr)
			return -EPROTO;

		h_proto = hdr->proto;
		offset += PPPOE_SES_HLEN;

		if (h_proto == htons(PPP_IP))
			h_proto = ETH_P_IP;
		else if (h_proto == htons(PPP_IPV6))
			h_proto = ETH_P_IPV6;
		else
			return -EPROTO;
	}

	*h_offset = offset;

	return h_proto;
}

static unsigned int is_ppe_support_type(struct sk_buff *skb)
{
	struct ethhdr *eth = NULL;
	struct iphdr *iph = NULL;
	struct ipv6hdr *ip6h = NULL;
	struct iphdr _iphdr;
	u16 h_proto, h_offset = 0;

	if (unlikely(!skb))
		return 0;

	if (!is_magic_tag_valid(skb) || !IS_SPACE_AVAILABLE_HEAD(skb))
		return 0;

	if (skb_mac_header_was_set(skb) && (skb_mac_header_len(skb) >= ETH_HLEN)) {
		eth = eth_hdr(skb);
		if (is_broadcast_ether_addr(eth->h_dest))
			return 0;
		if (is_multicast_ether_addr(eth->h_dest)) {
			if (!hnat_priv->data->mcast ||
			    (IS_MCAST_UNI_MODE && !hnat_is_mcast_uni(skb)))
				return 0;
		}
	}

	h_proto = hnat_get_hdr_protocol(skb, &h_offset);

	switch (h_proto) {
	case ETH_P_IP:
		iph = (struct iphdr *)(skb_network_header(skb) + h_offset);

		/* check if the packet is multicast */
		if (!hnat_priv->data->mcast && ipv4_is_multicast(iph->daddr))
			return 0;

		if (mtk_tnl_decap_offloadable && mtk_tnl_decap_offloadable(skb)) {
			/* tunnel protocol is offloadable */
			skb_hnat_set_is_decap(skb, 1);
			return 1;
		} else if ((iph->protocol == IPPROTO_TCP) ||
			   (iph->protocol == IPPROTO_UDP) ||
			   (iph->protocol == IPPROTO_IPV6)) {
			/* do not accelerate non tcp/udp traffic */
			return 1;
		}

		break;
	case ETH_P_IPV6:
		ip6h = (struct ipv6hdr *)(skb_network_header(skb) + h_offset);

		/* check if the packet is multicast */
		if (!hnat_priv->data->mcast && ip6h->daddr.s6_addr[0] == 0xff)
			return 0;

		if ((ip6h->nexthdr == NEXTHDR_TCP) ||
		    (ip6h->nexthdr == NEXTHDR_UDP)) {
			return 1;
		} else if (ip6h->nexthdr == NEXTHDR_IPIP) {
			iph = skb_header_pointer(skb, IPV6_HDR_LEN + h_offset,
						 sizeof(_iphdr), &_iphdr);
			if (unlikely(!iph))
				return 0;

			if ((iph->protocol == IPPROTO_TCP) ||
			    (iph->protocol == IPPROTO_UDP)) {
				return 1;
			}

		}

		break;
	case ETH_P_8021Q:
		return 1;
	default:
		if (l2br_toggle == 1)
			return 1;
		break;
	}

	return 0;
}

static unsigned int
mtk_hnat_ipv6_nf_pre_routing(void *priv, struct sk_buff *skb,
			     const struct nf_hook_state *state)
{
	if (!skb)
		goto drop;

	if (!is_magic_tag_valid(skb))
		return NF_ACCEPT;

	if (!is_ppe_support_type(skb)) {
		hnat_set_head_frags(state, skb, 1, hnat_set_alg);
		return NF_ACCEPT;
	}

	hnat_set_head_frags(state, skb, -1, hnat_set_iif);

	pre_routing_print(skb, state->in, state->out, __func__);

	/* packets from external devices -> xxx ,step 1 , learning stage & bound stage*/
	if (do_ext2ge_fast_try(state->in, skb)) {
		if (!do_hnat_ext_to_ge(skb, state->in, __func__))
			return NF_STOLEN;
		return NF_ACCEPT;
	}

	/* packets form ge -> external device
	 * For standalone wan interface
	 */
	if (do_ge2ext_fast(state->in, skb)) {
		if (!do_hnat_ge_to_ext(skb, __func__))
			return NF_STOLEN;
		goto drop;
	}


#if !(defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3))
	/* MapE need remove ipv6 header and pingpong. */
	if (do_mape_w2l_fast(state->in, skb)) {
		if (!do_hnat_mape_w2l_fast(skb, state->in, __func__))
			return NF_STOLEN;
		else
			return NF_ACCEPT;
	}

	if (is_from_mape(skb))
		clr_from_extge(skb);
#endif
	if (xlat_toggle)
		mtk_464xlat_pre_process(skb);

	return NF_ACCEPT;
drop:
	if (skb && (debug_level >= 7))
		printk_ratelimited(KERN_WARNING
				   "%s:drop (in_dev=%s, iif=0x%x, CB2=0x%x, ppe_hash=0x%x,\n"
				   "sport=0x%x, reason=0x%x, alg=0x%x)\n",
				   __func__, state->in->name, skb_hnat_iface(skb),
				   HNAT_SKB_CB2(skb)->magic, skb_hnat_entry(skb),
				   skb_hnat_sport(skb), skb_hnat_reason(skb),
				   skb_hnat_alg(skb));

	return NF_DROP;
}

bool is_eth_dev_speed_under(const struct net_device *dev, u32 speed)
{
	const struct mtk_mac *mac;

	if (!dev || !IS_ETH_GRP(dev) || netdev_uses_dsa((struct net_device *)dev))
		return false;

	mac = netdev_priv(dev);

	return mac->speed <= speed;
}

static unsigned int
mtk_hnat_ipv4_nf_pre_routing(void *priv, struct sk_buff *skb,
			     const struct nf_hook_state *state)
{
	struct flow_offload_hw_path hw_path = { 0 };

	if (!skb)
		goto drop;

	if (!is_magic_tag_valid(skb))
		return NF_ACCEPT;

	if (!is_ppe_support_type(skb)) {
		hnat_set_head_frags(state, skb, 1, hnat_set_alg);
		return NF_ACCEPT;
	}

	hnat_set_head_frags(state, skb, -1, hnat_set_iif);

	hw_path.dev = skb->dev;
	hw_path.virt_dev = skb->dev;

	if (skb_hnat_tops(skb) && skb_hnat_is_decap(skb) &&
	    is_magic_tag_valid(skb) &&
	    skb_hnat_iface(skb) == FOE_MAGIC_GE_VIRTUAL &&
	    mtk_tnl_decap_offload && !mtk_tnl_decap_offload(skb))
		return NF_ACCEPT;

	/*
	 * Avoid mistakenly binding of outer IP, ports in SW L2TP decap flow.
	 * In pre-routing, if dev is virtual iface, TOPS module is not loaded,
	 * and it's L2TP flow, then do not bind.
	 */
	if (skb_hnat_iface(skb) == FOE_MAGIC_GE_VIRTUAL &&
	    skb->dev->netdev_ops->ndo_flow_offload_check &&
	    !mtk_tnl_decap_offload) {
		skb->dev->netdev_ops->ndo_flow_offload_check(&hw_path);

		if (hw_path.flags & BIT(DEV_PATH_TNL))
			skb_hnat_alg(skb) = 1;
	}

	pre_routing_print(skb, state->in, state->out, __func__);

	/* packets from external devices -> xxx ,step 1 , learning stage & bound stage*/
	if (do_ext2ge_fast_try(state->in, skb)) {
		if (!do_hnat_ext_to_ge(skb, state->in, __func__))
			return NF_STOLEN;
		return NF_ACCEPT;
	}

	/* packets form ge -> external device
	 * For standalone wan interface
	 */
	if (do_ge2ext_fast(state->in, skb)) {
		if (!do_hnat_ge_to_ext(skb, __func__))
			return NF_STOLEN;
		goto drop;
	}
	if (xlat_toggle)
		mtk_464xlat_pre_process(skb);

	return NF_ACCEPT;
drop:
	if (skb && (debug_level >= 7))
		printk_ratelimited(KERN_WARNING
				   "%s:drop (in_dev=%s, iif=0x%x, CB2=0x%x, ppe_hash=0x%x,\n"
				   "sport=0x%x, reason=0x%x, alg=0x%x)\n",
				   __func__, state->in->name, skb_hnat_iface(skb),
				   HNAT_SKB_CB2(skb)->magic, skb_hnat_entry(skb),
				   skb_hnat_sport(skb), skb_hnat_reason(skb),
				   skb_hnat_alg(skb));

	return NF_DROP;
}

static unsigned int
mtk_hnat_br_nf_local_in(void *priv, struct sk_buff *skb,
			const struct nf_hook_state *state)
{
	struct vlan_ethhdr *veth;

	if (!skb)
		goto drop;

	if (!is_magic_tag_valid(skb))
		return NF_ACCEPT;

	if (IS_HQOS_MODE && hnat_priv->data->whnat) {
		veth = (struct vlan_ethhdr *)skb_mac_header(skb);

		if (eth_hdr(skb)->h_proto == HQOS_MAGIC_TAG) {
			skb_hnat_entry(skb) = ntohs(veth->h_vlan_TCI) & 0x3fff;
			skb_hnat_reason(skb) = HIT_BIND_FORCE_TO_CPU;
		}
	}

	if (!HAS_HQOS_MAGIC_TAG(skb) && !is_ppe_support_type(skb)) {
		hnat_set_head_frags(state, skb, 1, hnat_set_alg);
		return NF_ACCEPT;
	}

	hnat_set_head_frags(state, skb, -1, hnat_set_iif);

	if (skb_hnat_tops(skb) && skb_hnat_is_decap(skb) &&
	    is_magic_tag_valid(skb) &&
	    skb_hnat_iface(skb) == FOE_MAGIC_GE_VIRTUAL &&
	    mtk_tnl_decap_offload && !mtk_tnl_decap_offload(skb))
		return NF_ACCEPT;

	pre_routing_print(skb, state->in, state->out, __func__);

	if (unlikely(debug_level >= 7)) {
		hnat_cpu_reason_cnt(skb);
		if (skb_hnat_reason(skb) == dbg_cpu_reason)
			foe_dump_pkt(skb);
	}

	/* packets from external devices -> xxx ,step 1 , learning stage & bound stage*/
	if ((skb_hnat_iface(skb) == FOE_MAGIC_EXT) && !is_from_extge(skb) &&
	    !is_multicast_ether_addr(eth_hdr(skb)->h_dest)) {
		if (!hnat_priv->g_ppdev)
			hnat_priv->g_ppdev = dev_get_by_name(&init_net, hnat_priv->ppd);

		if (!do_hnat_ext_to_ge(skb, state->in, __func__))
			return NF_STOLEN;
		return NF_ACCEPT;
	}

	if (hnat_priv->data->whnat) {
		if (skb_hnat_iface(skb) == FOE_MAGIC_EXT)
			clr_from_extge(skb);

		/* packets from external devices -> xxx ,step 2, learning stage */
		if (do_ext2ge_fast_learn(state->in, skb) && (!qos_toggle ||
		    (qos_toggle && eth_hdr(skb)->h_proto != HQOS_MAGIC_TAG))) {
			if (!do_hnat_ext_to_ge2(skb, __func__))
				return NF_STOLEN;
			goto drop;
		}

		/* packets form ge -> external device */
		if (do_ge2ext_fast(state->in, skb)) {
			if (!do_hnat_ge_to_ext(skb, __func__))
				return NF_STOLEN;
			goto drop;
		}
	}

#if !(defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3))
	/* MapE need remove ipv6 header and pingpong. (bridge mode) */
	if (do_mape_w2l_fast(state->in, skb)) {
		if (!do_hnat_mape_w2l_fast(skb, state->in, __func__))
			return NF_STOLEN;
		else
			return NF_ACCEPT;
	}
#endif
	return NF_ACCEPT;
drop:
	if (skb && (debug_level >= 7))
		printk_ratelimited(KERN_WARNING
				   "%s:drop (in_dev=%s, iif=0x%x, CB2=0x%x, ppe_hash=0x%x,\n"
				   "sport=0x%x, reason=0x%x, alg=0x%x)\n",
				   __func__, state->in->name, skb_hnat_iface(skb),
				   HNAT_SKB_CB2(skb)->magic, skb_hnat_entry(skb),
				   skb_hnat_sport(skb), skb_hnat_reason(skb),
				   skb_hnat_alg(skb));

	return NF_DROP;
}

static int hnat_ipv6_get_nexthop(struct sk_buff *skb,
				 const struct net_device *out,
				 struct flow_offload_hw_path *hw_path)
{
	const struct in6_addr *ipv6_nexthop;
	struct neighbour *neigh = NULL;
	struct dst_entry *dst = skb_dst(skb);

	if (!dst)
		return 0;

	if (hw_path->flags & BIT(DEV_PATH_PPPOE))
		return 0;

	if (!skb_hnat_cdrt(skb) && dst && dst_xfrm(dst))
		return 0;

	rcu_read_lock_bh();
	ipv6_nexthop =
		rt6_nexthop((struct rt6_info *)dst, &ipv6_hdr(skb)->daddr);
	neigh = __ipv6_neigh_lookup_noref(dst->dev, ipv6_nexthop);
	if (unlikely(!neigh)) {
		dev_notice(hnat_priv->dev, "%s:No neigh (daddr=%pI6)\n", __func__,
			   &ipv6_hdr(skb)->daddr);
		rcu_read_unlock_bh();
		return -1;
	}

	/* why do we get all zero ethernet address ? */
	if (!is_valid_ether_addr(neigh->ha)) {
		rcu_read_unlock_bh();
		return -1;
	}

	ether_addr_copy(hw_path->eth_dest, neigh->ha);
	ether_addr_copy(hw_path->eth_src, out->dev_addr);

	rcu_read_unlock_bh();

	return 0;
}

static int hnat_ipv4_get_nexthop(struct sk_buff *skb,
				 const struct net_device *out,
				 struct flow_offload_hw_path *hw_path)
{
	u32 nexthop;
	struct neighbour *neigh;
	struct dst_entry *dst = skb_dst(skb);
	struct rtable *rt = (struct rtable *)dst;
	struct net_device *dev = (__force struct net_device *)out;

	if (hw_path->flags & BIT(DEV_PATH_PPPOE))
		return 0;

	if (!skb_hnat_cdrt(skb) && dst && dst_xfrm(dst))
		return 0;

	rcu_read_lock_bh();
	nexthop = (__force u32)rt_nexthop(rt, ip_hdr(skb)->daddr);
	neigh = __ipv4_neigh_lookup_noref(dev, nexthop);
	if (unlikely(!neigh)) {
		dev_notice(hnat_priv->dev, "%s:No neigh (daddr=%pI4)\n", __func__,
			   &ip_hdr(skb)->daddr);
		rcu_read_unlock_bh();
		return -1;
	}

	/* why do we get all zero ethernet address ? */
	if (!is_valid_ether_addr(neigh->ha)) {
		rcu_read_unlock_bh();
		return -1;
	}

	memcpy(hw_path->eth_dest, neigh->ha, ETH_ALEN);
	memcpy(hw_path->eth_src, out->dev_addr, ETH_ALEN);

	rcu_read_unlock_bh();

	return 0;
}

static u16 ppe_get_chkbase(struct iphdr *iph)
{
	u16 org_chksum = ntohs(iph->check);
	u16 org_tot_len = ntohs(iph->tot_len);
	u16 org_id = ntohs(iph->id);
	u16 chksum_tmp, tot_len_tmp, id_tmp;
	u32 tmp = 0;
	u16 chksum_base = 0;

	chksum_tmp = ~(org_chksum);
	tot_len_tmp = ~(org_tot_len);
	id_tmp = ~(org_id);
	tmp = chksum_tmp + tot_len_tmp + id_tmp;
	tmp = ((tmp >> 16) & 0x7) + (tmp & 0xFFFF);
	tmp = ((tmp >> 16) & 0x7) + (tmp & 0xFFFF);
	chksum_base = tmp & 0xFFFF;

	return chksum_base;
}

static int hnat_add_vlan_layer(struct foe_entry *entry, u16 vlan_tci, bool outer)
{
	if (entry->bfib1.vlan_layer >= 2)
		return -EINVAL;

	if (IS_IPV4_GRP(entry) || IS_L2_BRIDGE(entry)) {
		if (outer) {
			/* if the vlan is outer layer, insert it into vlan1 field */
			entry->ipv4_hnapt.vlan2 = entry->ipv4_hnapt.vlan1;
			entry->ipv4_hnapt.vlan1 = vlan_tci;
		} else if (entry->bfib1.vlan_layer == 0) {
			entry->ipv4_hnapt.vlan1 = vlan_tci;
		} else if (entry->bfib1.vlan_layer == 1) {
			entry->ipv4_hnapt.vlan2 = vlan_tci;
		}
	} else {
		if (outer) {
			/* if the vlan is outer layer, insert it into vlan1 field */
			entry->ipv6_5t_route.vlan2 = entry->ipv6_5t_route.vlan1;
			entry->ipv6_5t_route.vlan1 = vlan_tci;
		} else if (entry->bfib1.vlan_layer == 0) {
			entry->ipv6_5t_route.vlan1 = vlan_tci;
		} else if (entry->bfib1.vlan_layer == 1) {
			entry->ipv6_5t_route.vlan2 = vlan_tci;
		}
	}

	entry->bfib1.vlan_layer++;

	return 0;
}

static struct foe_entry ppe_fill_L2_info(struct foe_entry entry,
					 struct flow_offload_hw_path *hw_path)
{
	switch ((int)entry.bfib1.pkt_type) {
	case L2_BRIDGE:
		entry.l2_bridge.new_dmac_hi = swab32(*((u32 *)hw_path->eth_dest));
		entry.l2_bridge.new_dmac_lo = swab16(*((u16 *)&hw_path->eth_dest[4]));
		entry.l2_bridge.new_smac_hi = swab32(*((u32 *)hw_path->eth_src));
		entry.l2_bridge.new_smac_lo = swab16(*((u16 *)&hw_path->eth_src[4]));
		break;
	case IPV4_HNAPT:
	case IPV4_HNAT:
		entry.ipv4_hnapt.dmac_hi = swab32(*((u32 *)hw_path->eth_dest));
		entry.ipv4_hnapt.dmac_lo = swab16(*((u16 *)&hw_path->eth_dest[4]));
		entry.ipv4_hnapt.smac_hi = swab32(*((u32 *)hw_path->eth_src));
		entry.ipv4_hnapt.smac_lo = swab16(*((u16 *)&hw_path->eth_src[4]));
		entry.ipv4_hnapt.pppoe_id = hw_path->pppoe_sid;
		break;
	case IPV4_DSLITE:
	case IPV4_MAP_E:
	case IPV6_6RD:
	case IPV6_5T_ROUTE:
	case IPV6_3T_ROUTE:
	case IPV6_HNAPT:
	case IPV6_HNAT:
		entry.ipv6_5t_route.dmac_hi = swab32(*((u32 *)hw_path->eth_dest));
		entry.ipv6_5t_route.dmac_lo = swab16(*((u16 *)&hw_path->eth_dest[4]));
		entry.ipv6_5t_route.smac_hi = swab32(*((u32 *)hw_path->eth_src));
		entry.ipv6_5t_route.smac_lo = swab16(*((u16 *)&hw_path->eth_src[4]));
		entry.ipv6_5t_route.pppoe_id = hw_path->pppoe_sid;
		break;
	}
	return entry;
}

static bool hnat_is_hw_path_bridging(struct flow_offload_hw_path *hw_path)
{
	struct net_device *dev = hw_path->virt_dev;
	struct net_device *br_dev;
	bool ret = false;

	/* Only check for devices that are bridge slave ports */
	if (dev && netif_is_bridge_port(dev)) {
		rcu_read_lock_bh();
		/* Verify the upper device is indeed a bridge master */
		br_dev = netdev_master_upper_dev_get_rcu(dev);
		if (br_dev && netif_is_bridge_master(br_dev)) {
			/* Not from the bridge master nor the port itself -> bridged frame */
			if (!ether_addr_equal(hw_path->eth_src, br_dev->dev_addr) &&
			    !ether_addr_equal(hw_path->eth_src, dev->dev_addr))
				ret = true;
		}
		rcu_read_unlock_bh();
	}

	return ret;
}

static struct foe_entry ppe_fill_info_blk(struct foe_entry entry,
					  struct flow_offload_hw_path *hw_path)
{
	entry.bfib1.psn = (hw_path->flags & BIT(DEV_PATH_PPPOE)) ? 1 : 0;
	entry.bfib1.vlan_layer += (hw_path->flags & BIT(DEV_PATH_VLAN)) ? 1 : 0;
	/* after enabled IS_SP_TAG_EN in NETSYS_V3, info1.vpm becomes IS_SP_TAG flag control */
	entry.bfib1.vpm = 0;
	entry.bfib1.cah = 1;
	entry.bfib1.sta = 0;
	/* TTL should not be decremented in bridge layer forward */
	entry.bfib1.ttl = hnat_is_hw_path_bridging(hw_path) ? 0 : 1;

	switch ((int)entry.bfib1.pkt_type) {
	case L2_BRIDGE:
		if (IS_MCAST_MULTI_MODE &&
		    is_multicast_ether_addr(&hw_path->eth_dest[0]))
			entry.l2_bridge.iblk2.mcast = 1;
		else
			entry.l2_bridge.iblk2.mcast = 0;

		entry.l2_bridge.iblk2.port_ag = 0xf;

		if (entry.bfib1.vlan_layer)
			entry.l2_bridge.sp_tag = ETH_P_8021Q;
		break;
	case IPV4_HNAPT:
	case IPV4_HNAT:
		if (IS_MCAST_MULTI_MODE &&
		    is_multicast_ether_addr(&hw_path->eth_dest[0])) {
			entry.ipv4_hnapt.iblk2.mcast = 1;
			if (hnat_priv->data->version == MTK_HNAT_V1_3) {
				entry.bfib1.sta = 1;
				entry.ipv4_hnapt.m_timestamp = foe_timestamp(hnat_priv, true);
			}
		} else {
			entry.ipv4_hnapt.iblk2.mcast = 0;
		}
#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
		entry.ipv4_hnapt.iblk2.port_ag = 0xf;
#else
		entry.ipv4_hnapt.iblk2.port_ag = 0x3f;
#endif
		if (entry.bfib1.vlan_layer)
			entry.ipv4_hnapt.sp_tag = ETH_P_8021Q;
		break;
	case IPV4_DSLITE:
	case IPV4_MAP_E:
	case IPV6_6RD:
	case IPV6_5T_ROUTE:
	case IPV6_3T_ROUTE:
	case IPV6_HNAPT:
	case IPV6_HNAT:
		if (IS_MCAST_MULTI_MODE &&
		    is_multicast_ether_addr(&hw_path->eth_dest[0])) {
			entry.ipv6_5t_route.iblk2.mcast = 1;
			if (hnat_priv->data->version == MTK_HNAT_V1_3) {
				entry.bfib1.sta = 1;
				entry.ipv4_hnapt.m_timestamp = foe_timestamp(hnat_priv, true);
			}
		} else {
			entry.ipv6_5t_route.iblk2.mcast = 0;
		}

#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
		entry.ipv6_5t_route.iblk2.port_ag = 0xf;
#else
		entry.ipv6_5t_route.iblk2.port_ag = 0x3f;
#endif
		if (entry.bfib1.vlan_layer)
			entry.ipv6_5t_route.sp_tag = ETH_P_8021Q;
		break;
	}
	return entry;
}

static inline void hnat_get_filled_unbind_entry(struct sk_buff *skb,
						struct foe_entry *entry)
{
	if (unlikely(!skb || !entry))
		return;

	memcpy(entry,
	       &hnat_priv->foe_table_cpu[skb_hnat_ppe(skb)][skb_hnat_entry(skb)],
	       sizeof(*entry));

#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
	entry->bfib1.mc = 0;
#endif /* defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3) */
	entry->bfib1.ka = 0;
	entry->bfib1.vlan_layer = 0;
	entry->bfib1.psn = 0;
	entry->bfib1.vpm = 0;
	entry->bfib1.ps = 0;
}

/*
 * check offload engine data is prepared
 * return 0 for packets not related to offload engine
 * return positive value for offload engine prepared data done
 * return negative value for data is still constructing
 */
static inline int hnat_offload_engine_done(struct sk_buff *skb,
					   struct flow_offload_hw_path *hw_path)
{
	struct dst_entry *dst = skb_dst(skb);

	if ((skb_hnat_tops(skb) && !(hw_path->flags & BIT(DEV_PATH_TNL)))) {
		if (!tnl_toggle)
			return -1;

		/* tunnel encap'ed */
		if (dst && dst_xfrm(dst))
			/*
			 * skb not ready to bind since it is still needs
			 * to be encrypted
			 */
			return -1;

		/* nothing need to be done further for this skb */
		return 1;
	}

	/* no need for tunnel encapsulation or crypto encryption */
	return 0;
}

static inline void hnat_fill_offload_engine_entry(struct sk_buff *skb,
						  struct foe_entry *entry,
						  const struct net_device *dev)
{
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
	if (!tnl_toggle)
		return;

	if (skb_hnat_tops(skb) && skb_hnat_is_encap(skb)) {
		/*
		 * if skb_hnat_tops(skb) is setup for encapsulation,
		 * we fill in hnat tport and tops_entry for tunnel encapsulation
		 * offloading
		 */
		if (IS_IPV4_GRP(entry)) {
			if (skb_hnat_cdrt(skb) && skb_hnat_is_encrypt(skb)) {
				entry->ipv4_hnapt.tport_id = NR_TDMA_EIP197_QDMA_TPORT;
				entry->ipv4_hnapt.cdrt_id = skb_hnat_cdrt(skb);
			} else
				entry->ipv4_hnapt.tport_id = NR_TDMA_QDMA_TPORT;
			entry->ipv4_hnapt.tops_entry = skb_hnat_tops(skb);
		} else {
			if (skb_hnat_cdrt(skb) && skb_hnat_is_encrypt(skb)) {
				entry->ipv6_5t_route.tport_id = NR_TDMA_EIP197_QDMA_TPORT;
				entry->ipv6_5t_route.cdrt_id = skb_hnat_cdrt(skb);
			} else
				entry->ipv6_5t_route.tport_id = NR_TDMA_QDMA_TPORT;
			entry->ipv6_5t_route.tops_entry = skb_hnat_tops(skb);
		}
	} else if (skb_hnat_cdrt(skb) && skb_hnat_is_encrypt(skb)) {
		if (IS_IPV4_GRP(entry)) {
			entry->ipv4_hnapt.tport_id = NR_EIP197_QDMA_TPORT;
			entry->ipv4_hnapt.cdrt_id = skb_hnat_cdrt(skb);
		} else {
			entry->ipv6_5t_route.tport_id = NR_EIP197_QDMA_TPORT;
			entry->ipv6_5t_route.cdrt_id = skb_hnat_cdrt(skb);
		}
	} else
		return;
#endif /* defined(CONFIG_MEDIATEK_NETSYS_V3) */
}

static int hnat_foe_entry_commit(struct foe_entry *foe,
				 struct foe_entry *entry,
				 u32 state)
{
	/* Renew the entry timestamp */
	entry->bfib1.time_stamp = foe_timestamp(hnat_priv, false);
	/* After other fields have been written, write state to the entry */
	entry->bfib1.state = state;

	/* We must ensure all info has been updated before set to hw */
	wmb();
	/* Before entry enter BIND state, write other fields first,
	 * prevent racing with hardware accesses.
	 */
	memcpy(&foe->data, &entry->data,
	       PPE_ENTRY_SIZE - sizeof(foe->info_blk1));
	/* We must ensure all info has been updated before set to hw */
	wmb();

	foe->bfib1 = entry->bfib1;
	/* We must ensure all info has been updated */
	dma_wmb();

	return 0;
}

int hnat_bind_crypto_entry(struct sk_buff *skb, const struct net_device *dev, int fill_inner_info)
{
	struct net_device *master_dev;
	struct foe_entry *foe;
	struct foe_entry entry = { 0 };
	struct ethhdr eth = {0};
	struct ipv6hdr *ip6h;
	struct iphdr *iph;
	struct tcpudphdr _ports;
	const struct tcpudphdr *pptr;
	u32 gmac = NR_DISCARD;
	int port_id = 0;
	int udp = 0;
	u32 qid;
	struct mtk_mac *mac;
	struct flow_offload_hw_path hw_path = { .dev = (struct net_device *) dev,
						.virt_dev = (struct net_device *) dev };
	u16 h_proto = 0;

	if (!tnl_toggle) {
		pr_notice("tnl_toggle is disable now!|\n");
		return -1;
	}

	if (!skb_hnat_is_hashed(skb) || skb_hnat_ppe(skb) >= CFG_PPE_NUM)
		return 0;

	foe = &hnat_priv->foe_table_cpu[skb_hnat_ppe(skb)][skb_hnat_entry(skb)];

	if (entry_hnat_is_bound(foe))
		return 0;

	if (ip_hdr(skb)->version == IPVERSION_V4)
		h_proto = ETH_P_IP;
	else if (ip_hdr(skb)->version == IPVERSION_V6)
		h_proto = ETH_P_IPV6;
	else
		return 0;

	hnat_get_filled_unbind_entry(skb, &entry);

	if (dev->netdev_ops->ndo_flow_offload_check) {
		dev->netdev_ops->ndo_flow_offload_check(&hw_path);
		dev = (IS_GMAC1_MODE) ? hw_path.virt_dev : hw_path.dev;
	}

	if (hw_path.flags & BIT(DEV_PATH_PPPOE)) {
		memcpy(eth.h_dest, hw_path.eth_dest, ETH_ALEN);
		memcpy(eth.h_source, hw_path.eth_src, ETH_ALEN);
		eth.h_proto = htons(h_proto);
		if (skb_hnat_tops(skb) && mtk_tnl_encap_offload)
			mtk_tnl_encap_offload(skb, &eth);
	} else {
		memcpy(hw_path.eth_dest, eth_hdr(skb)->h_dest, ETH_ALEN);
		memcpy(hw_path.eth_src, eth_hdr(skb)->h_source, ETH_ALEN);
		if (skb_hnat_tops(skb) && mtk_tnl_encap_offload)
			mtk_tnl_encap_offload(skb, eth_hdr(skb));
	}

	if (!fill_inner_info)
		goto hnat_skip_fill_inner;

	/* For packets pass through VTI (route-based IPSec),
	 * We need to fill the inner packet info into hnat entry.
	 * Since the skb->mac_header is not pointed to correct position
	 * in skb_to_hnat_info().
	 */
	switch (h_proto) {
	case ETH_P_IP:
		iph = ip_hdr(skb);

		switch (iph->protocol) {
		case IPPROTO_UDP:
			udp = 1;
			fallthrough;
		case IPPROTO_TCP:
			entry.ipv4_hnapt.sp_tag = htons(ETH_P_IP);
			if (IS_IPV4_GRP(&entry)) {
				entry.ipv4_hnapt.iblk2.dscp = iph->tos;
				if (hnat_priv->data->per_flow_accounting)
					entry.ipv4_hnapt.iblk2.mibf = 1;

				entry.ipv4_hnapt.vlan1 = hw_path.vlan_id;

				if (skb_vlan_tagged(skb)) {
					entry.bfib1.vlan_layer += 1;

					if (entry.ipv4_hnapt.vlan1)
						entry.ipv4_hnapt.vlan2 = skb->vlan_tci;
					else
						entry.ipv4_hnapt.vlan1 = skb->vlan_tci;
				}

				entry.ipv4_hnapt.sip = foe->ipv4_hnapt.sip;
				entry.ipv4_hnapt.dip = foe->ipv4_hnapt.dip;
				entry.ipv4_hnapt.sport = foe->ipv4_hnapt.sport;
				entry.ipv4_hnapt.dport = foe->ipv4_hnapt.dport;

				entry.ipv4_hnapt.new_sip = ntohl(iph->saddr);
				entry.ipv4_hnapt.new_dip = ntohl(iph->daddr);

				if (IS_IPV4_HNAPT(&entry)) {
					pptr = skb_header_pointer(skb,
								iph->ihl * 4 + ETH_HLEN,
								sizeof(_ports),
								&_ports);
					if (unlikely(!pptr))
						return -1;

					entry.ipv4_hnapt.new_sport = ntohs(pptr->src);
					entry.ipv4_hnapt.new_dport = ntohs(pptr->dst);
				}
			} else
				return 0;

			entry.bfib1.udp = udp;

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
			entry.ipv4_hnapt.eg_keep_ecn = 1;
			entry.ipv4_hnapt.eg_keep_dscp = 1;
#endif
			break;

		default:
			return -1;
		}
		break;
	case ETH_P_IPV6:
		ip6h = ipv6_hdr(skb);
		switch (ip6h->nexthdr) {
		case NEXTHDR_UDP:
			udp = 1;
			fallthrough;
		case NEXTHDR_TCP:
			entry.ipv6_5t_route.sp_tag = htons(ETH_P_IPV6);
			entry.ipv6_5t_route.vlan1 = hw_path.vlan_id;

			if (skb_vlan_tagged(skb)) {
				entry.bfib1.vlan_layer += 1;

				if (entry.ipv6_5t_route.vlan1)
					entry.ipv6_5t_route.vlan2 =
						skb->vlan_tci;
				else
					entry.ipv6_5t_route.vlan1 =
						skb->vlan_tci;
			}

			if (hnat_priv->data->per_flow_accounting)
				entry.ipv6_5t_route.iblk2.mibf = 1;
			entry.bfib1.udp = udp;

			entry.ipv6_3t_route.ipv6_sip0 =
				foe->ipv6_3t_route.ipv6_sip0;
			entry.ipv6_3t_route.ipv6_sip1 =
				foe->ipv6_3t_route.ipv6_sip1;
			entry.ipv6_3t_route.ipv6_sip2 =
				foe->ipv6_3t_route.ipv6_sip2;
			entry.ipv6_3t_route.ipv6_sip3 =
				foe->ipv6_3t_route.ipv6_sip3;

			entry.ipv6_3t_route.ipv6_dip0 =
				foe->ipv6_3t_route.ipv6_dip0;
			entry.ipv6_3t_route.ipv6_dip1 =
				foe->ipv6_3t_route.ipv6_dip1;
			entry.ipv6_3t_route.ipv6_dip2 =
				foe->ipv6_3t_route.ipv6_dip2;
			entry.ipv6_3t_route.ipv6_dip3 =
				foe->ipv6_3t_route.ipv6_dip3;

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
			entry.ipv6_3t_route.eg_keep_ecn = 1;
			entry.ipv6_3t_route.eg_keep_cls = 1;
#endif
			if (IS_IPV6_3T_ROUTE(&entry)) {
				entry.ipv6_3t_route.prot =
					foe->ipv6_3t_route.prot;
				entry.ipv6_3t_route.hph =
					foe->ipv6_3t_route.hph;
			} else if (IS_IPV6_5T_ROUTE(&entry)) {
				entry.ipv6_5t_route.sport =
					foe->ipv6_5t_route.sport;
				entry.ipv6_5t_route.dport =
					foe->ipv6_5t_route.dport;
			} else {
				return 0;
			}

			entry.ipv6_5t_route.iblk2.dscp =
				(ip6h->priority << 4 |
				(ip6h->flow_lbl[0] >> 4));
			break;
		default:
			return -1;
		}
		break;

	default:
		return -1;
	}

hnat_skip_fill_inner:
	entry = ppe_fill_info_blk(entry, &hw_path);

	if (IS_LAN_GRP(dev) || IS_WAN(dev)) { /* Forward to GMAC Ports */
		master_dev = (struct net_device *)dev;
		port_id = hnat_dsa_get_port(&master_dev);
		if (port_id >= 0) {
			if (hnat_dsa_fill_stag(dev, &entry, h_proto, 0) < 0)
				return -1;
		}

		mac = netdev_priv(master_dev);
		gmac = HNAT_GMAC_FP(mac->id);
	} else {
		pr_notice("Unknown case of dp, iif=%x --> %s\n", skb_hnat_iface(skb), dev->name);
		return -1;
	}

	if (IS_IPV4_GRP(&entry)) {
		entry.ipv4_hnapt.iblk2.dp = gmac;
		entry.ipv4_hnapt.iblk2.port_mg =
			(hnat_priv->data->version == MTK_HNAT_V1_1) ? 0x3f : 0;
		entry.bfib1.ttl = 1;
		entry.bfib1.state = BIND;

		/* For L2 GRE and L2 VXLAN, should not modify MAC address */
		if (!skb_hnat_tops(skb)) {
			entry.ipv4_hnapt.dmac_hi = swab32(*((u32 *)hw_path.eth_dest));
			entry.ipv4_hnapt.dmac_lo = swab16(*((u16 *)&hw_path.eth_dest[4]));
			entry.ipv4_hnapt.smac_hi = swab32(*((u32 *)hw_path.eth_src));
			entry.ipv4_hnapt.smac_lo = swab16(*((u16 *)&hw_path.eth_src[4]));
		}
		entry.ipv4_hnapt.pppoe_id = hw_path.pppoe_sid;
	} else {
		entry.ipv6_5t_route.iblk2.dp = gmac;
		entry.ipv6_5t_route.iblk2.port_mg =
			(hnat_priv->data->version == MTK_HNAT_V1_1) ? 0x3f : 0;
		entry.bfib1.ttl = 1;
		entry.bfib1.state = BIND;
		if (!skb_hnat_tops(skb)) {
			entry.ipv6_5t_route.dmac_hi = swab32(*((u32 *)hw_path.eth_dest));
			entry.ipv6_5t_route.dmac_lo = swab16(*((u16 *)&hw_path.eth_dest[4]));
			entry.ipv6_5t_route.smac_hi = swab32(*((u32 *)hw_path.eth_src));
			entry.ipv6_5t_route.smac_lo = swab16(*((u16 *)&hw_path.eth_src[4]));
		}
		entry.ipv6_5t_route.pppoe_id = hw_path.pppoe_sid;
	}

	if (IS_HQOS_MODE || (skb->mark & MTK_QDMA_QUEUE_MASK) >= MAX_PPPQ_QUEUE_NUM)
		qid = skb->mark & (MTK_QDMA_QUEUE_MASK);
	else if (IS_PPPQ_MODE && IS_PPPQ_PATH(dev, skb))
		qid = ((port_id >= 0) ? MTK_GMAC_ID_MAX + port_id : mac->id) & MTK_QDMA_QUEUE_MASK;
	else
		qid = 0;

	if (IS_IPV4_GRP(&entry))
		entry.ipv4_hnapt.iblk2.qid = qid;
	else
		entry.ipv6_5t_route.iblk2.qid = qid;

	hnat_fill_offload_engine_entry(skb, &entry, dev);

	/* wait for entry written done */
	wmb();

	if (entry_hnat_is_bound(foe))
		return 0;

	spin_lock(&hnat_priv->entry_lock);
	hnat_foe_entry_commit(foe, &entry, BIND);
	spin_unlock(&hnat_priv->entry_lock);

	if (hnat_priv->data->per_flow_accounting &&
	    skb_hnat_entry(skb) < hnat_priv->foe_etry_num &&
	    skb_hnat_ppe(skb) < CFG_PPE_NUM)
		memset(&hnat_priv->acct[skb_hnat_ppe(skb)][skb_hnat_entry(skb)],
		       0, sizeof(struct mib_entry));

	return 0;
}
EXPORT_SYMBOL(hnat_bind_crypto_entry);

static int skb_to_hnat_info(struct sk_buff *skb,
			    const struct net_device *dev,
			    struct foe_entry *foe,
			    struct flow_offload_hw_path *hw_path)
{
	struct net_device *master_dev = (struct net_device *)dev;
	struct foe_entry entry = { 0 };
	struct hnat_flow_entry *flow_entry;
	struct mtk_mac *mac;
	struct tcphdr *tcph;
	struct iphdr *iph;
	struct ipv6hdr *ip6h;
	struct tcpudphdr _ports;
	const struct tcpudphdr *pptr;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	int whnat = IS_WHNAT(dev);
	int gmac = NR_DISCARD;
	int port_id = 0;
	int mape = 0;
	int udp = 0;
	int ret;
	u32 payload_len = 0;
	u32 qid = 0;
	u16 h_offset = 0;
	u16 h_proto = 0;

	/*do not bind multicast if PPE mcast not enable*/
	if (!hnat_priv->data->mcast && is_multicast_ether_addr(hw_path->eth_dest))
		return -1;

	ret = hnat_offload_engine_done(skb, hw_path);
	if (ret == 1) {
		hnat_get_filled_unbind_entry(skb, &entry);
		goto hnat_entry_bind;
	} else if (ret == -1) {
		return -1;
	}

	entry.bfib1.pkt_type = foe->udib1.pkt_type; /* Get packte type state*/
	entry.bfib1.state = foe->udib1.state;

	if (unlikely(entry.bfib1.state != UNBIND))
		return -1;

#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
	entry.bfib1.sp = foe->udib1.sp;
#endif

	if (ntohs(eth_hdr(skb)->h_proto) == ETH_P_PPP_SES) {
		/* Apply PPPoE Bridge packet info to hw_path for further PPE entry preparing */
		hw_path->flags |= BIT(DEV_PATH_PPPOE);
		hw_path->pppoe_sid = ntohs(pppoe_hdr(skb)->sid);
		h_proto = hnat_get_hdr_protocol(skb, &h_offset);
	} else if (ip_hdr(skb)->version == IPVERSION_V4)
		h_proto = ETH_P_IP;
	else if (ip_hdr(skb)->version == IPVERSION_V6)
		h_proto = ETH_P_IPV6;
	else
		return -1;

	switch (h_proto) {
	case ETH_P_IP:
		iph = (struct iphdr *)(skb_network_header(skb) + h_offset);
		/* Do not bind if pkt is fragmented */
		if (ip_is_fragment(iph))
			return -1;

		switch (iph->protocol) {
		case IPPROTO_UDP:
			udp = 1;
			fallthrough;
		case IPPROTO_TCP:
			entry.ipv4_hnapt.sp_tag = htons(ETH_P_IP);

			/* DS-Lite WAN->LAN */
			if (IS_IPV4_DSLITE(&entry) || IS_IPV4_MAPE(&entry)) {
				entry.ipv4_dslite.sip = foe->ipv4_dslite.sip;
				entry.ipv4_dslite.dip = foe->ipv4_dslite.dip;
				entry.ipv4_dslite.sport =
					foe->ipv4_dslite.sport;
				entry.ipv4_dslite.dport =
					foe->ipv4_dslite.dport;

#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
				if (IS_IPV4_MAPE(&entry)) {
					pptr = skb_header_pointer(skb,
								  iph->ihl * 4 + h_offset,
								  sizeof(_ports),
								  &_ports);
					if (unlikely(!pptr))
						return -1;

					entry.ipv4_mape.new_sip =
							ntohl(iph->saddr);
					entry.ipv4_mape.new_dip =
							ntohl(iph->daddr);
					entry.ipv4_mape.new_sport =
							ntohs(pptr->src);
					entry.ipv4_mape.new_dport =
							ntohs(pptr->dst);
				}
#endif

				entry.ipv4_dslite.tunnel_sipv6_0 =
					foe->ipv4_dslite.tunnel_sipv6_0;
				entry.ipv4_dslite.tunnel_sipv6_1 =
					foe->ipv4_dslite.tunnel_sipv6_1;
				entry.ipv4_dslite.tunnel_sipv6_2 =
					foe->ipv4_dslite.tunnel_sipv6_2;
				entry.ipv4_dslite.tunnel_sipv6_3 =
					foe->ipv4_dslite.tunnel_sipv6_3;

				entry.ipv4_dslite.tunnel_dipv6_0 =
					foe->ipv4_dslite.tunnel_dipv6_0;
				entry.ipv4_dslite.tunnel_dipv6_1 =
					foe->ipv4_dslite.tunnel_dipv6_1;
				entry.ipv4_dslite.tunnel_dipv6_2 =
					foe->ipv4_dslite.tunnel_dipv6_2;
				entry.ipv4_dslite.tunnel_dipv6_3 =
					foe->ipv4_dslite.tunnel_dipv6_3;

				entry.bfib1.rmt = 1;
				entry.ipv4_dslite.iblk2.dscp = iph->tos;
				entry.ipv4_dslite.vlan1 = hw_path->vlan_id;
				if (hnat_priv->data->per_flow_accounting)
					entry.ipv4_dslite.iblk2.mibf = 1;

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
				entry.ipv4_dslite.eg_keep_ecn = 1;
				entry.ipv4_dslite.eg_keep_cls = 1;
#endif

			} else if (IS_IPV4_GRP(&entry)) {
				entry.ipv4_hnapt.iblk2.dscp = iph->tos;
				if (hnat_priv->data->per_flow_accounting)
					entry.ipv4_hnapt.iblk2.mibf = 1;

				entry.ipv4_hnapt.vlan1 = hw_path->vlan_id;

				if (skb_vlan_tagged(skb)) {
					entry.bfib1.vlan_layer += 1;

					if (entry.ipv4_hnapt.vlan1)
						entry.ipv4_hnapt.vlan2 =
							skb->vlan_tci;
					else
						entry.ipv4_hnapt.vlan1 =
							skb->vlan_tci;
				}

				entry.ipv4_hnapt.sip = foe->ipv4_hnapt.sip;
				entry.ipv4_hnapt.dip = foe->ipv4_hnapt.dip;
				entry.ipv4_hnapt.sport = foe->ipv4_hnapt.sport;
				entry.ipv4_hnapt.dport = foe->ipv4_hnapt.dport;

				entry.ipv4_hnapt.new_sip = ntohl(iph->saddr);
				entry.ipv4_hnapt.new_dip = ntohl(iph->daddr);

				if (IS_IPV4_HNAPT(&entry)) {
					pptr = skb_header_pointer(skb, iph->ihl * 4 + h_offset,
								sizeof(_ports),
								&_ports);
					if (unlikely(!pptr))
						return -1;

					entry.ipv4_hnapt.new_sport = ntohs(pptr->src);
					entry.ipv4_hnapt.new_dport = ntohs(pptr->dst);
				}

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
				entry.ipv4_hnapt.eg_keep_ecn = 1;
				entry.ipv4_hnapt.eg_keep_dscp = 1;
#endif
			} else {
				return -1;
			}

			entry.bfib1.udp = udp;
			break;

		case IPPROTO_IPV6:
			/* 6RD LAN->WAN(6to4) */
			if (entry.bfib1.pkt_type == IPV6_6RD) {
				entry.ipv6_6rd.ipv6_sip0 = foe->ipv6_6rd.ipv6_sip0;
				entry.ipv6_6rd.ipv6_sip1 = foe->ipv6_6rd.ipv6_sip1;
				entry.ipv6_6rd.ipv6_sip2 = foe->ipv6_6rd.ipv6_sip2;
				entry.ipv6_6rd.ipv6_sip3 = foe->ipv6_6rd.ipv6_sip3;

				entry.ipv6_6rd.ipv6_dip0 = foe->ipv6_6rd.ipv6_dip0;
				entry.ipv6_6rd.ipv6_dip1 = foe->ipv6_6rd.ipv6_dip1;
				entry.ipv6_6rd.ipv6_dip2 = foe->ipv6_6rd.ipv6_dip2;
				entry.ipv6_6rd.ipv6_dip3 = foe->ipv6_6rd.ipv6_dip3;

				entry.ipv6_6rd.sport = foe->ipv6_6rd.sport;
				entry.ipv6_6rd.dport = foe->ipv6_6rd.dport;
				entry.ipv6_6rd.tunnel_sipv4 = ntohl(iph->saddr);
				entry.ipv6_6rd.tunnel_dipv4 = ntohl(iph->daddr);
				entry.ipv6_6rd.hdr_chksum = ppe_get_chkbase(iph);
				entry.ipv6_6rd.flag = (ntohs(iph->frag_off) >> 13);
				entry.ipv6_6rd.ttl = iph->ttl;
				entry.ipv6_6rd.dscp = iph->tos;
				entry.ipv6_6rd.per_flow_6rd_id = 1;
				entry.ipv6_6rd.vlan1 = hw_path->vlan_id;
				if (hnat_priv->data->per_flow_accounting)
					entry.ipv6_6rd.iblk2.mibf = 1;

				ip6h = (struct ipv6hdr *)skb_inner_network_header(skb);
				if (ip6h->nexthdr == NEXTHDR_UDP)
					entry.bfib1.udp = 1;

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
				entry.ipv6_6rd.eg_keep_ecn = 1;
				entry.ipv6_6rd.eg_keep_cls = 1;
#endif
				break;
			}

			return -1;
		default:
			return -1;
		}
		if (debug_level >= 7)
			trace_printk(
				"[%s]skb->head=%p, skb->data=%p,ip_hdr=%p, skb->len=%d, skb->data_len=%d\n",
				__func__, skb->head, skb->data, iph, skb->len,
				skb->data_len);
		break;

	case ETH_P_IPV6:
		ip6h = (struct ipv6hdr *)(skb_network_header(skb) + h_offset);
		switch (ip6h->nexthdr) {
		case NEXTHDR_UDP:
			udp = 1;
			fallthrough;
		case NEXTHDR_TCP: /* IPv6-5T or IPv6-3T */
			entry.ipv6_5t_route.sp_tag = htons(ETH_P_IPV6);

			entry.ipv6_5t_route.vlan1 = hw_path->vlan_id;

			if (skb_vlan_tagged(skb)) {
				entry.bfib1.vlan_layer += 1;

				if (entry.ipv6_5t_route.vlan1)
					entry.ipv6_5t_route.vlan2 =
						skb->vlan_tci;
				else
					entry.ipv6_5t_route.vlan1 =
						skb->vlan_tci;
			}

			if (hnat_priv->data->per_flow_accounting)
				entry.ipv6_5t_route.iblk2.mibf = 1;
			entry.bfib1.udp = udp;

			if (IS_IPV6_6RD(&entry)) {
				entry.bfib1.rmt = 1;
				entry.ipv6_6rd.tunnel_sipv4 =
					foe->ipv6_6rd.tunnel_sipv4;
				entry.ipv6_6rd.tunnel_dipv4 =
					foe->ipv6_6rd.tunnel_dipv4;
			}

			entry.ipv6_3t_route.ipv6_sip0 =
				foe->ipv6_3t_route.ipv6_sip0;
			entry.ipv6_3t_route.ipv6_sip1 =
				foe->ipv6_3t_route.ipv6_sip1;
			entry.ipv6_3t_route.ipv6_sip2 =
				foe->ipv6_3t_route.ipv6_sip2;
			entry.ipv6_3t_route.ipv6_sip3 =
				foe->ipv6_3t_route.ipv6_sip3;

			entry.ipv6_3t_route.ipv6_dip0 =
				foe->ipv6_3t_route.ipv6_dip0;
			entry.ipv6_3t_route.ipv6_dip1 =
				foe->ipv6_3t_route.ipv6_dip1;
			entry.ipv6_3t_route.ipv6_dip2 =
				foe->ipv6_3t_route.ipv6_dip2;
			entry.ipv6_3t_route.ipv6_dip3 =
				foe->ipv6_3t_route.ipv6_dip3;

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
			entry.ipv6_3t_route.eg_keep_ecn = 1;
			entry.ipv6_3t_route.eg_keep_cls = 1;
#endif

			if (IS_IPV6_3T_ROUTE(&entry)) {
				entry.ipv6_3t_route.prot =
					foe->ipv6_3t_route.prot;
				entry.ipv6_3t_route.hph =
					foe->ipv6_3t_route.hph;
			} else if (IS_IPV6_5T_ROUTE(&entry) || IS_IPV6_6RD(&entry)) {
				entry.ipv6_5t_route.sport =
					foe->ipv6_5t_route.sport;
				entry.ipv6_5t_route.dport =
					foe->ipv6_5t_route.dport;
			} else {
				return -1;
			}

			if (IS_IPV6_5T_ROUTE(&entry) &&
			    (!hnat_ipv6_addr_equal(&entry.ipv6_5t_route.ipv6_sip0, &ip6h->saddr) ||
			     !hnat_ipv6_addr_equal(&entry.ipv6_5t_route.ipv6_dip0, &ip6h->daddr))) {
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
				entry.bfib1.pkt_type = IPV6_HNAPT;

				if (IS_WAN(dev) || IS_DSA_WAN(dev)) {
					entry.ipv6_hnapt.eg_ipv6_dir =
						IPV6_SNAT;
					entry.ipv6_hnapt.new_ipv6_ip0 =
						ntohl(ip6h->saddr.s6_addr32[0]);
					entry.ipv6_hnapt.new_ipv6_ip1 =
						ntohl(ip6h->saddr.s6_addr32[1]);
					entry.ipv6_hnapt.new_ipv6_ip2 =
						ntohl(ip6h->saddr.s6_addr32[2]);
					entry.ipv6_hnapt.new_ipv6_ip3 =
						ntohl(ip6h->saddr.s6_addr32[3]);
				} else {
					entry.ipv6_hnapt.eg_ipv6_dir =
						IPV6_DNAT;
					entry.ipv6_hnapt.new_ipv6_ip0 =
						ntohl(ip6h->daddr.s6_addr32[0]);
					entry.ipv6_hnapt.new_ipv6_ip1 =
						ntohl(ip6h->daddr.s6_addr32[1]);
					entry.ipv6_hnapt.new_ipv6_ip2 =
						ntohl(ip6h->daddr.s6_addr32[2]);
					entry.ipv6_hnapt.new_ipv6_ip3 =
						ntohl(ip6h->daddr.s6_addr32[3]);
				}

				pptr = skb_header_pointer(skb, IPV6_HDR_LEN + h_offset,
							  sizeof(_ports),
							  &_ports);
				if (unlikely(!pptr))
					return -1;

				entry.ipv6_hnapt.new_sport = ntohs(pptr->src);
				entry.ipv6_hnapt.new_dport = ntohs(pptr->dst);
#else
				return -1;
#endif
			}

			entry.ipv6_5t_route.iblk2.dscp =
				(ip6h->priority << 4 |
				 (ip6h->flow_lbl[0] >> 4));
			break;

		case NEXTHDR_IPIP:
			iph = (struct iphdr *)(skb_inner_network_header(skb) + h_offset);
			/* don't process inner fragment packets */
			if (ip_is_fragment(iph))
				return -1;

			if ((!mape_toggle &&
			     entry.bfib1.pkt_type == IPV4_DSLITE) ||
			    (mape_toggle &&
			     entry.bfib1.pkt_type == IPV4_MAP_E)) {
				/* DS-Lite LAN->WAN */
				entry.bfib1.udp = foe->bfib1.udp;
				entry.ipv4_dslite.sip = foe->ipv4_dslite.sip;
				entry.ipv4_dslite.dip = foe->ipv4_dslite.dip;
				entry.ipv4_dslite.sport =
					foe->ipv4_dslite.sport;
				entry.ipv4_dslite.dport =
					foe->ipv4_dslite.dport;

				entry.ipv4_dslite.tunnel_sipv6_0 =
					ntohl(ip6h->saddr.s6_addr32[0]);
				entry.ipv4_dslite.tunnel_sipv6_1 =
					ntohl(ip6h->saddr.s6_addr32[1]);
				entry.ipv4_dslite.tunnel_sipv6_2 =
					ntohl(ip6h->saddr.s6_addr32[2]);
				entry.ipv4_dslite.tunnel_sipv6_3 =
					ntohl(ip6h->saddr.s6_addr32[3]);

				entry.ipv4_dslite.tunnel_dipv6_0 =
					ntohl(ip6h->daddr.s6_addr32[0]);
				entry.ipv4_dslite.tunnel_dipv6_1 =
					ntohl(ip6h->daddr.s6_addr32[1]);
				entry.ipv4_dslite.tunnel_dipv6_2 =
					ntohl(ip6h->daddr.s6_addr32[2]);
				entry.ipv4_dslite.tunnel_dipv6_3 =
					ntohl(ip6h->daddr.s6_addr32[3]);

				ppe_fill_flow_lbl(&entry, ip6h);

				entry.ipv4_dslite.priority = ip6h->priority;
				entry.ipv4_dslite.hop_limit = ip6h->hop_limit;
				entry.ipv4_dslite.vlan1 = hw_path->vlan_id;
				if (hnat_priv->data->per_flow_accounting)
					entry.ipv4_dslite.iblk2.mibf = 1;
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
				entry.ipv4_dslite.eg_keep_ecn = 1;
				entry.ipv4_dslite.eg_keep_cls = 1;
#endif
				/* Map-E LAN->WAN record inner IPv4 header info. */
#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
				if (mape_toggle) {
					entry.ipv4_dslite.iblk2.dscp = foe->ipv4_dslite.iblk2.dscp;
					entry.ipv4_mape.new_sip = foe->ipv4_mape.new_sip;
					entry.ipv4_mape.new_dip = foe->ipv4_mape.new_dip;
					entry.ipv4_mape.new_sport = foe->ipv4_mape.new_sport;
					entry.ipv4_mape.new_dport = foe->ipv4_mape.new_dport;
				}
#endif
			} else if (mape_toggle &&
				   entry.bfib1.pkt_type == IPV4_HNAPT) {
				/* MapE LAN -> WAN */
				mape = 1;
				entry.ipv4_hnapt.iblk2.dscp =
					foe->ipv4_hnapt.iblk2.dscp;
				if (hnat_priv->data->per_flow_accounting)
					entry.ipv4_hnapt.iblk2.mibf = 1;

				if (IS_GMAC1_MODE)
					entry.ipv4_hnapt.vlan1 = 1;
				else
					entry.ipv4_hnapt.vlan1 = hw_path->vlan_id;

				entry.ipv4_hnapt.sip = foe->ipv4_hnapt.sip;
				entry.ipv4_hnapt.dip = foe->ipv4_hnapt.dip;
				entry.ipv4_hnapt.sport = foe->ipv4_hnapt.sport;
				entry.ipv4_hnapt.dport = foe->ipv4_hnapt.dport;

				entry.ipv4_hnapt.new_sip =
					foe->ipv4_hnapt.new_sip;
				entry.ipv4_hnapt.new_dip =
					foe->ipv4_hnapt.new_dip;
				entry.ipv4_hnapt.sp_tag = htons(ETH_P_IP);

				if (IS_HQOS_MODE) {
					entry.ipv4_hnapt.iblk2.qid =
						(hnat_priv->data->version ==
						 MTK_HNAT_V2 ||
						 hnat_priv->data->version ==
						 MTK_HNAT_V3) ?
						 skb->mark & 0x7f : skb->mark & 0xf;
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
					entry.ipv4_hnapt.tport_id =
						HQOS_FLAG(dev, skb, entry.ipv4_hnapt.iblk2.qid);
#endif
					entry.ipv4_hnapt.iblk2.fqos =
						HQOS_FLAG(dev, skb, entry.ipv4_hnapt.iblk2.qid);
				}

				entry.bfib1.udp =
					foe->bfib1.udp;

				entry.ipv4_hnapt.new_sport =
					foe->ipv4_hnapt.new_sport;
				entry.ipv4_hnapt.new_dport =
					foe->ipv4_hnapt.new_dport;
				mape_l2w_v6h = *ip6h;
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
				entry.ipv4_hnapt.eg_keep_ecn = 1;
				entry.ipv4_hnapt.eg_keep_dscp = 1;
#endif
			}
			break;

		default:
			return -1;
		}
		if (debug_level >= 7)
			trace_printk(
				"[%s]skb->head=%p, skb->data=%p,ipv6_hdr=%p, skb->len=%d, skb->data_len=%d\n",
				__func__, skb->head, skb->data, ip6h, skb->len,
				skb->data_len);
		break;

	default:
		if (IS_L2_BRIDGE(&entry)) {
			entry.l2_bridge.dmac_hi = foe->l2_bridge.dmac_hi;
			entry.l2_bridge.dmac_lo = foe->l2_bridge.dmac_lo;
			entry.l2_bridge.smac_hi = foe->l2_bridge.smac_hi;
			entry.l2_bridge.smac_lo = foe->l2_bridge.smac_lo;
			entry.l2_bridge.etype = foe->l2_bridge.etype;
			entry.l2_bridge.hph = foe->l2_bridge.hph;
			entry.l2_bridge.vlan1 = foe->l2_bridge.vlan1;
			entry.l2_bridge.vlan2 = foe->l2_bridge.vlan2;
			entry.l2_bridge.sp_tag = htons(h_proto);

			if (hnat_priv->data->per_flow_accounting)
				entry.l2_bridge.iblk2.mibf = 1;

			entry.l2_bridge.new_vlan1 = hw_path->vlan_id;
			if (skb_vlan_tagged(skb)) {
				entry.bfib1.vlan_layer += 1;

				if (entry.l2_bridge.new_vlan1)
					entry.l2_bridge.new_vlan2 =
						skb->vlan_tci;
				else
					entry.l2_bridge.new_vlan1 =
						skb->vlan_tci;
			}
			break;
		}
		return -1;
	}

	/* Fill Layer2 Info.*/
	entry = ppe_fill_L2_info(entry, hw_path);

	if ((skb_hnat_tops(skb) && hw_path->flags & BIT(DEV_PATH_TNL)) ||
	    (!skb_hnat_cdrt(skb) && skb_hnat_is_encrypt(skb) &&
	    skb_dst(skb) && dst_xfrm(skb_dst(skb)))) {
		hnat_foe_entry_commit(foe, &entry, entry.udib1.state);
		return 0;
	}

hnat_entry_bind:
	/* Fill Info Blk*/
	entry = ppe_fill_info_blk(entry, hw_path);

	if (IS_LAN_GRP(dev) || IS_WAN(dev)) { /* Forward to GMAC Ports */
		port_id = hnat_dsa_get_port(&master_dev);
		if (port_id >= 0) {
			if (hnat_dsa_fill_stag(dev, &entry, h_proto, mape) < 0)
				return -1;
		}

		mac = netdev_priv(master_dev);
		gmac = HNAT_GMAC_FP(mac->id);

		if (IS_WAN(dev) && mape_toggle && mape == 1) {
			gmac = NR_PDMA_PORT;
			/* Set act_dp = wan_dev */
			entry.ipv4_hnapt.act_dp &= ~UDF_PINGPONG_IFIDX;
			entry.ipv4_hnapt.act_dp |= dev->ifindex & UDF_PINGPONG_IFIDX;
		}
	} else if (IS_EXT(dev) && (FROM_GE_PPD(skb) || FROM_GE_LAN_GRP(skb) ||
		   FROM_GE_WAN(skb) || FROM_GE_VIRTUAL(skb) || FROM_WED(skb))) {
		if (!hnat_priv->data->whnat && IS_GMAC1_MODE) {
			entry.bfib1.vpm = 1;
			entry.bfib1.vlan_layer = 1;

			if (FROM_GE_LAN(skb))
				entry.ipv4_hnapt.vlan1 = 1;
			else if (FROM_GE_WAN(skb) || FROM_GE_VIRTUAL(skb))
				entry.ipv4_hnapt.vlan1 = 2;
		}

		if (debug_level >= 7)
			trace_printk("learn of lan or wan(iif=%x) --> %s(ext)\n",
				     skb_hnat_iface(skb), dev->name);
		/* To CPU then stolen by pre-routing hant hook of LAN/WAN
		 * Current setting is PDMA RX.
		 */
		gmac = NR_PDMA_PORT;
		if (IS_IPV4_GRP(foe) || IS_L2_BRIDGE(foe)) {
			entry.ipv4_hnapt.act_dp &= ~UDF_PINGPONG_IFIDX;
			entry.ipv4_hnapt.act_dp |= dev->ifindex & UDF_PINGPONG_IFIDX;
		} else {
			entry.ipv6_5t_route.act_dp &= ~UDF_PINGPONG_IFIDX;
			entry.ipv6_5t_route.act_dp |= dev->ifindex & UDF_PINGPONG_IFIDX;
		}
	} else {
		gmac = -EINVAL;
	}

	if (gmac < 0) {
		if (debug_level >= 7)
			printk_ratelimited(KERN_WARNING
					   "Unknown case of dp, iif=%x --> %s\n",
					   skb_hnat_iface(skb), dev->name);
		return -1;
	}

	if (IS_HQOS_MODE || (skb->mark & MTK_QDMA_QUEUE_MASK) >= MAX_PPPQ_QUEUE_NUM)
		qid = skb->mark & (MTK_QDMA_QUEUE_MASK);
	else if (IS_PPPQ_MODE && IS_PPPQ_PATH(dev, skb))
		qid = ((port_id >= 0) ? MTK_GMAC_ID_MAX + port_id : mac->id) & MTK_QDMA_QUEUE_MASK;
	else
		qid = 0;

	if (IS_PPPQ_MODE && IS_PPPQ_PATH(dev, skb) && port_id >= 0) {
		if (h_proto == ETH_P_IP) {
			iph = (struct iphdr *)(skb_network_header(skb) + h_offset);
			if (iph->protocol == IPPROTO_TCP) {
				tcph = (struct tcphdr *)((u8 *)iph + iph->ihl * 4);
				payload_len = be16_to_cpu(iph->tot_len) -
					      iph->ihl * 4 - __tcp_hdrlen(tcph);
				/* Dispatch ACK packets to high priority queue */
				if (payload_len == 0)
					qid += MAX_SWITCH_PORT_NUM;
			}
		} else if (h_proto == ETH_P_IPV6) {
			ip6h = (struct ipv6hdr *)(skb_network_header(skb) + h_offset);
			if (ip6h->nexthdr == NEXTHDR_TCP) {
				tcph = (struct tcphdr *)((u8 *)ip6h + sizeof(struct ipv6hdr));
				payload_len = be16_to_cpu(ip6h->payload_len) - __tcp_hdrlen(tcph);
				/* Dispatch ACK packets to high priority queue */
				if (payload_len == 0)
					qid += MAX_SWITCH_PORT_NUM;
			}
		}
	}

	if (IS_IPV4_GRP(&entry)) {
		entry.ipv4_hnapt.iblk2.dp = gmac & 0xf;
#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
		entry.ipv4_hnapt.iblk2.port_mg = 0;
#else
		entry.ipv4_hnapt.iblk2.port_mg =
			(hnat_priv->data->version == MTK_HNAT_V1_1) ? 0x3f : 0;
#endif
		if (qos_toggle) {
			if (hnat_priv->data->version == MTK_HNAT_V2 ||
			    hnat_priv->data->version == MTK_HNAT_V3) {
				entry.ipv4_hnapt.iblk2.qid = qid & 0x7f;
			} else {
				/* qid[5:0]= port_mg[1:0]+ qid[3:0] */
				entry.ipv4_hnapt.iblk2.qid = qid & 0xf;
				if (hnat_priv->data->version != MTK_HNAT_V1_1)
					entry.ipv4_hnapt.iblk2.port_mg |=
						((qid >> 4) & 0x3);

				if (((IS_EXT(dev) && (FROM_GE_LAN_GRP(skb) ||
				      FROM_GE_WAN(skb) || FROM_GE_VIRTUAL(skb))) ||
				      ((mape_toggle && mape == 1) && !FROM_EXT(skb))) &&
				      (!whnat)) {
					entry.ipv4_hnapt.sp_tag = htons(HQOS_MAGIC_TAG);
					entry.ipv4_hnapt.vlan1 = skb_hnat_entry(skb);
					entry.bfib1.vlan_layer = 1;
				}
			}

			if (FROM_EXT(skb) || skb_hnat_sport(skb) == NR_QDMA_PORT)
				entry.ipv4_hnapt.iblk2.fqos = 0;
			else {
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
				entry.ipv4_hnapt.tport_id = HQOS_FLAG(dev, skb, qid) ?
							    NR_QDMA_TPORT : 0;
#endif
				entry.ipv4_hnapt.iblk2.fqos = HQOS_FLAG(dev, skb, qid) ? 1 : 0;
			}
		} else {
			entry.ipv4_hnapt.iblk2.fqos = 0;
		}
	} else if (IS_L2_BRIDGE(&entry)) {
		entry.l2_bridge.iblk2.dp = gmac & 0xf;
		entry.l2_bridge.iblk2.port_mg = 0;
		if (qos_toggle) {
			entry.l2_bridge.iblk2.qid = qid & 0x7f;
			if (FROM_EXT(skb) || skb_hnat_sport(skb) == NR_QDMA_PORT)
				entry.l2_bridge.iblk2.fqos = 0;
			else
				entry.l2_bridge.iblk2.fqos = HQOS_FLAG(dev, skb, qid) ? 1 : 0;
		} else {
			entry.l2_bridge.iblk2.fqos = 0;
		}
	} else {
		entry.ipv6_5t_route.iblk2.dp = gmac & 0xf;
#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
		entry.ipv6_5t_route.iblk2.port_mg = 0;
#else
		entry.ipv6_5t_route.iblk2.port_mg =
			(hnat_priv->data->version == MTK_HNAT_V1_1) ? 0x3f : 0;
#endif

		if (qos_toggle) {
			if (hnat_priv->data->version == MTK_HNAT_V2 ||
			    hnat_priv->data->version == MTK_HNAT_V3) {
				entry.ipv6_5t_route.iblk2.qid = qid & 0x7f;
			} else {
				/* qid[5:0]= port_mg[1:0]+ qid[3:0] */
				entry.ipv6_5t_route.iblk2.qid = qid & 0xf;
				if (hnat_priv->data->version != MTK_HNAT_V1_1)
					entry.ipv6_5t_route.iblk2.port_mg |=
								((qid >> 4) & 0x3);

				if (IS_EXT(dev) && (FROM_GE_LAN_GRP(skb) ||
				    FROM_GE_WAN(skb) || FROM_GE_VIRTUAL(skb)) &&
				    (!whnat)) {
					entry.ipv6_5t_route.sp_tag = htons(HQOS_MAGIC_TAG);
					entry.ipv6_5t_route.vlan1 = skb_hnat_entry(skb);
					entry.bfib1.vlan_layer = 1;
				}
			}

			if (FROM_EXT(skb) || skb_hnat_sport(skb) == NR_QDMA_PORT)
				entry.ipv6_5t_route.iblk2.fqos = 0;
			else {
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
				switch (entry.bfib1.pkt_type) {
				case IPV4_MAP_E:
				case IPV4_MAP_T:
					entry.ipv4_mape.tport_id =
						HQOS_FLAG(dev, skb, qid) ? NR_QDMA_TPORT : 0;
					break;
				case IPV6_HNAPT:
				case IPV6_HNAT:
					entry.ipv6_hnapt.tport_id =
						HQOS_FLAG(dev, skb, qid) ? NR_QDMA_TPORT : 0;
					break;
				default:
					entry.ipv6_5t_route.tport_id =
						HQOS_FLAG(dev, skb, qid) ? NR_QDMA_TPORT : 0;
					break;
				}
#endif
				entry.ipv6_5t_route.iblk2.fqos = HQOS_FLAG(dev, skb, qid) ? 1 : 0;
			}
		} else {
			entry.ipv6_5t_route.iblk2.fqos = 0;
		}
	}

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
	hnat_fill_offload_engine_entry(skb, &entry, dev);

	if (l4s_toggle && FROM_WED(skb) && !whnat) {
		/* For MT7988 TOPS L4S feature, BIND WiFi -> ETH entry to TDMA port */
		if (IS_IPV4_GRP(&entry)) {
			entry.ipv4_hnapt.tops_entry = entry.ipv4_hnapt.iblk2.dp;
			entry.ipv4_hnapt.iblk2.dp = NR_TDMA_PORT;
			/* If the entry BIND to TOPS, don't enter QDMA(HQoS) path */
			entry.ipv4_hnapt.tport_id = 0;
			entry.ipv4_hnapt.iblk2.fqos = 0;
			entry.ipv4_hnapt.iblk2.qid = 0;

		} else {
			if (IS_IPV4_MAPE(&entry) || IS_IPV4_MAPT(&entry)) {
				entry.ipv4_mape.tops_entry = entry.ipv4_mape.iblk2.dp;
				entry.ipv4_mape.tport_id = 0;
			} else if (IS_IPV6_HNAT(&entry) || IS_IPV6_HNAPT(&entry)) {
				entry.ipv6_hnapt.tops_entry = entry.ipv6_hnapt.iblk2.dp;
				entry.ipv6_hnapt.tport_id = 0;
			} else {
				entry.ipv6_5t_route.tops_entry = entry.ipv6_5t_route.iblk2.dp;
				entry.ipv6_5t_route.tport_id = 0;
			}
			entry.ipv6_5t_route.iblk2.dp = NR_TDMA_PORT;
			/* If the entry BIND to TOPS, don't enter QDMA(HQoS) path */
			entry.ipv6_5t_route.iblk2.fqos = 0;
			entry.ipv6_5t_route.iblk2.qid = 0;
		}
	}
#endif

	/* The INFO2.port_mg and 2nd VLAN ID fields of PPE entry are redefined
	 * by Wi-Fi whnat engine. These data and INFO2.dp will be updated and
	 * the entry is set to BIND state in mtk_sw_nat_hook_tx().
	 */
	if (whnat) {
		/* Final check if the entry is not in UNBIND state,
		 * we should not modify it right now.
		 */
		if (unlikely(foe->udib1.state != UNBIND))
			return -1;

		spin_lock_bh(&hnat_priv->flow_entry_lock);

		flow_entry = hnat_flow_entry_search(&entry,
						    skb_hnat_ppe(skb),
						    skb_hnat_entry(skb));
		if (!flow_entry) {
			flow_entry = kmalloc(sizeof(*flow_entry), GFP_KERNEL);
			if (!flow_entry) {
				spin_unlock_bh(&hnat_priv->flow_entry_lock);
				return -1;
			}
			INIT_HLIST_NODE(&flow_entry->list);
			flow_entry->ppe_index = skb_hnat_ppe(skb);
			flow_entry->hash = skb_hnat_entry(skb);
			hlist_add_head(&flow_entry->list,
				&hnat_priv->foe_flow[skb_hnat_ppe(skb)][skb_hnat_entry(skb) / 4]);
		}
		memcpy(&flow_entry->data, &entry, sizeof(entry));
		flow_entry->last_update = jiffies;

		/* We must ensure all info has been updated */
		wmb();
		skb_hnat_filled(skb) = HNAT_INFO_FILLED;

		spin_unlock_bh(&hnat_priv->flow_entry_lock);

		return 0;
	}

	if (!spin_trylock_bh(&hnat_priv->entry_lock))
		return -1;
	/* Final check if the entry is not in UNBIND state,
	 * we should not modify it right now.
	 */
	if (unlikely(foe->udib1.state != UNBIND)) {
		spin_unlock_bh(&hnat_priv->entry_lock);
		return -1;
	}
	hnat_foe_entry_commit(foe, &entry, BIND);
	spin_unlock_bh(&hnat_priv->entry_lock);

	/* reset statistic for this entry */
	if (hnat_priv->data->per_flow_accounting &&
	    skb_hnat_entry(skb) < hnat_priv->foe_etry_num &&
	    skb_hnat_ppe(skb) < CFG_PPE_NUM) {
		memset(&hnat_priv->acct[skb_hnat_ppe(skb)][skb_hnat_entry(skb)],
		       0, sizeof(struct hnat_accounting));
		ct = nf_ct_get(skb, &ctinfo);
		if (ct)
			hnat_priv->acct[skb_hnat_ppe(skb)][skb_hnat_entry(skb)].zone = ct->zone;
	}

	return 0;
}

int mtk_sw_nat_hook_tx(struct sk_buff *skb, int gmac_no)
{
	struct foe_entry *hw_entry, entry;
	struct hnat_flow_entry *flow_entry;
	struct vlan_hdr *vhdr;
	struct ethhdr *eth;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	u16 h_proto, h_offset = 0;

	if (!skb_hnat_is_hashed(skb) || skb_hnat_ppe(skb) >= CFG_PPE_NUM)
		return NF_ACCEPT;

	hw_entry = &hnat_priv->foe_table_cpu[skb_hnat_ppe(skb)][skb_hnat_entry(skb)];

	if (skb_hnat_alg(skb) || !is_hnat_info_filled(skb) ||
	    !is_magic_tag_valid(skb) || !IS_SPACE_AVAILABLE_HEAD(skb))
		return NF_ACCEPT;

	if (debug_level >= 7)
		trace_printk(
			"[%s]entry=%x reason=%x gmac_no=%x wdmaid=%x rxid=%x wcid=%x bssid=%x\n",
			__func__, skb_hnat_entry(skb), skb_hnat_reason(skb), gmac_no,
			skb_hnat_wdma_id(skb), skb_hnat_bss_id(skb),
			skb_hnat_wc_id(skb), skb_hnat_rx_id(skb));

	if ((gmac_no != NR_WDMA0_PORT) && (gmac_no != NR_WDMA1_PORT) &&
	    (gmac_no != NR_WDMA2_PORT) && (gmac_no != NR_WHNAT_WDMA_PORT))
		return NF_ACCEPT;

	if (unlikely(!skb_mac_header_was_set(skb)))
		return NF_ACCEPT;

	if (skb_hnat_reason(skb) != HIT_UNBIND_RATE_REACH)
		return NF_ACCEPT;

	spin_lock_bh(&hnat_priv->flow_entry_lock);
	/* Get the flow_entry prepared in skb_to_hnat_info */
	flow_entry = hnat_flow_entry_search(hw_entry,
					    skb_hnat_ppe(skb),
					    skb_hnat_entry(skb));

	if (unlikely((hw_entry->udib1.state != UNBIND) || (!flow_entry) ||
		     (time_after_eq(jiffies, flow_entry->last_update + 3 * HZ)))) {
		/* If the flow_entry is updated before 3 seconds(UNBIND AGE), delete it */
		if (flow_entry)
			hnat_flow_entry_delete(flow_entry);
		spin_unlock_bh(&hnat_priv->flow_entry_lock);
		return NF_ACCEPT;
	}

	memcpy(&entry, &flow_entry->data, sizeof(entry));
	hnat_flow_entry_delete(flow_entry);
	spin_unlock_bh(&hnat_priv->flow_entry_lock);

	eth = eth_hdr(skb);

	/* not bind multicast if PPE mcast not enable */
	if (!hnat_priv->data->mcast) {
		if (is_multicast_ether_addr(eth->h_dest))
			return NF_ACCEPT;

		if (IS_L2_BRIDGE(&entry))
			entry.l2_bridge.iblk2.mcast = 0;
		else if (IS_IPV4_GRP(&entry))
			entry.ipv4_hnapt.iblk2.mcast = 0;
		else
			entry.ipv6_5t_route.iblk2.mcast = 0;
	}

	/* Some mt_wifi virtual interfaces, such as apcli,
	 * will change the smac for specail purpose.
	 */
	switch ((int)entry.bfib1.pkt_type) {
	case L2_BRIDGE:
	case IPV4_HNAPT:
	case IPV4_HNAT:
		/*
		 * skip if packet is an encap tnl packet or it may
		 * screw up inner mac header
		 */
		if (skb_hnat_tops(skb) && skb_hnat_is_encap(skb))
			break;
		entry.ipv4_hnapt.smac_hi = swab32(*((u32 *)eth->h_source));
		entry.ipv4_hnapt.smac_lo = swab16(*((u16 *)&eth->h_source[4]));
		break;
	case IPV4_DSLITE:
	case IPV4_MAP_E:
	case IPV6_6RD:
	case IPV6_5T_ROUTE:
	case IPV6_3T_ROUTE:
	case IPV6_HNAPT:
	case IPV6_HNAT:
		entry.ipv6_5t_route.smac_hi = swab32(*((u32 *)eth->h_source));
		entry.ipv6_5t_route.smac_lo = swab16(*((u16 *)&eth->h_source[4]));
		break;
	}

	entry.bfib1.vlan_layer = 0;

	if (skb_vlan_tag_present(skb)) {
		if (skb->vlan_proto != htons(ETH_P_8021Q))
			return NF_ACCEPT;

		if (hnat_add_vlan_layer(&entry, skb->vlan_tci, false))
			return NF_ACCEPT;
	}

	h_proto = skb->protocol;
	while (h_proto == htons(ETH_P_8021Q)) {
		vhdr = (struct vlan_hdr *)(skb_mac_header(skb) + ETH_HLEN + h_offset);
		if (hnat_add_vlan_layer(&entry, ntohs(vhdr->h_vlan_TCI), false)) {
			if (debug_level >= 7)
				printk_ratelimited(KERN_WARNING
						   "Unsupported PPE VLAN layer%d in WiFiTx\n",
						   entry.bfib1.vlan_layer + 1);
			return NF_ACCEPT;
		}
		h_proto = vhdr->h_vlan_encapsulated_proto;
		h_offset += VLAN_HLEN;
	}

	if (entry.bfib1.vlan_layer) {
		if (IS_IPV4_GRP(&entry) || IS_L2_BRIDGE(&entry))
			entry.ipv4_hnapt.sp_tag = ETH_P_8021Q;
		else if (IS_IPV6_GRP(&entry))
			entry.ipv6_5t_route.sp_tag = ETH_P_8021Q;
		else
			return NF_ACCEPT;
	}

	/* MT7622 wifi hw_nat not support QoS */
	if (IS_IPV4_GRP(&entry)) {
		entry.ipv4_hnapt.iblk2.fqos = 0;
		if ((hnat_priv->data->version == MTK_HNAT_V1_2 &&
		     gmac_no == NR_WHNAT_WDMA_PORT) ||
		    ((hnat_priv->data->version == MTK_HNAT_V2 ||
		      hnat_priv->data->version == MTK_HNAT_V3) &&
		     (gmac_no == NR_WDMA0_PORT || gmac_no == NR_WDMA1_PORT ||
		      gmac_no == NR_WDMA2_PORT))) {
			entry.ipv4_hnapt.winfo.bssid = skb_hnat_bss_id(skb);
			entry.ipv4_hnapt.winfo.wcid = skb_hnat_wc_id(skb);
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
			entry.ipv4_hnapt.tport_id = IS_HQOS_DL_MODE ? NR_QDMA_TPORT : 0;
			entry.ipv4_hnapt.iblk2.fqos = IS_HQOS_DL_MODE ? 1 : 0;
			entry.ipv4_hnapt.iblk2.rxid = skb_hnat_rx_id(skb);
			entry.ipv4_hnapt.iblk2.winfoi = 1;
			entry.ipv4_hnapt.winfo_pao.usr_info =
				skb_hnat_usr_info(skb);
			entry.ipv4_hnapt.winfo_pao.tid = skb_hnat_tid(skb);
			entry.ipv4_hnapt.winfo_pao.is_fixedrate =
				skb_hnat_is_fixedrate(skb);
			entry.ipv4_hnapt.winfo_pao.is_prior =
				skb_hnat_is_prior(skb);
			entry.ipv4_hnapt.winfo_pao.is_sp = skb_hnat_is_sp(skb);
			entry.ipv4_hnapt.winfo_pao.hf = skb_hnat_hf(skb);
			entry.ipv4_hnapt.winfo_pao.amsdu = skb_hnat_amsdu(skb);
#elif defined(CONFIG_MEDIATEK_NETSYS_V2)
			entry.ipv4_hnapt.iblk2.rxid = skb_hnat_rx_id(skb);
			entry.ipv4_hnapt.iblk2.winfoi = 1;
#else
			entry.ipv4_hnapt.winfo.rxid = skb_hnat_rx_id(skb);
			entry.ipv4_hnapt.iblk2w.winfoi = 1;
			entry.ipv4_hnapt.iblk2w.wdmaid = skb_hnat_wdma_id(skb);
#endif
		} else {
			if (IS_GMAC1_MODE && !hnat_dsa_is_enable(hnat_priv)) {
				entry.bfib1.vpm = 1;
				entry.bfib1.vlan_layer = 1;

				if (FROM_GE_LAN_GRP(skb))
					entry.ipv4_hnapt.vlan1 = 1;
				else if (FROM_GE_WAN(skb) || FROM_GE_VIRTUAL(skb))
					entry.ipv4_hnapt.vlan1 = 2;
			}

			if (IS_HQOS_MODE &&
			    (FROM_GE_LAN_GRP(skb) || FROM_GE_WAN(skb) || FROM_GE_VIRTUAL(skb))) {
				entry.bfib1.vpm = 0;
				entry.bfib1.vlan_layer = 1;
				entry.ipv4_hnapt.sp_tag = htons(HQOS_MAGIC_TAG);
				entry.ipv4_hnapt.vlan1 = skb_hnat_entry(skb);
				entry.ipv4_hnapt.iblk2.fqos = 1;
			}
		}
		entry.ipv4_hnapt.iblk2.dp = gmac_no & 0xf;
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
	} else if (IS_IPV6_HNAPT(&entry) || IS_IPV6_HNAT(&entry)) {
		entry.ipv6_hnapt.iblk2.dp = gmac_no & 0xf;
		entry.ipv6_hnapt.iblk2.rxid = skb_hnat_rx_id(skb);
		entry.ipv6_hnapt.iblk2.winfoi = 1;

		entry.ipv6_hnapt.winfo.bssid = skb_hnat_bss_id(skb);
		entry.ipv6_hnapt.winfo.wcid = skb_hnat_wc_id(skb);
		entry.ipv6_hnapt.winfo_pao.usr_info = skb_hnat_usr_info(skb);
		entry.ipv6_hnapt.winfo_pao.tid = skb_hnat_tid(skb);
		entry.ipv6_hnapt.winfo_pao.is_fixedrate =
			skb_hnat_is_fixedrate(skb);
		entry.ipv6_hnapt.winfo_pao.is_prior = skb_hnat_is_prior(skb);
		entry.ipv6_hnapt.winfo_pao.is_sp = skb_hnat_is_sp(skb);
		entry.ipv6_hnapt.winfo_pao.hf = skb_hnat_hf(skb);
		entry.ipv6_hnapt.winfo_pao.amsdu = skb_hnat_amsdu(skb);
		entry.ipv6_hnapt.tport_id = IS_HQOS_DL_MODE ? NR_QDMA_TPORT : 0;
		entry.ipv6_hnapt.iblk2.fqos = IS_HQOS_DL_MODE ? 1 : 0;
	} else if (IS_L2_BRIDGE(&entry)) {
		entry.l2_bridge.iblk2.dp = gmac_no & 0xf;
		entry.l2_bridge.iblk2.rxid = skb_hnat_rx_id(skb);
		entry.l2_bridge.iblk2.winfoi = 1;

		entry.l2_bridge.winfo.bssid = skb_hnat_bss_id(skb);
		entry.l2_bridge.winfo.wcid = skb_hnat_wc_id(skb);
		entry.l2_bridge.winfo_pao.usr_info =
			skb_hnat_usr_info(skb);
		entry.l2_bridge.winfo_pao.tid = skb_hnat_tid(skb);
		entry.l2_bridge.winfo_pao.is_fixedrate =
			skb_hnat_is_fixedrate(skb);
		entry.l2_bridge.winfo_pao.is_prior = skb_hnat_is_prior(skb);
		entry.l2_bridge.winfo_pao.is_sp = skb_hnat_is_sp(skb);
		entry.l2_bridge.winfo_pao.hf = skb_hnat_hf(skb);
		entry.l2_bridge.winfo_pao.amsdu = skb_hnat_amsdu(skb);
		entry.l2_bridge.iblk2.fqos = IS_HQOS_DL_MODE ? 1 : 0;
#endif
	} else {
		entry.ipv6_5t_route.iblk2.fqos = 0;
		if ((hnat_priv->data->version == MTK_HNAT_V1_2 &&
		     gmac_no == NR_WHNAT_WDMA_PORT) ||
		    ((hnat_priv->data->version == MTK_HNAT_V2 ||
		      hnat_priv->data->version == MTK_HNAT_V3) &&
		     (gmac_no == NR_WDMA0_PORT || gmac_no == NR_WDMA1_PORT ||
		      gmac_no == NR_WDMA2_PORT))) {
#if defined(CONFIG_MEDIATEK_NETSYS_V3)
			switch (entry.bfib1.pkt_type) {
			case IPV4_MAP_E:
			case IPV4_MAP_T:
				entry.ipv4_mape.winfo.bssid = skb_hnat_bss_id(skb);
				entry.ipv4_mape.winfo.wcid = skb_hnat_wc_id(skb);
				entry.ipv4_mape.tport_id = IS_HQOS_DL_MODE ? NR_QDMA_TPORT : 0;
				entry.ipv4_mape.iblk2.fqos = IS_HQOS_DL_MODE ? 1 : 0;
				entry.ipv4_mape.iblk2.rxid = skb_hnat_rx_id(skb);
				entry.ipv4_mape.iblk2.winfoi = 1;
				entry.ipv4_mape.winfo_pao.usr_info =
					skb_hnat_usr_info(skb);
				entry.ipv4_mape.winfo_pao.tid =
					skb_hnat_tid(skb);
				entry.ipv4_mape.winfo_pao.is_fixedrate =
					skb_hnat_is_fixedrate(skb);
				entry.ipv4_mape.winfo_pao.is_prior =
					skb_hnat_is_prior(skb);
				entry.ipv4_mape.winfo_pao.is_sp =
					skb_hnat_is_sp(skb);
				entry.ipv4_mape.winfo_pao.hf =
					skb_hnat_hf(skb);
				entry.ipv4_mape.winfo_pao.amsdu =
					skb_hnat_amsdu(skb);
				break;
			default:
				entry.ipv6_5t_route.winfo.bssid = skb_hnat_bss_id(skb);
				entry.ipv6_5t_route.winfo.wcid = skb_hnat_wc_id(skb);
				entry.ipv6_5t_route.tport_id = IS_HQOS_DL_MODE ? NR_QDMA_TPORT : 0;
				entry.ipv6_5t_route.iblk2.fqos = IS_HQOS_DL_MODE ? 1 : 0;
				entry.ipv6_5t_route.iblk2.rxid = skb_hnat_rx_id(skb);
				entry.ipv6_5t_route.iblk2.winfoi = 1;
				entry.ipv6_5t_route.winfo_pao.usr_info =
					skb_hnat_usr_info(skb);
				entry.ipv6_5t_route.winfo_pao.tid =
					skb_hnat_tid(skb);
				entry.ipv6_5t_route.winfo_pao.is_fixedrate =
					skb_hnat_is_fixedrate(skb);
				entry.ipv6_5t_route.winfo_pao.is_prior =
					skb_hnat_is_prior(skb);
				entry.ipv6_5t_route.winfo_pao.is_sp =
					skb_hnat_is_sp(skb);
				entry.ipv6_5t_route.winfo_pao.hf =
					skb_hnat_hf(skb);
				entry.ipv6_5t_route.winfo_pao.amsdu =
					skb_hnat_amsdu(skb);
				break;
			}
#elif defined(CONFIG_MEDIATEK_NETSYS_V2)
			entry.ipv6_5t_route.winfo.bssid = skb_hnat_bss_id(skb);
			entry.ipv6_5t_route.winfo.wcid = skb_hnat_wc_id(skb);
			entry.ipv6_5t_route.iblk2.rxid = skb_hnat_rx_id(skb);
			entry.ipv6_5t_route.iblk2.winfoi = 1;
#else
			entry.ipv6_5t_route.winfo.bssid = skb_hnat_bss_id(skb);
			entry.ipv6_5t_route.winfo.wcid = skb_hnat_wc_id(skb);
			entry.ipv6_5t_route.winfo.rxid = skb_hnat_rx_id(skb);
			entry.ipv6_5t_route.iblk2w.winfoi = 1;
			entry.ipv6_5t_route.iblk2w.wdmaid = skb_hnat_wdma_id(skb);
#endif
		} else {
			if (IS_GMAC1_MODE && !hnat_dsa_is_enable(hnat_priv)) {
				entry.bfib1.vpm = 1;
				entry.bfib1.vlan_layer = 1;

				if (FROM_GE_LAN_GRP(skb))
					entry.ipv6_5t_route.vlan1 = 1;
				else if (FROM_GE_WAN(skb) || FROM_GE_VIRTUAL(skb))
					entry.ipv6_5t_route.vlan1 = 2;
			}

			if (IS_HQOS_MODE &&
			    (FROM_GE_LAN_GRP(skb) || FROM_GE_WAN(skb) || FROM_GE_VIRTUAL(skb))) {
				entry.bfib1.vpm = 0;
				entry.bfib1.vlan_layer = 1;
				entry.ipv6_5t_route.sp_tag = htons(HQOS_MAGIC_TAG);
				entry.ipv6_5t_route.vlan1 = skb_hnat_entry(skb);
				entry.ipv6_5t_route.iblk2.fqos = 1;
			}
		}
		entry.ipv6_5t_route.iblk2.dp = gmac_no & 0xf;
	}

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
	hnat_fill_offload_engine_entry(skb, &entry, NULL);
#endif

	spin_lock_bh(&hnat_priv->entry_lock);
	/* Final check if the entry is not in UNBIND state,
	 * we should not modify it right now.
	 */
	if (unlikely(hw_entry->udib1.state != UNBIND)) {
		spin_unlock_bh(&hnat_priv->entry_lock);
		return NF_ACCEPT;
	}
	hnat_foe_entry_commit(hw_entry, &entry, BIND);
	spin_unlock_bh(&hnat_priv->entry_lock);

	/* reset statistic for this entry */
	if (hnat_priv->data->per_flow_accounting) {
		memset(&hnat_priv->acct[skb_hnat_ppe(skb)][skb_hnat_entry(skb)],
			0, sizeof(struct hnat_accounting));
		ct = nf_ct_get(skb, &ctinfo);
		if (ct)
			hnat_priv->acct[skb_hnat_ppe(skb)][skb_hnat_entry(skb)].zone = ct->zone;
	}

#if defined(CONFIG_MEDIATEK_NETSYS_V3)
	if (debug_level >= 7) {
		pr_info("%s %d dp:%d rxid:%d tid:%d usr_info:%d bssid:%d wcid:%d hsh-idx:%d sp:%d\n",
			__func__, __LINE__,
			gmac_no, skb_hnat_rx_id(skb), skb_hnat_tid(skb),
			skb_hnat_usr_info(skb), skb_hnat_bss_id(skb),
			skb_hnat_wc_id(skb), skb_hnat_entry(skb),
			skb_hnat_sport(skb));

		if (IS_IPV4_GRP(&entry)) {
			pr_info("%s %d dp:%d rxid:%d tid:%d uinfo:%d bssid:%d wcid:%d hsh-idx:%d sp:%d\n",
				__func__, __LINE__,
				(hw_entry->ipv4_hnapt.iblk2.dp),
				(hw_entry->ipv4_hnapt.iblk2.rxid),
				(hw_entry->ipv4_hnapt.winfo_pao.tid),
				(hw_entry->ipv4_hnapt.winfo_pao.usr_info),
				(hw_entry->ipv4_hnapt.winfo.bssid),
				(hw_entry->ipv4_hnapt.winfo.wcid),
				skb_hnat_entry(skb), skb_hnat_sport(skb));
			pr_info("%s %d dip:%x sip:%x dp:%x sp:%x hsh-idx:%d\n",
				__func__, __LINE__,
				hw_entry->ipv4_hnapt.dip, hw_entry->ipv4_hnapt.sip,
				hw_entry->ipv4_hnapt.dport, hw_entry->ipv4_hnapt.sport,
				skb_hnat_entry(skb));
			pr_info("%s %d new_dip:%x new_sip:%x new_dp:%x new_sp:%x hsh-idx:%d\n",
				__func__, __LINE__,
				hw_entry->ipv4_hnapt.new_dip, hw_entry->ipv4_hnapt.new_sip,
				hw_entry->ipv4_hnapt.new_dport,
				hw_entry->ipv4_hnapt.new_sport, skb_hnat_entry(skb));
		} else {
			pr_info("%s %d dp:%d rxid:%d tid:%d uinfo:%d bssid:%d wcid:%d hidx:%d sp:%d\n",
				__func__, __LINE__,
				(hw_entry->ipv6_5t_route.iblk2.dp),
				(hw_entry->ipv6_5t_route.iblk2.rxid),
				(hw_entry->ipv6_5t_route.winfo_pao.tid),
				(hw_entry->ipv6_5t_route.winfo_pao.usr_info),
				(hw_entry->ipv6_5t_route.winfo.bssid),
				(hw_entry->ipv6_5t_route.winfo.wcid),
				skb_hnat_entry(skb), skb_hnat_sport(skb));
			pr_info("sip:%x-:%x-:%x-:%x dip0:%x-:%x-:%x-:%x dport:%x sport:%x\n",
				hw_entry->ipv6_5t_route.ipv6_sip0,
				hw_entry->ipv6_5t_route.ipv6_sip1,
				hw_entry->ipv6_5t_route.ipv6_sip2,
				hw_entry->ipv6_5t_route.ipv6_sip3,
				hw_entry->ipv6_5t_route.ipv6_dip0,
				hw_entry->ipv6_5t_route.ipv6_dip1,
				hw_entry->ipv6_5t_route.ipv6_dip2,
				hw_entry->ipv6_5t_route.ipv6_dip3,
				hw_entry->ipv6_5t_route.dport,
				hw_entry->ipv6_5t_route.sport);
		}
	}
#endif
	return NF_ACCEPT;
}

int mtk_sw_nat_hook_rx(struct sk_buff *skb)
{
	if (!IS_SPACE_AVAILABLE_HEAD(skb) || !FROM_WED(skb)) {
		skb_hnat_magic_tag(skb) = 0;
		return NF_ACCEPT;
	}

	skb_hnat_alg(skb) = 0;
	skb_hnat_filled(skb) = 0;
	skb_hnat_set_tops(skb, 0);
	skb_hnat_set_cdrt(skb, 0);
	skb_hnat_set_is_decrypt(skb, 0);
	skb_hnat_magic_tag(skb) = HNAT_MAGIC_TAG;

	if (skb_hnat_iface(skb) == FOE_MAGIC_WED0)
		skb_hnat_sport(skb) = NR_WDMA0_PORT;
	else if (skb_hnat_iface(skb) == FOE_MAGIC_WED1)
		skb_hnat_sport(skb) = NR_WDMA1_PORT;
	else if (skb_hnat_iface(skb) == FOE_MAGIC_WED2)
		skb_hnat_sport(skb) = NR_WDMA2_PORT;

	return NF_ACCEPT;
}

void mtk_ppe_dev_register_hook(struct net_device *dev)
{
	int i, number = 0;
	struct extdev_entry *ext_entry;

	for (i = 1; i < MAX_IF_NUM; i++) {
		if (hnat_priv->wifi_hook_if[i] == dev) {
			pr_info("%s : %s has been registered in wifi_hook_if table[%d]\n",
				__func__, dev->name, i);
			return;
		}
	}

	for (i = 1; i < MAX_IF_NUM; i++) {
		if (!hnat_priv->wifi_hook_if[i]) {
			if (find_extif_from_devname(dev->name)) {
				extif_set_dev(dev);
				goto add_wifi_hook_if;
			}

			number = get_ext_device_number();
			if (number >= MAX_EXT_DEVS) {
				pr_info("%s : extdev array is full. %s is not registered\n",
					__func__, dev->name);
				return;
			}

			ext_entry = kzalloc(sizeof(*ext_entry), GFP_KERNEL);
			if (!ext_entry)
				return;

			strncpy(ext_entry->name, dev->name, IFNAMSIZ - 1);
			dev_hold(dev);
			ext_entry->dev = dev;
			ext_if_add(ext_entry);

add_wifi_hook_if:
			dev_hold(dev);
			hnat_priv->wifi_hook_if[i] = dev;

			break;
		}
	}
	pr_info("%s : ineterface %s register (%d)\n", __func__, dev->name, i);
}

void mtk_ppe_dev_unregister_hook(struct net_device *dev)
{
	int i;

	for (i = 1; i < MAX_IF_NUM; i++) {
		if (hnat_priv->wifi_hook_if[i] == dev) {
			hnat_priv->wifi_hook_if[i] = NULL;
			dev_put(dev);

			break;
		}
	}

	extif_put_dev(dev);
	pr_info("%s : ineterface %s set null (%d)\n", __func__, dev->name, i);
}

static unsigned int mtk_hnat_accel_type(struct sk_buff *skb)
{
	struct dst_entry *dst;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	const struct nf_conn_help *help;

	/* Do not accelerate 1st round of xfrm flow, and 2nd round of xfrm flow
	 * is from local_out which is also filtered in sanity check.
	 */
	dst = skb_dst(skb);
	if (dst && dst_xfrm(dst) &&
	    (!mtk_crypto_offloadable || !mtk_crypto_offloadable(skb)))
		return 0;

	ct = nf_ct_get(skb, &ctinfo);
	if (!ct)
		return 1;

	/* rcu_read_lock()ed by nf_hook_slow */
	help = nfct_help(ct);
	if (help && rcu_dereference(help->helper))
		return 0;

	return 1;
}

static void mtk_hnat_dscp_update(struct sk_buff *skb, struct foe_entry *entry)
{
	struct iphdr *iph;
	struct ethhdr *eth;
	struct ipv6hdr *ip6h;
	bool flag = false;

	eth = eth_hdr(skb);
	switch (ntohs(eth->h_proto)) {
	case ETH_P_IP:
		iph = ip_hdr(skb);
		if (IS_IPV4_GRP(entry) && entry->ipv4_hnapt.iblk2.dscp != iph->tos) {
			entry->ipv4_hnapt.iblk2.dscp = iph->tos;
			flag = true;
		}
		break;
	case ETH_P_IPV6:
		ip6h = ipv6_hdr(skb);
		if ((IS_IPV6_3T_ROUTE(entry) || IS_IPV6_5T_ROUTE(entry)) &&
			(entry->ipv6_5t_route.iblk2.dscp !=
			(ip6h->priority << 4 | (ip6h->flow_lbl[0] >> 4)))) {
			entry->ipv6_5t_route.iblk2.dscp =
				(ip6h->priority << 4 | (ip6h->flow_lbl[0] >> 4));
			flag = true;
		}
		break;
	default:
		return;
	}

	if (flag) {
		if (debug_level >= 7)
			pr_info("%s %d update entry idx=%d\n", __func__, __LINE__,
			skb_hnat_entry(skb));
		/* clear HWNAT cache */
		hnat_cache_clr(skb_hnat_ppe(skb));
	}
}

static int mtk_464xlat_fill_mac(struct foe_entry *entry, struct sk_buff *skb,
				const struct net_device *out, bool l2w)
{
	const struct in6_addr *ipv6_nexthop;
	struct dst_entry *dst = skb_dst(skb);
	struct neighbour *neigh = NULL;
	struct rtable *rt = (struct rtable *)dst;
	u32 nexthop;

	rcu_read_lock_bh();
	if (l2w) {
		ipv6_nexthop = rt6_nexthop((struct rt6_info *)dst,
					   &ipv6_hdr(skb)->daddr);
		neigh = __ipv6_neigh_lookup_noref(dst->dev, ipv6_nexthop);
		if (unlikely(!neigh)) {
			dev_notice(hnat_priv->dev, "%s:No neigh (daddr=%pI6)\n",
				   __func__, &ipv6_hdr(skb)->daddr);
			rcu_read_unlock_bh();
			return -1;
		}
	} else {
		nexthop = (__force u32)rt_nexthop(rt, ip_hdr(skb)->daddr);
		neigh = __ipv4_neigh_lookup_noref(dst->dev, nexthop);
		if (unlikely(!neigh)) {
			dev_notice(hnat_priv->dev, "%s:No neigh (daddr=%pI4)\n",
				   __func__, &ip_hdr(skb)->daddr);
			rcu_read_unlock_bh();
			return -1;
		}
	}
	rcu_read_unlock_bh();

	entry->ipv4_dslite.dmac_hi = swab32(*((u32 *)neigh->ha));
	entry->ipv4_dslite.dmac_lo = swab16(*((u16 *)&neigh->ha[4]));
	entry->ipv4_dslite.smac_hi = swab32(*((u32 *)out->dev_addr));
	entry->ipv4_dslite.smac_lo = swab16(*((u16 *)&out->dev_addr[4]));

	return 0;
}

static int mtk_464xlat_get_hash(struct sk_buff *skb, u32 *hash, bool l2w)
{
	struct in6_addr addr_v6, prefix;
	struct ipv6hdr *ip6h;
	struct iphdr *iph;
	struct tcpudphdr *pptr, _ports;
	struct foe_entry tmp;
	u32 addr, protoff;

	if (l2w) {
		ip6h = ipv6_hdr(skb);
		if (mtk_ppe_get_xlat_v4_by_v6(&ip6h->daddr, &addr))
			return -1;
		protoff = IPV6_HDR_LEN;

		tmp.bfib1.pkt_type = IPV4_HNAPT;
		tmp.ipv4_hnapt.sip = ntohl(ip6h->saddr.s6_addr32[3]);
		tmp.ipv4_hnapt.dip = ntohl(addr);
	} else {
		iph = ip_hdr(skb);
		if (mtk_ppe_get_xlat_v6_by_v4(&iph->saddr, &addr_v6, &prefix))
			return -1;

		protoff = iph->ihl * 4;

		tmp.bfib1.pkt_type = IPV6_5T_ROUTE;
		tmp.ipv6_5t_route.ipv6_sip0 = ntohl(addr_v6.s6_addr32[0]);
		tmp.ipv6_5t_route.ipv6_sip1 = ntohl(addr_v6.s6_addr32[1]);
		tmp.ipv6_5t_route.ipv6_sip2 = ntohl(addr_v6.s6_addr32[2]);
		tmp.ipv6_5t_route.ipv6_sip3 = ntohl(addr_v6.s6_addr32[3]);
		tmp.ipv6_5t_route.ipv6_dip0 = ntohl(prefix.s6_addr32[0]);
		tmp.ipv6_5t_route.ipv6_dip1 = ntohl(prefix.s6_addr32[1]);
		tmp.ipv6_5t_route.ipv6_dip2 = ntohl(prefix.s6_addr32[2]);
		tmp.ipv6_5t_route.ipv6_dip3 = ntohl(iph->daddr);
	}

	pptr = skb_header_pointer(skb, protoff,
				  sizeof(_ports), &_ports);
	if (unlikely(!pptr))
		return -1;

	if (l2w) {
		tmp.ipv4_hnapt.sport = ntohs(pptr->src);
		tmp.ipv4_hnapt.dport = ntohs(pptr->dst);
	} else {
		tmp.ipv6_5t_route.sport = ntohs(pptr->src);
		tmp.ipv6_5t_route.dport = ntohs(pptr->dst);
	}

	*hash = hnat_get_ppe_hash(&tmp);

	return 0;
}

static void mtk_464xlat_fill_info1(struct foe_entry *entry,
				   struct sk_buff *skb, bool l2w)
{
	entry->bfib1.cah = 1;
	entry->bfib1.ttl = 1;
	entry->bfib1.state = BIND;
	entry->bfib1.time_stamp = readl(hnat_priv->fe_base + 0x0010) & (0xFF);
	if (l2w) {
		entry->bfib1.pkt_type = IPV4_DSLITE;
		entry->bfib1.udp = ipv6_hdr(skb)->nexthdr ==
				   IPPROTO_UDP ? 1 : 0;
	} else {
		entry->bfib1.pkt_type = IPV6_6RD;
		entry->bfib1.udp = ip_hdr(skb)->protocol ==
				   IPPROTO_UDP ? 1 : 0;
	}
}

static void mtk_464xlat_fill_info2(struct foe_entry *entry, bool l2w)
{
	entry->ipv4_dslite.iblk2.mibf = 1;
	entry->ipv4_dslite.iblk2.port_ag = 0xF;

	if (l2w)
		entry->ipv4_dslite.iblk2.dp = NR_GMAC2_PORT;
	else
		entry->ipv6_6rd.iblk2.dp = NR_GMAC1_PORT;
}

static void mtk_464xlat_fill_ipv4(struct foe_entry *entry, struct sk_buff *skb,
				  struct foe_entry *foe, bool l2w)
{
	struct iphdr *iph;

	if (l2w) {
		entry->ipv4_dslite.sip = foe->ipv4_dslite.sip;
		entry->ipv4_dslite.dip = foe->ipv4_dslite.dip;
		entry->ipv4_dslite.sport = foe->ipv4_dslite.sport;
		entry->ipv4_dslite.dport = foe->ipv4_dslite.dport;
	} else {
		iph = ip_hdr(skb);
		entry->ipv6_6rd.tunnel_sipv4 = ntohl(iph->saddr);
		entry->ipv6_6rd.tunnel_dipv4 = ntohl(iph->daddr);
		entry->ipv6_6rd.sport = foe->ipv6_6rd.sport;
		entry->ipv6_6rd.dport = foe->ipv6_6rd.dport;
		entry->ipv6_6rd.hdr_chksum = ppe_get_chkbase(iph);
		entry->ipv6_6rd.ttl = iph->ttl;
		entry->ipv6_6rd.dscp = iph->tos;
		entry->ipv6_6rd.flag = (ntohs(iph->frag_off) >> 13);
	}
}

static int mtk_464xlat_fill_ipv6(struct foe_entry *entry, struct sk_buff *skb,
				 struct foe_entry *foe, bool l2w)
{
	struct ipv6hdr *ip6h;
	struct in6_addr addr_v6, prefix;
	u32 addr;

	if (l2w) {
		ip6h = ipv6_hdr(skb);

		if (mtk_ppe_get_xlat_v4_by_v6(&ip6h->daddr, &addr))
			return -1;

		if (mtk_ppe_get_xlat_v6_by_v4(&addr, &addr_v6, &prefix))
			return -1;

		entry->ipv4_dslite.tunnel_sipv6_0 =
			ntohl(prefix.s6_addr32[0]);
		entry->ipv4_dslite.tunnel_sipv6_1 =
			ntohl(ip6h->saddr.s6_addr32[1]);
		entry->ipv4_dslite.tunnel_sipv6_2 =
			ntohl(ip6h->saddr.s6_addr32[2]);
		entry->ipv4_dslite.tunnel_sipv6_3 =
			ntohl(ip6h->saddr.s6_addr32[3]);
		entry->ipv4_dslite.tunnel_dipv6_0 =
			ntohl(ip6h->daddr.s6_addr32[0]);
		entry->ipv4_dslite.tunnel_dipv6_1 =
			ntohl(ip6h->daddr.s6_addr32[1]);
		entry->ipv4_dslite.tunnel_dipv6_2 =
			ntohl(ip6h->daddr.s6_addr32[2]);
		entry->ipv4_dslite.tunnel_dipv6_3 =
			ntohl(ip6h->daddr.s6_addr32[3]);

		ppe_fill_flow_lbl(entry, ip6h);
		entry->ipv4_dslite.priority = ip6h->priority;
		entry->ipv4_dslite.hop_limit = ip6h->hop_limit;

	} else {
		entry->ipv6_6rd.ipv6_sip0 = foe->ipv6_6rd.ipv6_sip0;
		entry->ipv6_6rd.ipv6_sip1 = foe->ipv6_6rd.ipv6_sip1;
		entry->ipv6_6rd.ipv6_sip2 = foe->ipv6_6rd.ipv6_sip2;
		entry->ipv6_6rd.ipv6_sip3 = foe->ipv6_6rd.ipv6_sip3;
		entry->ipv6_6rd.ipv6_dip0 = foe->ipv6_6rd.ipv6_dip0;
		entry->ipv6_6rd.ipv6_dip1 = foe->ipv6_6rd.ipv6_dip1;
		entry->ipv6_6rd.ipv6_dip2 = foe->ipv6_6rd.ipv6_dip2;
		entry->ipv6_6rd.ipv6_dip3 = foe->ipv6_6rd.ipv6_dip3;
	}

	return 0;
}

static int mtk_464xlat_fill_l2(struct foe_entry *entry, struct sk_buff *skb,
			       const struct net_device *dev, bool l2w)
{
	const unsigned int *port_reg;
	int port_index;
	u16 sp_tag;

	if (l2w)
		entry->ipv4_dslite.sp_tag = ETH_P_IP;
	else {
		if (IS_DSA_LAN(dev)) {
			port_reg = of_get_property(dev->dev.of_node,
						   "reg", NULL);
			if (unlikely(!port_reg))
				return -1;

			port_index = be32_to_cpup(port_reg);
			sp_tag = BIT(port_index);

			entry->bfib1.vlan_layer = 1;
			entry->bfib1.vpm = 0;
			entry->ipv6_6rd.sp_tag = sp_tag;
		} else
			entry->ipv6_6rd.sp_tag = ETH_P_IPV6;
	}

	if (mtk_464xlat_fill_mac(entry, skb, dev, l2w))
		return -1;

	return 0;
}


static int mtk_464xlat_fill_l3(struct foe_entry *entry, struct sk_buff *skb,
			       struct foe_entry *foe, bool l2w)
{
	mtk_464xlat_fill_ipv4(entry, skb, foe, l2w);

	if (mtk_464xlat_fill_ipv6(entry, skb, foe, l2w))
		return -1;

	return 0;
}

static int mtk_464xlat_post_process(struct sk_buff *skb, const struct net_device *out)
{
	struct foe_entry *foe, entry = {};
	u32 hash;
	bool l2w;

	if (skb->protocol == htons(ETH_P_IPV6))
		l2w = true;
	else if (skb->protocol == htons(ETH_P_IP))
		l2w = false;
	else
		return -1;

	if (mtk_464xlat_get_hash(skb, &hash, l2w))
		return -1;

	if (hash >= hnat_priv->foe_etry_num)
		return -1;

	if (headroom[hash].crsn != HIT_UNBIND_RATE_REACH)
		return -1;

	foe = &hnat_priv->foe_table_cpu[headroom_ppe(headroom[hash])][hash];

	mtk_464xlat_fill_info1(&entry, skb, l2w);

	if (mtk_464xlat_fill_l3(&entry, skb, foe, l2w))
		return -1;

	mtk_464xlat_fill_info2(&entry, l2w);

	if (mtk_464xlat_fill_l2(&entry, skb, out, l2w))
		return -1;

	/* We must ensure all info has been updated before set to hw */
	wmb();
	memcpy(foe, &entry, sizeof(struct foe_entry));

	return 0;
}

static unsigned int mtk_hnat_nf_post_routing(
	struct sk_buff *skb, const struct net_device *out,
	int (*fn)(struct sk_buff *, const struct net_device *,
		  struct flow_offload_hw_path *),
	const char *func)
{
	struct ethhdr eth = {0};
	struct foe_entry *entry;
	struct flow_offload_hw_path hw_path = { .virt_dev = (struct net_device *)out };
	const struct net_device *arp_dev = out;
	bool is_virt_dev = false;

	if (xlat_toggle && !mtk_464xlat_post_process(skb, out))
		return 0;

	if (skb_hnat_alg(skb) || unlikely(!is_magic_tag_valid(skb) ||
					  !IS_SPACE_AVAILABLE_HEAD(skb)))
		return 0;

	if (unlikely(!skb_mac_header_was_set(skb)))
		return 0;

	if (unlikely(!skb_hnat_is_hashed(skb)))
		return 0;

	if (unlikely(skb->mark == HNAT_EXCEPTION_TAG))
		return 0;

	/* Get bond device slave for the following binding flow */
	if (netif_is_bond_master(out) && out->netdev_ops->ndo_get_xmit_slave) {
		out = out->netdev_ops->ndo_get_xmit_slave((struct net_device *)out, skb, false);
		if (!out)
			return 0;
	}

	hw_path.dev = (struct net_device *)out;

	if (out->netdev_ops->ndo_flow_offload_check) {
		out->netdev_ops->ndo_flow_offload_check(&hw_path);

		out = (IS_GMAC1_MODE) ? hw_path.virt_dev : hw_path.dev;
		if (hw_path.flags & BIT(DEV_PATH_TNL) && mtk_tnl_encap_offload) {
			if (ntohs(skb->protocol) == ETH_P_IP &&
			    (ip_hdr(skb)->protocol == IPPROTO_TCP ||
			     ip_hdr(skb)->protocol == IPPROTO_UDP))
				skb_hnat_set_tops(skb, hw_path.tnl_type + 1);
			else {
				skb_hnat_alg(skb) = 1;
				return 0;
			}
		}
	}

	if (!IS_LAN_GRP(out) && !IS_WAN(out) && !IS_EXT(out))
		is_virt_dev = true;

	if (is_virt_dev
	    && !(skb_hnat_tops(skb) && skb_hnat_is_encap(skb)
		 && (hw_path.flags & BIT(DEV_PATH_TNL))))
		return 0;

	if (debug_level >= 7)
		trace_printk("[%s] case hit, %x-->%s, reason=%x\n", __func__,
			     skb_hnat_iface(skb), out->name, skb_hnat_reason(skb));

	if (skb_hnat_entry(skb) >= hnat_priv->foe_etry_num ||
	    skb_hnat_ppe(skb) >= CFG_PPE_NUM)
		return -1;

	entry = &hnat_priv->foe_table_cpu[skb_hnat_ppe(skb)][skb_hnat_entry(skb)];

	switch (skb_hnat_reason(skb)) {
	case HIT_UNBIND_RATE_REACH:
		if (entry_hnat_is_bound(entry))
			break;

		if (fn && !mtk_hnat_accel_type(skb))
			break;

		if (!fn) {
			memcpy(hw_path.eth_dest, eth_hdr(skb)->h_dest, ETH_ALEN);
			memcpy(hw_path.eth_src, eth_hdr(skb)->h_source, ETH_ALEN);
		} else {
			if (is_virt_dev && (hw_path.flags & BIT(DEV_PATH_TNL))) {
				memset(hw_path.eth_dest, 0, ETH_ALEN);
				memset(hw_path.eth_src, 0, ETH_ALEN);
			} else if (fn(skb, arp_dev, &hw_path))
				break;
		}
		/* skb_hnat_tops(skb) is updated in mtk_tnl_offload() */
		if (skb_hnat_tops(skb)) {
			memcpy(eth.h_dest, hw_path.eth_dest, ETH_ALEN);
			memcpy(eth.h_source, hw_path.eth_src, ETH_ALEN);
			if (ip_hdr(skb)->version == IPVERSION_V4)
				eth.h_proto = htons(ETH_P_IP);
			else if (ip_hdr(skb)->version == IPVERSION_V6)
				eth.h_proto = htons(ETH_P_IPV6);

			if (skb_hnat_is_encap(skb) && !is_virt_dev &&
			    mtk_tnl_encap_offload && mtk_tnl_encap_offload(skb, &eth))
				break;
			if (skb_hnat_is_decap(skb))
				break;
		}

		skb_to_hnat_info(skb, out, entry, &hw_path);
		break;
	case HIT_BIND_KEEPALIVE_DUP_OLD_HDR:
		/* update hnat count to nf_conntrack by keepalive */
		if (hnat_priv->data->per_flow_accounting && hnat_priv->nf_stat_en)
			hnat_get_count(hnat_priv, skb_hnat_ppe(skb), skb_hnat_entry(skb), NULL);

		if (fn && !mtk_hnat_accel_type(skb))
			break;

		/* update dscp for qos */
		mtk_hnat_dscp_update(skb, entry);

		/* update mcast timestamp*/
		if (hnat_priv->data->version == MTK_HNAT_V1_3 &&
		    hnat_priv->data->mcast && entry->bfib1.sta == 1)
			entry->ipv4_hnapt.m_timestamp = foe_timestamp(hnat_priv, true);

		if (entry_hnat_is_bound(entry)) {
			memset(skb_hnat_info(skb), 0, sizeof(struct hnat_desc));

			return -1;
		}
		break;
	case HIT_BIND_MULTICAST_TO_CPU:
	case HIT_BIND_MULTICAST_TO_GMAC_CPU:
		/*do not forward to gdma again,if ppe already done it*/
		if (!IS_DSA_LAN(out) && (IS_LAN_GRP(out) || IS_WAN(out)))
			return -1;
		break;
	}

	return 0;
}

static unsigned int
mtk_hnat_ipv6_nf_local_out(void *priv, struct sk_buff *skb,
			   const struct nf_hook_state *state)
{
	struct foe_entry *entry;
	struct ipv6hdr *ip6h;
	struct iphdr _iphdr;
	const struct iphdr *iph;
	struct tcpudphdr _ports;
	const struct tcpudphdr *pptr;
	int udp = 0;

	if (!is_magic_tag_valid(skb))
		return NF_ACCEPT;

	if (unlikely(!skb_hnat_is_hashed(skb)))
		return NF_ACCEPT;

	if (skb_hnat_entry(skb) >= hnat_priv->foe_etry_num ||
	    skb_hnat_ppe(skb) >= CFG_PPE_NUM)
		return NF_ACCEPT;

	entry = &hnat_priv->foe_table_cpu[skb_hnat_ppe(skb)][skb_hnat_entry(skb)];
	if (skb_hnat_reason(skb) == HIT_UNBIND_RATE_REACH) {
		ip6h = ipv6_hdr(skb);
		if (ip6h->nexthdr == NEXTHDR_IPIP) {
			/* Map-E LAN->WAN: need to record orig info before fn. */
			if (mape_toggle) {
				iph = skb_header_pointer(skb, IPV6_HDR_LEN,
							 sizeof(_iphdr), &_iphdr);
				if (unlikely(!iph))
					return NF_ACCEPT;

				switch (iph->protocol) {
				case IPPROTO_UDP:
					udp = 1;
				case IPPROTO_TCP:
				break;

				default:
					return NF_ACCEPT;
				}

				pptr = skb_header_pointer(skb, IPV6_HDR_LEN + iph->ihl * 4,
							  sizeof(_ports), &_ports);
				if (unlikely(!pptr))
					return NF_ACCEPT;
				/* don't process inner fragment packets */
				if (ip_is_fragment(iph))
					return NF_ACCEPT;

				entry->bfib1.udp = udp;

				/* Map-E LAN->WAN record inner IPv4 header info. */
#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
				entry->bfib1.pkt_type = IPV4_MAP_E;
				entry->ipv4_dslite.iblk2.dscp = iph->tos;
				entry->ipv4_mape.new_sip = ntohl(iph->saddr);
				entry->ipv4_mape.new_dip = ntohl(iph->daddr);
				entry->ipv4_mape.new_sport = ntohs(pptr->src);
				entry->ipv4_mape.new_dport = ntohs(pptr->dst);
#else
				entry->ipv4_hnapt.iblk2.dscp = iph->tos;
				entry->ipv4_hnapt.new_sip = ntohl(iph->saddr);
				entry->ipv4_hnapt.new_dip = ntohl(iph->daddr);
				entry->ipv4_hnapt.new_sport = ntohs(pptr->src);
				entry->ipv4_hnapt.new_dport = ntohs(pptr->dst);
#endif
			} else {
				entry->bfib1.pkt_type = IPV4_DSLITE;
			}
		}
	}
	return NF_ACCEPT;
}

static unsigned int
mtk_hnat_ipv6_nf_post_routing(void *priv, struct sk_buff *skb,
			      const struct nf_hook_state *state)
{
	if (!skb)
		goto drop;

	post_routing_print(skb, state->in, state->out, __func__);

	/* if bridge-nf-call-iptables is enabled and the skb is forwarded in bridge-layer,
	 * state->out would be changed to bridge dev in br_nf_post_routing.
	 */
	if (netif_is_bridge_master(state->out) && (state->out != skb->dev)) {
		if (!mtk_hnat_nf_post_routing(skb, skb->dev, 0, __func__))
			return NF_ACCEPT;
	} else {
		if (!mtk_hnat_nf_post_routing(skb, state->out, hnat_ipv6_get_nexthop, __func__))
			return NF_ACCEPT;
	}

drop:
	if (skb && (debug_level >= 7))
		printk_ratelimited(KERN_WARNING
				   "%s:drop (iif=0x%x, out_dev=%s, CB2=0x%x, ppe_hash=0x%x,\n"
				   "sport=0x%x, reason=0x%x, alg=0x%x)\n",
				   __func__, skb_hnat_iface(skb), state->out->name,
				   HNAT_SKB_CB2(skb)->magic, skb_hnat_entry(skb),
				   skb_hnat_sport(skb), skb_hnat_reason(skb),
				   skb_hnat_alg(skb));

	return NF_DROP;
}

static unsigned int
mtk_hnat_ipv4_nf_post_routing(void *priv, struct sk_buff *skb,
			      const struct nf_hook_state *state)
{
	if (!skb)
		goto drop;

	post_routing_print(skb, state->in, state->out, __func__);

	/* if bridge-nf-call-iptables is enabled and the skb is forwarded in bridge-layer,
	 * state->out would be changed to bridge dev in br_nf_post_routing.
	 */
	if (netif_is_bridge_master(state->out) && (state->out != skb->dev)) {
		if (!mtk_hnat_nf_post_routing(skb, skb->dev, 0, __func__))
			return NF_ACCEPT;
	} else {
		if (!mtk_hnat_nf_post_routing(skb, state->out, hnat_ipv4_get_nexthop, __func__))
			return NF_ACCEPT;
	}

drop:
	if (skb && (debug_level >= 7))
		printk_ratelimited(KERN_WARNING
				   "%s:drop (iif=0x%x, out_dev=%s, CB2=0x%x, ppe_hash=0x%x,\n"
				   "sport=0x%x, reason=0x%x, alg=0x%x)\n",
				   __func__, skb_hnat_iface(skb), state->out->name,
				   HNAT_SKB_CB2(skb)->magic, skb_hnat_entry(skb),
				   skb_hnat_sport(skb), skb_hnat_reason(skb),
				   skb_hnat_alg(skb));

	return NF_DROP;
}

static unsigned int
mtk_pong_hqos_handler(void *priv, struct sk_buff *skb,
		      const struct nf_hook_state *state)
{
	struct vlan_ethhdr *veth;

	if (!skb)
		goto drop;

	if (!is_magic_tag_valid(skb))
		return NF_ACCEPT;

	veth = (struct vlan_ethhdr *)skb_mac_header(skb);

	if (IS_HQOS_MODE && eth_hdr(skb)->h_proto == HQOS_MAGIC_TAG) {
		skb_hnat_entry(skb) = ntohs(veth->h_vlan_TCI) & 0x3fff;
		skb_hnat_reason(skb) = HIT_BIND_FORCE_TO_CPU;
	}

	if (skb_hnat_iface(skb) == FOE_MAGIC_EXT)
		clr_from_extge(skb);

	/* packets from external devices -> xxx ,step 2, learning stage */
	if (do_ext2ge_fast_learn(state->in, skb) && (!qos_toggle ||
	    (qos_toggle && eth_hdr(skb)->h_proto != HQOS_MAGIC_TAG))) {
		if (!do_hnat_ext_to_ge2(skb, __func__))
			return NF_STOLEN;
		goto drop;
	}

	/* packets form ge -> external device */
	if (do_ge2ext_fast(state->in, skb)) {
		if (!do_hnat_ge_to_ext(skb, __func__))
			return NF_STOLEN;
		goto drop;
	}

	return NF_ACCEPT;

drop:
	if (skb && (debug_level >= 7))
		printk_ratelimited(KERN_WARNING
				   "%s:drop (in_dev=%s, iif=0x%x, CB2=0x%x, ppe_hash=0x%x,\n"
				   "sport=0x%x, reason=0x%x, alg=0x%x)\n",
				   __func__, state->in->name, skb_hnat_iface(skb),
				   HNAT_SKB_CB2(skb)->magic, skb_hnat_entry(skb),
				   skb_hnat_sport(skb), skb_hnat_reason(skb),
				   skb_hnat_alg(skb));

	return NF_DROP;
}

static unsigned int
mtk_hnat_br_nf_local_out(void *priv, struct sk_buff *skb,
			 const struct nf_hook_state *state)
{
	if (!skb)
		goto drop;

	if (!is_magic_tag_valid(skb))
		return NF_ACCEPT;

	post_routing_print(skb, state->in, state->out, __func__);

#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)
	/* process it in ipv4/ipv6 post-routing hook if enabled bridge-nf-call-iptables */
	if (nf_bridge_info_exists(skb))
		return NF_ACCEPT;
#endif

	if (!mtk_hnat_nf_post_routing(skb, state->out, 0, __func__))
		return NF_ACCEPT;

drop:
	if (skb && (debug_level >= 7))
		printk_ratelimited(KERN_WARNING
				   "%s:drop (iif=0x%x, out_dev=%s, CB2=0x%x, ppe_hash=0x%x,\n"
				   "sport=0x%x, reason=0x%x, alg=0x%x)\n",
				   __func__, skb_hnat_iface(skb), state->out->name,
				   HNAT_SKB_CB2(skb)->magic, skb_hnat_entry(skb),
				   skb_hnat_sport(skb), skb_hnat_reason(skb),
				   skb_hnat_alg(skb));

	return NF_DROP;
}

static unsigned int
mtk_hnat_ipv4_nf_local_out(void *priv, struct sk_buff *skb,
			   const struct nf_hook_state *state)
{
	struct foe_entry *entry;
	struct iphdr *iph;

	if (!is_magic_tag_valid(skb))
		return NF_ACCEPT;

	if (unlikely(skb_headroom(skb) < FOE_INFO_LEN))
		return NF_ACCEPT;

	if (!skb_hnat_is_hashed(skb))
		return NF_ACCEPT;

	if (skb_hnat_entry(skb) >= hnat_priv->foe_etry_num ||
	    skb_hnat_ppe(skb) >= CFG_PPE_NUM)
		return NF_ACCEPT;

	entry = &hnat_priv->foe_table_cpu[skb_hnat_ppe(skb)][skb_hnat_entry(skb)];

	/* Make the flow from local not be bound. */
	iph = ip_hdr(skb);
	if (iph->protocol == IPPROTO_IPV6) {
		entry->udib1.pkt_type = IPV6_6RD;
		hnat_set_head_frags(state, skb, 0, hnat_set_alg);
	} else if (is_magic_tag_valid(skb) &&
		   ((skb_hnat_cdrt(skb) && skb_hnat_is_encrypt(skb)) || skb_hnat_tops(skb))) {
		hnat_set_head_frags(state, skb, 0, hnat_set_alg);
	} else {
		hnat_set_head_frags(state, skb, 1, hnat_set_alg);
	}

	return NF_ACCEPT;
}

static unsigned int mtk_hnat_br_nf_forward(void *priv,
					   struct sk_buff *skb,
					   const struct nf_hook_state *state)
{
	if ((hnat_priv->data->version == MTK_HNAT_V1_2) &&
	    unlikely(IS_EXT(state->in) && IS_EXT(state->out)))
		hnat_set_head_frags(state, skb, 1, hnat_set_alg);

	return NF_ACCEPT;
}

static struct nf_hook_ops mtk_hnat_nf_ops[] __read_mostly = {
	{
		.hook = mtk_hnat_ipv4_nf_pre_routing,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FIRST + 1,
	},
	{
		.hook = mtk_hnat_ipv6_nf_pre_routing,
		.pf = NFPROTO_IPV6,
		.hooknum = NF_INET_PRE_ROUTING,
		.priority = NF_IP_PRI_FIRST + 1,
	},
	{
		.hook = mtk_hnat_ipv6_nf_post_routing,
		.pf = NFPROTO_IPV6,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_LAST,
	},
	{
		.hook = mtk_hnat_ipv6_nf_local_out,
		.pf = NFPROTO_IPV6,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_LAST,
	},
	{
		.hook = mtk_hnat_ipv4_nf_post_routing,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_LAST,
	},
	{
		.hook = mtk_hnat_ipv4_nf_local_out,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_LAST,
	},
	{
		.hook = mtk_hnat_br_nf_local_in,
		.pf = NFPROTO_BRIDGE,
		.hooknum = NF_BR_LOCAL_IN,
		.priority = NF_BR_PRI_FIRST,
	},
	{
		.hook = mtk_hnat_br_nf_local_out,
		.pf = NFPROTO_BRIDGE,
		.hooknum = NF_BR_LOCAL_OUT,
		.priority = NF_BR_PRI_LAST - 1,
	},
	{
		.hook = mtk_pong_hqos_handler,
		.pf = NFPROTO_BRIDGE,
		.hooknum = NF_BR_PRE_ROUTING,
		.priority = NF_BR_PRI_FIRST + 1,
	},
};

int hnat_register_nf_hooks(void)
{
	return nf_register_net_hooks(&init_net, mtk_hnat_nf_ops, ARRAY_SIZE(mtk_hnat_nf_ops));
}

void hnat_unregister_nf_hooks(void)
{
	nf_unregister_net_hooks(&init_net, mtk_hnat_nf_ops, ARRAY_SIZE(mtk_hnat_nf_ops));
}

int whnat_adjust_nf_hooks(void)
{
	struct nf_hook_ops *hook = mtk_hnat_nf_ops;
	unsigned int n = ARRAY_SIZE(mtk_hnat_nf_ops);

	while (n-- > 0) {
		if (hook[n].hook == mtk_hnat_br_nf_local_in) {
			hook[n].hooknum = NF_BR_PRE_ROUTING;
			hook[n].priority = NF_BR_PRI_FIRST + 1;
		} else if (hook[n].hook == mtk_hnat_br_nf_local_out) {
			hook[n].hooknum = NF_BR_POST_ROUTING;
		} else if (hook[n].hook == mtk_pong_hqos_handler) {
			hook[n].hook = mtk_hnat_br_nf_forward;
			hook[n].hooknum = NF_BR_FORWARD;
			hook[n].priority = NF_BR_PRI_LAST - 1;
		}
	}

	return 0;
}

int mtk_hqos_ptype_cb(struct sk_buff *skb, struct net_device *dev,
		      struct packet_type *pt, struct net_device *unused)
{
	struct vlan_ethhdr *veth = (struct vlan_ethhdr *)skb_mac_header(skb);

	skb_hnat_entry(skb) = ntohs(veth->h_vlan_TCI) & 0x3fff;
	skb_hnat_reason(skb) = HIT_BIND_FORCE_TO_CPU;

	if (do_hnat_ge_to_ext(skb, __func__) == -1)
		return 1;

	return 0;
}

int mtk_hnat_skb_headroom_copy(struct sk_buff *new, struct sk_buff *old)
{
	if (skb_headroom(new) < skb_headroom(old))
		return -EPERM;

	if (skb_hnat_reason(old) == HIT_UNBIND_RATE_REACH && skb_hnat_tops(old))
		memcpy(new->head, old->head, skb_headroom(old));

	return 0;
}

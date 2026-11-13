/*
* switch_fun.h: switch function sets
*/
#ifndef SWITCH_FUN_H
#define SWITCH_FUN_H

#include <stdbool.h>

struct switch_func_s {
	void (*pf_table_dump)(int argc, char *argv[]);
	void (*pf_table_clear)(int argc, char *argv[]);
	void (*pf_switch_reset)(int argc, char *argv[]);
	void (*pf_doArlAging)(int argc, char *argv[]);
	void (*pf_read_mib_counters)(int argc, char *argv[]);
	void (*pf_clear_mib_counters)(int argc, char *argv[]);
	void (*pf_read_output_queue_counters)(int argc, char *argv[]);
	void (*pf_read_free_page_counters)(int argc, char *argv[]);
	void (*pf_rate_control)(int argc, char *argv[]);
	void (*pf_igress_rate_set)(int argc, char *argv[]);
	void (*pf_egress_rate_set)(int argc, char *argv[]);
	void (*pf_table_add)(int argc, char *argv[]);
	void (*pf_table_del_fid)(int argc, char *argv[]);
	void (*pf_table_del_vid)(int argc, char *argv[]);
	void (*pf_table_search_mac_fid)(int argc, char *argv[]);
	void (*pf_table_search_mac_vid)(int argc, char *argv[]);
	void (*pf_global_set_mac_fc)(int argc, char *argv[]);
	void (*pf_set_mac_pfc)(int argc, char *argv[]);
	void (*pf_qos_sch_select)(int argc, char *argv[]);
	void (*pf_qos_set_base)(int argc, char *argv[]);
	void (*pf_qos_wfq_set_weight)(int argc, char *argv[]);
	void (*pf_qos_set_portpri)(int argc, char *argv[]);
	void (*pf_qos_set_dscppri)(int argc, char *argv[]);
	void (*pf_qos_pri_mapping_queue)(int argc, char *argv[]);
	void (*pf_doStp)(int argc, char *argv[]);
	void (*pf_sip_dump)(int argc, char *argv[]);
	void (*pf_sip_add)(int argc, char *argv[]);
	void (*pf_sip_del)(int argc, char *argv[]);
	void (*pf_sip_clear)(int argc, char *argv[]);
	void (*pf_dip_dump)(int argc, char *argv[]);
	void (*pf_dip_add)(int argc, char *argv[]);
	void (*pf_dip_del)(int argc, char *argv[]);
	void (*pf_dip_clear)(int argc, char *argv[]);
	void (*pf_set_mirror_to)(int argc, char *argv[]);
	void (*pf_set_mirror_from)(int argc, char *argv[]);
	void (*pf_doMirrorEn)(int argc, char *argv[]);
	void (*pf_doMirrorPortBased)(int argc, char *argv[]);
	void (*pf_acl_dip_add)(int argc, char *argv[]);
	void (*pf_acl_dip_modify)(int argc, char *argv[]);
	void (*pf_acl_dip_pppoe)(int argc, char *argv[]);
	void (*pf_acl_dip_trtcm)(int argc, char *argv[]);
	void (*pf_acl_dip_meter)(int argc, char *argv[]);
	void (*pf_acl_mac_add)(int argc, char *argv[]);
	void (*pf_acl_ethertype)(int argc, char *argv[]);
	void (*pf_acl_sp_add)(int argc, char *argv[]);
	void (*pf_acl_l4_add)(int argc, char *argv[]);
	void (*pf_acl_port_enable)(int argc, char *argv[]);
	void (*pf_acl_table_add)(int argc, char *argv[]);
	void (*pf_acl_mask_table_add)(int argc, char *argv[]);
	void (*pf_acl_rule_table_add)(int argc, char *argv[]);
	void (*pf_acl_rate_table_add)(int argc, char *argv[]);
	void (*pf_vlan_dump)(int argc, char *argv[]);
	void (*pf_vlan_set)(int argc, char *argv[]);
	void (*pf_vlan_clear)(int argc, char *argv[]);
	void (*pf_doVlanSetVid)(int argc, char *argv[]);
	void (*pf_doVlanSetPvid)(int argc, char *argv[]);
	void (*pf_doVlanSetAccFrm)(int argc, char *argv[]);
	void (*pf_doVlanSetPortAttr)(int argc, char *argv[]);
	void (*pf_doVlanSetPortMode)(int argc, char *argv[]);
	void (*pf_doVlanSetEgressTagPCR)(int argc, char *argv[]);
	void (*pf_doVlanSetEgressTagPVC)(int argc, char *argv[]);
	void (*pf_igmp_on)(int argc, char *argv[]);
	void (*pf_igmp_off)(int argc, char *argv[]);
	void (*pf_igmp_enable)(int argc, char *argv[]);
	void (*pf_igmp_disable)(int argc, char *argv[]);
	void (*pf_collision_pool_enable)(int argc, char *argv[]);
	void (*pf_collision_pool_mac_dump)(int argc, char *argv[]);
	void (*pf_collision_pool_dip_dump)(int argc, char *argv[]);
	void (*pf_collision_pool_sip_dump)(int argc, char *argv[]);
	void (*pf_pfc_get_rx_counter)(int argc, char *argv[]);
	void (*pf_pfc_get_tx_counter)(int argc, char *argv[]);
	void (*pf_eee_enable)(int argc, char *argv[]);
	void (*pf_eee_dump)(int argc, char *argv[]);
};

#define MT7530_T10_TEST_CONTROL 0x145

#define MAX_PORT 6
#define MAX_PHY_PORT 5
#define CONFIG_MTK_7531_DVT 1

extern int chip_name;
extern struct mt753x_attr *attres;
extern bool nl_init_flag;
extern struct switch_func_s mt753x_switch_func;
extern struct switch_func_s an8855_switch_func;

/*basic operation*/
int reg_read(unsigned int offset, unsigned int *value);
int reg_write(unsigned int offset, unsigned int value);
int mii_mgr_read(unsigned int port_num, unsigned int reg, unsigned int *value);
int mii_mgr_write(unsigned int port_num, unsigned int reg, unsigned int value);
int mii_mgr_c45_read(unsigned int port_num, unsigned int dev, unsigned int reg,
		unsigned int *value);
int mii_mgr_c45_write(unsigned int port_num, unsigned int dev, unsigned int reg,
		unsigned int value);

/*phy setting*/
int phy_dump(int phy_addr);
void phy_crossover(int argc, char *argv[]);
int rw_phy_token_ring(int argc, char *argv[]);
/*arl setting*/
void doArlAging(int argc, char *argv[]);

/*acl setting*/
void acl_mac_add(int argc, char *argv[]);
void acl_dip_meter(int argc, char *argv[]);
void acl_dip_trtcm(int argc, char *argv[]);
void acl_ethertype(int argc, char *argv[]);
void acl_ethertype(int argc, char *argv[]);
void acl_dip_modify(int argc, char *argv[]);
void acl_dip_pppoe(int argc, char *argv[]);
void acl_dip_add(int argc, char *argv[]);
void acl_l4_add(int argc, char *argv[]);
void acl_sp_add(int argc, char *argv[]);

void acl_port_enable(int argc, char *argv[]);
void acl_table_add(int argc, char *argv[]);
void acl_mask_table_add(int argc, char *argv[]);
void acl_rule_table_add(int argc, char *argv[]);
void acl_rate_table_add(int argc, char *argv[]);

/*dip table*/
void dip_dump(int argc, char *argv[]);
void dip_add(int argc, char *argv[]);
void dip_del(int argc, char *argv[]);
void dip_clear(int argc, char *argv[]);

/*sip table*/
void sip_dump(int argc, char *argv[]);
void sip_add(int argc, char *argv[]);
void sip_del(int argc, char *argv[]);
void sip_clear(int argc, char *argv[]);

/*stp*/
void doStp(int argc, char *argv[]);

/*mac table*/
void table_dump(int argc, char *argv[]);
void table_add(int argc, char *argv[]);
void table_search_mac_vid(int argc, char *argv[]);
void table_search_mac_fid(int argc, char *argv[]);
void table_del_fid(int argc, char *argv[]);
void table_del_vid(int argc, char *argv[]);
void table_clear(int argc, char *argv[]);

/*vlan table*/
void vlan_dump(int argc, char *argv[]);
void vlan_clear(int argc, char *argv[]);
void vlan_set(int argc, char *argv[]);

void doVlanSetPvid(int argc, char *argv[]);
void doVlanSetVid(int argc, char *argv[]);
void doVlanSetAccFrm(int argc, char *argv[]);
void doVlanSetPortAttr(int argc, char *argv[]);
void doVlanSetPortMode(int argc, char *argv[]);
void doVlanSetEgressTagPCR(int argc, char *argv[]);
void doVlanSetEgressTagPVC(int argc, char *argv[]);

/*igmp function*/
void igmp_on(int argc, char *argv[]);
void igmp_off(int argc, char *argv[]);
void igmp_disable(int argc, char *argv[]);
void igmp_enable(int argc, char *argv[]);

/*mirror function*/
void set_mirror_to(int argc, char *argv[]);
void set_mirror_from(int argc, char *argv[]);
void doMirrorPortBased(int argc, char *argv[]);
void doMirrorEn(int argc, char *argv[]);

/*rate control*/
void rate_control(int argc, char *argv[]);
void ingress_rate_set(int argc, char *argv[]);
void egress_rate_set(int argc, char *argv[]);

/*QoS*/
void qos_sch_select(int argc, char *argv[]);
void qos_set_base(int argc, char *argv[]);
void qos_wfq_set_weight(int argc, char *argv[]);
void qos_set_portpri(int argc, char *argv[]);
void qos_set_dscppri(int argc, char *argv[]);
void qos_pri_mapping_queue(int argc, char *argv[]);

/*flow control*/
void global_set_mac_fc(int argc, char *argv[]);
void phy_set_fc(int argc, char *argv[]);
void phy_set_an(int argc, char *argv[]);

/* collision pool functions */
void collision_pool_enable(int argc, char *argv[]);
void collision_pool_mac_dump(int argc, char *argv[]);
void collision_pool_dip_dump(int argc, char *argv[]);
void collision_pool_sip_dump(int argc, char *argv[]);

/*pfc functions*/
void set_mac_pfc(int argc, char *argv[]);
void pfc_get_rx_counter(int argc, char *argv[]);
void pfc_get_tx_counter(int argc, char *argv[]);

/*switch reset*/
void switch_reset(int argc, char *argv[]);

/* EEE(802.3az) function  */
void eee_enable(int argc, char *argv[]);
void eee_dump(int argc, char *argv[]);

void read_mib_counters(int argc, char *argv[]);
void clear_mib_counters(int argc, char *argv[]);
void read_output_queue_counters(int argc, char *argv[]);
void read_free_page_counters(int argc, char *argv[]);

void phy_crossover(int argc, char *argv[]);
void exit_free(void);
#endif

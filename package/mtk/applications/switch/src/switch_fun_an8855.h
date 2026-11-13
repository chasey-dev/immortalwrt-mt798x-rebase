/*
 * switch_fun.h: switch function sets
 */
#ifndef SWITCH_FUN_AN8855_H
#define SWITCH_FUN_AN8855_H

#include "air.h"

AIR_ERROR_NO_T
an8855_reg_read(const UI32_T unit, const UI32_T addr_offset, UI32_T * ptr_data);

AIR_ERROR_NO_T
an8855_reg_write(const UI32_T unit,
		 const UI32_T addr_offset, const UI32_T data);

AIR_ERROR_NO_T
an8855_phy_cl22_read(const UI32_T unit,
		     const UI32_T port_id,
		     const UI32_T addr_offset, UI32_T *ptr_data);

AIR_ERROR_NO_T
an8855_phy_cl22_write(const UI32_T unit,
		      const UI32_T port_id,
		      const UI32_T addr_offset, const UI32_T data);

AIR_ERROR_NO_T
an8855_phy_cl45_read(const UI32_T unit,
		     const UI32_T port_id,
		     const UI32_T dev_type,
		     const UI32_T addr_offset, UI32_T *ptr_data);

AIR_ERROR_NO_T
an8855_phy_cl45_write(const UI32_T unit,
		      const UI32_T port_id,
		      const UI32_T dev_type,
		      const UI32_T addr_offset, const UI32_T data);

/*arl setting*/
void an8855_doArlAging(int argc, char *argv[]);

void an8855_not_supported(int argc, char *argv[]);

/*stp*/
void an8855_doStp(int argc, char *argv[]);

/*mac table*/
void an8855_table_dump(int argc, char *argv[]);
void an8855_table_add(int argc, char *argv[]);
void an8855_table_search_mac_vid(int argc, char *argv[]);
void an8855_table_search_mac_fid(int argc, char *argv[]);
void an8855_table_del_fid(int argc, char *argv[]);
void an8855_table_del_vid(int argc, char *argv[]);
void an8855_table_clear(int argc, char *argv[]);

/*vlan table*/
void an8855_vlan_dump(int argc, char *argv[]);
void an8855_vlan_clear(int argc, char *argv[]);
void an8855_vlan_set(int argc, char *argv[]);

void an8855_doVlanSetPvid(int argc, char *argv[]);
void an8855_doVlanSetVid(int argc, char *argv[]);
void an8855_doVlanSetAccFrm(int argc, char *argv[]);
void an8855_doVlanSetPortAttr(int argc, char *argv[]);
void an8855_doVlanSetPortMode(int argc, char *argv[]);
void an8855_doVlanSetEgressTagPCR(int argc, char *argv[]);
void an8855_doVlanSetEgressTagPVC(int argc, char *argv[]);

/*mirror function*/
void an8855_set_mirror_to(int argc, char *argv[]);
void an8855_set_mirror_from(int argc, char *argv[]);
void an8855_doMirrorPortBased(int argc, char *argv[]);
void an8855_doMirrorEn(int argc, char *argv[]);

/*rate control*/
void an8855_rate_control(int argc, char *argv[]);
void an8855_ingress_rate_set(int argc, char *argv[]);
void an8855_egress_rate_set(int argc, char *argv[]);

/*QoS*/
void an8855_qos_sch_select(int argc, char *argv[]);
void an8855_qos_set_base(int argc, char *argv[]);
void an8855_qos_wfq_set_weight(int argc, char *argv[]);
void an8855_qos_set_portpri(int argc, char *argv[]);
void an8855_qos_set_dscppri(int argc, char *argv[]);
void an8855_qos_pri_mapping_queue(int argc, char *argv[]);

/*flow control*/
void an8855_global_set_mac_fc(int argc, char *argv[]);

/*switch reset*/
void an8855_switch_reset(int argc, char *argv[]);

/* EEE(802.3az) function  */
void an8855_eee_enable(int argc, char *argv[]);
void an8855_eee_dump(int argc, char *argv[]);

void an8855_read_mib_counters(int argc, char *argv[]);
void an8855_clear_mib_counters(int argc, char *argv[]);
void an8855_read_output_queue_counters(int argc, char *argv[]);
void an8855_read_free_page_counters(int argc, char *argv[]);

#endif

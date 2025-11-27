/*
 * switch_fun.c: switch function sets
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <stdbool.h>
#include <time.h>

#include "switch_extend.h"
#include "switch_netlink.h"
#include "switch_fun.h"
#include "switch_fun_an8855.h"

#define MAC_STR         "%02X%02X%02X%02X%02X%02X"

const static C8_T * mac_address_forward_control_string[] = {
	"Default",
	"CPU include",
	"CPU exclude",
	"CPU only",
	"Drop"
};

struct switch_func_s an8855_switch_func = {
	.pf_table_dump = an8855_table_dump,
	.pf_table_clear = an8855_table_clear,
	.pf_switch_reset = an8855_switch_reset,
	.pf_doArlAging = an8855_doArlAging,
	.pf_read_mib_counters = an8855_read_mib_counters,
	.pf_clear_mib_counters = an8855_clear_mib_counters,
	.pf_read_output_queue_counters = an8855_read_output_queue_counters,
	.pf_read_free_page_counters = an8855_read_free_page_counters,
	.pf_rate_control = an8855_rate_control,
	.pf_igress_rate_set = an8855_ingress_rate_set,
	.pf_egress_rate_set = an8855_egress_rate_set,
	.pf_table_add = an8855_table_add,
	.pf_table_del_fid = an8855_table_del_fid,
	.pf_table_del_vid = an8855_table_del_vid,
	.pf_table_search_mac_fid = an8855_table_search_mac_fid,
	.pf_table_search_mac_vid = an8855_table_search_mac_vid,
	.pf_global_set_mac_fc = an8855_global_set_mac_fc,
	.pf_set_mac_pfc = an8855_not_supported,
	.pf_qos_sch_select = an8855_qos_sch_select,
	.pf_qos_set_base = an8855_qos_set_base,
	.pf_qos_wfq_set_weight = an8855_qos_wfq_set_weight,
	.pf_qos_set_portpri = an8855_qos_set_portpri,
	.pf_qos_set_dscppri = an8855_qos_set_dscppri,
	.pf_qos_pri_mapping_queue = an8855_qos_pri_mapping_queue,
	.pf_doStp = an8855_doStp,
	.pf_sip_dump = an8855_not_supported,
	.pf_sip_add = an8855_not_supported,
	.pf_sip_del = an8855_not_supported,
	.pf_sip_clear = an8855_not_supported,
	.pf_dip_dump = an8855_not_supported,
	.pf_dip_add = an8855_not_supported,
	.pf_dip_del = an8855_not_supported,
	.pf_dip_clear = an8855_not_supported,
	.pf_set_mirror_to = an8855_set_mirror_to,
	.pf_set_mirror_from = an8855_set_mirror_from,
	.pf_doMirrorEn = an8855_doMirrorEn,
	.pf_doMirrorPortBased = an8855_doMirrorPortBased,
	.pf_acl_dip_add = an8855_not_supported,
	.pf_acl_dip_modify = an8855_not_supported,
	.pf_acl_dip_pppoe = an8855_not_supported,
	.pf_acl_dip_trtcm = an8855_not_supported,
	.pf_acl_dip_meter = an8855_not_supported,
	.pf_acl_mac_add = an8855_not_supported,
	.pf_acl_ethertype = an8855_not_supported,
	.pf_acl_sp_add = an8855_not_supported,
	.pf_acl_l4_add = an8855_not_supported,
	.pf_acl_port_enable = an8855_not_supported,
	.pf_acl_table_add = an8855_not_supported,
	.pf_acl_mask_table_add = an8855_not_supported,
	.pf_acl_rule_table_add = an8855_not_supported,
	.pf_acl_rate_table_add = an8855_not_supported,
	.pf_vlan_dump = an8855_vlan_dump,
	.pf_vlan_set = an8855_vlan_set,
	.pf_vlan_clear = an8855_vlan_clear,
	.pf_doVlanSetVid = an8855_doVlanSetVid,
	.pf_doVlanSetPvid = an8855_doVlanSetPvid,
	.pf_doVlanSetAccFrm = an8855_doVlanSetAccFrm,
	.pf_doVlanSetPortAttr = an8855_doVlanSetPortAttr,
	.pf_doVlanSetPortMode = an8855_doVlanSetPortMode,
	.pf_doVlanSetEgressTagPCR = an8855_doVlanSetEgressTagPCR,
	.pf_doVlanSetEgressTagPVC = an8855_doVlanSetEgressTagPVC,
	.pf_igmp_on = an8855_not_supported,
	.pf_igmp_off = an8855_not_supported,
	.pf_igmp_enable = an8855_not_supported,
	.pf_igmp_disable = an8855_not_supported,
	.pf_collision_pool_enable = an8855_not_supported,
	.pf_collision_pool_mac_dump = an8855_not_supported,
	.pf_collision_pool_dip_dump = an8855_not_supported,
	.pf_collision_pool_sip_dump = an8855_not_supported,
	.pf_pfc_get_rx_counter = an8855_not_supported,
	.pf_pfc_get_tx_counter = an8855_not_supported,
	.pf_eee_enable = an8855_eee_enable,
	.pf_eee_dump = an8855_eee_dump,
};

AIR_ERROR_NO_T
an8855_reg_read(const UI32_T unit, const UI32_T addr_offset, UI32_T *ptr_data)
{
	int ret;

	ret = reg_read(addr_offset, ptr_data);
	if (ret < 0)
		return AIR_E_OTHERS;

	return AIR_E_OK;
}

AIR_ERROR_NO_T
an8855_reg_write(const UI32_T unit, const UI32_T addr_offset, const UI32_T data)
{
	int ret;

	ret = reg_write(addr_offset, data);
	if (ret < 0)
		return AIR_E_OTHERS;

	return AIR_E_OK;
}

AIR_ERROR_NO_T
an8855_phy_cl22_read(const UI32_T unit,
		     const UI32_T port_id,
		     const UI32_T addr_offset, UI32_T *ptr_data)
{
	int ret;

	ret = mii_mgr_read(port_id, addr_offset, ptr_data);
	if (ret < 0)
		return AIR_E_OTHERS;

	return AIR_E_OK;
}

AIR_ERROR_NO_T
an8855_phy_cl22_write(const UI32_T unit,
		      const UI32_T port_id,
		      const UI32_T addr_offset, const UI32_T data)
{
	int ret;

	ret = mii_mgr_write(port_id, addr_offset, data);
	if (ret < 0)
		return AIR_E_OTHERS;


	return AIR_E_OK;
}

AIR_ERROR_NO_T
an8855_phy_cl45_read(const UI32_T unit,
		     const UI32_T port_id,
		     const UI32_T dev_type,
		     const UI32_T addr_offset, UI32_T *ptr_data)
{
	int ret;

	ret = mii_mgr_c45_read(port_id, dev_type, addr_offset, ptr_data);
	if (ret < 0)
		return AIR_E_OTHERS;


	return AIR_E_OK;
}

AIR_ERROR_NO_T
an8855_phy_cl45_write(const UI32_T unit,
		      const UI32_T port_id,
		      const UI32_T dev_type,
		      const UI32_T addr_offset, const UI32_T data)
{
	int ret;

	ret = mii_mgr_c45_write(port_id, dev_type, addr_offset, data);
	if (ret < 0)
		return AIR_E_OTHERS;


	return AIR_E_OK;
}

void an8855_not_supported(int argc, char *argv[])
{
	printf("Cmd not supported by AN8855.\n");
}

static AIR_ERROR_NO_T
_printMacEntry(AIR_MAC_ENTRY_T *mt, UI32_T age_unit, UI8_T count, UI8_T title)
{
	AIR_ERROR_NO_T ret = AIR_E_OK;
	I32_T i = 0, j = 0;
	UI8_T first = 0;
	UI8_T find = 0;

	if (title) {
		printf("%-6s%-15s%-5s%-5s%-5s%-10s%-10s%-6s\n",
		       "unit",
		       "mac",
		       "ivl", "vid", "fid", "age-time", "forward", "port");
		return ret;
	}
	for (i = 0; i < count; i++) {
		printf("%-6d", age_unit);
		printf(MAC_STR, mt[i].mac[0], mt[i].mac[1], mt[i].mac[2],
			 mt[i].mac[3], mt[i].mac[4], mt[i].mac[5]);
		printf("...");
		if (mt[i].flags & AIR_L2_MAC_ENTRY_FLAGS_IVL) {
			printf("%-3s..", "ivl");
			printf("%-5d", mt[i].cvid);
			printf("%-5s", ".....");
		} else {
			printf("%-3s..", "svl");
			printf("%-5s", ".....");
			printf("%-5d", mt[i].fid);
		}
		if (mt[i].flags & AIR_L2_MAC_ENTRY_FLAGS_STATIC)
			printf("%-7s.", "static");
		else
			printf("%d sec..", mt[i].timer);

		printf("%-10s",
		       mac_address_forward_control_string[mt[i].sa_fwd]);
		first = 0;
		find = 0;
		for (j = (AIR_MAX_NUM_OF_PORTS - 1); j >= 0; j--) {
			if ((mt[i].port_bitmap[0]) & (1 << j)) {
				first = j;
				find = 1;
				break;
			}
		}
		if (find) {
			for (j = 0; j < AIR_MAX_NUM_OF_PORTS; j++) {
				if ((mt[i].port_bitmap[0]) & (1 << j)) {
					if (j == first)
						printf("%-2d", j);
					else
						printf("%-2d,", j);
				}
			}
		} else
			printf("no dst port");
		printf("\n");
	}
	return ret;
}

static AIR_ERROR_NO_T _str2mac(C8_T *str, C8_T *mac)
{
	UI32_T i;
	C8_T tmpstr[3];

	for (i = 0; i < 6; i++) {
		strncpy(tmpstr, str + (i * 2), 2);
		tmpstr[2] = '\0';
		mac[i] = strtoul(tmpstr, NULL, 16);
	}

	return AIR_E_OK;
}

static void an8855_table_dump_internal(int type)
{
	unsigned char count = 0;
	unsigned int total_count = 0;
	unsigned int bucket_size = 0;
	AIR_ERROR_NO_T ret = 0;
	AIR_MAC_ENTRY_T *ptr_mt;

	bucket_size = AIR_L2_MAC_SET_NUM;
	ptr_mt = malloc(sizeof(AIR_MAC_ENTRY_T) * bucket_size);
	if (ptr_mt == NULL) {
		printf("Error, malloc fail\n\r");
		return;
	}
	memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T) * bucket_size);
	_printMacEntry(ptr_mt, 0, count, TRUE);
	/* get 1st entry of MAC table */
	ret = air_l2_getMacAddr(0, &count, ptr_mt);
	switch (ret) {
	case AIR_E_ENTRY_NOT_FOUND:
		printf("Not Found!\n");
		goto DUMP_ERROR;
	case AIR_E_TIMEOUT:
		printf("Time Out!\n");
		goto DUMP_ERROR;
	case AIR_E_BAD_PARAMETER:
		printf("Bad Parameter!\n");
		goto DUMP_ERROR;
	default:
		break;
	}
	total_count += count;
	_printMacEntry(ptr_mt, 0, count, FALSE);
	/* get other entries of MAC table */
	while (1) {
		memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T) * bucket_size);
		ret = air_l2_getNextMacAddr(0, &count, ptr_mt);
		if (ret != AIR_E_OK)
			break;

		total_count += count;
		_printMacEntry(ptr_mt, 0, count, FALSE);
	}
	switch (ret) {
	case AIR_E_TIMEOUT:
		printf("Time Out!\n");
		break;
	case AIR_E_BAD_PARAMETER:
		printf("Bad Parameter!\n");
		break;
	default:
		printf("Found %u %s\n", total_count,
		       (total_count > 1) ? "entries" : "entry");
		break;
	}

DUMP_ERROR:
	free(ptr_mt);
}

void an8855_table_dump(int argc, char *argv[])
{
	an8855_table_dump_internal(GENERAL_TABLE);
}

void an8855_table_add(int argc, char *argv[])
{
	AIR_ERROR_NO_T ret = AIR_E_OK;
	AIR_MAC_ENTRY_T mt;
	unsigned int i = 0;
	unsigned int age_time = 0;

	memset(&mt, 0, sizeof(AIR_MAC_ENTRY_T));
	if (!argv[2] || strlen(argv[2]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}
	ret = _str2mac(argv[2], (C8_T *) mt.mac);
	if (ret != AIR_E_OK) {
		printf("Unrecognized command.\n");
		return;
	}
	if (argc > 4) {
		mt.cvid = strtoul(argv[4], NULL, 0);
		if (mt.cvid > 4095) {
			printf("wrong vid range, should be within 0~4095\n");
			return;
		}
		mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
	}
	if (!argv[3] || strlen(argv[3]) != 6) {
		/*bit0~5, each map port0~port5 */
		printf("portmap format error, should be of length 6\n");
		return;
	}
	for (i = 0; i < 6; i++) {
		if (argv[3][i] != '0' && argv[3][i] != '1') {
			printf
			    ("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		mt.port_bitmap[0] |= ((argv[3][i] - '0') << i);
	}
	if (argc > 5) {
		age_time = strtoul(argv[5], NULL, 0);
		if (age_time < 1 || 1000000 < age_time) {
			printf("wrong age range, should be within 1~1000000\n");
			return;
		}
	} else
		mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_STATIC;

	mt.sa_fwd = AIR_L2_FWD_CTRL_DEFAULT;
	ret = air_l2_addMacAddr(0, &mt);
	if (ret == AIR_E_OK) {
		printf("add mac address done.\n");
		usleep(5000);
		if (!(mt.flags & AIR_L2_MAC_ENTRY_FLAGS_STATIC)) {
			ret = air_l2_setMacAddrAgeOut(0, age_time);
			if (ret == AIR_E_OK)
				printf("set age out time done.\n");
			else
				printf("set age out time fail.\n");

		}
	} else
		printf("add mac address fail.\n");
}

void an8855_table_search_mac_vid(int argc, char *argv[])
{
	AIR_ERROR_NO_T ret = AIR_E_OK;
	unsigned char count = 0;
	char tmpstr[9];
	AIR_MAC_ENTRY_T *ptr_mt;

	if (!argv[3] || strlen(argv[3]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}

	ptr_mt = malloc(sizeof(AIR_MAC_ENTRY_T));
	if (ptr_mt == NULL) {
		printf("Error, malloc fail\n");
		return;
	}
	memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T));
	ret = _str2mac(argv[3], (C8_T *) ptr_mt->mac);
	if (ret != AIR_E_OK) {
		printf("Unrecognized command.\n");
		free(ptr_mt);
		return;
	}

	/* get mac entry by MAC address & vid */
	ptr_mt->cvid = strtoul(argv[5], NULL, 0);
	ptr_mt->flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
	if (ptr_mt->cvid > 4095) {
		printf("wrong vid range, should be within 0~4095\n");
		free(ptr_mt);
		return;
	}

	ret = air_l2_getMacAddr(0, &count, ptr_mt);
	if (ret == AIR_E_OK) {
		_printMacEntry(ptr_mt, 0, 1, TRUE);
		_printMacEntry(ptr_mt, 0, 1, FALSE);
	} else {
		printf("\n Not found!\n");
	}
	free(ptr_mt);
}

void an8855_table_search_mac_fid(int argc, char *argv[])
{
	AIR_ERROR_NO_T ret = AIR_E_OK;
	unsigned char count = 0;
	char tmpstr[9];
	AIR_MAC_ENTRY_T *ptr_mt;

	if (!argv[3] || strlen(argv[3]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}

	ptr_mt = malloc(sizeof(AIR_MAC_ENTRY_T));
	if (ptr_mt == NULL) {
		printf("Error, malloc fail\n");
		return;
	}
	memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T));
	ret = _str2mac(argv[3], (C8_T *) ptr_mt->mac);
	if (ret != AIR_E_OK) {
		printf("Unrecognized command.\n");
		free(ptr_mt);
		return;
	}

	/* get mac entry by MAC address & fid */
	ptr_mt->fid = strtoul(argv[5], NULL, 0);
	if (ptr_mt->fid > 7) {
		printf("wrong fid range, should be within 0~7\n");
		free(ptr_mt);
		return;
	}

	ret = air_l2_getMacAddr(0, &count, ptr_mt);
	if (ret == AIR_E_OK) {
		_printMacEntry(ptr_mt, 0, 1, TRUE);
		_printMacEntry(ptr_mt, 0, 1, FALSE);
	} else
		printf("\n Not found!\n");

	free(ptr_mt);
}

void an8855_table_del_fid(int argc, char *argv[])
{
	AIR_ERROR_NO_T ret = AIR_E_OK;
	char tmpstr[9];
	AIR_MAC_ENTRY_T mt;

	if (!argv[3] || strlen(argv[3]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}

	memset(&mt, 0, sizeof(AIR_MAC_ENTRY_T));
	ret = _str2mac(argv[3], (C8_T *) mt.mac);
	if (ret != AIR_E_OK) {
		printf("Unrecognized command.\n");
		return;
	}

	/* get mac entry by MAC address & fid */
	mt.fid = strtoul(argv[5], NULL, 0);
	if (mt.fid > 7) {
		printf("wrong fid range, should be within 0~7\n");
		return;
	}

	ret = air_l2_delMacAddr(0, &mt);
	if (ret == AIR_E_OK)
		printf("Done.\n");
	else
		printf("Fail.\n");
}

void an8855_table_del_vid(int argc, char *argv[])
{
	AIR_ERROR_NO_T ret = AIR_E_OK;
	char tmpstr[9];
	AIR_MAC_ENTRY_T mt;

	if (!argv[3] || strlen(argv[3]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}

	memset(&mt, 0, sizeof(AIR_MAC_ENTRY_T));
	ret = _str2mac(argv[3], (C8_T *) mt.mac);
	if (ret != AIR_E_OK) {
		printf("Unrecognized command.\n");
		return;
	}

	/* get mac entry by MAC address & vid */
	mt.cvid = strtoul(argv[5], NULL, 0);
	mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
	if (mt.cvid > 4095) {
		printf("wrong vid range, should be within 0~4095\n");
		return;
	}

	ret = air_l2_delMacAddr(0, &mt);
	if (ret == AIR_E_OK)
		printf("Done.\n");
	else
		printf("Fail.\n");

}

void an8855_table_clear(int argc, char *argv[])
{
	AIR_ERROR_NO_T ret = AIR_E_OK;

	ret = air_l2_clearMacAddr(0);
	if (ret == AIR_E_OK)
		printf("Clear MAC Address Table Done.\n");
	else
		printf("Clear MAC Address Table Fail.\n");

}

void an8855_set_mirror_to(int argc, char *argv[])
{
	int idx;
	AIR_MIR_SESSION_T session = { 0 };

	idx = strtoul(argv[3], NULL, 0);
	if (idx < 0 || idx > MAX_PORT) {
		printf("wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	memset(&session, 0, sizeof(AIR_MIR_SESSION_T));

	air_mir_getSession(0, 0, &session);
	session.dst_port = idx;
	session.flags |= AIR_MIR_SESSION_FLAGS_ENABLE;
	air_mir_addSession(0, 0, &session);
}

void an8855_set_mirror_from(int argc, char *argv[])
{
	int idx = 0, mirror = 0;
	AIR_MIR_SESSION_T session = { 0 };

	idx = _strtoul(argv[3], NULL, 0);
	mirror = _strtoul(argv[4], NULL, 0);

	if (idx < 0 || idx > MAX_PORT) {
		printf("wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	if (mirror < 0 || mirror > 3) {
		printf("wrong mirror setting, should be within 0~3\n");
		return;
	}

	memset(&session, 0, sizeof(AIR_MIR_SESSION_T));

	if (mirror & 0x1) {	// rx enable
		session.src_port = idx;
		air_mir_getMirrorPort(0, 0, &session);

		session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
		session.src_port = idx;
		air_mir_setMirrorPort(0, 0, &session);
	}

	if (mirror & 0x2) {	//tx enable
		session.src_port = idx;
		air_mir_getMirrorPort(0, 0, &session);

		session.flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
		session.src_port = idx;
		air_mir_setMirrorPort(0, 0, &session);
	}
}

void an8855_vlan_dump(int argc, char *argv[])
{
	AIR_VLAN_ENTRY_T vlan_entry = { 0 };
	unsigned int i;
	int eg_tag = 0;

	if (argc == 4) {
		if (!strncmp(argv[3], "egtag", 6))
			eg_tag = 1;
	}

	if (eg_tag)
		printf
		    ("  vid  fid  portmap   s-tag\teg_tag(0:untagged 2:tagged)\n");
	else
		printf("  vid  fid  portmap   s-tag\n");

	for (i = 1; i < 4096; i++) {
		_air_vlan_readEntry(0, i, &vlan_entry);

		if (vlan_entry.valid) {
			printf(" %4d  ", i);
			printf(" %2d ", vlan_entry.vlan_entry_format.fid);
			printf(" %c", (vlan_entry.vlan_entry_format.port_mem & 0b0000001)
				? '1' : '-');
			printf("%c", (vlan_entry.vlan_entry_format.port_mem & 0b0000010)
				? '1' : '-');
			printf("%c", (vlan_entry.vlan_entry_format.port_mem & 0b0000100)
				? '1' : '-');
			printf("%c", (vlan_entry.vlan_entry_format.port_mem & 0b0001000)
				? '1' : '-');
			printf("%c", (vlan_entry.vlan_entry_format.port_mem & 0b0010000)
				? '1' : '-');
			printf("%c", (vlan_entry.vlan_entry_format.port_mem & 0b0100000)
				? '1' : '-');
			printf("%c", (vlan_entry.vlan_entry_format.port_mem & 0b1000000)
				? '1' : '-');
			printf("    %4d", vlan_entry.vlan_entry_format.eg_ctrl);
			if (eg_tag) {
				printf("\t");
				if (vlan_entry.vlan_entry_format.eg_con
				    && vlan_entry.vlan_entry_format.eg_ctrl_en) {
					/* VTAG_EN=1 and EG_CON=1 */
					printf("CONSISTENT");
				} else if (vlan_entry.vlan_entry_format.eg_ctrl_en) {
					/* VTAG_EN=1 */
					printf("%d",
						(vlan_entry.vlan_entry_format.eg_ctrl >> 0) & 0x3);
					printf("%d",
						(vlan_entry.vlan_entry_format.eg_ctrl >> 2) & 0x3);
					printf("%d",
						(vlan_entry.vlan_entry_format.eg_ctrl >> 4) & 0x3);
					printf("%d",
						(vlan_entry.vlan_entry_format.eg_ctrl >> 6) & 0x3);
					printf("%d",
						(vlan_entry.vlan_entry_format.eg_ctrl >> 8) & 0x3);
					printf("%d",
						(vlan_entry.vlan_entry_format.eg_ctrl >> 10) & 0x3);
					printf("%d",
						(vlan_entry.vlan_entry_format.eg_ctrl >> 12) & 0x3);
				} else {
					/* VTAG_EN=0 */
					printf("DISABLED");
				}
			}
			printf("\n");
		} else {
			/*print 16 vid for reference information */
			if (i <= 16) {
				printf(" %4d  ", i);
				printf(" %2d ",
				       vlan_entry.vlan_entry_format.fid);
				printf(" invalid\n");
			}
		}
	}
}

void an8855_vlan_clear(int argc, char *argv[])
{
	air_vlan_destroyAll(0, 0);
}

void an8855_vlan_set(int argc, char *argv[])
{
	unsigned int vlan_mem = 0;
	int i, vid, fid;
	int stag = 0;
	unsigned long eg_con = 0;
	unsigned int eg_tag = 0;
	AIR_VLAN_ENTRY_T vlan_entry = { 0 };

	if (argc < 5) {
		printf("insufficient arguments!\n");
		return;
	}

	fid = strtoul(argv[3], NULL, 0);
	if (fid < 0 || fid > 7) {
		printf("wrong filtering db id range, should be within 0~7\n");
		return;
	}

	vid = strtoul(argv[4], NULL, 0);
	if (vid < 0 || MAX_VID_VALUE < vid) {
		printf("wrong vlan id range, should be within 0~4095\n");
		return;
	}

	if (strlen(argv[5]) != SWITCH_MAX_PORT) {
		printf("portmap format error, should be of length %d\n",
		       SWITCH_MAX_PORT);
		return;
	}

	vlan_mem = 0;
	for (i = 0; i < SWITCH_MAX_PORT; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf
			    ("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		vlan_mem += (argv[5][i] - '0') * (1 << i);
	}

	/* VLAN stag */
	if (argc > 6) {
		stag = strtoul(argv[6], NULL, 16);
		if (stag < 0 || 0xfff < stag) {
			printf
			    ("wrong stag id range, should be within 0~4095\n");
			return;
		}
	}

	/* set vlan member */
	vlan_entry.vlan_entry_format.port_mem = vlan_mem;
	vlan_entry.vlan_entry_format.ivl = 1;
	vlan_entry.vlan_entry_format.stag = stag;
	vlan_entry.valid = 1;

	if (argc > 7) {
		eg_con = strtoul(argv[7], NULL, 2);
		eg_con = !!eg_con;
		vlan_entry.vlan_entry_format.eg_con = eg_con;
		vlan_entry.vlan_entry_format.eg_ctrl_en = 1;
	}

	if (argc > 8 && !eg_con) {
		if (strlen(argv[8]) != SWITCH_MAX_PORT) {
			printf
			    ("egtag portmap format error, should be of length %d\n",
			     SWITCH_MAX_PORT);
			return;
		}

		for (i = 0; i < SWITCH_MAX_PORT; i++) {
			if (argv[8][i] < '0' || argv[8][i] > '3') {
				printf("egtag portmap format error,");
				printf("should be of combination of 0 or 3\n");
				return;
			}
			eg_tag |= (argv[8][i] - '0') << (i * 2);
		}

		vlan_entry.vlan_entry_format.eg_ctrl_en = 1;
		vlan_entry.vlan_entry_format.eg_ctrl = eg_tag;
	}

	_air_vlan_writeEntry(0, vid, &vlan_entry);
}

void an8855_switch_reset(int argc, char *argv[])
{
	air_switch_reset(0);
}

void an8855_global_set_mac_fc(int argc, char *argv[])
{
	unsigned char enable = 0;
	unsigned int reg = 0, value = 0;

	enable = _strtoul(argv[3], NULL, 10);
	printf("enable: %d\n", enable);

	/* Check the input parameters is right or not. */
	if (enable > 1) {
		printf(HELP_MACCTL_FC);
		return;
	}
	reg_read(0x10207e04, &value);
	value &= (~(1 << 31));
	value |= (enable << 31);
	reg_write(0x10207e04, value);
}				/*end mac_set_fc */

void an8855_qos_sch_select(int argc, char *argv[])
{
	unsigned char port, queue;
	unsigned char type = 0;
	unsigned int value, reg;

	if (argc < 7)
		return;

	port = _strtoul(argv[3], NULL, 10);
	queue = _strtoul(argv[4], NULL, 10);
	type = _strtoul(argv[6], NULL, 10);

	if (port > 6 || queue > 7) {
		printf("\n Illegal input parameters\n");
		return;
	}

	if ((type != 0 && type != 1 && type != 2)) {
		printf(HELP_QOS_TYPE);
		return;
	}

	printf("\r\nswitch qos type: %d.\n", type);

	if (type == 0) {
		air_qos_setScheduleAlgo(0, port, queue, AIR_QOS_SCH_MODE_WRR,
					1);
	} else if (type == 1) {
		air_qos_setScheduleAlgo(0, port, queue, AIR_QOS_SCH_MODE_SP, 1);
	} else {
		air_qos_setScheduleAlgo(0, port, queue, AIR_QOS_SCH_MODE_WFQ,
					1);
	}
}

void an8855_get_upw(unsigned int *value, unsigned char base)
{
	*value &= (~((0x7 << 0) | (0x7 << 4) | (0x7 << 8) | (0x7 << 12) |
		     (0x7 << 16) | (0x7 << 20)));
	switch (base) {
	case 0:		/* port-based 0x2x40[18:16] */
		*value |= ((0x2 << 0) | (0x2 << 4) | (0x2 << 8) |
			   (0x2 << 12) | (0x7 << 16) | (0x2 << 20));
		break;
	case 1:		/* tagged-based 0x2x40[10:8] */
		*value |= ((0x2 << 0) | (0x2 << 4) | (0x7 << 8) |
			   (0x2 << 12) | (0x2 << 16) | (0x2 << 20));
		break;
	case 2:		/* DSCP-based 0x2x40[14:12] */
		*value |= ((0x2 << 0) | (0x2 << 4) | (0x2 << 8) |
			   (0x7 << 12) | (0x2 << 16) | (0x2 << 20));
		break;
	case 3:		/* acl-based 0x2x40[2:0] */
		*value |= ((0x7 << 0) | (0x2 << 4) | (0x2 << 8) |
			   (0x2 << 12) | (0x2 << 16) | (0x2 << 20));
		break;
	case 4:		/* arl-based 0x2x40[22:20] */
		*value |= ((0x2 << 0) | (0x2 << 4) | (0x2 << 8) |
			   (0x2 << 12) | (0x2 << 16) | (0x7 << 20));
		break;
	case 5:		/* stag-based 0x2x40[6:4] */
		*value |= ((0x2 << 0) | (0x7 << 4) | (0x2 << 8) |
			   (0x2 << 12) | (0x2 << 16) | (0x2 << 20));
		break;
	default:
		break;
	}
}

void an8855_qos_set_base(int argc, char *argv[])
{
	unsigned char base = 0;
	unsigned char port;
	unsigned int value;

	if (argc < 5)
		return;

	port = _strtoul(argv[3], NULL, 10);
	base = _strtoul(argv[4], NULL, 10);

	if (base > 6) {
		printf(HELP_QOS_BASE);
		return;
	}

	if (port > 6) {
		printf("Illegal port index:%d\n", port);
		return;
	}

	printf("\r\nswitch qos base : %d. (port-based:0, tag-based:1,", base);
	printf("dscp-based:2, acl-based:3, arl-based:4, stag-based:5)\n");
	reg_read(0x10208030 + 0x200 * port, &value);
	an8855_get_upw(&value, base);
	reg_write(0x10208030 + 0x200 * port, value);
}

void an8855_qos_wfq_set_weight(int argc, char *argv[])
{
	int port, weight[8], i;
	unsigned char queue;
	unsigned int reg = 0, value = 0;

	port = _strtoul(argv[3], NULL, 10);

	for (i = 0; i < 8; i++)
		weight[i] = _strtoul(argv[i + 4], NULL, 10);

	/* MT7530 total 7 port */
	if (port < 0 || port > 6) {
		printf(HELP_QOS_PORT_WEIGHT);
		return;
	}

	for (i = 0; i < 8; i++) {
		if (weight[i] < 1 || weight[i] > 16) {
			printf(HELP_QOS_PORT_WEIGHT);
			return;
		}
	}
	printf("port: %x, q0: %x, q1: %x, q2: %x, q3: %x,",
		port, weight[0], weight[1], weight[2], weight[3]);
	printf("q4: %x, q5: %x, q6: %x, q7: %x\n",
		weight[4], weight[5], weight[6], weight[7]);

	for (queue = 0; queue < 8; queue++)
		air_qos_setScheduleAlgo(0, port, queue, AIR_QOS_SCH_MODE_WFQ,
					weight[queue]);

}

void an8855_qos_set_portpri(int argc, char *argv[])
{
	unsigned char port = 0, prio = 0;
	unsigned int value = 0;

	port = _strtoul(argv[3], NULL, 10);
	prio = _strtoul(argv[4], NULL, 10);

	if (port >= 7 || prio > 7) {
		printf(HELP_QOS_PORT_PRIO);
		return;
	}

	air_qos_setPortPriority(0, port, prio);
}

void an8855_qos_set_dscppri(int argc, char *argv[])
{
	unsigned char prio = 0, dscp = 0, pim_n = 0, pim_offset = 0;
	unsigned int reg = 0, value = 0;

	dscp = _strtoul(argv[3], NULL, 10);
	prio = _strtoul(argv[4], NULL, 10);

	if (dscp > 63 || prio > 7) {
		printf(HELP_QOS_DSCP_PRIO);
		return;
	}

	air_qos_setDscp2Pri(0, dscp, prio);
}

void an8855_qos_pri_mapping_queue(int argc, char *argv[])
{
	unsigned char prio = 0, queue = 0, pem_n = 0, port = 0;
	unsigned int reg = 0, value = 0;

	if (argc < 6)
		return;

	port = _strtoul(argv[3], NULL, 10);
	prio = _strtoul(argv[4], NULL, 10);
	queue = _strtoul(argv[5], NULL, 10);

	if (prio > 7 || queue > 7) {
		printf(HELP_QOS_PRIO_QMAP);
		return;
	}

	air_qos_setPri2Queue(0, prio, queue);
}

void an8855_doVlanSetPvid(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned short pvid = 0;

	port = _strtoul(argv[3], NULL, 10);
	pvid = _strtoul(argv[4], NULL, 10);
	/*Check the input parameters is right or not. */
	if ((port >= SWITCH_MAX_PORT) || (pvid > MAX_VID_VALUE)) {
		printf(HELP_VLAN_PVID);
		return;
	}

	air_vlan_setPortPVID(0, port, pvid);

	printf("port:%d pvid:%d,vlancap: max_port:%d maxvid:%d\r\n",
	       port, pvid, SWITCH_MAX_PORT, MAX_VID_VALUE);
}				/*end doVlanSetPvid */

void an8855_doVlanSetVid(int argc, char *argv[])
{
	unsigned char index = 0;
	unsigned char active = 0;
	unsigned char portMap = 0;
	unsigned char tagPortMap = 0;
	unsigned short vid = 0;

	unsigned char ivl_en = 0;
	unsigned char fid = 0;
	unsigned short stag = 0;
	int i = 0;
	AIR_VLAN_ENTRY_T vlan_entry = { 0 };

	index = _strtoul(argv[3], NULL, 10);
	active = _strtoul(argv[4], NULL, 10);
	vid = _strtoul(argv[5], NULL, 10);

	/*Check the input parameters is right or not. */
	if ((index >= MAX_VLAN_RULE) || (vid >= 4096) || (active > ACTIVED)) {
		printf(HELP_VLAN_VID);
		return;
	}

	/*CPU Port is always the membership */
	portMap = _strtoul(argv[6], NULL, 10);
	tagPortMap = _strtoul(argv[7], NULL, 10);

	printf("subcmd parameter argc = %d\r\n", argc);
	if (argc >= 9) {
		ivl_en = _strtoul(argv[8], NULL, 10);
		if (argc >= 10) {
			fid = _strtoul(argv[9], NULL, 10);
			if (argc >= 11)
				stag = _strtoul(argv[10], NULL, 10);
		}
	}

	printf("index: %x, active: %x, vid: %x, portMap: %x,",
		index, active, vid, portMap);
	printf("tagPortMap: %x, ivl_en: %x, fid: %x, stag: %x\n",
		tagPortMap, ivl_en, fid, stag);

	vlan_entry.valid = !!active;
	vlan_entry.vlan_entry_format.port_mem = portMap;
	/* Total 6 ports */
	for (i = 0; i < SWITCH_MAX_PORT; i++) {
		if (tagPortMap & (1 << i))
			vlan_entry.vlan_entry_format.eg_ctrl |= 0x2 << (i * 2);
	}
	vlan_entry.vlan_entry_format.ivl = !!ivl_en;
	vlan_entry.vlan_entry_format.fid = fid;
	vlan_entry.vlan_entry_format.stag = stag;

	_air_vlan_writeEntry(0, vid, &vlan_entry);

	printf("index:%d active:%d vid:%d\r\n", index, active, vid);
}				/*end doVlanSetVid */

void an8855_doVlanSetAccFrm(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char type = 0;
	AIR_VLAN_ACCEPT_FRAME_TYPE_T type_t = AIR_VLAN_ACCEPT_FRAME_TYPE_ALL;

	port = _strtoul(argv[3], NULL, 10);
	type = _strtoul(argv[4], NULL, 10);

	printf("port: %d, type: %d\n", port, type);

	/*Check the input parameters is right or not. */
	if ((port > SWITCH_MAX_PORT) || (type > REG_PVC_ACC_FRM_RELMASK)) {
		printf(HELP_VLAN_ACC_FRM);
		return;
	}

	type_t = (AIR_VLAN_ACCEPT_FRAME_TYPE_T) type;
	air_vlan_setPortAcceptFrameType(0, port, type_t);
}				/*end doVlanSetAccFrm */

void an8855_doVlanSetPortAttr(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char attr = 0;
	AIR_VLAN_PORT_ATTR_T attr_t = AIR_VLAN_PORT_ATTR_USER_PORT;

	port = _strtoul(argv[3], NULL, 10);
	attr = _strtoul(argv[4], NULL, 10);

	printf("port: %x, attr: %x\n", port, attr);

	/*Check the input parameters is right or not. */
	if (port > SWITCH_MAX_PORT || attr > 3) {
		printf(HELP_VLAN_PORT_ATTR);
		return;
	}

	attr_t = (AIR_VLAN_PORT_ATTR_T) attr;
	air_vlan_setPortAttr(0, port, attr_t);
}

void an8855_doVlanSetPortMode(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char mode = 0;
	AIR_PORT_VLAN_MODE_T mode_t = AIR_PORT_VLAN_MODE_PORT_MATRIX;

	port = _strtoul(argv[3], NULL, 10);
	mode = _strtoul(argv[4], NULL, 10);
	printf("port: %x, mode: %x\n", port, mode);

	/*Check the input parameters is right or not. */
	if (port > SWITCH_MAX_PORT || mode > 3) {
		printf(HELP_VLAN_PORT_MODE);
		return;
	}

	mode_t = (AIR_PORT_VLAN_MODE_T) mode;
	air_port_setVlanMode(0, port, mode_t);
}

void an8855_doVlanSetEgressTagPCR(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char eg_tag = 0;
	AIR_PORT_EGS_TAG_ATTR_T eg_tag_t = AIR_PORT_EGS_TAG_ATTR_UNTAGGED;

	port = _strtoul(argv[3], NULL, 10);
	eg_tag = _strtoul(argv[4], NULL, 10);

	printf("port: %d, eg_tag: %d\n", port, eg_tag);

	/*Check the input parameters is right or not. */
	if ((port > SWITCH_MAX_PORT) || (eg_tag > REG_PCR_EG_TAG_RELMASK)) {
		printf(HELP_VLAN_EGRESS_TAG_PCR);
		return;
	}

	eg_tag_t = (AIR_PORT_EGS_TAG_ATTR_T) eg_tag;
	air_vlan_setPortEgsTagAttr(0, port, eg_tag_t);
}				/*end doVlanSetEgressTagPCR */

void an8855_doVlanSetEgressTagPVC(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char eg_tag = 0;
	AIR_IGR_PORT_EG_TAG_ATTR_T eg_tag_t = 0;

	port = _strtoul(argv[3], NULL, 10);
	eg_tag = _strtoul(argv[4], NULL, 10);

	printf("port: %d, eg_tag: %d\n", port, eg_tag);

	/*Check the input parameters is right or not. */
	if ((port > SWITCH_MAX_PORT) || (eg_tag > REG_PVC_EG_TAG_RELMASK)) {
		printf(HELP_VLAN_EGRESS_TAG_PVC);
		return;
	}

	eg_tag_t = (AIR_IGR_PORT_EG_TAG_ATTR_T) eg_tag;
	air_vlan_setIgrPortTagAttr(0, port, eg_tag_t);
}				/*end doVlanSetEgressTagPVC */

void an8855_doArlAging(int argc, char *argv[])
{
	unsigned char aging_en = 0;
	unsigned int time = 0, port = 0;

	aging_en = _strtoul(argv[3], NULL, 10);
	time = _strtoul(argv[4], NULL, 10);
	printf("aging_en: %x, aging time: %x\n", aging_en, time);

	/*Check the input parameters is right or not. */
	if ((aging_en != 0 && aging_en != 1) || (time <= 0 || time > 65536)) {
		printf(HELP_ARL_AGING);
		return;
	}

	for (port = 0; port < 6; port++)
		air_l2_setAgeEnable(0, port, aging_en);

	air_l2_setMacAddrAgeOut(0, time);
}

void an8855_doMirrorEn(int argc, char *argv[])
{
	unsigned char mirror_en = 0;
	unsigned char mirror_port = 0;
	AIR_MIR_SESSION_T session = { 0 };

	mirror_en = _strtoul(argv[3], NULL, 10);
	mirror_port = _strtoul(argv[4], NULL, 10);

	printf("mirror_en: %d, mirror_port: %d\n", mirror_en, mirror_port);

	/*Check the input parameters is right or not. */
	if ((mirror_en > 1) || (mirror_port > REG_CFC_MIRROR_PORT_RELMASK)) {
		printf(HELP_MIRROR_EN);
		return;
	}

	memset(&session, 0, sizeof(AIR_MIR_SESSION_T));

	if (mirror_en) {
		session.dst_port = mirror_port;
		session.flags |= AIR_MIR_SESSION_FLAGS_ENABLE;
		air_mir_addSession(0, 0, &session);
	} else
		air_mir_delSession(0, 0);

	air_mir_setSessionAdminMode(0, 0, (int)mirror_en);
}				/*end doMirrorEn */

void an8855_doMirrorPortBased(int argc, char *argv[])
{
	unsigned char port = 0, port_tx_mir = 0, port_rx_mir = 0, vlan_mis =
	    0, acl_mir = 0, igmp_mir = 0;
	unsigned int reg = 0, value = 0;
	AIR_MIR_SESSION_T session = { 0 };

	port = _strtoul(argv[3], NULL, 10);
	port_tx_mir = _strtoul(argv[4], NULL, 10);
	port_rx_mir = _strtoul(argv[5], NULL, 10);
	acl_mir = _strtoul(argv[6], NULL, 10);
	vlan_mis = _strtoul(argv[7], NULL, 10);
	igmp_mir = _strtoul(argv[8], NULL, 10);

	printf("port:%d, port_tx_mir:%d, port_rx_mir:%d, ",
		port, port_tx_mir, port_rx_mir);
	printf("acl_mir:%d, vlan_mis:%d, igmp_mir:%d\n",
		acl_mir, vlan_mis, igmp_mir);

	/*Check the input parameters is right or not. */
	if ((port >= SWITCH_MAX_PORT) || (port_tx_mir > 1) || (port_rx_mir > 1)
		|| (acl_mir > 1) || (vlan_mis > 1)) {	// also allow CPU port (port6)
		printf(HELP_MIRROR_PORTBASED);
		return;
	}

	memset(&session, 0, sizeof(AIR_MIR_SESSION_T));
	air_mir_getSession(0, 0, &session);
	session.src_port = port;

	if (port_tx_mir)
		session.flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
	else
		session.flags &= ~AIR_MIR_SESSION_FLAGS_DIR_TX;

	if (port_rx_mir)
		session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
	else
		session.flags &= ~AIR_MIR_SESSION_FLAGS_DIR_RX;

	air_mir_setMirrorPort(0, 0, &session);

	/* not support acl/vlan/igmp mismatch */
}				/*end doMirrorPortBased */

void an8855_doStp(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char fid = 0;
	unsigned char state = 0;
	unsigned int value = 0;
	unsigned int reg = 0;

	port = _strtoul(argv[2], NULL, 10);
	fid = _strtoul(argv[3], NULL, 10);
	state = _strtoul(argv[4], NULL, 10);

	printf("port: %d, fid: %d, state: %d\n", port, fid, state);

	/*Check the input parameters is right or not. */
	if ((port > 5) || (fid > 16) || (state > 3)) {
		printf(HELP_STP);
		return;
	}

	air_stp_setPortstate(0, port, fid, state);
}

void _an8855_ingress_rate_set(int on_off, unsigned char port, unsigned int bw)
{
	AIR_ERROR_NO_T ret = AIR_E_OK;
	AIR_QOS_RATE_LIMIT_CFG_T rl = { 0 };

	if (on_off) {
		ret =
		    air_qos_setRateLimitEnable(0, port,
					       AIR_QOS_RATE_DIR_INGRESS, TRUE);
		if (ret != AIR_E_OK) {
			printf("an8855 set ingress ratelimit eanble fail\n");
			return;
		}
		ret = air_qos_getRateLimit(0, port, &rl);
		if (ret != AIR_E_OK) {
			printf("an8855 get port %d ratelimit fail\n",
				port);
			return;
		}
		rl.ingress_cir = bw;
		rl.flags |= AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_INGRESS;
		ret = air_qos_setRateLimit(0, port, &rl);
		if (ret != AIR_E_OK) {
			printf("an8855 set ingress ratelimit value %d fail\n",
			       bw);
			return;
		}
		printf("an8855 set ingress ratelimit value %d ok\n", bw);
	} else {
		ret =
		    air_qos_setRateLimitEnable(0, port,
					       AIR_QOS_RATE_DIR_INGRESS, FALSE);
		if (ret != AIR_E_OK) {
			printf("an8855 set ingress ratelimit disable fail\n");
			return;
		}
		printf("an8855 set ingress ratelimit disable ok\n");

	}
}

void _an8855_egress_rate_set(int on_off, unsigned char port, unsigned int bw)
{
	AIR_ERROR_NO_T ret = AIR_E_OK;
	AIR_QOS_RATE_LIMIT_CFG_T rl = { 0 };

	if (on_off) {
		ret =
		    air_qos_setRateLimitEnable(0, port, AIR_QOS_RATE_DIR_EGRESS,
					       TRUE);
		if (ret != AIR_E_OK) {
			printf("an8855 set egress ratelimit eanble fail\n");
			return;
		}
		ret = air_qos_getRateLimit(0, port, &rl);
		if (ret != AIR_E_OK) {
			printf("an8855 get port %d ratelimit fail\n",
				port);
			return;
		}
		rl.egress_cir = bw;
		rl.flags |= AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_EGRESS;
		ret = air_qos_setRateLimit(0, port, &rl);
		if (ret != AIR_E_OK) {
			printf("an8855 set egress ratelimit value %d fail\n",
			       bw);
			return;
		}
		printf("an8855 set egress ratelimit value %d ok\n", bw);

	} else {
		ret =
		    air_qos_setRateLimitEnable(0, port, AIR_QOS_RATE_DIR_EGRESS,
					       FALSE);
		if (ret != AIR_E_OK) {
			printf("an8855 set egress ratelimit disable fail\n");
			return;
		}
		printf("an8855 set egress ratelimit disable ok\n");

	}
}

void an8855_ingress_rate_set(int argc, char *argv[])
{
	int on_off = 0, port, bw = 0;

	port = _strtoul(argv[3], NULL, 0);
	if (argv[2][1] == 'n') {
		bw = _strtoul(argv[4], NULL, 0);
		on_off = 1;
	} else if (argv[2][1] == 'f') {
		if (argc != 4)
			return;

		on_off = 0;
	}

	_an8855_ingress_rate_set(on_off, port, bw);
}

void an8855_egress_rate_set(int argc, char *argv[])
{
	unsigned int reg = 0, value = 0;
	int on_off = 0, port = 0, bw = 0;

	port = _strtoul(argv[3], NULL, 0);
	if (argv[2][1] == 'n') {
		bw = _strtoul(argv[4], NULL, 0);
		on_off = 1;
	} else if (argv[2][1] == 'f') {
		if (argc != 4)
			return;

		on_off = 0;
	}

	_an8855_egress_rate_set(on_off, port, bw);
}

void an8855_rate_control(int argc, char *argv[])
{
	unsigned char dir = 0;
	unsigned char port = 0;
	unsigned int rate_cir = 0;

	dir = _strtoul(argv[2], NULL, 10);
	port = _strtoul(argv[3], NULL, 10);
	rate_cir = _strtoul(argv[4], NULL, 10);

	if (port > 5) {
		printf("Error, port %d is bigger than 5\n\r", port);
		return;
	}
	if (rate_cir > 80000) {
		printf("Error, rate_cir %d is bigger than 80000\n\r", rate_cir);
		return;
	}

	if (dir == 1)		//ingress
		_an8855_ingress_rate_set(1, port, rate_cir);
	else if (dir == 0)	//egress
		_an8855_egress_rate_set(1, port, rate_cir);
	else
		printf("Error, dir %d is not 1(ingress) and 0(egress)\n\r",
		       dir);
}

void an8855_read_output_queue_counters(int argc, char *argv[])
{
	unsigned int port = 0;
	unsigned int value = 0, output_queue = 0;

	for (port = 0; port < 7; port++) {
		reg_write(0x10207e48, 0x80000000 | (port << 8) | 0x0);
		reg_read(0x10207e4c, &output_queue);
		printf("\n port %d  output queue 0 counter is %d.\n", port,
		       output_queue);
		reg_write(0x10207e48, 0x80000000 | (port << 8) | 0x1);
		reg_read(0x10207e4c, &output_queue);
		printf("\n port %d  output queue 1 counter is %d.\n", port,
		       output_queue);
		reg_write(0x10207e48, 0x80000000 | (port << 8) | 0x2);
		reg_read(0x10207e4c, &output_queue);
		printf("\n port %d  output queue 2 counter is %d.\n", port,
		       output_queue);
		reg_write(0x10207e48, 0x80000000 | (port << 8) | 0x3);
		reg_read(0x10207e4c, &output_queue);
		printf("\n port %d  output queue 3 counter is %d.\n", port,
		       output_queue);
		reg_write(0x10207e48, 0x80000000 | (port << 8) | 0x4);
		reg_read(0x10207e4c, &output_queue);
		printf("\n port %d  output queue 4 counter is %d.\n", port,
		       output_queue);
		reg_write(0x10207e48, 0x80000000 | (port << 8) | 0x5);
		reg_read(0x10207e4c, &output_queue);
		printf("\n port %d  output queue 5 counter is %d.\n", port,
		       output_queue);
		reg_write(0x10207e48, 0x80000000 | (port << 8) | 0x6);
		reg_read(0x10207e4c, &output_queue);
		printf("\n port %d  output queue 6 counter is %d.\n", port,
		       output_queue);
		reg_write(0x10207e48, 0x80000000 | (port << 8) | 0x7);
		reg_read(0x10207e4c, &output_queue);
		printf("\n port %d  output queue 7 counter is %d.\n", port,
		       output_queue);
	}
}

void an8855_read_free_page_counters(int argc, char *argv[])
{
	unsigned int value = 0;
	unsigned int free_page = 0, free_page_min = 0;
	unsigned int fc_free_blk_lothd = 0, fc_free_blk_hithd = 0;
	unsigned int fc_port_blk_thd = 0, fc_port_blk_hi_thd = 0;
	unsigned int queue[8] = { 0 };

	/* get system free page link counter */
	reg_read(0x10207e00, &value);
	free_page = value & 0xFFF;
	free_page_min = (value & 0xFFF0000) >> 16;

	/* get system flow control waterwark */
	reg_read(0x10207e04, &value);
	fc_free_blk_lothd = value & 0x3FF;
	fc_free_blk_hithd = (value & 0x1FF8000) >> 15;

	/* get port flow control waterwark */
	reg_read(0x10207e08, &value);
	fc_port_blk_thd = (value & 0xFF00) >> 8;
	fc_port_blk_hi_thd = (value & 0xFF0000) >> 16;

	/* get queue flow control waterwark */
	reg_read(0x10207e10, &value);
	queue[0] = value & 0x3F;
	queue[1] = (value & 0x3F00) >> 8;
	queue[2] = (value & 0x3F0000) >> 16;
	queue[3] = (value & 0x3F000000) >> 24;
	reg_read(0x10207e0c, &value);
	queue[4] = value & 0x3F;
	queue[5] = (value & 0x3F00) >> 8;
	queue[6] = (value & 0x3F0000) >> 16;
	queue[7] = (value & 0x3F000000) >> 24;

	printf("<===Free Page=======Current============Minimal=========>\n");
	printf("\n");
	printf(" page counter      %u                %u\n",
	       free_page, free_page_min);
	printf("\n");
	printf("=========================================================\n");
	printf("<===Type=======High threshold======Low threshold=========\n");
	printf("\n");
	printf("  system:         %u                 %u\n",
	       fc_free_blk_hithd * 2, fc_free_blk_lothd * 2);
	printf("    port:         %u                 %u\n",
	       fc_port_blk_hi_thd * 2, fc_port_blk_thd * 2);
	printf(" queue 0:         %u                 NA\n",
	       queue[0]);
	printf(" queue 1:         %u                 NA\n",
	       queue[1]);
	printf(" queue 2:         %u                 NA\n",
	       queue[2]);
	printf(" queue 3:         %u                 NA\n",
	       queue[3]);
	printf(" queue 4:         %u                 NA\n",
	       queue[4]);
	printf(" queue 5:         %u                 NA\n",
	       queue[5]);
	printf(" queue 6:         %u                 NA\n",
	       queue[6]);
	printf(" queue 7:         %u                 NA\n",
	       queue[7]);
	printf("=========================================================\n");
}

void an8855_eee_enable(int argc, char *argv[])
{
	unsigned long enable = 0;
	unsigned int value, mode = 0;
	unsigned int eee_cap = 0;
	unsigned int eee_en_bitmap = 0;
	unsigned long port_map = 0;
	long port_num = -1;
	int p = 0;

	if (argc < 3)
		goto error;

	/* Check the input parameters is right or not. */
	if (!strncmp(argv[2], "enable", 7))
		enable = 1;
	else if (!strncmp(argv[2], "disable", 8))
		enable = 0;
	else
		goto error;

	if (argc > 3) {
		if (strlen(argv[3]) == 1) {
			port_num = strtol(argv[3], (char **)NULL, 10);
			if (port_num < 0 || port_num > MAX_PHY_PORT - 1) {
				printf("Illegal port index and port:0~4\n");
				goto error;
			}
			port_map = 1 << port_num;
		} else if (strlen(argv[3]) == 5) {
			port_map = 0;
			for (p = 0; p < MAX_PHY_PORT; p++) {
				if (argv[3][p] != '0' && argv[3][p] != '1') {
					printf("portmap format error, ");
					printf("should be combination of 0 or 1\n");
					goto error;
				}
				port_map |= ((argv[3][p] - '0') << p);
			}
		} else {
			printf
			    ("port_no or portmap format error, should be length of 1 or 5\n");
			goto error;
		}
	} else {
		port_map = 0x1f;
	}

	for (port_num = 0; port_num < MAX_PHY_PORT; port_num++) {
		if (port_map & (1 << port_num)) {
			air_port_getPsMode(0, port_num, &mode);
			if (enable)
				mode |= AIR_PORT_PS_EEE;
			else
				mode &= ~AIR_PORT_PS_EEE;

			air_port_setPsMode(0, port_num, mode);
		}
	}
	return;

error:
	printf(HELP_EEE_EN);
}

void an8855_eee_dump(int argc, char *argv[])
{
	unsigned int cap = 0, lp_cap = 0;
	long port = -1;
	int p = 0;

	if (argc > 3) {
		if (strlen(argv[3]) > 1) {
			printf("port# format error, should be of length 1\n");
			return;
		}

		port = strtol(argv[3], (char **)NULL, 0);
		if (port < 0 || port > MAX_PHY_PORT) {
			printf("port# format error, should be 0 to %d\n",
			       MAX_PHY_PORT);
			return;
		}
	}

	for (p = 0; p < MAX_PHY_PORT; p++) {
		if (port >= 0 && p != port)
			continue;

		mii_mgr_c45_read(p, 0x7, 0x3c, &cap);
		mii_mgr_c45_read(p, 0x7, 0x3d, &lp_cap);
		printf("port%d EEE cap=0x%02x, link partner EEE cap=0x%02x",
		       p, cap, lp_cap);

		if (port >= 0 && p == port) {
			mii_mgr_c45_read(p, 0x3, 0x1, &cap);
			printf(", st=0x%03x", cap);
		}
		printf("\n");
	}
}

void an8855_read_mib_counters(int argc, char *argv[])
{
	int port = 0;
	AIR_MIB_CNT_RX_T rx_mib[7];
	AIR_MIB_CNT_TX_T tx_mib[7];

	printf("======================== %8s %8s %8s %8s %8s %8s %8s\n",
	       "Port0", "Port1", "Port2", "Port3", "Port4", "Port5", "Port6");

	for (port = 0; port < 7; port++)
		air_mib_get(0, port, &rx_mib[port], &tx_mib[port]);

	printf("Tx Drop Packet      :");
	for (port = 0; port < 7; port++)
		printf("%8u ", tx_mib[port].TDPC);

	printf("\n");
	printf("Tx CRC Error        :");
	for (port = 0; port < 7; port++)
		printf("%8u ", tx_mib[port].TCRC);

	printf("\n");
	printf("Tx Unicast Packet   :");
	for (port = 0; port < 7; port++)
		printf("%8u ", tx_mib[port].TUPC);

	printf("\n");
	printf("Tx Multicast Packet :");
	for (port = 0; port < 7; port++)
		printf("%8u ", tx_mib[port].TMPC);

	printf("\n");
	printf("Tx Broadcast Packet :");
	for (port = 0; port < 7; port++)
		printf("%8u ", tx_mib[port].TBPC);

	printf("\n");
	printf("Tx Collision Event  :");
	for (port = 0; port < 7; port++)
		printf("%8u ", tx_mib[port].TCEC);

	printf("\n");
	printf("Tx Pause Packet     :");
	for (port = 0; port < 7; port++)
		printf("%8u ", tx_mib[port].TPPC);

	printf("\n");
	printf("Rx Drop Packet      :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RDPC);

	printf("\n");
	printf("Rx Filtering Packet :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RFPC);

	printf("\n");
	printf("Rx Unicast Packet   :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RUPC);

	printf("\n");
	printf("Rx Multicast Packet :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RMPC);

	printf("\n");
	printf("Rx Broadcast Packet :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RBPC);

	printf("\n");
	printf("Rx Alignment Error  :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RAEPC);

	printf("\n");
	printf("Rx CRC Error	    :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RCEPC);

	printf("\n");
	printf("Rx Undersize Error  :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RUSPC);

	printf("\n");
	printf("Rx Fragment Error   :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RFEPC);

	printf("\n");
	printf("Rx Oversize Error   :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].ROSPC);

	printf("\n");
	printf("Rx Jabber Error     :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RJEPC);

	printf("\n");
	printf("Rx Pause Packet     :");
	for (port = 0; port < 7; port++)
		printf("%8u ", rx_mib[port].RPPC);

	printf("\n");
}

void an8855_clear_mib_counters(int argc, char *argv[])
{
	air_mib_clear(0);
}

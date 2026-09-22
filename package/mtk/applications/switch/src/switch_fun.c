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
#include <errno.h>

#include "switch_extend.h"
#include "switch_netlink.h"
#include "switch_ioctl.h"
#include "switch_fun.h"

#define leaky_bucket 0

#define MAC_ENTRY_LIVE_STATUS		2
#define MAC_ENTRY_LIVE_STATUS_MASK	0x3

enum mac_live_status {
	MAC_ENTRY_EMPTY = 0,
	MAC_ENTRY_DYNAMIC_VALID,
	MAC_ENTRY_RESERVED,
	MAC_ENTRY_STATIC
};

struct switch_func_s mt753x_switch_func = {
	.pf_table_dump = table_dump,
	.pf_table_clear = table_clear,
	.pf_switch_reset = switch_reset,
	.pf_doArlAging = doArlAging,
	.pf_read_mib_counters = read_mib_counters,
	.pf_clear_mib_counters = clear_mib_counters,
	.pf_read_output_queue_counters = read_output_queue_counters,
	.pf_read_free_page_counters = read_free_page_counters,
	.pf_rate_control = rate_control,
	.pf_igress_rate_set = ingress_rate_set,
	.pf_egress_rate_set = egress_rate_set,
	.pf_table_add = table_add,
	.pf_table_del_fid = table_del_fid,
	.pf_table_del_vid = table_del_vid,
	.pf_table_search_mac_fid = table_search_mac_fid,
	.pf_table_search_mac_vid = table_search_mac_vid,
	.pf_global_set_mac_fc = global_set_mac_fc,
	.pf_set_mac_pfc = set_mac_pfc,
	.pf_qos_sch_select = qos_sch_select,
	.pf_qos_set_base = qos_set_base,
	.pf_qos_wfq_set_weight = qos_wfq_set_weight,
	.pf_qos_set_portpri = qos_set_portpri,
	.pf_qos_set_dscppri = qos_set_dscppri,
	.pf_qos_pri_mapping_queue = qos_pri_mapping_queue,
	.pf_doStp = doStp,
	.pf_sip_dump = sip_dump,
	.pf_sip_add = sip_add,
	.pf_sip_del = sip_del,
	.pf_sip_clear = sip_clear,
	.pf_dip_dump = dip_dump,
	.pf_dip_add = dip_add,
	.pf_dip_del = dip_del,
	.pf_dip_clear = dip_clear,
	.pf_set_mirror_to = set_mirror_to,
	.pf_set_mirror_from = set_mirror_from,
	.pf_doMirrorEn = doMirrorEn,
	.pf_doMirrorPortBased = doMirrorPortBased,
	.pf_acl_dip_add = acl_dip_add,
	.pf_acl_dip_modify = acl_dip_modify,
	.pf_acl_dip_pppoe = acl_dip_pppoe,
	.pf_acl_dip_trtcm = acl_dip_trtcm,
	.pf_acl_dip_meter = acl_dip_meter,
	.pf_acl_mac_add = acl_mac_add,
	.pf_acl_ethertype = acl_ethertype,
	.pf_acl_sp_add = acl_sp_add,
	.pf_acl_l4_add = acl_l4_add,
	.pf_acl_port_enable = acl_port_enable,
	.pf_acl_table_add = acl_table_add,
	.pf_acl_mask_table_add = acl_mask_table_add,
	.pf_acl_rule_table_add = acl_rule_table_add,
	.pf_acl_rate_table_add = acl_rate_table_add,
	.pf_vlan_dump = vlan_dump,
	.pf_vlan_set = vlan_set,
	.pf_vlan_clear = vlan_clear,
	.pf_doVlanSetVid = doVlanSetVid,
	.pf_doVlanSetPvid = doVlanSetPvid,
	.pf_doVlanSetAccFrm = doVlanSetAccFrm,
	.pf_doVlanSetPortAttr = doVlanSetPortAttr,
	.pf_doVlanSetPortMode = doVlanSetPortMode,
	.pf_doVlanSetEgressTagPCR = doVlanSetEgressTagPCR,
	.pf_doVlanSetEgressTagPVC = doVlanSetEgressTagPVC,
	.pf_igmp_on = igmp_on,
	.pf_igmp_off = igmp_off,
	.pf_igmp_enable = igmp_enable,
	.pf_igmp_disable = igmp_disable,
	.pf_collision_pool_enable = collision_pool_enable,
	.pf_collision_pool_mac_dump = collision_pool_mac_dump,
	.pf_collision_pool_dip_dump = collision_pool_dip_dump,
	.pf_collision_pool_sip_dump = collision_pool_sip_dump,
	.pf_pfc_get_rx_counter = pfc_get_rx_counter,
	.pf_pfc_get_tx_counter = pfc_get_tx_counter,
	.pf_eee_enable = eee_enable,
	.pf_eee_dump = eee_dump,
};

static int getnext(char *src, int separator, char *dest)
{
	char *c;
	int len;

	if ((src == NULL) || (dest == NULL))
		return -1;

	c = strchr(src, separator);
	if (c == NULL)
		return -1;

	len = c - src;
	strncpy(dest, src, len);
	dest[len] = '\0';
	return len + 1;
}

static int str_to_ip(unsigned int *ip, char *str)
{
	int i;
	int len;
	char *ptr = str;
	char buf[128];
	unsigned char c[4];

	for (i = 0; i < 3; ++i) {
		if ((len = getnext(ptr, '.', buf)) == -1)
			return 1;
		c[i] = atoi(buf);
		ptr += len;
	}
	c[3] = atoi(ptr);
	*ip = (c[0] << 24) + (c[1] << 16) + (c[2] << 8) + c[3];
	return 0;
}

/*convert IP address from number to string */
static void ip_to_str(char *str, size_t size, unsigned int ip)
{
	unsigned char *ptr = (unsigned char *)&ip;
	unsigned char c[4];
	int ret;

	if (str == NULL || size == 0) {
		printf("convert IP address failed\n");
		return;
	}

	c[0] = *(ptr);
	c[1] = *(ptr + 1);
	c[2] = *(ptr + 2);
	c[3] = *(ptr + 3);

	ret = snprintf(str, size, "%d.%d.%d.%d", c[3], c[2], c[1], c[0]);
	if (ret < 0) {
		printf("Encoding error in snprintf\n");
		return;
	} else if ((size_t)ret >= size) {
		printf("Required size %d, provided size %zu\n", ret, size);
		return;
	}
}

int reg_read(unsigned int offset, unsigned int *value)
{
	int ret = -1;

	if (nl_init_flag == true) {
		ret = reg_read_netlink(attres, offset, value);
	} else {
		if (attres->dev_id == -1)
			ret = reg_read_ioctl(offset, value);
	}
	if (ret < 0) {
		printf("Read fail\n");
		*value = 0;
		return ret;
	}

	return 0;
}

int reg_write(unsigned int offset, unsigned int value)
{
	int ret = -1;

	if (nl_init_flag == true) {
		ret = reg_write_netlink(attres, offset, value);
	} else {
		if (attres->dev_id == -1)
			ret = reg_write_ioctl(offset, value);
	}
	if (ret < 0) {
		printf("Write fail\n");
		exit_free();
		exit(0);
	}
	return 0;
}

int mii_mgr_read(unsigned int port_num, unsigned int reg, unsigned int *value)
{
	int ret;

	if (port_num > 31) {
		printf("Invalid Port or PHY addr\n");
		return -1;
	}

	if (nl_init_flag == true)
		ret = phy_cl22_read_netlink(attres, port_num, reg, value);
	else
		ret = mii_mgr_cl22_read_ioctl(port_num, reg, value);

	if (ret < 0) {
		printf("Phy cl22 read fail\n");
		exit_free();
		exit(0);
	}

	return 0;
}

int mii_mgr_write(unsigned int port_num, unsigned int reg, unsigned int value)
{
	int ret;

	if (port_num > 31) {
		printf("Invalid Port or PHY addr\n");
		return -1;
	}

	if (nl_init_flag == true)
		ret = phy_cl22_write_netlink(attres, port_num, reg, value);
	else
		ret = mii_mgr_cl22_write_ioctl(port_num, reg, value);

	if (ret < 0) {
		printf("Phy cl22 write fail\n");
		exit_free();
		exit(0);
	}

	return 0;
}

int mii_mgr_c45_read(unsigned int port_num, unsigned int dev, unsigned int reg,
		     unsigned int *value)
{
	int ret;

	if (port_num > 31) {
		printf("Invalid Port or PHY addr\n");
		return -1;
	}

	if (nl_init_flag == true)
		ret = phy_cl45_read_netlink(attres, port_num, dev, reg, value);
	else
		ret = mii_mgr_cl45_read_ioctl(port_num, dev, reg, value);

	if (ret < 0) {
		printf("Phy cl45 read fail\n");
		exit_free();
		exit(0);
	}

	return 0;
}

int mii_mgr_c45_write(unsigned int port_num, unsigned int dev, unsigned int reg,
		      unsigned int value)
{
	int ret;

	if (port_num > 31) {
		printf("Invalid Port or PHY addr\n");
		return -1;
	}

	if (nl_init_flag == true)
		ret = phy_cl45_write_netlink(attres, port_num, dev, reg, value);
	else
		ret = mii_mgr_cl45_write_ioctl(port_num, dev, reg, value);

	if (ret < 0) {
		printf("Phy cl45 write fail\n");
		exit_free();
		exit(0);
	}

	return 0;
}

int phy_dump(int phy_addr)
{
	int ret;

	if (nl_init_flag == true)
		ret = phy_dump_netlink(attres, phy_addr);
	else
		ret = phy_dump_ioctl(phy_addr);

	if (ret < 0) {
		printf("Phy dump fail\n");
		exit_free();
		exit(0);
	}

	return 0;
}

void phy_crossover(int argc, char *argv[])
{
	unsigned int port_num = strtoul(argv[2], NULL, 10);
	unsigned int value = 0;
	int ret;

	if (port_num > 4) {
		printf("invaild value, port_name:0~4\n");
		return;
	}

	if (nl_init_flag == true)
		ret =
		    phy_cl45_read_netlink(attres, port_num, 0x1E,
					  MT7530_T10_TEST_CONTROL, &value);
	else
		ret =
		    mii_mgr_cl45_read_ioctl(port_num, 0x1E,
					    MT7530_T10_TEST_CONTROL, &value);
	if (ret < 0) {
		printf("phy_cl45 read fail\n");
		exit_free();
		exit(0);
	}

	printf("mii_mgr_cl45:");
	printf("Read:  port#=%d, device=0x%x, reg=0x%x, value=0x%x\n",
			port_num, 0x1E, MT7530_T10_TEST_CONTROL, value);

	if (!strncmp(argv[3], "auto", 5)) {
		value &= (~(0x3 << 3));
	} else if (!strncmp(argv[3], "mdi", 4)) {
		value &= (~(0x3 << 3));
		value |= (0x2 << 3);
	} else if (!strncmp(argv[3], "mdix", 5)) {
		value |= (0x3 << 3);
	} else {
		printf("invaild parameter\n");
		return;
	}
	printf("Write: port#=%d, device=0x%x, reg=0x%x. value=0x%x\n",
		port_num, 0x1E, MT7530_T10_TEST_CONTROL, value);

	if (nl_init_flag == true)
		ret =
		    phy_cl45_write_netlink(attres, port_num, 0x1E,
					   MT7530_T10_TEST_CONTROL, value);
	else
		ret =
		    mii_mgr_cl45_write_ioctl(port_num, 0x1E,
					     MT7530_T10_TEST_CONTROL, value);

	if (ret < 0) {
		printf("phy_cl45 write fail\n");
		exit_free();
		exit(0);
	}
}

int rw_phy_token_ring(int argc, char *argv[])
{
	int ch_addr, node_addr, data_addr;
	unsigned int tr_reg_control;
	unsigned int val_l = 0;
	unsigned int val_h = 0;
	unsigned int port_num;
	char *endptr;

	if (argc < 4)
		return -1;

	if (argv[2][0] == 'r') {
		if (argc != 7)
			return -1;

		errno = 0;
		port_num = strtoul(argv[3], &endptr, 10);
		if (errno != 0 || *endptr != '\0' || port_num > MAX_PORT) {
			printf("Illegal port index and port:0~6\n");
			return -1;
		}
		mii_mgr_write(port_num, 0x1f, 0x52b5);	// r31 = 0x52b5

		errno = 0;
		ch_addr = strtoul(argv[4], &endptr, 10);
		if (errno != 0 || *endptr != '\0')
			goto error;

		errno = 0;
		node_addr = strtoul(argv[5], &endptr, 16);
		if (errno != 0 || *endptr != '\0')
			goto error;

		errno = 0;
		data_addr = strtoul(argv[6], &endptr, 16);
		if (errno != 0 || *endptr != '\0')
			goto error;

		printf("port = %x, ch_addr = %x, node_addr=%x, data_addr=%x\n",
				port_num, ch_addr, node_addr, data_addr);
		tr_reg_control =
		    (1 << 15) | (1 << 13) | (ch_addr << 11) | (node_addr << 7) |
		    (data_addr << 1);
		mii_mgr_write(port_num, 16, tr_reg_control);	// r16 = tr_reg_control
		mii_mgr_read(port_num, 17, &val_l);
		mii_mgr_read(port_num, 18, &val_h);
		printf("switch trreg read tr_reg_control=%x, value_H=%x, value_L=%x\n",
				tr_reg_control, val_h, val_l);
	} else if (argv[2][0] == 'w') {
		if (argc != 9)
			return -1;

		errno = 0;
		port_num = strtoul(argv[3], &endptr, 10);
		if (errno != 0 || *endptr != '\0' || port_num > MAX_PORT) {
			printf("Illegal port index and port:0~6\n");
			return -1;
		}
		mii_mgr_write(port_num, 0x1f, 0x52b5);	// r31 = 0x52b5

		errno = 0;
		ch_addr = strtoul(argv[4], &endptr, 10);
		if (errno != 0 || *endptr != '\0')
			goto error;

		errno = 0;
		node_addr = strtoul(argv[5], &endptr, 16);
		if (errno != 0 || *endptr != '\0')
			goto error;

		errno = 0;
		data_addr = strtoul(argv[6], &endptr, 16);
		if (errno != 0 || *endptr != '\0')
			goto error;

		errno = 0;
		val_h = strtoul(argv[7], &endptr, 16);
		if (errno != 0 || *endptr != '\0')
			goto error;

		errno = 0;
		val_l = strtoul(argv[8], &endptr, 16);
		if (errno != 0 || *endptr != '\0')
			goto error;

		printf("port = %x, ch_addr = %x, node_addr=%x, data_addr=%x\n",
				port_num, ch_addr, node_addr, data_addr);
		tr_reg_control =
		    (1 << 15) | (0 << 13) | (ch_addr << 11) | (node_addr << 7) |
		    (data_addr << 1);
		mii_mgr_write(port_num, 17, val_l);
		mii_mgr_write(port_num, 18, val_h);
		mii_mgr_write(port_num, 16, tr_reg_control);	// r16 = tr_reg_control
		printf("switch trreg Write tr_reg_control=%x, value_H=%x, value_L=%x\n",
				tr_reg_control, val_h, val_l);
	} else
		return -1;

	mii_mgr_write(port_num, 0x1f, 0x0); // switch back to page 0

	return 0;

error:
	printf("\n**Illegal parameters\n");
	return -1;
}

void write_acl_table(unsigned char tbl_idx, unsigned int vawd1,
		     unsigned int vawd2)
{
	unsigned int value = 0, reg = 0;
	unsigned int max_index;

	if (chip_name == 0x7531 || chip_name == 0x7988)
		max_index = 256;
	else
		max_index = 64;

	printf("Pattern_acl_tbl_idx:%d\n", tbl_idx);

	if (tbl_idx >= max_index) {
		printf(HELP_ACL_ACL_TBL_ADD);
		return;
	}

	reg = REG_VTCR_ADDR;
	while (1) {		// wait until not busy
		reg_read(reg, &value);
		if ((value & REG_VTCR_BUSY_MASK) == 0)
			break;
	}
	reg_write(REG_VAWD1_ADDR, vawd1);
	printf("write reg: %x, value: %x\n", REG_VAWD1_ADDR, vawd1);
	reg_write(REG_VAWD2_ADDR, vawd2);
	printf("write reg: %x, value: %x\n", REG_VAWD2_ADDR, vawd2);
	reg = REG_VTCR_ADDR;
	value = REG_VTCR_BUSY_MASK | (0x05 << REG_VTCR_FUNC_OFFT) | tbl_idx;
	reg_write(reg, value);
	printf("write reg: %x, value: %x\n", reg, value);

	while (1) {		// wait until not busy
		reg_read(reg, &value);
		if ((value & REG_VTCR_BUSY_MASK) == 0)
			break;
	}
}

void acl_table_add(int argc, char *argv[])
{
	unsigned int vawd1 = 0, vawd2 = 0;
	unsigned char tbl_idx = 0;
	unsigned int max_index = 0;
	char *endptr;

	if (chip_name == 0x7531 || chip_name == 0x7988)
		max_index = 256;
	else
		max_index = 64;

	errno = 0;
	tbl_idx = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || tbl_idx >= max_index) {
		printf("Error: wrong ACL rule table index\n");
		return;
	}

	errno = 0;
	vawd1 = strtoul(argv[4], &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: wrong ACL rule table write data 1\n");
		return;
	}

	errno = 0;
	vawd2 = strtoul(argv[5], &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: wrong ACL rule table write data 2\n");
		return;
	}

	write_acl_table(tbl_idx, vawd1, vawd2);
}

void write_acl_mask_table(unsigned char tbl_idx, unsigned int vawd1,
			  unsigned int vawd2)
{
	unsigned int value = 0, reg = 0;
	unsigned int max_index = 0;

	if (chip_name == 0x7531 || chip_name == 0x7988)
		max_index = 128;
	else
		max_index = 32;

	printf("Rule_mask_tbl_idx:%d\n", tbl_idx);

	if (tbl_idx >= max_index) {
		printf(HELP_ACL_MASK_TBL_ADD);
		return;
	}
	reg = REG_VTCR_ADDR;
	while (1) {		// wait until not busy
		reg_read(reg, &value);
		if ((value & REG_VTCR_BUSY_MASK) == 0)
			break;
	}
	reg_write(REG_VAWD1_ADDR, vawd1);
	printf("write reg: %x, value: %x\n", REG_VAWD1_ADDR, vawd1);
	reg_write(REG_VAWD2_ADDR, vawd2);
	printf("write reg: %x, value: %x\n", REG_VAWD2_ADDR, vawd2);
	reg = REG_VTCR_ADDR;
	value = REG_VTCR_BUSY_MASK | (0x09 << REG_VTCR_FUNC_OFFT) | tbl_idx;
	reg_write(reg, value);
	printf("write reg: %x, value: %x\n", reg, value);
	while (1) {		// wait until not busy
		reg_read(reg, &value);
		if ((value & REG_VTCR_BUSY_MASK) == 0)
			break;
	}
}

void acl_mask_table_add(int argc, char *argv[])
{
	unsigned int vawd1 = 0, vawd2 = 0;
	unsigned char tbl_idx = 0;
	unsigned int max_index = 0;
	char *endptr;

	if (chip_name == 0x7531 || chip_name == 0x7988)
		max_index = 128;
	else
		max_index = 32;

	errno = 0;
	tbl_idx = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || tbl_idx >= max_index) {
		printf("Error: wrong ACL mask table index\n");
		return;
	}

	errno = 0;
	vawd1 = strtoul(argv[4], &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: wrong ACL mask table write data 1\n");
		return;
	}

	errno = 0;
	vawd2 = strtoul(argv[5], &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: wrong ACL mask table write data 2\n");
		return;
	}

	write_acl_mask_table(tbl_idx, vawd1, vawd2);
}

void write_acl_rule_table(unsigned char tbl_idx, unsigned int vawd1,
			  unsigned int vawd2)
{
	unsigned int value = 0, reg = 0;
	unsigned int max_index = 0;

	if (chip_name == 0x7531 || chip_name == 0x7988)
		max_index = 128;
	else
		max_index = 32;

	printf("Rule_control_tbl_idx:%d\n", tbl_idx);

	if (tbl_idx >= max_index) {
		/* Check the input parameters is right or not. */
		printf(HELP_ACL_RULE_TBL_ADD);
		return;
	}
	reg = REG_VTCR_ADDR;

	while (1) {		// wait until not busy
		reg_read(reg, &value);
		if ((value & REG_VTCR_BUSY_MASK) == 0)
			break;
	}
	reg_write(REG_VAWD1_ADDR, vawd1);
	printf("write reg: %x, value: %x\n", REG_VAWD1_ADDR, vawd1);
	reg_write(REG_VAWD2_ADDR, vawd2);
	printf("write reg: %x, value: %x\n", REG_VAWD2_ADDR, vawd2);
	reg = REG_VTCR_ADDR;
	value = REG_VTCR_BUSY_MASK | (0x0B << REG_VTCR_FUNC_OFFT) | tbl_idx;
	reg_write(reg, value);
	printf("write reg: %x, value: %x\n", reg, value);

	while (1) {		// wait until not busy
		reg_read(reg, &value);
		if ((value & REG_VTCR_BUSY_MASK) == 0)
			break;
	}
}

void acl_rule_table_add(int argc, char *argv[])
{
	unsigned int vawd1 = 0, vawd2 = 0;
	unsigned char tbl_idx = 0;
	unsigned int max_index = 0;
	char *endptr;

	if (chip_name == 0x7531 || chip_name == 0x7988)
		max_index = 128;
	else
		max_index = 32;

	errno = 0;
	tbl_idx = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || tbl_idx >= max_index) {
		printf("Error: wrong ACL rule control table index\n");
		return;
	}

	errno = 0;
	vawd1 = strtoul(argv[4], &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: wrong ACL rule control table write data 1\n");
		return;
	}

	errno = 0;
	vawd2 = strtoul(argv[5], &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: wrong ACL rule control table write data 2\n");
		return;
	}

	write_acl_rule_table(tbl_idx, vawd1, vawd2);
}

void write_rate_table(unsigned char tbl_idx, unsigned int vawd1,
		      unsigned int vawd2)
{
	unsigned int value = 0, reg = 0;
	unsigned int max_index = 32;

	printf("Rule_action_tbl_idx:%d\n", tbl_idx);

	if (tbl_idx >= max_index) {
		printf(HELP_ACL_RATE_TBL_ADD);
		return;
	}

	reg = REG_VTCR_ADDR;
	while (1) {		// wait until not busy
		reg_read(reg, &value);
		if ((value & REG_VTCR_BUSY_MASK) == 0)
			break;
	}

	reg_write(REG_VAWD1_ADDR, vawd1);
	printf("write reg: %x, value: %x\n", REG_VAWD1_ADDR, vawd1);
	reg_write(REG_VAWD2_ADDR, vawd2);
	printf("write reg: %x, value: %x\n", REG_VAWD2_ADDR, vawd2);
	reg = REG_VTCR_ADDR;
	value = REG_VTCR_BUSY_MASK | (0x0D << REG_VTCR_FUNC_OFFT) | tbl_idx;
	reg_write(reg, value);
	printf("write reg: %x, value: %x\n", reg, value);

	while (1) {		// wait until not busy
		reg_read(reg, &value);
		if ((value & REG_VTCR_BUSY_MASK) == 0)
			break;
	}
}

void acl_rate_table_add(int argc, char *argv[])
{
	unsigned int vawd1 = 0, vawd2 = 0;
	unsigned char tbl_idx = 0;
	unsigned int max_index = 32;
	char *endptr;

	errno = 0;
	tbl_idx = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || tbl_idx >= max_index) {
		printf("Error: wrong ACL rate control table index\n");
		return;
	}

	errno = 0;
	vawd1 = strtoul(argv[4], &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: wrong ACL rate control table write data 1\n");
		return;
	}

	errno = 0;
	vawd2 = strtoul(argv[5], &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: wrong ACL rate control table write data 2\n");
		return;
	}

	write_rate_table(tbl_idx, vawd1, vawd2);
}

void write_trTCM_table(unsigned char tbl_idx, unsigned int vawd1,
		       unsigned int vawd2)
{
	unsigned int value = 0, reg = 0;
	unsigned int max_index = 32;

	printf("trTCM_tbl_idx:%d\n", tbl_idx);

	if (tbl_idx >= max_index) {
		printf(HELP_ACL_TRTCM_TBL_ADD);
		return;
	}

	reg = REG_VTCR_ADDR;
	while (1) {		// wait until not busy
		reg_read(reg, &value);
		if ((value & REG_VTCR_BUSY_MASK) == 0)
			break;
	}

	reg_write(REG_VAWD1_ADDR, vawd1);
	printf("write reg: %x, value: %x\n", REG_VAWD1_ADDR, vawd1);
	reg_write(REG_VAWD2_ADDR, vawd2);
	printf("write reg: %x, value: %x\n", REG_VAWD2_ADDR, vawd2);
	reg = REG_VTCR_ADDR;
	value = REG_VTCR_BUSY_MASK | (0x07 << REG_VTCR_FUNC_OFFT) | tbl_idx;
	reg_write(reg, value);
	printf("write reg: %x, value: %x\n", reg, value);

	while (1) {		// wait until not busy
		reg_read(reg, &value);
		if ((value & REG_VTCR_BUSY_MASK) == 0)
			break;
	}
}

int acl_parameters_pre_del(int len1, int len2, int argc, char *argv[],
			   int *port)
{
	int i = 0;

	*port = 0;
	if (argc < len1) {
		printf("insufficient arguments!\n");
		return -1;
	}

	if (len2 == 12) {
		if (!argv[4] || strlen(argv[4]) != len2) {
			printf("The [%s] format error, should be of length %d\n",
					argv[4], len2);
			return -1;
		}
	}

	if (!argv[5] || strlen(argv[5]) != 8) {
		printf("portsmap format error, should be of length 7\n");
		return -1;
	}

	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return -1;
		}
		*port += (argv[5][i] - '0') * (1 << i);
	}
	return 0;
}

void acl_compare_pattern(int ports, int comparion, int base, int word,
			 unsigned char table_index)
{
	unsigned int value = 0;

	comparion |= 0xffff0000;	//compare mask

	value = ports << 8;	//w_port_map
	value |= 0x1 << 19;	//enable
	value |= base << 16;	//mac header
	value |= word << 1;	//word offset

	write_acl_table(table_index, comparion, value);
}

void acl_mac_add(int argc, char *argv[])
{
	unsigned int value = 0;
	int ports = 0;
	char tmpstr[5];
	int ret;
	char *endptr;

	ret = acl_parameters_pre_del(6, 12, argc, argv, &ports);
	if (ret < 0)
		return;
	/* Set pattern */
	strncpy(tmpstr, argv[4], 4);
	tmpstr[4] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0')
		goto error;

	acl_compare_pattern(ports, value, 0x0, 0, 0);

	strncpy(tmpstr, argv[4] + 4, 4);
	tmpstr[4] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0')
		goto error;
	acl_compare_pattern(ports, value, 0x0, 1, 1);

	strncpy(tmpstr, argv[4] + 8, 4);
	tmpstr[4] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0')
		goto error;
	acl_compare_pattern(ports, value, 0x0, 2, 2);

	//set mask
	write_acl_mask_table(0, 0x7, 0);

	//set action
	value = 0x7;		//drop
	value |= 1 << 28;	//acl intterupt enable
	value |= 1 << 27;	//acl hit count
	value |= 2 << 24;	//acl hit count group index (0~3)
	write_acl_rule_table(0, value, 0);
	return;

error:
	printf("Error: string converting\n");
}

void acl_dip_meter(int argc, char *argv[])
{
	unsigned int value = 0, ip_value = 0, meter = 0;
	int ports = 0;
	int ret;

	ip_value = 0;
	ret = acl_parameters_pre_del(7, -1, argc, argv, &ports);
	if (ret < 0)
		return;

	str_to_ip(&ip_value, argv[4]);
	//set pattern
	value = (ip_value >> 16);
	acl_compare_pattern(ports, value, 0x2, 0x8, 0);

	//set pattern
	value = (ip_value & 0xffff);
	acl_compare_pattern(ports, value, 0x2, 0x9, 1);

	//set mask
	write_acl_mask_table(0, 0x3, 0);

	//set action
	meter = strtoul(argv[6], NULL, 0);
	if (((chip_name == 0x7530) && (meter > 1000000)) ||
	    ((chip_name == 0x7531) && (meter > 2500000)) ||
	    ((chip_name == 0x7988) && (meter > 4000000))) {
		printf("\n**Illegal meter input, and 7530: 0~1000000Kpbs, 7531: 0~2500000Kpbs, 7988: 0~4000000Kpbs**\n");
		return;
	}
	if (((chip_name == 0x7531 || chip_name == 0x7988) && (meter > 1000000))) {
		reg_read(0xc, &value);
		value |= 0x1 << 30;
		reg_write(0xC, value);
		printf("AGC: 0x%x\n", value);
		value = meter / 1000;	//uint is 1Mbps
	} else {
		reg_read(0xc, &value);
		value &= ~(0x1 << 30);
		reg_write(0xC, value);
		printf("AGC: 0x%x\n", value);
		value = meter >> 6;	//uint is 64Kbps
	}
	value |= 0x1 << 15;	//enable rate control
	printf("Acl rate control:0x%x\n", value);
	write_rate_table(0, value, 0);
}

void acl_dip_trtcm(int argc, char *argv[])
{
	unsigned int value, value2, ip_value;
	unsigned int CIR, CBS, PIR, PBS;
	int ports;
	int ret;

	ip_value = 0;
	ret = acl_parameters_pre_del(10, -1, argc, argv, &ports);
	if (ret < 0)
		return;

	str_to_ip(&ip_value, argv[4]);
	//set pattern
	value = (ip_value >> 16);
	acl_compare_pattern(ports, value, 0x2, 0x8, 0);

	//set pattern
	value = (ip_value & 0xffff);
	acl_compare_pattern(ports, value, 0x2, 0x9, 1);

	//set CBS PBS
	CIR = strtoul(argv[6], NULL, 0);
	CBS = strtoul(argv[7], NULL, 0);
	PIR = strtoul(argv[8], NULL, 0);
	PBS = strtoul(argv[9], NULL, 0);

	if (CIR > 65535 * 64 || CBS > 65535 || PIR > 65535 * 64 || PBS > 65535) {
		printf("\n**Illegal input parameters**\n");
		return;
	}

	value = CBS << 16;	//bit16~31
	value |= PBS;		//bit0~15
	//value |= 1;//valid
	CIR = CIR >> 6;
	PIR = PIR >> 6;

	value2 = CIR << 16;	//bit16~31
	value2 |= PIR;		//bit0~15
	write_trTCM_table(0, value, value2);

	//set pattern
	write_acl_mask_table(0, 0x3, 0);

	//set action
	value = 0x1 << (11 + 1);	//TrTCM green  meter#0 Low drop
	value |= 0x2 << (8 + 1);	//TrTCM yellow  meter#0 Med drop
	value |= 0x3 << (5 + 1);	//TrTCM red  meter#0    Hig drop
	value |= 0x1 << 0;	//TrTCM drop pcd select
	write_acl_rule_table(0, 0, value);
}

void acl_ethertype(int argc, char *argv[])
{
	unsigned int value, ethertype;
	int ports;
	int ret;

	ret = acl_parameters_pre_del(6, -1, argc, argv, &ports);
	if (ret < 0)
		return;
	printf("ports:0x%x\n", ports);
	ethertype = strtoul(argv[4], NULL, 16);
	//set pattern
	value = ethertype;
	acl_compare_pattern(ports, value, 0x0, 0x6, 0);

	//set pattern
	write_acl_mask_table(0, 0x1, 0);

	//set action(drop)
	value = 0x7;		//default. Nodrop
	value |= 1 << 28;	//acl intterupt enable
	value |= 1 << 27;	//acl hit count

	write_acl_rule_table(0, value, 0);
}

void acl_dip_modify(int argc, char *argv[])
{
	unsigned int value, ip_value;
	int ports;
	int priority;
	int ret;

	ip_value = 0;
	priority = strtoul(argv[6], NULL, 16);
	if (priority < 0 || priority > 7) {
		printf("\n**Illegal priority value!**\n");
		return;
	}

	ret = acl_parameters_pre_del(6, -1, argc, argv, &ports);
	if (ret < 0)
		return;

	str_to_ip(&ip_value, argv[4]);
	//set pattern
	value = (ip_value >> 16);
	acl_compare_pattern(ports, value, 0x2, 0x8, 0);

	//set pattern
	value = (ip_value & 0xffff);
	acl_compare_pattern(ports, value, 0x2, 0x9, 1);

	//set pattern
	write_acl_mask_table(0, 0x3, 0);

	//set action
	value = 0x0;		//default. Nodrop
	value |= 1 << 28;	//acl intterupt enable
	value |= 1 << 27;	//acl hit count
	value |= priority << 4;	//acl UP
	write_acl_rule_table(0, value, 0);
}

void acl_dip_pppoe(int argc, char *argv[])
{
	unsigned int value, ip_value;
	int ports;
	int ret;

	ip_value = 0;
	ret = acl_parameters_pre_del(6, -1, argc, argv, &ports);
	if (ret < 0)
		return;

	str_to_ip(&ip_value, argv[4]);
	//set pattern
	value = (ip_value >> 16);
	acl_compare_pattern(ports, value, 0x2, 0x8, 0);

	//set pattern
	value = (ip_value & 0xffff);
	acl_compare_pattern(ports, value, 0x2, 0x9, 1);

	//set pattern
	write_acl_mask_table(0, 0x3, 0);

	//set action
	value = 0x0;		//default. Nodrop
	value |= 1 << 28;	//acl intterupt enable
	value |= 1 << 27;	//acl hit count
	value |= 1 << 20;	//pppoe header remove
	value |= 1 << 21;	//SA MAC SWAP
	value |= 1 << 22;	//DA MAC SWAP
	write_acl_rule_table(0, value, 7);
}

void acl_dip_add(int argc, char *argv[])
{
	unsigned int value, ip_value;
	int ports;
	int ret;

	ip_value = 0;
	ret = acl_parameters_pre_del(6, -1, argc, argv, &ports);
	if (ret < 0)
		return;

	str_to_ip(&ip_value, argv[4]);
	//set pattern
	value = (ip_value >> 16);
	acl_compare_pattern(ports, value, 0x2, 0x8, 0);

	//set pattern
	value = (ip_value & 0xffff);
	acl_compare_pattern(ports, value, 0x2, 0x9, 1);

	//set pattern
	write_acl_mask_table(0, 0x3, 0);

	//set action
	//value = 0x0; //default
	value = 0x7;		//drop
	value |= 1 << 28;	//acl intterupt enable
	value |= 1 << 27;	//acl hit count
	value |= 2 << 24;	//acl hit count group index (0~3)
	write_acl_rule_table(0, value, 0);
}

void acl_l4_add(int argc, char *argv[])
{
	unsigned int value = 0;
	int ports;
	int ret;

	ret = acl_parameters_pre_del(6, -1, argc, argv, &ports);
	if (ret < 0)
		return;

	//set pattern
	value = strtoul(argv[4], NULL, 16);
	acl_compare_pattern(ports, value, 0x5, 0x0, 0);

	//set rue mask
	write_acl_mask_table(0, 0x1, 0);
	//set action
	value = 0x7;		//drop
	//value |= 1;//valid
	write_acl_rule_table(0, value, 0);
}

void acl_sp_add(int argc, char *argv[])
{
	unsigned int value = 0;
	int ports;
	int ret;

	ret = acl_parameters_pre_del(6, -1, argc, argv, &ports);
	if (ret < 0)
		return;
	//set pattern
	value = strtoul(argv[4], NULL, 0);
	acl_compare_pattern(ports, value, 0x4, 0x0, 0);

	//set rue mask
	write_acl_mask_table(0, 0x1, 0);

	//set action
	value = 0x7;		//drop
	//value |= 1;//valid
	write_acl_rule_table(0, value, 0);
}

void acl_port_enable(int argc, char *argv[])
{
	unsigned int value = 0, reg = 0;
	unsigned char acl_port = 0, acl_en = 0;
	char *endptr;

	errno = 0;
	acl_port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || acl_port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	acl_en = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || acl_en > 1) {
		printf(HELP_ACL_SETPORTEN);
		return;
	}

	printf("acl_port:%d, acl_en:%d\n", acl_port, acl_en);

	reg = REG_PCR_P0_ADDR + (0x100 * acl_port);	// 0x2004[10]
	reg_read(reg, &value);
	value &= (~REG_PORT_ACL_EN_MASK);
	value |= (acl_en << REG_PORT_ACL_EN_OFFT);

	printf("write reg: %x, value: %x\n", reg, value);
	reg_write(reg, value);
}

static void dip_dump_internal(int type)
{
	unsigned int i, j, value, mac, mac2, value2;
	char tmpstr[16] = { 0 };
	int table_size = 0;
	int hit_value1 = 0;
	int hit_value2 = 0;

	if (type == GENERAL_TABLE) {
		table_size = 0x800;
		reg_write(REG_ATC_ADDR, 0x8104);	//dip search command
	} else {
		table_size = 0x40;
		reg_write(REG_ATC_ADDR, 0x811c);	//dip search command
	}
	printf("hash   port(0:6)   rsp_cnt  flag  timer    dip-address       ATRD\n");
	for (i = 0; i < table_size; i++) {
		while (1) {
			reg_read(REG_ATC_ADDR, &value);
			if (type == GENERAL_TABLE) {
				hit_value1 = value & (0x1 << 13);
				hit_value2 = 1;
			} else {
				hit_value1 = value & (0x1 << 13);
				hit_value2 = value & (0x1 << 28);
			}

			if (hit_value1 && hit_value2) {	//search_rdy
				reg_read(REG_ATRD_ADDR, &value2);
				//printf("REG_ATRD_ADDR=0x%x\n\r",value2);

				printf("%03x:   ", (value >> 16) & 0xfff);	//hash_addr_lu
				j = (value2 >> 4) & 0xff;	//r_port_map
				printf("%c", (j & 0x01) ? '1' : '-');
				printf("%c", (j & 0x02) ? '1' : '-');
				printf("%c", (j & 0x04) ? '1' : '-');
				printf("%c ", (j & 0x08) ? '1' : '-');
				printf("%c", (j & 0x10) ? '1' : '-');
				printf("%c", (j & 0x20) ? '1' : '-');
				printf("%c", (j & 0x40) ? '1' : '-');

				reg_read(REG_TSRA2_ADDR, &mac2);

				printf("     0x%4x", (mac2 & 0xffff));	//RESP_CNT
				printf("  0x%2x", ((mac2 >> 16) & 0xff));	//RESP_FLAG
				printf("  %3d", ((mac2 >> 24) & 0xff));	//RESP_TIMER
				//printf(" %4d", (value2 >> 24) & 0xff); //r_age_field
				reg_read(REG_TSRA1_ADDR, &mac);
				ip_to_str(tmpstr, sizeof(tmpstr), mac);
				printf("     %s", tmpstr);
				printf("  0x%8x\n", value2);	//ATRD
				//printf("%04x", ((mac2 >> 16) & 0xffff));
				//printf("     %c\n", (((value2 >> 20) & 0x03)== 0x03)? 'y':'-');
				if (value & 0x4000) {
					printf("end of table %d\n", i);
					return;
				}
				break;
			} else if (value & 0x4000) {	//at_table_end
				printf("found the last entry %d (not ready)\n",
				       i);
				return;
			}
			usleep(5000);
		}

		if (type == GENERAL_TABLE)
			reg_write(REG_ATC_ADDR, 0x8105);	//search for next dip address
		else
			reg_write(REG_ATC_ADDR, 0x811d);	//search for next dip address
		usleep(5000);
	}
}

void dip_dump(int argc, char *argv[])
{
	dip_dump_internal(GENERAL_TABLE);

}

void dip_add(int argc, char *argv[])
{
	unsigned int value = 0;
	unsigned int i = 0, j = 0;

	value = 0;

	str_to_ip(&value, argv[3]);

	reg_write(REG_ATA1_ADDR, value);
	printf("REG_ATA1_ADDR is 0x%x\n\r", value);

	if (!argv[4] || strlen(argv[4]) != 8) {
		printf("portmap format error, should be of length 7\n");
		return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[4][i] != '0' && argv[4][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[4][i] - '0') * (1 << i);
	}
	value = j << 4;		//w_port_map
	value |= (0x3 << 2);	//static

	reg_write(REG_ATWD_ADDR, value);

	usleep(5000);
	reg_read(REG_ATWD_ADDR, &value);
	printf("REG_ATWD_ADDR is 0x%x\n\r", value);

	value = 0x8011;		//single w_dip_cmd
	reg_write(REG_ATC_ADDR, value);

	usleep(1000);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ATC_ADDR, &value);
		if ((value & 0x8000) == 0) {	//mac address busy
			printf("done.\n");
			return;
		}
		usleep(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void dip_del(int argc, char *argv[])
{
	unsigned int i, value;

	value = 0;
	str_to_ip(&value, argv[3]);

	reg_write(REG_ATA1_ADDR, value);

	value = 0;
	reg_write(REG_ATA2_ADDR, value);

	value = 0;		//STATUS=0, delete dip
	reg_write(REG_ATWD_ADDR, value);

	value = 0x8011;		//w_dip_cmd
	reg_write(REG_ATC_ADDR, value);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ATC_ADDR, &value);
		if ((value & 0x8000) == 0) {	//mac address busy
			if (argv[1] != NULL)
				printf("done.\n");
			return;
		}
		usleep(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void dip_clear(int argc, char *argv[])
{
	unsigned int value = 0;

	reg_write(REG_ATC_ADDR, 0x8102);	//clear all dip
	usleep(5000);
	reg_read(REG_ATC_ADDR, &value);
	printf("REG_ATC_ADDR is 0x%x\n\r", value);
}

static void sip_dump_internal(int type)
{
	unsigned int i = 0, j = 0, value = 0, mac = 0, mac2 = 0, value2 = 0;
	int table_size = 0;
	int hit_value1 = 0;
	int hit_value2 = 0;
	char tmpstr[16] = { 0 };

	if (type == GENERAL_TABLE) {
		table_size = 0x800;
		reg_write(REG_ATC_ADDR, 0x8204);	//sip search command
	} else {
		table_size = 0x40;
		reg_write(REG_ATC_ADDR, 0x822c);	//sip search command
	}
	printf("hash  port(0:6)   dip-address    sip-address      ATRD\n");
	for (i = 0; i < table_size; i++) {
		while (1) {
			reg_read(REG_ATC_ADDR, &value);
			if (type == GENERAL_TABLE) {
				hit_value1 = value & (0x1 << 13);
				hit_value2 = 1;
			} else {
				hit_value1 = value & (0x1 << 13);
				hit_value2 = value & (0x1 << 28);
			}

			if (hit_value1 && hit_value2) {	//search_rdy
				reg_read(REG_ATRD_ADDR, &value2);
				//printf("REG_ATRD_ADDR=0x%x\n\r",value2);

				printf("%03x:  ", (value >> 16) & 0xfff);	//hash_addr_lu
				j = (value2 >> 4) & 0xff;	//r_port_map
				printf("%c", (j & 0x01) ? '1' : '-');
				printf("%c", (j & 0x02) ? '1' : '-');
				printf("%c", (j & 0x04) ? '1' : '-');
				printf("%c", (j & 0x08) ? '1' : '-');
				printf(" %c", (j & 0x10) ? '1' : '-');
				printf("%c", (j & 0x20) ? '1' : '-');
				printf("%c", (j & 0x40) ? '1' : '-');

				reg_read(REG_TSRA2_ADDR, &mac2);

				ip_to_str(tmpstr, sizeof(tmpstr), mac2);
				printf("   %s", tmpstr);

				//printf(" %4d", (value2 >> 24) & 0xff); //r_age_field
				reg_read(REG_TSRA1_ADDR, &mac);
				ip_to_str(tmpstr, sizeof(tmpstr), mac);
				printf("    %s", tmpstr);
				printf("      0x%x\n", value2);
				//printf("%04x", ((mac2 >> 16) & 0xffff));
				//printf("     %c\n", (((value2 >> 20) & 0x03)== 0x03)? 'y':'-');
				if (value & 0x4000) {
					printf("end of table %d\n", i);
					return;
				}
				break;
			} else if (value & 0x4000) {	//at_table_end
				printf("found the last entry %d (not ready)\n",
				       i);
				return;
			}
			usleep(5000);
		}

		if (type == GENERAL_TABLE)
			reg_write(REG_ATC_ADDR, 0x8205);	//search for next sip address
		else
			reg_write(REG_ATC_ADDR, 0x822d);	//search for next sip address
		usleep(5000);
	}
}

void sip_dump(int argc, char *argv[])
{

	sip_dump_internal(GENERAL_TABLE);

}

void sip_add(int argc, char *argv[])
{
	unsigned int i = 0, j = 0, value = 0;

	value = 0;
	str_to_ip(&value, argv[3]);	//SIP

	reg_write(REG_ATA2_ADDR, value);
	printf("REG_ATA2_ADDR is 0x%x\n\r", value);

	value = 0;

	str_to_ip(&value, argv[4]);	//DIP
	reg_write(REG_ATA1_ADDR, value);
	printf("REG_ATA1_ADDR is 0x%x\n\r", value);

	if (!argv[5] || strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 7\n");
		return;
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[5][i] - '0') * (1 << i);
	}
	value = j << 4;		//w_port_map
	value |= (0x3 << 2);	//static

	reg_write(REG_ATWD_ADDR, value);

	usleep(5000);
	reg_read(REG_ATWD_ADDR, &value);
	printf("REG_ATWD_ADDR is 0x%x\n\r", value);

	value = 0x8021;		//single w_sip_cmd
	reg_write(REG_ATC_ADDR, value);

	usleep(1000);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ATC_ADDR, &value);
		if ((value & 0x8000) == 0) {	//mac address busy
			printf("done.\n");
			return;
		}
		usleep(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void sip_del(int argc, char *argv[])
{
	unsigned int i = 0, value = 0;

	value = 0;
	str_to_ip(&value, argv[3]);

	reg_write(REG_ATA2_ADDR, value);	//SIP

	str_to_ip(&value, argv[4]);
	reg_write(REG_ATA1_ADDR, value);	//DIP

	value = 0;		//STATUS=0, delete sip
	reg_write(REG_ATWD_ADDR, value);

	value = 0x8021;		//w_sip_cmd
	reg_write(REG_ATC_ADDR, value);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ATC_ADDR, &value);
		if ((value & 0x8000) == 0) {	//mac address busy
			if (argv[1] != NULL)
				printf("done.\n");
			return;
		}
		usleep(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void sip_clear(int argc, char *argv[])
{
	unsigned int value = 0;

	reg_write(REG_ATC_ADDR, 0x8202);	//clear all sip
	usleep(5000);
	reg_read(REG_ATC_ADDR, &value);
	printf("REG_ATC_ADDR is 0x%x\n\r", value);
}

static void table_dump_internal(int type)
{
	unsigned int i = 0, j = 0, value = 0, mac = 0, mac2 = 0, value2 = 0;
	int table_size = 0;
	int table_end = 0;
	int hit_value1 = 0;
	int hit_value2 = 0;

	if (type == GENERAL_TABLE) {
		table_size = 0x800;
		table_end = 0x7FF;
		reg_write(REG_ATC_ADDR, 0x8004);
	} else {
		table_size = 0x40;
		table_end = 0x3F;
		reg_write(REG_ATC_ADDR, 0x800C);
	}
	printf("hash  port(0:6)   fid   vid  age(s)   mac-address     filter my_mac\n");
	for (i = 0; i < table_size; i++) {
		while (1) {
			reg_read(REG_ATC_ADDR, &value);
			//printf("ATC =  0x%x\n", value);
			if (type == GENERAL_TABLE) {
				hit_value1 = value & (0x1 << 13);
				hit_value2 = 1;
			} else {
				hit_value1 = value & (0x1 << 13);
				hit_value2 = value & (0x1 << 28);
			}

			if (hit_value1 && hit_value2
			    && (((value >> 15) & 0x1) == 0)) {
				printf("%03x:   ", (value >> 16) & 0xfff);
				reg_read(REG_ATRD_ADDR, &value2);
				j = (value2 >> 4) & 0xff;	//r_port_map
				printf("%c", (j & 0x01) ? '1' : '-');
				printf("%c", (j & 0x02) ? '1' : '-');
				printf("%c", (j & 0x04) ? '1' : '-');
				printf("%c", (j & 0x08) ? '1' : '-');
				printf("%c", (j & 0x10) ? '1' : '-');
				printf("%c", (j & 0x20) ? '1' : '-');
				printf("%c", (j & 0x40) ? '1' : '-');
				printf("%c", (j & 0x80) ? '1' : '-');

				reg_read(REG_TSRA2_ADDR, &mac2);

				printf("   %2d", (mac2 >> 12) & 0x7);	//FID
				printf("  %4d", (mac2 & 0xfff));

				if (((value2 >> MAC_ENTRY_LIVE_STATUS) &
					MAC_ENTRY_LIVE_STATUS_MASK) == MAC_ENTRY_STATIC)
					printf("   --- ");
				else
					printf(" %5d ", (((value2 >> 24) & 0xff) + 1) * 2);

				reg_read(REG_TSRA1_ADDR, &mac);
				printf("  %08x", mac);
				printf("%04x", ((mac2 >> 16) & 0xffff));
				printf("     %c",
				       (((value2 >> 20) & 0x03) ==
					0x03) ? 'y' : '-');
				printf("     %c\n",
				       (((value2 >> 23) & 0x01) ==
					0x01) ? 'y' : '-');
				if ((value & 0x4000)
				    && (((value >> 16) & 0xfff) == table_end)) {
					printf("end of table %d\n", i);
					return;
				}
				break;
			} else if ((value & 0x4000) && (((value >> 15) & 0x1) == 0) && (((value >> 16) & 0xfff) == table_end)) {	//at_table_end
				printf("found the last entry %d (not ready)\n", i);
				return;
			} else
				usleep(5);
		}

		if (type == GENERAL_TABLE)
			reg_write(REG_ATC_ADDR, 0x8005);	//search for next address
		else
			reg_write(REG_ATC_ADDR, 0x800d);	//search for next address
		usleep(5);
	}
}

void table_dump(int argc, char *argv[])
{
	table_dump_internal(GENERAL_TABLE);

}

void table_add(int argc, char *argv[])
{
	unsigned int i = 0, j = 0, value = 0, is_filter = 0, is_mymac = 0;
	char tmpstr[9];
	char *endptr;

	is_filter = (argv[1][0] == 'f') ? 1 : 0;
	is_mymac = (argv[1][0] == 'm') ? 1 : 0;
	if (!argv[2] || strlen(argv[2]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}
	strncpy(tmpstr, argv[2], 8);
	tmpstr[8] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: string converting\n");
		return;
	}
	reg_write(REG_ATA1_ADDR, value);
	printf("REG_ATA1_ADDR is 0x%x\n\r", value);

	strncpy(tmpstr, argv[2] + 8, 4);
	tmpstr[4] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: string converting\n");
		return;
	}
	value = (value << 16);
	value |= (1 << 15);	//IVL=1

	if (argc > 4) {
		errno = 0;
		j = strtoul(argv[4], &endptr, 10);
		if (errno != 0 || *endptr != '\0' || j > 4095) {
			printf("wrong vid range, should be within 0~4095\n");
			return;
		}
		value |= j;	//vid
	}

	reg_write(REG_ATA2_ADDR, value);
	printf("REG_ATA2_ADDR is 0x%x\n\r", value);

	if (!argv[3] || strlen(argv[3]) != 8) {
		if (is_filter)
			argv[3] = "11111111";
		else {
			printf("portmap format error, should be of length 8\n");
			return;
		}
	}
	j = 0;
	for (i = 0; i < 7; i++) {
		if (argv[3][i] != '0' && argv[3][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		j += (argv[3][i] - '0') * (1 << i);
	}
	value = j << 4;		//w_port_map

	if (argc > 5) {
		errno = 0;
		j = strtoul(argv[5], &endptr, 10);
		if (errno != 0 || *endptr != '\0' || j < 1 || 255 < j) {
			printf("wrong age range, should be within 1~255\n");
			return;
		}
		value |= (j << 24);	//w_age_field
		value |= (0x1 << 2);	//dynamic
	} else {
		value |= (0xff << 24);	//w_age_field
		value |= (0x3 << 2);	//static
	}

	if (argc > 6) {
		errno = 0;
		j = strtoul(argv[6], &endptr, 10);
		if (errno != 0 || *endptr != '\0' || j > 7) {
			printf("wrong eg-tag range, should be within 0~7\n");
			return;
		}
		value |= (j << 13);	//EG_TAG
	}

	if (is_filter)
		value |= (7 << 20);	//sa_filter

	if (is_mymac)
		value |= (1 << 23);

	reg_write(REG_ATWD_ADDR, value);

	usleep(5000);
	reg_read(REG_ATWD_ADDR, &value);
	printf("REG_ATWD_ADDR is 0x%x\n\r", value);

	value = 0x8001;		//w_mac_cmd
	reg_write(REG_ATC_ADDR, value);

	usleep(1000);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ATC_ADDR, &value);
		if ((value & 0x8000) == 0) {	//mac address busy
			printf("done.\n");
			return;
		}
		usleep(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void table_search_mac_vid(int argc, char *argv[])
{
	unsigned int i = 0, j = 0, value = 0, mac = 0, mac2 = 0, value2 = 0;
	char tmpstr[9];
	char *endptr;

	if (!argv[3] || strlen(argv[3]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}
	strncpy(tmpstr, argv[3], 8);
	tmpstr[8] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: string converting\n");
		return;
	}
	reg_write(REG_ATA1_ADDR, value);
	//printf("REG_ATA1_ADDR is 0x%x\n\r",value);

	strncpy(tmpstr, argv[3] + 8, 4);
	tmpstr[4] = '\0';

	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: string converting\n");
		return;
	}
	value = (value << 16);
	value |= (1 << 15);	//IVL=1

	errno = 0;
	j = strtoul(argv[5], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || j > 4095) {
		printf("wrong vid range, should be within 0~4095\n");
		return;
	}

	value |= j;		//vid

	reg_write(REG_ATA2_ADDR, value);
	//printf("REG_ATA2_ADDR is 0x%x\n\r",value);

	value = 0x8000;		//w_mac_cmd
	reg_write(REG_ATC_ADDR, value);

	usleep(1000);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ATC_ADDR, &value);
		if ((value & 0x8000) == 0) {	//mac address busy
			break;
		}
		usleep(1000);
	}
	if (i == 20) {
		printf("search timeout.\n");
		return;
	}

	if (value & 0x1000) {
		printf("search no entry.\n");
		return;
	}

	printf("search done.\n");
	printf
	    ("hash  port(0:6)   fid   vid  age   mac-address     filter my_mac\n");

	printf("%03x:   ", (value >> 16) & 0xfff);	//hash_addr_lu
	reg_read(REG_ATRD_ADDR, &value2);
	j = (value2 >> 4) & 0xff;	//r_port_map
	printf("%c", (j & 0x01) ? '1' : '-');
	printf("%c", (j & 0x02) ? '1' : '-');
	printf("%c", (j & 0x04) ? '1' : '-');
	printf("%c ", (j & 0x08) ? '1' : '-');
	printf("%c", (j & 0x10) ? '1' : '-');
	printf("%c", (j & 0x20) ? '1' : '-');
	printf("%c", (j & 0x40) ? '1' : '-');
	printf("%c", (j & 0x80) ? '1' : '-');

	reg_read(REG_TSRA2_ADDR, &mac2);

	printf("   %2d", (mac2 >> 12) & 0x7);	//FID
	printf("  %4d", (mac2 & 0xfff));
	printf(" %4d", (value2 >> 24) & 0xff);	//r_age_field
	reg_read(REG_TSRA1_ADDR, &mac);
	printf("  %08x", mac);
	printf("%04x", ((mac2 >> 16) & 0xffff));
	printf("     %c", (((value2 >> 20) & 0x03) == 0x03) ? 'y' : '-');
	printf("     %c\n", (((value2 >> 23) & 0x01) == 0x01) ? 'y' : '-');
}

void table_search_mac_fid(int argc, char *argv[])
{
	unsigned int i = 0, j = 0, value = 0, mac = 0, mac2 = 0, value2 = 0;
	char tmpstr[9];
	char *endptr;

	if (!argv[3] || strlen(argv[3]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}
	strncpy(tmpstr, argv[3], 8);
	tmpstr[8] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: string converting\n");
		return;
	}
	reg_write(REG_ATA1_ADDR, value);
	//printf("REG_ATA1_ADDR is 0x%x\n\r",value);

	strncpy(tmpstr, argv[3] + 8, 4);
	tmpstr[4] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: string converting\n");
		return;
	}
	value = (value << 16);
	value &= ~(1 << 15);	//IVL=0

	errno = 0;
	j = strtoul(argv[5], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || j > 7) {
		printf("wrong fid range, should be within 0~7\n");
		return;
	}

	value |= (j << 12);	//vid

	reg_write(REG_ATA2_ADDR, value);
	//printf("REG_ATA2_ADDR is 0x%x\n\r",value);

	value = 0x8000;		//w_mac_cmd
	reg_write(REG_ATC_ADDR, value);

	usleep(1000);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ATC_ADDR, &value);
		if ((value & 0x8000) == 0) {	//mac address busy
			break;
		}
		usleep(1000);
	}
	if (i == 20) {
		printf("search timeout.\n");
		return;
	}

	if (value & 0x1000) {
		printf("search no entry.\n");
		return;
	}

	printf("search done.\n");
	printf
	    ("hash  port(0:6)   fid   vid  age   mac-address     filter my_mac\n");

	printf("%03x:   ", (value >> 16) & 0xfff);	//hash_addr_lu
	reg_read(REG_ATRD_ADDR, &value2);
	j = (value2 >> 4) & 0xff;	//r_port_map
	printf("%c", (j & 0x01) ? '1' : '-');
	printf("%c", (j & 0x02) ? '1' : '-');
	printf("%c", (j & 0x04) ? '1' : '-');
	printf("%c ", (j & 0x08) ? '1' : '-');
	printf("%c", (j & 0x10) ? '1' : '-');
	printf("%c", (j & 0x20) ? '1' : '-');
	printf("%c", (j & 0x40) ? '1' : '-');
	printf("%c", (j & 0x80) ? '1' : '-');

	reg_read(REG_TSRA2_ADDR, &mac2);

	printf("   %2d", (mac2 >> 12) & 0x7);	//FID
	printf("  %4d", (mac2 & 0xfff));
	printf(" %4d", (value2 >> 24) & 0xff);	//r_age_field
	reg_read(REG_TSRA1_ADDR, &mac);
	printf("  %08x", mac);
	printf("%04x", ((mac2 >> 16) & 0xffff));
	printf("     %c", (((value2 >> 20) & 0x03) == 0x03) ? 'y' : '-');
	printf("     %c\n", (((value2 >> 23) & 0x01) == 0x01) ? 'y' : '-');
}

void table_del_fid(int argc, char *argv[])
{
	unsigned int i = 0, j = 0, value = 0;
	char tmpstr[9];
	char *endptr;

	if (!argv[3] || strlen(argv[3]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}
	strncpy(tmpstr, argv[3], 8);
	tmpstr[8] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: string converting\n");
		return;
	}
	reg_write(REG_ATA1_ADDR, value);

	strncpy(tmpstr, argv[3] + 8, 4);
	tmpstr[4] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: string converting\n");
		return;
	}
	value = (value << 16);

	if (argc > 5) {
		errno = 0;
		j = strtoul(argv[5], &endptr, 10);
		if (errno != 0 || *endptr != '\0' || j > 7) {
			printf("wrong fid range, should be within 0~7\n");
			return;
		}
		value |= (j << 12);	/* fid */
	}

	reg_write(REG_ATA2_ADDR, value);

	value = 0;		/* STATUS=0, delete mac */
	reg_write(REG_ATWD_ADDR, value);

	value = 0x8001;		//w_mac_cmd
	reg_write(REG_ATC_ADDR, value);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ATC_ADDR, &value);
		if ((value & 0x8000) == 0) {	/* mac address busy */
			if (argv[1] != NULL)
				printf("done.\n");
			return;
		}
		usleep(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void table_del_vid(int argc, char *argv[])
{
	unsigned int i = 0, j = 0, value = 0;
	char tmpstr[9];
	char *endptr;

	if (!argv[3] || strlen(argv[3]) != 12) {
		printf("MAC address format error, should be of length 12\n");
		return;
	}
	strncpy(tmpstr, argv[3], 8);
	tmpstr[8] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: string converting\n");
		return;
	}
	reg_write(REG_ATA1_ADDR, value);

	strncpy(tmpstr, argv[3] + 8, 4);
	tmpstr[4] = '\0';
	errno = 0;
	value = strtoul(tmpstr, &endptr, 16);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: string converting\n");
		return;
	}
	value = (value << 16);

	errno = 0;
	j = strtoul(argv[5], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || j > 4095) {
		printf("wrong fid range, should be within 0~4095\n");
		return;
	}

	value |= j;		//vid
	value |= 1 << 15;
	reg_write(REG_ATA2_ADDR, value);

	value = 0;		//STATUS=0, delete mac
	reg_write(REG_ATWD_ADDR, value);

	value = 0x8001;		//w_mac_cmd
	reg_write(REG_ATC_ADDR, value);

	for (i = 0; i < 20; i++) {
		reg_read(REG_ATC_ADDR, &value);
		if ((value & 0x8000) == 0) {	//mac address busy
			if (argv[1] != NULL)
				printf("done.\n");
			return;
		}
		usleep(1000);
	}
	if (i == 20)
		printf("timeout.\n");
}

void table_clear(int argc, char *argv[])
{
	unsigned int value = 0;

	reg_write(REG_ATC_ADDR, 0x8002);
	usleep(5000);
	reg_read(REG_ATC_ADDR, &value);

	printf("REG_ATC_ADDR is 0x%x\n\r", value);
}

void set_mirror_to(int argc, char *argv[])
{
	unsigned int value = 0;
	int idx = 0;

	idx = strtoul(argv[3], NULL, 0);
	if (idx < 0 || MAX_PORT < idx) {
		printf("wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}
	if (chip_name == 0x7530) {

		reg_read(REG_MFC_ADDR, &value);
		value |= 0x1 << 3;
		value &= 0xfffffff8;
		value |= idx << 0;

		reg_write(REG_MFC_ADDR, value);
	} else {

		reg_read(REG_CFC_ADDR, &value);
		value &= (~REG_CFC_MIRROR_EN_MASK);
		value |= (1 << REG_CFC_MIRROR_EN_OFFT);
		value &= (~REG_CFC_MIRROR_PORT_MASK);
		value |= (idx << REG_CFC_MIRROR_PORT_OFFT);
		reg_write(REG_CFC_ADDR, value);
	}
}

void set_mirror_from(int argc, char *argv[])
{
	unsigned int offset = 0, value = 0;
	unsigned int idx = 0, mirror = 0;
	char *endptr;

	errno = 0;
	idx = strtoul(argv[3], &endptr, 0);
	if (errno != 0 || *endptr != '\0' || idx > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	mirror = strtoul(argv[4], &endptr, 0);

	if (errno != 0 || *endptr != '\0' || mirror > 3) {
		printf("wrong mirror setting, should be within 0~3\n");
		return;
	}

	offset = (0x2004 | (idx << 8));
	reg_read(offset, &value);

	value &= 0xfffffcff;
	value |= mirror << 8;

	reg_write(offset, value);
}

void vlan_dump(int argc, char *argv[])
{
	unsigned int i = 0, j = 0, value = 0, value2 = 0;
	int eg_tag = 0;

	if (argc == 4) {
		if (!strncmp(argv[3], "egtag", 6))
			eg_tag = 1;
	}

	if (eg_tag)
		printf("  vid  fid  portmap    s-tag\teg_tag(0:untagged 2:tagged)\n");
	else
		printf("  vid  fid  portmap    s-tag\n");

	for (i = 1; i < 4095; i++) {
		value = (0x80000000 + i);	//r_vid_cmd
		reg_write(REG_VTCR_ADDR, value);

		for (j = 0; j < 20; j++) {
			reg_read(REG_VTCR_ADDR, &value);
			if ((value & 0x80000000) == 0) {	//mac address busy
				break;
			}
			usleep(1000);
		}
		if (j == 20)
			printf("timeout.\n");

		reg_read(REG_VAWD1_ADDR, &value);
		reg_read(REG_VAWD2_ADDR, &value2);
		//printf("REG_VAWD1_ADDR value%d is 0x%x\n\r", i, value);
		//printf("REG_VAWD2_ADDR value%d is 0x%x\n\r", i, value2);

		if ((value & 0x01) != 0) {
			printf(" %4d  ", i);
			printf(" %2d ", ((value & 0xe) >> 1));
			printf(" %c", (value & 0x00010000) ? '1' : '-');
			printf("%c", (value & 0x00020000) ? '1' : '-');
			printf("%c", (value & 0x00040000) ? '1' : '-');
			printf("%c", (value & 0x00080000) ? '1' : '-');
			printf("%c", (value & 0x00100000) ? '1' : '-');
			printf("%c", (value & 0x00200000) ? '1' : '-');
			printf("%c", (value & 0x00400000) ? '1' : '-');
			printf("%c", (value & 0x00800000) ? '1' : '-');
			printf("    %4d", ((value & 0xfff0) >> 4));
			if (eg_tag) {
				printf("\t");
				if ((value & (0x3 << 28)) == (0x3 << 28)) {
					/* VTAG_EN=1 and EG_CON=1 */
					printf("CONSISTENT");
				} else if (value & (0x1 << 28)) {
					/* VTAG_EN=1 */
					printf("%d", (value2 & 0x0003) >> 0);
					printf("%d", (value2 & 0x000c) >> 2);
					printf("%d", (value2 & 0x0030) >> 4);
					printf("%d", (value2 & 0x00c0) >> 6);
					printf("%d", (value2 & 0x0300) >> 8);
					printf("%d", (value2 & 0x0c00) >> 10);
					printf("%d", (value2 & 0x3000) >> 12);
					printf("%d", (value2 & 0xc000) >> 14);
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
				printf(" %2d ", ((value & 0xe) >> 1));
				printf(" invalid\n");
			}
		}
	}
}

static long timespec_diff_us(struct timespec start, struct timespec end)
{
	struct timespec temp;
	unsigned long duration = 0;

	if ((end.tv_nsec - start.tv_nsec) < 0) {
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	/* calculate second part */
	duration += temp.tv_sec * 1000000;
	/* calculate ns part */
	duration += temp.tv_nsec >> 10;

	return duration;
}

void vlan_clear(int argc, char *argv[])
{
	unsigned int value = 0;
	int vid = 0;
	unsigned long duration_us = 0;
	struct timespec start, end;

	for (vid = 0; vid < 4096; vid++) {
		clock_gettime(CLOCK_REALTIME, &start);
		value = 0;	//invalid
		reg_write(REG_VAWD1_ADDR, value);

		value = (0x80001000 + vid);	//w_vid_cmd
		reg_write(REG_VTCR_ADDR, value);
		while (duration_us <= 1000) {
			reg_read(REG_VTCR_ADDR, &value);
			if ((value & 0x80000000) == 0) {	//table busy
				break;
			}
			clock_gettime(CLOCK_REALTIME, &end);
			duration_us = timespec_diff_us(start, end);
		}
		if (duration_us > 1000)
			printf("config vlan timeout: %ld.\n", duration_us);
	}
}

void vlan_set(int argc, char *argv[])
{
	unsigned int vlan_mem = 0;
	unsigned int value = 0;
	unsigned int value2 = 0;
	int i = 0, vid = 0, fid = 0;
	int stag = 0;
	unsigned long eg_con = 0;
	unsigned int eg_tag = 0;

	if (argc < 5) {
		printf("insufficient arguments!\n");
		return;
	}

	fid = strtoul(argv[3], NULL, 0);
	if (fid < 0 || fid > 7) {
		printf("wrong filtering db id range, should be within 0~7\n");
		return;
	}
	value |= (fid << 1);

	vid = strtoul(argv[4], NULL, 0);
	if (vid < 0 || 0xfff < vid) {
		printf("wrong vlan id range, should be within 0~4095\n");
		return;
	}

	if (strlen(argv[5]) != 8) {
		printf("portmap format error, should be of length 7\n");
		return;
	}

	vlan_mem = 0;
	for (i = 0; i < 8; i++) {
		if (argv[5][i] != '0' && argv[5][i] != '1') {
			printf("portmap format error, should be of combination of 0 or 1\n");
			return;
		}
		vlan_mem += (argv[5][i] - '0') * (1 << i);
	}

	/* VLAN stag */
	if (argc > 6) {
		stag = strtoul(argv[6], NULL, 16);
		if (stag < 0 || 0xfff < stag) {
			printf("wrong stag id range, should be within 0~4095\n");
			return;
		}
		//printf("STAG is 0x%x\n", stag);
	}

	/* set vlan member */
	value |= (vlan_mem << 16);
	value |= (1 << 30);	//IVL=1
	value |= ((stag & 0xfff) << 4);	//stag
	value |= 1;		//valid

	if (argc > 7) {
		eg_con = strtoul(argv[7], NULL, 2);
		eg_con = !!eg_con;
		value |= (eg_con << 29);	//eg_con
		value |= (1 << 28);	//eg tag control enable
	}

	if (argc > 8 && !eg_con) {
		if (strlen(argv[8]) != 8) {
			printf("egtag portmap format error, should be of length 7\n");
			return;
		}

		for (i = 0; i < 8; i++) {
			if (argv[8][i] < '0' || argv[8][i] > '3') {
				printf("egtag portmap format error, should be of combination of 0 or 3\n");
				return;
			}
			//eg_tag += (argv[8][i] - '0') * (1 << i * 2);
			eg_tag |= (argv[8][i] - '0') << (i * 2);
		}

		value |= (1 << 28);	//eg tag control enable
		value2 &= ~(0xffff);
		value2 |= eg_tag;
	}
	reg_write(REG_VAWD1_ADDR, value);
	reg_write(REG_VAWD2_ADDR, value2);
	//printf("VAWD1=0x%08x VAWD2=0x%08x ", value, value2);

	value = (0x80001000 + vid);	//w_vid_cmd
	reg_write(REG_VTCR_ADDR, value);
	//printf("VTCR=0x%08x\n", value);

	for (i = 0; i < 300; i++) {
		usleep(1000);
		reg_read(REG_VTCR_ADDR, &value);
		if ((value & 0x80000000) == 0)	//table busy
			break;
	}

	if (i == 300)
		printf("config vlan timeout.\n");
}

void igmp_on(int argc, char *argv[])
{
	unsigned int leaky_en = 0;
	unsigned int wan_num = 4;
	unsigned int port = 0, offset = 0, value = 0;
	char cmd[80];
	int ret;
	char *endptr;

	if (argc > 3) {
		errno = 0;
		leaky_en = strtoul(argv[3], &endptr, 10);
		if (errno != 0 || *endptr != '\0') {
			printf("Error: string converting\n");
			return;
		}
	}
	if (argc > 4) {
		errno = 0;
		wan_num = strtoul(argv[4], &endptr, 10);
		if (errno != 0 || *endptr != '\0') {
			printf("Error: string converting\n");
			return;
		}
	}

	if (leaky_en == 1) {
		if (wan_num == 4) {
			/* reg_write(0x2410, 0x810000c8); */
			reg_read(0x2410, &value);
			reg_write(0x2410, value | (1 << 3));
			/* reg_write(0x2010, 0x810000c0); */
			reg_read(0x2010, &value);
			reg_write(0x2010, value & (~(1 << 3)));
			reg_write(REG_ISC_ADDR, 0x10027d10);
		} else {
			/* reg_write(0x2010, 0x810000c8); */
			reg_read(0x2010, &value);
			reg_write(0x2010, value | (1 << 3));
			/* reg_write(0x2410, 0x810000c0); */
			reg_read(0x2410, &value);
			reg_write(0x2410, value & (~(1 << 3)));
			reg_write(REG_ISC_ADDR, 0x01027d01);
		}
	} else
		reg_write(REG_ISC_ADDR, 0x10027d60);

	reg_write(0x1c, 0x08100810);
	reg_write(0x2008, 0xb3ff);
	reg_write(0x2108, 0xb3ff);
	reg_write(0x2208, 0xb3ff);
	reg_write(0x2308, 0xb3ff);
	reg_write(0x2408, 0xb3ff);
	reg_write(0x2608, 0xb3ff);
	/* Enable Port ACL
	 * reg_write(0x2P04, 0xff0403);
	 */
	for (port = 0; port <= 6; port++) {
		offset = 0x2004 + port * 0x100;
		reg_read(offset, &value);
		reg_write(offset, value | (1 << 10));
	}

	/*IGMP query only p4 -> p5 */
	reg_write(0x94, 0x00ff0002);
	if (wan_num == 4)
		reg_write(0x98, 0x000a1008);
	else
		reg_write(0x98, 0x000a0108);
	reg_write(0x90, 0x80005000);
	reg_write(0x94, 0xff001100);
	if (wan_num == 4)
		reg_write(0x98, 0x000B1000);
	else
		reg_write(0x98, 0x000B0100);
	reg_write(0x90, 0x80005001);
	reg_write(0x94, 0x3);
	reg_write(0x98, 0x0);
	reg_write(0x90, 0x80009000);
	reg_write(0x94, 0x1a002080);
	reg_write(0x98, 0x0);
	reg_write(0x90, 0x8000b000);

	/*IGMP p5 -> p4 */
	reg_write(0x94, 0x00ff0002);
	reg_write(0x98, 0x000a2008);
	reg_write(0x90, 0x80005002);
	reg_write(0x94, 0x4);
	reg_write(0x98, 0x0);
	reg_write(0x90, 0x80009001);
	if (wan_num == 4)
		reg_write(0x94, 0x1a001080);
	else
		reg_write(0x94, 0x1a000180);
	reg_write(0x98, 0x0);
	reg_write(0x90, 0x8000b001);

	/*IGMP p0~p3 -> p6 */
	reg_write(0x94, 0x00ff0002);
	if (wan_num == 4)
		reg_write(0x98, 0x000a0f08);
	else
		reg_write(0x98, 0x000a1e08);
	reg_write(0x90, 0x80005003);
	reg_write(0x94, 0x8);
	reg_write(0x98, 0x0);
	reg_write(0x90, 0x80009002);
	reg_write(0x94, 0x1a004080);
	reg_write(0x98, 0x0);
	reg_write(0x90, 0x8000b002);

	/*IGMP query only p6 -> p0~p3 */
	reg_write(0x94, 0x00ff0002);
	reg_write(0x98, 0x000a4008);
	reg_write(0x90, 0x80005004);
	reg_write(0x94, 0xff001100);
	reg_write(0x98, 0x000B4000);
	reg_write(0x90, 0x80005005);
	reg_write(0x94, 0x30);
	reg_write(0x98, 0x0);
	reg_write(0x90, 0x80009003);
	if (wan_num == 4)
		reg_write(0x94, 0x1a000f80);
	else
		reg_write(0x94, 0x1a001e80);
	reg_write(0x98, 0x0);
	reg_write(0x90, 0x8000b003);

	/*Force eth2 to receive all igmp packets */
	ret = snprintf(cmd, sizeof(cmd),
		 "echo 2 > /sys/devices/virtual/net/%s/brif/%s/multicast_router",
		 BR_DEVNAME, ETH_DEVNAME);

	if (ret < 0 || ret >= sizeof(cmd))
		goto error;

	ret = system(cmd);
	if (ret)
		goto error;

	return;

error:
	printf("Failed to set /sys/devices/virtual/net/%s/brif/%s/multicast_router\n",
		BR_DEVNAME, ETH_DEVNAME);
}

void igmp_disable(int argc, char *argv[])
{
	unsigned int reg_offset = 0, value = 0;
	int port_num = 0;

	if (argc < 4) {
		printf("insufficient arguments!\n");
		return;
	}
	port_num = strtoul(argv[3], NULL, 0);
	if (port_num < 0 || 6 < port_num) {
		printf("wrong port range, should be within 0~6\n");
		return;
	}
	//set ISC: IGMP Snooping Control Register (offset: 0x0018)
	reg_offset = 0x2008;
	reg_offset |= (port_num << 8);
	value = 0x8000;

	reg_write(reg_offset, value);
}

void igmp_enable(int argc, char *argv[])
{
	unsigned int reg_offset = 0, value = 0;
	int port_num;

	if (argc < 4) {
		printf("insufficient arguments!\n");
		return;
	}
	port_num = strtoul(argv[3], NULL, 0);
	if (port_num < 0 || 6 < port_num) {
		printf("wrong port range, should be within 0~6\n");
		return;
	}
	//set ISC: IGMP Snooping Control Register (offset: 0x0018)
	reg_offset = 0x2008;
	reg_offset |= (port_num << 8);
	value = 0x9755;
	reg_write(reg_offset, value);
}

void igmp_off(int argc, char *argv[])
{
	unsigned int value = 0;
	//set ISC: IGMP Snooping Control Register (offset: 0x0018)
	reg_read(REG_ISC_ADDR, &value);
	value &= ~(1 << 18);	//disable
	reg_write(REG_ISC_ADDR, value);

	/*restore wan port multicast leaky vlan function: default disabled */
	reg_read(0x2010, &value);
	reg_write(0x2010, value & (~(1 << 3)));
	reg_read(0x2410, &value);
	reg_write(0x2410, value & (~(1 << 3)));

	printf("config igmpsnoop off.\n");
}

void switch_reset(int argc, char *argv[])
{
	if (chip_name == 0x7988)
		return;

	unsigned int value = 0;
	/*Software Register Reset  and Software System Reset */
	reg_write(0x7000, 0x3);
	reg_read(0x7000, &value);
	printf("SYS_CTRL(0x7000) register value =0x%x\n", value);
	if (chip_name == 0x7531) {
		reg_write(0x7c0c, 0x11111111);
		reg_read(0x7c0c, &value);
		printf("GPIO Mode (0x7c0c) select value =0x%x\n", value);
	}
	printf("Switch Software Reset !!!\n");
}

void phy_set_fc(int argc, char *argv[])
{
	unsigned int port = 0, pause_capable = 0;
	unsigned int phy_value = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT - 2) {
		printf("Error: wrong PHY port number, should be within 0~4\n");
		return;
	}

	errno = 0;
	pause_capable = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || pause_capable > 1) {
		printf("Illegal parameter, full_duplex_pause_capable:0|1\n");
		return;
	}

	printf("port=%d, full_duplex_pause_capable:%d\n", port, pause_capable);

	mii_mgr_read(port, 4, &phy_value);
	printf("read phy_value:0x%x\r\n", phy_value);
	phy_value &= (~(0x1 << 10));
	phy_value &= (~(0x1 << 11));
	if (pause_capable == 1) {
		phy_value |= (0x1 << 10);
		phy_value |= (0x1 << 11);
	}
	mii_mgr_write(port, 4, phy_value);
	printf("write phy_value:0x%x\r\n", phy_value);
}

void phy_set_an(int argc, char *argv[])
{
	unsigned int port = 0, auto_negotiation_en = 0;
	unsigned int phy_value = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT - 2) {
		printf("Error: wrong PHY port number, should be within 0~4\n");
		return;
	}

	errno = 0;
	auto_negotiation_en = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || auto_negotiation_en > 1) {
		printf("Illegal parameter, auto_negotiation_en:0|1\n");
		return;
	}

	printf("port=%d, auto_negotiation_en:%d\n", port, auto_negotiation_en);

	mii_mgr_read(port, 0, &phy_value);
	printf("read phy_value:0x%x\r\n", phy_value);
	phy_value &= (~(1 << 12));
	phy_value |= (auto_negotiation_en << 12);
	mii_mgr_write(port, 0, phy_value);
	printf("write phy_value:0x%x\r\n", phy_value);
}

void set_mac_pfc(int argc, char *argv[])
{
	unsigned int value = 0;
	unsigned int port, enable = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	enable = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || enable > 1) {
		printf("Error: Illegal paramete, enable|diable:0|1\n");
		return;
	}
	printf("enable: %d\n", enable);

	if (chip_name == 0x7531 || chip_name == 0x7988) {
		reg_read(REG_PFC_CTRL_ADDR, &value);
		value &= ~(1 << port);
		value |= (enable << port);
		printf("write reg: %x, value: %x\n", REG_PFC_CTRL_ADDR, value);
		reg_write(REG_PFC_CTRL_ADDR, value);
	} else
		printf("\nCommand not support by this chip.\n");
}

void global_set_mac_fc(int argc, char *argv[])
{
	unsigned char enable = 0;
	unsigned int value = 0, reg = 0;

	if (chip_name == 0x7530) {
		enable = atoi(argv[3]);
		printf("enable: %d\n", enable);

		/*Check the input parameters is right or not. */
		if (enable > 1) {
			printf(HELP_MACCTL_FC);
			return;
		}
		reg_write(0x7000, 0x3);
		reg = REG_GFCCR0_ADDR;
		reg_read(REG_GFCCR0_ADDR, &value);
		value &= (~REG_FC_EN_MASK);
		value |= (enable << REG_FC_EN_OFFT);
		printf("write reg: %x, value: %x\n", reg, value);
		reg_write(REG_GFCCR0_ADDR, value);
	} else
		printf("\r\nCommand not support by this chip.\n");
}				/*end mac_set_fc */

void qos_sch_select(int argc, char *argv[])
{
	unsigned char port = 0, queue = 0;
	unsigned char type = 0;
	unsigned int value = 0, reg = 0;
	char *endptr;

	if (argc < 7)
		return;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	queue = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || queue > 7) {
		printf("Error: wrong port queue member\n");
		return;
	}

	errno = 0;
	type = strtoul(argv[6], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || type > 2) {
		printf(HELP_QOS_TYPE);
		return;
	}

	printf("\r\nswitch qos type: %d.\n", type);

	if (!strncmp(argv[5], "min", 4)) {

		if (type == 0) {
			/*min sharper-->round roubin, disable min sharper rate limit */
			reg = GSW_MMSCR0_Q(queue) + 0x100 * port;
			reg_read(reg, &value);
			value = 0x0;
			reg_write(reg, value);
		} else if (type == 1) {
			/*min sharper-->sp, disable min sharper rate limit */
			reg = GSW_MMSCR0_Q(queue) + 0x100 * port;
			reg_read(reg, &value);
			value = 0x0;
			value |= (1 << 31);
			reg_write(reg, value);
		} else {
			printf("min sharper only support: rr or sp\n");
			return;
		}
	} else if (!strncmp(argv[5], "max", 4)) {
		if (type == 1) {
			/*max sharper-->sp, disable max sharper rate limit */
			reg = GSW_MMSCR1_Q(queue) + 0x100 * port;
			reg_read(reg, &value);
			value = 0x0;
			value |= (1 << 31);
			reg_write(reg, value);
		} else if (type == 2) {
			/*max sharper-->wfq, disable max sharper rate limit */
			reg = GSW_MMSCR1_Q(queue) + 0x100 * port;
			reg_read(reg, &value);
			value = 0x0;
			reg_write(reg, value);
		} else {
			printf("max sharper only support: wfq or sp\n");
			return;
		}
	} else {
		printf("\r\nIllegal sharper:%s\n", argv[5]);
		return;
	}
	printf("reg:0x%x--value:0x%x\n", reg, value);
}

void get_upw(unsigned int *value, unsigned char base)
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

void qos_set_base(int argc, char *argv[])
{
	unsigned char base = 0;
	unsigned char port = 0;
	unsigned int value = 0;
	char *endptr;

	if (argc < 5)
		return;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	base = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || base > 5) {
		printf(HELP_QOS_BASE);
		return;
	}

	printf("\r\nswitch qos base : %d. (port-based:0, tag-based:1,\
		dscp-based:2, acl-based:3, arl-based:4, stag-based:5)\n", base);
	if (chip_name == 0x7530) {

		reg_read(0x44, &value);
		get_upw(&value, base);
		reg_write(0x44, value);
		printf("reg: 0x44, value: 0x%x\n", value);

	} else if (chip_name == 0x7531 || chip_name == 0x7988) {

		reg_read(GSW_UPW(port), &value);
		get_upw(&value, base);
		reg_write(GSW_UPW(port), value);
		printf("reg:0x%x, value: 0x%x\n", GSW_UPW(port), value);

	} else {
		printf("unknown switch device");
		return;
	}
}

void qos_wfq_set_weight(int argc, char *argv[])
{
	int port = 0, weight[8], i = 0;
	unsigned char queue = 0;
	unsigned int reg = 0, value = 0;

	port = atoi(argv[3]);

	for (i = 0; i < 8; i++)
		weight[i] = atoi(argv[i + 4]);

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
	printf("port: %x, q0: %x, q1: %x, q2: %x, q3: %x, \
		q4: %x, q5: %x, q6: %x, q7: %x\n", port, weight[0], weight[1], weight[2], weight[3], weight[4], weight[5], weight[6], weight[7]);

	for (queue = 0; queue < 8; queue++) {
		reg = GSW_MMSCR1_Q(queue) + 0x100 * port;
		reg_read(reg, &value);
		value &= (~(0xf << 24));	//bit24~27
		value |= (((weight[queue] - 1) & 0xf) << 24);
		printf("reg: %x, value: %x\n", reg, value);
		reg_write(reg, value);
	}
}

void qos_set_portpri(int argc, char *argv[])
{
	unsigned char port = 0, prio = 0;
	unsigned int value = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	prio = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || prio > 7) {
		printf("Error: wrong priority, should be within 0~7\n");
		return;
	}

	reg_read(GSW_PCR(port), &value);
	value &= (~(0x7 << 24));
	value |= (prio << 24);
	reg_write(GSW_PCR(port), value);
	printf("write reg: %x, value: %x\n", GSW_PCR(port), value);
}

void qos_set_dscppri(int argc, char *argv[])
{
	unsigned char prio = 0, dscp = 0, pim_n = 0, pim_offset = 0;
	unsigned int value = 0, reg = 0;
	char *endptr;

	errno = 0;
	dscp = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || dscp > 63) {
		printf(HELP_QOS_DSCP_PRIO);
		return;
	}

	errno = 0;
	prio = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || prio > 7) {
		printf(HELP_QOS_DSCP_PRIO);
		return;
	}

	pim_n = dscp / 10;
	pim_offset = (dscp - pim_n * 10) * 3;
	reg = 0x0058 + pim_n * 4;
	reg_read(reg, &value);
	value &= (~(0x7 << pim_offset));
	value |= ((prio & 0x7) << pim_offset);
	reg_write(reg, value);
	printf("write reg: %x, value: %x\n", reg, value);
}

void qos_pri_mapping_queue(int argc, char *argv[])
{
	unsigned char prio = 0, queue = 0, pem_n = 0, port = 0;
	unsigned int value = 0, reg = 0;
	char *endptr;

	if (argc < 6)
		return;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	prio = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || prio > 7) {
		printf(HELP_QOS_PRIO_QMAP);
		return;
	}

	errno = 0;
	queue = strtoul(argv[5], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || queue > 7) {
		printf(HELP_QOS_PRIO_QMAP);
		return;
	}

	if (chip_name == 0x7530) {
		pem_n = prio / 2;
		reg = pem_n * 0x4 + 0x48;
		reg_read(reg, &value);
		if (prio % 2) {
			value &= (~(0x7 << 24));
			value |= ((queue & 0x7) << 24);
		} else {
			value &= (~(0x7 << 8));
			value |= ((queue & 0x7) << 8);
		}
		reg_write(reg, value);
		printf("write reg: %x, value: %x\n", reg, value);
	} else if (chip_name == 0x7531 || chip_name == 0x7988) {
		pem_n = prio / 2;
		reg = GSW_PEM(pem_n) + 0x100 * port;
		reg_read(reg, &value);
		if (prio % 2) {	// 1 1
			value &= (~(0x7 << 25));
			value |= ((queue & 0x7) << 25);
		} else {	// 0 0
			value &= (~(0x7 << 9));
			value |= ((queue & 0x7) << 9);
		}
		reg_write(reg, value);
		printf("write reg: %x, value: %x\n", reg, value);
	} else {
		printf("unknown switch device");
		return;
	}
}

static int macMT753xVlanSetVid(unsigned char index, unsigned char active,
			       unsigned short vid, unsigned char portMap,
			       unsigned char tagPortMap, unsigned char ivl_en,
			       unsigned char fid, unsigned short stag)
{
	unsigned int value = 0;
	unsigned int value2 = 0;
	unsigned int reg = 0;
	int i = 0;

	printf("index: %x, active: %x, vid: %x, portMap: %x, \
		tagPortMap: %x, ivl_en: %x, fid: %x, stag: %x\n", index, active, vid, portMap, tagPortMap, ivl_en, fid, stag);

	value = (portMap << 16);
	value |= (stag << 4);
	value |= (ivl_en << 30);
	value |= (fid << 1);
	value |= (active ? 1 : 0);

	// total 7 ports
	for (i = 0; i < 7; i++) {
		if (tagPortMap & (1 << i))
			value2 |= 0x2 << (i * 2);
	}

	if (value2)
		value |= (1 << 28);	// eg_tag

	reg = 0x98;		// VAWD2
	reg_write(reg, value2);

	reg = 0x94;		// VAWD1
	reg_write(reg, value);

	reg = 0x90;		// VTCR
	value = (0x80001000 + vid);
	reg_write(reg, value);

	reg = 0x90;		// VTCR
	while (1) {
		reg_read(reg, &value);
		if ((value & 0x80000000) == 0)	//table busy
			break;
	}

	/* switch clear */
	reg = 0x80;
	reg_write(reg, 0x8002);
	usleep(5000);
	reg_read(reg, &value);

	printf("SetVid: index:%d active:%d vid:%d portMap:%x tagPortMap:%x\r\n",
			index, active, vid, portMap, tagPortMap);
	return 0;

}				/*end macMT753xVlanSetVid */

static int macMT753xVlanSetPvid(unsigned char port, unsigned short pvid)
{
	unsigned int value = 0;
	unsigned int reg = 0;

	/*Parameters is error */
	if (port > 6)
		return -1;

	reg = 0x2014 + (port * 0x100);
	reg_read(reg, &value);
	value &= ~0xfff;
	value |= pvid;
	reg_write(reg, value);

	/* switch clear */
	reg = 0x80;
	reg_write(reg, 0x8002);
	usleep(5000);
	reg_read(reg, &value);

	printf("SetPVID: port:%d pvid:%d\r\n", port, pvid);
	return 0;
}

void doVlanSetPvid(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned short pvid = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	pvid = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || pvid > MAX_VID_VALUE) {
		printf(HELP_VLAN_PVID);
		return;
	}

	macMT753xVlanSetPvid(port, pvid);

	printf("port:%d pvid:%d,vlancap: max_port:%d maxvid:%d\r\n",
			port, pvid, SWITCH_MAX_PORT, MAX_VID_VALUE);
}

void doVlanSetVid(int argc, char *argv[])
{
	unsigned char index = 0, active = 0;
	unsigned char portMap = 0, tagPortMap = 0;
	unsigned short vid = 0, stag = 0;
	unsigned char ivl_en = 0, fid = 0;
	char *endptr;

	errno = 0;
	index = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || index >= MAX_VLAN_RULE) {
		printf(HELP_VLAN_VID);
		return;
	}

	errno = 0;
	active = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || active > ACTIVED) {
		printf(HELP_VLAN_VID);
		return;
	}

	errno = 0;
	vid = strtoul(argv[5], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || vid >= 4096) {
		printf(HELP_VLAN_VID);
		return;
	}

	errno = 0;
	portMap = strtoul(argv[6], &endptr, 10);
	if (errno != 0 || *endptr != '\0') {
		printf(HELP_VLAN_VID);
		return;
	}

	errno = 0;
	tagPortMap = strtoul(argv[7], &endptr, 10);
	if (errno != 0 || *endptr != '\0') {
		printf(HELP_VLAN_VID);
		return;
	}

	printf("subcmd parameter argc = %d\r\n", argc);
	if (argc >= 9) {
		errno = 0;
		ivl_en = strtoul(argv[8], &endptr, 10);
		if (errno != 0 || *endptr != '\0') {
			printf(HELP_VLAN_VID);
			return;
		}
		if (argc >= 10) {
			errno = 0;
			fid = strtoul(argv[9], &endptr, 16);
			if (errno != 0 || *endptr != '\0') {
				printf(HELP_VLAN_VID);
				return;
			}
			if (argc >= 11) {
				errno = 0;
				stag = strtoul(argv[10], &endptr, 10);
				if (errno != 0 || *endptr != '\0') {
					printf(HELP_VLAN_VID);
					return;
				}
			}
		}
	}
	macMT753xVlanSetVid(index, active, vid, portMap, tagPortMap,
						ivl_en, fid, stag);
	printf("index:%d active:%d vid:%d\r\n", index, active, vid);
}

void doVlanSetAccFrm(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char type = 0;
	unsigned int value = 0;
	unsigned int reg = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	type = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || type > REG_PVC_ACC_FRM_RELMASK) {
		printf(HELP_VLAN_ACC_FRM);
		return;
	}

	printf("port: %d, type: %d\n", port, type);

	reg = REG_PVC_P0_ADDR + port * 0x100;
	reg_read(reg, &value);
	value &= (~REG_PVC_ACC_FRM_MASK);
	value |= ((unsigned int)type << REG_PVC_ACC_FRM_OFFT);

	printf("write reg: %x, value: %x\n", reg, value);
	reg_write(reg, value);
}

void doVlanSetPortAttr(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char attr = 0;
	unsigned int value = 0;
	unsigned int reg = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	attr = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || attr > 3) {
		printf(HELP_VLAN_PORT_ATTR);
		return;
	}

	printf("port: %x, attr: %x\n", port, attr);

	reg = 0x2010 + port * 0x100;
	reg_read(reg, &value);
	value &= (0xffffff3f);
	value |= (attr << 6);

	printf("write reg: %x, value: %x\n", reg, value);
	reg_write(reg, value);
}

void doVlanSetPortMode(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char mode = 0;
	unsigned int value = 0;
	unsigned int reg = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	mode = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || mode > 3) {
		printf(HELP_VLAN_PORT_MODE);
		return;
	}

	printf("port: %x, mode: %x\n", port, mode);

	reg = 0x2004 + port * 0x100;
	reg_read(reg, &value);
	value &= (~((1 << 0) | (1 << 1)));
	value |= (mode & 0x3);
	printf("write reg: %x, value: %x\n", reg, value);
	reg_write(reg, value);
}

void doVlanSetEgressTagPCR(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char eg_tag = 0;
	unsigned int value = 0;
	unsigned int reg = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	eg_tag = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || (eg_tag > REG_PCR_EG_TAG_RELMASK)) {
		printf(HELP_VLAN_EGRESS_TAG_PCR);
		return;
	}

	printf("port: %d, eg_tag: %d\n", port, eg_tag);

	reg = REG_PCR_P0_ADDR + port * 0x100;
	reg_read(reg, &value);
	value &= (~REG_PCR_EG_TAG_MASK);
	value |= ((unsigned int)eg_tag << REG_PCR_EG_TAG_OFFT);

	printf("write reg: %x, value: %x\n", reg, value);
	reg_write(reg, value);

}				/*end doVlanSetEgressTagPCR */

void doVlanSetEgressTagPVC(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char eg_tag = 0;
	unsigned int value = 0;
	unsigned int reg = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	eg_tag = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || (eg_tag > REG_PVC_EG_TAG_RELMASK)) {
		printf(HELP_VLAN_EGRESS_TAG_PVC);
		return;
	}

	printf("port: %d, eg_tag: %d\n", port, eg_tag);

	reg = REG_PVC_P0_ADDR + port * 0x100;
	reg_read(reg, &value);
	value &= (~REG_PVC_EG_TAG_MASK);
	value |= ((unsigned int)eg_tag << REG_PVC_EG_TAG_OFFT);

	printf("write reg: %x, value: %x\n", reg, value);
	reg_write(reg, value);
}				/*end doVlanSetEgressTagPVC */

void doArlAging(int argc, char *argv[])
{
	unsigned char aging_en = 0;
	unsigned int time = 0, aging_cnt = 0, aging_unit = 0;
	unsigned int value = 0, reg = 0;
	char *endptr;

	errno = 0;
	aging_en = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || aging_en > 1) {
		printf(HELP_ARL_AGING);
		return;
	}

	errno = 0;
	time = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || (time <= 0 || time > 65536)) {
		printf(HELP_ARL_AGING);
		return;
	}

	printf("aging_en: %x, aging time: %x\n", aging_en, time);

	reg = 0xa0;
	reg_read(reg, &value);
	value &= (~(1 << 20));
	if (!aging_en)
		value |= (1 << 20);

	aging_unit = (time / 0x100) + 1;
	aging_cnt = (time / aging_unit);
	aging_unit--;
	aging_cnt--;

	value &= (0xfff00000);
	value |= ((aging_cnt << 12) | aging_unit);

	printf("aging_unit: %x, aging_cnt: %x\n", aging_unit, aging_cnt);
	printf("write reg: %x, value: %x\n", reg, value);

	reg_write(reg, value);
}

void doMirrorEn(int argc, char *argv[])
{
	unsigned char mirror_en = 0;
	unsigned char mirror_port = 0;
	unsigned int value = 0, reg = 0;
	char *endptr;

	errno = 0;
	mirror_en = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || mirror_en > 1) {
		printf(HELP_MIRROR_EN);
		return;
	}

	errno = 0;
	mirror_port = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || mirror_port > REG_CFC_MIRROR_PORT_RELMASK) {
		printf(HELP_MIRROR_EN);
		return;
	}

	printf("mirror_en: %d, mirror_port: %d\n", mirror_en, mirror_port);

	reg = REG_CFC_ADDR;
	reg_read(reg, &value);
	value &= (~REG_CFC_MIRROR_EN_MASK);
	value |= (mirror_en << REG_CFC_MIRROR_EN_OFFT);
	value &= (~REG_CFC_MIRROR_PORT_MASK);
	value |= (mirror_port << REG_CFC_MIRROR_PORT_OFFT);

	printf("write reg: %x, value: %x\n", reg, value);
	reg_write(reg, value);

}				/*end doMirrorEn */

void doMirrorPortBased(int argc, char *argv[])
{
	unsigned char port = 0, port_tx_mir = 0, port_rx_mir = 0, vlan_mis =
	    0, acl_mir = 0, igmp_mir = 0;
	unsigned int value = 0, reg = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT)
		goto error;

	errno = 0;
	port_tx_mir = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port_tx_mir > 1)
		goto error;

	errno = 0;
	port_rx_mir = strtoul(argv[5], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port_rx_mir > 1)
		goto error;

	errno = 0;
	acl_mir = strtoul(argv[6], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || acl_mir > 1)
		goto error;

	errno = 0;
	vlan_mis = strtoul(argv[7], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || vlan_mis > 1)
		goto error;

	errno = 0;
	igmp_mir = strtoul(argv[8], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || igmp_mir > 1)
		goto error;

	printf("port:%d, port_tx_mir:%d, port_rx_mir:%d, acl_mir:%d, vlan_mis:%d, igmp_mir:%d\n",
			port, port_tx_mir, port_rx_mir, acl_mir, vlan_mis, igmp_mir);

	reg = REG_PCR_P0_ADDR + port * 0x100;
	reg_read(reg, &value);
	value &=
	    ~(REG_PORT_TX_MIR_MASK | REG_PORT_RX_MIR_MASK | REG_PCR_ACL_MIR_MASK
	      | REG_PCR_VLAN_MIS_MASK);
	value |=
	    (port_tx_mir << REG_PORT_TX_MIR_OFFT) +
	    (port_rx_mir << REG_PORT_RX_MIR_OFFT);
	value |=
	    (acl_mir << REG_PCR_ACL_MIR_OFFT) +
	    (vlan_mis << REG_PCR_VLAN_MIS_OFFT);

	printf("write reg: %x, value: %x\n", reg, value);
	reg_write(reg, value);

	reg = REG_PIC_P0_ADDR + port * 0x100;
	reg_read(reg, &value);
	value &= ~(REG_PIC_IGMP_MIR_MASK);
	value |= (igmp_mir << REG_PIC_IGMP_MIR_OFFT);

	printf("write reg: %x, value: %x\n", reg, value);
	reg_write(reg, value);
	return;

error:
	printf(HELP_MIRROR_PORTBASED);
	return;
}				/*end doMirrorPortBased */

void doStp(int argc, char *argv[])
{
	unsigned char port = 0;
	unsigned char fid = 0;
	unsigned char state = 0;
	unsigned int value = 0;
	unsigned int reg = 0;
	char *endptr;

	errno = 0;
	port = strtoul(argv[2], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	fid = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || fid > 7) {
		printf(HELP_STP);
		return;
	}

	errno = 0;
	state = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || state > 3) {
		printf(HELP_STP);
		return;
	}

	printf("port: %d, fid: %d, state: %d\n", port, fid, state);

	reg = REG_SSC_P0_ADDR + port * 0x100;
	reg_read(reg, &value);
	value &= (~(3 << (fid << 2)));
	value |= ((unsigned int)state << (fid << 2));

	printf("write reg: %x, value: %x\n", reg, value);
	reg_write(reg, value);
}

void _ingress_rate_set(int on_off, int port, int bw)
{
	unsigned int reg = 0, value = 0;

	reg = 0x1800 + (0x100 * port);
	value = 0;
	/*token-bucket */
	if (on_off == 1) {
		if (chip_name == 0x7530) {
			if (bw > 1000000) {
				printf("\n**Charge rate(%d) is larger than line rate(1000000kbps)**\n", bw);
				return;
			}
			value = ((bw / 32) << 16) + (1 << 15) + (7 << 8) + (1 << 7) + 0x0f;
		} else if (chip_name == 0x7531 || chip_name == 0x7988) {
			if ((chip_name == 0x7531) && (bw > 2500000)) {
				printf("\n**Charge rate(%d) is larger than line rate(2500000kbps)**\n", bw);
				return;
			}

			if ((chip_name == 0x7988) && (bw > 4000000)) {
				printf("\n**Charge rate(%d) is larger than line rate(4000000kbps)**\n", bw);
				return;
			}

			if (bw / 32 >= 65536)	//supoort 2.5G case
				value = ((bw / 32) << 16) + (1 << 15) + (1 << 14) + (1 << 12) +
					(7 << 8) + 0xf;
			else
				value = ((bw / 32) << 16) + (1 << 15) + (1 << 14) + (7 << 8) + 0xf;
		} else
			printf("unknow chip\n");
	}
#if leaky_bucket
	reg_read(reg, &value);
	value &= 0xffff0000;
	if (on_off == 1) {
		value |= on_off << 15;
		//7530 same as 7531
		if (bw < 100) {
			value |= (0x0 << 8);
			value |= bw;
		} else if (bw < 1000) {
			value |= (0x1 << 8);
			value |= bw / 10;
		} else if (bw < 10000) {
			value |= (0x2 << 8);
			value |= bw / 100;
		} else if (bw < 100000) {
			value |= (0x3 << 8);
			value |= bw / 1000;
		} else {
			value |= (0x4 << 8);
			value |= bw / 10000;
		}
	}
#endif
	reg_write(reg, value);
	reg = 0x1FFC;
	reg_read(reg, &value);
	value = 0x110104;
	reg_write(reg, value);

	if (on_off)
		printf("switch port=%d, bw=%d\n", port, bw);
	else
		printf("switch port=%d ingress rate limit off\n", port);
}

void ingress_rate_set(int argc, char *argv[])
{
	int on_off = 0, port = 0, bw = 0;
	char *endptr;

	/* clear errno before conversion to detect overflow */
	errno = 0;
	port = strtoul(argv[3], &endptr, 0);

	if (errno == ERANGE) {
		printf("Conversion error, value out of range\n");
		return;
	}
	if (*endptr != '\0') {
		printf("Conversion error, no digits were found\n");
		return;
	}

	if (port < 0 || port > 6) {
		printf("Wrong port range, should be within 0-6\n");
		return;
	}

	if (argv[2][1] == 'n') {
		errno = 0;
		bw = strtoul(argv[4], &endptr, 0);
		if (errno == ERANGE) {
			printf("Conversion error, value out of range\n");
			return;
		}
		if (*endptr != '\0') {
			printf("Conversion error, no digits were found\n");
			return;
		}
		on_off = 1;
	} else if (argv[2][1] == 'f') {
		if (argc != 4)
			return;
		on_off = 0;
	}

	_ingress_rate_set(on_off, port, bw);
}

void _egress_rate_set(int on_off, int port, int bw)
{
	unsigned int value = 0, reg = 0;

	reg = 0x1040 + (0x100 * port);
	value = 0;
	/*token-bucket */
	if (on_off == 1) {
		if (chip_name == 0x7530) {
			if (bw < 0 || bw > 1000000) {
				printf("\n**Charge rate(%d) is larger than line rate(1000000kbps)**\n", bw);
				return;
			}
			value = ((bw / 32) << 16) + (1 << 15) + (7 << 8) + (1 << 7) + 0xf;
		} else if (chip_name == 0x7531 || chip_name == 0x7988) {
			if ((chip_name == 0x7531) && (bw < 0 || bw > 2500000)) {
				printf("\n**Charge rate(%d) is larger than line rate(2500000kbps)**\n", bw);
				return;
			}
			if ((chip_name == 0x7988) && (bw < 0 || bw > 4000000)) {
				printf("\n**Charge rate(%d) is larger than line rate(4000000kbps)**\n", bw);
				return;
			}

			if (bw / 32 >= 65536)	//support 2.5G cases
				value = ((bw / 32) << 16) + (1 << 15) + (1 << 14) + (1 << 12) +
					(7 << 8) + 0xf;
			else
				value = ((bw / 32) << 16) + (1 << 15) + (1 << 14) + (7 << 8) + 0xf;
		} else
			printf("unknow chip\n");
	}
	reg_write(reg, value);
	reg = 0x10E0;
	reg_read(reg, &value);
	value &= 0x18;
	reg_write(reg, value);

	if (on_off)
		printf("switch port=%d, bw=%d\n", port, bw);
	else
		printf("switch port=%d egress rate limit off\n", port);
}

void egress_rate_set(int argc, char *argv[])
{
	unsigned int value = 0, reg = 0;
	int on_off = 0, port = 0, bw = 0;
	char *endptr;

	/* clear errno before conversion to detect overflow */
	errno = 0;
	port = strtoul(argv[3], &endptr, 0);
	if (errno == ERANGE) {
		printf("Conversion error, value out of range\n");
		return;
	}
	if (*endptr != '\0') {
		printf("Conversion error, no digits were found\n");
		return;
	}
	if (port < 0 || port > 6) {
		printf("Wrong port range, should be within 0-6\n");
		return;
	}
	if (argv[2][1] == 'n') {
		errno = 0;
		bw = strtoul(argv[4], &endptr, 0);
		if (errno == ERANGE) {
			printf("Conversion error, value out of range\n");
			return;
		}
		if (*endptr != '\0') {
			printf("Conversion error, no digits were found\n");
			return;
		}
		on_off = 1;
	} else if (argv[2][1] == 'f') {
		if (argc != 4)
			return;
		on_off = 0;
	}

	_egress_rate_set(on_off, port, bw);
}

void rate_control(int argc, char *argv[])
{
	unsigned char dir = 0;
	unsigned char port = 0;
	unsigned int rate = 0;
	char *endptr;

	errno = 0;
	dir = strtoul(argv[2], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || dir > 1) {
		printf("Error: wrong port member, should be 0:egress, 1:ingress\n");
		return;
	}

	errno = 0;
	port = strtoul(argv[3], &endptr, 10);
	if (errno != 0 || *endptr != '\0' || port > MAX_PORT) {
		printf("Error: wrong port member, should be within 0~%d\n", MAX_PORT);
		return;
	}

	errno = 0;
	rate = strtoul(argv[4], &endptr, 10);
	if (errno != 0 || *endptr != '\0') {
		printf("Error: wrong traffic rate, unit is kbps\n");
		return;
	}

	if (dir == 1)		//ingress
		_ingress_rate_set(1, port, rate);
	else if (dir == 0)	//egress
		_egress_rate_set(1, port, rate);
}

void collision_pool_enable(int argc, char *argv[])
{

	unsigned char enable = 0;
	unsigned int value = 0, reg = 0;

	enable = atoi(argv[3]);

	/*Check the input parameters is right or not. */
	if (enable > 1) {
		printf(HELP_COLLISION_POOL_EN);
		return;
	}

	printf("collision pool enable: %d\n", enable);
	if (chip_name == 0x7531 || chip_name == 0x7988) {
		reg = REG_CPGC_ADDR;
		if (enable == 1) {
			/* active reset */
			reg_read(reg, &value);
			value &= (~REG_CPCG_COL_RST_N_MASK);
			reg_write(reg, value);

			/* enanble clock */
			reg_read(reg, &value);
			value &= (~REG_CPCG_COL_CLK_EN_MASK);
			value |= (1 << REG_CPCG_COL_CLK_EN_OFFT);
			reg_write(reg, value);

			/* inactive reset */
			reg_read(reg, &value);
			value &= (~REG_CPCG_COL_RST_N_MASK);
			value |= (1 << REG_CPCG_COL_RST_N_OFFT);
			reg_write(reg, value);

			/* enable collision pool */
			reg_read(reg, &value);
			value &= (~REG_CPCG_COL_EN_MASK);
			value |= (1 << REG_CPCG_COL_EN_OFFT);
			reg_write(reg, value);

			reg_read(reg, &value);
			printf("write reg: %x, value: %x\n", reg, value);
		} else {

			/* disable collision pool */
			reg_read(reg, &value);
			value &= (~REG_CPCG_COL_EN_MASK);
			reg_write(reg, value);

			/* active reset */
			reg_read(reg, &value);
			value &= (~REG_CPCG_COL_RST_N_MASK);
			reg_write(reg, value);

			/* inactive reset */
			reg_read(reg, &value);
			value &= (~REG_CPCG_COL_RST_N_MASK);
			value |= (1 << REG_CPCG_COL_RST_N_OFFT);
			reg_write(reg, value);

			/* disable clock */
			reg_read(reg, &value);
			value &= (~REG_CPCG_COL_CLK_EN_MASK);
			reg_write(reg, value);

			reg_read(reg, &value);
			printf("write reg: %x, value: %x\n", reg, value);

		}
	} else {
		printf("\nCommand not support by this chip.\n");
	}
}

void collision_pool_mac_dump(int argc, char *argv[])
{
	unsigned int value = 0, reg = 0;

	if (chip_name == 0x7531 || chip_name == 0x7988) {
		reg = REG_CPGC_ADDR;
		reg_read(reg, &value);
		if (value & REG_CPCG_COL_EN_MASK)
			table_dump_internal(COLLISION_TABLE);
		else
			printf("\ncollision pool is disabled, please enable it before use this command.\n");
	} else {
		printf("\nCommand not support by this chip.\n");
	}
}

void collision_pool_dip_dump(int argc, char *argv[])
{
	unsigned int value = 0, reg = 0;

	if (chip_name == 0x7531 || chip_name == 0x7988) {
		reg = REG_CPGC_ADDR;
		reg_read(reg, &value);
		if (value & REG_CPCG_COL_EN_MASK)
			dip_dump_internal(COLLISION_TABLE);
		else
			printf("\ncollision pool is disabled, please enable it before use this command.\n");
	} else {
		printf("\nCommand not support by this chip.\n");
	}
}

void collision_pool_sip_dump(int argc, char *argv[])
{
	unsigned int value = 0, reg = 0;

	if (chip_name == 0x7531 || chip_name == 0x7988) {
		reg = REG_CPGC_ADDR;
		reg_read(reg, &value);
		if (value & REG_CPCG_COL_EN_MASK)
			sip_dump_internal(COLLISION_TABLE);
		else
			printf("\ncollision pool is disabled, please enable it before use this command.\n");
	} else {
		printf("\nCommand not support by this chip.\n");
	}
}

void pfc_get_rx_counter(int argc, char *argv[])
{
	int port = 0;
	unsigned int value = 0, reg = 0;
	unsigned int user_pri = 0;

	port = strtoul(argv[3], NULL, 0);
	if (port < 0 || 6 < port) {
		printf("wrong port range, should be within 0~6\n");
		return;
	}

	if (chip_name == 0x7531 || chip_name == 0x7988) {
		reg = PFC_RX_COUNTER_L(port);
		reg_read(reg, &value);
		user_pri = value & 0xff;
		printf("\n port %d rx pfc (up=0)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff00) >> 8;
		printf("\n port %d rx pfc (up=1)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff0000) >> 16;
		printf("\n port %d rx pfc (up=2)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff000000) >> 24;
		printf("\n port %d rx pfc (up=3)pause on counter is %d.\n",
		       port, user_pri);

		reg = PFC_RX_COUNTER_H(port);
		reg_read(reg, &value);
		user_pri = value & 0xff;
		printf("\n port %d rx pfc (up=4)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff00) >> 8;
		printf("\n port %d rx pfc (up=5)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff0000) >> 16;
		printf("\n port %d rx pfc (up=6)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff000000) >> 24;
		printf("\n port %d rx pfc (up=7)pause on counter is %d.\n",
		       port, user_pri);

		/* for rx counter could be updated successfully */
		reg_read(PMSR_P(port), &value);
		reg_read(PMSR_P(port), &value);
	} else {
		printf("\nCommand not support by this chip.\n");
	}

}

void pfc_get_tx_counter(int argc, char *argv[])
{
	int port = 0;
	unsigned int value = 0, reg = 0;
	unsigned int user_pri = 0;

	port = strtoul(argv[3], NULL, 0);
	if (port < 0 || 6 < port) {
		printf("wrong port range, should be within 0~6\n");
		return;
	}

	if (chip_name == 0x7531 || chip_name == 0x7988) {
		reg = PFC_TX_COUNTER_L(port);
		reg_read(reg, &value);
		user_pri = value & 0xff;
		printf("\n port %d tx pfc (up=0)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff00) >> 8;
		printf("\n port %d tx pfc (up=1)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff0000) >> 16;
		printf("\n port %d tx pfc (up=2)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff000000) >> 24;
		printf("\n port %d tx pfc (up=3)pause on counter is %d.\n",
		       port, user_pri);

		reg = PFC_TX_COUNTER_H(port);
		reg_read(reg, &value);
		user_pri = value & 0xff;
		printf("\n port %d tx pfc (up=4)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff00) >> 8;
		printf("\n port %d tx pfc (up=5)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff0000) >> 16;
		printf("\n port %d tx pfc (up=6)pause on counter is %d.\n",
		       port, user_pri);
		user_pri = (value & 0xff000000) >> 24;
		printf("\n port %d tx pfc (up=7)pause on counter is %d.\n",
		       port, user_pri);

		/* for tx counter could be updated successfully */
		reg_read(PMSR_P(port), &value);
		reg_read(PMSR_P(port), &value);
	} else {
		printf("\nCommand not support by this chip.\n");
	}
}

void read_output_queue_counters(int argc, char *argv[])
{
	unsigned int port = 0;
	unsigned int value = 0, output_queue = 0;
	unsigned int base = 0x220;

	for (port = 0; port < 7; port++) {
		reg_write(0x7038, base + (port * 4));
		reg_read(0x7034, &value);
		output_queue = value & 0xff;
		printf("\n port %d  output queue 0 counter is %d.\n", port,
		       output_queue);
		output_queue = (value & 0xff00) >> 8;
		printf("\n port %d  output queue 1 counter is %d.\n", port,
		       output_queue);

		reg_write(0x7038, base + (port * 4) + 1);
		reg_read(0x7034, &value);
		output_queue = value & 0xff;
		printf("\n port %d  output queue 2 counter is %d.\n", port,
		       output_queue);
		output_queue = (value & 0xff00) >> 8;
		printf("\n port %d  output queue 3 counter is %d.\n", port,
		       output_queue);

		reg_write(0x7038, base + (port * 4) + 2);
		reg_read(0x7034, &value);
		output_queue = value & 0xff;
		printf("\n port %d  output queue 4 counter is %d.\n", port,
		       output_queue);
		output_queue = (value & 0xff00) >> 8;
		printf("\n port %d  output queue 5 counter is %d.\n", port,
		       output_queue);

		reg_write(0x7038, base + (port * 4) + 3);
		reg_read(0x7034, &value);
		output_queue = value & 0xff;
		printf("\n port %d  output queue 6 counter is %d.\n", port,
		       output_queue);
		output_queue = (value & 0xff00) >> 8;
		printf("\n port %d  output queue 7 counter is %d.\n", port,
		       output_queue);
	}
}

void read_free_page_counters(int argc, char *argv[])
{
	unsigned int value = 0;
	unsigned int free_page = 0, free_page_last_read = 0;
	unsigned int fc_free_blk_lothd = 0, fc_free_blk_hithd = 0;
	unsigned int fc_port_blk_thd = 0, fc_port_blk_hi_thd = 0;
	unsigned int queue[8] = { 0 };

	if (chip_name == 0x7531 || chip_name == 0x7988) {
		/* get system free page link counter */
		reg_read(0x1fc0, &value);
		free_page = value & 0xFFF;
		free_page_last_read = (value & 0xFFF0000) >> 16;

		/* get system flow control waterwark */
		reg_read(0x1fe0, &value);
		fc_free_blk_lothd = value & 0x3FF;
		fc_free_blk_hithd = (value & 0x3FF0000) >> 16;

		/* get port flow control waterwark */
		reg_read(0x1fe4, &value);
		fc_port_blk_thd = value & 0x3FF;
		fc_port_blk_hi_thd = (value & 0x3FF0000) >> 16;

		/* get queue flow control waterwark */
		reg_read(0x1fe8, &value);
		queue[0] = value & 0x3F;
		queue[1] = (value & 0x3F00) >> 8;
		queue[2] = (value & 0x3F0000) >> 16;
		queue[3] = (value & 0x3F000000) >> 24;
		reg_read(0x1fec, &value);
		queue[4] = value & 0x3F;
		queue[5] = (value & 0x3F00) >> 8;
		queue[6] = (value & 0x3F0000) >> 16;
		queue[7] = (value & 0x3F000000) >> 24;
	} else {
		/* get system free page link counter */
		reg_read(0x1fc0, &value);
		free_page = value & 0x3FF;
		free_page_last_read = (value & 0x3FF0000) >> 16;

		/* get system flow control waterwark */
		reg_read(0x1fe0, &value);
		fc_free_blk_lothd = value & 0xFF;
		fc_free_blk_hithd = (value & 0xFF00) >> 8;

		/* get port flow control waterwark */
		reg_read(0x1fe0, &value);
		fc_port_blk_thd = (value & 0xFF0000) >> 16;
		reg_read(0x1ff4, &value);
		fc_port_blk_hi_thd = (value & 0xFF00) >> 8;

		/* get queue flow control waterwark */
		reg_read(0x1fe4, &value);
		queue[0] = value & 0xF;
		queue[1] = (value & 0xF0) >> 4;
		queue[2] = (value & 0xF00) >> 8;
		queue[3] = (value & 0xF000) >> 12;
		queue[4] = (value & 0xF0000) >> 16;
		queue[5] = (value & 0xF00000) >> 20;
		queue[6] = (value & 0xF000000) >> 24;
		queue[7] = (value & 0xF0000000) >> 28;
	}

	printf("<===Free Page=======Current=======Last Read access=====>\n");
	printf("\n");
	printf(" page counter      %u                %u\n ",
	       free_page, free_page_last_read);
	printf("\n ");
	printf("=========================================================\n");
	printf("<===Type=======High threshold======Low threshold=========\n");
	printf("\n ");
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

void eee_enable(int argc, char *argv[])
{
	unsigned long enable = 0;
	unsigned int value = 0;
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
					printf("portmap format error, should be combination of 0 or 1\n");
					goto error;
				}
				port_map |= ((argv[3][p] - '0') << p);
			}
		} else {
			printf("port_no or portmap format error, should be length of 1 or 5\n");
			goto error;
		}
	} else {
		port_map = 0x1f;
	}

	eee_cap = (enable) ? 6 : 0;
	for (p = 0; p < MAX_PHY_PORT; p++) {
		/* port_map describe p0p1p2p3p4 from left to rignt */
		if (!!(port_map & (1 << p)))
			mii_mgr_c45_write(p, 0x7, 0x3c, eee_cap);

		mii_mgr_c45_read(p, 0x7, 0x3c, &value);
		/* mt7531: Always readback eee_cap = 0 when global EEE switch
		 * is turned off.
		 */
		if (value | eee_cap)
			eee_en_bitmap |= (1 << (MAX_PHY_PORT - 1 - p));
	}

	/* Turn on/off global EEE switch */
	if (chip_name == 0x7531 || chip_name == 0x7988) {
		mii_mgr_c45_read(0, 0x1f, 0x403, &value);
		if (eee_en_bitmap)
			value |= (1 << 6);
		else
			value &= ~(1 << 6);
		mii_mgr_c45_write(0, 0x1f, 0x403, value);
	} else {
		printf("\nCommand not support by this chip.\n");
	}

	printf("EEE(802.3az) %s", (enable) ? "enable" : "disable");
	if (argc == 4) {
		if (port_num >= 0)
			printf(" port%ld", port_num);
		else
			printf(" port_map: %s", argv[3]);
	} else {
		printf(" all ports");
	}
	printf("\n");

	return;
error:
	printf(HELP_EEE_EN);
}

void eee_dump(int argc, char *argv[])
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

void dump_each_port(unsigned int base)
{
	unsigned int pkt_cnt = 0;
	int i = 0;

	for (i = 0; i < 7; i++) {
		if (chip_name == 0x7988) {
			if ((base == 0x402C) && (i == 6))
				base = 0x408C;
			else if ((base == 0x408C) && (i == 6))
				base = 0x402C;
			else
				;
		}
		reg_read((base) + (i * 0x100), &pkt_cnt);
		printf("%8u ", pkt_cnt);
	}
	printf("\n");
}

void read_mib_counters(int argc, char *argv[])
{
	printf("===================== %8s %8s %8s %8s %8s %8s %8s\n",
	       "Port0", "Port1", "Port2", "Port3", "Port4", "Port5", "Port6");
	printf("Tx Drop Packet      :");
	dump_each_port(0x4000);
	printf("Tx CRC Error        :");
	dump_each_port(0x4004);
	printf("Tx Unicast Packet   :");
	dump_each_port(0x4008);
	printf("Tx Multicast Packet :");
	dump_each_port(0x400C);
	printf("Tx Broadcast Packet :");
	dump_each_port(0x4010);
	printf("Tx Collision Event  :");
	dump_each_port(0x4014);
	printf("Tx Pause Packet     :");
	dump_each_port(0x402C);
	printf("Rx Drop Packet      :");
	dump_each_port(0x4060);
	printf("Rx Filtering Packet :");
	dump_each_port(0x4064);
	printf("Rx Unicast Packet   :");
	dump_each_port(0x4068);
	printf("Rx Multicast Packet :");
	dump_each_port(0x406C);
	printf("Rx Broadcast Packet :");
	dump_each_port(0x4070);
	printf("Rx Alignment Error  :");
	dump_each_port(0x4074);
	printf("Rx CRC Error	    :");
	dump_each_port(0x4078);
	printf("Rx Undersize Error  :");
	dump_each_port(0x407C);
	printf("Rx Fragment Error   :");
	dump_each_port(0x4080);
	printf("Rx Oversize Error   :");
	dump_each_port(0x4084);
	printf("Rx Jabber Error     :");
	dump_each_port(0x4088);
	printf("Rx Pause Packet     :");
	dump_each_port(0x408C);
}

void clear_mib_counters(int argc, char *argv[])
{
	reg_write(0x4fe0, 0xf0);
	read_mib_counters(argc, argv);
	reg_write(0x4fe0, 0x800000f0);
}

void exit_free(void)
{
	free(attres);
	attres = NULL;
	switch_ioctl_fini();
	mt753x_netlink_free();
}

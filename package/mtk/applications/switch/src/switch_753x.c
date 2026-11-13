/*
 * switch_753x.c: set for 753x switch
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <errno.h>

#include "switch_netlink.h"
#include "switch_ioctl.h"
#include "switch_fun.h"
#include "switch_fun_an8855.h"

#define SWITCH_APP_VERSION "1.0.7"

#define CLOSE_FILE(fp) do { \
	if (fclose(fp) == EOF) { \
		printf("Failed to close file: %s\n", strerror(errno)); \
	} \
} while (0)

struct mt753x_attr *attres;
int chip_name;
bool nl_init_flag;
bool air_skip_check_flag;
struct switch_func_s *p_switch_func;

static void usage(char *cmd)
{
	printf("==================Usage===============================================================================================================================\n");

	/* 1. basic operations */
	printf("1) mt753x switch Basic operations=================================================================================================================>>>>\n");
	printf(" 1.1) %s devs							- list switch device id and model name\n", cmd);
	printf(" 1.2) %s sysctl							- show the ways to access kenerl driver: netlink or ioctl\n", cmd);
	printf(" 1.3) %s reset							- sw reset switch fsm and registers\n", cmd);
	printf(" 1.4) %s reg r [offset]						- read the reg with default switch\n", cmd);
	printf(" 1.5) %s reg w [offset] [value]					- write the reg with default switch\n", cmd);
	printf(" 1.6) %s reg d [offset]						- dump the reg with default switch\n", cmd);
	printf(" 1.7) %s dev [devid] reg r [addr]				- read the reg with the switch devid\n", cmd);
	printf(" 1.8) %s dev [devid] reg w [addr] [value]			- write the regs with the switch devid\n", cmd);
	printf(" 1.9) %s dev [devid] reg d [addr]				- dump the regs with the switch devid\n", cmd);
	printf("\n");

	/* 2. phy operations */
	printf("2) mt753x switch PHY operations===================================================================================================================>>>>\n");
	printf(" 2.1) %s phy							- dump all phy registers (clause 22)\n", cmd);
	printf(" 2.2) %s phy [phy_addr]						- dump phy register of specific port (clause 22)\n", cmd);
	printf(" 2.3) %s phy cl22 r [port_num] [phy_reg]			- read specific phy register of specific port by clause 22\n", cmd);
	printf(" 2.4) %s phy cl22 w [port_num] [phy_reg] [value]		- write specific phy register of specific port by clause 22\n", cmd);
	printf(" 2.5) %s phy cl45 r [port_num] [dev_num] [phy_reg]		- read specific phy register of specific port by clause 45\n", cmd);
	printf(" 2.6) %s phy cl45 w [port_num] [dev_num] [phy_reg] [value]	- write specific phy register of specific port by clause 45\n", cmd);
	printf(" 2.7) %s phy fc [port_num] [enable 0|1]				- set switch phy flow control, port is 0~4, enable is 1, disable is 0\n", cmd);
	printf(" 2.8) %s phy an [port_num] [enable 0|1]				- set switch phy auto-negotiation, port is 0~4, enable is 1, disable is 0\n", cmd);
	printf(" 2.9) %s trreg r [port_num] [ch_addr] [node_addr] [data_addr]	- read phy token-ring of specific port\n", cmd);
	printf(" 2.10) %s trreg w [port_num] [ch_addr] [node_addr] [data_addr]	- write phy token-ring of specific port\n", cmd);
	printf("		[high_value] [low_value]\n");
	printf(" 2.11) %s crossover [port_num] [mode auto|mdi|mdix]		- switch auto or force mdi/mdix mode for crossover cable\n", cmd);
	printf("\n");

	/* 3. mac operations */
	printf("3) mt753x switch MAC operations====================================================================================================================>>>>\n");
	printf(" 3.1) %s dump							- dump switch mac table\n", cmd);
	printf(" 3.2) %s clear							- clear switch mac table\n", cmd);
	printf(" 3.3) %s add [mac] [portmap]					- add an entry (with portmap) to switch mac table\n", cmd);
	printf(" 3.4) %s add [mac] [portmap] [vlan id]				- add an entry (with portmap, vlan id) to switch mac table\n", cmd);
	printf(" 3.5) %s add [mac] [portmap] [vlan id] [age]			- add an entry (with portmap, vlan id, age out time) to switch mac table\n", cmd);
	printf(" 3.6) %s del mac [mac] vid [vid]				- delete an entry from switch mac table\n", cmd);
	printf(" 3.7) %s del mac [mac] fid [fid]				- delete an entry from switch mac table\n", cmd);
	printf(" 3.8) %s search mac [mac] vid [vid]				- search an entry with specific mac and vid\n", cmd);
	printf(" 3.9) %s search mac [mac] fid [fid]				- search an entry with specific mac and fid\n", cmd);
	printf(" 3.10) %s filt [mac]						- add a SA filtering entry (with portmap 1111111) to switch mac table\n", cmd);
	printf(" 3.11) %s filt [mac] [portmap]					- add a SA filtering entry (with portmap)to switch mac table\n", cmd);
	printf(" 3.12) %s filt [mac] [portmap] [vlan id				- add a SA filtering entry (with portmap, vlan id)to switch mac table\n", cmd);
	printf(" 3.13) %s filt [mac] [portmap] [vlan id] [age]			- add a SA filtering entry (with portmap, vlan id, age out time) to switch table\n", cmd);
	printf(" 3.14) %s arl aging [active:0|1] [time:1~65536]			- set switch arl aging timeout value\n", cmd);
	printf(" 3.15) %s macctl fc [enable|disable]				- set switch mac global flow control,enable is 1, disable is 0\n", cmd);
	printf("\n");

	/* 4. mib counter operations */
	printf("4) mt753x switch mib counter operations============================================================================================================>>>>\n");
	printf(" 4.1) %s esw_cnt get						-get switch mib counters\n", cmd);
	printf(" 4.2) %s esw_cnt clear						-clear switch mib counters\n", cmd);
	printf(" 4.3) %s output_queue_cnt get					-get switch output queue counters\n", cmd);
	printf(" 4.4) %s free_page get						-get switch system free page counters\n", cmd);
	printf("\n");

	/* 5. acl function operations */
	printf("5) mt753x switch acl function operations============================================================================================================>>>>\n");
	printf(" 5.1) %s acl enable [port] [port_enable:0|1]			- set switch acl function enabled, port is 0~6,enable is 1, disable is 0\n", cmd);
	printf(" 5.2) %s acl etype add [ethtype] [portmap]			- drop L2 ethertype packets\n", cmd);
	printf(" 5.3) %s acl dmac add [mac] [portmap]				- drop L2 dest-Mac packets\n", cmd);
	printf(" 5.4) %s acl dip add [dip] [portmap]				- drop dip packets\n", cmd);
	printf(" 5.5) %s acl port add [sport] [portmap]				- drop L4 UDP/TCP source port packets\n", cmd);
	printf(" 5.6) %s acl L4 add [2byes] [portmap]				- drop L4 packets with 2bytes payload\n", cmd);
	printf(" 5.7) %s acl acltbl-add  [tbl_idx:0~63/255] [vawd1] [vawd2]	- set switch acl table new entry, max index-7530:63,7531:255\n", cmd);
	printf(" 5.8) %s acl masktbl-add [tbl_idx:0~31/127] [vawd1] [vawd2]	- set switch acl mask table new entry, max index-7530:31,7531:127\n", cmd);
	printf(" 5.9) %s acl ruletbl-add [tbl_idx:0~31/127] [vawd1] [vawd2]	- set switch acl rule table new entry, max index-7530:31,7531:127\n", cmd);
	printf(" 5.10) %s acl ratetbl-add [tbl_idx:0~31] [vawd1] [vawd2]	- set switch acl rate table new entry\n", cmd);
	printf(" 5.11) %s acl dip meter [dip] [portmap][meter:kbps]		- rate limit dip packets\n", cmd);
	printf(" 5.12) %s acl dip trtcm [dip] [portmap][CIR:kbps][CBS][PIR][PBS]- TrTCM dip packets\n", cmd);
	printf(" 5.13) %s acl dip modup [dip] [portmap][usr_pri]		- modify usr priority from ACL\n", cmd);
	printf(" 5.14) %s acl dip pppoe [dip] [portmap]				- pppoe header removal\n", cmd);
	printf("\n");

	/* 6. dip table operations */
	printf("6) mt753x switch dip table operations=================================================================================================================>>>>\n");
	printf(" 6.1) %s dip dump						- dump switch dip table\n", cmd);
	printf(" 6.2) %s dip clear						- clear switch dip table\n", cmd);
	printf(" 6.3) %s dip add [dip] [portmap]				- add a dip entry to switch table\n", cmd);
	printf(" 6.4) %s dip del [dip]						- del a dip entry to switch table\n", cmd);
	printf("\n");

	/* 7. sip table operations */
	printf("7) mt753x switch sip table operations=================================================================================================================>>>>\n");
	printf(" 7.1) %s sip dump						- dump switch sip table\n", cmd);
	printf(" 7.2) %s sip clear						- clear switch sip table\n", cmd);
	printf(" 7.3) %s sip add [sip] [dip] [portmap]				- add a sip entry to switch table\n", cmd);
	printf(" 7.4) %s sip del [sip] [dip]					- del a sip entry to switch table\n", cmd);
	printf("\n");

	/* 8. vlan table operations */
	printf("8) mt753x switch sip table operations====================================================================================================================>>>>\n");
	printf(" 8.1) %s vlan dump (egtag)					- dump switch vlan table (with per port eg_tag setting)\n", cmd);
	printf(" 8.2) %s vlan set [fid:0~7] [vid] [portmap]			- set vlan id and associated member at switch vlan table\n", cmd);
	printf("			([stag:0~4095] [eg_con:0|1] [egtagPortMap 0:untagged 2:tagged])\n");
	printf("			Full Example: %s vlan set 0 3 10000100 0 0 20000200\n", cmd);
	printf(" 8.3) %s vlan vid [vlan idx] [active:0|1] [vid] [portMap]	- set switch vlan vid elements\n", cmd);
	printf("			[egtagPortMap] [ivl_en] [fid] [stag]\n");
	printf(" 8.4) %s vlan pvid [port] [pvid]				- set switch vlan pvid\n", cmd);
	printf(" 8.5) %s vlan acc-frm [port] [acceptable_frame_type:0~3]	- set switch vlan acceptable_frame type : admit all frames: 0,\n", cmd);
	printf("									admit only vlan-taged frames: 1,admit only untagged or priority-tagged frames: 2, reserved:3\n");
	printf(" 8.6) %s vlan port-attr [port] [attr:0~3]			- set switch vlan port attribute: user port: 0, statck port: 1,\n", cmd);
	printf("									translation port: 2, transparent port:3\n");
	printf(" 8.7) %s vlan port-mode [port] [mode:0~3]			- set switch vlan port mode : port matrix mode: 0, fallback mode: 1,\n", cmd);
	printf("									check mode: 2, security mode:3\n");
	printf(" 8.8) %s vlan eg-tag-pvc [port] [eg_tag:0~7]			- set switch vlan eg tag pvc : disable: 0, consistent: 1, reserved: 2,\n", cmd);
	printf("									reserved:3,untagged:4,swap:5,tagged:6, stack:7\n");
	printf(" 8.9) %s vlan eg-tag-pcr [port] [eg_tag:0~3]			- set switch vlan eg tag pcr : untagged: 0, swap: 1, tagged: 2, stack:3\n", cmd);
	printf("\n");

	/* 9. rate limit operations */
	printf("9) mt753x switch rate limit operations=================================================================================================================>>>>\n");
	printf(" 9.1) %s ratectl [in_ex_gress:0|1] [port] [rate]		- set switch port ingress(1) or egress(0) rate\n", cmd);
	printf(" 9.2) %s ingress-rate on [port] [Kbps]				- set ingress rate limit on port n (n= 0~ switch max port)\n", cmd);
	printf(" 9.3) %s egress-rate on [port] [Kbps]				- set egress rate limit on port n (n= 0~ switch max port)\n", cmd);
	printf(" 9.4) %s ingress-rate off [port]				- disable ingress rate limit on port n (n= 0~ switch max port)\n", cmd);
	printf(" 9.5) %s egress-rate off [port]					- disable egress rate limit on port n (n= 0~ switch max port)\n", cmd);
	printf("\n");

	/* 10. igmp operations */
	printf("10) mt753x igmp operations===============================================================================================================================>>>>\n");
	printf(" 10.1) %s igmpsnoop on [leaky_en] [wan_num]			- turn on IGMP snoop and router port learning\n", cmd);
	printf("									leaky_en: 1 or 0. default 0; wan_num: 0 or 4. default 4\n");
	printf(" 10.2) %s igmpsnoop off						- turn off IGMP snoop and router port learning\n", cmd);
	printf(" 10.3) %s igmpsnoop enable [port#]				- enable IGMP HW leave/join/Squery/Gquery\n", cmd);
	printf(" 10.4) %s igmpsnoop disable [port#]				- disable IGMP HW leave/join/Squery/Gquery\n", cmd);
	printf("\n");

	/* 11. QoS operations */
	printf("11) mt753x QoS operations================================================================================================================================>>>>\n");
	printf(" 11.1) %s qos sch [port:0~6] [queue:0~7] [shaper:min|max] [type:rr:0|sp:1|wfq:2]	 - set switch qos sch type\n", cmd);
	printf(" 11.2) %s qos base [port:0~6] [base]					- set switch qos base(UPW); port-based:0, tag-based:1,\n", cmd);
	printf("									dscp-based:2, acl-based:3, arl-based:4, stag-based:5\n");
	printf(" 11.3) %s qos port-weight [port:0~6] [q0] [q1][q2][q3]		- set switch qos port queue weight;\n", cmd);
	printf("				[q4][q5][q6][q7]				 [qn]: the weight of queue n, range: 1~16\n");
	printf(" 11.4) %s qos port-prio [port:0~6] [prio:0~7]			- set switch port qos user priority;  port is 0~6, priority is 0~7\n", cmd);
	printf(" 11.5) %s qos dscp-prio [dscp:0~63] [prio:0~7]			- set switch qos dscp user priority;  dscp is 0~63, priority is 0~7\n", cmd);
	printf(" 11.6) %s qos prio-qmap [port:0~6] [prio:0~7]  [queue:0~7]			- set switch qos priority queue map; priority is 0~7,queue is 0~7\n", cmd);
	printf("\n");

	/*12. port mirror operations*/
	printf(" 12) mt753x port mirror operations========================================================================================================================>>>>\n");
	printf(" 12.1) %s mirror monitor [port]					- enable port mirror and indicate monitor port number\n", cmd);
	printf(" 12.2) %s mirror target  [port]					- set port mirror target\n", cmd);
	printf("			[direction| 0:off, 1:rx, 2:tx, 3:all]\n");
	printf(" 12.3) %s mirror enable [mirror_en:0|1] [mirror_port: 0-6]	- set switch mirror function enable(1) or disabled(0) for port 0~6\n", cmd);
	printf(" 12.4) %s mirror port-based [port] [port_tx_mir:0|1]		- set switch mirror port: target tx/rx/acl/vlan/igmp\n", cmd);
	printf("				[port_rx_mir:0|1] [acl_mir:0|1]\n");
	printf("				[vlan_mis:0|1] [igmp_mir:0|1]\n");
	printf("\n");

	/*13. stp function*/
	printf(" 13) mt753x stp operations===============================================================================================================================>>>>\n");
	printf(" 13.1) %s stp [port] [fid] [state]				- set switch spanning tree state, port is 0~6, fid is 0~7,\n", cmd);
	printf("									state is 0~3(Disable/Discarding:0,Blocking/Listening/Discarding:1,)\n");
	printf("									Learning:2,Forwarding:3\n");
	printf("\n");

	/*14. collision pool operations*/
	printf("14) mt753x collision pool operations========================================================================================================================>>>>\n");
	printf(" 14.1) %s collision-pool enable [enable 0|1]			- enable or disable collision pool\n", cmd);
	printf(" 14.2) %s collision-pool mac dump				- dump collision pool mac table\n", cmd);
	printf(" 14.3) %s collision-pool dip dump				- dump collision pool dip table\n", cmd);
	printf(" 14.4) %s collision-pool sip dump				- dump collision pool sip table\n", cmd);
	printf("\n");

	/*15. pfc(priority flow control) operations*/
	printf("15) mt753x pfc(priority flow control) operations==============================================================================================================>>>>\n");
	printf(" 15.1) %s pfc enable [port] [enable 0|1]			- enable or disable port's pfc\n", cmd);
	printf(" 15.2) %s pfc rx_counter [port]					- get port n pfc 8 up rx counter\n", cmd);
	printf(" 15.3) %s pfc tx_counter [port]					- get port n pfc 8 up rx counter\n", cmd);
	printf("\n");

	/*15. pfc(priority flow control) operations*/
	printf("16) mt753x EEE(802.3az) operations==============================================================================================================>>>>\n");
	printf(" 16.1) %s eee enable [enable 0|1] ([portMap])			- enable or disable EEE (by portMap)\n", cmd);
	printf(" 16.2) %s eee dump ([port])					- dump EEE capability (by port)\n", cmd);
	printf("\n");

	if (chip_name == 0x8855) {
		printf("switch an8855 <sub cmd> supported commands===================================================================================================================>>>>\n");
		printf("\n");
		printf("Register/GPHY access commands===============================================================================================================>>>>\n");
		printf("reg r <reg(4'hex)>\n");
		printf("reg w <reg(4'hex)> <value(8'hex)>\n");
		printf("phy cl22 r <port(0..4)> <reg(2'hex)>\n");
		printf("phy cl22 w <port(0..4)> <reg(2'hex)> <value(4'hex)>\n");
		printf("phy cl45 r <port(0..4)> <dev(2'hex)> <reg(3'hex)>\n");
		printf("phy cl45 w <port(0..4)> <dev(2'hex)> <reg(3'hex)> <value(4'hex)>\n");
		printf("\n");
		printf("Port configuration commands=================================================================================================================>>>>\n");
		printf("port set matrix <port(0..6)> <matrix(6:0)>\n");
		printf("port get matrix <port(0..6)>\n");
		printf("port set vlanMode <port(0..6)> <vlanMode(0:matrix,1:fallback,2:check,3:security)>\n");
		printf("port get vlanMode <port(0..6)>\n");
		printf("port set flowCtrl <port(0..6)> <dir(0:Tx,1:Rx)> <fc_en(1:En,0:Dis)>\n");
		printf("port get flowCtrl <port(0..6)> <dir(0:Tx,1:Rx)>\n");
		printf("port set jumbo <pkt_len(0:1518,1:1536,2:1552,3:max)> <frame_len(2..15)>\n");
		printf("port get jumbo\n");
		printf("port set anMode <port(0..4)> <en(0:force,1:AN)>\n");
		printf("port get anMode <port(0..4)>\n");
		printf("port set localAdv <port(0..4)> <10H(1:En,0:Dis)> <10F(1:En,0:Dis)> <100H(1:En,0:Dis)> <100F(1:En,0:Dis)> <1000F(1:En,0:Dis)> <pause(1:En,0:Dis)>\n");
		printf("port get localAdv <port(0..4)>\n");
		printf("port get remoteAdv <port(0..4)>\n");
		printf("port set speed <port(0..4)> <speed(0:10M,1:100M,2:1G,3:2.5G)>\n");
		printf("port get speed <port(0..4)>\n");
		printf("port set duplex <port(0..4)> <duplex(0:half,1:full)>\n");
		printf("port get duplex <port(0..4)>\n");
		printf("port get status <port(0..4)>\n");
		printf("port set bckPres <port(0..6)> <bckPres(1:En,0:Dis)>\n");
		printf("port get bckPres <port(0..6)>\n");
		printf("port set psMode <port(0..4)> <ls(1:En,0:Dis)> <eee(1:En,0:Dis)>\n");
		printf("port get psMode <port(0..4)>\n");
		printf("port set smtSpdDwn <port(0..4)> <en(1:En,0:Dis)> <retry(2..5)>\n");
		printf("port get smtSpdDwn <port(0..4)>\n");
		printf("port set spTag <port(0..6)> <en(1:En,0:Dis)>\n");
		printf("port get spTag <port(0..6)>\n");
		printf("port set enable <port(0..4)> <en(1:En,0:Dis)>\n");
		printf("port get enable <port(0..4)>\n");
		printf("port set 5GBaseRMode\n");
		printf("port set hsgmiiMode\n");
		printf("port set sgmiiMode <mode(0:AN,1:Force)> <speed(0:10M,1:100M,2:1G)>\n");
		printf("port set rmiiMode <speed(0:10M,1:100M)>\n");
		printf("port set rgmiiMode <speed(0:10M,1:100M,2:1G)>\n");
		printf("\n");
		printf("Special tag commands========================================================================================================================>>>>\n");
		printf("sptag  setEnable port<port(0..6)> enable<1:enable 0:disable>\n");
		printf("sptag  getEnable port<port(0..6)>\n");
		printf("sptag  setmode port<port(0..6)> mode<0:inset 1:replace>\n");
		printf("sptag  getmode port<port(0..6)>\n");
		printf("sptag  encode mode={ insert | replace } opc={ portmap | portid | lookup } dp={bitimap hex} vpm={ untagged | 8100 | 88a8 } pri=<UINT> cfi=<UINT> vid=<UINT>\n");
		printf("sptag  decode <byte(hex)> <byte(hex)> <byte(hex)> <byte(hex)>\n");
		printf("\n");
		printf("Vlan commands===============================================================================================================================>>>>\n");
		printf("sptag  set fid <vid(0..4095)> <fid(0..7)>\n");
		printf("sptag  set memPort <vid(0..4095)> <bitmap(6:0)>\n");
		printf("sptag  set ivl <vid(0..4095)> <(1:En,0:Dis)>\n");
		printf("sptag  set portBaseStag <vid(0..4095)> <(1:En,0:Dis)>\n");
		printf("sptag  set stag <vid(0..4095)> <stag(0..4095)>\n");
		printf("sptag  set egsTagCtlEn <vid(0..4095)> <(1:En,0:Dis)>\n");
		printf("sptag  set egsTagCtlCon <vid(0..4095)> <(1:En,0:Dis)>\n");
		printf("sptag  set egsTagCtl <vid(0..4095)> <port(0..6)> <ctlType(0:untag,2:tagged)>\n");
		printf("sptag  set portActFrame <port(0..6)> <frameType(0:all,1:tagged,2:untagged)>\n");
		printf("sptag  get portActFrame <port(0..6)>\n");
		printf("sptag  set LeakyVlanEn <port(0..6)> <pktType(0:uc,1:mc,2:bc,3:ipmc)> <(1:En,0:Dis)>\n");
		printf("sptag  get leakyVlanEn <port(0..6)>\n");
		printf("sptag  set portVlanAttr <port(0..6)> <vlanAttr(0:user,1:stack,2:translation,3:transparent)>\n");
		printf("sptag  get portVlanAttr <port(0..6)>\n");
		printf("sptag  set igsPortETagAttr <port(0..6)> <egsTagAttr(0:disable,1:consistent,4:untagged,5:swap,6:tagged,7:stack)>\n");
		printf("sptag  get igsPortETagAttr <port(0..6)>\n");
		printf("sptag  set portEgsTagAttr <port(0..6)> <egsTagAttr(0:untagged,1:swap,2:tagged,3:stack)>\n");
		printf("sptag  get portEgsTagAttr <port(0..6)>\n");
		printf("sptag  set portOuterTPID <port(0..6)> <TPID(hex)>\n");
		printf("sptag  get portOuterTPID <port(0..6)>\n");
		printf("sptag  set pvid <port(0..6)> <vid(0..4095)>\n");
		printf("sptag  get pvid <port(0..6)>\n");
		printf("sptag  initiate <vid(0..4095)> <fid(0..7)> <bitmap(6:0)> <ivl(1:En,0:Dis)> <portbasestag(1:En,0:Dis)> <stag(0..4095)> <egstagctlen(1:En,0:Dis)> <egstagcon(1:En,0:Dis)> <taggedbitmap(6:0)>\n");
		printf("sptag  create <vid(0..4095)>\n");
		printf("sptag  destroy [ <vid(0..4095)> | <vidRange(vid0-vid1)> ]\n");
		printf("sptag  destroyAll [ <restoreDefVlan(0:false,1:true)> | ]\n");
		printf("sptag  dump [ <vid(0..4095)> | <vidRange(vid0-vid1)> | ]\n");
		printf("sptag  addPortMem <vid(0..4095)> <port(0..6)>\n");
		printf("sptag  addPortMem <vid(0..4095)> <port(0..6)>\n");
		printf("\n");
		printf("Layer2 commands=============================================================================================================================>>>>\n");
		printf("l2 dump mac\n");
		printf("l2 add mac <static(0:dynamic,1:static)> <unauth(0:auth,1:unauth)> <mac(12'hex)> <portlist(uintlist)> [ vid <vid(0..4095)> | fid <fid(0..15)> ] <src_mac_forward=(0:default,1:cpu-exclude,2:cpu-include,3:cpu-only,4:drop)>\n");
		printf("l2 del mac <mac(12'hex)> [ vid <vid(0..4095)> | fid <fid(0..15)> ]\n");
		printf("l2 get mac <mac(12'hex)> [ vid <vid(0..4095)> | fid <fid(0..15)> ]\n");
		printf("l2 clear mac\n");
		printf("l2 set macAddrAgeOut <time(1, 1000000)>\n");
		printf("l2 get macAddrAgeOut\n");
		printf("\n");
		printf("Link Aggregation commands===================================================================================================================>>>>\n");
		printf("lag set member <group_id(0 or 1)> <member_index(0..3)> <enable(0,1)> <port index(0..6)>\n");
		printf("lag get member group_id(0 or 1)\n");
		printf("lag set dstInfo <sp(1:En,0:Dis)> <sa(1:En,0:Dis)> <da(1:En,0:Dis)> <sip(1:En,0:Dis)> <dip(1:En,0:Dis)> <sport(1:En,0:Dis)> <dport(1:En,0:Dis)>\n");
		printf("lag get dstInfo\n");
		printf("lag set ptseed <hex32>\n");
		printf("lag get ptseed\n");
		printf("lag set hashtype <0-crc32lsb;1-crc32msb;2-crc16;3-xor4>\n");
		printf("lag get hashtype\n");
		printf("lag set state <state(1:En,0:Dis)>\n");
		printf("lag get state\n");
		printf("lag set spsel <soure port enable(1:En,0:Dis)>\n");
		printf("lag get spsel\n");
		printf("\n");
		printf("STP commands================================================================================================================================>>>>\n");
		printf("stp set portstate <port(0..6)> <fid(0..15)> <state(0:disable,1:listen,2:learn,3:forward)>\n");
		printf("stp get portstate <port(0..6)> <fid(0..15)>\n");
		printf("\n");
		printf("Mirror commands=============================================================================================================================>>>>\n");
		printf("mirror set session <sid(0,1)> <dst_port(UINT)> <state(1:En,0:Dis)> <tag(1:on, 0:off)> <list(UINTLIST)> <dir(0:none,1:tx,2:rx,3:both)>\n");
		printf("mirror set session-enable <sid(0,1)> <state(1:En,0:Dis)>\n");
		printf("mirror add session-rlist <sid(0,1)> <list(UINTLIST)>\n");
		printf("mirror add session-tlist <sid(0,1)> <list(UINTLIST)>\n");
		printf("mirror get session <sid(0,1)>\n");
		printf("mirror del session <sid(0,1)>\n");
		printf("\n");
		printf("MIB commands================================================================================================================================>>>>\n");
		printf("mib get port <port(0..6)>\n");
		printf("mib get acl <event(0..7)>\n");
		printf("mib clear port <port(0..6)>\n");
		printf("mib clear all\n");
		printf("mib clear acl\n");
		printf("\n");
		printf("QoS commands================================================================================================================================>>>>\n");
		printf("qos set scheduleAlgo <portlist(UINTLIST)> <queue(UINT)> <scheduler(0:SP,1:WRR,2:WFQ)> <weight(0..128)>, weight 0 is valid only on sp mode\n");
		printf("qos get scheduleAlgo <portlist(UINTLIST)> <queue(UINT)>\n");
		printf("qos set trustMode <portlist(UINTLIST)> <mode(0:port,1:1p-port,2:dscp-port,3:dscp-1p-port>\n");
		printf("qos get trustMode <portlist(UINTLIST)>\n");
		printf("qos set pri2Queue <priority(0..7)> <queue(0..7)>\n");
		printf("qos get pri2Queue\n");
		printf("qos set dscp2Pri <dscp(0..63)> <priority(0..7)>\n");
		printf("qos get dscp2Pri <dscp(0..63)>\n");
		printf("qos set rateLimitEnable <portlist(UINTLIST)> <dir(0:egress,1:ingress)> <rate_en(1:En,0:Dis)>\n");
		printf("qos get rateLimitEnable <portlist(UINTLIST)>\n");
		printf("qos set rateLimit <portlist(UINTLIST)> <I_CIR(0..80000)> <I_CBS(0..127)> <E_CIR(0..80000)> <E_CBS(0..127)>\n");
		printf("qos get rateLimit <portlist(UINTLIST)>\n");
		printf("qos set portPriority <portlist(UINTLIST)> <priority(0..7)>\n");
		printf("qos get portPriority <portlist(UINTLIST)>\n");
		printf("qos set rateLmtExMngFrm <dir(0:egress)> <en(0:include,1:exclude)>\n");
		printf("qos get rateLmtExMngFrm\n");
		printf("\n");
		printf("Diag commands===============================================================================================================================>>>>\n");
		printf("diag set txComply <phy(0..4)> <mode(0..8)>\n");
		printf("diag get txComply <phy(0..4)>\n");
		printf("\n");
		printf("LED commands================================================================================================================================>>>>\n");
		printf("led set mode <mode(0:disable, 1..3:2 LED, 4:user-define)>\n");
		printf("led get mode\n");
		printf("led set state <led(0..1)> <state(1:En,0:Dis)>\n");
		printf("led get state <led(0..1)>\n");
		printf("led set usr <led(0..1)> <polarity(0:low, 1:high)> <on_evt(7'bin)> <blink_evt(10'bin)>\n");
		printf("led get usr <led(0..1)>\n");
		printf("led set time <time(0..5:32ms~1024ms)>\n");
		printf("led get time\n");
		printf("\n");
		printf("Security commands===========================================================================================================================>>>>\n");
		printf("sec set stormEnable <port(0..6)> <type(0:bcst,1:mcst,2:ucst)> <en(1:En,0:Dis)>\n");
		printf("sec get stormEnable <port(0..6)> <type(0:bcst,1:mcst,2:ucst)>\n");
		printf("sec set stormRate <port(0..6)> <type(0:bcst,1:mcst,2:ucst)> <count(0..255)> <unit(0:64k,1:256k,2:1M,3:4M,4:16M)>\n");
		printf("sec get stormRate <port(0..6)> <type(0:bcst,1:mcst,2:ucst)>\n");
		printf("sec set fldMode <port(0..6)> <type(0:bcst,1:mcst,2:ucst,3:qury> <en(1:En,0:Dis)>\n");
		printf("sec get fldMode <port(0..6)> <type(0:bcst,1:mcst,2:ucst,3:qury>\n");
		printf("sec set saLearning <port(0..6)> <learn(0:disable,1:enable)>\n");
		printf("sec get saLearning <port(0..6)>\n");
		printf("sec set saLimit <port(0..6)> <mode(0:disable,1:enable)> <count(0..4095)>\n");
		printf("sec get saLimit <port(0..6)>\n");
		printf("\n");
		printf("Switch commands=============================================================================================================================>>>>\n");
		printf("switch set cpuPortEn <cpu_en(1:En,0:Dis)>\n");
		printf("switch get cpuPortEn\n");
		printf("switch set cpuPort <port_number>\n");
		printf("switch get cpuPort\n");
		printf("switch set phyLCIntrEn <phy(0..6)> <(1:En,0:Dis)>\n");
		printf("switch get phyLCIntrEn <phy(0..6)>\n");
		printf("switch set phyLCIntrSts <phy(0..6)> <(1:Clear)>\n");
		printf("switch get phyLCIntrSts <phy(0..6)>\n");
		printf("\n");
		printf("ACL commands================================================================================================================================>>>>\n");
		printf("acl set en <en(1:En,0:Dis)>\n");
		printf("acl get en\n");
		printf("acl set rule <idx(0..127)>\n <state(0:Dis,1:En)> <reverse(0:Dis,1:En)> <end(0:Dis,1:En)>\n <portmap(7'bin)><ipv6(0:Dis,1:En,2:Not care)>\n[ dmac <dmac(12'hex)> <dmac_mask(12'hex)> ]\n[ smac <smac(12'hex)> <smac_mask(12'hex)> ]\n[ stag <stag(4'hex)> <stag_mask(4'hex)> ]\n[ ctag <ctag(4'hex)> <ctag_mask(4'hex)> ]\n[ etype <etype(4'hex)> <etype_mask(4'hex)> ]\n[ dip <dip(IPADDR)> <dip_mask(IPADDR)> ]\n[ sip <sip(IPADDR)> <sip_mask(IPADDR)> ]\n[ dscp <dscp(2'hex)> <dscp_mask(2'hex)> ]\n[ protocol <protocol(12'hex)> <protocol_mask(12'hex)> ]\n[ dport <dport(4'hex)> <dport_mask(4'hex)> ]\n[ sport <sport(4'hex)> <sport_mask(4'hex)> ]\n[ flow_label <flow_label(4'hex)> <flow_label_mask(4'hex)> ]\n[ udf <udf(4'hex)> <udf_mask(4'hex)> ] ");
		printf("acl get rule <idx(0..127)> ");
		printf("acl del rule <idx(0..127)>\n");
		printf("acl clear rule\n");
		printf("acl set udfRule <idx(0..15)> <mode(0:pattern, 1:threshold)> [ <pat(4'hex)> <mask(4'hex)> | <low(4'hex)> <high(4'hex)> ] <start(0:MAC header, 1:L2 payload, 2:IPv4 header, 3:IPv6 header, 4:L3 payload, 5:TCP header, 6:UDP header, 7: L4 payload)> <offset(0..127,unit:2 bytes)> <portmap(7'bin)>\n");
		printf("acl get udfRule <idx(0..15)>\n");
		printf("acl del udfRule <idx(0..15)>\n");
		printf("acl clear udfRule\n");
		printf("acl set action <idx(0..127)>\n[ forward <forward(0:Default,4:Exclude CPU,5:Include CPU,6:CPU only,7:Drop)> ]\n[ egtag <egtag(0:Default,1:Consistent,4:Untag,5:Swap,6:Tag,7:Stack)> ]\n[ mirrormap <mirrormap(2'bin)> ]\n[ priority <priority(0..7)> ]\n[ redirect <redirect(0:Dst,1:Vlan)> <portmap(7'bin)> ]\n[ leaky_vlan <leaky_vlan(1:En,0:Dis)> ]\n[ cnt_idx <cnt_idx(0..63)> ]\n[ rate_idx <rate_idx(0..31)> ]\n[ attack_idx <attack_idx(0..95)> ]\n[ vid <vid(0..4095)> ]\n[ manage <manage(1:En,0:Dis)> ]\n[ bpdu <bpdu(1:En,0:Dis)> ]\n[ class <class(0:Original,1:Defined)>[0..7] ]\n[ drop_pcd <drop_pcd(0:Original,1:Defined)> [red <red(0..7)>][yellow <yellow(0..7)>][green <green(0..7)>] ]\n[ color <color(0:Defined,1:Trtcm)> [ <defined_color(0:Dis,1:Green,2:Yellow,3:Red)> | <trtcm_idx(0..31)> ] ]");
		printf("acl get action <idx(0..127)>\n");
		printf("acl del action <idx(0..127)>\n");
		printf("acl clear action\n");
		printf("acl set trtcm <idx(1..31)> <cir(4'hex)> <pir(4'hex)> <cbs(4'hex)> <pbs(4'hex)>\n");
		printf("acl get trtcm <idx(1..31)>\n");
		printf("acl del trtcm <idx(0..31)>\n");
		printf("acl clear trtcm\n");
		printf("acl set trtcmEn <en(1:En,0:Dis)>\n");
		printf("acl get trtcmEn\n");
		printf("acl set portEn <port(0..6)> <en(1:En,0:Dis)>\n");
		printf("acl get portEn <port(0..6)>\n");
		printf("acl set dropEn <port(0..6)> <en(1:En,0:Dis)>\n");
		printf("acl get dropEn <port(0..6)>\n");
		printf("acl set dropThrsh <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)> <high(0..2047)> <low(0..2047)>\n");
		printf("acl get dropThrsh <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)>\n");
		printf("acl set dropPbb <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)> <probability(0..1023)>\n");
		printf("acl get dropPbb <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)>\n");
		printf("acl set meter <idx(0..31)> <en(1:En,0:Dis)> <rate(0..65535)>\n Note: Limit rate = rate * 64Kbps");
		printf("acl get meter <idx(0..31)>\n");
		printf("\n");
	}

	exit_free();
	exit(0);
}

static void parse_reg_cmd(int argc, char *argv[], int len)
{
	unsigned int val = 0;
	unsigned int off = 0;
	char *endptr;
	int i, j;

	if (!strncmp(argv[len - 3], "reg", 4)) {
		if (argv[len - 2][0] == 'r') {
			errno = 0;
			off = strtoul(argv[len - 1], &endptr, 16);
			if (errno != 0 || *endptr != '\0')
				goto error;
			reg_read(off, &val);
			printf(" Read reg=%x, value=%x\n", off, val);
		} else if (argv[len - 2][0] == 'w') {
			errno = 0;
			off = strtoul(argv[len - 1], &endptr, 16);
			if (errno != 0 || *endptr != '\0')
				goto error;
			if (argc != len + 1)
				usage(argv[0]);
			errno = 0;
			val = strtoul(argv[len], &endptr, 16);
			if (errno != 0 || *endptr != '\0')
				goto error;
			reg_write(off, val);
			printf(" Write reg=%x, value=%x\n", off, val);
		} else if (argv[len - 2][0] == 'd') {
			errno = 0;
			off = strtoul(argv[len - 1], &endptr, 16);
			if (errno != 0 || *endptr != '\0')
				goto error;
			for (i = 0; i < 16; i++) {
				printf("0x%08x: ", off + 0x10 * i);
				for (j = 0; j < 4; j++) {
					reg_read(off + i * 0x10 + j * 0x4, &val);
					printf(" 0x%08x", val);
				}
				printf("\n");
			}
		} else
			usage(argv[0]);
	} else
		usage(argv[0]);
	return;
error:
	printf("Error: string converting\n");
}

static int get_chip_name()
{
	int temp = 0, rc = 0;
	FILE *fp = NULL;
	char *buff;
	int filesize, i;

	/*judge an8855, must be placed before reg_read*/
	if (air_skip_check_flag) {
		temp = 0x8855;
		return temp;
	}

	/*judge jaguar embedded switch */
	fp = fopen("/proc/device-tree/compatible", "r");
	if (fp != NULL) {
		if (fseek(fp, 0, SEEK_END) < 0) {
			CLOSE_FILE(fp);
			return -1;
		}

		filesize = ftell(fp);
		if (filesize < 0) {
			printf("Failed to get file size");
			CLOSE_FILE(fp);
			return -1;
		}

		if (fseek(fp, 0, SEEK_SET) < 0) {
			CLOSE_FILE(fp);
			return -1;
		}

		buff = (char *)malloc(filesize + 1);
		if (buff == NULL) {
			printf("Memory allocation failed\n");
			CLOSE_FILE(fp);
			return -1;
		}

		if (fread(buff, 1, filesize, fp) != filesize) {
			printf("Failed to read compatible content\n");
			free(buff);
			CLOSE_FILE(fp);
			return -1;
		}

		/* Null-terminate the buffer and replace '\0' characters with space characters */
		buff[filesize] = '\0';
		for (i = 0; i < filesize; i++) {
			if (buff[i] == '\0')
				buff[i] = ' ';
		}

		if (strstr(buff, "mt7988") != NULL)
			temp = 0x7988;

		free(buff);
		CLOSE_FILE(fp);

		if (temp == 0x7988)
			return temp;
	}

	/*judge 7530 */
	reg_read((0x7ffc), &temp);
	temp = temp >> 16;
	if (temp == 0x7530)
		return temp;

	/*judge 7531 */
	reg_read(0x781c, &temp);
	temp = temp >> 16;
	if (temp == 0x7531)
		return temp;

	return -1;
}

static int phy_operate(int argc, char *argv[])
{
	unsigned int port_num = 0;
	unsigned int dev_num = 0;
	unsigned int value = 0, cl_value = 0;
	unsigned int reg = 0;
	int ret = 0, cl_ret = 0;
	char op;

	if (strncmp(argv[2], "cl22", 4) && strncmp(argv[2], "cl45", 4))
		usage(argv[0]);

	op = argv[3][0];

	switch (op) {
	case 'r':
		reg = strtoul(argv[argc - 1], NULL, 0);
		if (reg >= 0xFFFFFFFF) {
			printf(" Phy read reg fail\n");
			ret = -1;
			break;
		}

		if (argc == 6) {
			port_num = strtoul(argv[argc - 2], NULL, 0);
			if (port_num > MAX_PORT) {
				printf(" Phy read reg fail\n");
				ret = -1;
				break;
			}

			ret = mii_mgr_read(port_num, reg, &value);
			if (ret < 0)
				printf(" Phy read reg fail\n");
			else
				printf(" Phy read reg=0x%x, value=0x%x\n",
				       reg, value);
		} else if (argc == 7) {
			dev_num = strtoul(argv[argc - 2], NULL, 0);
			if (dev_num >= 0xFFFFFFFF) {
				printf(" Phy read reg fail\n");
				ret = -1;
				break;
			}

			port_num = strtoul(argv[argc - 3], NULL, 0);
			if (port_num > MAX_PORT) {
				printf(" Phy read reg fail\n");
				ret = -1;
				break;
			}

			ret = mii_mgr_c45_read(port_num, dev_num, reg,
					       &value);
			if (ret < 0)
				printf(" Phy read reg fail\n");
			else
				printf(" Phy read dev_num=0x%x, reg=0x%x, value=0x%x\n",
				       dev_num, reg, value);
		} else
			ret = phy_dump(32);
		break;
	case 'w':
		reg = strtoul(argv[argc - 2], NULL, 0);
		if (reg >= 0xFFFFFFFF) {
			printf(" Phy write reg fail\n");
			ret = -1;
			break;
		}

		value = strtoul(argv[argc - 1], NULL, 16);
		if (value > 0xFFFF) {
			printf(" Phy write reg fail\n");
			ret = -1;
			break;
		}

		if (argc == 7) {
			port_num = strtoul(argv[argc - 3], NULL, 0);
			if (port_num > MAX_PORT) {
				printf(" Phy write reg fail\n");
				ret = -1;
				break;
			}

			ret = mii_mgr_write(port_num, reg, value);
			cl_ret = mii_mgr_read(port_num, reg, &cl_value);
			if (cl_ret < 0)
				printf(" Phy write reg fail\n");
			else
				printf(" Phy write reg=0x%x, value=0x%x\n",
				       reg, cl_value);
		} else if (argc == 8) {
			dev_num = strtoul(argv[argc - 3], NULL, 0);
			if (dev_num >= 0xFFFFFFFF) {
				printf(" Phy write reg fail\n");
				ret = -1;
				break;
			}

			port_num = strtoul(argv[argc - 4], NULL, 0);
			if (port_num > MAX_PORT) {
				printf(" Phy write reg fail\n");
				ret = -1;
				break;
			}

			ret = mii_mgr_c45_write(port_num, dev_num, reg, value);
			cl_ret = mii_mgr_c45_read(port_num, dev_num, reg,
						  &cl_value);
			if (cl_ret < 0)
				printf(" Phy write reg fail\n");
			else
				printf(" Phy write dev_num=0x%x reg=0x%x, value=0x%x\n",
				       dev_num, reg, cl_value);
		}
		break;
	default:
		break;
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int err = -EINVAL;
	FILE *fp = NULL;
	char buff[255];
	char *endptr;

	attres = (struct mt753x_attr *)malloc(sizeof(struct mt753x_attr));
	if (attres == NULL) {
		printf("Failed to allocate memory.\n");
		exit(0);
	}
	attres->dev_id = -1;
	attres->port_num = -1;
	attres->phy_dev = -1;
	nl_init_flag = true;
	air_skip_check_flag = false;

	fp = fopen("/proc/air_sw/device", "r");
	if (fp != NULL) {
		if (fgets(buff, 255, (FILE *) fp) && strstr(buff, "an8855"))
			air_skip_check_flag = true;

		if ((fclose(fp) == 0) && air_skip_check_flag) {
			err = mt753x_netlink_init(AN8855_DSA_GENL_NAME);
			if (!err)
				chip_name = get_chip_name();

			if (err < 0) {
				err = mt753x_netlink_init(AN8855_GENL_NAME);
				if (!err)
					chip_name = get_chip_name();
			}
		}
	} else {
		/* dsa netlink family might not be enabled. Try gsw netlink family. */
		err = mt753x_netlink_init(MT753X_DSA_GENL_NAME);
		if (!err)
			chip_name = get_chip_name();

		if (err < 0) {
			err = mt753x_netlink_init(MT753X_GENL_NAME);
			if (!err)
				chip_name = get_chip_name();
		}
	}

	if (err < 0) {
		err = switch_ioctl_init();
		if (!err) {
			nl_init_flag = false;
			chip_name = get_chip_name();
			if (chip_name < 0) {
				printf
				    ("no chip unsupport or chip id is invalid!\n");
				exit_free();
				exit(0);
			}
		}
	}
#ifndef COMPAT_MODE
	if (chip_name == 0x8855) {
		AIR_INIT_PARAM_T init_param = { 0 };

		init_param.printf = printf;
		init_param.malloc = malloc;
		init_param.free = free;
		init_param.udelay = usleep;
		init_param.dev_access.read_callback = an8855_reg_read;
		init_param.dev_access.write_callback = an8855_reg_write;
		init_param.dev_access.phy_read_callback = an8855_phy_cl22_read;
		init_param.dev_access.phy_write_callback =
		    an8855_phy_cl22_write;
		init_param.dev_access.phy_cl45_read_callback =
		    an8855_phy_cl45_read;
		init_param.dev_access.phy_cl45_write_callback =
		    an8855_phy_cl45_write;

		air_init(0, &init_param);
		air_parse_cmd((argc - 1), &argv[1]);
	}

	exit_free();
	return 0;
#else
	if (argc < 2)
		usage(argv[0]);

	if (chip_name == 0x8855) {
		AIR_INIT_PARAM_T init_param = { 0 };

		init_param.printf = printf;
		init_param.malloc = malloc;
		init_param.free = free;
		init_param.udelay = usleep;
		init_param.dev_access.read_callback = an8855_reg_read;
		init_param.dev_access.write_callback = an8855_reg_write;
		init_param.dev_access.phy_read_callback = an8855_phy_cl22_read;
		init_param.dev_access.phy_write_callback =
		    an8855_phy_cl22_write;
		init_param.dev_access.phy_cl45_read_callback =
		    an8855_phy_cl45_read;
		init_param.dev_access.phy_cl45_write_callback =
		    an8855_phy_cl45_write;
		air_init(0, &init_param);

		p_switch_func = &an8855_switch_func;
	} else {
		p_switch_func = &mt753x_switch_func;
	}

	if ((argc > 2) && !strcmp(argv[1], "an8855")
	    && (chip_name == 0x8855)) {
		air_parse_cmd((argc - 2), (const char **)&argv[2]);
		exit_free();
		return 0;
	}

	if (!strcmp(argv[1], "dev")) {
		errno = 0; /* To distinguish success/failure after call */
		attres->dev_id = strtoul(argv[2], &endptr, 0);
		if (errno != 0 || *endptr != '\0')
			goto error;

		argv += 2;
		argc -= 2;
		if (argc < 2)
			usage(argv[0]);
	}

	if (argc == 2) {
		if (!strcmp(argv[1], "devs")) {
			attres->type = MT753X_ATTR_TYPE_MESG;
			mt753x_list_swdev(attres, MT753X_CMD_REQUEST);
		} else if (!strncmp(argv[1], "dump", 5)) {
			p_switch_func->pf_table_dump(argc, argv);
		} else if (!strncmp(argv[1], "clear", 6)) {
			p_switch_func->pf_table_clear(argc, argv);
			printf("done.\n");
		} else if (!strncmp(argv[1], "reset", 5)) {
			p_switch_func->pf_switch_reset(argc, argv);
		} else if (!strncmp(argv[1], "phy", 4)) {
			phy_dump(32);	//dump all phy register
		} else if (!strncmp(argv[1], "sysctl", 7)) {
			if (nl_init_flag) {
				if (chip_name == 0x8855)
					printf("netlink(%s)\n",
					       AN8855_GENL_NAME);
				else
					printf("netlink(%s)\n",
					       MT753X_GENL_NAME);
			} else
				printf("ioctl(%s)\n", ETH_DEVNAME);
		} else if (!strncmp(argv[1], "ver", 4)) {
			if (chip_name == 0x8855)
				printf("Switch APP version: %s\r\n",
				       SWITCH_APP_VERSION);
		} else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "arl", 4)) {
		if (!strncmp(argv[2], "aging", 6))
			p_switch_func->pf_doArlAging(argc, argv);
	} else if (!strncmp(argv[1], "esw_cnt", 8)) {
		if (!strncmp(argv[2], "get", 4))
			p_switch_func->pf_read_mib_counters(argc, argv);
		else if (!strncmp(argv[2], "clear", 6))
			p_switch_func->pf_clear_mib_counters(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "output_queue_cnt", 17)) {
		if (!strncmp(argv[2], "get", 4))
			p_switch_func->pf_read_output_queue_counters(argc,
								     argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "free_page", 10)) {
		if (!strncmp(argv[2], "get", 4))
			p_switch_func->pf_read_free_page_counters(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "ratectl", 8))
		p_switch_func->pf_rate_control(argc, argv);
	else if (!strncmp(argv[1], "add", 4))
		p_switch_func->pf_table_add(argc, argv);
	else if (!strncmp(argv[1], "filt", 5))
		p_switch_func->pf_table_add(argc, argv);
	else if (!strncmp(argv[1], "del", 4)) {
		if (!strncmp(argv[4], "fid", 4))
			p_switch_func->pf_table_del_fid(argc, argv);
		else if (!strncmp(argv[4], "vid", 4))
			p_switch_func->pf_table_del_vid(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "search", 7)) {
		if (!strncmp(argv[4], "fid", 4))
			p_switch_func->pf_table_search_mac_fid(argc, argv);
		else if (!strncmp(argv[4], "vid", 4))
			p_switch_func->pf_table_search_mac_vid(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "phy", 4)) {
		if (argc == 3) {
			int phy_addr = strtoul(argv[2], NULL, 0);
			if (phy_addr < 0 || phy_addr > 31)
				usage(argv[0]);
			phy_dump(phy_addr);
		} else if (argc == 5) {
			if (!strncmp(argv[2], "fc", 2))
				phy_set_fc(argc, argv);
			else if (!strncmp(argv[2], "an", 2))
				phy_set_an(argc, argv);
			else
				phy_dump(32);
		} else
			phy_operate(argc, argv);
	} else if (!strncmp(argv[1], "trreg", 4)) {
		if (rw_phy_token_ring(argc, argv) < 0)
			usage(argv[0]);
	} else if (!strncmp(argv[1], "macctl", 7)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "fc", 3))
			p_switch_func->pf_global_set_mac_fc(argc, argv);
		else if (!strncmp(argv[2], "pfc", 4))
			p_switch_func->pf_set_mac_pfc(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "qos", 4)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "sch", 4))
			p_switch_func->pf_qos_sch_select(argc, argv);
		else if (!strncmp(argv[2], "base", 5))
			p_switch_func->pf_qos_set_base(argc, argv);
		else if (!strncmp(argv[2], "port-weight", 12))
			p_switch_func->pf_qos_wfq_set_weight(argc, argv);
		else if (!strncmp(argv[2], "port-prio", 10))
			p_switch_func->pf_qos_set_portpri(argc, argv);
		else if (!strncmp(argv[2], "dscp-prio", 10))
			p_switch_func->pf_qos_set_dscppri(argc, argv);
		else if (!strncmp(argv[2], "prio-qmap", 10))
			p_switch_func->pf_qos_pri_mapping_queue(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "stp", 4)) {
		if (argc < 3)
			usage(argv[0]);
		else
			p_switch_func->pf_doStp(argc, argv);
	} else if (!strncmp(argv[1], "sip", 4)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dump", 5))
			p_switch_func->pf_sip_dump(argc, argv);
		else if (!strncmp(argv[2], "add", 4))
			p_switch_func->pf_sip_add(argc, argv);
		else if (!strncmp(argv[2], "del", 4))
			p_switch_func->pf_sip_del(argc, argv);
		else if (!strncmp(argv[2], "clear", 6))
			p_switch_func->pf_sip_clear(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "dip", 4)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dump", 5))
			p_switch_func->pf_dip_dump(argc, argv);
		else if (!strncmp(argv[2], "add", 4))
			p_switch_func->pf_dip_add(argc, argv);
		else if (!strncmp(argv[2], "del", 4))
			p_switch_func->pf_dip_del(argc, argv);
		else if (!strncmp(argv[2], "clear", 6))
			p_switch_func->pf_dip_clear(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "mirror", 7)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "monitor", 8))
			p_switch_func->pf_set_mirror_to(argc, argv);
		else if (!strncmp(argv[2], "target", 7))
			p_switch_func->pf_set_mirror_from(argc, argv);
		else if (!strncmp(argv[2], "enable", 7))
			p_switch_func->pf_doMirrorEn(argc, argv);
		else if (!strncmp(argv[2], "port-based", 11))
			p_switch_func->pf_doMirrorPortBased(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "acl", 4)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dip", 4)) {
			if (!strncmp(argv[3], "add", 4))
				p_switch_func->pf_acl_dip_add(argc, argv);
			else if (!strncmp(argv[3], "modup", 6))
				p_switch_func->pf_acl_dip_modify(argc, argv);
			else if (!strncmp(argv[3], "pppoe", 6))
				p_switch_func->pf_acl_dip_pppoe(argc, argv);
			else if (!strncmp(argv[3], "trtcm", 4))
				p_switch_func->pf_acl_dip_trtcm(argc, argv);
			else if (!strncmp(argv[3], "meter", 6))
				p_switch_func->pf_acl_dip_meter(argc, argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "dmac", 5)) {
			if (!strncmp(argv[3], "add", 4))
				p_switch_func->pf_acl_mac_add(argc, argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "etype", 6)) {
			if (!strncmp(argv[3], "add", 4))
				p_switch_func->pf_acl_ethertype(argc, argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "port", 5)) {
			if (!strncmp(argv[3], "add", 4))
				p_switch_func->pf_acl_sp_add(argc, argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "L4", 3)) {
			if (!strncmp(argv[3], "add", 4))
				p_switch_func->pf_acl_l4_add(argc, argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "enable", 7))
			p_switch_func->pf_acl_port_enable(argc, argv);
		else if (!strncmp(argv[2], "acltbl-add", 11))
			p_switch_func->pf_acl_table_add(argc, argv);
		else if (!strncmp(argv[2], "masktbl-add", 12))
			p_switch_func->pf_acl_mask_table_add(argc, argv);
		else if (!strncmp(argv[2], "ruletbl-add", 12))
			p_switch_func->pf_acl_rule_table_add(argc, argv);
		else if (!strncmp(argv[2], "ratetbl-add", 12))
			p_switch_func->pf_acl_rate_table_add(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "vlan", 5)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dump", 5))
			p_switch_func->pf_vlan_dump(argc, argv);
		else if (!strncmp(argv[2], "set", 4))
			p_switch_func->pf_vlan_set(argc, argv);
		else if (!strncmp(argv[2], "clear", 6))
			p_switch_func->pf_vlan_clear(argc, argv);
		else if (!strncmp(argv[2], "vid", 4))
			p_switch_func->pf_doVlanSetVid(argc, argv);
		else if (!strncmp(argv[2], "pvid", 5))
			p_switch_func->pf_doVlanSetPvid(argc, argv);
		else if (!strncmp(argv[2], "acc-frm", 8))
			p_switch_func->pf_doVlanSetAccFrm(argc, argv);
		else if (!strncmp(argv[2], "port-attr", 10))
			p_switch_func->pf_doVlanSetPortAttr(argc, argv);
		else if (!strncmp(argv[2], "port-mode", 10))
			p_switch_func->pf_doVlanSetPortMode(argc, argv);
		else if (!strncmp(argv[2], "eg-tag-pcr", 11))
			p_switch_func->pf_doVlanSetEgressTagPCR(argc, argv);
		else if (!strncmp(argv[2], "eg-tag-pvc", 11))
			p_switch_func->pf_doVlanSetEgressTagPVC(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "reg", 4)) {
		parse_reg_cmd(argc, argv, 4);
	} else if (!strncmp(argv[1], "ingress-rate", 6)) {
		p_switch_func->pf_igress_rate_set(argc, argv);
	} else if (!strncmp(argv[1], "egress-rate", 6)) {
		p_switch_func->pf_egress_rate_set(argc, argv);
	} else if (!strncmp(argv[1], "igmpsnoop", 10)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "on", 3))
			p_switch_func->pf_igmp_on(argc, argv);
		else if (!strncmp(argv[2], "off", 4))
			p_switch_func->pf_igmp_off(argc, argv);
		else if (!strncmp(argv[2], "enable", 7))
			p_switch_func->pf_igmp_enable(argc, argv);
		else if (!strncmp(argv[2], "disable", 8))
			p_switch_func->pf_igmp_disable(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "collision-pool", 15)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "enable", 7))
			p_switch_func->pf_collision_pool_enable(argc, argv);
		else if (!strncmp(argv[2], "mac", 4)) {
			if (!strncmp(argv[3], "dump", 5))
				p_switch_func->pf_collision_pool_mac_dump(argc,
									  argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "dip", 4)) {
			if (!strncmp(argv[3], "dump", 5))
				p_switch_func->pf_collision_pool_dip_dump(argc,
									  argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "sip", 4)) {
			if (!strncmp(argv[3], "dump", 5))
				p_switch_func->pf_collision_pool_sip_dump(argc,
									  argv);
			else
				usage(argv[0]);
		} else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "pfc", 4)) {
		if (argc < 4 || argc > 5)
			usage(argv[0]);
		if (!strncmp(argv[2], "enable", 7))
			p_switch_func->pf_set_mac_pfc(argc, argv);
		else if (!strncmp(argv[2], "rx_counter", 11)) {
			p_switch_func->pf_pfc_get_rx_counter(argc, argv);
		} else if (!strncmp(argv[2], "tx_counter", 11)) {
			p_switch_func->pf_pfc_get_tx_counter(argc, argv);
		} else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "crossover", 10)) {
		if (argc < 4)
			usage(argv[0]);
		else
			phy_crossover(argc, argv);
	} else if (!strncmp(argv[1], "eee", 4)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "enable", 7) ||
		    !strncmp(argv[2], "disable", 8))
			p_switch_func->pf_eee_enable(argc, argv);
		else if (!strncmp(argv[2], "dump", 5))
			p_switch_func->pf_eee_dump(argc, argv);
		else
			usage(argv[0]);
	} else
		usage(argv[0]);
error:
	exit_free();
	return 0;
#endif
}

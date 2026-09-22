/* FILE NAME:   air_cmd.c
 * PURPOSE:
 *      Define the command line function in AIR SDK.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
*/
#include "air.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/
#define MAC_STR         "%02X%02X%02X%02X%02X%02X"
#define MAC2STR(m)      (m)[0],(m)[1],(m)[2],(m)[3],(m)[4],(m)[5]
#define AIR_MAC_LEN    (12)
#define CMD_NO_PARA     (0xFFFFFFFF)
#define CMD_VARIABLE_PARA (0xFFFFFFFE)
#define L2_WDOG_KICK_NUM            (100)

#define TOLOWER(x)      ((x) | 0x20)
#define isxdigit(c)     (('0' <= (c) && (c) <= '9') || ('a' <= (c) && (c) <= 'f') || ('A' <= (c) && (c) <= 'F'))
#define isdigit(c)      ('0' <= (c) && (c) <= '9')
#define CMD_CHECK_PARA(__shift__, __op__, __size__) do          \
{                                                               \
    if ((__shift__) __op__ (__size__))                          \
    {                                                           \
        ;                                                       \
    }                                                           \
    else                                                        \
    {                                                           \
        return (AIR_E_BAD_PARAMETER);                           \
    }                                                           \
} while(0)

#define CMD_IPV4_STR_SIZE        (16)
#define CMD_IPV4_TO_STR(__buf__,__ipv4__)    \
                        snprintf(__buf__,CMD_IPV4_STR_SIZE,"%d.%d.%d.%d",   \
                        ((__ipv4__)&0xFF000000)>>24,((__ipv4__)&0x00FF0000)>>16,    \
                        ((__ipv4__)&0x0000FF00)>>8, ((__ipv4__)&0x000000FF))

#define CMD_IPV6_STR_SIZE        (40)
#define CMD_IPV6_TO_STR(__buf__,__ipv6__)    \
                        snprintf(__buf__,CMD_IPV6_STR_SIZE, \
                        "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",  \
                        (__ipv6__)[0],  (__ipv6__)[1],  (__ipv6__)[2],  (__ipv6__)[3],    \
                        (__ipv6__)[4],  (__ipv6__)[5],  (__ipv6__)[6],  (__ipv6__)[7],    \
                        (__ipv6__)[8],  (__ipv6__)[9],  (__ipv6__)[10], (__ipv6__)[11],  \
                        (__ipv6__)[12], (__ipv6__)[13], (__ipv6__)[14], (__ipv6__)[15])

#define CMD_IP_ADDR_STR_SIZE     (CMD_IPV6_STR_SIZE)

/* DATA TYPE DECLARATIONS
*/
typedef struct {
    C8_T*               name;
    AIR_ERROR_NO_T     (*func)(UI32_T argc, C8_T *argv[]);
    UI32_T              argc_min;
    C8_T*               argc_errmsg;
} AIR_CMD_T;

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
*/
/* String Utility */
static BOOL_T _strcmp(const char *s1, const char *s2);
static C8_T * _strtok_r(C8_T *s, const C8_T *delim, C8_T **last);
static C8_T * _strtok(C8_T *s, const C8_T *delim, C8_T **last);
UI32_T _strtoul(const C8_T *cp, C8_T **endp, UI32_T base);
static I32_T _strtol(const C8_T *cp, C8_T **endp, UI32_T base);

/* Type Converter */
static AIR_ERROR_NO_T _str2mac(C8_T *str, C8_T *mac);
static AIR_ERROR_NO_T _hex2bit(const UI32_T hex, UI32_T *ptr_bit);
static AIR_ERROR_NO_T _hex2bitstr(const UI32_T hex, C8_T *ptr_bit_str, UI32_T str_len);
static AIR_ERROR_NO_T _portListStr2Ary(const C8_T *str, UI32_T *ary, const UI32_T ary_num);

/* Register Operation */
static AIR_ERROR_NO_T doRegRead(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doRegWrite(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doReg(UI32_T argc, C8_T *argv[]);

/* PHY Operation */
static AIR_ERROR_NO_T doPhyCL22Read(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPhyCL22Write(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPhyCL22(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPhyCL45Read(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPhyCL45Write(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPhyCL45(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPhy(UI32_T argc, C8_T *argv[]);

/* Porting setting */
static AIR_ERROR_NO_T doPortSetMatrix(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortSetVlanMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortSet(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doPortGetMatrix(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortGetVlanMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortGet(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doPort(UI32_T argc, C8_T *argv[]);

/* Vlan setting */
static AIR_ERROR_NO_T doVlanInitiate(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanCreate(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanDestroy(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanDestroyAll(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanDump(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanAddPortMem(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanDelPortMem(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doVlanSetFid(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetMemPort(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetIVL(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetPortBaseStag(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetStag(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetEgsTagCtlEn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetEgsTagCtlCon(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetEgsTagCtl(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetPortActFrame(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetLeakyVlanEn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetPortVlanAttr(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetIgsPortETagAttr(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetPortETagAttr(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetPortOuterTPID(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSetPvid(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanSet(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doVlanGetPortActFrame(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanGetLeakyVlanEn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanGetPortVlanAttr(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanGetIgsPortETagAttr(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanGetPortETagAttr(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanGetPortOuterTPID(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanGetPvid(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doVlanGet(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doVlan(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doFlowCtrl(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doJumbo(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doL2Add(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doL2Del(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doL2Clear(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doL2Get(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doL2Set(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doL2Dump(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doL2(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doAnMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLocalAdv(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doRemoteAdv(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortSpeed(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortDuplex(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortStatus(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortBckPres(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortPsMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortSmtSpdDwn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortSpTag(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortEnable(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPort5GBaseRMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortHsgmiiMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortSgmiiMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortRmiiMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doPortRgmiiMode(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doSptagEn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSptagMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSptagDecode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSptagEncode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSptag(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doMacAddr(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T _printMacEntry(AIR_MAC_ENTRY_T * mt, UI32_T age_unit, UI8_T count, UI8_T title);
static AIR_ERROR_NO_T doGetMacAddr(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMacAddrAgeOut(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doDumpMacAddr(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doLagMember(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLagMemberCnt(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLagPtseed(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLagHashtype(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLagDstInfo(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLagState(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLagSpsel(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLagGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLagSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLag(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doStpPortstate(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doStpGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doStpSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doStp(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doMirrorGetSid(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMirrorDelSid(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMirrorAddRlist(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMirrorAddTlist(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMirrorSetSessionEnable(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMirrorSetSession(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMirrorSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMirrorAdd(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMirrorGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMirrorDel(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMirror(UI32_T argc,C8_T *argv[]);

static AIR_ERROR_NO_T doMibClearPort(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMibClearAcl(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMibGetPort(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMibGetAcl(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMibClear(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMibGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doMib(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doQosScheduleAlgo(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doQosTrustMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doQosPri2Queue(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doQosDscp2Pri(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doQosRateLimitEnable(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doQosRateLimit(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doQosPortPriority(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doQosRateLimitExMngFrm(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doQosGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doQosSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doQos(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doDiagTxComply(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doDiagSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doDiagGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doDiag(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doLedMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLedState(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLedUsrDef(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLedBlkTime(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLedSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLedGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLed(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doSwitchCpuPortEn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSwitchCpuPort(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSwitchPhyLCIntrEn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSwitchPhyLCIntrSts(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSwitchSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSwitchGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSwitch(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doShowVersion(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doShow(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T doStormEnable(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doStormRate(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doFldMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSaLearning(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSaLimit(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSecGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSecSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doSec(UI32_T argc, C8_T *argv[]);

static void _air_acl_printRuleMap(UI32_T *rule_map, UI32_T ary_num);
static AIR_ERROR_NO_T doAclEn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclRule(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclUdfRule(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclRmvRule(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclRmvUdfRule(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclAction(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclRmvAction(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclDumpAction(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclTrtcm(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclTrtcmEn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclRmvTrtcm(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclPortEn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclDropEn(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclDropThrsh(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclDropPbb(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclMeter(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclDump(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclDel(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAclClear(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doAcl(UI32_T argc, C8_T *argv[]);

static void _ipmc_cmd_printTblHead(const AIR_IPMC_MATCH_TYPE_T match_type);
static AIR_IPMC_MATCH_TYPE_T _ipmc_cmd_getMatchType(AIR_IPMC_ENTRY_T *ptr_entry);
static void _printIpmcMcast(const UI32_T unit, AIR_IPMC_ENTRY_T *mcast, const AIR_IPMC_MATCH_TYPE_T match_type, UI8_T title);
static AIR_ERROR_NO_T doIpmcAddMcast(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcAddMcastMem(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcDelMcast(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcDelMcastMem(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcGetMcast(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcSearchMode(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcAdd(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcDel(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcClear(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmcDump(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doIpmc(UI32_T argc, C8_T *argv[]);


static AIR_ERROR_NO_T doLpdetClearStatus(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLpdetGetSrcMac(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLpdetGetCtrl(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLpdetGetStatus(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLpdetSetSrcMac(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLpdetSetCtrl(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLpdetClear(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLpdetGet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLpdetSet(UI32_T argc, C8_T *argv[]);
static AIR_ERROR_NO_T doLpdet(UI32_T argc, C8_T *argv[]);

static AIR_ERROR_NO_T subcmd(const AIR_CMD_T tab[], UI32_T argc, C8_T *argv[]);

/* STATIC VARIABLE DECLARATIONS
*/
const static C8_T *_sptag_vpm[] =
{
    "untagged",
    "8100",
    "predefined",
    "unknown"
};

const static C8_T *_sptag_pt[] =
{
    "disable pass through",
    "enable pass through"
};

const static C8_T *_air_mac_address_forward_control_string [] =
{
    "Default",
    "CPU include",
    "CPU exclude",
    "CPU only",
    "Drop"
};

static AIR_CMD_T regCmds[] =
{
    {"r",           doRegRead,      1,      "reg r <reg(4'hex)>"},
    {"w",           doRegWrite,     2,      "reg w <reg(4'hex)> <value(8'hex)>"},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T phyCL22Cmds[] =
{
    {"r",           doPhyCL22Read,  2,      "phy cl22 r <port(0..4)> <reg(2'hex)>"},
    {"w",           doPhyCL22Write, 3,      "phy cl22 w <port(0..4)> <reg(2'hex)> <value(4'hex)>"},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T phyCL45Cmds[] =
{
    {"r",           doPhyCL45Read,  3,      "phy cl45 r <port(0..4)> <dev(2'hex)> <reg(3'hex)>"},
    {"w",           doPhyCL45Write, 4,      "phy cl45 w <port(0..4)> <dev(2'hex)> <reg(3'hex)> <value(4'hex)>"},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T phyCmds[] =
{
    {"cl22",         doPhyCL22,     0,      NULL},
    {"cl45",         doPhyCL45,     0,      NULL},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T portSetCmds[] =
{
    {"matrix",      doPortSetMatrix,        2,                "port set matrix <port(0..6)> <matrix(6:0)>"},
    {"vlanMode",    doPortSetVlanMode,      2,                "port set vlanMode <port(0..6)> <vlanMode(0:matrix,1:fallback,2:check,3:security)>"},
    {"flowCtrl",    doFlowCtrl,             3,                "port set flowCtrl <port(0..6)> <dir(0:Tx,1:Rx)> <fc_en(1:En,0:Dis)>"},
    {"jumbo",       doJumbo,                2,                "port set jumbo <pkt_len(0:1518,1:1536,2:1552,3:max)> <frame_len(2..15)>"},
    {"anMode",      doAnMode,               2,                "port set anMode <port(0..4)> <en(0:force,1:AN)>"},
    {"localAdv",    doLocalAdv,             7,                "port set localAdv <port(0..4)> <10H(1:En,0:Dis)> <10F(1:En,0:Dis)> <100H(1:En,0:Dis)> <100F(1:En,0:Dis)> <1000F(1:En,0:Dis)> <pause(1:En,0:Dis)>"},
    {"speed",       doPortSpeed,            2,                "port set speed <port(0..4)> <speed(0:10M,1:100M,2:1G,3:2.5G)>"},
    {"duplex",      doPortDuplex,           2,                "port set duplex <port(0..4)> <duplex(0:half,1:full)>"},
    {"bckPres",     doPortBckPres,          2,                "port set bckPres <port(0..6)> <bckPres(1:En,0:Dis)>"},
    {"psMode",      doPortPsMode,           3,                "port set psMode <port(0..4)> <ls(1:En,0:Dis)> <eee(1:En,0:Dis)>"},
    {"smtSpdDwn",   doPortSmtSpdDwn,        3,                "port set smtSpdDwn <port(0..4)> <en(1:En,0:Dis)> <retry(2..5)>"},
    {"spTag",       doPortSpTag,            2,                "port set spTag <port(0..6)> <en(1:En,0:Dis)>"},
    {"enable",      doPortEnable,           2,                "port set enable <port(0..4)> <en(1:En,0:Dis)>"},
    {"5GBaseRMode", doPort5GBaseRMode,      CMD_NO_PARA,      "port set 5GBaseRMode"},
    {"hsgmiiMode",  doPortHsgmiiMode,       CMD_NO_PARA,      "port set hsgmiiMode"},
    {"sgmiiMode",   doPortSgmiiMode,        2,                "port set sgmiiMode <mode(0:AN,1:Force)> <speed(0:10M,1:100M,2:1G)>"},
    {"rmiiMode",    doPortRmiiMode,         1,                "port set rmiiMode <speed(0:10M,1:100M)>"},
    {"rgmiiMode",   doPortRgmiiMode,        1,                "port set rgmiiMode <speed(0:10M,1:100M,2:1G)>"},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T portGetCmds[] =
{
    {"matrix",      doPortGetMatrix,        1,                "port get matrix <port(0..6)>"},
    {"vlanMode",    doPortGetVlanMode,      1,                "port get vlanMode <port(0..6)>"},
    {"flowCtrl",    doFlowCtrl,             2,                "port get flowCtrl <port(0..6)> <dir(0:Tx,1:Rx)>"},
    {"jumbo",       doJumbo,                CMD_NO_PARA,      "port get jumbo"},
    {"anMode",      doAnMode,               1,                "port get anMode <port(0..4)>"},
    {"localAdv",    doLocalAdv,             1,                "port get localAdv <port(0..4)>"},
    {"remoteAdv",   doRemoteAdv,            1,                "port get remoteAdv <port(0..4)>"},
    {"speed",       doPortSpeed,            1,                "port get speed <port(0..4)>"},
    {"duplex",      doPortDuplex,           1,                "port get duplex <port(0..4)>"},
    {"status",      doPortStatus,           1,                "port get status <port(0..4)>"},
    {"bckPres",     doPortBckPres,          1,                "port get bckPres <port(0..6)>"},
    {"psMode",      doPortPsMode,           1,                "port get psMode <port(0..4)>"},
    {"smtSpdDwn",   doPortSmtSpdDwn,        1,                "port get smtSpdDwn <port(0..4)>"},
    {"spTag",       doPortSpTag,            1,                "port get spTag <port(0..6)>"},
    {"enable",      doPortEnable,           1,                "port get enable <port(0..4)>"},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T portCmds[] =
{
    {"set",         doPortSet,      0,      NULL},
    {"get",         doPortGet,      0,      NULL},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T sptagCmds[] =
{
    {"setEnable",       doSptagEn,          2,      "sptag setEnable port<port(0..6)> enable<1:enable 0:disable>"},
    {"getEnable",       doSptagEn,          1,      "sptag getEnable port<port(0..6)>"},
    {"setmode",         doSptagMode,        2,      "sptag setmode port<port(0..6)> mode<0:inset 1:replace>"},
    {"getmode",         doSptagMode,        1,      "sptag getmode port<port(0..6)>"},
    {"encode",          doSptagEncode,      7,      "sptag encode mode={ insert | replace } opc={ portmap | portid | lookup } dp={bitimap hex} vpm={ untagged | 8100 | 88a8 } pri=<UINT> cfi=<UINT> vid=<UINT> "},
    {"decode",          doSptagDecode,      4,      "sptag decode <byte(hex)> <byte(hex)> <byte(hex)> <byte(hex)>"},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T vlanSetCmds[] =
{
    {"fid",                 doVlanSetFid,               2,      "vlan set fid <vid(0..4095)> <fid(0..7)>"},
    {"memPort",             doVlanSetMemPort,           2,      "vlan set memPort <vid(0..4095)> <bitmap(6:0)>"},
    {"ivl",                 doVlanSetIVL,               2,      "vlan set ivl <vid(0..4095)> <(1:En,0:Dis)>"},
    {"portBaseStag",        doVlanSetPortBaseStag,      2,      "vlan set portBaseStag <vid(0..4095)> <(1:En,0:Dis)>"},
    {"stag",                doVlanSetStag,              2,      "vlan set stag <vid(0..4095)> <stag(0..4095)>"},
    {"egsTagCtlEn",         doVlanSetEgsTagCtlEn,       2,      "vlan set egsTagCtlEn <vid(0..4095)> <(1:En,0:Dis)>"},
    {"egsTagCtlCon",        doVlanSetEgsTagCtlCon,      2,      "vlan set egsTagCtlCon <vid(0..4095)> <(1:En,0:Dis)>"},
    {"egsTagCtl",           doVlanSetEgsTagCtl,         3,      "vlan set egsTagCtl <vid(0..4095)> <port(0..6)> <ctlType(0:untag,2:tagged)>"},

    {"portActFrame",        doVlanSetPortActFrame,      2,      "vlan set portActFrame <port(0..6)> <frameType(0:all,1:tagged,2:untagged)>"},
    {"leakyVlanEn",         doVlanSetLeakyVlanEn,       3,      "vlan set LeakyVlanEn <port(0..6)> <pktType(0:uc,1:mc,2:bc,3:ipmc)> <(1:En,0:Dis)>"},
    {"portVlanAttr",        doVlanSetPortVlanAttr,      2,      "vlan set portVlanAttr <port(0..6)> <vlanAttr(0:user,1:stack,2:translation,3:transparent)>"},
    {"igsPortETagAttr",     doVlanSetIgsPortETagAttr,   2,      "vlan set igsPortETagAttr <port(0..6)> <egsTagAttr(0:disable,1:consistent,4:untagged,5:swap,6:tagged,7:stack)>"},
    {"portEgsTagAttr",      doVlanSetPortETagAttr,      2,      "vlan set portEgsTagAttr <port(0..6)> <egsTagAttr(0:untagged,1:swap,2:tagged,3:stack)>"},
    {"portOuterTPID",       doVlanSetPortOuterTPID,     2,      "vlan set portOuterTPID <port(0..6)> <TPID(hex)>"},
    {"pvid",                doVlanSetPvid,              2,      "vlan set pvid <port(0..6)> <vid(0..4095)>"},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T vlanGetCmds[] =
{
    {"portActFrame",        doVlanGetPortActFrame,      1,      "vlan get portActFrame <port(0..6)>"},
    {"leakyVlanEn",         doVlanGetLeakyVlanEn,       1,      "vlan get leakyVlanEn <port(0..6)>"},
    {"portVlanAttr",        doVlanGetPortVlanAttr,      1,      "vlan get portVlanAttr <port(0..6)>"},
    {"igsPortETagAttr",     doVlanGetIgsPortETagAttr,   1,      "vlan get igsPortETagAttr <port(0..6)>"},
    {"portEgsTagAttr",      doVlanGetPortETagAttr,      1,      "vlan get portEgsTagAttr <port(0..6)>"},
    {"portOuterTPID",       doVlanGetPortOuterTPID,     1,      "vlan get portOuterTPID <port(0..6)>"},
    {"pvid",                doVlanGetPvid,              1,      "vlan get pvid <port(0..6)>"},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T vlanCmds[] =
{
    {"initiate",        doVlanInitiate,         9,      "vlan initiate <vid(0..4095)> <fid(0..7)> <bitmap(6:0)> <ivl(1:En,0:Dis)> <portbasestag(1:En,0:Dis)> <stag(0..4095)> <egstagctlen(1:En,0:Dis)> <egstagcon(1:En,0:Dis)> <taggedbitmap(6:0)>"},
    {"create",          doVlanCreate,           1,      "vlan create <vid(0..4095)>"},
    {"destroy",         doVlanDestroy,          1,      "vlan destroy [ <vid(0..4095)> | <vidRange(vid0-vid1)> ]"},
    {"destroyAll",      doVlanDestroyAll,       0,      "vlan destroyAll [ <restoreDefVlan(0:false,1:true)> | ]"},
    {"dump",            doVlanDump,             0,      "vlan dump [ <vid(0..4095)> | <vidRange(vid0-vid1)> | ]"},
    {"addPortMem",      doVlanAddPortMem,       2,      "vlan addPortMem <vid(0..4095)> <port(0..6)>"},
    {"delPortMem",      doVlanDelPortMem,       2,      "vlan addPortMem <vid(0..4095)> <port(0..6)>"},
    {"set",             doVlanSet,              0,      NULL},
    {"get",             doVlanGet,              0,      NULL},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T l2ClearCmds[] =
{
    {"mac",              doMacAddr,          CMD_NO_PARA,    "l2 clear mac"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T l2DelCmds[] =
{
    {"mac",             doMacAddr,           3,    "l2 del mac <mac(12'hex)> [ vid <vid(0..4095)> | fid <fid(0..15)> ]"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T l2AddCmds[] =
{
    {"mac",             doMacAddr,           7,    "l2 add mac <static(0:dynamic,1:static)> <unauth(0:auth,1:unauth)> <mac(12'hex)> <portlist(uintlist)> [ vid <vid(0..4095)> | fid <fid(0..15)> ] <src_mac_forward=(0:default,1:cpu-exclude,2:cpu-include,3:cpu-only,4:drop)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T l2SetCmds[] =
{
    {"macAddrAgeOut",   doMacAddrAgeOut,    1,    "l2 set macAddrAgeOut <time(1, 1000000)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T l2GetCmds[] =
{
    {"mac",             doGetMacAddr,       3,              "l2 get mac <mac(12'hex)> [ vid <vid(0..4095)> | fid <fid(0..15)> ]"},
    {"macAddrAgeOut",   doMacAddrAgeOut,    CMD_NO_PARA,    "l2 get macAddrAgeOut"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T l2DumpCmds[] =
{
    {"mac",                doDumpMacAddr,        0,    "l2 dump mac"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T l2Cmds[] =
{
    {"add",         doL2Add,        0,        NULL},
    {"del",         doL2Del,        0,        NULL},
    {"clear",       doL2Clear,      0,        NULL},
    {"get",         doL2Get,        0,        NULL},
    {"set",         doL2Set,        0,        NULL},
    {"dump",        doL2Dump,       0,        NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T lagGetCmds[] =
{
    {"member",      doLagMember,    1,              "lag get member group_id(0 or 1)"},
    {"dstInfo",     doLagDstInfo,   CMD_NO_PARA,    "lag get dstInfo"},
    {"ptseed",      doLagPtseed,    CMD_NO_PARA,    "lag get ptseed"},
    {"hashtype",    doLagHashtype,  CMD_NO_PARA,    "lag get hashtype"},
    {"state",       doLagState,     CMD_NO_PARA,    "lag get state"},
    {"spsel",       doLagSpsel,     CMD_NO_PARA,    "lag get spsel"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T lagSetCmds[] =
{
    {"member",       doLagMember,        4,    "lag set member <group_id(0 or 1)> <member_index(0..3)> <enable(0,1)> <port index(0..6)>"},
    {"dstInfo",      doLagDstInfo,       7,    "lag set dstInfo <sp(1:En,0:Dis)> <sa(1:En,0:Dis)> <da(1:En,0:Dis)> <sip(1:En,0:Dis)> <dip(1:En,0:Dis)> <sport(1:En,0:Dis)> <dport(1:En,0:Dis)>"},
    {"ptseed",       doLagPtseed,        1,    "lag set ptseed <hex32>"},
    {"hashtype",     doLagHashtype,      1,    "lag set hashtype <0-crc32lsb;1-crc32msb;2-crc16;3-xor4>"},
    {"state",        doLagState,         1,    "lag set state <state(1:En,0:Dis)>"},
    {"spsel",        doLagSpsel,         1,    "lag set spsel <soure port enable(1:En,0:Dis)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T lagCmds[] =
{
    {"get",          doLagGet,        0,        NULL},
    {"set",          doLagSet,        0,        NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T stpGetCmds[] =
{
    {"portstate",    doStpPortstate,  2,    "stp get portstate <port(0..6)> <fid(0..15)>"},
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T stpSetCmds[] =
{
    {"portstate",    doStpPortstate,  3,    "stp set portstate <port(0..6)> <fid(0..15)> <state(0:disable,1:listen,2:learn,3:forward)>"},
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T stpCmds[] =
{
    {"get",         doStpGet,           0,      NULL},
    {"set",         doStpSet,           0,      NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T mirrorSetCmds[] =
{
    {"session",        doMirrorSetSession,       6,      "mirror set session <sid(0,1)> <dst_port(UINT)> <state(1:En,0:Dis)> <tag(1:on, 0:off)> <list(UINTLIST)> <dir(0:none,1:tx,2:rx,3:both)>"},
    {"session-enable", doMirrorSetSessionEnable, 2,      "mirror set session-enable <sid(0,1)> <state(1:En,0:Dis)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T mirrorAddCmds[] =
{
    {"session-rlist",  doMirrorAddRlist,       2,      "mirror add session-rlist <sid(0,1)> <list(UINTLIST)>"},
    {"session-tlist",  doMirrorAddTlist,       2,      "mirror add session-tlist <sid(0,1)> <list(UINTLIST)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T mirrorGetCmds[] =
{
    {"session",        doMirrorGetSid,       1,      "mirror get session <sid(0,1)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T mirrorDelCmds[] =
{
    {"session",        doMirrorDelSid,       1,      "mirror del session <sid(0,1)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T mirrorCmds[] =
{
    {"set",         doMirrorSet,        0,      NULL},
    {"add",         doMirrorAdd,        0,      NULL},
    {"get",         doMirrorGet,        0,      NULL},
    {"del",         doMirrorDel,        0,      NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T mibClearCmds[] =
{
    {"port",        doMibClearPort,     1,      "mib clear port <port(0..6)>"},
    {"all",         doMibClearPort,     0,      "mib clear all"},
    {"acl",         doMibClearAcl,      0,      "mib clear acl"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T mibGetCmds[] =
{
    {"port",        doMibGetPort,       1,      "mib get port <port(0..6)>"},
    {"acl",         doMibGetAcl,        1,      "mib get acl <event(0..7)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T mibCmds[] =
{
    {"clear",       doMibClear,         0,      NULL},
    {"get",         doMibGet,           0,      NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T qosGetCmds[] =
{
    {"scheduleAlgo",    doQosScheduleAlgo,      2,  "qos get scheduleAlgo <portlist(UINTLIST)> <queue(UINT)>"},
    {"trustMode",       doQosTrustMode,         1,  "qos get trustMode <portlist(UINTLIST)>"},
    {"pri2Queue",       doQosPri2Queue,         0,  "qos get pri2Queue"},
    {"dscp2Pri",        doQosDscp2Pri,          1,  "qos get dscp2Pri <dscp(0..63)>"},
    {"rateLimitEnable", doQosRateLimitEnable,   1,  "qos get rateLimitEnable <portlist(UINTLIST)>"},
    {"rateLimit",       doQosRateLimit,         1,  "qos get rateLimit <portlist(UINTLIST)>"},
    {"portPriority",    doQosPortPriority,      1,  "qos get portPriority <portlist(UINTLIST)>"},
    {"rateLmtExMngFrm", doQosRateLimitExMngFrm, 0,  "qos get rateLmtExMngFrm"},
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T qosSetCmds[] =
{
    {"scheduleAlgo",    doQosScheduleAlgo,      4,  "qos set scheduleAlgo <portlist(UINTLIST)> <queue(UINT)> <scheduler(0:SP,1:WRR,2:WFQ)> <weight(0..128)>, weight 0 is valid only on sp mode"},
    {"trustMode",       doQosTrustMode,         2,  "qos set trustMode <portlist(UINTLIST)> <mode(0:port,1:1p-port,2:dscp-port,3:dscp-1p-port>"},
    {"pri2Queue",       doQosPri2Queue,         2,  "qos set pri2Queue <priority(0..7)> <queue(0..7)>"},
    {"dscp2Pri",        doQosDscp2Pri,          2,  "qos set dscp2Pri <dscp(0..63)> <priority(0..7)>"},
    {"rateLimitEnable", doQosRateLimitEnable,   3,  "qos set rateLimitEnable <portlist(UINTLIST)> <dir(0:egress,1:ingress)> <rate_en(1:En,0:Dis)>"},
    {"rateLimit",       doQosRateLimit,         5,  "qos set rateLimit <portlist(UINTLIST)> <I_CIR(0..80000)> <I_CBS(0..127)> <E_CIR(0..80000)> <E_CBS(0..127)>"},
    {"portPriority",    doQosPortPriority,      2,  "qos set portPriority <portlist(UINTLIST)> <priority(0..7)>"},
    {"rateLmtExMngFrm", doQosRateLimitExMngFrm, 2,  "qos set rateLmtExMngFrm <dir(0:egress)> <en(0:include,1:exclude)>"},
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T qosCmds[] =
{
    {"get",          doQosGet,        0,        NULL},
    {"set",          doQosSet,        0,        NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T diagSetCmds[] =
{
    {"txComply",    doDiagTxComply,     2,      "diag set txComply <phy(0..5)> <mode(0..8)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T diagGetCmds[] =
{
    {"txComply",    doDiagTxComply,     1,      "diag get txComply <phy(0..5)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T diagCmds[] =
{
    {"set",         doDiagSet,          0,      NULL},
    {"get",         doDiagGet,          0,      NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T ledSetCmds[] =
{
    {"mode",        doLedMode,          1,      "led set mode <mode(0:disable, 1..3:2 LED, 4:user-define)>"},
    {"state",       doLedState,         2,      "led set state <led(0..1)> <state(1:En,0:Dis)>"},
    {"usr",         doLedUsrDef,        4,      "led set usr <led(0..1)> <polarity(0:low, 1:high)> <on_evt(7'bin)> <blink_evt(10'bin)>"},
    {"time",        doLedBlkTime,       1,      "led set time <time(0..5:32ms~1024ms)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T ledGetCmds[] =
{
    {"mode",        doLedMode,          CMD_NO_PARA,      "led get mode"},
    {"state",       doLedState,         1,                "led get state <led(0..1)>"},
    {"usr",         doLedUsrDef,        1,                "led get usr <led(0..1)>"},
    {"time",        doLedBlkTime,       CMD_NO_PARA,      "led get time"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T ledCmds[] =
{
    {"set",         doLedSet,           0,      NULL},
    {"get",         doLedGet,           0,      NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T showCmds[] =
{
    {"version",     doShowVersion,        0,        NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T secGetCmds[] =
{
    {"stormEnable",     doStormEnable,   2,  "sec get stormEnable <port(0..6)> <type(0:bcst,1:mcst,2:ucst)>"},
    {"stormRate",       doStormRate,     2,  "sec get stormRate <port(0..6)> <type(0:bcst,1:mcst,2:ucst)>"},
    {"fldMode",         doFldMode,       2,  "sec get fldMode <port(0..6)> <type(0:bcst,1:mcst,2:ucst,3:qury>"},
    {"saLearning",      doSaLearning,    1,  "sec get saLearning <port(0..6)>"},
    {"saLimit",         doSaLimit,       1,  "sec get saLimit <port(0..6)>"},
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T secSetCmds[] =
{
    {"stormEnable",     doStormEnable,   3,  "sec set stormEnable <port(0..6)> <type(0:bcst,1:mcst,2:ucst)> <en(1:En,0:Dis)>"},
    {"stormRate",       doStormRate,     4,  "sec set stormRate <port(0..6)> <type(0:bcst,1:mcst,2:ucst)> <count(0..255)> <unit(0:64k,1:256k,2:1M,3:4M,4:16M)>"},
    {"fldMode",         doFldMode,       3,  "sec set fldMode <port(0..6)> <type(0:bcst,1:mcst,2:ucst,3:qury> <en(1:En,0:Dis)>"},
    {"saLearning",      doSaLearning,    2,  "sec set saLearning <port(0..6)> <learn(0:disable,1:enable)>"},
    {"saLimit",         doSaLimit,       3,  "sec set saLimit <port(0..6)> <mode(0:disable,1:enable)> <count(0..4095)>"},
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T secCmds[] =
{
    {"get",          doSecGet,        0,        NULL},
    {"set",          doSecSet,        0,        NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T switchSetCmds[] =
{
    {"cpuPortEn",   doSwitchCpuPortEn,   1,              "switch set cpuPortEn <cpu_en(1:En,0:Dis)>"},
    {"cpuPort",     doSwitchCpuPort,     1,              "switch set cpuPort <port_number>"},
    {"phyLCIntrEn",     doSwitchPhyLCIntrEn,     2,      "switch set phyLCIntrEn <phy(0..6)> <(1:En,0:Dis)>"},
    {"phyLCIntrSts",    doSwitchPhyLCIntrSts,    2,      "switch set phyLCIntrSts <phy(0..6)> <(1:Clear)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T switchGetCmds[] =
{
    {"cpuPortEn",   doSwitchCpuPortEn,   CMD_NO_PARA,      "switch get cpuPortEn"},
    {"cpuPort",     doSwitchCpuPort,     CMD_NO_PARA,      "switch get cpuPort"},
    {"phyLCIntrEn",     doSwitchPhyLCIntrEn,     1,        "switch get phyLCIntrEn <phy(0..6)>"},
    {"phyLCIntrSts",    doSwitchPhyLCIntrSts,    1,        "switch get phyLCIntrSts <phy(0..6)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T switchCmds[] =
{
    {"set",         doSwitchSet,        0,      NULL},
    {"get",         doSwitchGet,        0,      NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T aclSetCmds[] =
{
    {"en",              doAclEn,            1,                     "acl set en <en(1:En,0:Dis)>"},
    {"rule",            doAclRule,          CMD_VARIABLE_PARA,     "acl set rule <idx(0..127)>\n <state(0:Dis,1:En)> <reverse(0:Dis,1:En)> <end(0:Dis,1:En)>\n <portmap(7'bin)><ipv6(0:Dis,1:En,2:Not care)>\n[ dmac <dmac(12'hex)> <dmac_mask(12'hex)> ]\n[ smac <smac(12'hex)> <smac_mask(12'hex)> ]\n[ stag <stag(4'hex)> <stag_mask(4'hex)> ]\n[ ctag <ctag(4'hex)> <ctag_mask(4'hex)> ]\n[ etype <etype(4'hex)> <etype_mask(4'hex)> ]\n[ dip <dip(IPADDR)> <dip_mask(IPADDR)> ]\n[ sip <sip(IPADDR)> <sip_mask(IPADDR)> ]\n[ dscp <dscp(2'hex)> <dscp_mask(2'hex)> ]\n[ protocol <protocol(12'hex)> <protocol_mask(12'hex)> ]\n[ dport <dport(4'hex)> <dport_mask(4'hex)> ]\n[ sport <sport(4'hex)> <sport_mask(4'hex)> ]\n[ flow_label <flow_label(4'hex)> <flow_label_mask(4'hex)> ]\n[ udf <udf(4'hex)> <udf_mask(4'hex)> ] "},
    {"udfRule",         doAclUdfRule,       7,                     "acl set udfRule <idx(0..15)> <mode(0:pattern, 1:threshold)> [ <pat(4'hex)> <mask(4'hex)> | <low(4'hex)> <high(4'hex)> ] <start(0:MAC header, 1:L2 payload, 2:IPv4 header, 3:IPv6 header, 4:L3 payload, 5:TCP header, 6:UDP header, 7: L4 payload)> <offset(0..127,unit:2 bytes)> <portmap(7'bin)>"},
    {"action",          doAclAction,        CMD_VARIABLE_PARA,     "acl set action <idx(0..127)> \n[ forward <forward(0:Default,4:Exclude CPU,5:Include CPU,6:CPU only,7:Drop)> ]\n[ egtag <egtag(0:Default,1:Consistent,4:Untag,5:Swap,6:Tag,7:Stack)> ]\n[ mirrormap <mirrormap(2'bin)> ]\n[ priority <priority(0..7)> ]\n[ redirect <redirect(0:Dst,1:Vlan)> <portmap(7'bin)> ]\n[ leaky_vlan <leaky_vlan(1:En,0:Dis)> ]\n[ cnt_idx <cnt_idx(0..63)> ]\n[ rate_idx <rate_idx(0..31)> ] \n[ attack_idx <attack_idx(0..95)> ] \n[ vid <vid(0..4095)> ] \n[ manage <manage(1:En,0:Dis)> ] \n[ bpdu <bpdu(1:En,0:Dis)> ]\n[ class <class(0:Original,1:Defined)>[0..7] ]\n[ drop_pcd <drop_pcd(0:Original,1:Defined)> [red <red(0..7)>][yellow <yellow(0..7)>][green <green(0..7)>] ]\n[ color <color(0:Defined,1:Trtcm)> [ <defined_color(0:Dis,1:Green,2:Yellow,3:Red)> | <trtcm_idx(0..31)> ] ]"},
    {"trtcm",           doAclTrtcm,         5,                     "acl set trtcm <idx(1..31)> <cir(4'hex)> <pir(4'hex)> <cbs(4'hex)> <pbs(4'hex)>"},
    {"trtcmEn",         doAclTrtcmEn,       1,                     "acl set trtcmEn <en(1:En,0:Dis)>"},
    {"portEn",          doAclPortEn,        2,                     "acl set portEn <port(0..6)> <en(1:En,0:Dis)>"},
    {"dropEn",          doAclDropEn,        2,                     "acl set dropEn <port(0..6)> <en(1:En,0:Dis)>"},
    {"dropThrsh",       doAclDropThrsh,     5,                     "acl set dropThrsh <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)> <high(0..2047)> <low(0..2047)>"},
    {"dropPbb",         doAclDropPbb,       4,                     "acl set dropPbb <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)> <probability(0..1023)>"},
    {"meter",           doAclMeter,         3,                     "acl set meter <idx(0..31)> <en(1:En,0:Dis)> <rate(0..65535)>\n Note: Limit rate = rate * 64Kbps"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T aclGetCmds[] =
{
    {"en",              doAclEn,            CMD_NO_PARA,    "acl get en"},
    {"rule",            doAclRule,          1,              "acl get rule <idx(0..127)> "},
    {"udfRule",         doAclUdfRule,       1,              "acl get udfRule <idx(0..15)>"},
    {"action",          doAclAction,        1,              "acl get action <idx(0..127)>"},
    {"trtcm",           doAclTrtcm,         1,              "acl get trtcm <idx(1..31)>"},
    {"trtcmEn",         doAclTrtcmEn,       CMD_NO_PARA,    "acl get trtcmEn"},
    {"portEn",          doAclPortEn,        1,              "acl get portEn <port(0..6)>"},
    {"dropEn",          doAclDropEn,        1,              "acl get dropEn <port(0..6)>"},
    {"dropThrsh",       doAclDropThrsh,     3,              "acl get dropThrsh <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)>"},
    {"dropPbb",         doAclDropPbb,       3,              "acl get dropPbb <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)>"},
    {"meter",           doAclMeter,         1,              "acl get meter <idx(0..31)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T aclDelCmds[] =
{
    {"rule",        doAclRmvRule,          1,         "acl del rule <idx(0..127)>"},
    {"udfRule",     doAclRmvUdfRule,       1,         "acl del udfRule <idx(0..15)>"},
    {"action",      doAclRmvAction,        1,         "acl del action <idx(0..127)>"},
    {"trtcm",       doAclRmvTrtcm,         1,         "acl del trtcm <idx(0..31)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T aclClearCmds[] =
{
    {"rule",        doAclRmvRule,          CMD_NO_PARA,       "acl clear rule"},
    {"udfRule",     doAclRmvUdfRule,       CMD_NO_PARA,       "acl clear udfRule"},
    {"action",      doAclRmvAction,        CMD_NO_PARA,       "acl clear action"},
    {"trtcm",       doAclRmvTrtcm,         CMD_NO_PARA,       "acl clear trtcm"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T aclCmds[] =
{
    {"set",         doAclSet,           0,      NULL},
    {"get",         doAclGet,           0,      NULL},
    {"del",         doAclDel,           0,      NULL},
    {"clear",       doAclClear,         0,      NULL},
    {"dump",        doAclDump,          0,      NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T ipmcAddCmds[] = {
    {"mcast",       doIpmcAddMcast,        3,      "ipmc add mcast <vid(0..4095)> <gaddr(IPADDR)> <portmap(7'bin)> [ disable-egrs-vlan-filter(1:En,0:Dis) ]"},
    {"mcastMem",    doIpmcAddMcastMem,     3,      "ipmc add mcastMem <vid(0..4095)> <gaddr(IPADDR)> <portmap(7'bin)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T ipmcDelCmds[] = {
    {"mcast",       doIpmcDelMcast,        2,      "ipmc del mcast <vid(0..4095}> <gaddr(IPADDR)>"},
    {"mcastMem",    doIpmcDelMcastMem,     3,      "ipmc del mcastMem <vid(0..4095)> <gaddr(IPADDR)> <portmap(7'bin)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T ipmcGetCmds[] = {
    {"mcast",       doIpmcGetMcast,         2,       "ipmc get mcast <vid(0..4095}> <gaddr(IPADDR)>"},
    {"searchMode",  doIpmcSearchMode,       1,       "ipmc get searchMode <port(0..6)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T ipmcSetCmds[] = {
    {"searchMode",      doIpmcSearchMode,       2,       "ipmc set searchMode <port(0..6)> <mode(0:l2mc, 1:ipmc)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T ipmcCmds[] =
{
    {"add",         doIpmcAdd,        0,            NULL},
    {"del",         doIpmcDel,        0,            NULL},
    {"clear",       doIpmcClear,      CMD_NO_PARA,  "ipmc clear"},
    {"get",         doIpmcGet,        0,             NULL},
    {"set",         doIpmcSet,        0,            NULL},
    {"dump",        doIpmcDump,       1,            "ipmc dump searchType<0:dip4, 1:dip6>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T lpdetClearCmds[] = {
    {"status",      doLpdetClearStatus, 2,            "lpdet clear status <port(0..6)> <type(0:tx-lp-frame, 1:rx-lp-alarm)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T lpdetGetCmds[] = {
    {"srcMac",      doLpdetGetSrcMac,   CMD_NO_PARA,  "lpdet get srcMac"},
    {"control",     doLpdetGetCtrl,     1,            "lpdet get control <port(0..6)>"},
    {"status",      doLpdetGetStatus,   1,            "lpdet get status <port(0..6)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T lpdetSetCmds[] = {
    {"srcMac",      doLpdetSetSrcMac,   1,            "lpdet set srcMac <mac(12'hex)>"},
    {"control",     doLpdetSetCtrl,     3,            "lpdet set control <port(0..6)> <type(0:tx-lp-frame, 1:rx-lp-alarm)> <en(1:En,0:Dis)>"},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T lpdetCmds[] =
{
    {"clear",       doLpdetClear,       0,           NULL},
    {"get",         doLpdetGet,         0,           NULL},
    {"set",         doLpdetSet,         0,           NULL},
    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

static AIR_CMD_T Cmds[] =
{
    {"reg",         doReg,          0,      NULL},
    {"phy",         doPhy,          0,      NULL},
    {"port",        doPort,         0,      NULL},
    {"vlan",        doVlan,         0,      NULL},
    {"l2",          doL2,           0,      NULL},
    {"lag",         doLag,          0,      NULL},
    {"stp",         doStp,          0,      NULL},
    {"mirror",      doMirror,       0,      NULL},
    {"mib",         doMib,          0,      NULL},
    {"qos",         doQos,          0,      NULL},
    {"diag",        doDiag,         0,      NULL},
    {"led",         doLed,          0,      NULL},
    {"switch",      doSwitch,       0,      NULL},
    {"show",        doShow,         0,      NULL},
    {"sec",         doSec,          0,      NULL},
    {"acl",         doAcl,          0,      NULL},
    {"sptag",       doSptag,        0,      NULL},
    {"ipmc",        doIpmc,         0,      NULL},
    {"lpdet",       doLpdet,        0,      NULL},

    /* last entry, do not modify this entry */
    {NULL, NULL, 0, NULL},
};

/* EXPORTED SUBPROGRAM BODIES
*/

/* LOCAL SUBPROGRAM BODIES
*/
static BOOL_T
_strcmp(const char *s1, const char *s2)
{
    while(*s1 == *s2++)
        if (*s1++ == '\0')
            return (0);
    return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 -1));
}

static C8_T *
_strtok_r(
    C8_T *s,
    const C8_T *delim,
    C8_T **last)
{
    char *spanp;
    int c = 0, sc = 0;
    char *tok;

    if (s == NULL && (s = *last) == NULL)
    {
        return (NULL);
    }

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
    for (;;)
    {
        c = *s++;
        spanp = (char *)delim;
        do
        {
            if (c == (sc = *spanp++))
            {
                break;
            }
        } while (sc != 0);
        if (sc == 0)
        {
            break;
        }
    }

    if (c == 0)
    {   /* no non-delimiter characters */
        *last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;)
    {
        c = *s++;
        spanp = (char *)delim;
        do
        {
            if ((sc = *spanp++) == c)
            {
                if (c == 0)
                {
                    s = NULL;
                }
                else
                {
                    s[-1] = 0;
                }
                *last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

static C8_T *
_strtok(
    C8_T *s,
    const C8_T *delim,
    C8_T **last)
{
    return _strtok_r(s, delim, last);
}

UI32_T
_strtoul(
    const C8_T *cp,
    C8_T **endp,
    UI32_T base)
{
    UI32_T result = 0, value = 0;

    if (!base)
    {
        base = 10;
        if (*cp == '0')
        {
            base = 8;
            cp++;
            if ((TOLOWER(*cp) == 'x') && isxdigit(cp[1]))
            {
                cp++;
                base = 16;
            }
        }
    }
    else if (base == 16)
    {
        if (cp[0] == '0' && TOLOWER(cp[1]) == 'x')
        {
            cp += 2;
        }
    }
    while (isxdigit(*cp) &&
           (value = isdigit(*cp) ? *cp-'0' : TOLOWER(*cp)-'a'+10) < base)
    {
        result = result*base + value;
        cp++;
    }
    if (endp)
    {
        *endp = (char *)cp;
    }
    return result;
}

static I32_T
_strtol(
    const C8_T *cp,
    C8_T **endp,
    UI32_T base)
{
    if(*cp=='-')
    {
        return -_strtoul(cp + 1, endp, base);
    }
    return _strtoul(cp, endp, base);
}

static AIR_ERROR_NO_T
_str2mac(
        C8_T *str,
        C8_T *mac)
{
    UI32_T i = 0;
    C8_T tmpstr[3];

    /* check str */
    AIR_CHECK_PTR(str);
    AIR_PARAM_CHK(strlen(str) != AIR_MAC_LEN, AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(mac);

    for(i=0; i<6; i++)
    {
        strncpy(tmpstr, str+(i*2), 2);
        tmpstr[2] = '\0';
        mac[i] = _strtoul(tmpstr, NULL, 16);
    }

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_str2ipv4(
    const C8_T *ptr_str,
    UI32_T     *ptr_addr)
{
    UI32_T value = 0, idx = 0, shift = 0;

    AIR_CHECK_PTR(ptr_str);
    AIR_CHECK_PTR(ptr_addr);

    /* e.g. 192.168.1.2, strlen = 11 */
    for (idx = 0; idx < strlen(ptr_str); idx++)
    {
        if (('0' <= ptr_str[idx]) && ('9' >= ptr_str[idx]))
        {
            value = (value * 10) + (ptr_str[idx] - '0');
        }
        else if ('.' == ptr_str[idx])
        {
            CMD_CHECK_PARA(value, <, 256); /* Error: invalid value */
            CMD_CHECK_PARA(shift, <, 4);   /* Error: mem-overwrite */
            *ptr_addr |= value << (24 - shift * 8);
            shift += 1;
            value = 0;
        }
        else
        {
            return AIR_E_BAD_PARAMETER; /* Error: not a digit number or dot */
        }
    }
    CMD_CHECK_PARA(value, <, 256); /* Error: invalid value */
    CMD_CHECK_PARA(shift, ==, 3);  /* Error: not an ipv4 addr */
    *ptr_addr |= value << (24 - shift * 8);

    return AIR_E_OK;
}

AIR_ERROR_NO_T
_str2ipv6(
    const C8_T  *ptr_str,
    UI8_T       *ptr_addr)
{
    UI32_T              hex_value = 0, dec_value = 0, idx = 0;
    BOOL_T              double_colon = FALSE, ipv4_compatible = FALSE;
    UI32_T              double_colon_pos = 0, last_pos = 0;
    UI8_T               tmp_ipv6[16] = {0};

    AIR_CHECK_PTR(ptr_str);
    AIR_CHECK_PTR(ptr_addr);

    /* e.g. invalid:
     * 3ffe::c0a8:0:      last cannot be colon except double-colon
     * 3ffe:::c0a8:0      triple-colon
     * 3ffe::c0a8::0      two double-colons
     */

    /* e.g. valid:
     * 3ffe::c0a8:0       strlen = 12 (double-colon in middle)
     * 3ffe:c0a8:0::      strlen = 13 (double-colon in last)
     * ::3ffe:c0a8:0      strlen = 13 (double-colon in first)
     * 3ffe::192.168.0.0  strlen = 17 (IPv4-compatible address)
     */
    for (idx = 0; idx < strlen(ptr_str); idx++)
    {
        if (('0' <= ptr_str[idx]) && ('9' >= ptr_str[idx]))
        {
            hex_value = (hex_value << 4) + (ptr_str[idx] - '0');
            dec_value = (dec_value * 10) + (ptr_str[idx] - '0');
        }
        else if (('a' <= ptr_str[idx]) && ('f' >= ptr_str[idx]))
        {
            hex_value = (hex_value << 4) + (ptr_str[idx] - 'a') + 10;
        }
        else if (('A' <= ptr_str[idx]) && ('F' >= ptr_str[idx]))
        {
            hex_value = (hex_value << 4) + (ptr_str[idx] - 'A') + 10;
        }
        else if (':' == ptr_str[idx])
        {
            /* must belong to double-colon, calculate from last */
            if (0 == idx)
            {
                continue;
            }
            /* not the first ch but a double-colon */
            else if (':' == ptr_str[idx - 1])
            {
                CMD_CHECK_PARA(double_colon, ==, FALSE); /* Error: triple-colon or two double-colons */
                double_colon = TRUE;
            }
            /* not the first ch and a double-colon */
            else
            {
                CMD_CHECK_PARA(double_colon_pos, <, 15); /* Error: only 16 units for UI8_T  */
                CMD_CHECK_PARA(last_pos,         <, 15); /* Error: only 16 units for UI8_T  */
                tmp_ipv6[last_pos]     = (UI8_T)((hex_value >> 8) & 0xff);
                tmp_ipv6[last_pos + 1] = (UI8_T)((hex_value >> 0) & 0xff);
                double_colon_pos += (FALSE == double_colon)? 2 : 0;
                last_pos += 2;
                hex_value = 0;
                dec_value = 0;
            }
        }
        else if ('.' == ptr_str[idx])
        {
            CMD_CHECK_PARA(last_pos, <, 16); /* Error: only 16 units for UI8_T  */
            tmp_ipv6[last_pos] = dec_value;
            last_pos += 1;
            dec_value = 0;
            ipv4_compatible = TRUE;
        }
        else
        {
            return AIR_E_BAD_PARAMETER; /* Error: not a hex number or colon */
        }
    }

    /* last data */
    if ((idx != 0) && (':' != ptr_str[idx - 1]))
    {
        if (FALSE == ipv4_compatible)
        {
            CMD_CHECK_PARA(last_pos, <, 15); /* Error: only 16 units for UI8_T  */
            tmp_ipv6[last_pos]     = (UI8_T)((hex_value >> 8) & 0xff);
            tmp_ipv6[last_pos + 1] = (UI8_T)((hex_value >> 0) & 0xff);
            last_pos += 2;
        }
        else
        {
            CMD_CHECK_PARA(last_pos, <, 16); /* Error: only 16 units for UI8_T  */
            tmp_ipv6[last_pos] = dec_value;
            last_pos += 1;
        }
    }
    else
    {
        if ((idx >= 2) && (':' != ptr_str[idx - 2]))
        {
            return AIR_E_BAD_PARAMETER; /* Error: last cannot be colon except double-colon */
        }
    }

    /* move tmp_ipv6 to ptr_value */
    if (TRUE == double_colon)
    {
        /* e.g.
         * 3ffe::c0a8:0       double_colon_pos = 2, last_pos = 4+2, tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
         * 3ffe:c0a8:0::      double_colon_pos = 6, last_pos = 6,   tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
         * ::3ffe:c0a8:0      double_colon_pos = 0, last_pos = 4+2, tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
         * 3ffe::192.168.0.0  double_colon_pos = 2, last_pos = 5+1, tmp_ipv6 = {3f,fe,c0,a8,00,00,...}
         *
         *                                 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
         * 3ffe::c0a8:0       ptr_value = {3f,fe,--,--,--,--,--,--,--,--,--,--,--,--,--,--}
         * 3ffe:c0a8:0::      ptr_value = {3f,fe,c0,a8,00,00,--,--,--,--,--,--,--,--,--,--}
         * ::3ffe:c0a8:0      ptr_value = {--,--,--,--,--,--,--,--,--,--,--,--,--,--,--,--}
         * 3ffe::192.168.0.0  ptr_value = {3f,fe,--,--,--,--,--,--,--,--,--,--,--,--,--,--}
         */
        for (idx = 0; idx < double_colon_pos; idx++)
        {
            ptr_addr[idx] = tmp_ipv6[idx];
        }
        /* e.g.
         *                                 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
         * 3ffe::c0a8:0       ptr_value = {3f,fe,--,--,--,--,--,--,--,--,--,--,c0,a8,00,00}
         * 3ffe:c0a8:0::      ptr_value = {3f,fe,c0,a8,00,00,--,--,--,--,--,--,--,--,--,--}
         * ::3ffe:c0a8:0      ptr_value = {--,--,--,--,--,--,--,--,--,--,3f,fe,c0,a8,00,00}
         * 3ffe::192.168.0.0  ptr_value = {3f,fe,--,--,--,--,--,--,--,--,--,--,c0,a8,00,00}
         */
        for (idx = double_colon_pos; idx < last_pos; idx++)
        {
            ptr_addr[16 - (last_pos - idx)] = tmp_ipv6[idx];
        }
    }
    else
    {
        for (idx = 0; idx < 16; idx++)
        {
            ptr_addr[idx] = tmp_ipv6[idx];
        }
    }

    return AIR_E_OK;
}

void
_showIpv6Str(
    const UI8_T *ptr_ipv6,
    C8_T *ptr_str)
{
    UI32_T idx = 0, next = 0, last = 16;
    UI32_T cont_zero = 0;

    while (idx < last)
    {
        if ((0 == cont_zero) && (0 == ptr_ipv6[idx]) && (0 == ptr_ipv6[idx + 1]))
        {
            next = idx + 2;

            while (next < last)
            {
                if ((ptr_ipv6[next]) || (ptr_ipv6[next + 1]))
                {
                    AIR_PRINT(
                            ptr_str + strlen(ptr_str),
                            40 - strlen(ptr_str),
                            "%s", (cont_zero) ? (":") : (":0"));
                    break;
                }

                if (0 == cont_zero)
                {
                    cont_zero = 1;
                }
                next += 2;
            }

            if (next == last)
            {

                AIR_PRINT(
                        ptr_str + strlen(ptr_str),
                        40 - strlen(ptr_str),
                        "%s", (cont_zero) ? ("::") : (":0"));
            }

            idx = next;
        }
        else
        {
            if (idx)
            {
                AIR_PRINT(
                    ptr_str + strlen(ptr_str),
                    40 - strlen(ptr_str),
                    ":");
            }

            if (ptr_ipv6[idx])
            {
                AIR_PRINT(
                    ptr_str + strlen(ptr_str),
                    40 - strlen(ptr_str),
                    "%0x%02x", ptr_ipv6[idx], ptr_ipv6[idx + 1]);
            }
            else
            {
                AIR_PRINT(
                    ptr_str + strlen(ptr_str),
                    40 - strlen(ptr_str),
                    "%0x", ptr_ipv6[idx + 1]);
            }

            idx += 2;
        }
    }
}

void
_getIpv4Str(
    const AIR_IPV4_T *ptr_ipv4,
    C8_T *ptr_str)
{
    int ret;
    ret = CMD_IPV4_TO_STR(ptr_str, *ptr_ipv4);
    if (ret < 0)
        printf("Encoding error in snprintf\n");
}

void
_getIpv6Str(
    const AIR_IPV6_T *ptr_ipv6,
    C8_T *ptr_str)
{
    UI32_T idx = 0, next = 0, last = 16;
    UI32_T cont_zero = 0;
    int ret;

    while (idx < last)
    {
        if ((0 == cont_zero) && (0 == (*ptr_ipv6)[idx]) && (0 == (*ptr_ipv6)[idx + 1]))
        {
            next = idx + 2;

            while (next < last)
            {
                if (((*ptr_ipv6)[next]) || ((*ptr_ipv6)[next + 1]))
                {
                    ret = snprintf(
                             ptr_str + strlen(ptr_str),
                             CMD_IPV6_STR_SIZE - strlen(ptr_str),
                             "%s", (cont_zero) ? (":") : (":0"));
                    if (ret < 0) {
                        printf("Encoding error in snprintf\n");
                        return;
                    }
                    break;
                }

                if (0 == cont_zero)
                {
                    cont_zero = 1;
                }
                next += 2;
            }

            if (next == last)
            {
                ret = snprintf(
                         ptr_str + strlen(ptr_str),
                         CMD_IPV6_STR_SIZE - strlen(ptr_str),
                         "%s", (cont_zero) ? ("::") : (":0"));
                if (ret < 0) {
                    printf("Encoding error in snprintf\n");
                    return;
                }
            }

            idx = next;
        }
        else
        {
            if (idx)
            {
                ret = snprintf(
                     ptr_str + strlen(ptr_str),
                     CMD_IPV6_STR_SIZE - strlen(ptr_str),
                     ":");
                if (ret < 0) {
                    printf("Encoding error in snprintf\n");
                    return;
                }
            }

            if ((*ptr_ipv6)[idx])
            {
                ret = snprintf(
                     ptr_str + strlen(ptr_str),
                     CMD_IPV6_STR_SIZE - strlen(ptr_str),
                     "%0x%02x", (*ptr_ipv6)[idx], (*ptr_ipv6)[idx + 1]);
                if (ret < 0) {
                    printf("Encoding error in snprintf\n");
                    return;
                }
            }
            else
            {
                ret = snprintf(
                     ptr_str + strlen(ptr_str),
                     CMD_IPV6_STR_SIZE - strlen(ptr_str),
                     "%0x", (*ptr_ipv6)[idx + 1]);
                if (ret < 0) {
                    printf("Encoding error in snprintf\n");
                    return;
                }
            }

            idx += 2;
        }
    }
}

void
_getIpAddrStr(
    const AIR_IP_ADDR_T *ptr_ip_addr,
    C8_T *ptr_str)
{
    if(TRUE == ptr_ip_addr->ipv4)
    {
        _getIpv4Str(&ptr_ip_addr->ip_addr.ipv4_addr, ptr_str);
    }
    else
    {
        _getIpv6Str(&ptr_ip_addr->ip_addr.ipv6_addr, ptr_str);
    }
}
static AIR_ERROR_NO_T
_hex2bit(
        const UI32_T hex,
        UI32_T *ptr_bit)
{
    UI32_T i = 0;

    /* Mistake proofing */
    AIR_CHECK_PTR(ptr_bit);

    (*ptr_bit) = 0;
    for(i=0; i<AIR_MAX_NUM_OF_PORTS; i++)
    {
        if(hex & BIT(i))
        {
            (*ptr_bit) |= BITS_OFF_L(1UL, 4*(AIR_MAX_NUM_OF_PORTS - i - 1), 4);
        }
    }
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hex2bitstr(
        const UI32_T hex,
        C8_T *ptr_bit_str,
        UI32_T str_len)
{
    UI32_T i = 0;
    C8_T str_bitmap[AIR_MAX_NUM_OF_PORTS+1];

    /* Mistake proofing */
    AIR_CHECK_PTR(ptr_bit_str);
    AIR_PARAM_CHK(str_len <= AIR_MAX_NUM_OF_PORTS, AIR_E_BAD_PARAMETER);

    memset(str_bitmap, 0, AIR_MAX_NUM_OF_PORTS+1);

    for(i=0; i<AIR_MAX_NUM_OF_PORTS; i++)
    {
        if(hex & BIT(i))
        {
            str_bitmap[i] = '1';
        }
        else
        {
            str_bitmap[i] = '-';
        }
    }
    str_bitmap[i] = '\0';
    strncpy(ptr_bit_str, str_bitmap, i+1);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_portListStr2Ary(
    const C8_T *str,
    UI32_T *ary,
    const UI32_T ary_num)
{
    UI32_T i = 0;
    UI32_T str_len = 0;
    UI32_T val = 0;
    C8_T *str2;
    C8_T *pch;
    C8_T *last;

    /* Mistake proofing */
    AIR_CHECK_PTR(str);
    AIR_CHECK_PTR(ary);
    AIR_PARAM_CHK(0 == ary_num, AIR_E_BAD_PARAMETER);

    /* Allocate new string */
    str_len = strlen(str);
    str2 = AIR_MALLOC(str_len+1);
    AIR_CHECK_PTR(str2);
    memset(str2, 0, str_len+1);
    strncpy(str2, str, str_len+1);

    /* clear array */
    memset(ary, 0, ary_num*4);

    /* split string by ',' */
    pch = _strtok(str2, ",", &last);
    while(NULL != pch)
    {
        val = _strtoul(pch, NULL, 0);
        ary[val/32] |= BIT(val%32);
        pch = _strtok(NULL, ",", &last);
    }

    AIR_FREE(str2);
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
doRegRead(
        UI32_T argc,
        C8_T *argv[])
{
    UI32_T reg = 0, val = 0;

    reg = _strtoul(argv[0], NULL, 16);
    aml_readReg(0, reg, &val);
    AIR_PRINT("Read reg=0x%x, value=0x%x\n", reg, val);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
doRegWrite(
        UI32_T argc,
        C8_T *argv[])
{
    UI32_T reg = 0, val = 0;

    reg = _strtoul(argv[0], NULL, 16);
    val = _strtoul(argv[1], NULL, 16);
    aml_writeReg(0, reg, val);
    AIR_PRINT("Write reg=0x%x, value=0x%x\n", reg, val);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
doReg(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(regCmds, argc, argv);
}

static AIR_ERROR_NO_T
doPhyCL22Read(
        UI32_T argc,
        C8_T *argv[])
{
    UI32_T port = 0, reg = 0, val = 0;

    port = _strtoul(argv[0], NULL, 0);
    reg  = _strtoul(argv[1], NULL, 16);
    aml_readPhyReg(0, port, reg, &val);
    AIR_PRINT("Phy read port=%d, reg=0x%x, value=0x%x\n", port, reg, val);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
doPhyCL22Write(
        UI32_T argc,
        C8_T *argv[])
{
    UI32_T port = 0, reg = 0, val = 0;

    port = _strtoul(argv[0], NULL, 0);
    reg  = _strtoul(argv[1], NULL, 16);
    val  = _strtoul(argv[2], NULL, 16);
    aml_writePhyReg(0, port, reg, val);
    AIR_PRINT("Phy write port=%d, reg=0x%x, value=0x%x\n", port, reg, val);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
doPhyCL22(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(phyCL22Cmds, argc, argv);
}

static AIR_ERROR_NO_T
doPhyCL45Read(
        UI32_T argc,
        C8_T *argv[])
{
    UI32_T port = 0, dev = 0, reg = 0, val = 0;

    port = _strtoul(argv[0], NULL, 0);
    dev  = _strtoul(argv[1], NULL, 16);
    reg  = _strtoul(argv[2], NULL, 16);
    aml_readPhyRegCL45(0, port, dev, reg, &val);
    AIR_PRINT("Phy read port=%d, dev=0x%x, reg=0x%x, value=0x%x\n", port, dev, reg, val);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
doPhyCL45Write(
        UI32_T argc,
        C8_T *argv[])
{
    UI32_T port = 0, dev = 0, reg = 0, val = 0;

    port = _strtoul(argv[0], NULL, 0);
    dev  = _strtoul(argv[1], NULL, 16);
    reg  = _strtoul(argv[2], NULL, 16);
    val  = _strtoul(argv[3], NULL, 16);
    aml_writePhyRegCL45(0, port, dev, reg, val);
    AIR_PRINT("Phy write port=%d, dev=0x%x, reg=0x%x, value=0x%x\n", port, dev, reg, val);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
doPhyCL45(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(phyCL45Cmds, argc, argv);
}

static AIR_ERROR_NO_T
doPhy(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(phyCmds, argc, argv);
}

static AIR_ERROR_NO_T
doPortSetMatrix(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    UI32_T matrix = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port   = _strtoul(argv[0], NULL, 0);
    matrix = _strtoul(argv[1], NULL, 16);
    rc = air_port_setPortMatrix(0, port, matrix);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doPortSetVlanMode(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    AIR_PORT_VLAN_MODE_T vlan_mode;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port      = _strtoul(argv[0], NULL, 0);
    vlan_mode = _strtoul(argv[1], NULL, 0);
    rc = air_port_setVlanMode(0, port, vlan_mode);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doPortSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(portSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doPortGetMatrix(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    UI32_T matrix = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    rc = air_port_getPortMatrix(0, port, &matrix);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
        return rc;
    }
    AIR_PRINT("Port %d Matrix: %2x\n", port, matrix);
    return rc;
}

static AIR_ERROR_NO_T
doPortGetVlanMode(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    AIR_PORT_VLAN_MODE_T vlan_mode;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    rc = air_port_getVlanMode(0, port, &vlan_mode);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
        return rc;
    }
    AIR_PRINT("Port %d Vlan Mode: ", port);
    switch(vlan_mode)
    {
        case AIR_PORT_VLAN_MODE_PORT_MATRIX:
            AIR_PRINT("matrix(%d)\n", vlan_mode);
            break;
        case AIR_PORT_VLAN_MODE_FALLBACK:
            AIR_PRINT("fallback(%d)\n", vlan_mode);
            break;
        case AIR_PORT_VLAN_MODE_CHECK:
            AIR_PRINT("check(%d)\n", vlan_mode);
            break;
        case AIR_PORT_VLAN_MODE_SECURITY:
            AIR_PRINT("security(%d)\n", vlan_mode);
            break;
        default:
            AIR_PRINT("unknown(%d)\n", vlan_mode);
            break;
    };
    return rc;
}

static AIR_ERROR_NO_T
doPortGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(portGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doPort(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(portCmds, argc, argv);
}

static AIR_ERROR_NO_T
doVlanInitiate(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0;
    AIR_VLAN_ENTRY_ATTR_T vlan_entry = {0};
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid = _strtoul(argv[0], NULL, 0);
    if (9 == argc)
    {
        vlan_entry.vlan_entry_format.fid           = _strtoul(argv[1], NULL, 0);
        vlan_entry.vlan_entry_format.port_mem      = _strtoul(argv[2], NULL, 0);
        vlan_entry.vlan_entry_format.ivl           = _strtoul(argv[3], NULL, 0);
        vlan_entry.vlan_entry_format.port_stag     = _strtoul(argv[4], NULL, 0);
        vlan_entry.vlan_entry_format.stag          = _strtoul(argv[5], NULL, 0);
        vlan_entry.vlan_entry_format.eg_ctrl_en    = _strtoul(argv[6], NULL, 0);
        vlan_entry.vlan_entry_format.eg_con        = _strtoul(argv[7], NULL, 0);
        vlan_entry.vlan_entry_format.eg_ctrl       = _strtoul(argv[8], NULL, 0);

        rc = air_vlan_create(0, vid, &vlan_entry);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }

    switch (rc)
    {
        case     AIR_E_OK:                                                            break;
        case     AIR_E_ENTRY_EXISTS:  AIR_PRINT("VLAN already exist!\n");             break;
        default:                      AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanCreate(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid = _strtoul(argv[0], NULL, 0);
    rc  = air_vlan_create(0, vid, NULL);

    switch (rc)
    {
        case     AIR_E_OK:                                                            break;
        case     AIR_E_ENTRY_EXISTS:  AIR_PRINT("VLAN already exist!\n");             break;
        default:                      AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanDestroy(UI32_T argc, C8_T *argv[])
{
    C8_T *token = NULL;
    UI16_T vid = 0, vid_limit = AIR_VLAN_ID_MAX;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (argc > 0)
    {
        if (isdigit(argv[0][0]))
        {
            token = _strtok(argv[0], "-", &argv[0]);
            vid = _strtoul(token, NULL, 0);
            if ((token = _strtok(argv[0], "-", &argv[0])))
                vid_limit = _strtoul(token, NULL, 0);
            else
                vid_limit = vid;
            if (AIR_VLAN_ID_MAX < vid_limit)
            {
                AIR_PRINT("vid number should less than %d!\n", AIR_VLAN_ID_MAX);
                return AIR_E_BAD_PARAMETER;
            }
            if (vid > vid_limit)
            {
                AIR_PRINT("vid0 should less than vid1!\n");
                return AIR_E_BAD_PARAMETER;
            }
        }
        else
        {
            AIR_PRINT("Bad parameter!\n");
            return AIR_E_BAD_PARAMETER;
        }
    }

    for (; vid <= vid_limit; vid++)
    {
        rc = air_vlan_destroy(0, vid);
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanDestroyAll(UI32_T argc, C8_T *argv[])
{
    UI32_T restore_def_vlan = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (argc > 0)
    {
        restore_def_vlan = _strtoul(argv[0], NULL, 0);
    }

    rc = air_vlan_destroyAll(0, restore_def_vlan);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanDump(UI32_T argc, C8_T *argv[])
{
    C8_T *token = NULL;
    UI16_T port = 0, valid_count = 0, vid = 0, vid_limit = AIR_VLAN_ID_MAX;
    AIR_PORT_EGS_TAG_ATTR_T tag_ctl[AIR_MAX_NUM_OF_PORTS] = {0};
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    if (argc > 0)
    {
        if (isdigit(argv[0][0]))
        {
            token = _strtok(argv[0], "-", &argv[0]);
            vid = _strtoul(token, NULL, 0);
            if ((token = _strtok(argv[0], "-", &argv[0])))
                vid_limit = _strtoul(token, NULL, 0);
            else
                vid_limit = vid;
            if (AIR_VLAN_ID_MAX < vid_limit)
            {
                AIR_PRINT("vid number should less than %d!\n", AIR_VLAN_ID_MAX);
                return AIR_E_BAD_PARAMETER;
            }
            if (vid > vid_limit)
            {
                AIR_PRINT("vid0 should less than vid1!\n");
                return AIR_E_BAD_PARAMETER;
            }
        }
        else
        {
            AIR_PRINT("Bad parameter!\n");
            return AIR_E_BAD_PARAMETER;
        }
    }

    for (valid_count = 0; vid <= vid_limit; vid++)
    {
        _air_vlan_readEntry(0, vid, &vlan_entry);
        if (vlan_entry.valid)
        {
            valid_count++;
            if (1 == valid_count)
                AIR_PRINT(" Vid Fid MemPort Ivl PortBaseStag Stag EgsTagCtlEn EgsTagCon EgsTagCtl\n======================================================================\n");
            for (port = 0; port < AIR_MAX_NUM_OF_PORTS; port++)
                tag_ctl[port] = (vlan_entry.vlan_entry_format.eg_ctrl >> (port * 2)) & 0x3;
            AIR_PRINT("%4d %3d      %2x %3d %12d %4d %11d %9d   %1x%1x%1x%1x%1x%1x%1x\n",
                vid, vlan_entry.vlan_entry_format.fid, vlan_entry.vlan_entry_format.port_mem, vlan_entry.vlan_entry_format.ivl,
                vlan_entry.vlan_entry_format.port_stag, vlan_entry.vlan_entry_format.stag, vlan_entry.vlan_entry_format.eg_ctrl_en, vlan_entry.vlan_entry_format.eg_con,
                tag_ctl[6], tag_ctl[5], tag_ctl[4], tag_ctl[3], tag_ctl[2], tag_ctl[1], tag_ctl[0]);
        }
    }

    if (!valid_count)
        AIR_PRINT("not found!\n");
    else
        AIR_PRINT("Found %d valid entries!\n", valid_count);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
doVlanAddPortMem(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0, port = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid  = _strtoul(argv[0], NULL, 0);
    port = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_addMemberPort(0, vid, port);
    switch (rc)
    {
        case     AIR_E_OK:                                                               break;
        case     AIR_E_ENTRY_NOT_FOUND:  AIR_PRINT("VLAN not found!\n");                 break;
        default:                         AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanDelPortMem(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0, port = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid  = _strtoul(argv[0], NULL, 0);
    port = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_delMemberPort(0, vid, port);
    switch (rc)
    {
        case     AIR_E_OK:                                                               break;
        case     AIR_E_ENTRY_NOT_FOUND:  AIR_PRINT("VLAN not found!\n");                 break;
        default:                         AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetFid(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0;
    UI8_T  fid = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid = _strtoul(argv[0], NULL, 0);
    fid = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setFid(0, vid, fid);
    switch (rc)
    {
        case     AIR_E_OK:                                                               break;
        case     AIR_E_ENTRY_NOT_FOUND:  AIR_PRINT("VLAN not found!\n");                 break;
        default:                         AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetMemPort(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0, port_bitmap = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid         = _strtoul(argv[0], NULL, 0);
    port_bitmap = _strtoul(argv[1], NULL, 16);
    rc = air_vlan_setMemberPort(0, vid, port_bitmap);
    switch (rc)
    {
        case     AIR_E_OK:                                                               break;
        case     AIR_E_ENTRY_NOT_FOUND:  AIR_PRINT("VLAN not found!\n");                 break;
        default:                         AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetIVL(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0;
    BOOL_T enable = TRUE;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid    = _strtoul(argv[0], NULL, 0);
    enable = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setIVL(0, vid, enable);
    switch (rc)
    {
        case     AIR_E_OK:                                                               break;
        case     AIR_E_ENTRY_NOT_FOUND:  AIR_PRINT("VLAN not found!\n");                 break;
        default:                         AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetPortBaseStag(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0;
    BOOL_T enable = TRUE;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid    = _strtoul(argv[0], NULL, 0);
    enable = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setPortBasedStag(0, vid, enable);
    switch (rc)
    {
        case     AIR_E_OK:                                                               break;
        case     AIR_E_ENTRY_NOT_FOUND:  AIR_PRINT("VLAN not found!\n");                 break;
        default:                         AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetStag(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0, stag = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid  = _strtoul(argv[0], NULL, 0);
    stag = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setServiceTag(0, vid, stag);
    switch (rc)
    {
        case     AIR_E_OK:                                                               break;
        case     AIR_E_ENTRY_NOT_FOUND:  AIR_PRINT("VLAN not found!\n");                 break;
        default:                         AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetEgsTagCtlEn(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0;
    BOOL_T enable = TRUE;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid    = _strtoul(argv[0], NULL, 0);
    enable = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setEgsTagCtlEnable(0, vid, enable);
    switch (rc)
    {
        case     AIR_E_OK:                                                               break;
        case     AIR_E_ENTRY_NOT_FOUND:  AIR_PRINT("VLAN not found!\n");                 break;
        default:                         AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetEgsTagCtlCon(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0;
    BOOL_T enable = TRUE;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid    = _strtoul(argv[0], NULL, 0);
    enable = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setEgsTagConsistent(0, vid, enable);
    switch (rc)
    {
        case     AIR_E_OK:                                                               break;
        case     AIR_E_ENTRY_NOT_FOUND:  AIR_PRINT("VLAN not found!\n");                 break;
        default:                         AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetEgsTagCtl(UI32_T argc, C8_T *argv[])
{
    UI16_T vid = 0, port = 0;
    AIR_PORT_EGS_TAG_ATTR_T tag_ctl = AIR_PORT_EGS_TAG_ATTR_UNTAGGED;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    vid     = _strtoul(argv[0], NULL, 0);
    port    = _strtoul(argv[1], NULL, 0);
    tag_ctl = _strtoul(argv[2], NULL, 0);
    rc = air_vlan_setPortEgsTagCtl(0, vid, port, tag_ctl);
    switch (rc)
    {
        case     AIR_E_OK:                                                               break;
        case     AIR_E_ENTRY_NOT_FOUND:  AIR_PRINT("VLAN not found!\n");                 break;
        default:                         AIR_PRINT("Error %d: Operation failed!\n", rc); break;
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetPortActFrame(UI32_T argc, C8_T *argv[])
{
    UI16_T port = 0;
    AIR_VLAN_ACCEPT_FRAME_TYPE_T type = AIR_VLAN_ACCEPT_FRAME_TYPE_ALL;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    type = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setPortAcceptFrameType(0, port, type);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetLeakyVlanEn(UI32_T argc, C8_T *argv[])
{
    UI16_T port = 0;
    AIR_LEAKY_PKT_TYPE_T pkt_type = AIR_LEAKY_PKT_TYPE_UNICAST;
    BOOL_T enable = TRUE;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port     = _strtoul(argv[0], NULL, 0);
    pkt_type = _strtoul(argv[1], NULL, 0);
    enable   = _strtoul(argv[2], NULL, 0);
    rc = air_vlan_setPortLeakyVlanEnable(0, port, pkt_type, enable);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetPortVlanAttr(UI32_T argc, C8_T *argv[])
{
    UI16_T port = 0;
    AIR_VLAN_PORT_ATTR_T attr = AIR_VLAN_PORT_ATTR_USER_PORT;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    attr = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setPortAttr(0, port, attr);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetIgsPortETagAttr(UI32_T argc, C8_T *argv[])
{
    UI16_T port = 0;
    AIR_IGR_PORT_EG_TAG_ATTR_T attr = AIR_IGR_PORT_EG_TAG_ATTR_DISABLE;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    attr = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setIgrPortTagAttr(0, port, attr);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetPortETagAttr(UI32_T argc, C8_T *argv[])
{
    UI16_T port = 0;
    AIR_PORT_EGS_TAG_ATTR_T attr = AIR_PORT_EGS_TAG_ATTR_UNTAGGED;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    attr = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setPortEgsTagAttr(0, port, attr);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetPortOuterTPID(UI32_T argc, C8_T *argv[])
{
    UI16_T port = 0, tpid = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    tpid = _strtoul(argv[1], NULL, 16);
    rc = air_vlan_setPortOuterTPID(0, port, tpid);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSetPvid(UI32_T argc, C8_T *argv[])
{
    UI16_T port = 0, pvid = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    pvid = _strtoul(argv[1], NULL, 0);
    rc = air_vlan_setPortPVID(0, port, pvid);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doVlanSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(vlanSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doVlanGetPortActFrame(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    AIR_VLAN_ACCEPT_FRAME_TYPE_T type;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    rc = air_vlan_getPortAcceptFrameType(0, port, &type);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
        return rc;
    }
    AIR_PRINT("Port %d Acceptable Frame Type: ", port);
    switch(type)
    {
        case AIR_VLAN_ACCEPT_FRAME_TYPE_ALL:
            AIR_PRINT("all(%d)\n", type);
            break;
        case AIR_VLAN_ACCEPT_FRAME_TYPE_TAG_ONLY:
            AIR_PRINT("tagged-only(%d)\n", type);
            break;
        case AIR_VLAN_ACCEPT_FRAME_TYPE_UNTAG_ONLY:
            AIR_PRINT("untagged-only(%d)\n", type);
            break;
        default:
            AIR_PRINT("unknown(%d)\n", type);
            break;
    };
    return rc;
}

static AIR_ERROR_NO_T
doVlanGetLeakyVlanEn(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    BOOL_T uc = FALSE, mc = FALSE, bc = FALSE;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    rc += air_vlan_getPortLeakyVlanEnable(0, port, AIR_LEAKY_PKT_TYPE_UNICAST, &uc);
    rc += air_vlan_getPortLeakyVlanEnable(0, port, AIR_LEAKY_PKT_TYPE_MULTICAST, &mc);
    rc += air_vlan_getPortLeakyVlanEnable(0, port, AIR_LEAKY_PKT_TYPE_BROADCAST, &bc);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
        return rc;
    }

    AIR_PRINT("Port %d Leaky Vlan Enable\n", port);
    AIR_PRINT("Unicast     : %s\n", uc ? "TRUE" : "FALSE");
    AIR_PRINT("Multicast   : %s\n", mc ? "TRUE" : "FALSE");
    AIR_PRINT("Broadcast   : %s\n", bc ? "TRUE" : "FALSE");
    return rc;
}

static AIR_ERROR_NO_T
doVlanGetPortVlanAttr(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    AIR_VLAN_PORT_ATTR_T attr;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    rc = air_vlan_getPortAttr(0, port, &attr);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
        return rc;
    }
    AIR_PRINT("Port %d Vlan Attr: ", port);
    switch(attr)
    {
        case AIR_VLAN_PORT_ATTR_USER_PORT:
            AIR_PRINT("user port(%d)\n", attr);
            break;
        case AIR_VLAN_PORT_ATTR_STACK_PORT:
            AIR_PRINT("stack port(%d)\n", attr);
            break;
        case AIR_VLAN_PORT_ATTR_TRANSLATION_PORT:
            AIR_PRINT("translation port(%d)\n", attr);
            break;
        case AIR_VLAN_PORT_ATTR_TRANSPARENT_PORT:
            AIR_PRINT("transparent port(%d)\n", attr);
            break;
        default:
            AIR_PRINT("unknown(%d)\n", attr);
            break;
    };
    return rc;
}

static AIR_ERROR_NO_T
doVlanGetIgsPortETagAttr(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    AIR_IGR_PORT_EG_TAG_ATTR_T attr;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    rc = air_vlan_getIgrPortTagAttr(0, port, &attr);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
        return rc;
    }
    AIR_PRINT("Port %d Incomming Port Egress Tag Attr: ", port);
    switch(attr)
    {
        case AIR_IGR_PORT_EG_TAG_ATTR_DISABLE:
            AIR_PRINT("disable(%d)\n", attr);
            break;
        case AIR_IGR_PORT_EG_TAG_ATTR_CONSISTENT:
            AIR_PRINT("consistent(%d)\n", attr);
            break;
        case AIR_IGR_PORT_EG_TAG_ATTR_UNTAGGED:
            AIR_PRINT("untagged(%d)\n", attr);
            break;
        case AIR_IGR_PORT_EG_TAG_ATTR_SWAP:
            AIR_PRINT("swap(%d)\n", attr);
            break;
        case AIR_IGR_PORT_EG_TAG_ATTR_TAGGED:
            AIR_PRINT("tagged(%d)\n", attr);
            break;
        case AIR_IGR_PORT_EG_TAG_ATTR_STACK:
            AIR_PRINT("stack(%d)\n", attr);
            break;
        default:
            AIR_PRINT("unknown(%d)\n", attr);
            break;
    };
    return rc;
}

static AIR_ERROR_NO_T
doVlanGetPortETagAttr(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    AIR_PORT_EGS_TAG_ATTR_T attr;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    rc = air_vlan_getPortEgsTagAttr(0, port, &attr);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
        return rc;
    }
    AIR_PRINT("Port %d Egress Tag Attr: ", port);
    switch(attr)
    {
        case AIR_PORT_EGS_TAG_ATTR_UNTAGGED:
            AIR_PRINT("untagged(%d)\n", attr);
            break;
        case AIR_PORT_EGS_TAG_ATTR_SWAP:
            AIR_PRINT("swap(%d)\n", attr);
            break;
        case AIR_PORT_EGS_TAG_ATTR_TAGGED:
            AIR_PRINT("tagged(%d)\n", attr);
            break;
        case AIR_PORT_EGS_TAG_ATTR_STACK:
            AIR_PRINT("stack(%d)\n", attr);
            break;
        default:
            AIR_PRINT("unknown(%d)\n", attr);
            break;
    };
    return rc;
}

static AIR_ERROR_NO_T
doVlanGetPortOuterTPID(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    UI16_T tpid = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    rc = air_vlan_getPortOuterTPID(0, port, &tpid);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
        return rc;
    }
    AIR_PRINT("Port %d Outer TPID: %4x\n", port, tpid);
    return rc;
}

static AIR_ERROR_NO_T
doVlanGetPvid(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    UI16_T pvid = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    rc = air_vlan_getPortPVID(0, port, &pvid);
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("Error: Operation failed!\n");
        return rc;
    }
    AIR_PRINT("Port %d PVID: %d\n", port, pvid);
    return rc;
}

static AIR_ERROR_NO_T
doVlanGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(vlanGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doVlan(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(vlanCmds, argc, argv);
}

static AIR_ERROR_NO_T
doJumbo(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    I32_T pkt_len = 0, frame_len = 0;

    if(0 == argc)
    {
        /* get command */
        ret = air_port_getJumbo(0, &pkt_len, &frame_len);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get ");
            switch(pkt_len)
            {
                case 0:
                    AIR_PRINT("RX_1518 ");
                    break;
                case 1:
                    AIR_PRINT("RX_1536 ");
                    break;
                case 2:
                    AIR_PRINT("RX_1552 ");
                    break;
                case 3:
                    AIR_PRINT("RX_JUMBO ");
                    break;
            }
            AIR_PRINT("frames lengths %d KBytes\n", frame_len);
        }
        else
        {
            AIR_PRINT("Get Jumbo Fail.\n");
        }
    }
    else
    {
        /* set command */
        pkt_len = _strtol(argv[0], NULL, 10);
        frame_len = _strtol(argv[1], NULL, 10);

        ret = air_port_setJumbo(0, pkt_len, frame_len);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set ");
            switch(pkt_len)
            {
                case 0:
                    AIR_PRINT("RX_1518 ");
                    break;
                case 1:
                    AIR_PRINT("RX_1536 ");
                    break;
                case 2:
                    AIR_PRINT("RX_1552 ");
                    break;
                case 3:
                    AIR_PRINT("RX_JUMBO ");
                    break;
            }
            AIR_PRINT("frames lengths %d KBytes\n", frame_len);
        }
        else
            AIR_PRINT("Set Jumbo Fail.\n");
    }

    return ret;
}

static AIR_ERROR_NO_T
doFlowCtrl(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    BOOL_T fc_en = 0, dir = 0;
    I32_T port = 0;

    port = _strtol(argv[0], NULL, 10);
    dir = _strtol(argv[1], NULL, 10);

    if(2 == argc)
    {
        /* get command */
        ret = air_port_getFlowCtrl(0, port, dir, &fc_en);
        if(ret == AIR_E_OK)
            AIR_PRINT("Get Port%02d %s Flow Control %s\n", port, ((dir)?"RX":"TX"), ((fc_en)?"Enable":"Disable"));
        else
            AIR_PRINT("Get Flow Control Fail.\n");
    }
    else
    {
        /* set command */
        fc_en = _strtol(argv[2], NULL, 10);

        ret = air_port_setFlowCtrl(0, port, dir, fc_en);
        if(ret == AIR_E_OK)
            AIR_PRINT("Set Port%02d %s Flow Control %s\n", port, ((dir)?"RX":"TX"), ((fc_en)?"Enable":"Disable"));
        else
            AIR_PRINT("Set Flow Control Fail.\n");
    }

    return ret;
}

static AIR_ERROR_NO_T
doL2Set(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(l2SetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doL2Get(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(l2GetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doL2Clear(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(l2ClearCmds, argc, argv);
}

static AIR_ERROR_NO_T
doL2Del(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(l2DelCmds, argc, argv);
}

static AIR_ERROR_NO_T
doL2Add(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(l2AddCmds, argc, argv);
}

static AIR_ERROR_NO_T
doL2(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(l2Cmds, argc, argv);
}

static AIR_ERROR_NO_T
doAnMode(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    BOOL_T en = 0;

    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get anCap <port> */
        ret = air_port_getAnMode(0, port, &en);
        if(ret == AIR_E_OK)
            AIR_PRINT("Get Port%02d Auto-Negotiation %s\n", port, ((en)?"Enabled":"Disabled"));
        else
            AIR_PRINT("Get Port%02d Auto-Negotiation Fail.\n", port);
    }
    else if(2 == argc)
    {
        /* "port set anMode <port> <en> */
        en = _strtol(argv[1], NULL, 10);
        ret = air_port_setAnMode(0, port, en);
        if(ret == AIR_E_OK)
            AIR_PRINT("Set Port%02d Auto-Negotiation Mode:%s\n", port, ((en)?"Enabled":"Disabled"));
        else
            AIR_PRINT("Set Port%02d Auto-Negotiation Fail.\n", port);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doLocalAdv(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    AIR_AN_ADV_T adv;

    memset(&adv, 0, sizeof(AIR_AN_ADV_T));
    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get localAdv <port> */
        ret = air_port_getLocalAdvAbility(0, port, &adv);
        AIR_PRINT("Get Port%02d Local Auto-Negotiation Advertisement: ", port);
        if(AIR_E_OK != ret)
        {
            AIR_PRINT("Fail!\n");
        }
    }
    else if(7 == argc)
    {
        /* port set localAdv <port> <10H> <10F> <100H> <100F> <1000F> <pause> */
        adv.advCap10HDX = _strtol(argv[1], NULL, 0) & BIT(0);
        adv.advCap10FDX = _strtol(argv[2], NULL, 0) & BIT(0);
        adv.advCap100HDX = _strtol(argv[3], NULL, 0) & BIT(0);
        adv.advCap100FDX = _strtol(argv[4], NULL, 0) & BIT(0);
        adv.advCap1000FDX = _strtol(argv[5], NULL, 0) & BIT(0);
        adv.advPause = _strtol(argv[6], NULL, 0) & BIT(0);
        ret = air_port_setLocalAdvAbility(0, port, adv);
        AIR_PRINT("Set Port%02d Local Auto-Negotiation Advertisement: ", port);
        if(AIR_E_OK != ret)
        {
            AIR_PRINT("Fail!\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        AIR_PRINT("\n");
        AIR_PRINT("\tAdvertise 10BASE-T Half Duplex: %s\n", (adv.advCap10HDX)?"Effective":"Not Effective" );
        AIR_PRINT("\tAdvertise 10BASE-T Full Duplex: %s\n", (adv.advCap10FDX)?"Effective":"Not Effective" );
        AIR_PRINT("\tAdvertise 100BASE-T Half Duplex: %s\n", (adv.advCap100HDX)?"Effective":"Not Effective" );
        AIR_PRINT("\tAdvertise 100BASE-T Full Duplex: %s\n", (adv.advCap100FDX)?"Effective":"Not Effective" );
        AIR_PRINT("\tAdvertise 1000BASE-T Full Duplex: %s\n", (adv.advCap1000FDX)?"Effective":"Not Effective" );
        AIR_PRINT("\tAdvertise Asynchronous Pause: %s\n", (adv.advPause)?"Effective":"Not Effective" );
    }

    return ret;
}

static AIR_ERROR_NO_T
doRemoteAdv(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    AIR_AN_ADV_T lp_adv;

    memset(&lp_adv, 0, sizeof(AIR_AN_ADV_T));
    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get remoteAdv <port> */
        ret = air_port_getRemoteAdvAbility(0, port, &lp_adv);
        AIR_PRINT("Get Port%02d Remote Auto-Negotiation Advertisement: ", port);
        if(AIR_E_OK != ret)
        {
            AIR_PRINT("Fail!\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        AIR_PRINT("\n");
        AIR_PRINT("\tAdvertise 10BASE-T Half Duplex: %s\n", lp_adv.advCap10HDX?"Effective":"Not Effective" );
        AIR_PRINT("\tAdvertise 10BASE-T Full Duplex: %s\n", lp_adv.advCap10FDX?"Effective":"Not Effective" );
        AIR_PRINT("\tAdvertise 100BASE-T Half Duplex: %s\n", lp_adv.advCap100HDX?"Effective":"Not Effective" );
        AIR_PRINT("\tAdvertise 100BASE-T Full Duplex: %s\n", lp_adv.advCap100FDX?"Effective":"Not Effective" );
        AIR_PRINT("\tAdvertise 1000BASE-T Full Duplex: %s\n", (lp_adv.advCap1000FDX)?"Effective":"Not Effective" );
        AIR_PRINT("\tAdvertise Asynchronous Pause: %s\n", (lp_adv.advPause)?"Effective":"Not Effective" );
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortSpeed(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    UI32_T speed = 0;

    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get speed <port> */
        ret = air_port_getSpeed(0, port, &speed);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get Port%02d Speed:", port);
        }
        else
        {
            AIR_PRINT("Get Port%02d Speed Fail!\n", port);
        }
    }
    else if(2 == argc)
    {
        /* port set speed <port> <speed> */
        speed = _strtol(argv[1], NULL, 10);
        ret = air_port_setSpeed(0, port, speed);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set Port%02d Speed:", port);
        }
        else
        {
            AIR_PRINT("Set Port%02d Speed Fail!\n", port);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        switch(speed)
        {
            case AIR_PORT_SPEED_10M:
                AIR_PRINT(" 10 Mbps\n");
                break;
            case AIR_PORT_SPEED_100M:
                AIR_PRINT(" 100 Mbps\n");
                break;
            case AIR_PORT_SPEED_1000M:
                AIR_PRINT(" 1 Gbps\n");
                break;
            case AIR_PORT_SPEED_2500M:
                AIR_PRINT(" 2.5 Gbps\n");
                break;
            default:
                AIR_PRINT(" Reserved\n");
                break;
        }
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortDuplex(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    UI32_T duplex = 0;

    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get duplex <port> */
        ret = air_port_getDuplex(0, port, &duplex);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get Port%02d Duplex:%s\n", port, duplex?"Full":"Half");
        }
        else
        {
            AIR_PRINT("Get Port%02d Duplex Fail!\n", port);
        }
    }
    else if(2 == argc)
    {
        /* port set duplex <port> <duplex> */
        duplex = _strtol(argv[1], NULL, 10);
        ret = air_port_setDuplex(0, port, duplex);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set Port%02d Duplex:%s\n", port, duplex?"Full":"Half");
        }
        else
        {
            AIR_PRINT("Set Port%02d Duplex Fail!\n", port);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortStatus(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    AIR_PORT_STATUS_T ps;

    memset(&ps, 0, sizeof(AIR_PORT_STATUS_T));
    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get status <port> */
        ret = air_port_getLink(0, port, &ps);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get Port%02d Link-Status\n", port);
            AIR_PRINT("\tLink: %s\n", ps.link?"Up":"Down");
            AIR_PRINT("\tDuplex: %s\n", ps.duplex?"Full":"Half");
            AIR_PRINT("\tSpeed: ");
            switch(ps.speed)
            {
                case AIR_PORT_SPEED_10M:
                    AIR_PRINT("10 Mbps\n");
                    break;
                case AIR_PORT_SPEED_100M:
                    AIR_PRINT("100 Mbps\n");
                    break;
                case AIR_PORT_SPEED_1000M:
                    AIR_PRINT("1 Gbps\n");
                    break;
                case AIR_PORT_SPEED_2500M:
                    AIR_PRINT("2.5 Gbps\n");
                    break;
                default:
                    AIR_PRINT("Reserved\n");
                    break;
            }
        }
        else
            AIR_PRINT("Get Port%02d Link-Status Fail!", port);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortBckPres(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    UI32_T bckPres = 0;

    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get bckPres <port> */
        ret = air_port_getBckPres(0, port, &bckPres);
        if(ret == AIR_E_OK)
            AIR_PRINT("Get Port%02d BckPres:%s\n", port, bckPres?"Enabled":"Disabled");
        else
            AIR_PRINT("Get Port%02d BckPres Fail!\n", port);
    }
    else if(2 == argc)
    {
        /* port set bckPres <port> <bckPres> */
        bckPres = _strtol(argv[1], NULL, 10);
        ret = air_port_setBckPres(0, port, bckPres);
        if(ret == AIR_E_OK)
            AIR_PRINT("Set Port%02d BckPres:%s\n", port, bckPres?"Enabled":"Disabled");
        else
            AIR_PRINT("Set Port%02d BckPres Fail!\n", port);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortPsMode(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    UI32_T mode = 0;
    BOOL_T ls_en = 0;
    BOOL_T eee_en = 0;

    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get psMode <port> */
        ret = air_port_getPsMode(0, port, &mode);
        AIR_PRINT("Get Port%02d Power-Saving: ", port);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Done\n");
        }
        else
        {
            AIR_PRINT("Fail!\n");
        }
    }
    else if(3 == argc)
    {
        /* port set psMode <port> <ls> <eee> */
        ls_en = _strtol(argv[1], NULL, 0);
        eee_en = _strtol(argv[2], NULL, 0);
        if(TRUE == ls_en)
        {
            mode |= AIR_PORT_PS_LINKSTATUS;
        }
        if(TRUE == eee_en)
        {
            mode |= AIR_PORT_PS_EEE;
        }
        ret = air_port_setPsMode(0, port, mode);
        AIR_PRINT("Set Port%02d Power-Saving: ", port);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Done\n");
        }
        else
        {
            AIR_PRINT("Fail!\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }
    if(ret == AIR_E_OK)
    {
        AIR_PRINT("\tLink status:%s\n", (mode & AIR_PORT_PS_LINKSTATUS)?"Enable":"Disable");
        AIR_PRINT("\tEEE:%s\n", (mode & AIR_PORT_PS_EEE)?"Enable":"Disable");
    }
    return ret;
}

static AIR_ERROR_NO_T
doPortSmtSpdDwn(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    UI32_T state = 0;
    UI32_T retry = 0;

    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get smtSpdDwn <port> */
        ret = air_port_getSmtSpdDwn(0, port, &state, &retry);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get Port%02d Smart Speed Down: %s\n", port, state?"Enabled":"Disabled");
            AIR_PRINT("Get Port%02d Retry Time: %d\n", port, retry + 2);
        }
        else
            AIR_PRINT("Get Port%02d Smart-SpeedDown Fail!\n", port);
    }
    else if(3 == argc)
    {
        /* port set smtSpdDwn <port> <en> <retry> */
        state = _strtol(argv[1], NULL, 10);
        retry = _strtol(argv[2], NULL, 10);
        if(retry >= 2)
        {
            ret = air_port_setSmtSpdDwn(0, port, state, retry - 2);
            if(ret == AIR_E_OK)
            {
                AIR_PRINT("Set Port%02d Smart Speed Down: %s\n", port, state?"Enabled":"Disabled");
                AIR_PRINT("Set Port%02d Retry Time: %d\n", port, retry);
            }
            else
                AIR_PRINT("Set Port%02d Smart-SpeedDown Fail!\n", port);
        }
        else
        {
            ret = AIR_E_BAD_PARAMETER;
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortSpTag(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    BOOL_T sptag_en = FALSE;

    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get spTag <port> */
        ret = air_port_getSpTag(0, port, &sptag_en);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Get Port%02d Special Tag %s\n", port, ((sptag_en)?"Enabled":"Disabled"));
        }
        else
        {
            AIR_PRINT("Get Port%02d Special Tag Fail.\n", port);
        }
    }
    else if(2 == argc)
    {
        /* port set spTag <port> <en> */
        sptag_en = _strtol(argv[1], NULL, 10);
        ret = air_port_setSpTag(0, port, sptag_en);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Set Port%02d Special Tag:%s\n", port, ((sptag_en)?"Enabled":"Disabled"));
        }
        else
        {
            AIR_PRINT("Set Port%02d Special Tag Fail.\n", port);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortEnable(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    UI32_T state = 0;

    port = _strtol(argv[0], NULL, 10);

    if(1 == argc)
    {
        /* port get enable <port> */
        ret = air_port_getEnable(0, port, &state);
        if(ret == AIR_E_OK)
            AIR_PRINT("Get Port%02d State:%s\n", port, state?"Enable":"Disable");
        else
            AIR_PRINT("Get Port%02d State Fail!\n", port);
    }
    else if(2 == argc)
    {
        /* port set enable <port> <en> */
        state = _strtol(argv[1], NULL, 10);
        ret = air_port_setEnable(0, port, state);
        if(ret == AIR_E_OK)
            AIR_PRINT("Set Port%02d State:%s\n", port, state?"Enable":"Disable");
        else
            AIR_PRINT("Set Port%02d State Fail!\n", port);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doPort5GBaseRMode(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    if(0 == argc)
    {
        /* port set 5GBaseRMode */
        ret = air_port_set5GBaseRModeEn(0);
        if(ret == AIR_E_OK)
            AIR_PRINT("Set Port05 Mode: 5GBase-R\n");
        else
            AIR_PRINT("Set Port05 HSGMII Mode Fail.\n");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortHsgmiiMode(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    if(0 == argc)
    {
        /* port set hsgmiiMode */
        ret = air_port_setHsgmiiModeEn(0);
        if(ret == AIR_E_OK)
            AIR_PRINT("Set Port05 Mode: HSGMII\n");
        else
            AIR_PRINT("Set Port05 HSGMII Mode Fail.\n");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortSgmiiMode(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T mode = 0;
    UI32_T speed = 0;

    if(2 == argc)
    {
        /* port set sgmiiMode <mode(0:AN,1:Force)> <speed> */
        mode = _strtol(argv[0], NULL, 10);
        speed = _strtol(argv[1], NULL, 10);
        ret = air_port_setSgmiiMode(0, mode, speed);
        if(ret == AIR_E_OK)
            AIR_PRINT("Set Port05 SGMII Mode:%s\nIf in Force Mode, speed:", mode?"Force":"AN");
        else
            AIR_PRINT("Set Port05 SGMII Mode Fail.\n");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        switch(speed)
        {
            case AIR_PORT_SPEED_10M:
                AIR_PRINT(" 10 Mbps\n");
                break;
            case AIR_PORT_SPEED_100M:
                AIR_PRINT(" 100 Mbps\n");
                break;
            case AIR_PORT_SPEED_1000M:
                AIR_PRINT(" 1 Gbps\n");
                break;
            default:
                AIR_PRINT(" Reserved\n");
                break;
        }
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortRmiiMode(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T speed = 0;

    if(1 == argc)
    {
        /* port set rmiiMode <speed> */
        speed = _strtol(argv[0], NULL, 10);
        ret = air_port_setRmiiMode(0, speed);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set Port05 RMII Mode Speed:");
        }
        else
        {
            AIR_PRINT("Set Port05 RMII Mode Fail!\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        switch(speed)
        {
            case AIR_PORT_SPEED_10M:
                AIR_PRINT(" 10 Mbps\n");
                break;
            case AIR_PORT_SPEED_100M:
                AIR_PRINT(" 100 Mbps\n");
                break;
            default:
                AIR_PRINT(" Reserved\n");
                break;
        }
    }

    return ret;
}

static AIR_ERROR_NO_T
doPortRgmiiMode(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T speed = 0;

    if(1 == argc)
    {
        /* port set rgmiiMode <speed> */
        speed = _strtol(argv[0], NULL, 10);
        ret = air_port_setRgmiiMode(0, speed);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set Port05 RGMII Mode Speed:");
        }
        else
        {
            AIR_PRINT("Set Port05 RGMII Mode Fail!\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        switch(speed)
        {
            case AIR_PORT_SPEED_10M:
                AIR_PRINT(" 10 Mbps\n");
                break;
            case AIR_PORT_SPEED_100M:
                AIR_PRINT(" 100 Mbps\n");
                break;
            case AIR_PORT_SPEED_1000M:
                AIR_PRINT(" 1 Gbps\n");
                break;
            default:
                AIR_PRINT(" Reserved\n");
                break;
        }
    }

    return ret;
}

static AIR_ERROR_NO_T
doSptagEn(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    BOOL_T sp_en = FALSE;

    port =  _strtol(argv[0], NULL, 10);
    if (2 == argc)
    {
        sp_en =  _strtol(argv[1], NULL, 10);
        ret = air_sptag_setState(0,port,sp_en);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("set port %d SpTag state %s sucess\n", port,sp_en?"Enable":"Disable");
        }
        else
        {
            AIR_PRINT("set port %d SpTag state %s fail\n", port,sp_en?"Enable":"Disable");
        }
    }
    else if(1 == argc)
    {
        air_sptag_getState(0,port,&sp_en);
        AIR_PRINT("get port %d SpTag state: %s \n", port,sp_en?"Enable":"Disable");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doSptagMode(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    BOOL_T sp_mode = FALSE;


    port =  _strtol(argv[0], NULL, 10);
    if (2 == argc)
    {
        sp_mode  =  _strtol(argv[1], NULL, 10);
        ret = air_sptag_setMode(0,port,sp_mode);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("set port %d SpTag Mode  %s sucess\n", port,sp_mode?"replace":"insert");
        }
        else
        {
            AIR_PRINT("set port %d SpTag state %s fail\n", port,sp_mode?"replace":"insert");
        }
    }
    else if(1 == argc)
    {
        air_sptag_getMode(0,port,&sp_mode);
        AIR_PRINT("get port %d SpTag state: %s \n", port,sp_mode?"replace":"insert");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doSptagDecode(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_SPTAG_RX_PARA_T sptag_rx = {0};
    UI8_T buf[AIR_STAG_BUF_LEN] = {0};
    UI32_T len = AIR_STAG_BUF_LEN, i = 0;

    if (4 == argc)
    {
        for(i = 0; i < len; i++)
        {
            buf[i] = _strtoul(argv[i], NULL, 16);
        }

        ret = air_sptag_decodeRx(0, buf, len, &sptag_rx);
        if (AIR_E_OK != ret)
        {
            AIR_PRINT("SpTag decode fail\n");
            return ret;
        }

        AIR_PRINT("SpTag decode success:\n");
        AIR_PRINT("RSN : %s\n", _sptag_pt[sptag_rx.rsn]);
        AIR_PRINT("VPM : %s\n", _sptag_vpm[sptag_rx.vpm]);
        AIR_PRINT("SPN : %d\n", sptag_rx.spn);
        AIR_PRINT("PRI : %d\n", sptag_rx.pri);
        AIR_PRINT("CFI : %d\n", sptag_rx.cfi);
        AIR_PRINT("VID : %d\n", sptag_rx.vid);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doSptagEncode(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_STAG_TX_PARA_T sptag_tx = {0};
    C8_T *token = NULL;
    UI8_T buf[AIR_STAG_BUF_LEN] = {0};
    UI32_T len = AIR_STAG_BUF_LEN;
    char str[128] = {'\0'};
    UI32_T data = 0;
    AIR_STAG_MODE_T mode = AIR_STAG_MODE_LAST;

    if (7 == argc)
    {
        if(_strcmp(argv[0],"mode=replace") == 0)
            mode = AIR_STAG_MODE_REPLACE;
        else if(_strcmp(argv[0],"mode=insert") == 0)
            mode = AIR_STAG_MODE_INSERT;
        else
            printf("mode is wrong!!");

        if(_strcmp(argv[1],"opc=portmap") == 0)
            sptag_tx.opc = AIR_STAG_OPC_PORTMAP;
        else if(_strcmp(argv[1],"opc=portid") == 0)
            sptag_tx.opc = AIR_STAG_OPC_PORTID;
        else if(_strcmp(argv[1],"opc=lookup") == 0)
            sptag_tx.opc = AIR_STAG_OPC_LOOKUP;
        else
            printf("opc is wrong!!");

        token = _strtok(argv[2], "=", &argv[2]);
        if(_strcmp(token,"dp") != 0) {
            AIR_PRINT("Bad parameter\r\n");
        } else {
            if ((token = _strtok(argv[2], "=", &argv[2]))) {
                data = _strtoul(token, NULL, 16);
                sptag_tx.pbm = data;
                AIR_PRINT("sptag_tx.pbm %x\n",sptag_tx.pbm);
            }
        }

        if(_strcmp(argv[3],"vpm=untagged") == 0)
            sptag_tx.vpm = AIR_STAG_VPM_UNTAG;
        else if(_strcmp(argv[3],"vpm=8100") == 0)
            sptag_tx.vpm = AIR_STAG_VPM_TPID_8100;
        else if(_strcmp(argv[3],"vpm=88a8") == 0)
            sptag_tx.vpm = AIR_STAG_VPM_TPID_88A8;
        else
            printf("vpm is wrong!!");

        token = _strtok(argv[4], "=", &argv[4]);
        if(_strcmp(token, "pri") != 0) {
            AIR_PRINT("Bad parameter\r\n");
        } else {
            if ((token = _strtok(argv[4], "=", &argv[4]))) {
                data = _strtoul(token, NULL, 0);
                sptag_tx.pri = data;
                AIR_PRINT("sptag_tx.pri %d\n",sptag_tx.pri);
            }
        }

        token = _strtok(argv[5], "=", &argv[5]);
        if(_strcmp(token, "cfi") != 0) {
            AIR_PRINT("Bad parameter\r\n");
        } else {
            if ((token = _strtok(argv[5], "=", &argv[5]))) {
                data = _strtoul(token, NULL, 0);
                sptag_tx.cfi  = data;
                AIR_PRINT("sptag_tx.cfi %d\n",sptag_tx.cfi);
            }
        }

        token = _strtok(argv[6], "=", &argv[6]);
        if(_strcmp(token, "vid") != 0) {
            AIR_PRINT("Bad parameter\r\n");
        } else {
            if ((token = _strtok(argv[6], "=", &argv[6]))) {
                data = _strtoul(token, NULL, 0);
                sptag_tx.vid = data;
                AIR_PRINT("sptag_tx.vid %d\n",sptag_tx.vid);
            }
        }

        ret = air_sptag_encodeTx(0,mode, &sptag_tx, (UI8_T *)&buf, &len);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("SpTag encode sucess, returned len=%d\n", len);
            AIR_PRINT("Encoded SpTag: %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
        }
        else
        {
            AIR_PRINT("SpTag encode fail\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doSptag(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(sptagCmds, argc, argv);
}

static AIR_ERROR_NO_T
doL2Dump(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(l2DumpCmds, argc, argv);
}

static AIR_ERROR_NO_T
doMacAddr(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_MAC_ENTRY_T mt;
    UI32_T fwd = 0;

    memset(&mt, 0, sizeof(AIR_MAC_ENTRY_T));

    if(0 == argc)
    {
        /* l2 clear mac */
        ret = air_l2_clearMacAddr(0);
        if(ret == AIR_E_OK)
            AIR_PRINT("Clear MAC Address Table Done.\n");
        else
            AIR_PRINT("Clear MAC Address Table Fail.\n");
    }
    else if(3 == argc)
    {
        /* l2 del mac <mac(12'hex)> { vid <vid(0..4095)> | fid <fid(0..15)> } */
        ret = _str2mac(argv[0], (C8_T *)mt.mac);
        if(ret != AIR_E_OK)
        {
            AIR_PRINT("Unrecognized command.\n");
            return ret;
        }

        /* check argument 1 */
        if(FALSE == _strcmp(argv[1], "vid"))
        {
            /* get mac entry by MAC address & vid */
            mt.cvid = _strtoul(argv[2], NULL, 0);
            mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
            AIR_PRINT("Get MAC Address:" MAC_STR " with vid:%u", MAC2STR(mt.mac), mt.cvid);
        }
        else if(FALSE == _strcmp(argv[1], "fid"))
        {
            /* get mac entry by MAC address & fid */
            mt.fid = _strtoul(argv[2], NULL, 0);
            AIR_PRINT("Get MAC Address:" MAC_STR " with fid:%u", MAC2STR(mt.mac), mt.fid);
        }
        else
        {
            AIR_PRINT("Unrecognized command.\n");
            return AIR_E_BAD_PARAMETER;
        }
        ret = air_l2_delMacAddr(0, &mt);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT(" Done.\n");
        }
        else
            AIR_PRINT("\n Fail!\n");
    }
    else if(7 == argc)
    {
        /* l2 add mac <static(0:dynamic,1:static)> <unauth(0:auth,1:unauth)> <mac(12'hex)> <portlist(uintlist)> [ vid <vid(0..4095)> | fid <fid(0..15)> ] <src_mac_forward=(0:default,1:cpu-include,2:cpu-exclude,3:cpu-only,4:drop)> */
        if(argv[0])
            mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_STATIC;

        if(argv[1])
            mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_UNAUTH;

        ret = _str2mac(argv[2], (C8_T *)mt.mac);
        if(ret != AIR_E_OK)
        {
            AIR_PRINT("Unrecognized command.\n");
            return ret;
        }

        ret = _portListStr2Ary(argv[3], mt.port_bitmap, 1);
        if(ret != AIR_E_OK)
        {
            AIR_PRINT("Unrecognized command.\n");
            return ret;
        }

        /* check argument fid or vid */
        if(FALSE == _strcmp(argv[4], "vid"))
        {
            /* get mac entry by MAC address & vid */
            mt.cvid = _strtoul(argv[5], NULL, 0);
            mt.flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
        }
        else if(FALSE == _strcmp(argv[4], "fid"))
        {
            /* get mac entry by MAC address & fid */
            mt.fid = _strtoul(argv[5], NULL, 0);
        }
        else
        {
            AIR_PRINT("Unrecognized command.\n");
            return AIR_E_BAD_PARAMETER;
        }
        fwd = _strtoul(argv[6], NULL, 0);
        if(0 == fwd)
            mt.sa_fwd = AIR_L2_FWD_CTRL_DEFAULT;
        else if(1 == fwd)
            mt.sa_fwd = AIR_L2_FWD_CTRL_CPU_INCLUDE;
        else if(2 == fwd)
            mt.sa_fwd = AIR_L2_FWD_CTRL_CPU_EXCLUDE;
        else if(3 == fwd)
            mt.sa_fwd = AIR_L2_FWD_CTRL_CPU_ONLY;
        else if(4 == fwd)
            mt.sa_fwd = AIR_L2_FWD_CTRL_DROP;
        else
        {
            AIR_PRINT("Unrecognized command.\n");
            return AIR_E_BAD_PARAMETER;
        }
        ret = air_l2_addMacAddr(0, &mt);
        AIR_PRINT("Add MAC Address:" MAC_STR, MAC2STR(mt.mac));
        if(ret == AIR_E_OK)
            AIR_PRINT(" Done.\n");
        else
            AIR_PRINT(" Fail.\n");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
_printMacEntry(
        AIR_MAC_ENTRY_T *mt,
        UI32_T age_unit,
        UI8_T count,
        UI8_T title)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    I32_T i = 0, j = 0;
    UI8_T first = 0;
    UI8_T find = 0;
    if(title)
    {
        AIR_PRINT("%-6s%-15s%-5s%-5s%-5s%-10s%-10s%-6s\n",
                "unit",
                "mac",
                "ivl",
                "vid",
                "fid",
                "age-time",
                "forward",
                "port");
        return ret;
    }
    for(i = 0; i < count; i++)
    {
        AIR_PRINT("%-6d", age_unit);
        AIR_PRINT(MAC_STR, MAC2STR(mt[i].mac));
        AIR_PRINT("...");
        if(mt[i].flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
        {
            AIR_PRINT("%-3s..", "ivl");
            AIR_PRINT("%-5d", mt[i].cvid);
            AIR_PRINT("%-5s", ".....");
        }
        else
        {
            AIR_PRINT("%-3s..", "svl");
            AIR_PRINT("%-5s", ".....");
            AIR_PRINT("%-5d", mt[i].fid);
        }
        if(mt[i].flags & AIR_L2_MAC_ENTRY_FLAGS_STATIC)
        {
            AIR_PRINT("%-10s.", "static");
        }
        else
        {
            AIR_PRINT("%d sec..", mt[i].timer);
        }
        AIR_PRINT("%-10s", _air_mac_address_forward_control_string[mt[i].sa_fwd]);
        first = 0;
        find = 0;
        for (j = (AIR_MAX_NUM_OF_PORTS - 1); j >= 0; j--)
        {
            if((mt[i].port_bitmap[0]) & (1 << j))
            {
                first = j;
                find = 1;
                break;
            }
        }
        if(find)
        {
            for (j = 0; j < AIR_MAX_NUM_OF_PORTS; j++)
            {
                if((mt[i].port_bitmap[0]) & (1 << j))
                {
                    if(j == first)
                        AIR_PRINT("%-2d", j);
                    else
                        AIR_PRINT("%-2d,", j);
                }
            }
        }
        else
            AIR_PRINT("no dst port");
        AIR_PRINT("\n");
    }
    return ret;
}

static AIR_ERROR_NO_T
doGetMacAddr(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI8_T count = 0;
    AIR_MAC_ENTRY_T * ptr_mt;

    if(3 == argc)
    {
        ptr_mt = AIR_MALLOC(sizeof(AIR_MAC_ENTRY_T));
        if (NULL == ptr_mt)
        {
            AIR_PRINT("***Error***, allocate memory fail\n");
            return AIR_E_OTHERS;
        }
        memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T));
        /* l2 get mac <mac(12'hex)> { vid <vid(0..4095)> | fid <fid(0..15)> } */
        ret = _str2mac(argv[0], (C8_T *)ptr_mt->mac);
        if(ret != AIR_E_OK)
        {
            AIR_PRINT("Unrecognized command.\n");
            AIR_FREE(ptr_mt);
            return ret;
        }

        /* check argument 1 */
        if(FALSE == _strcmp(argv[1], "vid"))
        {
            /* get mac entry by MAC address & vid */
            ptr_mt->cvid = _strtoul(argv[2], NULL, 0);
            ptr_mt->flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
            AIR_PRINT("Get MAC Address:" MAC_STR " with vid:%u", MAC2STR(ptr_mt->mac), ptr_mt->cvid);
        }
        else if(FALSE == _strcmp(argv[1], "fid"))
        {
            /* get mac entry by MAC address & fid */
            ptr_mt->fid = _strtoul(argv[2], NULL, 0);
            AIR_PRINT("Get MAC Address:" MAC_STR " with fid:%u", MAC2STR(ptr_mt->mac), ptr_mt->fid);
        }
        else
        {
            AIR_PRINT("Unrecognized command.\n");
            AIR_FREE(ptr_mt);
            return AIR_E_BAD_PARAMETER;
        }
        ret = air_l2_getMacAddr(0, &count, ptr_mt);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT(" Done.\n");
            _printMacEntry(ptr_mt, 0, 1, TRUE);
            _printMacEntry(ptr_mt, 0, 1, FALSE);
        }
        else
            AIR_PRINT("\n Not found!\n");
        AIR_FREE(ptr_mt);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doMacAddrAgeOut(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T time = 0;
    if(0 == argc)
    {
        /* l2 get macAddrAgeOut */
        ret = air_l2_getMacAddrAgeOut(0, &time);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get MAC Address Age Out Time Done.\n");
        }
        else
        {
            AIR_PRINT("Get MAC Address Age Out Time Fail.\n");
        }
    }
    else if(1 == argc)
    {
        /* l2 set macAddrAgeOut <time(1, 1000000)> */
        time = _strtoul(argv[0], NULL, 0);
        if(time < 1 || time > 1000000)
        {
            AIR_PRINT("Unrecognized command.\n");
            return AIR_E_BAD_PARAMETER;
        }
        ret = air_l2_setMacAddrAgeOut(0, time);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set MAC Address Age Out Time Done.\n");
        }
        else
        {
            AIR_PRINT("Set MAC Address Age Out Time Fail.\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        AIR_PRINT("MAC Address Age Out Time: %u seconds.\n", time);
    }

    return ret;
}

static AIR_ERROR_NO_T
doDumpMacAddr(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_MAC_ENTRY_T *ptr_mt;
    UI8_T count = 0;
    UI32_T bucket_size = 0;
    UI32_T total_count = 0;

    if(0 != argc)
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }
    /* get unit of aging time */
    ret = air_l2_getMacBucketSize(0, &bucket_size);
    if(ret != AIR_E_OK)
    {
        AIR_PRINT("Get MAC Age Time Fail!\n");
        return ret;
    }
    ptr_mt = AIR_MALLOC(sizeof(AIR_MAC_ENTRY_T) * bucket_size);
    if (NULL == ptr_mt)
    {
        AIR_PRINT("***Error***, allocate memory fail\n");
        return AIR_E_OTHERS;
    }
    memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T) * bucket_size);
    _printMacEntry(ptr_mt, 0, count, TRUE);
    /* get 1st entry of MAC table */
    ret = air_l2_getMacAddr(0, &count, ptr_mt);
    switch(ret)
    {
        case AIR_E_ENTRY_NOT_FOUND:
            AIR_FREE(ptr_mt);
            AIR_PRINT("Not Found!\n");
            return ret;
        case AIR_E_TIMEOUT:
            AIR_FREE(ptr_mt);
            AIR_PRINT("Time Out!\n");
            return ret;
        case AIR_E_BAD_PARAMETER:
            AIR_FREE(ptr_mt);
            AIR_PRINT("Bad Parameter!\n");
            return ret;
        default:
            break;
    }
    total_count += count;
    _printMacEntry(ptr_mt, 0, count, FALSE);

    /* get other entries of MAC table */
    while(1)
    {
        memset(ptr_mt, 0, sizeof(AIR_MAC_ENTRY_T) * bucket_size);
        ret = air_l2_getNextMacAddr(0, &count, ptr_mt);
        if(AIR_E_OK != ret)
        {
            break;
        }
        total_count += count;
        _printMacEntry(ptr_mt, 0, count, FALSE);
    }
    switch(ret)
    {
        case AIR_E_TIMEOUT:
            AIR_PRINT("Time Out!\n");
            break;
        case AIR_E_BAD_PARAMETER:
            AIR_PRINT("Bad Parameter!\n");
            break;
        default:
            AIR_PRINT("Found %u %s\n", total_count, (total_count>1)?"entries":"entry");
            break;
    }
    AIR_FREE(ptr_mt);
    return ret;
}

static AIR_ERROR_NO_T
doLagMember(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T portrunk_index = 0, member_index = 0, member_enable = 0, port_index = 0, i = 0;
    AIR_LAG_PTGINFO_T  member;
    memset(&member,0,sizeof(AIR_LAG_PTGINFO_T));

    if(4 == argc)
    {
        /* lag set member <port trunk index> <member index> <member enable> <port_index>*/
        portrunk_index  = _strtol(argv[0], NULL, 10);
        member_index    = _strtol(argv[1], NULL, 10);
        member_enable   = _strtol(argv[2], NULL, 10);
        port_index      = _strtol(argv[3], NULL, 10);
        ret = air_lag_setMember(0, portrunk_index, member_index, member_enable,port_index);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set port trunk index %d member_index:%d member_enable:%d, port_index:%d ok.\n", portrunk_index, member_index, member_enable,port_index);
        }
        else
        {
            AIR_PRINT("Set port trunk index %d member_index:%d member_enable:%d, port_index:%d fail.\n", portrunk_index, member_index, member_enable,port_index);
        }
        memset(&member,0,sizeof(member));
        air_lag_getMember(0, portrunk_index, &member);
        if(! member.csr_gp_enable[0])
        {
            AIR_PRINT("\r\n!!!!!!!!!Port trunk index %d member_index:0 must be set,or else have taffic issues.\n", portrunk_index);
        }
    }
    else if(1 == argc)
    {
        portrunk_index = _strtol(argv[0], NULL, 10);

        /* lag get member <port> */
        memset(&member,0,sizeof(member));
        ret = air_lag_getMember(0, portrunk_index, &member);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get port trunk %u member:\n", portrunk_index);
            for(i = 0; i < AIR_LAG_MAX_MEM_NUM; i++)
            {
                if(member.csr_gp_enable[i])
                    AIR_PRINT("port %d \r\n", member.csr_gp_port[i]);
            }
            AIR_PRINT("\r\n");
        }
        else
        {
            AIR_PRINT("Get port trunk:%u Member Fail.\n", portrunk_index);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}


static AIR_ERROR_NO_T
doLagDstInfo(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_LAG_DISTINFO_T dstInfo;

    memset(&dstInfo, 0, sizeof(AIR_LAG_DISTINFO_T));
    if(7 == argc)
    {
        /* lag set dstInfo <sp> <sa> <da> <sip> <dip> <sport> <dport> */
        dstInfo.sp = _strtol(argv[0], NULL, 10) & BIT(0);
        dstInfo.sa = _strtol(argv[1], NULL, 10) & BIT(0);
        dstInfo.da = _strtol(argv[2], NULL, 10) & BIT(0);
        dstInfo.sip = _strtol(argv[3], NULL, 10) & BIT(0);
        dstInfo.dip = _strtol(argv[4], NULL, 10) & BIT(0);
        dstInfo.sport = _strtol(argv[5], NULL, 10) & BIT(0);
        dstInfo.dport = _strtol(argv[6], NULL, 10) & BIT(0);
        ret = air_lag_setDstInfo(0, dstInfo);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set LAG packet distrubution.\n");
        }
        else
        {
            AIR_PRINT("Set LAG packet distrubution Fail.\n");
        }
    }
    else if(0 == argc)
    {
        /* lag get dstInfo */
        ret = air_lag_getDstInfo(0, &dstInfo);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get LAG packet distrubution:\n");
        }
        else
        {
            AIR_PRINT("Get LAG packet distrubution Fail.\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }
    if(ret == AIR_E_OK)
    {
        AIR_PRINT("%-5s|%-5s|%-5s|%-5s|%-5s|%-5s|%-5s\n",
                "SP", "SA", "DA", "SIP", "DIP", "SPORT", "DPORT");
        AIR_PRINT("%-5s|%-5s|%-5s|%-5s|%-5s|%-5s|%-5s\n",
                (dstInfo.sp)?"En":"Dis",
                (dstInfo.sa)?"En":"Dis",
                (dstInfo.da)?"En":"Dis",
                (dstInfo.sip)?"En":"Dis",
                (dstInfo.dip)?"En":"Dis",
                (dstInfo.sport)?"En":"Dis",
                (dstInfo.dport)?"En":"Dis");
    }
    return ret;
}

static AIR_ERROR_NO_T
doLagHashtype(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T hashtype = 0;

    if(1 == argc)
    {
        hashtype = _strtol(argv[0], NULL, 10);
        ret = air_lag_sethashtype(0, hashtype);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set LAG hashtype Ok.\n");
        }
        else
        {
            AIR_PRINT("Set LAG hashtype Fail.\n");
        }
    }
    else if(0 == argc)
    {
        /* lag get dstInfo */
        ret = air_lag_gethashtype(0, &hashtype);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get LAG hashtype:\n");
        }
        else
        {
            AIR_PRINT("Get LLAG hashtype Fail.\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }
    if(ret == AIR_E_OK)
    {
        switch (hashtype)
        {
            case 0:
                AIR_PRINT("hashtype:crc32lsb.\n");
                break;
            case 1:
                AIR_PRINT("hashtype:crc32msb.\n");
                break;
            case 2:
                AIR_PRINT("hashtype:crc16.\n");
                break;
            case 3:
                AIR_PRINT("hashtype:xor4.\n");
                break;
            default:
                AIR_PRINT("wrong hashtype:%d.\n",hashtype);
        }

    }
    return ret;
}

static AIR_ERROR_NO_T
doLagPtseed(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T ptseed = 0;

    if(1 == argc)
    {
        ptseed = _strtol(argv[0], NULL, 16);
        ret = air_lag_setPTSeed(0, ptseed);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set LAG Port Seed:%x(hex) ok\n",ptseed);
        }
        else
        {
            AIR_PRINT("Set LAG Port Seed:%x(hex) fail\n",ptseed);
        }
    }
    else if(0 == argc)
    {
        /* lag get seed */
        ret = air_lag_getPTSeed(0, &ptseed);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get port trunk seed: %x(hex)\n",ptseed);
        }
        else
        {
            AIR_PRINT("Get port trunk seed Fail.\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doLagSpsel(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    UI32_T state = 0;

    if(1 == argc)
    {
        /* lag set spsel <state> */
        state = _strtol(argv[0], NULL, 10);
        ret = air_lag_setSpSel(0,state);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set source port compare function:%s.\n", (state)?"Enabled":"Disabled");
        }
        else
        {
            AIR_PRINT("Set source port compare function Fail.\n");
        }
    }
    else if(0 == argc)
    {
        /* lag get spsel*/
        ret = air_lag_getSpSel(0, &state);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get source port compare function:%s.\n", (state)?"Enabled":"Disabled");
        }
        else
        {
            AIR_PRINT("Get source port compare function Fail.\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}


static AIR_ERROR_NO_T
doLagState(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    UI32_T state = 0;

    if(1 == argc)
    {
        /* lag set state <state> */
        state = _strtol(argv[0], NULL, 10);
        ret = air_lag_set_ptgc_state(0,state);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set LAG Port Trunk State:%s.\n", (state)?"Enabled":"Disabled");
        }
        else
        {
            AIR_PRINT("Set LAG Port Trunk State Fail.\n");
        }
    }
    else if(0 == argc)
    {
        /* lag get state*/
        ret = air_lag_get_ptgc_state(0, &state);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get LAG Port Trunk State:%s.\n", (state)?"Enabled":"Disabled");
        }
        else
        {
            AIR_PRINT("Get LAG Port Trunk State Fail.\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doLagGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(lagGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doLagSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(lagSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doLag(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(lagCmds, argc, argv);
}

static AIR_ERROR_NO_T
doStpPortstate(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    UI32_T fid = 0;
    UI32_T state = 0;

    port = _strtol(argv[0], NULL, 10);
    fid = _strtol(argv[1], NULL, 10);
    if(3 == argc)
    {
        /* stp set portstate <port> <fid(0..15)> <state> */
        state = _strtol(argv[2], NULL, 10);
        ret = air_stp_setPortstate(0, port, fid, state);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Set STP Port:%u FID:%u State:", port, fid);
            switch(state)
            {
                case AIR_STP_STATE_DISABLE:
                    AIR_PRINT("Disable(STP) / Discard(RSTP).\n");
                    break;
                case AIR_STP_STATE_LISTEN:
                    AIR_PRINT("Listening(STP) / Discard(RSTP).\n");
                    break;
                case AIR_STP_STATE_LEARN:
                    AIR_PRINT("Learning(STP) / Learning(RSTP).\n");
                    break;
                case AIR_STP_STATE_FORWARD:
                    AIR_PRINT("Forwarding(STP) / Forwarding(RSTP).\n");
                    break;
                default:
                    break;
            }
        }
        else
        {
            AIR_PRINT("Set STP Port:%u FID:%u State Fail.", port, fid);
        }
    }
    else if(2 == argc)
    {
        /* stp get portstate <port> <fid(0..15)> */
        ret = air_stp_getPortstate(0, port, fid, &state);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Get STP Port:%u FID:%u State:", port, fid);
            switch(state)
            {
                case AIR_STP_STATE_DISABLE:
                    AIR_PRINT("Disable(STP) / Discard(RSTP).\n");
                    break;
                case AIR_STP_STATE_LISTEN:
                    AIR_PRINT("Listening(STP) / Discard(RSTP).\n");
                    break;
                case AIR_STP_STATE_LEARN:
                    AIR_PRINT("Learning(STP) / Learning(RSTP).\n");
                    break;
                case AIR_STP_STATE_FORWARD:
                    AIR_PRINT("Forwarding(STP) / Forwarding(RSTP).\n");
                    break;
                default:
                    break;
            }
        }
        else
        {
            AIR_PRINT("Get STP Port:%u FID:%u State Fail.", port, fid);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doStpGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(stpGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doStpSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(stpSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doStp(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(stpCmds, argc, argv);
}

static void
_mir_printPortList(UI32_T * mt)
{
    I8_T j = 0;
    UI8_T first = 0;
    UI8_T find = 0;
    for(j = (AIR_MAX_NUM_OF_PORTS - 1); j >= 0; j--)
    {
        if((*mt) & (1 << j))
        {
            first = j;
            find = 1;
            break;
        }
    }
    if(find)
    {
        for(j = 0; j < AIR_MAX_NUM_OF_PORTS; j++)
        {
            if((*mt) & (1 << j))
            {
                if(j == first)
                    AIR_PRINT("%-2d", j);
                else
                    AIR_PRINT("%-2d,", j);
            }
        }
    }
    else
        AIR_PRINT("NULL");
    AIR_PRINT("\n");
}

static void
_mir_printSrcPortList(
    const UI32_T         unit,
    const UI32_T         sessionid)
{
    I8_T i = 0;
    AIR_MIR_SESSION_T   session;
    AIR_PORT_BITMAP_T txPbm = {0}, rxPbm = {0};
	AIR_ERROR_NO_T      rc = AIR_E_OK;

    for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
    {
         memset(&session, 0, sizeof(session));
         session.src_port = i;
         rc = air_mir_getMirrorPort(unit, sessionid, &session);
		 if (AIR_E_OK != rc)
		 {
			AIR_PRINT("***Error***,get port=%u error\n", i);
			return;
		 }

         if(session.flags & AIR_MIR_SESSION_FLAGS_DIR_TX)
         {
            txPbm[0] |= (1 << i);
         }
         if(session.flags & AIR_MIR_SESSION_FLAGS_DIR_RX)
         {
            rxPbm[0] |= (1 << i);
         }
    }
    AIR_PRINT("Src PortList\n");
    AIR_PRINT(" - Rx portlist = ");
    _mir_printPortList(rxPbm);
    AIR_PRINT(" - Tx portlist = ");
    _mir_printPortList(txPbm);
}

static void
_mir_printSession(
    const UI32_T            unit,
    const UI32_T            session_id,
    const AIR_MIR_SESSION_T *ptr_session)
{

    AIR_PRINT("Session id: %d\n", session_id);
    AIR_PRINT("State: %s \n", (ptr_session->flags & AIR_MIR_SESSION_FLAGS_ENABLE)? "enable": "disable");
    AIR_PRINT("Tx tag: %s \n", (ptr_session->flags & AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG)? "On": "Off");
    AIR_PRINT("Dst port: %d \n", ptr_session->dst_port);
    _mir_printSrcPortList(unit,session_id);
}

static AIR_ERROR_NO_T
doMirrorGetSid(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              session_id = 0;
    AIR_MIR_SESSION_T   session = {0};
    I8_T i = 0;

    session_id = _strtoul(argv[0], NULL, 0);
    rc = air_mir_getSession(0, session_id, &session);
    if (AIR_E_OK != rc)
    {
        AIR_PRINT("***Error***, get mirror session fail\n");
        return rc;
    }
    /* print session information */
    if(session.dst_port == AIR_PORT_INVALID)
    {
        AIR_PRINT("Session id %d not found\n", session_id);
    }
    else
    {
        _mir_printSession(0, session_id, &session);
    }
    return rc;
}

static AIR_ERROR_NO_T
doMirrorDelSid(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              session_id = 0;
    AIR_MIR_SESSION_T   session = {0};
    UI8_T i = 0;

    session_id = _strtoul(argv[0], NULL, 0);
    rc = air_mir_delSession(0, session_id);
    if (AIR_E_OK != rc)
    {
        AIR_PRINT("***Error***, del mirror session fail\n");
        return rc;
    }
    for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
    {
        session.src_port = i;
        rc = air_mir_setMirrorPort(0, session_id, &session);
        if (AIR_E_OK != rc)
        {
            AIR_PRINT("***Error***,port=%u error\n", i);
            return rc;
        }
    }
    if (rc != AIR_E_OK)
    {
        AIR_PRINT("***Error***, delete mirror session fail\n");
    }
    else
        AIR_PRINT("***OK***, delete mirror session success\n");

    return rc;
}

static AIR_ERROR_NO_T
doMirrorAddRlist(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              session_id = 0;
    AIR_MIR_SESSION_T   session = {0};
    AIR_PORT_BITMAP_T rxPbm = {0};
    UI8_T i = 0;

    /*mirror add session-rlist <sid(0..3)> <list(UINTLIST)>*/
    session_id = _strtoul(argv[0], NULL, 0);
    rc = _portListStr2Ary(argv[1], rxPbm, 1);
    if(rc != AIR_E_OK)
    {
        AIR_PRINT("Unrecognized command.\n");
        return rc;
    }
    if(!rxPbm[0])
    {
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            memset(&session, 0, sizeof(AIR_MIR_SESSION_T));
            session.src_port = i;
            rc = air_mir_getMirrorPort(0, session_id, &session);
            if (AIR_E_OK != rc)
            {
                AIR_PRINT("***Error***,get port=%u error\n", i);
                return rc;
            }

            session.flags &= ~AIR_MIR_SESSION_FLAGS_DIR_RX;
            session.src_port = i;
            rc = air_mir_setMirrorPort(0, session_id, &session);
            if (AIR_E_OK != rc)
            {
                AIR_PRINT("***Error***,set rx port=%u error\n", i);
                return rc;
            }
        }
    }
    else
    {
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(rxPbm[0] & (1 << i))
            {
                memset(&session, 0, sizeof(AIR_MIR_SESSION_T));
                session.src_port = i;
                rc = air_mir_getMirrorPort(0, session_id, &session);
                if (AIR_E_OK != rc)
                {
                    AIR_PRINT("***Error***,get port=%u error\n", i);
                    return rc;
                }

                session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
                session.src_port = i;
                rc = air_mir_setMirrorPort(0, session_id, &session);
                if (AIR_E_OK != rc)
                {
                    AIR_PRINT("***Error***,port=%u error\n", i);
                    return rc;
                }
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
doMirrorAddTlist(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              session_id = 0;
    AIR_MIR_SESSION_T   session = {0};
    AIR_PORT_BITMAP_T txPbm = {0};
    UI8_T i = 0;

    /*mirror add session-tlist <sid(0..3)> <list(UINTLIST)>*/
    session_id = _strtoul(argv[0], NULL, 0);
    rc = _portListStr2Ary(argv[1], txPbm, 1);
    if(rc != AIR_E_OK)
    {
        AIR_PRINT("Unrecognized command.\n");
        return rc;
    }
    if(!txPbm[0])
    {
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            memset(&session, 0, sizeof(AIR_MIR_SESSION_T));
            session.src_port = i;
            rc = air_mir_getMirrorPort(0, session_id, &session);
            if (AIR_E_OK != rc)
            {
                AIR_PRINT("***Error***,get port=%u error\n", i);
                return rc;
            }

            session.flags &= ~AIR_MIR_SESSION_FLAGS_DIR_TX;
            session.src_port = i;
            rc = air_mir_setMirrorPort(0, session_id, &session);
            if (AIR_E_OK != rc)
            {
                AIR_PRINT("***Error***,set rx port=%u error\n", i);
                return rc;
            }
        }
    }
    else
    {
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(txPbm[0] & (1 << i))
            {
                memset(&session, 0, sizeof(AIR_MIR_SESSION_T));
                session.src_port = i;
                rc = air_mir_getMirrorPort(0, session_id, &session);
                if (AIR_E_OK != rc)
                {
                    AIR_PRINT("***Error***,get port=%u error\n", i);
                    return rc;
                }

                session.flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
                session.src_port = i;
                rc = air_mir_setMirrorPort(0, session_id, &session);
                if (AIR_E_OK != rc)
                {
                    AIR_PRINT("***Error***,port=%u error\n", i);
                    return rc;
                }
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
doMirrorSetSessionEnable(
    UI32_T argc,
    C8_T *argv[])

{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              session_id = 0;
    UI32_T              enable = 0;
    BOOL_T              tmp_en = FALSE;

    /*mirror set session-enable <sid(0..3)> <state(1:En,0:Dis)>*/
    session_id = _strtoul(argv[0], NULL, 0);
    enable = _strtoul(argv[1], NULL, 0);
    if(enable)
        tmp_en = TRUE;
    /* set port mirror state */
    rc = air_mir_setSessionAdminMode(0, session_id, tmp_en);
    if(AIR_E_OK!=rc)
    {
        AIR_PRINT("***Error***\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
doMirrorSetSession(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              session_id = 0;
    UI32_T              dst_port = 0;
    UI8_T               enable = 0;
    UI8_T               tag_en = 0;
    UI8_T               dir = 0;
    AIR_MIR_SESSION_T  session = {0};
    AIR_PORT_BITMAP_T   rxPbm = {0};
    I8_T               i = 0;

    /*mirror set session <sid(0..3)> <dst_port(UINT)> <state(1:En,0:Dis)> <tag(1:on, 0:off)> <list(UINTLIST)> <dir(0:none,1:tx,2:rx,3:both)>*/
    session_id = _strtoul(argv[0], NULL, 0);
    dst_port = _strtoul(argv[1], NULL, 0);
    AIR_PRINT("session id %d dst port %d.\n", session_id, dst_port);
    session.dst_port = dst_port;
    enable = _strtoul(argv[2], NULL, 0);
    if(enable)
    {
        session.flags |= AIR_MIR_SESSION_FLAGS_ENABLE;
    }
    else
    {
        session.flags &= ~AIR_MIR_SESSION_FLAGS_ENABLE;
    }
    tag_en = _strtoul(argv[3], NULL, 0);
    if(tag_en)
    {
        session.flags |= AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG;
    }
    else
    {
        session.flags &= ~AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG;
    }
    rc = _portListStr2Ary(argv[4], rxPbm, 1);
    if(rc != AIR_E_OK)
    {
        AIR_PRINT("Unrecognized command.\n");
        return rc;
    }
    AIR_PRINT("pbm %x.\n", rxPbm);
    dir = _strtoul(argv[5], NULL, 0);
    if(dir == 1)
    {
        session.flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
    }
    else if(dir == 2)
    {
        session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
    }
    else if(dir == 3)
    {
        session.flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
        session.flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
    }
    else if (!dir)
    {
        session.flags &= ~AIR_MIR_SESSION_FLAGS_DIR_TX;
        session.flags &= ~AIR_MIR_SESSION_FLAGS_DIR_RX;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
    {
        if(rxPbm[0] & (1 << i))
        {
            session.src_port = i;
            /* set port mirror session */
            rc = air_mir_addSession(0, session_id, &session);

            if(AIR_E_OK!=rc)
            {
                AIR_PRINT("***Error***,dst-port=%u, src-port=%u error\n", session.dst_port, session.src_port);
                return rc;
            }
            else
                AIR_PRINT("add session %d,dst-port=%u, src-port=%u\n", session_id, session.dst_port, session.src_port);
        }
    }

    return rc;
}



static AIR_ERROR_NO_T
doMirrorSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(mirrorSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doMirrorAdd(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(mirrorAddCmds, argc, argv);
}

static AIR_ERROR_NO_T
doMirrorGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(mirrorGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doMirrorDel(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(mirrorDelCmds, argc, argv);
}

static AIR_ERROR_NO_T
doMirror(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(mirrorCmds, argc, argv);
}

static AIR_ERROR_NO_T
doMibClearPort(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;

    if(1 == argc)
    {
        /* mib clear port */
        port = _strtoul(argv[0], NULL, 0);
        ret = air_mib_clear_by_port(0,port);
        AIR_PRINT("Clear port %d mib stats",port);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Done.\n");
        }
        else
        {
            AIR_PRINT("Fail.\n");
        }
    }
    else if(0 == argc)
    {
        /*restart mib counter*/
        air_mib_clear(0);
        AIR_PRINT("Clear all mib stats",port);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doMibClearAcl(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;

    if(0 == argc)
    {
        /* mib clear acl */
        ret = air_mib_clearAclEvent(0);
        AIR_PRINT("Clear ACL Event Counter ");
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Done.\n");
        }
        else
        {
            AIR_PRINT("Fail.\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doMibGetPort(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0;
    UI32_T tmp32 = 0xffffffff;
    AIR_MIB_CNT_RX_T rx_mib = {0};
    AIR_MIB_CNT_TX_T tx_mib = {0};

    port = _strtoul(argv[0], NULL, 0);
    if(1 == argc)
    {
        /* mib get <port(0..6)> */
        ret = air_mib_get(0, port, &rx_mib, &tx_mib);
        AIR_PRINT("Get MIB Counter of Port %u ", port);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Done.\n");
            AIR_PRINT("RX Drop Packet                    : %u\n", rx_mib.RDPC);
            AIR_PRINT("RX filtering Packet               : %u\n", rx_mib.RFPC);
            AIR_PRINT("RX Unicast Packet                 : %u\n", rx_mib.RUPC);
            AIR_PRINT("RX Multicast Packet               : %u\n", rx_mib.RMPC);
            AIR_PRINT("RX Broadcast Packet               : %u\n", rx_mib.RBPC);
            AIR_PRINT("RX Alignment Error Packet         : %u\n", rx_mib.RAEPC);
            AIR_PRINT("RX CRC Packet                     : %u\n", rx_mib.RCEPC);
            AIR_PRINT("RX Undersize Packet               : %u\n", rx_mib.RUSPC);
            AIR_PRINT("RX Fragment Error Packet          : %u\n", rx_mib.RFEPC);
            AIR_PRINT("RX Oversize Packet                : %u\n", rx_mib.ROSPC);
            AIR_PRINT("RX Jabber Error Packet            : %u\n", rx_mib.RJEPC);
            AIR_PRINT("RX Pause Packet                   : %u\n", rx_mib.RPPC);
            AIR_PRINT("RX Packet Length 64 bytes         : %u\n", rx_mib.RL64PC);
            AIR_PRINT("RX Packet Length 65 ~ 127 bytes   : %u\n", rx_mib.RL65PC);
            AIR_PRINT("RX Packet Length 128 ~ 255 bytes  : %u\n", rx_mib.RL128PC);
            AIR_PRINT("RX Packet Length 256 ~ 511 bytes  : %u\n", rx_mib.RL256PC);
            AIR_PRINT("RX Packet Length 512 ~ 1023 bytes : %u\n", rx_mib.RL512PC);
            AIR_PRINT("RX Packet Length 1024 ~ 1518 bytes: %u\n", rx_mib.RL1024PC);
            AIR_PRINT("RX Packet Length 1519 ~ max bytes : %u\n", rx_mib.RL1519PC);
            AIR_PRINT("RX_CTRL Drop Packet               : %u\n", rx_mib.RCDPC);
            AIR_PRINT("RX Ingress Drop Packet            : %u\n", rx_mib.RIDPC);
            AIR_PRINT("RX ARL Drop Packet                : %u\n", rx_mib.RADPC);
            AIR_PRINT("FLow Control Drop Packet          : %u\n", rx_mib.FCDPC);
            AIR_PRINT("WRED Drop Packtet                 : %u\n", rx_mib.WRDPC);
            AIR_PRINT("Mirror Drop Packet                : %u\n", rx_mib.MRDPC);
            AIR_PRINT("RX  sFlow Sampling Packet         : %u\n", rx_mib.SFSPC);
            AIR_PRINT("Rx sFlow Total Packet             : %u\n", rx_mib.SFTPC);
            AIR_PRINT("Port Control Drop Packet          : %u\n", rx_mib.RXC_DPC);
            AIR_PRINT("RX Octets good or bad packtes l32 : %u\n", (UI32_T)(rx_mib.ROC & tmp32));
            AIR_PRINT("RX Octets good or bad packtes h32 : %u\n", (UI32_T)((rx_mib.ROC >> 32) & tmp32));
            AIR_PRINT("RX Octets bad packets l32         : %u\n", (UI32_T)(rx_mib.ROC2 & tmp32));
            AIR_PRINT("RX Octets bad packets h32         : %u\n", (UI32_T)((rx_mib.ROC2 >> 32) & tmp32));
            AIR_PRINT("\n");
            AIR_PRINT("TX Drop Packet                    : %u\n", tx_mib.TDPC);
            AIR_PRINT("TX CRC Packet                     : %u\n", tx_mib.TCRC);
            AIR_PRINT("TX Unicast Packet                 : %u\n", tx_mib.TUPC);
            AIR_PRINT("TX Multicast Packet               : %u\n", tx_mib.TMPC);
            AIR_PRINT("TX Broadcast Packet               : %u\n", tx_mib.TBPC);
            AIR_PRINT("TX Collision Event Count          : %u\n", tx_mib.TCEC);
            AIR_PRINT("TX Single Collision Event Count   : %u\n", tx_mib.TSCEC);
            AIR_PRINT("TX Multiple Conllision Event Count: %u\n", tx_mib.TMCEC);
            AIR_PRINT("TX Deferred Event Count           : %u\n", tx_mib.TDEC);
            AIR_PRINT("TX Late Collision Event Count     : %u\n", tx_mib.TLCEC);
            AIR_PRINT("TX Excessive Collision Event Count: %u\n", tx_mib.TXCEC);
            AIR_PRINT("TX Pause Packet                   : %u\n", tx_mib.TPPC);
            AIR_PRINT("TX Packet Length 64 bytes         : %u\n", tx_mib.TL64PC);
            AIR_PRINT("TX Packet Length 65 ~ 127 bytes   : %u\n", tx_mib.TL65PC);
            AIR_PRINT("TX Packet Length 128 ~ 255 bytes  : %u\n", tx_mib.TL128PC);
            AIR_PRINT("TX Packet Length 256 ~ 511 bytes  : %u\n", tx_mib.TL256PC);
            AIR_PRINT("TX Packet Length 512 ~ 1023 bytes : %u\n", tx_mib.TL512PC);
            AIR_PRINT("TX Packet Length 1024 ~ 1518 bytes: %u\n", tx_mib.TL1024PC);
            AIR_PRINT("TX Packet Length 1519 ~ max bytes : %u\n", tx_mib.TL1519PC);
            AIR_PRINT("TX Oversize Drop Packet           : %u\n", tx_mib.TODPC);
            AIR_PRINT("TX Octets good or bad packtes l32 : %u\n", (UI32_T)(tx_mib.TOC & tmp32));
            AIR_PRINT("TX Octets good or bad packtes h32 : %u\n", (UI32_T)((tx_mib.TOC >> 32) & tmp32));
            AIR_PRINT("TX Octets bad packets l32         : %u\n", (UI32_T)(tx_mib.TOC2 & tmp32));
            AIR_PRINT("TX Octets bad packets h32         : %u\n", (UI32_T)((tx_mib.TOC2 >> 32) & tmp32));
        }
        else
        {
            AIR_PRINT("Fail.\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doMibGetAcl(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T event = 0;
    UI32_T cnt = 0;

    if(1 == argc)
    {
        /* mib get acl <event(0..7)> */
        event = _strtoul(argv[0], NULL, 0);
        ret = air_mib_getAclEvent(0, event, &cnt);
        AIR_PRINT("Get counter of ACL event %u ", event);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Done.\n");
            AIR_PRINT("ACL Event Counter:%u\n", cnt);
        }
        else
        {
            AIR_PRINT("Fail.\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doMibClear(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(mibClearCmds, argc, argv);
}

static AIR_ERROR_NO_T
doMibGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(mibGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doMib(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(mibCmds, argc, argv);
}

static AIR_ERROR_NO_T
doQosRateLimitExMngFrm(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T dir = 0;
    BOOL_T enable = FALSE;

    if(2 == argc)
    {
		dir = _strtoul(argv[0], NULL, 0);
        if(dir == 0)
            dir = AIR_QOS_RATE_DIR_EGRESS;
        else if(dir == 1)
            dir = AIR_QOS_RATE_DIR_INGRESS;
        else
        {
            AIR_PRINT("Unrecognized command.\n");
            ret = AIR_E_BAD_PARAMETER;
            return ret;
        }
        enable = _strtoul(argv[1], NULL, 0);
        ret = air_qos_setRateLimitExMngFrm(0, dir, enable);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Success.\n");
            AIR_PRINT("Set %s Rate Limit Control %s management frame.\n",
                    (AIR_QOS_RATE_DIR_INGRESS == dir)?"Ingress":"Egress",
                    (TRUE == enable)?"exclude":"include");
        }
        else
        {
            AIR_PRINT("Fail.\n");
            return ret;
        }
    }
    else if(0 == argc)
    {
        dir = AIR_QOS_RATE_DIR_EGRESS;
        ret = air_qos_getRateLimitExMngFrm(0, dir, &enable);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Success.\n");
            AIR_PRINT("Get %s Rate Limit Control %s management frame.\n",
                    (AIR_QOS_RATE_DIR_INGRESS == dir)?"Ingress":"Egress",
                    (TRUE == enable)?"exclude":"include");
        }
        else
        {
            AIR_PRINT("Fail.\n");
            return ret;
        }
        dir = AIR_QOS_RATE_DIR_INGRESS;
        ret = air_qos_getRateLimitExMngFrm(0, dir, &enable);
        if(ret == AIR_E_OK)
        {
            AIR_PRINT("Success.\n");
            AIR_PRINT("Get %s Rate Limit Control %s management frame.\n",
                    (AIR_QOS_RATE_DIR_INGRESS == dir)?"Ingress":"Egress",
                    (TRUE == enable)?"exclude":"include");
        }
        else
        {
            AIR_PRINT("Fail.\n");
            return ret;
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;

}

static AIR_ERROR_NO_T
doQosPortPriority(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_PORT_BITMAP_T portlist = {0};
    UI32_T priority = 0;
    UI8_T i = 0;

    ret = _portListStr2Ary(argv[0], portlist, 1);
    if(ret != AIR_E_OK)
    {
        AIR_PRINT("Unrecognized command.\n");
        return ret;
    }
    if(2 == argc)
    {
        priority = _strtoul(argv[1], NULL, 0);
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(portlist[0] & (1 << i))
            {
                ret = air_qos_setPortPriority(0, i, priority);
                if(ret == AIR_E_OK)
                {
                    AIR_PRINT("Set Port%02d port based priority %d Success.\n", i, priority);
                }
                else
                {
                    AIR_PRINT("Set Port%02d port based priority %d Fail.\n", i, priority);
                }
            }
        }
    }
    else if(1 == argc)
    {
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(portlist[0] & (1 << i))
            {
                ret = air_qos_getPortPriority(0, i, &priority);
                if(ret == AIR_E_OK)
                {
                    AIR_PRINT("Get Port%d port based priority %d.\n", i, priority);
                }
                else
                {
                    AIR_PRINT("Get Port%d port based priority Fail.\n", i);
                }
            }
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doQosRateLimit(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_PORT_BITMAP_T portlist = {0};
    AIR_QOS_RATE_LIMIT_CFG_T rl = {0};
    UI8_T i = 0;

    ret = _portListStr2Ary(argv[0], portlist, 1);
    if(ret != AIR_E_OK)
    {
        AIR_PRINT("Unrecognized command.\n");
        return ret;
    }
    if(5 == argc)
    {
        rl.ingress_cir = _strtoul(argv[1], NULL, 0);
        rl.ingress_cbs = _strtoul(argv[2], NULL, 0);
        rl.egress_cir = _strtoul(argv[3], NULL, 0);
        rl.egress_cbs = _strtoul(argv[4], NULL, 0);
        rl.flags |= AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_INGRESS;
        rl.flags |= AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_EGRESS;
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(portlist[0] & (1 << i))
            {
                ret = air_qos_setRateLimit(0, i, &rl);
                if(ret == AIR_E_OK)
                {
                    AIR_PRINT("Set Port%02d Ingress CIR %d CBS %d Egress CIR %d CBS %d Success.\n", i, rl.ingress_cir, rl.ingress_cbs, rl.egress_cir, rl.egress_cbs);
                }
                else
                {
                    AIR_PRINT("Set Port%02d Ingress CIR %d CBS %d Egress CIR %d CBS %d Fail.\n", i, rl.ingress_cir, rl.ingress_cbs, rl.egress_cir, rl.egress_cbs);
                }
            }
        }
    }
    else if(1 == argc)
    {
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(portlist[0] & (1 << i))
            {
                ret = air_qos_getRateLimit(0, i, &rl);
                if(ret == AIR_E_OK)
                {
                    AIR_PRINT("Get Port%02d Ingress CIR %d CBS %d Egress CIR %d CBS %d\n", i, rl.ingress_cir, rl.ingress_cbs, rl.egress_cir, rl.egress_cbs);
                }
                else
                {
                    AIR_PRINT("Get Port%02d Rate Info Fail.\n", i);
                }
            }
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doQosRateLimitEnable(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_PORT_BITMAP_T portlist = {0};
    C8_T sten[2][10] = {"Disable", "Enable"};
    C8_T stdir[2][10] = {"Egress", "Ingress"};
    UI32_T dir = 0, en = 0;
    AIR_QOS_RATE_DIR_T tmp_dir = AIR_QOS_RATE_DIR_LAST;
    BOOL_T state = FALSE;
    UI8_T i = 0;

    ret = _portListStr2Ary(argv[0], portlist, 1);
    if(ret != AIR_E_OK)
    {
        AIR_PRINT("Unrecognized command.\n");
        return ret;
    }
    if(3 == argc)
    {
        dir = _strtoul(argv[1], NULL, 0);
        en = _strtoul(argv[2], NULL, 0);
        if(dir == 0)
            tmp_dir = AIR_QOS_RATE_DIR_EGRESS;
        else if(dir == 1)
            tmp_dir = AIR_QOS_RATE_DIR_INGRESS;
        else
        {
            AIR_PRINT("Unrecognized command.\n");
            return AIR_E_BAD_PARAMETER;
        }
        if(en)
            state= TRUE;
        else
            state = FALSE;

        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(portlist[0] & (1 << i))
            {
                ret = air_qos_setRateLimitEnable(0, i, tmp_dir, state);
                if(AIR_E_OK == ret)
                {
                    AIR_PRINT("Set Port%02d %s rate %s Success.\n", i, stdir[dir], sten[en]);
                }
                else
                {
                    AIR_PRINT("Set Port%02d %s rate %s Fail.\n", i, stdir[dir], sten[en]);
                }
            }
        }
    }
    else if(1 == argc)
    {
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(portlist[0] & (1 << i))
            {
                tmp_dir = AIR_QOS_RATE_DIR_EGRESS;
                dir = 0;
                ret = air_qos_getRateLimitEnable(0, i, tmp_dir, &state);
                if(AIR_E_OK == ret)
                {
                    AIR_PRINT("Get Port%02d %s rate %s Success.\n", i, stdir[dir], sten[state]);
                }
                else
                {
                    AIR_PRINT("Get Port%02d %s rate state Fail.\n", i, stdir[dir]);
                }
                tmp_dir = AIR_QOS_RATE_DIR_INGRESS;
                dir = 1;
                ret = air_qos_getRateLimitEnable(0, i, tmp_dir, &state);
                if(AIR_E_OK == ret)
                {
                    AIR_PRINT("Get Port%02d %s rate %s Success.\n", i, stdir[dir], sten[state]);
                }
                else
                {
                    AIR_PRINT("Get Port%02d %s rate state Fail.\n", i, stdir[dir]);
                }
            }
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;

}

static AIR_ERROR_NO_T
doQosDscp2Pri(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T dscp = 0, priority = 0;

    dscp = _strtoul(argv[0], NULL, 0);
    if(2 == argc)
    {
        priority = _strtoul(argv[1], NULL, 0);
        ret = air_qos_setDscp2Pri(0, dscp, priority);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Set DSCP %d to priority %d Success.\n", dscp, priority);
        }
        else
        {
            AIR_PRINT("Set DSCP %d to priority %d Fail.\n", dscp, priority);
        }
    }
    else if(1 == argc)
    {
        ret = air_qos_getDscp2Pri(0, dscp, &priority);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Get DSCP %d to priority %d\n", dscp, priority);
        }
        else
        {
            AIR_PRINT("Get DSCP %d to priority Fail.\n", dscp);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doQosPri2Queue(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T priority = 0, queue = 0;

    priority = _strtoul(argv[1], NULL, 0);

    if(2 == argc)
    {
        priority = _strtoul(argv[0], NULL, 0);
        queue = _strtoul(argv[1], NULL, 0);
        ret = air_qos_setPri2Queue(0, priority, queue);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Set priority %d to queue %d Success.\n", priority, queue);
        }
        else
        {
            AIR_PRINT("Set priority %d to queue %d Fail.\n", priority, queue);
        }
    }
    else
    {
        for(; priority < AIR_QOS_QUEUE_MAX_NUM; priority++)
        {
            ret = air_qos_getPri2Queue(0, priority, &queue);
            if(AIR_E_OK == ret)
            {
                AIR_PRINT("Get priority %d to queue %d\n", priority, queue);
            }
            else
            {
                AIR_PRINT("Get priority %d to queue Fail.\n", priority);
            }
        }
    }

    return ret;
}

static AIR_ERROR_NO_T
doQosTrustMode(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T mode = 0;
    C8_T bs[4][13] = {"port", "1p_port", "dscp_port", "dscp_1p_port"};
    AIR_QOS_TRUST_MODE_T mode_t = AIR_QOS_TRUST_MODE_LAST;
    AIR_PORT_BITMAP_T portlist = {0};
    UI8_T i = 0;

    ret = _portListStr2Ary(argv[0], portlist, 1);
    if(ret != AIR_E_OK)
    {
        AIR_PRINT("Unrecognized command.\n");
        return ret;
    }
    if(2 == argc)
    {
        mode = _strtoul(argv[1], NULL, 0);
        if(mode == 0)
            mode_t = AIR_QOS_TRUST_MODE_PORT;
        else if(mode == 1)
            mode_t = AIR_QOS_TRUST_MODE_1P_PORT;
        else if(mode == 2)
            mode_t = AIR_QOS_TRUST_MODE_DSCP_PORT;
        else if(mode == 3)
            mode_t = AIR_QOS_TRUST_MODE_DSCP_1P_PORT;
        else
        {
            AIR_PRINT("Unrecognized command.\n");
            return AIR_E_BAD_PARAMETER;
        }
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(portlist[0] & (1 << i))
            {
                ret = air_qos_setTrustMode(0, i, mode_t);
                if(AIR_E_OK == ret)
                {
                    AIR_PRINT("port %d Set Trust mode %s Success.\n", i, bs[mode]);
                }
                else
                {
                    AIR_PRINT("port %d Set Trust mode %s Fail.\n", i, bs[mode]);
                }
            }
        }
    }
    else if(1 == argc)
    {
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(portlist[0] & (1 << i))
            {
                mode_t = AIR_QOS_TRUST_MODE_LAST;
                ret = air_qos_getTrustMode(0, i, &mode_t);
                if(AIR_E_OK == ret)
                {
                    if(mode_t == AIR_QOS_TRUST_MODE_PORT)
                        mode = 0;
                    else if(mode_t == AIR_QOS_TRUST_MODE_1P_PORT)
                        mode = 1;
                    else if(mode_t == AIR_QOS_TRUST_MODE_DSCP_PORT)
                        mode = 2;
                    else if(mode_t == AIR_QOS_TRUST_MODE_DSCP_1P_PORT)
                        mode = 3;
                    else
                    {
                        AIR_PRINT("port %d Get Trust mode Fail.\n", i);
                        return AIR_E_OTHERS;
                    }
                    AIR_PRINT("port %d Get Trust mode %s\n", i, bs[mode]);
                }
                else
                {
                    AIR_PRINT("port %d Get Trust mode Fail.\n", i);
                }
            }
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doQosScheduleAlgo(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_PORT_BITMAP_T portlist = {0};
    AIR_QOS_SCH_MODE_T sch_mode = AIR_QOS_SCH_MODE_LAST;
    UI32_T scheduler = 0;
    UI8_T queue = 0;
    C8_T sche[3][5] = {"SP", "WRR", "WFQ"};
    UI32_T weight = AIR_QOS_SHAPER_NOSETTING;
    UI8_T i = 0;

    ret = _portListStr2Ary(argv[0], portlist, 1);
    if(ret != AIR_E_OK)
    {
        AIR_PRINT("Unrecognized command.\n");
        return ret;
    }
    AIR_PRINT("port list is %d\n", portlist[0]);
    if(4 == argc)
    {
        queue = _strtoul(argv[1], NULL, 0);
        AIR_PRINT("queue is %d\n", queue);
        scheduler = _strtoul(argv[2], NULL, 0);
        AIR_PRINT("scheduler is %d\n", scheduler);
        weight = _strtoul(argv[3], NULL, 0);
        AIR_PRINT("weight is %d\n", weight);
        if(scheduler == 0)
        {
            sch_mode = AIR_QOS_SCH_MODE_SP;
            weight = AIR_QOS_SHAPER_NOSETTING;
            if(weight != AIR_QOS_SHAPER_NOSETTING)
                AIR_PRINT("[Warning] SP schedule mode no need weight\n");
        }
        else if(scheduler == 1)
        {
            sch_mode = AIR_QOS_SCH_MODE_WRR;
            if(weight == AIR_QOS_SHAPER_NOSETTING)
            {
                AIR_PRINT("[Warning] No weight value input , plz check\n");
                return AIR_E_BAD_PARAMETER;
            }
            AIR_PRINT("sch_mode is 1\n");
        }
        else if(scheduler == 2)
        {
            sch_mode = AIR_QOS_SCH_MODE_WFQ;
            if(weight == AIR_QOS_SHAPER_NOSETTING)
            {
                AIR_PRINT("[Warning] No weight value input , plz check\n");
                return AIR_E_BAD_PARAMETER;
            }
        }
        else
        {
            AIR_PRINT("Unknown schedule mode, plz check again\n");
            return AIR_E_BAD_PARAMETER;
        }
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(portlist[0] & (1 << i))
            {
                AIR_PRINT("port %d\n", i);
                ret = air_qos_setScheduleAlgo(0, i, queue, sch_mode, weight);
                if(AIR_E_OK == ret)
                {
                    AIR_PRINT("Set Port%02d Scheduler %s Success.\n", i, sche[scheduler]);
                }
                else
                {
                    AIR_PRINT("Set Port%02d Scheduler %s Fail.\n", i, sche[scheduler]);
                }
            }
        }
    }
    else if(2 == argc)
    {
        queue = _strtoul(argv[1], NULL, 0);
        for(i = 0; i < AIR_MAX_NUM_OF_PORTS; i++)
        {
            if(portlist[0] & (1 << i))
            {
                ret = air_qos_getScheduleAlgo(0, i, queue, &sch_mode, &weight);
                if(AIR_E_OK == ret)
                {
                    if(sch_mode == AIR_QOS_SCH_MODE_SP)
                        AIR_PRINT("Get Port%02d queue %d Scheduler %s\n", i, queue, sche[sch_mode]);
                    else if((sch_mode == AIR_QOS_SCH_MODE_WRR) || (sch_mode == AIR_QOS_SCH_MODE_WFQ))
                        AIR_PRINT("Get Port%02d queue %d Scheduler %s weight %d\n", i, queue, sche[sch_mode], weight);
                    else
                        AIR_PRINT("Get Port%02d queue %d Scheduler unknown\n", i, queue);
                }
                else
                {
                    AIR_PRINT("Get Port%02d queue %d Scheduler Fail.\n", i, queue);
                }
            }
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doQosGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(qosGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doQosSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(qosSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doQos(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(qosCmds, argc, argv);
}
static AIR_ERROR_NO_T
doDiagTxComply(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T phy = 0;
    UI32_T mode = 0;

    phy = _strtoul(argv[0], NULL, 0);
    if(2 == argc)
    {
        /* diag set txComply <phy(0~5)> <mode(0~8)> */
        mode = _strtoul(argv[1], NULL, 0);
        ret = air_diag_setTxComplyMode(0, phy, mode);
        AIR_PRINT("Set diagnostic function: PHY %u Tx Compliance mode = %u ", phy, mode);
    }
    else if(1 == argc)
    {
        /* diag get txComply <phy(0~5)> */
        ret = air_diag_getTxComplyMode(0, phy, &mode);
        AIR_PRINT("Get diagnostic function: PHY %u Tx Compliance mode ", phy);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        AIR_PRINT("Done.\n\tMode=");
        switch(mode)
        {
            case AIR_DIAG_TXCOMPLY_MODE_10M_NLP:
                AIR_PRINT("%s\n", "10M_NLP");
                break;
            case AIR_DIAG_TXCOMPLY_MODE_10M_RANDOM:
                AIR_PRINT("%s\n", "10M_Random");
                break;
            case AIR_DIAG_TXCOMPLY_MODE_10M_SINE:
                AIR_PRINT("%s\n", "10M_Sine");
                break;
            case AIR_DIAG_TXCOMPLY_MODE_100M_PAIR_A:
                AIR_PRINT("%s\n", "100M_Pair_a");
                break;
            case AIR_DIAG_TXCOMPLY_MODE_100M_PAIR_B:
                AIR_PRINT("%s\n", "100M_Pair_b");
                break;
            case AIR_DIAG_TXCOMPLY_MODE_1000M_TM1:
                AIR_PRINT("%s\n", "1000M_TM1");
                break;
            case AIR_DIAG_TXCOMPLY_MODE_1000M_TM2:
                AIR_PRINT("%s\n", "1000M_TM2");
                break;
            case AIR_DIAG_TXCOMPLY_MODE_1000M_TM3:
                AIR_PRINT("%s\n", "1000M_TM3");
                break;
            case AIR_DIAG_TXCOMPLY_MODE_1000M_TM4:
                AIR_PRINT("%s\n", "1000M_TM4");
                break;
            default:
                break;
        }
    }
    else
    if(AIR_E_OTHERS == ret)
    {
        AIR_PRINT("isn't setting.\n");
    }
    else
    {
        AIR_PRINT("Fail.\n");
    }
    return ret;
}

static AIR_ERROR_NO_T
doDiagSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(diagSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doDiagGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(diagGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doDiag(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(diagCmds, argc, argv);
}

static AIR_ERROR_NO_T
doLedMode(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T mode = 0;

    if(1 == argc)
    {
        /* led set mode <mode(0:disable, 1~3:2 LED, 4:User-Define)> */
        mode = _strtoul(argv[0], NULL, 0);
        ret = air_led_setMode(0, 0, mode);
        AIR_PRINT("Set LED mode ");
    }
    else if(0 == argc)
    {
        /* led get mode */
        ret = air_led_getMode(0, 0, &mode);
        AIR_PRINT("Get LED mode ");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        switch(mode)
        {
            case AIR_LED_MODE_DISABLE:
                AIR_PRINT(": Disabled.\n");
                break;
            case AIR_LED_MODE_2LED_MODE0:
                AIR_PRINT(": LED 0:Link / LED 1:Activity.\n");
                break;
            case AIR_LED_MODE_2LED_MODE1:
                AIR_PRINT(": LED 0:1000M Activity / LED 1:100M Activity.\n");
                break;
            case AIR_LED_MODE_2LED_MODE2:
                AIR_PRINT(": LED 0:1000M Activity / LED 1:10&100M Activity.\n");
                break;
            case AIR_LED_MODE_USER_DEFINE:
                AIR_PRINT(": User-Defined.\n");
                break;
            default:
                AIR_PRINT(": Fail.\n");
                break;
        }
    }
    else
    if(AIR_E_OTHERS == ret)
    {
        AIR_PRINT(": Unrecognized.\n");
    }
    else
    {
        AIR_PRINT("Fail.\n");
    }
    return ret;
}

static AIR_ERROR_NO_T
doLedState(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI8_T entity = 0;
    BOOL_T state = FALSE;

    entity = _strtoul(argv[0], NULL, 0);
    if(2 == argc)
    {
        /* led set state <led(0..1)> <state(1:En 0:Dis)> */
        state = _strtoul(argv[1], NULL, 0);
        ret = air_led_setState(0, 0, entity, state);
        AIR_PRINT("Set LED %u state ", entity);
    }
    else if(1 == argc)
    {
        /* led get state <led(0..1)> */
        ret = air_led_getState(0, 0, entity, &state);
        AIR_PRINT("Get LED %u state ", entity );
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        AIR_PRINT(": %s.\n", (state)?"Enable":"Disabled");
    }
    else
    if(AIR_E_OTHERS == ret)
    {
        AIR_PRINT(": Unrecognized.\n");
    }
    else
    {
        AIR_PRINT("Fail.\n");
    }
    return ret;
}

static AIR_ERROR_NO_T
doLedUsrDef(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T i = 0;
    UI8_T entity = 0;
    BOOL_T polarity = LOW;
    UI32_T on_evt_map = 0;
    UI32_T blk_evt_map = 0;
    AIR_LED_ON_EVT_T on_evt;
    AIR_LED_BLK_EVT_T blk_evt;

    entity = _strtoul(argv[0], NULL, 0);
    if(4 == argc)
    {
        /* led set usr <led(0..1)> <polarity(0:low, 1:high)> <on_evt(7'bin)> <blink_evt(10'bin)> */
        polarity = _strtoul(argv[1], NULL, 0);
        on_evt_map = _strtoul(argv[2], NULL, 2);
        blk_evt_map = _strtoul(argv[3], NULL, 2);

        memset(&on_evt, 0, sizeof(AIR_LED_ON_EVT_T));
        if(on_evt_map & BIT(0))
        {
            on_evt.link_1000m = TRUE;
        }
        if(on_evt_map & BIT(1))
        {
            on_evt.link_100m = TRUE;
        }
        if(on_evt_map & BIT(2))
        {
            on_evt.link_10m = TRUE;
        }
        if(on_evt_map & BIT(3))
        {
            on_evt.link_dn = TRUE;
        }
        if(on_evt_map & BIT(4))
        {
            on_evt.fdx = TRUE;
        }
        if(on_evt_map & BIT(5))
        {
            on_evt.hdx = TRUE;
        }
        if(on_evt_map & BIT(6))
        {
            on_evt.force = TRUE;
        }

        memset(&blk_evt, 0, sizeof(AIR_LED_BLK_EVT_T));
        if(blk_evt_map & BIT(0))
        {
            blk_evt.tx_act_1000m = TRUE;
        }
        if(blk_evt_map & BIT(1))
        {
            blk_evt.rx_act_1000m = TRUE;
        }
        if(blk_evt_map & BIT(2))
        {
            blk_evt.tx_act_100m = TRUE;
        }
        if(blk_evt_map & BIT(3))
        {
            blk_evt.rx_act_100m = TRUE;
        }
        if(blk_evt_map & BIT(4))
        {
            blk_evt.tx_act_10m = TRUE;
        }
        if(blk_evt_map & BIT(5))
        {
            blk_evt.rx_act_10m = TRUE;
        }
        if(blk_evt_map & BIT(6))
        {
            blk_evt.cls = TRUE;
        }
        if(blk_evt_map & BIT(7))
        {
            blk_evt.rx_crc = TRUE;
        }
        if(blk_evt_map & BIT(8))
        {
            blk_evt.rx_idle = TRUE;
        }
        if(blk_evt_map & BIT(9))
        {
            blk_evt.force = TRUE;
        }
        ret = air_led_setUsrDef(0, 0, entity, polarity, on_evt, blk_evt);
        AIR_PRINT("Set LED %u User-define ", entity);
    }
    else if(1 == argc)
    {
        /* led get usr <led(0..1)> */
        ret = air_led_getUsrDef(0, 0, entity, &polarity, &on_evt, &blk_evt);
        AIR_PRINT("Get LED %u User-define ", entity );
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        AIR_PRINT("Done.\n");
        AIR_PRINT("Polarity:%u.\n", polarity);
        AIR_PRINT("On Event:\n");
        i = 6;
        AIR_PRINT("\t(%u)Force on :%s\n", i--, (on_evt.force)?"On":"Off");
        AIR_PRINT("\t(%u)Half Duplex :%s\n", i--, (on_evt.hdx)?"On":"Off");
        AIR_PRINT("\t(%u)Full Duplex :%s\n", i--, (on_evt.fdx)?"On":"Off");
        AIR_PRINT("\t(%u)Link Down :%s\n", i--, (on_evt.link_dn)?"On":"Off");
        AIR_PRINT("\t(%u)Link 10M :%s\n", i--, (on_evt.link_10m)?"On":"Off");
        AIR_PRINT("\t(%u)Link 100M :%s\n", i--, (on_evt.link_100m)?"On":"Off");
        AIR_PRINT("\t(%u)Link 1000M :%s\n", i, (on_evt.link_1000m)?"On":"Off");

        AIR_PRINT("Blinking Event:\n");
        i = 9;
        AIR_PRINT("\t(%u)Force blinks :%s\n", i--, (blk_evt.force)?"On":"Off");
        AIR_PRINT("\t(%u)Rx Idle Error :%s\n", i--, (blk_evt.rx_idle)?"On":"Off");
        AIR_PRINT("\t(%u)Rx CRC Error :%s\n", i--, (blk_evt.rx_crc)?"On":"Off");
        AIR_PRINT("\t(%u)Collision :%s\n", i--, (blk_evt.cls)?"On":"Off");
        AIR_PRINT("\t(%u)10Mbps RX Activity :%s\n", i--, (blk_evt.rx_act_10m)?"On":"Off");
        AIR_PRINT("\t(%u)10Mbps TX Activity :%s\n", i--, (blk_evt.tx_act_10m)?"On":"Off");
        AIR_PRINT("\t(%u)100Mbps RX Activity :%s\n", i--, (blk_evt.rx_act_100m)?"On":"Off");
        AIR_PRINT("\t(%u)100Mbps TX Activity :%s\n", i--, (blk_evt.tx_act_100m)?"On":"Off");
        AIR_PRINT("\t(%u)1000Mbps RX Activity :%s\n", i--, (blk_evt.rx_act_1000m)?"On":"Off");
        AIR_PRINT("\t(%u)1000Mbps TX Activity :%s\n", i, (blk_evt.tx_act_1000m)?"On":"Off");
    }
    else
    {
        AIR_PRINT("Fail.\n");
    }
    return ret;
}

static AIR_ERROR_NO_T
doLedBlkTime(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_LED_BLK_DUR_T time = 0;

    if(1 == argc)
    {
        /* led set time <time(0~5)> */
        time = _strtoul(argv[0], NULL, 0);
        ret = air_led_setBlkTime(0, 0, time);
        AIR_PRINT("Set Blinking Duration ");
    }
    else if(0 == argc)
    {
        /* led get time */
        ret = air_led_getBlkTime(0, 0, &time);
        AIR_PRINT("Get Blinking Duration ");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        AIR_PRINT("Done.\n");
        AIR_PRINT("\tBlinking duration : %u (ms)\n", (32 << time) );
    }
    else
    {
        AIR_PRINT("Fail.\n");
    }
    return ret;
}

static AIR_ERROR_NO_T
doLedSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(ledSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doLedGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(ledGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doLed(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(ledCmds, argc, argv);
}

static AIR_ERROR_NO_T
doShowVersion(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_PRINT("VERSION: %s\n", AIR_VER_SDK);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
doShow(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(showCmds, argc, argv);
}

static AIR_ERROR_NO_T
doStormRate(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0, type = 0;
    UI32_T unit = 0, count = 0;
    C8_T stype[3][5] = {"Bcst", "Mcst", "Ucst"};
    UI32_T kb = 0;

    port = _strtol(argv[0], NULL, 10);
    type = _strtol(argv[1], NULL, 10);
    if(4 == argc)
    {
        count = _strtol(argv[2], NULL, 10);
        unit = _strtol(argv[3], NULL, 10);

        if(0 == unit)
            kb = 64;
        else if(1 == unit)
            kb = 256;
        else if(2 == unit)
            kb = 1024;
        else if(3 == unit)
            kb = 4096;
        else
            kb = 16384;
        ret = air_sec_setStormRate(0, port, type, count, unit);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Set Port%02d %s storm rate (%d * %d) = %d Kbps\n", port, stype[type], count, kb, (count*kb));
        }
        else
        {
            AIR_PRINT("Set Port%02d %s storm rate Fail.\n", port, stype[type]);
            AIR_PRINT("Note: Port(0..4) can only select unit(0..3), port(5..6) can only select unit(4)\n");
        }
    }
    else if(2 == argc)
    {
        ret = air_sec_getStormRate(0, port, type, &count, &unit);
        if(AIR_E_OK == ret)
        {
            if(0 == unit)
                kb = 64;
            else if(1 == unit)
                kb = 256;
            else if(2 == unit)
                kb = 1024;
            else if(3 == unit)
                kb = 4096;
            else
                kb = 16384;
            AIR_PRINT("Port%02d %s storm rate (%d * %d) = %d Kbps\n", port, stype[type], count, kb, (count*kb));
        }
        else
        {
            AIR_PRINT("Get Port%02d %s storm rate Fail\n", port, stype[type]);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doFldMode(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0, type = 0;
    BOOL_T fld_en = 0;
    C8_T stype[4][5] = {"Bcst", "Mcst", "Ucst", "Qury"};
    C8_T sen[2][10] = {"Disable", "Enable"};

    port = _strtol(argv[0], NULL, 10);
    type = _strtol(argv[1], NULL, 10);

    if(2 == argc)
    {
        ret = air_sec_getFldMode(0, port, type, &fld_en);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Get Port%02d flooding %s frame %s\n", port, stype[type], sen[fld_en]);
        }
        else
        {
            AIR_PRINT("Get Port%02d flooding %s frame Fail\n", port, stype[type]);
        }
    }
    else if(3 == argc)
    {
        fld_en = _strtol(argv[2], NULL, 10);
        ret = air_sec_setFldMode(0, port, type, fld_en);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Set Port%02d flooding %s frame %s Success\n", port, stype[type], sen[fld_en]);
        }
        else
        {
            AIR_PRINT("Set Port%02d flooding %s frame %s Fail\n", port, stype[type], sen[fld_en]);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doStormEnable(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T port = 0, type = 0;
    BOOL_T en = 0;
    C8_T sen[2][10] = {"Disable", "Enable"};
    C8_T stype[3][5] = {"Bcst", "Mcst", "Ucst"};

    port = _strtol(argv[0], NULL, 10);
    type = _strtol(argv[1], NULL, 10);
    if(3 == argc)
    {
        en = _strtol(argv[2], NULL, 10);
        ret = air_sec_setStormEnable(0, port, type, en);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Set Port%02d %s storm %s Success.\n", port, stype[type], sen[en]);
        }
        else
        {
            AIR_PRINT("Set Port%02d %s storm %s Fail.\n", port, stype[type], sen[en]);
        }
    }
    else if(2 == argc)
    {
        ret = air_sec_getStormEnable(0, port, type, &en);
        if(AIR_E_OK == ret)
        {
            AIR_PRINT("Port%02d %s storm %s\n", port, stype[type], sen[en]);
        }
        else
        {
            AIR_PRINT("Get Port%02d %s storm Fail\n", port, stype[type]);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doSaLearning(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    AIR_SEC_PORTSEC_PORT_CONFIG_T port_config;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    if(2 == argc)
    {
        memset(&port_config, 0, sizeof(AIR_SEC_PORTSEC_PORT_CONFIG_T));
        rc = air_sec_getPortSecPortCfg(0, port, &port_config);
        if (AIR_E_OK != rc)
        {
            AIR_PRINT("Get Port%02d sa learn Fail.\n", port);
            return rc;
        }
        port_config.sa_lrn_en = _strtoul(argv[1], NULL, 0);
        rc = air_sec_setPortSecPortCfg(0, port, port_config);
        if(AIR_E_OK == rc)
        {
            AIR_PRINT("Set Port%02d sa learn %s Success.\n", port, port_config.sa_lrn_en?"Enable":"Disable");
        }
        else
        {
            AIR_PRINT("Set Port%02d sa learn %s Fail.\n", port, port_config.sa_lrn_en?"Enable":"Disable");
        }
    }
    else if(1 == argc)
    {
        rc = air_sec_getPortSecPortCfg(0, port, &port_config);
        if(AIR_E_OK == rc)
        {
            AIR_PRINT("Port%02d sa learn: %s\n", port, port_config.sa_lrn_en?"Enable":"Disable");
        }
        else
        {
            AIR_PRINT("Get Port%02d sa learn Fail\n", port);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T
doSaLimit(UI32_T argc, C8_T *argv[])
{
    UI32_T port = 0;
    AIR_SEC_PORTSEC_PORT_CONFIG_T port_config;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    port = _strtoul(argv[0], NULL, 0);
    if(3 == argc)
    {
        memset(&port_config, 0, sizeof(AIR_SEC_PORTSEC_PORT_CONFIG_T));
        rc = air_sec_getPortSecPortCfg(0, port, &port_config);
        if (AIR_E_OK != rc)
        {
            AIR_PRINT("Get Port%02d sa limit Fail.\n", port);
            return rc;
        }
        port_config.sa_lmt_en = _strtoul(argv[1], NULL, 0);
        port_config.sa_lmt_cnt = _strtoul(argv[2], NULL, 0);
        rc = air_sec_setPortSecPortCfg(0, port, port_config);
        if(AIR_E_OK == rc)
        {
            AIR_PRINT("Set Port%02d sa limit %s Success.\n", port, port_config.sa_lmt_en?"Enable":"Disable");
        }
        else
        {
            AIR_PRINT("Set Port%02d sa limit %s Fail.\n", port, port_config.sa_lmt_en?"Enable":"Disable");
        }
    }
    else if(1 == argc)
    {
        rc = air_sec_getPortSecPortCfg(0, port, &port_config);
        if(AIR_E_OK == rc)
        {
            AIR_PRINT("Port%02d ", port);
            AIR_PRINT("sa limit: %s\n", port_config.sa_lmt_en?"Enable":"Disable");
            if(TRUE == (port_config.sa_lmt_en && (AIR_MAX_NUM_OF_MAC ==  port_config.sa_lmt_cnt)))
            {
                AIR_PRINT("Sa learning without limitation\n");
            }
            else if(TRUE == (port_config.sa_lmt_en && (AIR_MAX_NUM_OF_MAC >  port_config.sa_lmt_cnt)))
            {
                AIR_PRINT("Rx sa allowable learning number: %d\n", port_config.sa_lmt_cnt);
            }
        }
        else
        {
            AIR_PRINT("Get Port%02d sa limit Fail\n", port);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}

static AIR_ERROR_NO_T
doSecGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(secGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doSecSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(secSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doSec(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(secCmds, argc, argv);
}

static AIR_ERROR_NO_T
doSwitchCpuPortEn(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    BOOL_T cpu_en = FALSE;

    if(0 == argc)
    {
        /* switch get sysPhyEn */
        ret = air_switch_getCpuPortEn(0, &cpu_en);
        AIR_PRINT("Get Cpu Port State ");
        if(ret == AIR_E_OK)
        {
            AIR_PRINT(": %s\n", cpu_en?"Enable":"Disable");
        }
        else
        {
            AIR_PRINT("Fail!\n");
        }
    }
    else if(1 == argc)
    {
        /* switch set sysPhyEn <phy_en> */
        cpu_en = _strtol(argv[0], NULL, 0);
        ret = air_switch_setCpuPortEn(0, cpu_en);
        AIR_PRINT("Set CPU port State ");
        if(ret == AIR_E_OK)
        {
            AIR_PRINT(": %s\n", cpu_en?"Enable":"Disable");
        }
        else
        {
            AIR_PRINT("Fail!\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doSwitchCpuPort(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    BOOL_T cpu_en = FALSE;
    UI32_T port = 0;
    C8_T str_temp[AIR_MAX_NUM_OF_PORTS+1];

    if(1 == argc)
    {
        /* switch set cpuPort <portnumber> */
        port = _strtol(argv[0], NULL, 10);
        ret = air_switch_setCpuPort(0, port);
        AIR_PRINT("Set CPU Port ");
    }
    else if(0 == argc)
    {
        /* switch get cpuPort */
        ret = air_switch_getCpuPort(0, &port);
        AIR_PRINT("Get CPU Port ");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        AIR_PRINT(": %d\n", port);
    }
    else
    {
        AIR_PRINT("Fail!\n");
    }
    return ret;
}

static AIR_ERROR_NO_T
doSwitchPhyLCIntrEn(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T phy = 0;
    BOOL_T enable = FALSE;

    if(2 == argc)
    {
        /* switch set phyLCIntrEn <phy(0..6)> <(1:En,0:Dis)> */
        phy    = _strtol(argv[0], NULL, 10);
        enable = _strtol(argv[1], NULL, 10);
        ret    = air_switch_setSysIntrEn(0, (phy + AIR_SYS_INTR_TYPE_PHY0_LC), enable);
    }
    else if(1 == argc)
    {
        /* switch get phyLCIntrEn <phy(0..6)> */
        phy = _strtol(argv[0], NULL, 10);
        ret = air_switch_getSysIntrEn(0, (phy + AIR_SYS_INTR_TYPE_PHY0_LC), &enable);
        AIR_PRINT("PHY(%d) LinkChange interrupt : %s\n", phy, (TRUE == enable) ? "enable" : "disable");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doSwitchPhyLCIntrSts(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T phy = 0;
    BOOL_T enable = FALSE;

    if(2 == argc)
    {
        /* switch set phyLCIntrSts <phy(0..6)> <(1:En)> */
        phy    = _strtol(argv[0], NULL, 10);
        enable = _strtol(argv[1], NULL, 10);
        ret    = air_switch_setSysIntrStatus(0, (phy + AIR_SYS_INTR_TYPE_PHY0_LC), enable);
    }
    else if(1 == argc)
    {
        /* switch get phyLCIntrSts <phy(0..6)> */
        phy = _strtol(argv[0], NULL, 10);
        ret = air_switch_getSysIntrStatus(0, (phy + AIR_SYS_INTR_TYPE_PHY0_LC), &enable);
        AIR_PRINT("PHY(%d) LinkChange interrupt state : %s\n", phy, (TRUE == enable) ? "set" : "unset");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        ret = AIR_E_BAD_PARAMETER;
    }

    return ret;
}


static AIR_ERROR_NO_T
doSwitchSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(switchSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doSwitchGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(switchGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doSwitch(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(switchCmds, argc, argv);
}

static void _air_acl_printRuleMap(UI32_T *rule_map, UI32_T ary_num)
{
    UI32_T i;
    BOOL_T first;

    first = TRUE;
    for(i=0; i<ary_num*32; i++)
    {
        if(rule_map[i/32] & BIT(i%32))
        {
            if(TRUE == first)
            {
                AIR_PRINT("%u", i);
                first = FALSE;
            }
            else
            {
                AIR_PRINT(",%u", i);
            }
        }
    }
    AIR_PRINT("\n");
}

static AIR_ERROR_NO_T
doAclEn(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T argi=0;
    UI32_T port = 0;
    BOOL_T en = FALSE;

    if(1 == argc)
    {
        /* acl set en <en(1:En,0:Dis)> */
        en = _strtoul(argv[argi++], NULL, 2);
        ret = air_acl_setGlobalState(0, en);
        AIR_PRINT("Set Global ACL function ");
    }
    else if(0 == argc)
    {
        /* acl get en */
        ret = air_acl_getGlobalState(0, &en);
        AIR_PRINT("Get Global ACL function ");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        AIR_PRINT(": %s\n", (TRUE == en)?"Enable":"Disable");
    }
    else
    {
        AIR_PRINT("Fail!\n");
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclRule(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T rule_idx = 0;
    BOOL_T state = FALSE, reverse = FALSE, end = FALSE;
    UI32_T argi = 0, ipv6 = 0, i = 0;
    AIR_ACL_RULE_T rule;
    UI8_T tmp_ip[16] = {0};
    C8_T str_temp[AIR_MAX_NUM_OF_PORTS+1];
    C8_T str[40];

    memset(&rule, 0, sizeof(AIR_ACL_RULE_T));
    if(argc >= 6)
    {
        /* acl set rule <idx(0..255)> <state(0:Dis,1:En)> <reverse(0:Dis,1:En)> <end(0:Dis,1:En)> <portmap(7'bin)>
        <ipv6(0:Dis,1:En,2:Not care)>
        [ dmac <dmac(12'hex)> <dmac_mask(12'hex)> ]
        [ smac <smac(12'hex)> <smac_mask(12'hex)> ]
        [ stag <stag(4'hex)> <stag_mask(4'hex)> ]
        [ ctag <ctag(4'hex)> <ctag_mask(4'hex)> ]
        [ etype <etype(4'hex)> <etype_mask(4'hex)> ]
        [ dip <dip(IPADDR)> <dip_mask(IPADDR)> ]
        [ sip <sip(IPADDR)> <sip_mask(IPADDR)> ]
        [ dscp <dscp(2'hex)> <dscp_mask(2'hex)> ]
        [ protocol <protocol(2'hex)> <protocol_mask(2'hex)> ]
        [ dport <dport(4'hex)> <dport_mask(4'hex)> ]
        [ sport <sport(4'hex)> <sport_mask(4'hex)> ]
        [ flow_label <flow_label(4'hex)> <flow_label_mask(4'hex)> ]
        [ udf <udf(4'hex)> <udf_mask(4'hex)> ] */

        rule_idx = _strtoul(argv[argi++], NULL, 0);

        rule.ctrl.rule_en = _strtoul(argv[argi++], NULL, 0);
        rule.ctrl.reverse = _strtoul(argv[argi++], NULL, 0);
        rule.ctrl.end = _strtoul(argv[argi++], NULL, 0);

        rule.key.portmap = _strtoul(argv[argi++], NULL, 2);
        rule.mask.portmap = (~rule.key.portmap) & AIR_ALL_PORT_BITMAP;

        ipv6 = _strtoul(argv[argi++], NULL, 0);
        if(0 == ipv6)
        {
            rule.key.isipv6 = FALSE;
            rule.mask.isipv6 = TRUE;
        }
        else if(1 == ipv6)
        {
            rule.key.isipv6 = TRUE;
            rule.mask.isipv6 = TRUE;
        }
        else
        {
            rule.mask.isipv6 = FALSE;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "dmac")))
        {
            argi++;
            _str2mac(argv[argi++], rule.key.dmac);
            _str2mac(argv[argi++], rule.mask.dmac);
            rule.key.fieldmap |= 1 << AIR_ACL_DMAC;
        }
        if((argi < argc) && (0 == _strcmp(argv[argi], "smac")))
        {
            argi++;
            _str2mac(argv[argi++], rule.key.smac);
            _str2mac(argv[argi++], rule.mask.smac);
            rule.key.fieldmap |= 1 << AIR_ACL_SMAC;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "stag")))
        {
            argi++;
            rule.key.stag = _strtoul(argv[argi++], NULL, 16);
            rule.mask.stag = _strtoul(argv[argi++], NULL, 16);
            rule.key.fieldmap |= 1 << AIR_ACL_STAG;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "ctag")))
        {
            argi++;
            rule.key.ctag = _strtoul(argv[argi++], NULL, 16);
            rule.mask.ctag = _strtoul(argv[argi++], NULL, 16);
            rule.key.fieldmap |= 1 << AIR_ACL_CTAG;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "etype")))
        {
            argi++;
            rule.key.etype= _strtoul(argv[argi++], NULL, 16);
            rule.mask.etype = _strtoul(argv[argi++], NULL, 16);
            rule.key.fieldmap |= 1 << AIR_ACL_ETYPE;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "dip")))
        {
            argi++;
            if(0 == ipv6)
            {
                _str2ipv4(argv[argi++], rule.key.dip);
                _str2ipv4(argv[argi++], rule.mask.dip);
            }
            else if(1 == ipv6)
            {
                _str2ipv6(argv[argi++], tmp_ip);
                rule.key.dip[3] = (tmp_ip[0]<<24) | (tmp_ip[1]<<16) | (tmp_ip[2]<<8) | tmp_ip[3];
                rule.key.dip[2] = (tmp_ip[4]<<24) | (tmp_ip[5]<<16) | (tmp_ip[6]<<8) | tmp_ip[7];
                rule.key.dip[1] = (tmp_ip[8]<<24) | (tmp_ip[9]<<16) | (tmp_ip[10]<<8) | tmp_ip[11];
                rule.key.dip[0] = (tmp_ip[12]<<24) | (tmp_ip[13]<<16) | (tmp_ip[14]<<8) | tmp_ip[15];
                _str2ipv6(argv[argi++], tmp_ip);
                rule.mask.dip[3] = (tmp_ip[0]<<24) | (tmp_ip[1]<<16) | (tmp_ip[2]<<8) | tmp_ip[3];
                rule.mask.dip[2] = (tmp_ip[4]<<24) | (tmp_ip[5]<<16) | (tmp_ip[6]<<8) | tmp_ip[7];
                rule.mask.dip[1] = (tmp_ip[8]<<24) | (tmp_ip[9]<<16) | (tmp_ip[10]<<8) | tmp_ip[11];
                rule.mask.dip[0] = (tmp_ip[12]<<24) | (tmp_ip[13]<<16) | (tmp_ip[14]<<8) | tmp_ip[15];
            }
            rule.key.fieldmap |= 1 << AIR_ACL_DIP;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "sip")))
        {
            argi++;
            if(0 == ipv6)
            {
                _str2ipv4(argv[argi++], rule.key.sip);
                _str2ipv4(argv[argi++], rule.mask.sip);
            }
            else if(1 == ipv6)
            {
                _str2ipv6(argv[argi++], tmp_ip);
                rule.key.sip[3] = (tmp_ip[0]<<24) | (tmp_ip[1]<<16) | (tmp_ip[2]<<8) | tmp_ip[3];
                rule.key.sip[2] = (tmp_ip[4]<<24) | (tmp_ip[5]<<16) | (tmp_ip[6]<<8) | tmp_ip[7];
                rule.key.sip[1] = (tmp_ip[8]<<24) | (tmp_ip[9]<<16) | (tmp_ip[10]<<8) | tmp_ip[11];
                rule.key.sip[0] = (tmp_ip[12]<<24) | (tmp_ip[13]<<16) | (tmp_ip[14]<<8) | tmp_ip[15];
                _str2ipv6(argv[argi++], tmp_ip);
                rule.mask.sip[3] = (tmp_ip[0]<<24) | (tmp_ip[1]<<16) | (tmp_ip[2]<<8) | tmp_ip[3];
                rule.mask.sip[2] = (tmp_ip[4]<<24) | (tmp_ip[5]<<16) | (tmp_ip[6]<<8) | tmp_ip[7];
                rule.mask.sip[1] = (tmp_ip[8]<<24) | (tmp_ip[9]<<16) | (tmp_ip[10]<<8) | tmp_ip[11];
                rule.mask.sip[0] = (tmp_ip[12]<<24) | (tmp_ip[13]<<16) | (tmp_ip[14]<<8) | tmp_ip[15];
            }
            rule.key.fieldmap |= 1 << AIR_ACL_SIP;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "dscp")))
        {
            argi++;
            rule.key.dscp = _strtoul(argv[argi++], NULL, 16);
            rule.mask.dscp = _strtoul(argv[argi++], NULL, 16);
            rule.key.fieldmap |= 1 << AIR_ACL_DSCP;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "protocol")))
        {
            argi++;
            rule.key.protocol = _strtoul(argv[argi++], NULL, 16);
            rule.mask.protocol = _strtoul(argv[argi++], NULL, 16);
            rule.key.fieldmap |= 1 << AIR_ACL_PROTOCOL;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "dport")))
        {
            argi++;
            rule.key.dport = _strtoul(argv[argi++], NULL, 16);
            rule.mask.dport = _strtoul(argv[argi++], NULL, 16);
            rule.key.fieldmap |= 1 << AIR_ACL_DPORT;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "sport")))
        {
            argi++;
            rule.key.sport = _strtoul(argv[argi++], NULL, 16);
            rule.mask.sport = _strtoul(argv[argi++], NULL, 16);
            rule.key.fieldmap |= 1 << AIR_ACL_SPORT;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "flow_label")))
        {
            argi++;
            rule.key.flow_label= _strtoul(argv[argi++], NULL, 16);
            rule.mask.flow_label= _strtoul(argv[argi++], NULL, 16);
            rule.key.fieldmap |= 1 << AIR_ACL_FLOW_LABEL;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "udf")))
        {
            argi++;
            rule.key.udf = _strtoul(argv[argi++], NULL, 16);
            rule.mask.udf = _strtoul(argv[argi++], NULL, 16);
            rule.key.fieldmap |= 1 << AIR_ACL_UDF;
        }
        rule.mask.fieldmap = rule.key.fieldmap;
        ret = air_acl_setRule(0, rule_idx, &rule);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Set ACL Rule(%u): %s\n", rule_idx, air_error_getString(ret));
    }
    else if(1 == argc)
    {
        rule_idx = _strtoul(argv[0], NULL, 0);
        ret = air_acl_getRule(0, rule_idx, &rule);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Get ACL Rule(%u): %s\n", rule_idx, air_error_getString(ret));
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        if(TRUE == rule.ctrl.rule_en)
        {
            AIR_PRINT("\t Rule end          : %s\n", (TRUE == rule.ctrl.end)?"Enable":"Disable");
            AIR_PRINT("\t Rule reverse      : %s\n", (TRUE == rule.ctrl.reverse)?"Enable":"Disable");
            _hex2bitstr((~rule.mask.portmap) & AIR_ALL_PORT_BITMAP, str_temp, AIR_MAX_NUM_OF_PORTS+1);
            AIR_PRINT("\t Portmap[0:6]      : %s\n", str_temp);
            for(i = AIR_ACL_DMAC; i < AIR_ACL_FIELD_TYPE_LAST; i++)
            {
                if((1 << i) & rule.mask.fieldmap)
                {
                    switch (i)
                    {
                        case AIR_ACL_DMAC:
                            AIR_PRINT("\t dmac: ");
                            AIR_PRINT("%02x-%02x-%02x-%02x-%02x-%02x",
                            rule.key.dmac[0], rule.key.dmac[1], rule.key.dmac[2],
                            rule.key.dmac[3], rule.key.dmac[4], rule.key.dmac[5]);
                            AIR_PRINT(", dmac-mask: ");
                            AIR_PRINT("%02x-%02x-%02x-%02x-%02x-%02x\n",
                            rule.mask.dmac[0], rule.mask.dmac[1], rule.mask.dmac[2],
                            rule.mask.dmac[3], rule.mask.dmac[4], rule.mask.dmac[5]);
                            break;
                        case AIR_ACL_SMAC:
                            AIR_PRINT("\t smac: ");
                            AIR_PRINT("%02x-%02x-%02x-%02x-%02x-%02x",
                            rule.key.smac[0], rule.key.smac[1], rule.key.smac[2],
                            rule.key.smac[3], rule.key.smac[4], rule.key.smac[5]);
                            AIR_PRINT(", smac-mask: ");
                            AIR_PRINT("%02x-%02x-%02x-%02x-%02x-%02x\n",
                            rule.mask.smac[0], rule.mask.smac[1], rule.mask.smac[2],
                            rule.mask.smac[3], rule.mask.smac[4], rule.mask.smac[5]);
                            break;
                        case AIR_ACL_ETYPE:
                            AIR_PRINT("\t etype: 0x%x, etype-mask: 0x%x\n", rule.key.etype, rule.mask.etype);
                            break;
                        case AIR_ACL_STAG:
                            AIR_PRINT("\t stag: 0x%x, stag-mask: 0x%x\n", rule.key.stag, rule.mask.stag);
                            break;
                        case AIR_ACL_CTAG:
                            AIR_PRINT("\t ctag: 0x%x, ctag-mask: 0x%x\n", rule.key.ctag, rule.mask.ctag);
                            break;
                        case AIR_ACL_DPORT:
                            AIR_PRINT("\t dport: 0x%x, dport-mask: 0x%x\n", rule.key.dport, rule.mask.dport);
                            break;
                        case AIR_ACL_SPORT:
                            AIR_PRINT("\t sport: 0x%x, sport-mask: 0x%x\n", rule.key.sport, rule.mask.sport);
                            break;
                        case AIR_ACL_UDF:
                            AIR_PRINT("\t udf: 0x%x, udf-mask: 0x%x\n", rule.key.udf, rule.mask.udf);
                            break;
                        case AIR_ACL_DIP:
                            if (0 == rule.key.isipv6)
                            {
                                AIR_PRINT("\t dip: ");
                                AIR_PRINT("%d.%d.%d.%d",
                                ((rule.key.dip[0])&0xFF000000)>>24,((rule.key.dip[0])&0x00FF0000)>>16,
                                ((rule.key.dip[0])&0x0000FF00)>>8, ((rule.key.dip[0])&0x000000FF));
                                AIR_PRINT(", dip-mask: ");
                                AIR_PRINT("%d.%d.%d.%d\n ",
                                ((rule.mask.dip[0])&0xFF000000)>>24,((rule.mask.dip[0])&0x00FF0000)>>16,
                                ((rule.mask.dip[0])&0x0000FF00)>>8, ((rule.mask.dip[0])&0x000000FF));
                            }
                            else
                            {
                                for(i=0; i<4; i++){
                                    tmp_ip[i] = (rule.key.dip[3] >> (8*(3-i))) & 0xff;
                                    AIR_PRINT("get tmp_ip[%d]=0x%x\n", i, tmp_ip[i]);
                                }
                                for(i=4; i<8; i++){
                                    tmp_ip[i] = (rule.key.dip[2] >> (8*(7-i))) & 0xff;
                                }
                                for(i=8; i<12; i++){
                                    tmp_ip[i] = (rule.key.dip[1] >> (8*(11-i))) & 0xff;
                                }
                                for(i=12; i<16; i++){
                                    tmp_ip[i] = (rule.key.dip[0] >> (8*(15-i))) & 0xff;
                                }

                                AIR_PRINT("\t dip: ");
                                AIR_PRINT("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                                    tmp_ip[0], tmp_ip[1],tmp_ip[2], tmp_ip[3],tmp_ip[4], tmp_ip[5],tmp_ip[6], tmp_ip[7],
                                    tmp_ip[8], tmp_ip[9],tmp_ip[10], tmp_ip[11],tmp_ip[12], tmp_ip[13],tmp_ip[14], tmp_ip[15]);
                                for(i=0; i<4; i++){
                                    tmp_ip[i] = (rule.mask.dip[3] >> (8*(3-i))) & 0xff;
                                }
                                for(i=4; i<8; i++){
                                    tmp_ip[i] = (rule.mask.dip[2] >> (8*(7-i))) & 0xff;
                                }
                                for(i=8; i<12; i++){
                                    tmp_ip[i] = (rule.mask.dip[1] >> (8*(11-i))) & 0xff;
                                }
                                for(i=12; i<16; i++){
                                    tmp_ip[i] = (rule.mask.dip[0] >> (8*(15-i))) & 0xff;
                                }
                                AIR_PRINT(", dip-mask: ");
                                AIR_PRINT("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                                    tmp_ip[0], tmp_ip[1],tmp_ip[2], tmp_ip[3],tmp_ip[4], tmp_ip[5],tmp_ip[6], tmp_ip[7],
                                    tmp_ip[8], tmp_ip[9],tmp_ip[10], tmp_ip[11],tmp_ip[12], tmp_ip[13],tmp_ip[14], tmp_ip[15]);
                            }
                            break;
                        case AIR_ACL_SIP:
                            if (0 == rule.key.isipv6)
                            {
                                AIR_PRINT("\t sip: ");
                                AIR_PRINT("%d.%d.%d.%d ",
                                ((rule.key.sip[0])&0xFF000000)>>24,((rule.key.sip[0])&0x00FF0000)>>16,
                                ((rule.key.sip[0])&0x0000FF00)>>8, ((rule.key.sip[0])&0x000000FF));
                                AIR_PRINT(", sip-mask: ");
                                AIR_PRINT("%d.%d.%d.%d\n ",
                                ((rule.mask.sip[0])&0xFF000000)>>24,((rule.mask.sip[0])&0x00FF0000)>>16,
                                ((rule.mask.sip[0])&0x0000FF00)>>8, ((rule.mask.sip[0])&0x000000FF));
                            }
                            else
                            {
                                for(i=0; i<4; i++){
                                    tmp_ip[i] = (rule.key.sip[3] >> (8*(3-i))) & 0xff;
                                }
                                for(i=4; i<8; i++){
                                    tmp_ip[i] = (rule.key.sip[2] >> (8*(7-i))) & 0xff;
                                }
                                for(i=8; i<12; i++){
                                    tmp_ip[i] = (rule.key.sip[1] >> (8*(11-i))) & 0xff;
                                }
                                for(i=12; i<16; i++){
                                    tmp_ip[i] = (rule.key.sip[0] >> (8*(15-i))) & 0xff;
                                }
                                AIR_PRINT("\t sip: ");
                                AIR_PRINT("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                                    tmp_ip[0], tmp_ip[1],tmp_ip[2], tmp_ip[3],tmp_ip[4], tmp_ip[5],tmp_ip[6], tmp_ip[7],
                                    tmp_ip[8], tmp_ip[9],tmp_ip[10], tmp_ip[11],tmp_ip[12], tmp_ip[13],tmp_ip[14], tmp_ip[15]);
                                for(i=0; i<4; i++){
                                    tmp_ip[i] = (rule.mask.sip[3] >> (8*(3-i))) & 0xff;
                                }
                                for(i=4; i<8; i++){
                                    tmp_ip[i] = (rule.mask.sip[2] >> (8*(7-i))) & 0xff;
                                }
                                for(i=8; i<12; i++){
                                    tmp_ip[i] = (rule.mask.sip[1] >> (8*(11-i))) & 0xff;
                                }
                                for(i=12; i<16; i++){
                                    tmp_ip[i] = (rule.mask.sip[0] >> (8*(15-i))) & 0xff;
                                }
                                AIR_PRINT(", sip-mask: ");
                                AIR_PRINT("%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                                    tmp_ip[0], tmp_ip[1],tmp_ip[2], tmp_ip[3],tmp_ip[4], tmp_ip[5],tmp_ip[6], tmp_ip[7],
                                    tmp_ip[8], tmp_ip[9],tmp_ip[10], tmp_ip[11],tmp_ip[12], tmp_ip[13],tmp_ip[14], tmp_ip[15]);
                            }
                            break;
                        case AIR_ACL_DSCP:
                            AIR_PRINT("\t dscp: 0x%x, dscp-mask: 0x%x\n", rule.key.dscp, rule.mask.dscp);
                            break;
                        case AIR_ACL_PROTOCOL:
                            AIR_PRINT("\t protocol: 0x%x, protocol-mask: 0x%x\n", rule.key.protocol, rule.mask.protocol);
                            break;
                        case AIR_ACL_FLOW_LABEL:
                            AIR_PRINT("\t flow-label: 0x%x, flow-label-mask: 0x%x\n", rule.key.flow_label, rule.mask.flow_label);
                            break;
                        default:
                            AIR_PRINT("error\n");
                            break;
                    }
                }
            }
        }
        else
        {
            AIR_PRINT("Entry is Invalid.\n");
        }
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclRmvRule(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T rule_idx = 0;

    if(1 == argc)
    {
        /* acl del rule <idx(0..127)> */
        rule_idx = _strtoul(argv[0], NULL, 0);
        ret = air_acl_delRule(0, rule_idx);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Delete ACL Rule(%u): %s\n", rule_idx, air_error_getString(ret));
    }
    else if(0 == argc)
    {
        /* acl clear rule */
        ret = air_acl_clearRule(0);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Clear ACL Rule: %s\n", air_error_getString(ret));
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclUdfRule(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T rule_idx;
    AIR_ACL_UDF_RULE_T rule;
    C8_T start_addr[8][12]=
    {
        "MAC header",
        "L2 payload",
        "IPv4 header",
        "IPv6 header",
        "L3 payload",
        "TCP header",
        "UDP header",
        "L4 payload"
    };
    C8_T str_temp[AIR_MAX_NUM_OF_PORTS+1];

    memset(&rule, 0, sizeof(AIR_ACL_UDF_RULE_T));
    if(7 == argc)
    {
        /* acl set rule <idx(0..255)> <mode(0:pattern, 1:threshold)> [ <pat(4'hex)> <mask(4'hex)> | <low(4'hex)> <high(4'hex)> ] <start(0:MAC,1:ether,2:IP,3:IP_data,4:TCP/UDP,5:TCP/UDP data,6:IPv6)> <offset(0..62,unit:2 bytes)> <portmap(7'bin)> */
        rule_idx = _strtoul(argv[0], NULL, 0);
        rule.cmp_sel = _strtoul(argv[1], NULL, 2);
        if(AIR_ACL_RULE_CMP_SEL_PATTERN == rule.cmp_sel)
        {
            rule.pattern = _strtoul(argv[2], NULL, 16);
            rule.mask = _strtoul(argv[3], NULL, 16);
        }
        else
        {
            rule.low_threshold = _strtoul(argv[2], NULL, 16);
            rule.high_threshold = _strtoul(argv[3], NULL, 16);
        }
        rule.offset_format = _strtoul(argv[4], NULL, 0);
        rule.offset = _strtoul(argv[5], NULL, 0);
        rule.portmap = _strtoul(argv[6], NULL, 2);
        rule.valid = TRUE;
        ret = air_acl_setUdfRule(0, rule_idx, rule);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Set ACL UDF Rule(%u): %s\n", rule_idx, air_error_getString(ret));
    }
    else if(1 == argc)
    {
        rule_idx = _strtoul(argv[0], NULL, 0);
        ret = air_acl_getUdfRule(0, rule_idx, &rule);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Get ACL UDF Rule(%u): %s\n", rule_idx, air_error_getString(ret));
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        if(TRUE == rule.valid)
        {
            AIR_PRINT("\tMode          : %s\n", (AIR_ACL_RULE_CMP_SEL_PATTERN == rule.cmp_sel)?"Pattern":"Threshold");
            if(AIR_ACL_RULE_CMP_SEL_PATTERN == rule.cmp_sel)
            {
                AIR_PRINT("\tPattern       : 0x%04X\n", rule.pattern);
                AIR_PRINT("\tMask          : 0x%04X\n", rule.mask);
            }
            else
            {
                AIR_PRINT("\tLow Threshold : 0x%04X\n", rule.low_threshold);
                AIR_PRINT("\tHigh Threshold: 0x%04X\n", rule.high_threshold);
            }
            AIR_PRINT("\tOffset Start  : %s\n", start_addr[rule.offset_format]);
            AIR_PRINT("\tOffset        : %u %s\n", rule.offset*2, (0==rule.offset)?"Byte":"Bytes");
            _hex2bitstr(rule.portmap, str_temp, AIR_MAX_NUM_OF_PORTS+1);
            AIR_PRINT("\tPortmap[0:6]  : %s\n", str_temp);
        }
        else
        {
            AIR_PRINT("Entry is Invalid.\n");
        }
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclRmvUdfRule(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T rule_idx;

    if(1 == argc)
    {
        /* acl del udfRule <idx(0..15)> */
        rule_idx = _strtoul(argv[0], NULL, 0);
        ret = air_acl_delUdfRule(0, rule_idx);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Delete ACL UDF Rule(%u): %s\n", rule_idx, air_error_getString(ret));
    }
    else if(0 == argc)
    {
        /* acl clear udfRule */
        ret = air_acl_clearUdfRule(0);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Clear ACL UDF Rule: %s\n", air_error_getString(ret));
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclAction(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T argi = 0;
    UI32_T act_idx;
    AIR_ACL_ACTION_T act;
    UI32_T redirect, trtcm, fwd;
    C8_T fwding[AIR_ACL_ACT_FWD_LAST][25] =
    {
        "Default",
        "Default",
        "Default",
        "Default",
        "Default & CPU excluded",
        "Default & CPU included",
        "CPU only",
        "Drop"
    };
    C8_T trtcm_usr[AIR_ACL_ACT_USR_TCM_LAST][8] =
    {
        "Default",
        "Green",
        "Yellow",
        "Red"
    };
    C8_T egtag[AIR_ACL_ACT_EGTAG_LAST][11] =
    {
        "Default",
        "Consistent",
        "",
        "",
        "Untag",
        "Swap",
        "Tag",
        "Stack"
    };
    C8_T str_temp[20];

    memset(&act, 0, sizeof(AIR_ACL_ACTION_T));
    if(2 < argc)
    {
        /* acl set action <idx(0..127)>
        [ forward <forward(0:Default,4:Exclude CPU,5:Include CPU,6:CPU only,7:Drop)> ]
        [ egtag <egtag(0:Default,1:Consistent,4:Untag,5:Swap,6:Tag,7:Stack)> ]
        [ mirrormap <mirrormap(2'bin)> ]
        [ priority <priority(0..7)> ]
        [ redirect <redirect(0:Dst,1:Vlan)> <portmap(7'bin)> ]
        [ leaky_vlan <leaky_vlan(1:En,0:Dis)> ]
        [ cnt_idx <cnt_idx(0..63)> ]
        [ rate_idx <rate_idx(0..31)> ]
        [ attack_idx <attack_idx(0..95)> ]
        [ vid <vid(0..4095)> ]
        [ manage <manage(1:En,0:Dis)> ]
        [ bpdu <bpdu(1:En,0:Dis)> ]
        [ class <class(0:Original,1:Defined)>[0..7] ]
        [ drop_pcd <drop_pcd(0:Original,1:Defined)> [red <red(0..7)>][yellow <yellow(0..7)>][green <green(0..7)>] ]
        [ color <color(0:Defined,1:Trtcm)> [ <defined_color(0:Dis,1:Green,2:Yellow,3:Red)> | <trtcm_idx(0..31)> ] ]*/

        act_idx = _strtoul(argv[argi++], NULL, 0);
        if((argi < argc) && (0 == _strcmp(argv[argi], "forward")))
        {
            argi++;
            fwd = _strtoul(argv[argi++], NULL, 0);
            act.fwd_en = TRUE;
            act.fwd = fwd;
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "egtag")))
        {
            argi++;
            act.egtag_en = TRUE;
            act.egtag = _strtoul(argv[argi++], NULL, 0);
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "mirrormap")))
        {
            argi++;
            act.mirrormap = _strtoul(argv[argi++], NULL, 2);
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "priority")))
        {
            argi++;
            act.pri_user_en = TRUE;
            act.pri_user= _strtoul(argv[argi++], NULL, 0);
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "redirect")))
        {
            argi++;
            redirect = _strtoul(argv[argi++], NULL, 0);
            if(0 ==  redirect)
            {
                act.port_en = TRUE;
                act.dest_port_sel = TRUE;
                act.portmap = _strtoul(argv[argi++], NULL, 2);
            }
            else
            {
                act.port_en = TRUE;
                act.vlan_port_sel = TRUE;
                act.portmap = _strtoul(argv[argi++], NULL, 2);
            }
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "leaky_vlan")))
        {
            argi++;
            act.lyvlan_en = TRUE;
            act.lyvlan = _strtoul(argv[argi++], NULL, 0);
        }

        /* ACL event counter */
        if((argi < argc) && (0 == _strcmp(argv[argi], "cnt_idx")))
        {
            argi++;
            act.cnt_en = TRUE;
            act.cnt_idx = _strtol(argv[argi++], NULL, 0);
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "rate_idx")))
        {
            argi++;
            act.rate_en = TRUE;
            act.rate_idx = _strtol(argv[argi++], NULL, 0);
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "attack_idx")))
        {
            argi++;
            act.attack_en = TRUE;
            act.attack_idx = _strtol(argv[argi++], NULL, 0);
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "vid")))
        {
            argi++;
            act.vlan_en = TRUE;
            act.vlan_idx = _strtol(argv[argi++], NULL, 0);
        }

        /* Management frame */
        if((argi < argc) && (0 == _strcmp(argv[argi], "manage")))
        {
            argi++;
            act.mang = _strtoul(argv[argi++], NULL, 2);
        }

        if((argi < argc) && (0 == _strcmp(argv[argi], "bpdu")))
        {
            argi++;
            act.bpdu = _strtoul(argv[argi++], NULL, 2);
        }

        /* DSCP class remap */
        if((argi < argc) && (0 == _strcmp(argv[argi], "class")))
        {
            argi++;
            act.trtcm_en = TRUE;
            act.trtcm.cls_slr_sel = _strtoul(argv[argi++], NULL, 2);
            if(TRUE == act.trtcm.cls_slr_sel)
            {
                act.trtcm.cls_slr = _strtoul(argv[argi++], NULL, 0);
            }
        }
        if((argi < argc) && (0 == _strcmp(argv[argi], "drop_pcd")))
        {
            argi++;
            act.trtcm_en = TRUE;
            act.trtcm.drop_pcd_sel = _strtoul(argv[argi++], NULL, 2);
            if(TRUE == act.trtcm.drop_pcd_sel)
            {
                if(0 == _strcmp(argv[argi], "red"))
                {
                    argi++;
                    act.trtcm.drop_pcd_r= _strtoul(argv[argi++], NULL, 0);
                }
                if(0 == _strcmp(argv[argi], "yellow"))
                {
                    argi++;
                    act.trtcm.drop_pcd_y= _strtoul(argv[argi++], NULL, 0);
                }
                if(0 == _strcmp(argv[argi], "green"))
                {
                    argi++;
                    act.trtcm.drop_pcd_g= _strtoul(argv[argi++], NULL, 0);
                }
            }
        }

        /* trTCM */
        if((argi < argc) && (0 == _strcmp(argv[argi], "color")))
        {
            argi++;
            act.trtcm_en = TRUE;
            act.trtcm.tcm_sel = _strtoul(argv[argi++], NULL, 2);
            trtcm = _strtoul(argv[argi++], NULL, 0);
            if(FALSE == act.trtcm.tcm_sel)
            {
                act.trtcm.usr_tcm = trtcm;
            }
            else
            {
                act.trtcm.tcm_idx = trtcm;
            }
        }
        ret = air_acl_setAction(0, act_idx, act);
        AIR_PRINT("Set ACL Action(%u): %s\n", act_idx, air_error_getString(ret));
    }
    else if(1 == argc)
    {
        /* acl get action <idx(0..127)> */
        act_idx = _strtoul(argv[argi++], NULL, 0);
        ret = air_acl_getAction(0, act_idx, &act);
        AIR_PRINT("Get ACL Action(%u): %s\n", act_idx, air_error_getString(ret));
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        if(TRUE == act.fwd_en)
        {
            AIR_PRINT("\t Forwarding           : %s\n", fwding[act.fwd]);
        }

        if(TRUE == act.egtag_en)
        {
            AIR_PRINT("\t Egress tag           : %s\n", egtag[act.egtag]);
        }

        if(act.mirrormap)
        {
            AIR_PRINT("\t Mirror Session Map   : %u\n", act.mirrormap);
        }

        if(TRUE == act.pri_user_en)
        {
            AIR_PRINT("\t User Priority        : %u\n", act.pri_user);
        }

        if(TRUE == act.port_en)
        {
            _hex2bitstr(act.portmap, str_temp, AIR_MAX_NUM_OF_PORTS+1);
            if(TRUE == act.dest_port_sel)
            {
                AIR_PRINT("\t Destination Port[0:6]: %s\n", str_temp);
            }
            else
            {
                AIR_PRINT("\t VLAN Port[0:6]       : %s\n", str_temp);
            }
        }

        if(TRUE == act.lyvlan_en)
        {
            AIR_PRINT("\t Leaky VLAN           : %s\n", (TRUE == act.lyvlan)?"Enable":"Disable");
        }
        AIR_PRINT("\t Management Frame     : %s\n", (TRUE == act.mang)?"Enable":"Disable");
        AIR_PRINT("\t BPDU Frame           : %s\n", (TRUE == act.bpdu)?"Enable":"Disable");

        if(TRUE == act.cnt_en)
        {
            AIR_PRINT("\t Event Index          : %u\n", act.cnt_idx);
        }

        /* trTCM*/
        if(TRUE == act.trtcm_en)
        {
            if(TRUE == act.trtcm.cls_slr_sel)
            {
                AIR_PRINT("\t Class Selector Remap : %u\n", act.trtcm.cls_slr);
            }
            else
            {
                AIR_PRINT("\t Class Selector Remap : %s\n", "Disable");
            }
            if(TRUE == act.trtcm.drop_pcd_sel)
            {
                AIR_PRINT("\t Drop Precedence Remap(Red): %u\n", act.trtcm.drop_pcd_r);
                AIR_PRINT("\t Drop Precedence Remap(Yel): %u\n", act.trtcm.drop_pcd_y);
                AIR_PRINT("\t Drop Precedence Remap(Gre): %u\n", act.trtcm.drop_pcd_g);
            }
            else
            {
                AIR_PRINT("\t Drop Precedence Remap: %s\n", "Disable");
            }

            if(TRUE == act.trtcm.tcm_sel)
            {
                AIR_PRINT("\t trTCM Meter Index    : %u\n", act.trtcm.tcm_idx);
            }
            else
            {
                AIR_PRINT("\t trTCM User Defined   : %s\n", trtcm_usr[act.trtcm.usr_tcm]);
            }
        }
        /* rate control */
        if(TRUE == act.rate_en)
        {
            AIR_PRINT("\t Rate Control Index   : %u\n", act.rate_idx);
        }

        if(TRUE == act.attack_en)
        {
            AIR_PRINT("\t Attack Rate Control Index: %u\n", act.attack_idx);
        }

        if(TRUE == act.vlan_en)
        {
            AIR_PRINT("\t ACL VLAN Index       : %u\n", act.vlan_idx);
        }
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclRmvAction(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T act_idx;

    if(1 == argc)
    {
        /* acl del action <idx(0..127)> */
        act_idx = _strtoul(argv[0], NULL, 0);
        ret = air_acl_delAction(0, act_idx);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Delete ACL Action(%u): %s\n", act_idx, air_error_getString(ret));
    }
    else if(0 == argc)
    {
        /* acl clear action */
        ret = air_acl_clearAction(0);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Clear ACL Action: %s\n", air_error_getString(ret));
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclTrtcm(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T argi = 0;
    UI32_T tcm_idx;
    AIR_ACL_TRTCM_T tcm;

    memset(&tcm, 0, sizeof(AIR_ACL_TRTCM_T));
    if(5 == argc)
    {
        /* acl set trtcm <idx(0..31)> <cir(4'hex)> <pir(4'hex)> <cbs(4'hex)> <pbs(4'hex)> */
        tcm_idx = _strtoul(argv[argi++], NULL, 0);
        tcm.cir = _strtoul(argv[argi++], NULL, 16);
        tcm.pir = _strtoul(argv[argi++], NULL, 16);
        tcm.cbs = _strtoul(argv[argi++], NULL, 16);
        tcm.pbs = _strtoul(argv[argi++], NULL, 16);

        ret = air_acl_setTrtcm(0, tcm_idx, tcm);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Set ACL trTCM(%u): %s\n", tcm_idx, air_error_getString(ret));
    }
    else if(1 == argc)
    {
        /* acl get trtcm <idx(1..31)> */
        tcm_idx = _strtoul(argv[argi++], NULL, 0);
        ret = air_acl_getTrtcm(0, tcm_idx, &tcm);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Get ACL trTCM(%u): %s\n", tcm_idx, air_error_getString(ret));
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        AIR_PRINT("\t CIR: 0x%04X(unit:64Kbps)\n", tcm.cir);
        AIR_PRINT("\t PIR: 0x%04X(unit:64Kbps)\n", tcm.pir);
        AIR_PRINT("\t CBS: 0x%04X(unit:Byte)\n", tcm.cbs);
        AIR_PRINT("\t PBS: 0x%04X(unit:Byte)\n", tcm.pbs);
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclTrtcmEn(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    BOOL_T state = FALSE;

    if (1 == argc)
    {
        /* acl set trtcmEn <en(1:En,0:Dis)> */
        state = _strtol(argv[0], NULL, 10);
        ret = air_acl_setTrtcmEnable(0, state);
        AIR_PRINT("Set trTCM State ");
    }
    else if (0 == argc)
    {
        /* acl get trtcmEn */
        ret = air_acl_getTrtcmEnable(0, &state);
        AIR_PRINT("Get trTCM State ");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if (ret == AIR_E_OK)
    {
        AIR_PRINT(": %s\n", (TRUE == state) ? "Enable" : "Disable");
    }
    else
    {
        AIR_PRINT("Fail!\n");
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclRmvTrtcm(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T tcm_idx;

    if(1 == argc)
    {
        /* acl del trtcm <idx(1..31)> */
        tcm_idx = _strtoul(argv[0], NULL, 0);
        ret = air_acl_delTrtcm(0, tcm_idx);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Delete ACL TRTCM(%u): %s\n", tcm_idx, air_error_getString(ret));
    }
    else if(0 == argc)
    {
        /* acl clear trtcm */
        ret = air_acl_clearTrtcm(0);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Clear ACL TRTCM: %s\n", air_error_getString(ret));
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclPortEn(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T argi=0;
    UI32_T port = 0;
    BOOL_T en;

    if(2 == argc)
    {
        /* acl set portEn <port(0..6)> <en(1:En,0:Dis)> */
        port = _strtoul(argv[argi++], NULL, 0);
        en = _strtoul(argv[argi++], NULL, 2);
        ret = air_acl_setPortEnable(0, port, en);
        AIR_PRINT("Set Port:%u ACL function ", port);
    }
    else if(1 == argc)
    {
        /* acl get portEn <port(0..6)> */
        port = _strtoul(argv[argi++], NULL, 0);
        ret = air_acl_getPortEnable(0, port, &en);
        AIR_PRINT("Get Port:%u ACL function ", port);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        AIR_PRINT(": %s\n", (TRUE == en)?"Enable":"Disable");
    }
    else
    {
        AIR_PRINT("Fail!\n");
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclDropEn(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T argi=0;
    UI32_T port = 0;
    BOOL_T en;

    if(2 == argc)
    {
        /* acl set dropEn <port(0..6)> <en(1:En,0:Dis)> */
        port = _strtoul(argv[argi++], NULL, 0);
        en = _strtoul(argv[argi++], NULL, 2);
        ret = air_acl_setDropEnable(0, port, en);
        AIR_PRINT("Set ACL Drop precedence ");
    }
    else if(1 == argc)
    {
        /* acl set dropEn <port(0..6)> */
        port = _strtoul(argv[argi++], NULL, 0);
        ret = air_acl_getDropEnable(0, port, &en);
        AIR_PRINT("Get ACL Drop precedence ");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        AIR_PRINT("(Port %u):%s\n",
                port,
                (TRUE == en)?"Enable":"Disable");
    }
    else
    {
        AIR_PRINT("Fail!\n");
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclDropThrsh(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T argi=0;
    UI32_T port = 0;
    AIR_ACL_DP_COLOR_T color;
    UI8_T queue;
    UI32_T high, low;
    C8_T dp_color[AIR_ACL_DP_COLOR_LAST][7] =
    {
        "Green",
        "Yellow",
        "Red"
    };

    if(5 == argc)
    {
        /* acl set dropThrsh <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)> <high(0..2047)> <low(0..2047) */
        port = _strtoul(argv[argi++], NULL, 0);
        color = _strtoul(argv[argi++], NULL, 0);
        queue = _strtoul(argv[argi++], NULL, 0);
        high = _strtoul(argv[argi++], NULL, 0);
        low = _strtoul(argv[argi++], NULL, 0);
        ret = air_acl_setDropThreshold(0, port, color, queue, high, low);
        AIR_PRINT("Set ACL Drop precedence ");
    }
    else if(3 == argc)
    {
        /* acl get dropThrsh <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)> */
        port = _strtoul(argv[argi++], NULL, 0);
        color = _strtoul(argv[argi++], NULL, 0);
        queue = _strtoul(argv[argi++], NULL, 0);
        ret = air_acl_getDropThreshold(0, port, color, queue, &high, &low);
        AIR_PRINT("Get ACL Drop precedence ");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        AIR_PRINT("(Port %u, color:%s, queue:%u):\n",
                port,
                dp_color[color],
                queue);
        AIR_PRINT("\tHigh Threshold :%u\n", high);
        AIR_PRINT("\tLow Threshold  :%u\n", low);
    }
    else
    {
        AIR_PRINT("Fail!\n");
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclDropPbb(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T argi=0;
    UI32_T port = 0;
    AIR_ACL_DP_COLOR_T color;
    UI8_T queue;
    UI32_T pbb;
    C8_T dp_color[AIR_ACL_DP_COLOR_LAST][7] =
    {
        "Green",
        "Yellow",
        "Red"
    };

    if(4 == argc)
    {
        /* acl set dropPbb <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)> <probability(0..1023) */
        port = _strtoul(argv[argi++], NULL, 0);
        color = _strtoul(argv[argi++], NULL, 0);
        queue = _strtoul(argv[argi++], NULL, 0);
        pbb = _strtoul(argv[argi++], NULL, 0);
        ret = air_acl_setDropProbability(0, port, color, queue, pbb);
        AIR_PRINT("Set ACL Drop precedence ");
    }
    else if(3 == argc)
    {
        /* acl get dropPbb <port(0..6)> <color(0:green,1:yellow,2:red)> <queue(0..7)> */
        port = _strtoul(argv[argi++], NULL, 0);
        color = _strtoul(argv[argi++], NULL, 0);
        queue = _strtoul(argv[argi++], NULL, 0);
        ret = air_acl_getDropProbability(0, port, color, queue, &pbb);
        AIR_PRINT("Get ACL Drop precedence ");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(ret == AIR_E_OK)
    {
        AIR_PRINT("(Port %u, color:%s, queue %u):\n",
                port,
                dp_color[color],
                queue);
        AIR_PRINT("\tDrop probability:%u(unit=1/1023)\n", pbb);
    }
    else
    {
        AIR_PRINT("Fail!\n");
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclMeter(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T argi = 0;
    UI32_T meter_idx, state, rate;

    if(3 == argc)
    {
        /* acl set meter <idx(0..31)> <en(1:En,0:Dis)> <rate(0..65535)>
           Note: Limit rate = rate * 64Kbps */
        meter_idx = _strtoul(argv[argi++], NULL, 0);
        state = _strtoul(argv[argi++], NULL, 2);
        rate = _strtoul(argv[argi++], NULL, 0);

        ret = air_acl_setMeterTable(0, meter_idx, state, rate);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Set ACL Meter(%u): %s\n", meter_idx, air_error_getString(ret));
    }
    else if(1 == argc)
    {
        /* acl get meter <idx(0..31)> */
        meter_idx = _strtoul(argv[argi++], NULL, 0);
        ret = air_acl_getMeterTable(0, meter_idx, &state, &rate);
        if(ret < AIR_E_LAST)
            AIR_PRINT("Get ACL Meter(%u): %s\n", meter_idx, air_error_getString(ret));
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK == ret)
    {
        AIR_PRINT("\t State: %s\n", (TRUE == state)?"Enable":"Disable");
        if(TRUE == state)
        {
            AIR_PRINT("\t Rate : %u(unit:64Kbps)\n", rate);
        }
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclDump(
    UI32_T argc,
    C8_T *argv[])
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T i, cnt = 0;
    AIR_ACL_CTRL_T ctrl;

    if(0 != argc)
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    for(i=0; i<ACL_MAX_RULE_NUM; i++)
    {
        memset(&ctrl, 0, sizeof(AIR_ACL_CTRL_T));
        ret = air_acl_getRuleCtrl(0, i, &ctrl);
        if(AIR_E_OK == ret)
        {
            if(TRUE == ctrl.rule_en)
            {
                cnt++;
                AIR_PRINT("\t Entry-%d vaild\n", i);
            }
        }
    }
    if(0 == cnt)
    {
        AIR_PRINT("\t No entry vaild\n");
    }

    return ret;
}

static AIR_ERROR_NO_T
doAclSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(aclSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doAclGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(aclGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doAclDel(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(aclDelCmds, argc, argv);
}

static AIR_ERROR_NO_T
doAclClear(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(aclClearCmds, argc, argv);
}

static AIR_ERROR_NO_T
doAcl(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(aclCmds, argc, argv);
}

static AIR_ERROR_NO_T
doIpmcAddMcast(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    AIR_IPMC_ENTRY_T mcst;
    UI32_T unit = 0;
    C8_T dip_str[CMD_IP_ADDR_STR_SIZE];

    /* ipmc add mcast <vid(0..4095)> <gaddr(IPADDR)> <portmap(7'bin)> [ disable-egrs-vlan-filter<1:En,0:Dis> ] */

    memset(&mcst, 0, sizeof(AIR_IPMC_ENTRY_T));
    memset(dip_str, 0, CMD_IP_ADDR_STR_SIZE);
    if((argc == 3) || (argc == 4))
    {
        mcst.vid = _strtoul(argv[0], NULL, 0);

        rc = _str2ipv4(argv[1], &(mcst.group_addr.ip_addr.ipv4_addr));
        if(rc == AIR_E_OK)
        {
            mcst.group_addr.ipv4 = TRUE;
        }
        else
        {
            rc = _str2ipv6(argv[1], mcst.group_addr.ip_addr.ipv6_addr);
            if(rc != AIR_E_OK)
            {
                AIR_PRINT("Unrecognized command.\n");
                return rc;
            }
        }
        mcst.port_bitmap[0] = _strtoul(argv[2], NULL, 2);
        if(argc == 4)
        {
            if(_strtol(argv[3], NULL, 10) == 1)
                mcst.flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
            else
                mcst.flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
        }
        rc = air_ipmc_addMcastAddr(unit, &mcst);
        _getIpAddrStr(&mcst.group_addr, dip_str);
        AIR_PRINT("Add Group: %s ", dip_str);
        if(rc == AIR_E_OK)
            AIR_PRINT(" Done.\n");
        else
            AIR_PRINT(" Fail.\n");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T
doIpmcAddMcastMem(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_IPMC_ENTRY_T mcst;
    UI32_T unit = 0;
    C8_T dip_str[CMD_IP_ADDR_STR_SIZE];

    /* ipmc add mcastMem <vid(0..4095)> <gaddr(IPADDR)> <portmap(7'bin)> */
    memset(&mcst, 0, sizeof(AIR_IPMC_ENTRY_T));
    memset(dip_str, 0, CMD_IP_ADDR_STR_SIZE);
    if(argc == 3)
    {
        mcst.vid = _strtoul(argv[0], NULL, 0);

        rc = _str2ipv4(argv[1], &(mcst.group_addr.ip_addr.ipv4_addr));
        if(rc == AIR_E_OK)
        {
            mcst.group_addr.ipv4 = TRUE;
        }
        else
        {
            rc = _str2ipv6(argv[1], mcst.group_addr.ip_addr.ipv6_addr);
            if(rc != AIR_E_OK)
            {
                AIR_PRINT("Unrecognized command.\n");
                return rc;
            }
        }
        mcst.port_bitmap[0] = _strtoul(argv[2], NULL, 2);
        rc = air_ipmc_addMcastMember(unit, &mcst);
        AIR_PRINT("Add group member");
        if(rc == AIR_E_OK)
            AIR_PRINT(" Done.\n");
        else if(rc == AIR_E_ENTRY_NOT_FOUND)
            AIR_PRINT(" Fail. Entry not found.\n");
        else
            AIR_PRINT(" Fail.\n");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T
doIpmcDelMcast(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_IPMC_ENTRY_T mcst;
    C8_T dip_str[CMD_IP_ADDR_STR_SIZE];
    UI32_T unit = 0;

    /* ipmc del mcast <vid(0..4095}> <gaddr(IPADDR)> */
    memset(&mcst, 0, sizeof(AIR_IPMC_ENTRY_T));
    memset(dip_str, 0, CMD_IP_ADDR_STR_SIZE);
    if(argc == 2)
    {
        mcst.vid = _strtoul(argv[0], NULL, 0);

        rc = _str2ipv4(argv[1], &(mcst.group_addr.ip_addr.ipv4_addr));
        if(rc == AIR_E_OK)
        {
            mcst.group_addr.ipv4 = TRUE;
        }
        else
        {
            rc = _str2ipv6(argv[1], mcst.group_addr.ip_addr.ipv6_addr);
            if(rc != AIR_E_OK)
            {
                AIR_PRINT("Unrecognized command.\n");
                return rc;
            }
        }
        rc = air_ipmc_delMcastAddr(unit, &mcst);
        _getIpAddrStr(&mcst.group_addr, dip_str);
        AIR_PRINT("Del group: %s", dip_str);
        if(rc == AIR_E_OK)
            AIR_PRINT(" Done.\n");
        else
            AIR_PRINT(" Fail.\n");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T
doIpmcDelMcastMem(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_IPMC_ENTRY_T mcst;
    UI32_T unit = 0;
    C8_T dip_str[CMD_IP_ADDR_STR_SIZE];

    /* ipmc del mcastMem <vid(0..4095)> <gaddr(IPADDR)> <portmap(7'bin)> */
    memset(&mcst, 0, sizeof(AIR_IPMC_ENTRY_T));
    memset(dip_str, 0, CMD_IP_ADDR_STR_SIZE);
    if(argc == 3)
    {
        mcst.vid = _strtoul(argv[0], NULL, 0);

        rc = _str2ipv4(argv[1], &(mcst.group_addr.ip_addr.ipv4_addr));
        if(rc == AIR_E_OK)
        {
            mcst.group_addr.ipv4 = TRUE;
        }
        else
        {
            rc = _str2ipv6(argv[1], mcst.group_addr.ip_addr.ipv6_addr);
            if(rc != AIR_E_OK)
            {
                AIR_PRINT("Unrecognized command.\n");
                return rc;
            }
        }
        mcst.port_bitmap[0] = _strtoul(argv[2], NULL, 2);
        rc = air_ipmc_delMcastMember(unit, &mcst);
        AIR_PRINT("Del group member");
        if(rc == AIR_E_OK)
            AIR_PRINT(" Done.\n");
        else if(rc == AIR_E_ENTRY_NOT_FOUND)
            AIR_PRINT(" Fail. Entry not found.\n");
        else
            AIR_PRINT(" Fail.\n");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_IPMC_MATCH_TYPE_T
_ipmc_cmd_getMatchType(
    AIR_IPMC_ENTRY_T *ptr_entry)
{
    if (TRUE == ptr_entry->group_addr.ipv4)
    {
        return AIR_IPMC_MATCH_TYPE_IPV4_GRP;
    }
    else
    {
        return AIR_IPMC_MATCH_TYPE_IPV6_GRP;
    }
}

static void
_ipmc_cmd_printTblHead(
    const AIR_IPMC_MATCH_TYPE_T match_type)
{
    const C8_T *_str_unit    = "unit";
    const C8_T *_str_vid     = "vid";
    const C8_T *_str_gaddr   = "group-address";
    const C8_T *_str_portlist= "portmap[0:6]";
    const C8_T *_str_memCnt  = "member-count";
    const C8_T *_str_na      = "---";
    const C8_T *_str_en      = "enable";
    const C8_T *_str_dis     = "disable";
    if (match_type < AIR_IPMC_MATCH_TYPE_IPV6_GRP)
    {
        AIR_PRINT("\n%5s %-5s%-16s%s\n",
                _str_unit, _str_vid, _str_gaddr, _str_portlist);
    }
    else
    {
        AIR_PRINT("\n%5s %-5s%-40s%s\n",
                _str_unit, _str_vid, _str_gaddr, _str_portlist);
    }
}

static void
_printIpmcMcast(
    const UI32_T unit,
    AIR_IPMC_ENTRY_T *ptr_entry,
    const AIR_IPMC_MATCH_TYPE_T match_type,
    UI8_T title)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    C8_T dip_str[CMD_IP_ADDR_STR_SIZE];
    C8_T str_temp[20];

    memset(dip_str, 0, CMD_IP_ADDR_STR_SIZE);
    memset(str_temp, 0, 20);

    if(title)
    {
        _ipmc_cmd_printTblHead(match_type);
    }
    _getIpAddrStr(&ptr_entry->group_addr, dip_str);
    _hex2bitstr(ptr_entry->port_bitmap[0], str_temp, AIR_MAX_NUM_OF_PORTS+1);
    switch (match_type)
    {
        case AIR_IPMC_MATCH_TYPE_IPV4_GRP:
            AIR_PRINT("%5u %-5u%-16s%s\n", unit, ptr_entry->vid, dip_str, str_temp);
            break;
        case AIR_IPMC_MATCH_TYPE_IPV6_GRP:
            AIR_PRINT("%5u %-5u%-40s%s\n", unit, ptr_entry->vid, dip_str, str_temp);
            break;
        default:
            AIR_PRINT("unknown match type:%u\n", match_type);
    }
}

static AIR_ERROR_NO_T
doIpmcGetMcast(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_IPMC_ENTRY_T entry;
    UI32_T unit = 0;
    AIR_IPMC_MATCH_TYPE_T match_type;
    C8_T dip_str[CMD_IP_ADDR_STR_SIZE];

    /* ipmc get mcastMem <vid(0..4095)> <gaddr(IPADDR)> */
    memset(&entry, 0, sizeof(AIR_IPMC_ENTRY_T));
    memset(dip_str, 0, CMD_IP_ADDR_STR_SIZE);
    if(argc == 2)
    {
        entry.vid = _strtoul(argv[0], NULL, 0);

        rc = _str2ipv4(argv[1], &(entry.group_addr.ip_addr.ipv4_addr));
        if(rc == AIR_E_OK)
        {
            entry.group_addr.ipv4 = TRUE;
        }
        else
        {
            rc = _str2ipv6(argv[1], entry.group_addr.ip_addr.ipv6_addr);
            if(rc != AIR_E_OK)
            {
                AIR_PRINT("Unrecognized command.\n");
                return rc;
            }
        }
        rc = air_ipmc_getMcastAddr(unit, &entry);
        if(rc == AIR_E_OK)
        {
            match_type = _ipmc_cmd_getMatchType(&entry);
            _printIpmcMcast(unit, &entry, match_type, 1);
        }
        else if(rc == AIR_E_ENTRY_NOT_FOUND)
            AIR_PRINT("Fail. Entry not found.\n");
        else
            AIR_PRINT("Fail.\n", rc);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T
doIpmcSearchMode(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T unit = 0, port = 0, mode_num = 0;
    BOOL_T mode = 0;

    if(argc == 1)
    {
        /* ipmc get searchMode <port(0..6)> */
        port = _strtoul(argv[0], NULL, 0);
        rc = air_ipmc_getPortIpmcMode(unit, port, &mode);
        if(rc == AIR_E_OK)
        {
            AIR_PRINT("Get Port%02d searchMode: %s\n", port, (TRUE == mode)? "ipmc": "l2mc");
        }
        else
        {
            AIR_PRINT("get searchMode fail(%u)\n", rc);
        }
    }
    else if(argc == 2)
    {
        /* ipmc set searchMode <port(0..6)> <mode(0:l2mc, 1:ipmc)> */
        port = _strtoul(argv[0], NULL, 0);
        mode_num = _strtoul(argv[1], NULL, 0);
        mode = (mode_num == 0) ? FALSE : TRUE;
        rc = air_ipmc_setPortIpmcMode(unit, port, mode);
        if(rc != AIR_E_OK)
        {
            AIR_PRINT("set searchMode fail(%u) on port(%u)\n", rc, port);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T
doIpmcAdd(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(ipmcAddCmds, argc, argv);
}

static AIR_ERROR_NO_T
doIpmcDel(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(ipmcDelCmds, argc, argv);
}

static AIR_ERROR_NO_T
doIpmcClear(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T unit = 0;

    if(argc == 0)
    {
        /* ipmc clear */
        rc = air_ipmc_delAllMcastAddr(unit);
        if(rc != AIR_E_OK)
        {
            AIR_PRINT("ipmc clear fail");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}

static AIR_ERROR_NO_T
doIpmcGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(ipmcGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doIpmcSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(ipmcSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doIpmcDump(
        UI32_T argc,
        C8_T *argv[])
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_IPMC_ENTRY_T *ptr_entry;
    UI32_T unit = 0, search_type = 0, set_num = 0, rt_cnt = 0, total_cnt = 0;
    UI32_T i = 0;
    AIR_IPMC_MATCH_TYPE_T match_type = AIR_IPMC_MATCH_TYPE_LAST;
    C8_T dip_str[CMD_IP_ADDR_STR_SIZE];

    /* ipmc dump searchType<0:dip4, 1:dip6> */
    memset(dip_str, 0, CMD_IP_ADDR_STR_SIZE);
    if(argc == 1)
    {
        search_type = _strtoul(argv[0], NULL, 0);
        if(search_type == 0)
            match_type = AIR_IPMC_MATCH_TYPE_IPV4_GRP;
        else if(search_type == 1)
            match_type = AIR_IPMC_MATCH_TYPE_IPV6_GRP;
        else
        {
            AIR_PRINT("Unrecognized command.\n");
            return AIR_E_BAD_PARAMETER;
        }

        /* get the bucket size */
        rc = air_ipmc_getMcastBucketSize(unit, &set_num);
        if (AIR_E_OK != rc)
        {
            AIR_PRINT("get bucket size fail\n");
            return rc;
        }
        ptr_entry = AIR_MALLOC(sizeof(AIR_IPMC_ENTRY_T) * set_num);
        if(ptr_entry == NULL)
        {
            AIR_PRINT("allocate memory fail\n");
            return AIR_E_NO_MEMORY;
        }

        memset(ptr_entry, 0, sizeof(AIR_IPMC_ENTRY_T) * set_num);
        rc = air_ipmc_getFirstMcastAddr(unit, match_type, &rt_cnt, ptr_entry);
        if(rc != AIR_E_OK)
        {
            if(rc == AIR_E_ENTRY_NOT_FOUND)
                AIR_PRINT("ipmc table(%s) is empty\n", match_type == AIR_IPMC_MATCH_TYPE_IPV4_GRP ? "dip4" : "dip6");
            else
                AIR_PRINT("dump IPMC table fail\n");

            AIR_FREE(ptr_entry);
            return rc;
        }
        total_cnt += rt_cnt;
        _printIpmcMcast(unit, &ptr_entry[0], match_type, 1);
        for(i = 1 ; i < rt_cnt; i++)
            _printIpmcMcast(unit, &ptr_entry[i], match_type, 0);

        while(rc == AIR_E_OK)
        {
            memset(ptr_entry, 0, sizeof(AIR_IPMC_ENTRY_T) * set_num);
            rt_cnt = 0;
            rc = air_ipmc_getNextMcastAddr(unit, match_type, &rt_cnt, ptr_entry);
            if(rc == AIR_E_ENTRY_NOT_FOUND)
                break;
            else if(rc != AIR_E_OK)
            {
                AIR_PRINT("dump mac table fail\n");
                break;
            }
            else
            {
                /* rc == AIR_E_OK */
                for(i = 0 ; i < rt_cnt ; i++)
                    _printIpmcMcast(unit, &ptr_entry[i], match_type, 0);
                total_cnt += rt_cnt;
            }
        }
        AIR_PRINT("\n total %u %s %s\n",
            total_cnt,
            match_type == AIR_IPMC_MATCH_TYPE_IPV4_GRP ? "dip4" : "dip6",
            (total_cnt>1)?"entries":"entry");
        if(ptr_entry != NULL)
            AIR_FREE(ptr_entry);
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T
doIpmc(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(ipmcCmds, argc, argv);
}

static AIR_ERROR_NO_T doLpdetClearStatus(UI32_T argc, C8_T *argv[])
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
    UI32_T                      unit = 0, port = 0, type = 0;
    AIR_SWC_LPDET_CTRL_TYPE_T   ctrl_type = AIR_SWC_LPDET_CTRL_TYPE_LAST;
    AIR_PORT_BITMAP_T           pbm = {0};

    if(2 == argc)
    {
        port = _strtoul(argv[0], NULL, 0);
        type = _strtoul(argv[1], NULL, 0);

        if (0 == type)
        {
            ctrl_type = AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME;
        }
        else if (1 == type)
        {
            ctrl_type = AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM;
        }
        else
        {
            AIR_PRINT("Unrecognized type.\n");
            return AIR_E_BAD_PARAMETER;
        }

        AIR_PORT_ADD(pbm, port);

        rc = air_lpdet_clearStatus(unit, ctrl_type, pbm);
        AIR_PRINT("clear port %u lpdet %s status", port, (type == 1) ? "rx-lp-alarm" : "tx-lp-frame");
        if(AIR_E_OK == rc)
        {
            AIR_PRINT("Done!\n");
        }
        else
        {
            AIR_PRINT("Fail!\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T doLpdetGetSrcMac(UI32_T argc, C8_T *argv[])
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          unit = 0;
    AIR_MAC_T       mac = {0};

    if(0 == argc)
    {
        rc = air_lpdet_getLdfSrcMac(unit, mac);
        AIR_PRINT("lpdet frame source mac address ");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(rc == AIR_E_OK)
    {
        AIR_PRINT(" : %02X-%02X-%02X-%02X-%02X-%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else
    {
        AIR_PRINT("Fail!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T doLpdetGetCtrl(UI32_T argc, C8_T *argv[])
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
    UI32_T                      unit = 0, port = 0, type = 0, mode_num = 0;
    BOOL_T                      enable = FALSE;

    if(1 == argc)
    {
        port = _strtoul(argv[0], NULL, 0);

        rc = air_lpdet_getCtrl(unit, port, AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME, &enable);
        if(AIR_E_OK == rc)
        {
            AIR_PRINT("Get port %u lpdet tx loop frame is %s\n", port, (enable == TRUE) ? "enable" : "disable");
        }
        else
        {
            AIR_PRINT("Get port %u lpdet tx loop frame fail!\n", port);
        }
        rc = air_lpdet_getCtrl(unit, port, AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM, &enable);
        if(AIR_E_OK == rc)
        {
            AIR_PRINT("Get port %u lpdet rx loop alarm is %s\n", port, (enable == TRUE) ? "enable" : "disable");
        }
        else
        {
            AIR_PRINT("Get port %u lpdet rx loop alarm fail!\n", port);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T doLpdetGetStatus(UI32_T argc, C8_T *argv[])
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
    UI32_T                      unit = 0, port = 0;
    AIR_SWC_LPDET_CTRL_TYPE_T   ctrl_type = AIR_SWC_LPDET_CTRL_TYPE_LAST;
    AIR_PORT_BITMAP_T           tx_pbm = {0}, rx_pbm = {0};

    if(1 == argc)
    {
        port = _strtoul(argv[0], NULL, 0);

        rc |= air_lpdet_getStatus(unit, AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME, tx_pbm);
        if(AIR_E_OK == rc)
        {
            AIR_PRINT("Get port %u lpdet tx loop frame is %s\n", port,
                        AIR_PORT_CHK(tx_pbm, port) ? "active" : "inactive");
        }
        else
        {
            AIR_PRINT("Get port %u lpdet tx loop frame fail!\n", port);
        }
        rc |= air_lpdet_getStatus(unit, AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM, rx_pbm);
        if(AIR_E_OK == rc)
        {
            AIR_PRINT("Get port %u lpdet rx loop alarm is %s\n", port,
                        AIR_PORT_CHK(rx_pbm, port) ? "loop" : "normal");
        }
        else
        {
            AIR_PRINT("Get port %u lpdet rx loop alarm fail!\n", port);
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T doLpdetSetSrcMac(UI32_T argc, C8_T *argv[])
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          unit = 0;
    AIR_MAC_T       mac = {0};

    if(1 == argc)
    {
        rc = _str2mac(argv[0], (C8_T *)mac);
        if(rc != AIR_E_OK)
        {
            AIR_PRINT("Unrecognized command.\n");
            return rc;
        }
        rc = air_lpdet_setLdfSrcMac(unit, mac);
        AIR_PRINT("Set lpdet frame source mac address to ");
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }

    if(rc == AIR_E_OK)
    {
        AIR_PRINT("%02X-%02X-%02X-%02X-%02X-%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    else
    {
        AIR_PRINT("Fail!\n");
    }
    return rc;
}

static AIR_ERROR_NO_T doLpdetSetCtrl(UI32_T argc, C8_T *argv[])
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
    UI32_T                      unit = 0, port = 0, type = 0, mode_num = 0;
    AIR_MAC_T                   mac = {0};
    BOOL_T                      enable = 0;
    AIR_SWC_LPDET_CTRL_TYPE_T   ctrl_type = AIR_SWC_LPDET_CTRL_TYPE_LAST;

    if(3 == argc)
    {
        port = _strtoul(argv[0], NULL, 0);
        type = _strtoul(argv[1], NULL, 0);
        mode_num = _strtoul(argv[2], NULL, 0);
        enable = (mode_num == 0) ? FALSE : TRUE;

        if (0 == type)
        {
            ctrl_type = AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME;
        }
        else if (1 == type)
        {
            ctrl_type = AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM;
        }
        else
        {
            AIR_PRINT("Unrecognized type.\n");
            return AIR_E_BAD_PARAMETER;
        }

        rc = air_lpdet_setCtrl(unit, port, ctrl_type, enable);
        AIR_PRINT("Set port %u lpdet %s control to %s ",
                    port, (type == 1) ? "rx-lp-alarm" : "tx-lp-frame", (enable == TRUE) ? "enable" : "disable");
        if(AIR_E_OK == rc)
        {
            AIR_PRINT("Done!\n");
        }
        else
        {
            AIR_PRINT("Fail!\n");
        }
    }
    else
    {
        AIR_PRINT("Unrecognized command.\n");
        return AIR_E_BAD_PARAMETER;
    }
    return rc;
}

static AIR_ERROR_NO_T
doLpdetClear(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(lpdetClearCmds, argc, argv);
}

static AIR_ERROR_NO_T
doLpdetGet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(lpdetGetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doLpdetSet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(lpdetSetCmds, argc, argv);
}

static AIR_ERROR_NO_T
doLpdet(
        UI32_T argc,
        C8_T *argv[])
{
    return subcmd(lpdetCmds, argc, argv);
}

static AIR_ERROR_NO_T
subcmd(
        const AIR_CMD_T tab[],
        UI32_T argc,
        C8_T *argv[])
{
    const AIR_CMD_T *cmdp;
    I32_T found = 0;
    I32_T i, len;

    if (argc < 1)
    {
        goto print_out_cmds;
    }

    for (cmdp = tab; cmdp->name != NULL; cmdp++)
    {
        if (strlen(argv[0]) == strlen(cmdp->name))
        {
            if (strncmp(argv[0], cmdp->name, strlen(argv[0])) == 0)
            {
                found = 1;
                break;
            }
        }
    }

    if(!found)
    {
        C8_T buf[66];

print_out_cmds:
        AIR_PRINT("valid subcommands:\n");
        memset(buf, ' ', sizeof(buf));
        buf[64] = '\n';
        buf[65] = '\0';

        for (i=0, cmdp = tab; cmdp->name != NULL; cmdp++)
        {
            len = strlen(cmdp->name);
            strncpy(&buf[i*16], cmdp->name, (len > 16) ? 16 : len);
            if(3 == i)
            {
                AIR_PRINT("%s\n", buf);
                memset(buf, ' ', sizeof(buf));
                buf[64] = '\n';
                buf[65] = '\0';
            }
            i = (i + 1) % 4;
        }

        if (0 != i)
            AIR_PRINT("%s\n", buf);

        return AIR_E_BAD_PARAMETER;
    }

    if (CMD_NO_PARA == cmdp->argc_min)
    {
        if (argc != 1)
        {
            if (cmdp->argc_errmsg != NULL)
            {
                AIR_PRINT("Usage: %s\n", cmdp->argc_errmsg);
            }

            return AIR_E_BAD_PARAMETER;
        }
    }
    else if (CMD_VARIABLE_PARA == cmdp->argc_min)
    {
        if (argc < 3)
        {
            if (cmdp->argc_errmsg != NULL)
            {
                AIR_PRINT("Usage: %s\n", cmdp->argc_errmsg);
            }

            return AIR_E_BAD_PARAMETER;
        }
    }
    else
    {
        if ((argc <= cmdp->argc_min) || ((cmdp->argc_min != 0) && (argc != (cmdp->argc_min + 1))))
        {
            if (cmdp->argc_errmsg != NULL)
            {
                AIR_PRINT("Usage: %s\n", cmdp->argc_errmsg);
            }

            return AIR_E_BAD_PARAMETER;
        }
    }

    if (cmdp->func)
    {
        argc--;
        argv++;
        return (*cmdp->func)(argc, argv);
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:   air_parse_cmd
 * PURPOSE:
 *      This function is used process diagnostic cmd
 * INPUT:
 *      argc         -- parameter number
 *      argv         -- parameter strings
 * OUTPUT:
 *      None
 * RETURN:
 *      NPS_E_OK     -- Successfully read the data.
 *      NPS_E_OTHERS -- Failed to read the data.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
air_parse_cmd(
        const UI32_T argc,
        const C8_T **argv)
{
    return subcmd(Cmds, argc, (C8_T **)argv);
}


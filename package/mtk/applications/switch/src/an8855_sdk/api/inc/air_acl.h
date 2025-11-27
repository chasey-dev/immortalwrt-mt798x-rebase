/* FILE NAME: air_acl.h
 * PURPOSE:
 *      Define the ACL function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_ACL_H
#define AIR_ACL_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/
#define ACL_MAX_BUSY_TIME            (20)
#define ACL_MAX_RULE_NUM             (128)
#define ACL_MAX_ACTION_NUM           (128)
#define ACL_MAX_TRTCM_NUM            (32)
#define ACL_MAX_MIB_NUM              (64)
#define ACL_MAX_UDF_NUM              (16)
#define ACL_MAX_METER_NUM            (32)
#define ACL_MAX_MIR_SESSION_NUM      (2)
#define ACL_MAX_TOKEN_NUM            (0xffff)
#define ACL_MAX_VLAN_NUM             (4096)
#define ACL_MAX_DROP_PCD_NUM         (8)
#define ACL_MAX_CLASS_SLR_NUM        (8)
#define ACL_MAX_ATTACK_RATE_NUM      (96)
#define ACL_MAX_WORD_OFST_NUM        (128)
#define ACL_MAX_CMP_PAT_NUM          (0xffff)
#define ACL_MAX_CMP_BIT_NUM          (0xffff)
#define ACL_MAX_CBS_NUM              (0xffff)
#define ACL_MAX_CIR_NUM              (0xffff)
#define ACL_MAX_PBS_NUM              (0xffff)
#define ACL_MAX_PIR_NUM              (0xffff)

#define ACL_EN_MASK                  (0x1)
#define ACL_MEM_CFG_DONE_OFFSET      (31)
#define ACL_MEM_CFG_RULE_ID_OFFSET   (16)
#define ACL_MEM_CFG_TCAM_CELL_OFFSET (12)
#define ACL_MEM_CFG_DATA_BN_OFFSET   (8)
#define ACL_MEM_CFG_MEM_SEL_OFFSET   (4)
#define ACL_MEM_CFG_FUNC_SEL_OFFSET  (1)
#define ACL_MEM_CFG_EN               (1)

#define ACL_MIB_SEL_OFFSET           (4)
#define ACL_MIB_CLEAR                (1)
#define ACL_MIB_SEL_MASK             (0x3f)
#define ACL_TRTCM_EN_OFFSET          (31)
#define ACL_TRTCM_BUSY_OFFSET        (31)
#define ACL_TRTCM_WRITE              (1U << 27)
#define ACL_TRTCM_READ               (0U << 27)
#define ACL_TRTCM_ID_OFFSET          (0)
#define ACL_TRTCM_ID_MASK            (0x1f)
#define ACL_TRTCM_CBS_MASK           (0xffff)
#define ACL_TRTCM_EBS_MASK           (0xffff)
#define ACL_TRTCM_CIR_MASK           (0xffff)
#define ACL_TRTCM_EIR_MASK           (0xffff)
#define ACL_RATE_BUSY_OFFSET         (31)
#define ACL_RATE_WRITE               (1U << 30)
#define ACL_RATE_READ                (0U << 30)
#define ACL_RATE_ID_OFFSET           (20)
#define ACL_RATE_ID_MASK             (0x1f)
#define ACL_RATE_EN_OFFSET           (19)
#define ACL_RATE_EN                  (1U << 19)
#define ACL_RATE_DIS                 (0U << 19)
#define ACL_RATE_TOKEN_MASK          (0xffff)

/*ACL rule field offset and width*/
#define RULE_TYPE0_OFFSET            (0)
#define DMAC_OFFSET                  (1)
#define SMAC_OFFSET                  (49)
#define STAG_OFFSET                  (97)
#define CTAG_OFFSET                  (113)
#define ETYPE_OFFSET                 (129)
#define DIP_OFFSET                   (145)
#define SIP_OFFSET                   (177)
#define DSCP_OFFSET                  (209)
#define PROTOCOL_OFFSET              (217)
#define DPORT_OFFSET                 (225)
#define SPORT_OFFSET                 (241)
#define UDF_OFFSET                   (257)
#define FIELDMAP_OFFSET              (273)
#define IS_IPV6_OFFSET               (286)
#define PORTMAP_OFFSET               (287)

#define RULE_TYPE1_OFFSET            (0)
#define DIP_IPV6_OFFSET              (1)
#define SIP_IPV6_OFFSET              (97)
#define FLOW_LABEL_OFFSET            (193)

#define RULE_TYPE0_WIDTH            (1)
#define DMAC_WIDTH                  (8)
#define SMAC_WIDTH                  (8)
#define STAG_WIDTH                  (16)
#define CTAG_WIDTH                  (16)
#define ETYPE_WIDTH                 (16)
#define DIP_WIDTH                   (32)
#define SIP_WIDTH                   (32)
#define DSCP_WIDTH                  (8)
#define PROTOCOL_WIDTH              (8)
#define DPORT_WIDTH                 (16)
#define SPORT_WIDTH                 (16)
#define UDF_WIDTH                   (16)
#define FIELDMAP_WIDTH              (13)
#define IS_IPV6_WIDTH               (1)
#define PORTMAP_WIDTH               (7)

#define RULE_TYPE1_WIDTH            (1)
#define DIP_IPV6_WIDTH              (32)
#define SIP_IPV6_WIDTH              (32)
#define FLOW_LABEL_WIDTH            (20)

/*ACL action offset and width*/
#define ACL_VLAN_VID_OFFSET         (0)
#define ACL_VLAN_HIT_OFFSET         (12)
#define ACL_CLASS_IDX_OFFSET        (13)
#define ACL_TCM_OFFSET              (18)
#define ACL_TCM_SEL_OFFSET          (20)
#define ACL_DROP_PCD_G_OFFSET       (21)
#define ACL_DROP_PCD_Y_OFFSET       (24)
#define ACL_DROP_PCD_R_OFFSET       (27)
#define CLASS_SLR_OFFSET            (30)
#define CLASS_SLR_SEL_OFFSET        (33)
#define DROP_PCD_SEL_OFFSET         (34)
#define TRTCM_EN_OFFSET             (35)
#define ACL_MANG_OFFSET             (36)
#define LKY_VLAN_OFFSET             (37)
#define LKY_VLAN_EN_OFFSET          (38)
#define EG_TAG_OFFSET               (39)
#define EG_TAG_EN_OFFSET            (42)
#define PRI_USER_OFFSET             (43)
#define PRI_USER_EN_OFFSET          (46)
#define MIRROR_OFFSET               (47)
#define FW_PORT_OFFSET              (49)
#define PORT_FW_EN_OFFSET           (52)
#define RATE_INDEX_OFFSET           (53)
#define RATE_EN_OFFSET              (58)
#define ATTACK_RATE_ID_OFFSET       (59)
#define ATTACK_RATE_EN_OFFSET       (66)
#define ACL_MIB_ID_OFFSET           (67)
#define ACL_MIB_EN_OFFSET           (73)
#define VLAN_PORT_SWAP_OFFSET       (74)
#define DST_PORT_SWAP_OFFSET        (75)
#define BPDU_OFFSET                 (76)
#define PORT_OFFSET                 (77)
#define PORT_FORCE_OFFSET           (84)

#define ACL_VLAN_VID_WIDTH          (12)
#define ACL_VLAN_HIT_WIDTH          (1)
#define ACL_CLASS_IDX_WIDTH         (5)
#define ACL_TCM_WIDTH               (2)
#define ACL_TCM_SEL_WIDTH           (1)
#define ACL_DROP_PCD_G_WIDTH        (3)
#define ACL_DROP_PCD_Y_WIDTH        (3)
#define ACL_DROP_PCD_R_WIDTH        (3)
#define CLASS_SLR_WIDTH             (3)
#define CLASS_SLR_SEL_WIDTH         (1)
#define DROP_PCD_SEL_WIDTH          (1)
#define TRTCM_EN_WIDTH              (1)
#define ACL_MANG_WIDTH              (1)
#define LKY_VLAN_WIDTH              (1)
#define LKY_VLAN_EN_WIDTH           (1)
#define EG_TAG_WIDTH                (3)
#define EG_TAG_EN_WIDTH             (1)
#define PRI_USER_WIDTH              (3)
#define PRI_USER_EN_WIDTH           (1)
#define MIRROR_WIDTH                (2)
#define FW_PORT_WIDTH               (3)
#define PORT_FW_EN_WIDTH            (1)
#define RATE_INDEX_WIDTH            (5)
#define RATE_EN_WIDTH               (1)
#define ATTACK_RATE_ID_WIDTH        (7)
#define ATTACK_RATE_EN_WIDTH        (1)
#define ACL_MIB_ID_WIDTH            (6)
#define ACL_MIB_EN_WIDTH            (1)
#define VLAN_PORT_SWAP_WIDTH        (1)
#define DST_PORT_SWAP_WIDTH         (1)
#define BPDU_WIDTH                  (1)
#define PORT_WIDTH                  (7)
#define PORT_FORCE_WIDTH            (1)

/*ACL UDF table offset and width*/
#define UDF_RULE_EN_OFFSET          (0)
#define UDF_PKT_TYPE_OFFSET         (1)
#define WORD_OFST_OFFSET            (4)
#define CMP_SEL_OFFSET              (11)
#define CMP_PAT_OFFSET              (32)
#define CMP_MASK_OFFSET             (48)
#define PORT_BITMAP_OFFSET          (64)

#define UDF_RULE_EN_WIDTH           (1)
#define UDF_PKT_TYPE_WIDTH          (3)
#define WORD_OFST_WIDTH             (7)
#define CMP_SEL_WIDTH               (1)
#define CMP_PAT_WIDTH               (16)
#define CMP_MASK_WIDTH              (16)
#define PORT_BITMAP_WIDTH           (29)

#define ACL_UDF_ACC_OFFSET          (31)
#define ACL_UDF_READ_MASK           (0x0)
#define ACL_UDF_WRITE_MASK          (0x1)
#define ACL_UDF_CLEAR_MASK          (0x2)
#define ACL_UDF_CMD_OFFSET          (28)
#define ACL_UDF_READ                (ACL_UDF_READ_MASK << ACL_UDF_CMD_OFFSET)
#define ACL_UDF_WRITE               (ACL_UDF_WRITE_MASK << ACL_UDF_CMD_OFFSET)
#define ACL_UDF_CLEAR               (ACL_UDF_CLEAR_MASK << ACL_UDF_CMD_OFFSET)
#define ACL_UDF_ADDR_MASK           (0xf)

#define DPCR_HIGH_THRSH_OFFSET      (11)
#define DPCR_PBB_OFFSET             (22)
#define DPCR_LOW_THRSH_WIDTH        (11)
#define DPCR_LOW_THRSH_MASK         (0x7ff)
#define DPCR_HIGH_THRSH_WIDTH       (11)
#define DPCR_HIGH_THRSH_MASK        (0x7ff)
#define DPCR_PBB_WIDTH              (10)
#define DPCR_PBB_MASK               (0x3ff)
#define DP_MFRM_EX_OFFSET           (24)

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef enum
{
    AIR_ACL_RULE_OFS_FMT_MAC_HEADER = 0,
    AIR_ACL_RULE_OFS_FMT_L2_PAYLOAD,
    AIR_ACL_RULE_OFS_FMT_IPV4_HEADER,
    AIR_ACL_RULE_OFS_FMT_IPV6_HEADER,
    AIR_ACL_RULE_OFS_FMT_L3_PAYLOAD,
    AIR_ACL_RULE_OFS_FMT_TCP_HEADER,
    AIR_ACL_RULE_OFS_FMT_UDP_HEADER,
    AIR_ACL_RULE_OFS_FMT_L4_PAYLOAD,
    AIR_ACL_RULE_OFS_FMT_LAST
} AIR_ACL_RULE_OFS_FMT_T;

typedef enum
{
    AIR_ACL_RULE_CMP_SEL_PATTERN,          /* Pattern hit by data pattern and bit mask */
    AIR_ACL_RULE_CMP_SEL_THRESHOLD,        /* Pattern hit by low and high threshold */
    AIR_ACL_RULE_CMP_SEL_LAST
}AIR_ACL_RULE_CMP_SEL_T;

typedef enum
{
    AIR_ACL_ACT_FWD_DIS,             /* Don't change forwarding behaviour by ACL */
    AIR_ACL_ACT_FWD_CPU_EX = 4,      /* Forward by system default & CPU port is excluded */
    AIR_ACL_ACT_FWD_CPU_IN,          /* Forward by system default & CPU port is included */
    AIR_ACL_ACT_FWD_CPU,             /* Forward to CPU port only */
    AIR_ACL_ACT_FWD_DROP,            /* Frame dropped */
    AIR_ACL_ACT_FWD_LAST
}AIR_ACL_ACT_FWD_T;

typedef enum
{
    AIR_ACL_ACT_EGTAG_DIS,
    AIR_ACL_ACT_EGTAG_CONSISTENT,
    AIR_ACL_ACT_EGTAG_UNTAG = 4,
    AIR_ACL_ACT_EGTAG_SWAP,
    AIR_ACL_ACT_EGTAG_TAG,
    AIR_ACL_ACT_EGTAG_STACK,
    AIR_ACL_ACT_EGTAG_LAST
}AIR_ACL_ACT_EGTAG_T;

typedef enum
{
    AIR_ACL_ACT_USR_TCM_DEFAULT,         /* Normal packets, don't work based on color */
    AIR_ACL_ACT_USR_TCM_GREEN,           /* Green */
    AIR_ACL_ACT_USR_TCM_YELLOW,          /* Yellow */
    AIR_ACL_ACT_USR_TCM_RED,             /* Red */
    AIR_ACL_ACT_USR_TCM_LAST
}AIR_ACL_ACT_USR_TCM_T;

typedef enum
{
    AIR_ACL_DP_COLOR_GREEN,
    AIR_ACL_DP_COLOR_YELLOW,
    AIR_ACL_DP_COLOR_RED,
    AIR_ACL_DP_COLOR_LAST
}AIR_ACL_DP_COLOR_T;

typedef enum
{
    AIR_ACL_RULE_TYPE_0,
    AIR_ACL_RULE_TYPE_1,
    AIR_ACL_RULE_TYPE_LAST
}AIR_ACL_RULE_TYPE_T;

typedef enum
{
    AIR_ACL_RULE_T_CELL,
    AIR_ACL_RULE_C_CELL,
    AIR_ACL_RULE_TCAM_LAST
}AIR_ACL_RULE_TCAM_T;

typedef enum
{
    AIR_ACL_MEM_SEL_RULE,
    AIR_ACL_MEM_SEL_ACTION,
    AIR_ACL_MEM_SEL_LAST
}AIR_ACL_MEM_SEL_T;

typedef enum
{
    AIR_ACL_MEM_FUNC_READ = 0,
    AIR_ACL_MEM_FUNC_WRITE,
    AIR_ACL_MEM_FUNC_CLEAR,
    AIR_ACL_MEM_FUNC_CONFIG_READ = 4,
    AIR_ACL_MEM_FUNC_CONFIG_WRITE,
    AIR_ACL_MEM_FUNC_LAST
}AIR_ACL_MEM_FUNC_T;

typedef enum
{
    AIR_ACL_RULE_CONFIG_ENABLE,
    AIR_ACL_RULE_CONFIG_END,
    AIR_ACL_RULE_CONFIG_REVERSE,
    AIR_ACL_RULE_CONFIG_LAST
}AIR_ACL_RULE_CONFIG_T;

typedef enum
{
    AIR_ACL_CHECK_ACL,
    AIR_ACL_CHECK_UDF,
    AIR_ACL_CHECK_TRTCM,
    AIR_ACL_CHECK_METER,
    AIR_ACL_CHECK_TYPE_LAST
}AIR_ACL_CHECK_TYPE_T;

typedef enum
{
    AIR_ACL_DMAC,
    AIR_ACL_SMAC,
    AIR_ACL_STAG,
    AIR_ACL_CTAG,
    AIR_ACL_ETYPE,
    AIR_ACL_DIP,
    AIR_ACL_SIP,
    AIR_ACL_DSCP,
    AIR_ACL_PROTOCOL,
    AIR_ACL_DPORT,
    AIR_ACL_SPORT,
    AIR_ACL_UDF,
    AIR_ACL_FLOW_LABEL,
    AIR_ACL_FIELD_TYPE_LAST
}AIR_ACL_FIELD_TYPE_T;

typedef struct AIR_ACL_FIELD_S
{
    UI8_T dmac[6];
    UI8_T smac[6];
    UI16_T stag;
    UI16_T ctag;
    UI16_T etype;
    UI32_T dip[4];
    UI32_T sip[4];
    UI8_T dscp;
    UI8_T protocol;
    UI32_T flow_label;
    UI16_T dport;
    UI16_T sport;
    UI16_T udf;
    BOOL_T isipv6;
    UI16_T fieldmap;
    UI32_T portmap;
} AIR_ACL_FIELD_T;

typedef struct AIR_ACL_CTRL_S
{
    BOOL_T rule_en;
    BOOL_T reverse;
    BOOL_T end;
}AIR_ACL_CTRL_T;

typedef struct AIR_ACL_RULE_S
{
    AIR_ACL_FIELD_T key;
    AIR_ACL_FIELD_T mask;
    AIR_ACL_CTRL_T ctrl;
} AIR_ACL_RULE_T;

typedef struct AIR_ACL_UDF_RULE_S
{
    BOOL_T valid;                           /* Valid bit */
    AIR_ACL_RULE_OFS_FMT_T offset_format;   /* Format Type for Word Offset Range */
    UI32_T offset;                          /* Word Offset */
    UI32_T portmap;                         /* Physical Source Port Bit-map */
    AIR_ACL_RULE_CMP_SEL_T cmp_sel;         /* Comparison mode selection */
    UI16_T pattern;                         /* Comparison Pattern when cmp_sel=AIR_ACL_RULE_CMP_SEL_PATTERN */
    UI16_T mask;                            /* Comparison Pattern Mask when cmp_sel=AIR_ACL_RULE_CMP_SEL_PATTERN */
    UI16_T low_threshold;                   /* Low Threshold when cmp_sel=AIR_ACL_RULE_CMP_SEL_THRESHOLD */
    UI16_T high_threshold;                  /* High Threshold when cmp_sel=AIR_ACL_RULE_CMP_SEL_THRESHOLD */
}AIR_ACL_UDF_RULE_T;

typedef struct AIR_ACL_ACT_TRTCM_S
{
    BOOL_T cls_slr_sel;                     /* FALSE: Select original class selector value
                                               TRUE:  Select ACL control table defined class selector value */
    UI8_T cls_slr;                          /* User defined class selector */
    BOOL_T drop_pcd_sel;                    /* FALSE: Select original drop precedence value
                                               TRUE:  Select ACL control table defined drop precedence value */
    UI8_T drop_pcd_r;                       /* User defined drop precedence for red packets */
    UI8_T drop_pcd_y;                       /* User defined drop precedence for yellow packets */
    UI8_T drop_pcd_g;                       /* User defined drop precedence for green packets */
    BOOL_T tcm_sel;                         /* FALSE: Select user defined color value
                                               TRUE:  Select color remark by trtcm table */
    AIR_ACL_ACT_USR_TCM_T usr_tcm;          /* User defined color remark */
    UI8_T tcm_idx;                          /* Index for the 32-entries trtcm table */
}AIR_ACL_ACT_TRTCM_T;

typedef struct AIR_ACL_ACTION_S
{
    BOOL_T port_en;
    BOOL_T dest_port_sel;               /* Swap destination port member by portmap when port_en=1 */
    BOOL_T vlan_port_sel;               /* Swap VLAN port member by portmap when port_en=1 */
    UI32_T portmap;

    BOOL_T cnt_en;
    UI32_T cnt_idx;                     /* Counter index */

    BOOL_T attack_en;
    UI32_T attack_idx;                  /* Attack rate index */

    BOOL_T rate_en;
    UI32_T rate_idx;                    /* Index of meter table */

    BOOL_T vlan_en;
    UI32_T vlan_idx;                    /* Vid from ACL */

    UI8_T mirrormap;                    /* mirror session bitmap */

    BOOL_T pri_user_en;
    UI8_T pri_user;                     /* User Priority from ACL */

    BOOL_T lyvlan_en;
    BOOL_T lyvlan;                      /* Leaky VLAN */

    BOOL_T mang;                        /* Management frame attribute */

    BOOL_T bpdu;                        /* BPDU frame attribute */

    BOOL_T fwd_en;
    AIR_ACL_ACT_FWD_T fwd;              /* Frame TO_CPU Forwarding */

    BOOL_T egtag_en;
    AIR_ACL_ACT_EGTAG_T egtag;          /* Egress tag control */

    BOOL_T trtcm_en;
    AIR_ACL_ACT_TRTCM_T trtcm;          /* TRTCM control */

}AIR_ACL_ACTION_T;

typedef struct AIR_ACL_TRTCM_S
{
    UI16_T cir;                             /* Committed information rate (unit: 64Kbps) */
    UI16_T pir;                             /* Peak information rate (unit: 64Kbps) */
    UI16_T cbs;                             /* Committed burst size (unit: byte) */
    UI16_T pbs;                             /* Peak burst size (unit: byte) */
}AIR_ACL_TRTCM_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: air_acl_setRuleCtrl
 * PURPOSE:
 *      Set ACL rule control.
 *
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 *      ptr_rule        --  Structure of ACL rule control
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setRuleCtrl(
    const UI32_T unit,
    const UI32_T rule_idx,
    AIR_ACL_CTRL_T *ptr_ctrl);

/* FUNCTION NAME: air_acl_getRuleCtrl
 * PURPOSE:
 *      Get ACL rule control.
 *
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 *
 * OUTPUT:
 *      ptr_ctrl        --  Structure of ACL rule control
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getRuleCtrl(
    const UI32_T unit,
    const UI32_T rule_idx,
    AIR_ACL_CTRL_T *ptr_ctrl);

/* FUNCTION NAME: air_acl_setRule
 * PURPOSE:
 *      Set ACL rule entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 *      rule            --  Structure of ACL rule entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setRule(
    const UI32_T unit,
    const UI32_T rule_idx,
    AIR_ACL_RULE_T *rule);

/* FUNCTION NAME: air_acl_delRule
 * PURPOSE:
 *      Delete an ACL rule entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_delRule(
    const UI32_T unit,
    const UI32_T rule_idx);

/* FUNCTION NAME: air_acl_clearRule
 * PURPOSE:
 *      Clear all ACL rule entries.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_clearRule(
    const UI32_T unit);

/* FUNCTION NAME: air_acl_getRule
 * PURPOSE:
 *      Get ACL rule entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 *
 * OUTPUT:
 *      ptr_rule        --  Structure of ACL rule entry
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getRule(
    const UI32_T unit,
    const UI32_T rule_idx,
    AIR_ACL_RULE_T *ptr_rule);

/* FUNCTION NAME: air_acl_setAction
 * PURPOSE:
 *      Set an ACL action entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      act_idx         --  Index of ACL action entry
 *      act             --  Structure of ACL action entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setAction(
    const UI32_T unit,
    const UI32_T act_idx,
    const AIR_ACL_ACTION_T act);

/* FUNCTION NAME: air_acl_delAction
 * PURPOSE:
 *      Delete an ACL action entry with specific index
 *
 * INPUT:
 *      unit            --  Device ID
 *      act_idx         --  Index of ACL action entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_delAction(
    const UI32_T unit,
    const UI32_T act_idx);

/* FUNCTION NAME: air_acl_clearAction
 * PURPOSE:
 *      Clear all ACL action entries.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_clearAction(
    const UI32_T unit);

/* FUNCTION NAME: air_acl_getAction
 * PURPOSE:
 *      Get an ACL action entry with speficic index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      act_idx         --  Index of ACL action entry
 *
 * OUTPUT:
 *      ptr_act         --  Structure of ACL action entry
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getAction(
    const UI32_T unit,
    const UI32_T act_idx,
    AIR_ACL_ACTION_T *ptr_act);

/* FUNCTION NAME: air_acl_setTrtcm
 * PURPOSE:
 *      Set a trTCM entry with the specific index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      tcm_idx         --  Index of trTCM entry
 *      tcm             --  Structure of trTCM entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      Index 0 ~ 31 can be selected in tcm_idx.
 */
AIR_ERROR_NO_T
air_acl_setTrtcm(
    const UI32_T unit,
    const UI32_T tcm_idx,
    const AIR_ACL_TRTCM_T tcm);

/* FUNCTION NAME: air_acl_delTrtcm
 * PURPOSE:
 *      Delete an ACL trTCM entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      trtcm_idx       --  Index of ACL trTCM entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      Index 0 ~ 31 can be selected in tcm_idx.
 */
AIR_ERROR_NO_T
air_acl_delTrtcm(
    const UI32_T unit,
    const UI32_T trtcm_idx);

/* FUNCTION NAME: air_acl_clearTrtcm
 * PURPOSE:
 *      Clear all ACL trTCM entries.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_clearTrtcm(
    const UI32_T unit);

/* FUNCTION NAME: air_acl_getTrtcm
 * PURPOSE:
 *      Get a trTCM entry with the specific index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      tcm_idx         --  Index of trTCM entry
 *
 * OUTPUT:
 *      ptr_tcm         --  Structure of trTCM entry
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      Index 0 ~ 31 can be selected in tcm_idx.
 */
AIR_ERROR_NO_T
air_acl_getTrtcm(
    const UI32_T unit,
    const UI32_T tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm);

/* FUNCTION NAME: air_acl_setTrtcmEnable
 * PURPOSE:
 *      Set trTCM enable so the meter table will be updated based on PIR and CIR.
 *      The color marker will also be enabled when ACL is hit.
 *
 * INPUT:
 *      unit            --  Device ID
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setTrtcmEnable(
    const UI32_T unit,
    const BOOL_T state);

/* FUNCTION NAME: air_acl_getTrtcm
 * PURPOSE:
 *      Get a trTCM enable state.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_state       --  FALSE:Disable
 *                          TRUE: Enable
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getTrtcmEnable(
    const UI32_T unit,
    BOOL_T *const ptr_state);

/* FUNCTION NAME: air_acl_setPortEnable
 * PURPOSE:
 *      Set ACL state for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setPortEnable(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state);

/* FUNCTION NAME: air_acl_getPortEnable
 * PURPOSE:
 *      Get ACL state for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_state       --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getPortEnable(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_state);

/* FUNCTION NAME: air_acl_setDropEnable
 * PURPOSE:
 *      Set ACL drop precedence state
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setDropEnable(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state);

/* FUNCTION NAME: air_acl_getDropEnable
 * PURPOSE:
 *      Get ACL drop precedence state
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_state       --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getDropEnable(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_state);

/* FUNCTION NAME: air_acl_setDropThreshold
 * PURPOSE:
 *      Set ACL drop threshold.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      color           --  AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *      high            --  High threshold
 *      low             --  Low threshold
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_setDropThreshold(
    const UI32_T unit,
    const UI32_T port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T queue,
    const UI32_T high,
    const UI32_T low);

/* FUNCTION NAME: air_acl_getDropThreshold
 * PURPOSE:
 *      Get ACL drop threshold.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      color           --  AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *
 * OUTPUT:
 *      ptr_high        --  High threshold
 *      ptr_low         --  Low threshold
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_getDropThreshold(
    const UI32_T unit,
    const UI32_T port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T queue,
    UI32_T *ptr_high,
    UI32_T *ptr_low);

/* FUNCTION NAME: air_acl_setDropProbability
 * PURPOSE:
 *      Set ACL drop probability.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      color           --  AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *      probability     --  Drop probability (value:0 ~ 1023, unit:1/1023; eg: value-102 = 10% )
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_setDropProbability(
    const UI32_T unit,
    const UI32_T port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T queue,
    const UI32_T probability);

/* FUNCTION NAME: air_acl_getDropProbability
 * PURPOSE:
 *      Get ACL drop probability.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      color           --  AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *
 * OUTPUT:
 *      ptr_probability --  Drop probability (value:0 ~ 1023, unit:1/1023; eg: value-102 = 10% )
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_getDropProbability(
    const UI32_T unit,
    const UI32_T port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T queue,
    UI32_T *ptr_probability);

/* FUNCTION NAME:
 *      air_acl_setGlobalState
 * PURPOSE:
 *      Set the ACL global enable state.
 * INPUT:
 *      unit        -- Device ID
 *      state       -- Enable state of ACL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setGlobalState(
    const UI32_T        unit,
    const BOOL_T        state);

/* FUNCTION NAME:
 *      air_acl_getGlobalState
 * PURPOSE:
 *      Get the ACL global enable state.
 * INPUT:
 *      unit             -- Device ID
 * OUTPUT:
 *      ptr_state        -- Enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getGlobalState(
    const UI32_T         unit,
    BOOL_T               *ptr_state);

/* FUNCTION NAME:
 *      air_acl_setUdfRule
 * PURPOSE:
 *      Set ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- Device ID
 *      rule_idx         -- ACLUDF table entry index
 *      udf_rule         -- Structure of ACL UDF rule entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setUdfRule(
    const UI32_T                unit,
    const UI32_T                rule_idx,
    AIR_ACL_UDF_RULE_T          udf_rule);

/* FUNCTION NAME:
 *      air_acl_getUdfRule
 * PURPOSE:
 *      Get ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- Device ID
 *      rule_idx         -- ACLUDF table entry index
 * OUTPUT:
 *      ptr_udf_rule     -- Structure of ACL UDF rule entry
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getUdfRule(
    const UI32_T                unit,
    const UI8_T                 rule_idx,
    AIR_ACL_UDF_RULE_T          *ptr_udf_rule);

/* FUNCTION NAME:
 *      air_acl_delUdfRule
 * PURPOSE:
 *      Delete ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- Device ID
 *      rule_idx         -- ACLUDF table entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_delUdfRule(
    const UI32_T      unit,
    const UI8_T       rule_idx);

/* FUNCTION NAME:
 *      air_acl_clearUdfRule
 * PURPOSE:
 *      Clear acl all udf rule.
 * INPUT:
 *      unit             -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_clearUdfRule(
    const UI32_T    unit);

/* FUNCTION NAME:
 *      air_acl_setMeterTable
 * PURPOSE:
 *      Set flow ingress rate limit by meter table.
 * INPUT:
 *      unit                -- Device ID
 *      meter_id            -- Meter id
 *      enable              -- Meter enable state
 *      rate                -- Ratelimit(unit:64kbps)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setMeterTable(
    const UI32_T            unit,
    const UI32_T            meter_id,
    const BOOL_T            enable,
    const UI32_T            rate);

/* FUNCTION NAME:
 *      air_acl_getMeterTable
 * PURPOSE:
 *      Get meter table configuration.
 * INPUT:
 *      unit                -- Device ID
 *      meter_id            -- Meter id
 * OUTPUT:
 *      ptr_enable          -- Meter enable state
 *      ptr_rate            -- Ratelimit(unit:64kbps)
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getMeterTable(
    const UI32_T            unit,
    const UI32_T            meter_id,
    BOOL_T                  *ptr_enable,
    UI32_T                  *ptr_rate);

#endif /* End of AIR_ACL_H */

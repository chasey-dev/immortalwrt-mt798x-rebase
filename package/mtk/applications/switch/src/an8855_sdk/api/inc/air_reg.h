/* FILE NAME:   air_reg.h
* PURPOSE:
*      Define the chip registers in AIR SDK.
* NOTES:
*/

#ifndef AIR_REG_H
#define AIR_REG_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/
#define PORT_CTRL_PORT_OFFSET               (0x200)

/* SYS SCU */
#define RST_CTRL1                           (0x100050c0)
#define SYS_SW_RST_OFFT                     (31)

/* ARL Register Base */
#define REG_ARL_BASE_ADDRESS                (0x10200000)

#define AGC                                 (0x000C)
/* fields of AGC */
#define AGC_TICK_SEL                        (0x1 << 30)

#define MFC                                 (REG_ARL_BASE_ADDRESS + 0x0010)
/* fields of MFC */


#define VTCR_ADDR                           (0x0090)
#define VTCR_VID_OFFT                       (0)
#define VTCR_VID_LENG                       (12)
#define VTCR_VID_RELMASK                    (0x00000FFF)
#define VTCR_VID_MASK                       (VTCR_VID_RELMASK << VTCR_VID_OFFT)
#define VTCR_FUNC_OFFT                      (12)
#define VTCR_FUNC_LENG                      (4)
#define VTCR_FUNC_RELMASK                   (0x0000000F)
#define VTCR_FUNC_MASK                      (VTCR_FUNC_RELMASK << VTCR_FUNC_OFFT)
#define VTCR_FUNC_READ_ACL_RULE             (0x4 << VTCR_FUNC_OFFT)
#define VTCR_FUNC_WRITE_ACL_RULE            (0x5 << VTCR_FUNC_OFFT)
#define VTCR_FUNC_READ_TRTCM                (0x6 << VTCR_FUNC_OFFT)
#define VTCR_FUNC_WRITE_TRTCM               (0x7 << VTCR_FUNC_OFFT)
#define VTCR_FUNC_READ_ACL_MASK             (0x8 << VTCR_FUNC_OFFT)
#define VTCR_FUNC_WRITE_ACL_MASK            (0x9 << VTCR_FUNC_OFFT)
#define VTCR_FUNC_READ_ACL_RULE_CTRL        (0xA << VTCR_FUNC_OFFT)
#define VTCR_FUNC_WRITE_ACL_RULE_CTRL       (0xB << VTCR_FUNC_OFFT)
#define VTCR_FUNC_READ_ACL_RATE_CTRL        (0xC << VTCR_FUNC_OFFT)
#define VTCR_FUNC_WRITE_ACL_RATE_CTRL       (0xD << VTCR_FUNC_OFFT)
#define VTCR_IDX_INVLD_OFFT                 (16)
#define VTCR_IDX_INVLD_RELMASK              (0x00000001)
#define VTCR_IDX_INVLD_MASK                 (VTCR_IDX_INVLD_RELMASK << VTCR_IDX_INVLD_OFFT)
#define VTCR_BUSY_OFFT                      (31)
#define VTCR_BUSY_LENG                      (1)
#define VTCR_BUSY_RELMASK                   (0x00000001)
#define VTCR_BUSY_MASK                      (VTCR_BUSY_RELMASK << VTCR_BUSY_OFFT)

#define PIM_DSCP(d)                         (0x0058 + (4 * (d / 10)))

#define UNUF                                (0x102000B4)
#define UNMF                                (0x102000B8)
#define BCF                                 (0x102000BC)
#define QRYP                                (0x102000D8)

#define VAWD1_ADDR                          (0x0094)
#define VAWD2_ADDR                          (0x0098)
#define VAWD3_ADDR                          (0x00E0)
#define VAWD4_ADDR                          (0x00E4)
#define VAWD5_ADDR                          (0x00E8)
#define VAWD6_ADDR                          (0x00EC)
#define VAWD7_ADDR                          (0x00F0)
#define VAWD8_ADDR                          (0x00F4)
#define VAWD_ADDR(n)                        ((n<2)?(VAWD1_ADDR+(0x4*n)):(VAWD3_ADDR+(0x4*(n-2))))

#define SCH_CTRL_BASE                       (0x1000)
#define SCH_CTRL_PORT_OFFSET                (0x100)
#define MMSCR(p)                            (SCH_CTRL_BASE + (p * SCH_CTRL_PORT_OFFSET) + 0x90)
#define MMSCR0(p, q)                        (SCH_CTRL_BASE + (p * SCH_CTRL_PORT_OFFSET) + (8 * q))
#define MMSCR1(p, q)                        (SCH_CTRL_BASE + (p * SCH_CTRL_PORT_OFFSET) + (8 * q) + 0x04)
#define MMSCR2(p, q)                        (SCH_CTRL_BASE + (p * SCH_CTRL_PORT_OFFSET) + (8 * q) + 0x50)
#define MMSCR3(p, q)                        (SCH_CTRL_BASE + (p * SCH_CTRL_PORT_OFFSET) + (8 * q) + 0x54)

/* fields of GERLCR */
#define EGC_MFRM_EX_OFFSET                  (9)
#define EGC_MFRM_EX_LENGTH                  (1)

#define BMU_CTRL_BASE                       (0x1800)
#define BMU_CTRL_PORT_OFFSET                (0x100)

/* fields of MMDPR */
#define MMDPR_BASE                          (BMU_CTRL_BASE + 0xC)
#define MMDPR_PORT_OFFSET                   (0x100)
#define MMDPR_COLOR_OFFSET                  (0x20)
#define MMDPR_QUEUE_OFFSET                  (0x4)
#define MMDPR(p,c,q)                        (MMDPR_BASE + (p * MMDPR_PORT_OFFSET) + (c * MMDPR_COLOR_OFFSET) + (q * MMDPR_QUEUE_OFFSET))
#define MMDPR_EN                            (1 << 31)
#define MMDPR_PR_OFFSET                     (24)
#define MMDPR_PR_LENGTH                     (3)
#define MMDPR_HT_OFFSET                     (12)
#define MMDPR_HT_LENGTH                     (9)
#define MMDPR_LT_OFFSET                     (0)
#define MMDPR_LT_LENGTH                     (9)

/* fields of GIRLCR */
#define IGC_MFRM_EX_OFFSET                  (9)
#define IGC_MFRM_EX_LENGTH                  (1)

#define PORT_CTRL_BASE                      (0x10208000)
#define PORT_CTRL_REG(p, r)                 (PORT_CTRL_BASE + (p) * PORT_CTRL_PORT_OFFSET + (r))

#define PORTMATRIX(p)                       PORT_CTRL_REG(p, 0x44)

#define PCR(p)                              PORT_CTRL_REG(p, 0x04)
#define PCR_PORT_VLAN_OFFT                  (0)
#define PCR_PORT_VLAN_LENG                  (2)
#define PCR_PORT_VLAN_RELMASK               (0x00000003)
#define PCR_PORT_VLAN_MASK                  (PCR_PORT_VLAN_RELMASK << PCR_PORT_VLAN_OFFT)
#define PCR_PORT_RX_MIR_OFFT                (16)
#define PCR_PORT_RX_MIR_LENG                (2)
#define PCR_PORT_TX_MIR_OFFT                (20)
#define PCR_PORT_TX_MIR_LENG                (2)
#define PCR_PORT_PRI_OFFT                   (24)
#define PCR_PORT_PRI_LENG                   (3)
#define PCR_PORT_PRI_RELMASK                (0x00000007)
#define PCR_PORT_PRI_MASK                   (PCR_PORT_PRI_RELMASK << PCR_PORT_PRI_OFFT)
#define PCR_PORT_ACL_MIS_FWD_OFFT           (4)
#define PCR_PORT_ACL_MIS_FWD_LENG           (3)
#define PCR_PORT_ACL_MIS_FWD_RELMASK        (0x00000007)
#define PCR_PORT_ACL_MIS_FWD_MASK           (PCR_PORT_ACL_MIS_FWD_RELMASK << PCR_PORT_ACL_MIS_FWD_OFFT)
#define PCR_EG_TAG_OFFT                     (28)
#define PCR_EG_TAG_LENG                     (2)
#define PCR_EG_TAG_RELMASK                  (0x00000003)
#define PCR_EG_TAG_MASK                     (PCR_EG_TAG_RELMASK << PCR_EG_TAG_OFFT)
#define PCR_RMK_DSCP_EN                     (0x1000)
#define PCR_RMK_1Q_EN                       (0x0800)

#define PVC(p)                              PORT_CTRL_REG(p, 0x10)
#define PVC_ACC_FRM_OFFT                    (0)
#define PVC_ACC_FRM_LENG                    (2)
#define PVC_ACC_FRM_RELMASK                 (0x00000003)
#define PVC_ACC_FRM_MASK                    (PVC_ACC_FRM_RELMASK << PVC_ACC_FRM_OFFT)
#define PVC_UC_LKYV_EN_OFFT                 (2)
#define PVC_UC_LKYV_EN_LENG                 (1)
#define PVC_UC_LKYV_EN_RELMASK              (0x00000001)
#define PVC_UC_LKYV_EN_MASK                 (PVC_UC_LKYV_EN_RELMASK << PVC_UC_LKYV_EN_OFFT)
#define PVC_MC_LKYV_EN_OFFT                 (3)
#define PVC_MC_LKYV_EN_LENG                 (1)
#define PVC_MC_LKYV_EN_RELMASK              (0x00000001)
#define PVC_MC_LKYV_EN_MASK                 (PVC_MC_LKYV_EN_RELMASK << PVC_MC_LKYV_EN_OFFT)
#define PVC_BC_LKYV_EN_OFFT                 (4)
#define PVC_BC_LKYV_EN_LENG                 (1)
#define PVC_BC_LKYV_EN_RELMASK              (0x00000001)
#define PVC_BC_LKYV_EN_MASK                 (PVC_BC_LKYV_EN_RELMASK << PVC_BC_LKYV_EN_OFFT)
#define PVC_SPTAG_EN_OFFT                   (5)
#define PVC_SPTAG_EN_LENG                   (1)
#define PVC_SPTAG_EN_RELMASK                (0x00000001)
#define PVC_SPTAG_EN_MASK                   (PVC_SPTAG_EN_RELMASK << PVC_SPTAG_EN_OFFT)
#define PVC_VLAN_ATTR_OFFT                  (6)
#define PVC_VLAN_ATTR_LENG                  (2)
#define PVC_VLAN_ATTR_RELMASK               (0x00000003)
#define PVC_VLAN_ATTR_MASK                  (PVC_VLAN_ATTR_RELMASK << PVC_VLAN_ATTR_OFFT)
#define PVC_EG_TAG_OFFT                     (8)
#define PVC_EG_TAG_LENG                     (3)
#define PVC_EG_TAG_RELMASK                  (0x00000007)
#define PVC_EG_TAG_MASK                     (PVC_EG_TAG_RELMASK << PVC_EG_TAG_OFFT)
#define PVC_SPTAG_MODE_OFFT                 (11)
#define PVC_SPTAG_MODE_LENG                 (1)
#define PVC_STAG_VPID_OFFT                  (16)
#define PVC_STAG_VPID_LENG                  (16)
#define PVC_STAG_VPID_RELMASK               (0x0000FFFF)
#define PVC_STAG_VPID_MASK                  (PVC_STAG_VPID_RELMASK << PVC_STAG_VPID_OFFT)

#define PPBV1(p)                            PORT_CTRL_REG(p, 0x14)
#define PPBV1_G0_PORT_VID_OFFT              (0)
#define PPBV1_G0_PORT_VID_LENG              (12)
#define PPBV1_G0_PORT_VID_RELMASK           (0x00000FFF)
#define PPBV1_G0_PORT_VID_MASK              (PPBV1_G0_PORT_VID_RELMASK << PPBV1_G0_PORT_VID_OFFT)

#define PVID(p)                             PORT_CTRL_REG(p, 0x48)
#define PVID_PCVID_OFFT                     (0)
#define PVID_PCVID_LENG                     (12)
#define PVID_PCVID_RELMASK                  (0x00000FFF)
#define PVID_PCVID_MASK                     (PVID_PCVID_RELMASK << PVID_PCVID_OFFT)

#define BSR(p)                              PORT_CTRL_REG(p, 0x50)
#define BSR1(p)                             PORT_CTRL_REG(p, 0x54)
#define BSR_EXT1(p)                         PORT_CTRL_REG(p, 0x58)
#define BSR1_EXT1(p)                        PORT_CTRL_REG(p, 0x5C)
#define BSR_EXT2(p)                         PORT_CTRL_REG(p, 0x60)
#define BSR1_EXT2(p)                        PORT_CTRL_REG(p, 0x64)
#define BSR_EXT3(p)                         PORT_CTRL_REG(p, 0x68)
#define BSR1_EXT3(p)                        PORT_CTRL_REG(p, 0x6C)
#define BSR_STORM_COUNT_MSK                 (0xFF)
#define BSR_STORM_UNIT_MSK                  (7)
#define BSR_STORM_UNIT_OFFT                 (8)
#define BSR_STORM_RATE_BASED                (0x1)
#define BSR_STORM_DROP_EN                   (0x10)
#define BSR_STORM_BCST_EN                   (0x2)
#define BSR_STORM_MCST_EN                   (0x4)
#define BSR_STORM_UCST_EN                   (0x8)
#define BSR1_10M_COUNT_OFFT                 (0)
#define BSR1_100M_COUNT_OFFT                (8)
#define BSR1_1000M_COUNT_OFFT               (16)
#define BSR1_2500M_COUNT_OFFT               (24)

#define PEM(p, q)                           (PORT_CTRL_REG(p, (0x44 + (4 * (q/2)))))

#define PORT_MAC_CTRL_BASE                  (0x10210000)
#define PORT_MAC_CTRL_PORT_OFFSET           (0x200)
#define PORT_MAC_CTRL_REG(p, r)             (PORT_MAC_CTRL_BASE + (p) * PORT_MAC_CTRL_PORT_OFFSET + (r))

#define PMCR(p)                             PORT_MAC_CTRL_REG(p, 0x00)

#define ARL_CTRL_BASE                       (0x0000)
#define CFC                                 (0x0004)
/* fields of CFC */
#define CFC_MIRROR_EN_OFFSET                (19)
#define CFC_MIRROR_EN_LEN                   (1)
#define CFC_MIRROR_PORT_OFFSET              (16)
#define CFC_MIRROR_PORT_LEN                 (3)
#define MFC_CPU_PORT_OFFSET                 (8)
#define MFC_CPU_PORT_LENGTH                 (5)
#define MFC_CPU_EN_OFFSET                   (15)
#define MFC_CPU_EN_LENGTH                   (1)


#define ISC                                 (0x0018)
#define TSRA1                               (0x0084)
#define TSRA2                               (0x0088)
#define ATRD                                (0x008C)
#define CPGC                                (0x00B0)

#define PSC(p)                              PORT_CTRL_REG(p, 0xC)
#define PSC_DIS_LRN_OFFSET                  (4)
#define PSC_DIS_LRN_LENGTH                  (1)
#define PSC_SA_CNT_EN_OFFSET                (5)
#define PSC_SA_CNT_EN_LENGTH                (1)
#define PSC_SA_CNT_LMT_OFFSET               (8)
#define PSC_SA_CNT_LMT_LENGTH               (12)
#define PSC_SA_CNT_LMT_REALMASK             (0x00000FFF)
#define PSC_SA_CNT_LMT_MASK                 (PSC_SA_CNT_LMT_REALMASK << PSC_SA_CNT_LMT_OFFSET)
#define PSC_SA_CNT_LMT_MAX                  (0x800)

/* fields of CPGC */
#define COL_EN                              (0x01)
#define COL_CLK_EN                          (0x02)

#define CKGCR                               (0x10213E1C)
#define CKG_LNKDN_GLB_STOP                  (0x01)
#define CKG_LNKDN_PORT_STOP                 (0x02)

/* fields of TRTCM*/
#define TRTCM                               (ARL_CTRL_BASE + 0x009C)
#define TRTCM_EN                            (1 << 31)

#define ARL_TRUNK_BASE                      (0x10200000)
#define PTC                                 (ARL_TRUNK_BASE + 0x400)
#define PTSEED                              (ARL_TRUNK_BASE + 0x404)
#define PTGC                                (ARL_TRUNK_BASE + 0x408)
#define PTG(g)                              (ARL_TRUNK_BASE + 0x40C + ((g) * 0x8 ))
#define PTGRSTS                             (ARL_TRUNK_BASE + 0x44C)

#define MIR                                 (REG_ARL_BASE_ADDRESS + (0xCC))
/* fields of MIR */
#define MIR_MIRROR_BASE_OFFSER              (8)
#define MIR_MIRROR_EN_OFFSER(p)             ((p) * MIR_MIRROR_BASE_OFFSER + 0x07)
#define MIR_MIRROR_EN_LEN                   (1)
#define MIR_MIRROR_PORT_OFFSER(p)           ((p) * MIR_MIRROR_BASE_OFFSER + 0x00)
#define MIR_MIRROR_PORT_LEN                 (5)
#define MIR_MIRROR_TAG_TX_EN_OFFSER(p)      ((p) * MIR_MIRROR_BASE_OFFSER + 0x06)
#define MIR_MIRROR_TAG_TX_EN_LEN            (1)

/* fields of ATA1 */
#define ATA1                                (REG_ARL_BASE_ADDRESS + 0x0304)
#define ATA1_MAC_ADDR_MSB_OFFSET            (0)
#define ATA1_MAC_ADDR_MSB_LENGTH            (32)
//#define ATA1_SAT_ADDR_OFFSET                (0)
//#define ATA1_SAT_ADDR_LENGTH                (11)
//#define ATA1_SAT_BANK_OFFSET                (16)
//#define ATA1_SAT_BANK_LENGTH                (4)

/* fields of ATA2 */
#define ATA2                                (REG_ARL_BASE_ADDRESS + 0x0308)
#define ATA2_MAC_AGETIME_OFFSET             (0)
#define ATA2_MAC_AGETIME_LENGTH             (9)
#define ATA2_MAC_LIFETIME_OFFSET            (9)
#define ATA2_MAC_LIFETIME_LENGTH            (1)
#define ATA2_MAC_UNAUTH_OFFSET              (10)
#define ATA2_MAC_UNAUTH_LENGTH              (1)
#define ATA2_MAC_ADDR_LSB_OFFSET            (16)
#define ATA2_MAC_ADDR_LSB_LENGTH            (16)

/* fields of ATA3 */
#define ATA3                                (REG_ARL_BASE_ADDRESS + 0x030C)

/* fields of ATA4 */
#define ATA4                                (REG_ARL_BASE_ADDRESS + 0x0310)

/* fields of ATA5 */
#define ATA5                                (REG_ARL_BASE_ADDRESS + 0x0314)

/* fields of ATA6 */
#define ATA6                                (REG_ARL_BASE_ADDRESS + 0x0318)

/* fields of ATA7 */
#define ATA7                                (REG_ARL_BASE_ADDRESS + 0x031C)

/* fields of ATA8 */
#define ATA8                                (REG_ARL_BASE_ADDRESS + 0x0320)

/* fields of ATWD */
#define ATWD                                (REG_ARL_BASE_ADDRESS + 0x0324)
#define ATWD_MAC_LIVE_OFFSET                (0)
#define ATWD_MAC_LIVE_LENGTH                (1)
#define ATWD_MAC_LEAK_OFFSET                (1)
#define ATWD_MAC_LEAK_LENGTH                (1)
#define ATWD_MAC_UPRI_OFFSET                (2)
#define ATWD_MAC_UPRI_LENGTH                (3)
#define ATWD_MAC_FWD_OFFSET                 (5)
#define ATWD_MAC_FWD_LENGTH                 (3)
#define ATWD_MAC_MIR_OFFSET                 (8)
#define ATWD_MAC_MIR_LENGTH                 (2)
#define ATWD_MAC_ETAG_OFFSET                (12)
#define ATWD_MAC_ETAG_LENGTH                (3)
#define ATWD_MAC_IVL_OFFSET                 (15)
#define ATWD_MAC_IVL_LENGTH                 (1)
#define ATWD_MAC_VID_OFFSET                 (16)
#define ATWD_MAC_VID_LENGTH                 (12)
#define ATWD_MAC_FID_OFFSET                 (28)
#define ATWD_MAC_FID_LENGTH                 (4)

/* fields of MAC and Multicast */
#define ATWD_IPM_VLD_OFFSET                 (0)   /* bit[0]     */
#define ATWD_IPM_LEAKY_OFFSET               (1)   /* bit[1]     */
#define ATWD_IPM_UPRI_OFFSET                (2)   /* bit[4:2]   */
#define ATWD_IPM_IPV6_OFFSET                (5)   /* bit[5]     */
#define ATWD_IPM_EG_TAG_OFFSET              (12)  /* bit[14:12] */
#define ATWD_IPM_VID_OFFSET                 (16)  /* bit[27:16] */

#define ATWD_IPM_VLD_RANGE                  (1)
#define ATWD_IPM_LEAKY_RANGE                (1)
#define ATWD_IPM_UPRI_RANGE                 (3)
#define ATWD_IPM_IPV6_RANGE                 (1)
#define ATWD_IPM_EG_TAG_RANGE               (3)
#define ATWD_IPM_VID_RANGE                  (12)

#define ATRD0_IPM_LEAKY_OFFSET              (5)  /* bit[5] */
#define ATRD0_IPM_LEAKY_RANGE               (1)
#define ATRD0_IPM_VID_OFFSET                (10) /* bit[21:10] */
#define ATRD0_IPM_VID_RANGE                 (12)

/* fields of ATWD2 */
#define ATWD2                               (REG_ARL_BASE_ADDRESS + 0x0328)
#define ATWD2_MAC_PORT_OFFSET               (0)
#define ATWD2_MAC_PORT_LENGTH               (8)

/* fields of ATC */
#define ATC                                 (REG_ARL_BASE_ADDRESS + 0x0300)
#define ATC_MAC_OFFSET                      (0)
#define ATC_MAC_LENGTH                      (3)
#define ATC_SAT_OFFSET                      (4)
#define ATC_SAT_LENGTH                      (2)
#define ATC_MAT_OFFSET                      (7)
#define ATC_MAT_LENGTH                      (5)
#define ATC_ENTRY_HIT_OFFSET                (12)
#define ATC_ENTRY_HIT_LENGTH                (4)
#define ATC_ADDR_OFFSET                     (16)
#define ATC_ADDR_LENGTH                     (9)
#define ATC_SINGLE_HIT_OFFSET               (30)
#define ATC_SINGLE_HIT_LENGTH               (1)
#define ATC_BUSY_OFFSET                     (31)
#define ATC_BUSY_LENGTH                     (1)

typedef enum {
    _ATC_CMD_READ = 0,
    _ATC_CMD_WRITE,
    _ATC_CMD_CLEAN,
    _ATC_CMD_SEARCH = 4,
    _ATC_CMD_SEARCH_NEXT,
    _ATC_CMD_LAST
}_ATC_CMD_T;

#define ATC_CMD_READ                        (_ATC_CMD_READ << ATC_MAC_OFFSET)
#define ATC_CMD_WRITE                       (_ATC_CMD_WRITE << ATC_MAC_OFFSET)
#define ATC_CMD_CLEAN                       (_ATC_CMD_CLEAN << ATC_MAC_OFFSET)
#define ATC_CMD_SEARCH                      (_ATC_CMD_SEARCH << ATC_MAC_OFFSET)
#define ATC_CMD_SEARCH_NEXT                 (_ATC_CMD_SEARCH_NEXT << ATC_MAC_OFFSET)

typedef enum {
    _ATC_SAT_MAC = 0,
    _ATC_SAT_DIP,
    _ATC_SAT_SIP,
    _ATC_SAT_ADDR,
    _ATC_SAT_LAST
}_ATC_SAT_T;

#define ATC_SAT_MAC                         (_ATC_SAT_MAC << ATC_SAT_OFFSET)
#define ATC_SAT_DIP                         (_ATC_SAT_DIP << ATC_SAT_OFFSET)
#define ATC_SAT_SIP                         (_ATC_SAT_SIP << ATC_SAT_OFFSET)
#define ATC_SAT_ADDR                        (_ATC_SAT_ADDR << ATC_SAT_OFFSET)

typedef enum {
    _ATC_MAT_ALL = 0,
    _ATC_MAT_MAC,
    _ATC_MAT_DYNAMIC_MAC,
    _ATC_MAT_STATIC_MAC,
    _ATC_MAT_DIP,
    _ATC_MAT_DIPV4,
    _ATC_MAT_DIPV6,
    _ATC_MAT_SIP,
    _ATC_MAT_SIPV4,
    _ATC_MAT_SIPV6,
    _ATC_MAT_MAC_BY_VID,
    _ATC_MAT_MAC_BY_FID,
    _ATC_MAT_MAC_BY_PORT,
    _ATC_MAT_SIP_BY_DIPV4,
    _ATC_MAT_SIP_BY_SIPV4,
    _ATC_MAT_SIP_BY_DIPV6,
    _ATC_MAT_SIP_BY_SIPV6,
    _ATC_MAT_LAST
}_ATC_MAT_T;

#define ATC_MAT_ALL                         (_ATC_MAT_ALL << ATC_MAT_OFFSET)
#define ATC_MAT_MAC                         (_ATC_MAT_MAC << ATC_MAT_OFFSET)
#define ATC_MAT_DYNAMIC_MAC                 (_ATC_MAT_DYNAMIC_MAC << ATC_MAT_OFFSET)
#define ATC_MAT_STATIC_MAC                  (_ATC_MAT_STATIC_MAC << ATC_MAT_OFFSET)
#define ATC_MAT_DIP                         (_ATC_MAT_DIP << ATC_MAT_OFFSET)
#define ATC_MAT_DIPV4                       (_ATC_MAT_DIPV4 << ATC_MAT_OFFSET)
#define ATC_MAT_DIPV6                       (_ATC_MAT_DIPV6 << ATC_MAT_OFFSET)
#define ATC_MAT_SIP                         (_ATC_MAT_SIP << ATC_MAT_OFFSET)
#define ATC_MAT_SIPV4                       (_ATC_MAT_SIPV4 << ATC_MAT_OFFSET)
#define ATC_MAT_SIPV6                       (_ATC_MAT_SIPV6 << ATC_MAT_OFFSET)
#define ATC_MAT_MAC_BY_VID                  (_ATC_MAT_MAC_BY_VID << ATC_MAT_OFFSET)
#define ATC_MAT_MAC_BY_FID                  (_ATC_MAT_MAC_BY_FID << ATC_MAT_OFFSET)
#define ATC_MAT_MAC_BY_PORT                 (_ATC_MAT_MAC_BY_PORT << ATC_MAT_OFFSET)
#define ATC_MAT_SIP_BY_DIPV4                (_ATC_MAT_SIP_BY_DIPV4 << ATC_MAT_OFFSET)
#define ATC_MAT_SIP_BY_SIPV4                (_ATC_MAT_SIP_BY_SIPV4 << ATC_MAT_OFFSET)
#define ATC_MAT_SIP_BY_DIPV6                (_ATC_MAT_SIP_BY_DIPV6 << ATC_MAT_OFFSET)
#define ATC_MAT_SIP_BY_SIPV6                (_ATC_MAT_SIP_BY_SIPV6 << ATC_MAT_OFFSET)

#define ATC_START_BUSY                      (0x01 << ATC_BUSY_OFFSET)

/* fields of AAC*/
#define AAC                                 (REG_ARL_BASE_ADDRESS + 0x00A0)
#define AAC_AGE_UNIT_OFFSET                 (0)
#define AAC_AGE_UNIT_LENGTH                 (11)
#define AAC_AGE_CNT_OFFSET                  (12)
#define AAC_AGE_CNT_LENGTH                  (9)
#define AAC_AUTO_FLUSH_OFFSET               (28)
#define AAC_AUTO_FLUSH_LENGTH               (1)

#define AGDIS                               (REG_ARL_BASE_ADDRESS + 0x00C0)

/* fields of ATWDS */
#define ATRDS                               (REG_ARL_BASE_ADDRESS + 0x0330)
#define ATRD0_MAC_SEL_OFFSET                (0)
#define ATRD0_MAC_SEL_LENGTH                (2)

/* fields of ATRD0 */
#define ATRD0                               (REG_ARL_BASE_ADDRESS + 0x0334)
//need to verify 32'b 0
#define ATRD0_MAC_LIVE_OFFSET               (0)
#define ATRD0_MAC_LIVE_LENGTH               (1)
#define ATRD0_MAC_LIFETIME_OFFSET           (1)
#define ATRD0_MAC_LIFETIME_LENGTH           (2)
#define ATRD0_MAC_TYPE_OFFSET               (3)
#define ATRD0_MAC_TYPE_LENGTH               (2)
#define ATRD0_MAC_LEAK_OFFSET               (5)
#define ATRD0_MAC_LEAK_LENGTH               (1)
#define ATRD0_MAC_UPRI_OFFSET               (6)
#define ATRD0_MAC_UPRI_LENGTH               (3)
#define ATRD0_MAC_IVL_OFFSET                (9)
#define ATRD0_MAC_IVL_LENGTH                (1)
#define ATRD0_MAC_VID_OFFSET                (10)
#define ATRD0_MAC_VID_LENGTH                (12)
#define ATRD0_MAC_ETAG_OFFSET               (22)
#define ATRD0_MAC_ETAG_LENGTH               (3)
#define ATRD0_MAC_FID_OFFSET                (25)
#define ATRD0_MAC_FID_LENGTH                (4)
#define ATRD1_MAC_UNAUTH_OFFSET             (31)
#define ATRD1_MAC_UNAUTH_LENGTH             (1)

/* fields of ATRD1 */
#define ATRD1                               (REG_ARL_BASE_ADDRESS + 0x0338)
//need to verify 32'b 0
#define ATRD1_MAC_FWD_OFFSET                (0)
#define ATRD1_MAC_FWD_LENGTH                (3)
#define ATRD1_MAC_AGETIME_OFFSET            (3)
#define ATRD1_MAC_AGETIME_LENGTH            (9)
#define ATRD1_MAC_MIR_OFFSET                (12)
#define ATRD1_MAC_MIR_LENGTH                (4)
#define ATRD1_MAC_ADDR_LSB_OFFSET           (16)
#define ATRD1_MAC_ADDR_LSB_LENGTH           (16)

/* fields of ATRD2 */
#define ATRD2                               (REG_ARL_BASE_ADDRESS + 0x033C)
#define ATRD2_MAC_ADDR_MSB_OFFSET           (0)
#define ATRD2_MAC_ADDR_MSB_LENGTH           (32)

/* fields of ATRD3 */
#define ATRD3                               (REG_ARL_BASE_ADDRESS + 0x0340)
//need to verify 32'b 0
#define ATRD3_MAC_PORT_OFFSET               (0)
#define ATRD3_MAC_PORT_LENGTH               (8)

#define REG_SCH_PORT_ADDRESS           (REG_ARL_BASE_ADDRESS + 0xc000)
#define MMSCR0_Q0(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p))
#define MMSCR1_Q0(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x4)
#define MMSCR0_Q1(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x8)
#define MMSCR1_Q1(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0xc)
#define MMSCR0_Q2(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x10)
#define MMSCR1_Q2(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x14)
#define MMSCR0_Q3(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x18)
#define MMSCR1_Q3(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x1c)
#define MMSCR0_Q4(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x20)
#define MMSCR1_Q4(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x24)
#define MMSCR0_Q5(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x28)
#define MMSCR1_Q5(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x2c)
#define MMSCR0_Q6(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x30)
#define MMSCR1_Q6(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x34)
#define MMSCR0_Q7(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x38)
#define MMSCR1_Q7(p)                   (REG_SCH_PORT_ADDRESS + PORT_CTRL_PORT_OFFSET * (p) + 0x3c)

#define PUPW(p)                             PORT_CTRL_REG(p, 0x30)

#define PEM1                                (REG_ARL_BASE_ADDRESS + 0x0048)
#define PEM2                                (REG_ARL_BASE_ADDRESS + 0x004c)
#define PEM3                                (REG_ARL_BASE_ADDRESS + 0x0050)
#define PEM4                                (REG_ARL_BASE_ADDRESS + 0x0054)

#define PIM1                                (REG_ARL_BASE_ADDRESS + 0x0058)
#define PIM2                                (REG_ARL_BASE_ADDRESS + 0x005c)
#define PIM3                                (REG_ARL_BASE_ADDRESS + 0x0060)
#define PIM4                                (REG_ARL_BASE_ADDRESS + 0x0064)
#define PIM5                                (REG_ARL_BASE_ADDRESS + 0x0068)
#define PIM6                                (REG_ARL_BASE_ADDRESS + 0x006c)
#define PIM7                                (REG_ARL_BASE_ADDRESS + 0x0070)

/* fields of ingress and egress rate control */
#define IRLCR(p)                            (REG_ARL_BASE_ADDRESS + (p * PORT_CTRL_PORT_OFFSET) + 0x4000)
#define ERLCR(p)                            (REG_ARL_BASE_ADDRESS + (p * PORT_CTRL_PORT_OFFSET) + 0xC040)
#define REG_RATE_CIR_OFFT                   (0)
#define REG_RATE_CIR_LENG                   (17)
#define REG_TB_EN_OFFT                      (19)
#define REG_RATE_TB_OFFT                    (20)
#define REG_RATE_TB_LENG                    (4)
#define REG_RATE_CBS_OFFT                   (24)
#define REG_RATE_CBS_LENG                   (7)
#define REG_RATE_EN_OFFT                    (31)

/* fields of global ingress and egress rate control */
#define GIRLCR                              (REG_ARL_BASE_ADDRESS + 0x7E24)
#define GERLCR                              (REG_ARL_BASE_ADDRESS + 0xFE00)
#define REG_IPG_BYTE_OFFT                   (0)
#define REG_IPG_BYTE_LENG                   (8)
#define REG_MFRM_EX_OFFT                    (9)
#define REG_MFRM_EX_LENG                    (1)
#define SFLOW_MFRM_EX_OFFT                  (25)
#define SFLOW_MFRM_EX_LENG                  (1)
#define L1_RATE_IPG_BYTE_CNT                (0x18)
#define L2_RATE_IPG_BYTE_CNT                (0x04)


/* fields of PTC */
#define PTC_INFO_SEL_SP                     (1 << 0)
#define PTC_INFO_SEL_SA                     (1 << 1)
#define PTC_INFO_SEL_DA                     (1 << 2)
#define PTC_INFO_SEL_SIP                    (1 << 3)
#define PTC_INFO_SEL_DIP                    (1 << 4)
#define PTC_INFO_SEL_SPORT                  (1 << 5)
#define PTC_INFO_SEL_DPORT                  (1 << 6)

#define SSC(p)                              PORT_CTRL_REG(p, 0x00)
#define PIC(p)                              PORT_CTRL_REG(p, 0x08)
#define PIC_PORT_IGMP_CTRL_CSR_IPM_01       (1 << 8)
#define PIC_PORT_IGMP_CTRL_CSR_IPM_33       (1 << 9)
#define PIC_PORT_IGMP_CTRL_CSR_IPM_224      (1 << 10)

/* fields of IGMP SNOOPING */
#define IGMP_HW_GQUERY                      1
#define IGMP_HW_SQUERY                      (1 << 2)
#define IGMP_HW_JOIN                        (1 << 4)
#define IGMPV3_HW_JOIN                      (1 << 6)
#define DMAC_01005E                         (1 << 8)
#define DMAC_3333XX                         (1 << 9)
#define MCAST_DIP                           (1 << 10)
#define IGMP_HW_LEAVE                       (1 << 12)
#define IGMP_AUTO_DOWNGRADE                 (1 << 20)
#define IGMP_AUTO_ROUTER                    (1 << 18)
#define IGMP_ROBUSTNESS_OFFSET              16
#define IGMP_QUERYINTERVAL_OFFSET           8

/* fields of MLD SNOOPING */
#define MLD_HW_GQUERY                       (1 << 1)
#define MLD_HW_SQUERY                       (1 << 3)
#define MLD_HW_JOIN                         (1 << 5)
#define MLDV2_HW_JOIN                       (1 << 7)
#define MLD_HW_LEAVE                        (1 << 13)
#define MLD_AUTO_ROUTER                     (1 << 19)

#define PMSR(p)                             PORT_MAC_CTRL_REG(p, 0x10)

/* fields of loop detect */
#define LPDET_CTRL                          0x30C0
#define LPDET_OFFSET                        24
#define LPDET_TRIGGER_OFFSET                23
#define LPDET_TRIGGER_PERIODICAL            1
#define LPDET_TRIGGER_BROADCAST             0

/* Port debug count register */
#define DBG_CNT_BASE                        0x3018
#define DBG_CNT_PORT_BASE                   0x100
#define DBG_CNT(p)                          (DBG_CNT_BASE + (p) * DBG_CNT_PORT_BASE)
#define DIS_CLR                             (1 << 31)

#define PFC_STS(p)                          PORT_MAC_CTRL_REG(p, 0x24)
#define PFC_CTRL                            (PORT_MAC_CTRL_BASE + 0xB0)
#define PFC_EN(p)                           (1 << p)
#define PFC_SYN_EN(p)                       (0x80 << p)

#define GMACCR                              (PORT_MAC_CTRL_BASE + 0x3e00)
#define MTCC_LMT_S                          8
#define MAX_RX_JUMBO_S                      4

/* Values of MAX_RX_PKT_LEN */
#define RX_PKT_LEN_1518                     (0)
#define RX_PKT_LEN_1536                     (1)
#define RX_PKT_LEN_1522                     (2)
#define RX_PKT_LEN_MAX_JUMBO                (3)

/* Fields of PMCR */
#define FORCE_MODE                          (1 << 31)
#define IPG_CFG_S                           (20)
#define IPG_CFG_M                           (0x300000)
#define EXT_PHY                             (1 << 19)
#define MAC_MODE                            (1 << 18)
#define MAC_TX_EN                           (1 << 16)
#define MAC_RX_EN                           (1 << 15)
#define MAC_PRE                             (1 << 14)
#define BKOFF_EN                            (1 << 12)
#define BACKPR_EN                           (1 << 11)
#define FORCE_EEE1G                         (1 << 7)
#define FORCE_EEE100                        (1 << 6)
#define FORCE_RX_FC                         (1 << 5)
#define FORCE_TX_FC                         (1 << 4)
#define FORCE_SPD_S                         (28)
#define FORCE_SPD_M                         (0x70000000)
#define FORCE_DPX                           (1 << 25)
#define FORCE_LINK                          (1 << 24)

/* Fields of PMSR */
#define EEE1G_STS                           (1 << 7)
#define EEE100_STS                          (1 << 6)
#define RX_FC_STS                           (1 << 5)
#define TX_FC_STS                           (1 << 4)
#define MAC_SPD_STS_S                       (28)
#define MAC_SPD_STS_M                       (0x70000000)
#define MAC_DPX_STS                         (1 << 25)
#define MAC_LNK_STS                         (1 << 24)

/* Values of MAC_SPD_STS */
#define MAC_SPD_10                          0
#define MAC_SPD_100                         1
#define MAC_SPD_1000                        2
#define MAC_SPD_2500                        3

/* Values of IPG_CFG */
#define IPG_96BIT                           0
#define IPG_96BIT_WITH_SHORT_IPG            1
#define IPG_64BIT                           2

/* Register of MIB Base address */
#define MAC_GLOBAL_REG_BASE                 0x10213E00
#define ARL_ACL_ATK_REG_BASE                0x10200000

#define MIB_BASE                            0x10214000
#define MIB_PORT_OFFSET                     0x0200
#define MIB_ACL_EVENT_OFFSET                0x0F00
#define MIB_TDPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x00)
#define MIB_TCRC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x04)
#define MIB_TUPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x08)
#define MIB_TMPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x0C)
#define MIB_TBPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x10)
#define MIB_TCEC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x14)
#define MIB_TSCEC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x18)
#define MIB_TMCEC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x1C)
#define MIB_TDEC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x20)
#define MIB_TLCEC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x24)
#define MIB_TXCEC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x28)
#define MIB_TPPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x2C)
#define MIB_TL64PC(p)                       (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x30)
#define MIB_TL65PC(p)                       (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x34)
#define MIB_TL128PC(p)                      (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x38)
#define MIB_TL256PC(p)                      (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x3C)
#define MIB_TL512PC(p)                      (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x40)
#define MIB_TL1024PC(p)                     (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x44)
#define MIB_TL1519PC(p)                     (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x48)

#define MIB_TOCL(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x4C)
#define MIB_TOCH(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x50)
#define MIB_TODPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x54)
#define MIB_TOCL2(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x58)
#define MIB_TOCH2(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x5C)



#define MIB_RDPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x80)
#define MIB_RFPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x84)
#define MIB_RUPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x88)
#define MIB_RMPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x8C)
#define MIB_RBPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x90)
#define MIB_RAEPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x94)
#define MIB_RCEPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x98)
#define MIB_RUSPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x9C)
#define MIB_RFEPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xA0)
#define MIB_ROSPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xA4)
#define MIB_RJEPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xA8)
#define MIB_RPPC(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xAC)
#define MIB_RL64PC(p)                       (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xB0)
#define MIB_RL65PC(p)                       (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xB4)
#define MIB_RL128PC(p)                      (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xB8)
#define MIB_RL256PC(p)                      (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xBC)
#define MIB_RL512PC(p)                      (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xC0)
#define MIB_RL1024PC(p)                     (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xC4)
#define MIB_RL1519PC(p)                     (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xC8)

#define MIB_ROCL(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xCC)
#define MIB_ROCH(p)                         (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xD0)
#define MIB_RCDPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xD4)
#define MIB_RIDPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xD8)
#define MIB_RADPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xDC)
#define MIB_FCDPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xE0)
#define MIB_WRDPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xE4)
#define MIB_MRDPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xE8)
#define MIB_ROCL2(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xEC)
#define MIB_ROCH2(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xF0)
#define MIB_SFSPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xF4)
#define MIB_SFTPC(p)                        (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xF8)
#define MIB_RXC_DPC(p)                      (MIB_BASE + (p) * MIB_PORT_OFFSET + 0xFC)


#define MIB_TMIB_HF_STS(p)                  (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x7C)
#define MIB_RMIB_HF_STS(p)                  (MIB_BASE + (p) * MIB_PORT_OFFSET + 0x100)

#define ACL_MIB_CNT_CFG                     (ARL_ACL_ATK_REG_BASE + 0x534)
#define ACL_MIB_CNT                         (ARL_ACL_ATK_REG_BASE + 0x538)


#define MIB_CCR                             (MAC_GLOBAL_REG_BASE + 0x0030)
#define MIB_PCLR                            (MAC_GLOBAL_REG_BASE + 0x0034)
#define MIB_CCR_MIB_ENABLE_OFFSET           (31)
#define MIB_CCR_MIB_ENABLE_LENGTH            (1)
#define MIB_CCR_MIB_ENABLE                  (1 << 31)
#define MIB_CCR_RX_OCT_CNT_GOOD             (1 << 7)
#define MIB_CCR_RX_OCT_CNT_BAD              (1 << 6)
#define MIB_CCR_TX_OCT_CNT_GOOD             (1 << 5)
#define MIB_CCR_TX_OCT_CNT_BAD              (1 << 4)

#define CSR_ACL_MIB_CLEAR                   (1 << 0)

#define LPDETTXCR                           (MAC_GLOBAL_REG_BASE + 0x100)
#define LPDETRXCR                           (MAC_GLOBAL_REG_BASE + 0x104)
#define LPDETCR                             (MAC_GLOBAL_REG_BASE + 0x108)
#define LPDETCR_OVER_RXPAUSE                (1 << 11)
#define LPDETCR_PERIOD_1S                   (1 << 7)
#define LPDETRXSR                           (MAC_GLOBAL_REG_BASE + 0x110)
#define LPDETTXSR                           (MAC_GLOBAL_REG_BASE + 0x114)
#define LPDET_SA_MSB                        (MAC_GLOBAL_REG_BASE + 0x120)
#define LPDET_FRAME_TYPE_OFFT               (16)
#define LPDET_FRAME_TYPE_LENG               (16)
#define LPDET_SMAC_MASK                     (0xFFFF)
#define LPDET_SA_LSB                        (MAC_GLOBAL_REG_BASE + 0x124)

#define SGMII_REG_BASE                      0x5000
#define SGMII_REG_PORT_BASE                 0x1000
#define SGMII_REG(p, r)                     (SGMII_REG_BASE + (p) * SGMII_REG_PORT_BASE + (r))
#define PCS_CONTROL_1(p)                    SGMII_REG(p, 0x00)
#define PCS_SPEED_ABILITY(p)                SGMII_REG(p, 0x08)
#define SGMII_MODE(p)                       SGMII_REG(p, 0x20)
#define QPHY_PWR_STATE_CTRL(p)              SGMII_REG(p, 0xE8)
#define PHYA_CTRL_SIGNAL3(p)                SGMII_REG(p, 0x128)

/* Fields of PCS_CONTROL_1 */
#define SGMII_AN_ABILITY                    (1 << 19)
#define SGMII_LINK_STATUS                   (1 << 18)
#define SGMII_AN_ENABLE_OFFT                (12)
#define SGMII_AN_ENABLE                     (1 << 12)
#define SGMII_AN_RESTART                    (1 << 9)

/* Fields of SGMII_MODE */
#define SGMII_REMOTE_FAULT_DIS              (1 << 8)
#define SGMII_IF_MODE_FORCE_DUPLEX          (1 << 4)
#define SGMII_IF_MODE_FORCE_SPEED_OFFT      (0x2)
#define SGMII_IF_MODE_FORCE_SPEED_R         (0x2)
#define SGMII_IF_MODE_FORCE_SPEED_M         (0x0C)
#define SGMII_IF_MODE_ADVERT_AN             (1 << 1)

/* Config of AN Tx control information (SGMII)
* LINK- 1:link up, 0:link down
* DUPLEX- 1:full, 0:half
* MODE - 1:SGMII, 0:Clause37
*/
#define AN_CONFIG_TX_LINK                   (1 << 15)
#define AN_CONFIG_TX_DUPLEX                 (1 << 12)
#define AN_CONFIG_TX_SPEED_OFFT             (10)
#define AN_CONFIG_TX_SPEED_MSK              (3 << AN_CONFIG_TX_SPEED_OFFT)
#define AN_CONFIG_TX_MODE_SGMII             (1)

/* Config of AN Tx control information (Clause37)
* MODE - 1:SGMII, 0:Clause37
*/
#define AN_CONFIG_TX_FULL_DUPLEX_CL37       (1 << 5)
#define AN_CONFIG_TX_HALF_DUPLEX_CL37       (1 << 6)
#define AN_CONFIG_TX_SYMMETRIC_PAUSE        (1 << 7)
#define AN_CONFIG_TX_ASYMMETRIC_PAUSE       (1 << 8)

/* Values of SGMII_IF_MODE_FORCE_SPEED */
#define SGMII_IF_MODE_FORCE_SPEED_10        0
#define SGMII_IF_MODE_FORCE_SPEED_100       1
#define SGMII_IF_MODE_FORCE_SPEED_1000      2

/* Fields of QPHY_PWR_STATE_CTRL */
#define PHYA_PWD                            (1 << 4)

/* Fields of PHYA_CTRL_SIGNAL3 */
#define RG_TPHY_SPEED_S                     2
#define RG_TPHY_SPEED_M                     0x0c

/* Values of RG_TPHY_SPEED */
#define RG_TPHY_SPEED_1000                  0
#define RG_TPHY_SPEED_2500                  1


#define SCU_BASE                            0x10000000
#define RG_RGMII_TXCK_C                     (SCU_BASE + 0x1d0)

#define HSGMII_AN_CSR_BASE                  0x10220000
#define SGMII_REG_AN0                       (HSGMII_AN_CSR_BASE + 0x000)
#define SGMII_REG_AN_13                     (HSGMII_AN_CSR_BASE + 0x034)
#define SGMII_REG_AN_FORCE_CL37             (HSGMII_AN_CSR_BASE + 0x060)

#define HSGMII_CSR_PCS_BASE                 0x10220000
#define RG_HSGMII_PCS_CTROL_1               (HSGMII_CSR_PCS_BASE + 0xa00)
#define RG_AN_SGMII_MODE_FORCE              (HSGMII_CSR_PCS_BASE + 0xa24)

#define MULTI_SGMII_CSR_BASE                0x10224000
#define SGMII_STS_CTRL_0                    (MULTI_SGMII_CSR_BASE + 0x018)
#define MSG_RX_CTRL_0                       (MULTI_SGMII_CSR_BASE + 0x100)
#define MSG_RX_LIK_STS_0                    (MULTI_SGMII_CSR_BASE + 0x514)
#define MSG_RX_LIK_STS_2                    (MULTI_SGMII_CSR_BASE + 0x51c)
#define PHY_RX_FORCE_CTRL_0                 (MULTI_SGMII_CSR_BASE + 0x520)

#define XFI_CSR_PCS_BASE                    0x10225000
#define RG_USXGMII_AN_CONTROL_0             (XFI_CSR_PCS_BASE + 0xbf8)

#define MULTI_PHY_RA_CSR_BASE               0x10226000
#define RG_RATE_ADAPT_CTRL_0               (MULTI_PHY_RA_CSR_BASE + 0x000)
#define RATE_ADP_P0_CTRL_0                 (MULTI_PHY_RA_CSR_BASE + 0x100)
#define MII_RA_AN_ENABLE                   (MULTI_PHY_RA_CSR_BASE + 0x300)

#define QP_DIG_CSR_BASE                     0x1022a000
#define QP_CK_RST_CTRL_4                    (QP_DIG_CSR_BASE + 0x310)
#define QP_DIG_MODE_CTRL_0                  (QP_DIG_CSR_BASE + 0x324)
#define QP_DIG_MODE_CTRL_1                  (QP_DIG_CSR_BASE + 0x330)

#define SERDES_WRAPPER_BASE                 0x1022c000
#define USGMII_CTRL_0                       (SERDES_WRAPPER_BASE + 0x000)

#define QP_PMA_TOP_BASE                     0x1022e000
#define PON_RXFEDIG_CTRL_0                  (QP_PMA_TOP_BASE + 0x100)
#define PON_RXFEDIG_CTRL_9                  (QP_PMA_TOP_BASE + 0x124)

#define SS_LCPLL_PWCTL_SETTING_2            (QP_PMA_TOP_BASE + 0x208)
#define SS_LCPLL_TDC_FLT_2                  (QP_PMA_TOP_BASE + 0x230)
#define SS_LCPLL_TDC_FLT_5                  (QP_PMA_TOP_BASE + 0x23c)
#define SS_LCPLL_TDC_PCW_1                  (QP_PMA_TOP_BASE + 0x248)
#define INTF_CTRL_8                         (QP_PMA_TOP_BASE + 0x320)
#define INTF_CTRL_9                         (QP_PMA_TOP_BASE + 0x324)
#define PLL_CTRL_0                          (QP_PMA_TOP_BASE + 0x400)
#define PLL_CTRL_2                          (QP_PMA_TOP_BASE + 0x408)
#define PLL_CTRL_3                          (QP_PMA_TOP_BASE + 0x40c)
#define PLL_CTRL_4                          (QP_PMA_TOP_BASE + 0x410)
#define PLL_CK_CTRL_0                       (QP_PMA_TOP_BASE + 0x414)
#define RX_DLY_0                            (QP_PMA_TOP_BASE + 0x614)
#define RX_CTRL_2                           (QP_PMA_TOP_BASE + 0x630)
#define RX_CTRL_5                           (QP_PMA_TOP_BASE + 0x63c)
#define RX_CTRL_6                           (QP_PMA_TOP_BASE + 0x640)
#define RX_CTRL_7                           (QP_PMA_TOP_BASE + 0x644)
#define RX_CTRL_8                           (QP_PMA_TOP_BASE + 0x648)
#define RX_CTRL_26                          (QP_PMA_TOP_BASE + 0x690)
#define RX_CTRL_42                          (QP_PMA_TOP_BASE + 0x6d0)

#define QP_ANA_CSR_BASE                     0x1022f000
#define RG_QP_RX_DAC_EN                     (QP_ANA_CSR_BASE + 0x00)
#define RG_QP_RXAFE_RESERVE                 (QP_ANA_CSR_BASE + 0x04)
#define RG_QP_CDR_LPF_MJV_LIM               (QP_ANA_CSR_BASE + 0x0c)
#define RG_QP_CDR_LPF_SETVALUE              (QP_ANA_CSR_BASE + 0x14)
#define RG_QP_CDR_PR_CKREF_DIV1             (QP_ANA_CSR_BASE + 0x18)
#define RG_QP_CDR_PR_KBAND_DIV_PCIE         (QP_ANA_CSR_BASE + 0x1c)
#define RG_QP_CDR_FORCE_IBANDLPF_R_OFF      (QP_ANA_CSR_BASE + 0x20)
#define RG_QP_TX_MODE_16B_EN                (QP_ANA_CSR_BASE + 0x28)
#define RG_QP_PLL_IPLL_DIG_PWR_SEL          (QP_ANA_CSR_BASE + 0x3c)
#define RG_QP_PLL_SDM_ORD                   (QP_ANA_CSR_BASE + 0x40)

#define ETHER_SYS_BASE                      0x1028c800
#define RG_P5MUX_MODE                       (ETHER_SYS_BASE + 0x00)
#define RG_FORCE_CKDIR_SEL                  (ETHER_SYS_BASE + 0x04)
#define RG_SWITCH_MODE                      (ETHER_SYS_BASE + 0x08)
#define RG_FORCE_MAC5_SB                    (ETHER_SYS_BASE + 0x2c)
#define CSR_RMII                            (ETHER_SYS_BASE + 0x70)


#define SYS_CTRL                            0x7000
#define SW_PHY_RST                          (1 << 2)
#define SW_SYS_RST                          (1 << 1)
#define SW_REG_RST                          (1 << 0)

#define PHY_IAC                             0x1000e000

#define CLKGEN_CTRL                         0x7500
#define CLK_SKEW_OUT_S                      8
#define CLK_SKEW_OUT_M                      0x300
#define CLK_SKEW_IN_S                       6
#define CLK_SKEW_IN_M                       0xc0
#define RXCLK_NO_DELAY                      (1 << 5)
#define TXCLK_NO_REVERSE                    (1 << 4)
#define GP_MODE_S                           1
#define GP_MODE_M                           0x06
#define GP_CLK_EN                           (1 << 0)

/* Values of GP_MODE */
#define GP_MODE_RGMII                       0
#define GP_MODE_MII                         1
#define GP_MODE_REV_MII                     2

/* Values of CLK_SKEW_IN */
#define CLK_SKEW_IN_NO_CHANGE               0
#define CLK_SKEW_IN_DELAY_100PPS            1
#define CLK_SKEW_IN_DELAY_200PPS            2
#define CLK_SKEW_IN_REVERSE                 3

/* Values of CLK_SKEW_OUT */
#define CLK_SKEW_OUT_NO_CHANGE              0
#define CLK_SKEW_OUT_DELAY_100PPS           1
#define CLK_SKEW_OUT_DELAY_200PPS           2
#define CLK_SKEW_OUT_REVERSE                3

#define HWSTRAP                             0x7800
#define XTAL_FSEL_S                         7
#define XTAL_FSEL_M                         (1 << 7)

#define XTAL_40MHZ                          0
#define XTAL_25MHZ                          1

#define PLLGP_EN                            0x7820
#define EN_COREPLL                          (1 << 2)
#define SW_CLKSW                            (1 << 1)
#define SW_PLLGP                            (1 << 0)

#define PLLGP_CR0                           0x78a8
#define RG_COREPLL_EN                       (1 << 22)
#define RG_COREPLL_POSDIV_S                 23
#define RG_COREPLL_POSDIV_M                 0x3800000
#define RG_COREPLL_SDM_PCW_S                1
#define RG_COREPLL_SDM_PCW_M                0x3ffffe
#define RG_COREPLL_SDM_PCW_CHG              (1 << 0)

#define MHWSTRAP                            0x7804
#define STRAP_CHG_STRAP                     (1 << 8)
#define STRAP_PHY_EN                        (1 << 6)

#define TOP_SIG_SR                          0x780c
#define PAD_DUAL_SGMII_EN                   (1 << 1)

/* RGMII and SGMII PLL clock */
#define ANA_PLLGP_CR2                       0x78b0
#define ANA_PLLGP_CR5                       0x78bc

/* Efuse Register Define */
#define GBE_EFUSE                           0x7bc8
#define GBE_SEL_EFUSE_EN                    (1 << 0)

/* GPIO_PAD_0 */
#define GPIO_MODE0                          0x7c0c
#define GPIO_MODE0_S                        0
#define GPIO_MODE0_M                        0xf
#define GPIO_0_INTERRUPT_MODE               0x1

#define SMT0_IOLB                           0x7f04
#define SMT_IOLB_5_SMI_MDC_EN               (1 << 5)

/* PHY CL22 reg */
#define PHY_PAGE_0                          0x0
/* Mode Control Register */
#define PHY_MCR                             0x00
#define MCR_SR                              (1 << 15)
#define MCR_LB                              (1 << 14)
#define MCR_MR_FC_SPD_INT_0                 (1 << 13)
#define MCR_AN_EN                           (1 << 12)
#define MCR_PW_DN                           (1 << 11)
#define MCR_ISO                             (1 << 10)
#define MCR_RST_AN                          (1 << 9)
#define MCR_MR_DUX                          (1 << 8)
#define MCR_MR_CLS_TEST                     (1 << 7)
#define MCR_MR_FC_SPD_INT_1                 (1 << 6)

/* Mode Status Register */
#define PHY_MSR                             0x01
#define MSR_CAP_100T4                       (1 << 15)
#define MSR_CAP_100X_FDX                    (1 << 14)
#define MSR_CAP_100X_HDX                    (1 << 13)
#define MSR_CAP_10T_FDX                     (1 << 12)
#define MSR_CAP_10T_HDX                     (1 << 11)
#define MSR_CAP_100T2_HDX                   (1 << 10)
#define MSR_CAP_100T2_FDX                   (1 << 9)
#define MSR_EXT_STA_EN                      (1 << 8)
#define MSR_PRAM_SUP_CAP                    (1 << 6)
#define MSR_AN_COMP                         (1 << 5)
#define MSR_RMT_FAULT                       (1 << 4)
#define MSR_AN_CAP                          (1 << 3)
#define MSR_LINK_STA                        (1 << 2)
#define MSR_JAB_DECT                        (1 << 1)
#define MSR_EXT_CAP                         (1 << 0)

/* Auto-Negotiation Advertisement Register */
#define PHY_AN_ADV                          0x04
#define AN_ADV_NX_PAGE_REQ                  (1 << 15)
#define AN_ADV_RF                           (1 << 13)
#define AN_ADV_CAP_PAUSE                    (3 << 10)
#define AN_ADV_CAP_100_T4                   (1 << 9)
#define AN_ADV_CAP_100_FDX                  (1 << 8)
#define AN_ADV_CAP_100_HDX                  (1 << 7)
#define AN_ADV_CAP_10_FDX                   (1 << 6)
#define AN_ADV_CAP_10_HDX                   (1 << 5)
#define AN_ADV_802_9_ISLAN_16T              (2 << 0)
#define AN_ADV_802_3                        (1 << 0)

/* Auto-Negotiation Link Partner Advertisement Register */
#define PHY_AN_LP_ADV                       0x05
#define AN_LP_NX_PAGE_REQ                   (1 << 15)
#define AN_LP_ACK                           (1 << 14)
#define AN_LP_RF                            (1 << 13)
#define AN_LP_CAP_PAUSE                     (3 << 10)
#define AN_LP_CAP_100_T4                    (1 << 9)
#define AN_LP_CAP_100_FDX                   (1 << 8)
#define AN_LP_CAP_100_HDX                   (1 << 7)
#define AN_LP_CAP_10_FDX                    (1 << 6)
#define AN_LP_CAP_10_HDX                    (1 << 5)
#define AN_LP_802_9_ISLAN_16T               (2 << 0)
#define AN_LP_802_3                         (1 << 0)

/* 1000BASE-T Control Register */
#define PHY_CR1G                            0x09
#define CR1G_TEST_TM4                       (4 << 13)
#define CR1G_TEST_TM3                       (3 << 13)
#define CR1G_TEST_TM2                       (2 << 13)
#define CR1G_TEST_TM1                       (1 << 13)
#define CR1G_TEST_NORMAL                    (0 << 13)
#define CR1G_MS_EN                          (1 << 12)
#define CR1G_MS_CONF                        (1 << 11)
#define CR1G_PORT_TYPE                      (1 << 10)
#define CR1G_ADV_CAP1000_FDX                (1 << 9)
#define CR1G_ADV_CAP1000_HDX                (1 << 8)

/* 1000BASE-T Status Register */
#define PHY_SR1G                            0x0A
#define SR1G_MS_CFG_FAULT                   (1 << 15)
#define SR1G_MS_CFG_RES                     (1 << 14)
#define SR1G_LOC_RX                         (1 << 13)
#define SR1G_RMT_RX                         (1 << 12)
#define SR1G_CAP1000_FDX                    (1 << 11)
#define SR1G_CAP1000_HDX                    (1 << 10)
#define SR1G_IDLE_ERR_MASK                  0xFF

#define PHY_PAGE_1                          0x1
/* Ethernet Packet Generator Control Register */
#define PHY_EPG                             0x1D
#define EPG_EN                              (1 << 15)
#define EPG_RUN                             (1 << 14)
#define EPG_TX_DUR                          (1 << 13)
#define EPG_PKT_LEN_10KB                    (3 << 11)
#define EPG_PKT_LEN_1518B                   (2 << 11)
#define EPG_PKT_LEN_64B                     (1 << 11)
#define EPG_PKT_LEN_125B                    (0 << 11)
#define EPG_PKT_GAP                         (1 << 10)
#define EPG_DES_ADDR(a)                     ( ( (a) & 0xF ) << 6 )
#define EPG_SUR_ADDR(a)                     ( ( (a) & 0xF ) << 2 )
#define EPG_PL_TYP_RANDOM                   (1 << 1)
#define EPG_BAD_FCS                         (1 << 0)

/* external*/
#define EXPHY_CMD_WRITE                     (0x10)
#define DFETAILDC_COEFF_L                   (0x11)
#define DFETAILDC_COEFF_M                   (0x12)

/* PHY CL22 reg */
#define PHY_PAGE                            (0x1F)

/* PHY CL45 reg */
#define PHY_DEV_07H                         (0x07)
#define PHY_DEV_1FH                         (0x1F)
#define PHY_DEV_1EH                         (0x1E)

/* dev 07h, reg 03Ch: EEE Advertisement Register */
#define EEE_ADV_REG                         (0x3C)
#define EEE_ADV_1000BT                      (1 << 2)
#define EEE_ADV_100BT                       (1 << 1)

/* dev 1Eh, reg 013h: TX pair delay Register */
#define TX_PAIR_DELAY_SEL_REG               (0x013)

/* dev 1Eh, reg 03Ch: Bypass power-down Register */
#define BYPASS_POWER_DOWN_REG0              (0x3C)
#define BYPASS_POWER_DOWN_REG1              (0x3D)
#define BYPASS_POWER_DOWN_REG2              (0x3E)


/* dev 1Eh, reg 145h: T10 Test Conttrol Register */
#define PD_DIS                              (1 << 15)
#define FC_TDI_EN                           (1 << 14)
#define FC_DI_ACT                           (1 << 13)
#define FC_LITN_NO_COMP                     (1 << 12)
#define FC_MDI_CO_MDIX                      (3 << 3)
#define FC_MDI_CO_MDI                       (2 << 3)
#define FC_MDI_CO_NOT                       (0 << 3)
#define FC_10T_POLAR_SWAP                   (3 << 1)
#define FC_10T_POLAR_NORMAL                 (2 << 1)
#define FC_10T_POLAR_NOT                    (0 << 1)

/* dev 1Eh, reg 14Ah: DSP control 1 Register */
#define DSP_CONTROL_REG                     (0x14A)
#define PICMD_MISER_MODE_INT(v)             (((v) & 0x7ff) << 5)

/* dev 1Eh, reg 20Bh: DSP state machine PM control Register */
#define DSP_FRE_PM_REG                      (0x20B)

/* dev 1Eh, reg 20Eh: DSP state machine FRE control Register */
#define DSP_FRE_REG                         (0x20E)
#define DSP_FRE_RP_FSM_EN                   (1 << 4)
#define DSP_FRE_DW_AUTO_INC                 (1 << 2)
#define DSP_FRE_WR_EN                       (1 << 1)
#define DSP_FRE_SW_RST                      (1 << 0)

/* dev 1Eh, reg 2d1h: Register */
#define RG_LPI_REG                          (0x2D1)
#define RG_LPI_VCO_EEE_STGO_EN              (1 << 10)
#define RG_LPI_TR_READY                     (1 << 9)
#define RG_LPI_SKIP_SD_SLV_TR               (1 << 8)
#define VCO_SLICER_THRES_H                  (0x33)

/* dev 1Fh, reg 021h: LED Basic control Register */
#define LED_BCR                             (0x021)
#define LED_BCR_EXT_CTRL                    (1 << 15)
#define LED_BCR_EVT_ALL                     (1 << 4)
#define LED_BCR_CLK_EN                      (1 << 3)
#define LED_BCR_TIME_TEST                   (1 << 2)
#define LED_BCR_MODE_MASK                   (3)
#define LED_BCR_MODE_DISABLE                (0)
#define LED_BCR_MODE_2LED                   (1)
#define LED_BCR_MODE_3LED_1                 (2)
#define LED_BCR_MODE_3LED_2                 (3)

/* dev 1Fh, reg 022h: LED On Duration Register */
#define LED_ON_DUR                          (0x022)
#define LED_ON_DUR_MASK                     (0xFFFF)

/* dev 1Fh, reg 023h: LED Blinking Duration Register */
#define LED_BLK_DUR                         (0x023)
#define LED_BLK_DUR_MASK                    (0xFFFF)

/* dev 1Fh, reg 024h: LED On Control Register */
#define LED_ON_CTRL(i)                      (0x024 + ( (i) * 2 ))
#define LED_ON_EN                           (1 << 15)
#define LED_ON_POL                          (1 << 14)
#define LED_ON_EVT_MASK                     (0x7F)
#define LED_ON_EVT_FORCE                    (1 << 6)
#define LED_ON_EVT_HDX                      (1 << 5)
#define LED_ON_EVT_FDX                      (1 << 4)
#define LED_ON_EVT_LINK_DN                  (1 << 3)
#define LED_ON_EVT_LINK_10M                 (1 << 2)
#define LED_ON_EVT_LINK_100M                (1 << 1)
#define LED_ON_EVT_LINK_1000M               (1 << 0)

/* dev 1Fh, reg 025h: LED Blinking Control Register */
#define LED_BLK_CTRL(i)                     (0x025 + ( (i) * 2 ))
#define LED_BLK_EVT_MASK                    (0x3FF)
#define LED_BLK_EVT_FORCE                   (1 << 9)
#define LED_BLK_EVT_RX_IDL                  (1 << 8)
#define LED_BLK_EVT_RX_CRC                  (1 << 7)
#define LED_BLK_EVT_CLS                     (1 << 6)
#define LED_BLK_EVT_10M_RX_ACT              (1 << 5)
#define LED_BLK_EVT_10M_TX_ACT              (1 << 4)
#define LED_BLK_EVT_100M_RX_ACT             (1 << 3)
#define LED_BLK_EVT_100M_TX_ACT             (1 << 2)
#define LED_BLK_EVT_1000M_RX_ACT            (1 << 1)
#define LED_BLK_EVT_1000M_TX_ACT            (1 << 0)

/* dev 1Fh, reg 27Bh: 10M Driver Register */
#define CR_RG_TX_CM_10M(val)                ( ( (val) & 0x3 ) << 12 )
#define CR_RG_DELAY_TX_10M(val)             ( ( (val) & 0x3 ) << 8 )
#define CR_DA_TX_GAIN_10M_EEE(val)          ( ( ( ( (val) / 10 ) - 3 ) & 0x7 ) << 4 )
#define CR_DA_TX_GAIN_10M(val)              ( ( ( (val) / 10 ) - 3 ) & 0x7 )

/* dev 1Fh, reg 403h: PLL_group Control Register */
#define PLL_GROUP_CONTROL_REG               (0x403)
#define RG_SYSPLL_DDSFBK_EN                 (1 << 12)
#define RG_SYSPLL_DMY1                      (3 << 8)
#define RG_SYSPLL_EEE_EN                    (1 << 7)    //1:enable/0:disable EEE mode when RG_SYSPLL_EEE_EN_PYPASS = 1
#define RG_SYSPLL_EEE_EN_PYPASS             (1 << 6)    //EEE enable is control by 0:top/1:RG_SYSPLL_EEE_EN
#define RG_SYSPLL_AFE_PWD                   (1 << 5)    //1:enable/0:disable analog power down when RG_SYSPLL_AFE_PWD_BYPASS = 1
#define RG_SYSPLL_AFE_PWD_BYPASS            (1 << 4)    //Analog power down is control by 0:top/1:RG_SYSPLL_AFE_PWD
#define RG_SYSPLL_EFUSE_DIS                 (1 << 3)    //1:efuse mode / 0:disable efuse mode, and use internal RG
#define RG_CLKDRV_FORCEIN                   (3 << 1)    //analog test mode
#define RG_XSQ_LPF_EN                       (1 << 0)    //analog test mode

/* Register of ACL address */
#define ARL_GLOBAL_CNTRL        (REG_ARL_BASE_ADDRESS + 0x00c)
#define ACL_BASE                (REG_ARL_BASE_ADDRESS + 0x500)
#define ACL_GLOBAL_CFG          (ACL_BASE + 0x00)
#define ACL_PORT_EN             (ACL_BASE + 0x04)
#define ACL_GROUP_CFG           (ACL_BASE + 0x08)
#define ACL_MEM_CFG             (ACL_BASE + 0x0c)
#define ACL_MEM_CFG_WDATA0      (ACL_BASE + 0x10)
#define ACL_MEM_CFG_WDATA1      (ACL_BASE + 0x14)
#define ACL_MEM_CFG_WDATA2      (ACL_BASE + 0x18)
#define ACL_MEM_CFG_WDATA3      (ACL_BASE + 0x1c)
#define ACL_MEM_CFG_RDATA0      (ACL_BASE + 0x20)
#define ACL_MEM_CFG_RDATA1      (ACL_BASE + 0x24)
#define ACL_MEM_CFG_RDATA2      (ACL_BASE + 0x28)
#define ACL_MEM_CFG_RDATA3      (ACL_BASE + 0x2c)
#define ACL_STATUS              (ACL_BASE + 0x30)
#define ACL_TRTCM               (REG_ARL_BASE_ADDRESS + 0x100)
#define ACL_TRTCMA              (REG_ARL_BASE_ADDRESS + 0x104)
#define ACL_TRTCMW_CBS          (REG_ARL_BASE_ADDRESS + 0x108)
#define ACL_TRTCMW_EBS          (REG_ARL_BASE_ADDRESS + 0x10C)
#define ACL_TRTCMW_CIR          (REG_ARL_BASE_ADDRESS + 0x110)
#define ACL_TRTCMW_EIR          (REG_ARL_BASE_ADDRESS + 0x114)
#define ACL_TRTCMR_CBS          (REG_ARL_BASE_ADDRESS + 0x118)
#define ACL_TRTCMR_EBS          (REG_ARL_BASE_ADDRESS + 0x11c)
#define ACL_TRTCMR_CIR          (REG_ARL_BASE_ADDRESS + 0x120)
#define ACL_TRTCMR_EIR          (REG_ARL_BASE_ADDRESS + 0x124)

#define ACLRMC                  (REG_ARL_BASE_ADDRESS + 0x470)
#define ACLRMD1                 (REG_ARL_BASE_ADDRESS + 0x474)
#define ACLRMD2                 (REG_ARL_BASE_ADDRESS + 0x478)

#define ACL_UDF_BASE            (REG_ARL_BASE_ADDRESS + 0x200)
#define ACL_AUTC                (ACL_UDF_BASE + 0x00)
#define ACL_AUTW0               (ACL_UDF_BASE + 0x08)
#define ACL_AUTW1               (ACL_UDF_BASE + 0x0c)
#define ACL_AUTW2               (ACL_UDF_BASE + 0x10)
#define ACL_AUTR0               (ACL_UDF_BASE + 0x20)
#define ACL_AUTR1               (ACL_UDF_BASE + 0x24)
#define ACL_AUTR2               (ACL_UDF_BASE + 0x28)

/* Register of DPCR */
#define BMU_PORT_BASE               (0x10204000)
#define DPCR_COLOR_OFFSET           (0x20)
#define DPCR_QUEUE_OFFSET           (0x4)
#define DPCR_EN(p)                  (BMU_PORT_BASE + (p * PORT_CTRL_PORT_OFFSET) + 0x08)
#define DPCR_BASE                   (BMU_PORT_BASE + 0x10)
#define DPCR(p, c, q)               (DPCR_BASE + (p * PORT_CTRL_PORT_OFFSET) + (c * DPCR_COLOR_OFFSET) + (q * DPCR_QUEUE_OFFSET))

/* Register of VLAN */
#define VTCR                    (0x10200600)
#define VLNWDATA0               (0x10200604)
#define VLNWDATA1               (0x10200608)
#define VLNWDATA2               (0x1020060C)
#define VLNWDATA3               (0x10200610)
#define VLNWDATA4               (0x10200614)
#define VLNRDATA0               (0x10200618)
#define VLNRDATA1               (0x1020061C)
#define VLNRDATA2               (0x10200620)
#define VLNRDATA3               (0x10200624)
#define VLNRDATA4               (0x10200628)

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/

#endif  /* AIR_REG_H */

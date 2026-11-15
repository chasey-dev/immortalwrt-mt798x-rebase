    /* FILE NAME:  an8855_reg.h
 * PURPOSE:
 *      It provides AN8855 register definition.
 * NOTES:
 *
 */

#ifndef AN8855_REG_H
#define AN8855_REG_H

#define PORT_CTRL_BASE                      0x10208000
#define PORT_CTRL_PORT_OFFSET               0x200
#define PORT_CTRL_REG(p, r)                 (PORT_CTRL_BASE + (p) * PORT_CTRL_PORT_OFFSET + (r))
#define PCR(p)                              PORT_CTRL_REG(p, 0x04)

#define PORT_MAC_CTRL_BASE                  0x10210000
#define PORT_MAC_CTRL_PORT_OFFSET           0x200
#define PORT_MAC_CTRL_REG(p, r)             (PORT_MAC_CTRL_BASE + (p) * PORT_MAC_CTRL_PORT_OFFSET + (r))
#define PMCR(p)                             PORT_MAC_CTRL_REG(p, 0x00)

/* Port debug count register */
#define DBG_CNT_BASE                        0x3018
#define DBG_CNT_PORT_BASE                   0x100
#define DBG_CNT(p)                          (DBG_CNT_BASE + (p) * DBG_CNT_PORT_BASE)
#define DIS_CLR                             (1 << 31)

#define GMACCR                              (PORT_MAC_CTRL_BASE + 0x30e0)
#define MTCC_LMT_S                          8
#define MAX_RX_JUMBO_S                      4

/* Values of MAX_RX_PKT_LEN */
#define RX_PKT_LEN_1518                     0
#define RX_PKT_LEN_1536                     1
#define RX_PKT_LEN_1522                     2
#define RX_PKT_LEN_MAX_JUMBO                3

/* Fields of PMCR */
#define FORCE_MODE                          (1 << 31)
#define IPG_CFG_S                           20
#define IPG_CFG_M                           0x300000
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
#define FORCE_SPD_S                         28
#define FORCE_SPD_M                         0x70000000
#define FORCE_DPX                           (1 << 25)
#define FORCE_LINK                          (1 << 24)

/* Fields of PMSR */
#define EEE1G_STS                           (1 << 7)
#define EEE100_STS                          (1 << 6)
#define RX_FC_STS                           (1 << 5)
#define TX_FC_STS                           (1 << 4)
#define MAC_SPD_STS_S                       28
#define MAC_SPD_STS_M                       0x70000000
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

#define SGMII_REG_BASE                      0x5000
#define SGMII_REG_PORT_BASE                 0x1000
#define SGMII_REG(p, r)                     (SGMII_REG_BASE + (p) * SGMII_REG_PORT_BASE + (r))
#define PCS_CONTROL_1(p)                    SGMII_REG(p, 0x00)
#define SGMII_MODE(p)                       SGMII_REG(p, 0x20)
#define QPHY_PWR_STATE_CTRL(p)              SGMII_REG(p, 0xe8)
#define PHYA_CTRL_SIGNAL3(p)                SGMII_REG(p, 0x128)

/* Fields of PCS_CONTROL_1 */
#define SGMII_LINK_STATUS                   (1 << 18)
#define SGMII_AN_ENABLE                     (1 << 12)
#define SGMII_AN_RESTART                    (1 << 9)

/* Fields of SGMII_MODE */
#define SGMII_REMOTE_FAULT_DIS              (1 << 8)
#define SGMII_IF_MODE_FORCE_DUPLEX          (1 << 4)
#define SGMII_IF_MODE_FORCE_SPEED_S         0x2
#define SGMII_IF_MODE_FORCE_SPEED_M         0x0c
#define SGMII_IF_MODE_ADVERT_AN             (1 << 1)

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

#define SYS_CTRL                            0x7000
#define SW_PHY_RST                          (1 << 2)
#define SW_SYS_RST                          (1 << 1)
#define SW_REG_RST                          (1 << 0)

#define PHY_IAC                             (0x1000e000)
#define IAC_MAX_BUSY_TIME                   (1000)

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

#endif  /* End of AN8855_REG_H */

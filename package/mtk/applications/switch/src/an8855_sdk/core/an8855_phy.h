/* FILE NAME:  an8855_phy.h
 * PURPOSE:
 *      It provides AN8855 phy definition.
 * NOTES:
 *
 */

#ifndef _AN8855_PHY_H_
#define _AN8855_PHY_H_


/* Type Definitions */
#define int8_t char
#define uint8_t unsigned char
#define int16_t short
#define uint16_t unsigned short
#define int32_t int
#define uint32_t unsigned int
/* DATA TYPE DECLARATIONS
 */
typedef int                 BOOL_T;
typedef signed char         I8_T;
typedef unsigned char       UI8_T;
typedef signed short        I16_T;
typedef unsigned short      UI16_T;
typedef signed int          I32_T;
typedef unsigned int        UI32_T;
typedef char                C8_T;
typedef unsigned long long  UI64_T;

typedef UI8_T   AIR_MAC_T[6];

/* Debug flags */
//#define _MDIO_BOOTS_MODE     1 // Boots for preamble
#define _DEBUG_PRINT         1 // Debug print for Arduino
//#define _DEBUG_PRINT_eFuse     1
#define _write_eFuse     1

#define _DEBUG_SCAN_ALL      0 // Scan all Code for SerDes Calibration
#define _WRITE_RG_DIR        1 // Write RG directly for Calibration
#define _USER_DEFINE_MODE    0 // Replace to user-defined RG for Calibration
#define _DEBUG_MANUAL        1 // dbg_20210604  // manual dbg_20210604
/**************************************************************************/

/* Phy Address */
//#define phyadd_common 0x1d            // EN8801
//#define PHY_NUM 1             // EN8801
//#define phyadd_common 0x9         // EN7523
//#define PHY_NUM 4             // EN7523
//#define phyadd_common 0x0         // EN8850
//#define PHY_NUM   5           // EN8850
#define PHY_NUM   4             // EN8851
#define CALIPOLARITY  1

#define TXAMP_offset  0    // for 8851

/* On/Off */
//#define ENABLE  1
//#define DISABLE 0
#define Relay_ENABLE  1
#define Relay_DISABLE 0

/* FT Pattern */
#define _MDIO     0x0
#define _I2C      0x1
#define FT_USB3_T101    0x0
#define FT_PCIE0_T101   0x1
#define FT_PCIE1_T101   0x2
#define FT_PON_T101     0x3

/********* Digital pin definition *************/
#define Relay_Tx_PN         22   // relay 1
#define Relay_R_R50         23   // relay 2
#define Relay_Tx_Vol        24   // relay 3
#define Relay_Rx_Vol        25   // relay 4
#define Relay_DUT_GND       26   // relay 5
#define Relay_I2C           27   // PIN for SCL&SDA , relay 6
//#define Relay_I2C_SCL       27   // PIN for SCL&SDA , relay 6
//#define Relay_I2C_SDA       28   // PIN for SCL&SDA , relay 6

#define pin_MDIO           36   // PIN for MDIO
#define pin_MDC            37   // PIN for MDC
#define FT_PATTERN_bit0    49   // PIN for FT0
#define FT_PATTERN_bit1    48   // PIN for FT1
#define Relay_MDIO         35   // PIN for MDIO relay, relay 7

/***********************************************/
/* Use for I/O register PORTA control */
#define POR_Relay_Tx_PN    D22   // use for PORTA control, relay 1
#define POR_Relay_R_R50    D23   // use for PORTA control, relay 2
#define POR_Relay_Tx_Vol   D24   // use for PORTA control, relay 3
#define POR_Relay_Rx_Vol   D25   // use for PORTA control, relay 4
#define POR_Relay_DUT_GND  D26   // use for PORTA control, relay 5
//#define POR_Relay_I2C      D27   // use for PORTA control, relay 6
#define POR_Relay_I2C_SCL  D27   // use for PORTA control, relay 6
#define POR_Relay_I2C_SDA  D28   // use for PORTA control, relay 7

/* Use for I/O register PORTC control */
#define POR_MDIO               D36  // use for PORTC control
#define POR_MDC                D37  // use for PORTC control
#define POR_Relay_MDIO         D35  // use for PORTC control, relay 7

/* Use for I/O register PORTL control */
#define POR_FT_PATTERN_bit0    D49  // use for PORTL control
#define POR_FT_PATTERN_bit1    D48  // use for PORTL control


/* I/O register Port A */
#define D22   0
#define D23   1
#define D24   2
#define D25   3
#define D26   4
#define D27   5
#define D28   6
#define D29   7

/* I/O register Port C */
#define D37   0
#define D36   1
#define D35   2
#define D34   3
#define D33   4
#define D32   5
#define D31   6
#define D30   7

/* I/O register Port L */
#define D49   0
#define D48   1
#define D47   2
#define D46   3
#define D45   4
#define D44   5
#define D43   6
#define D42   7

/* I/O register Port D */
#define D21   0
#define D20   1
#define D19   2
#define D18   3


/***************************************************************************
**************************************************************************
* MDC/MDIO
***************************************************************************
***************************************************************************/
#define SET_HIGH(data, nbit) ((data)|=(nbit))
#define SET_LOW(data, nbit) ((data)&=~(nbit))

#define MDIO_ONE  _BV(POR_MDIO)
#define MDIO_ZERO 0x00
#define MDC_ONE   _BV(POR_MDC)
#define MDC_ZERO  0x00

#define delay_us delayMicroseconds(0)

#define ANACAL_INIT        0x01
#define ANACAL_ERROR       0xFD
#define ANACAL_SATURATION  0xFE
#define ANACAL_FINISH      0xFF
#define ANACAL_PAIR_A      0
#define ANACAL_PAIR_B      1
#define ANACAL_PAIR_C      2
#define ANACAL_PAIR_D      3
#define DAC_IN_0V          0x000
#define DAC_IN_2V          0x0f0  // +/-1V

#define ZCAL_MIDDLE        0x20
#define TX_OFFSET_0mV_idx  31
#define TX_AMP_MIDDLE      0x20

#define TX_i2mpb_hbt_ofs  0x4   // 8851 fine tune 100M v1 (20220414)
#define R50_OFFSET_VALUE  0x5

//============== definition value for GbE ===================//
#define BG_VOLTAGE_OUT     0xc0
#define FORCE_MDI          2
#define FORCE_MDIX         3
#define LDO_1p15_VOSEL_1   1
#define RX_CAL_VALUE_9       0x3
#define RX_CAL_HVGA_BW_2     0x2
#define RX_CAL_DCO_Normal    0x0
#define RX_CAL_DCO_BYPASS_TX_RX  0x3
#define RX_CAL_DCO_0xF    0xF

#define TANA_MON_DCV_SEL__MASK         0xE0
#define TANA_MON_DCV_SEL__MPX_TANA_A   0x20
#define TANA_MON_DCV_SEL__MPX_TANA_B   0x40
#define TANA_MON_DCV_SEL__MPX_TANA_C   0x60
#define TANA_MON_DCV_SEL__MPX_TANA_D   0x80
#define TANA_MON_DCV_SEL__MONVC__MASK  0x008000C8
#define TANA_MON_DCV__TANA__VBG_MON    0x000000C0
#define TANA_MON_DCV__TANA__MONVC      0x000000C8

#define AN_disable_force_1000M 0x0140
#define BG_voltage_output 0xc000
#define Fix_mdi 0x1010
#define Disable_tx_slew_control 0x0000
#define LDO_control 0x0100
#define Cal_control_BG 0x1110
#define Cal_control_R50 0x1100
#define Cal_control_TX_AMP 0x1100
#define Cal_control_TX_OFST 0x0100
#define Cal_control_R50_pairA_ENABLE 0x1101
#define Disable_all 0x0
#define Zcalen_A_ENABLE 0x0000
#define Zcalen_B_ENABLE 0x1000
#define Zcalen_C_ENABLE 0x0100
#define Zcalen_D_ENABLE 0x0010
#define MASK_MSB_8bit 0xff00
#define MASK_LSB_8bit 0x00ff
#define MASK_r50ohm_rsel_tx_a 0x7f00
#define MASK_r50ohm_rsel_tx_b 0x007f
#define MASK_r50ohm_rsel_tx_c 0x7f00
#define MASK_r50ohm_rsel_tx_d 0x007f
#define Rg_r50ohm_rsel_tx_a_en 0x8000
#define Rg_r50ohm_rsel_tx_b_en 0x0080
#define Rg_r50ohm_rsel_tx_c_en 0x8000
#define Rg_r50ohm_rsel_tx_d_en 0x0080
#define Rg_txvos_calen_ENABLE 0x0001
#define Bypass_tx_offset_cal 0x8000
#define Enable_Tx_VLD 0xf808
#define Rg_txg_calen_a_ENABLE 0x1000
#define Rg_txg_calen_b_ENABLE 0x0100
#define Rg_txg_calen_c_ENABLE 0x0010
#define Rg_txg_calen_d_ENABLE 0x0001
#define Force_dasn_dac_in0_ENABLE 0x8000
#define Force_dasn_dac_in1_ENABLE 0x8000
#define MASK_cr_tx_amp_offset_MSB 0x3f00
#define MASK_cr_tx_amp_offset_LSB 0x003f
#define Rg_cal_refsel_ENABLE 0x0010
#define MASK_da_tx_i2mpb_a_gbe 0xfc00
#define MASK_da_tx_i2mpb_b_c_d_gbe 0x3f00

#define LED_basic_control_en_active_low 0x800a
#define LED_led0_en_active_high 0xc007
#define LED_led0_force_blinking 0x0200



/*phy calibration use*/
//Type defines
typedef unsigned char    UINT8;
typedef unsigned short   UINT16;
typedef unsigned long    UINT32;

typedef struct
{
  UINT16 DATA_Lo;
  UINT8  DATA_Hi;
}TR_DATA_T;

//CL22 Reg Support Page Select//
#define RgAddr_Reg1Fh        0x1f
#define CL22_Page_Reg        0x0000
#define CL22_Page_ExtReg     0x0001
#define CL22_Page_MiscReg    0x0002
#define CL22_Page_LpiReg     0x0003
#define CL22_Page_tReg       0x02A3
#define CL22_Page_TrReg      0x52B5

//CL45 Reg Support DEVID//
#define DEVID_03             0x03
#define DEVID_07             0x07
#define DEVID_1E             0x1E
#define DEVID_1F             0x1F

//TokenRing Reg Access//
#define TrReg_PKT_XMT_STA    0x8000
#define TrReg_WR             0x8000
#define TrReg_RD             0xA000

/* ----------------- gephy_all Bit Field Definitions ------------------- */



//-------------------------------------
//0x0000
#define RgAddr_Reg00h                               0x00

//0x51e01200
#define RgAddr_dev1Eh_reg120h                       0x0120
//0x51e01220
#define RgAddr_dev1Eh_reg122h                       0x0122
//0x51e01440
#define RgAddr_dev1Eh_reg144h                       0x0144
//0x51e014a0
#define RgAddr_dev1Eh_reg14Ah                       0x014a
//0x51e019b0
#define RgAddr_dev1Eh_reg19Bh                       0x019b
//0x51e02340
#define RgAddr_dev1Eh_reg234h                       0x0234
//0x51e02380
#define RgAddr_dev1Eh_reg238h                       0x0238
//0x51e02390
#define RgAddr_dev1Eh_reg239h                       0x0239
//0x51f02680
#define RgAddr_dev1Fh_reg268h                       0x0268
//0x51e02d10
#define RgAddr_dev1Eh_reg2D1h                       0x02d1
//0x51e03230
#define RgAddr_dev1Eh_reg323h                       0x0323
//0x51e03240
#define RgAddr_dev1Eh_reg324h                       0x0324
//0x51e03260
#define RgAddr_dev1Eh_reg326h                       0x0326

//0x51f01000
#define RgAddr_dev1Fh_reg100h                       0x0100
//0x51e01450
#define RgAddr_dev1Eh_reg145h                       0x0145
//0x51f00ff0
#define RgAddr_dev1Fh_reg0FFh                       0x00ff
//0x51e00db0
#define RgAddr_dev1Eh_reg0DBh                       0x00db
//0x51e00dc0
#define RgAddr_dev1Eh_reg0DCh                       0x00dc
//0x51e00e00
#define RgAddr_dev1Eh_reg0E0h                       0x00e0
//0x51e00e10
#define RgAddr_dev1Eh_reg0E1h                       0x00e1
//0x51e00e00
#define RgAddr_dev1Eh_reg0E0h                       0x00e0
//0x51e017a0
#define RgAddr_dev1Eh_reg17Ah                       0x017a
//0x51f01150
#define RgAddr_dev1Fh_reg115h                       0x0115
//0x51f01000
#define RgAddr_dev1Fh_reg100h                       0x0100
//0x51e01450
#define RgAddr_dev1Eh_reg145h                       0x0145
//0x51e01450
#define RgAddr_dev1Eh_reg145h                       0x0145
//0x51e01850
#define RgAddr_dev1Eh_reg185h                       0x0185
//0x51e00fb0
#define RgAddr_dev1Eh_reg0FBh                       0x00fb
//0x51e01740
#define RgAddr_dev1Eh_reg174h                       0x0174
//0x51e01750
#define RgAddr_dev1Eh_reg175h                       0x0175
//0x51e01850
#define RgAddr_dev1Eh_reg185h                       0x0185
//0x51e00fb0
#define RgAddr_dev1Eh_reg0FBh                       0x00fb
//0x51e00960
#define RgAddr_dev1Eh_reg096h                       0x0096
//0x51e003e0
#define RgAddr_dev1Eh_reg03Eh                       0x003e
//0x51e00dd0
#define RgAddr_dev1Eh_reg0DDh                       0x00dd
//0x51e017d0
#define RgAddr_dev1Eh_reg17Dh                       0x017d
//0x51e01810
#define RgAddr_dev1Eh_reg181h                       0x0181
//0x51e00120
#define RgAddr_dev1Eh_reg012h                       0x0012
//0x51e017e0
#define RgAddr_dev1Eh_reg17Eh                       0x017e
//0x51e01820
#define RgAddr_dev1Eh_reg182h                       0x0182
//0x51e00170
#define RgAddr_dev1Eh_reg017h                       0x0017
//0x51e01830
#define RgAddr_dev1Eh_reg183h                       0x0183
//0x51e00190
#define RgAddr_dev1Eh_reg019h                       0x0019
//0x51e01800
#define RgAddr_dev1Eh_reg180h                       0x0180
//0x51e01840
#define RgAddr_dev1Eh_reg184h                       0x0184
//0x51e00210
#define RgAddr_dev1Eh_reg021h                       0x0021
//0x51e01720
#define RgAddr_dev1Eh_reg172h                       0x0172
//0x51e01730
#define RgAddr_dev1Eh_reg173h                       0x0173
//0x51e017c0
#define RgAddr_dev1Eh_reg17Ch                       0x017c
//0x51e017f0
#define RgAddr_dev1Eh_reg17Fh                       0x017f

//0x52b5100
#define RgAddr_TrReg10h                             0x10
//0x52b5110
#define RgAddr_TrReg11h                             0x11
//0x52b5120
#define RgAddr_TrReg12h                             0x12

//0x31c0
#define RgAddr_LpiReg1Ch                            0x1c
//0x31d0
#define RgAddr_LpiReg1Dh                            0x1d
uint8_t BG_Calibration(uint8_t phyadd, int8_t calipolarity);
uint8_t R50_Calibration(uint8_t phyadd, uint8_t phyadd_common);
uint8_t TX_OFS_Calibration(uint8_t phyadd, uint8_t phyadd_common);
uint8_t TX_AMP_Calibration(uint8_t phyadd, uint8_t phyadd_common);
//void config_gphy_port(UINT8, UINT8);

void set_gphy_reg_cl22(uint8_t, uint8_t, uint16_t);
uint16_t get_gphy_reg_cl45(uint8_t, uint8_t, uint16_t);
void set_gphy_reg_cl45(uint8_t, uint8_t, uint16_t, uint16_t);
void anacal_exe(uint8_t);

#endif /* _AN8855_PHY_H_ */


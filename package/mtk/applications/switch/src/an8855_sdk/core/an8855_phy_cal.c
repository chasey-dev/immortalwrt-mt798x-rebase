/* FILE NAME:  an8855_phy_cal.c
* PURPOSE:
*    It provides an8855 switch phy calibration function.
*
* NOTES:
*
*/

/* INCLUDE FILE DECLARATIONS
*/
#include "an8855_mdio.h"
#include "an8855_phy.h"
//#include "swk_gphy_reg.h"
//#include "gphy_calibration.h"
//#include "gsw_reg.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MII_BMCR                (0)
#define BMCR_PDOWN              (0x0800)
/* MACRO FUNCTION DECLARATIONS
 */

#define FULL_BITS(_n_) ((1UL << (_n_)) - 1)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
/* Zcal to R50 mapping table (20220404) */
const uint8_t ZCAL_TO_R50ohm_TBL[64] =
{
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 123, 118, 114, 110, 106, 102, 98, 96, 92, 88, 85,
    82, 80, 76, 72, 70, 67, 64, 62, 60, 56, 54, 52, 49, 48, 45, 43,
    40, 39, 36, 34, 32, 32, 30, 28, 25, 24, 22, 20, 18, 16, 16, 14
};

/* Tx offset table, value is from small to big */
const uint8_t  EN753x_TX_OFS_TBL[64] =
{
    0x3f, 0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
    0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

#define TOTAL_PATCH_C45_ITEMS   (15)
#define TOTAL_PATCH_TR_ITEMS    (19)
const uint16_t C45_PATCH_TABLE[TOTAL_PATCH_C45_ITEMS][3] =
{
    {0x1E, 0x120, 0x8014},
    {0x1E, 0x122, 0xFFFF},
    {0x1E, 0x122, 0xFFFF},
    {0x1E, 0x144, 0x0200},
    {0x1E, 0x14A, 0xEE20},
    {0x1E, 0x189, 0x0110},
    {0x1E, 0x19B, 0x0111},
    {0x1E, 0x234, 0x0181},
    {0x1E, 0x238, 0x0120},
    {0x1E, 0x239, 0x0117},
    {0x1F, 0x268, 0x07F4},
    {0x1E, 0x2d1, 0x0733},
    {0x1E, 0x323, 0x0011},
    {0x1E, 0x324, 0x013F},
    {0x1E, 0x326, 0x0037},
};

const uint32_t TR_PATCH_TABLE[TOTAL_PATCH_TR_ITEMS][2] =
{
    {0x83AA, 0x055a0 },
    {0x83AE, 0x7FF3F },
    {0x8F80, 0x0001e },
    {0x8F82, 0x6FB90A},
    {0x8FAE, 0x060671},
    {0x8FB0, 0xE2F00 },
    {0x8ECC, 0x444444},
    {0x9686, 0x00000 },
    {0x968C, 0x2EBAEF},
    {0x9690, 0x00000b},
    {0x9698, 0x0504D },
    {0x969A, 0x2314f },
    {0x969E, 0x03028 },
    {0x96A0, 0x05010 },
    {0x96A2, 0x40001 },
    {0x96A6, 0x018670},
    {0x96A8, 0x0024A },
    {0x96B6, 0x00072 },
    {0x96B8, 0x03210 },
};

#define TOTAL_NUMBER_OF_PATCH    (14)
static uint16_t eee_patch_table[TOTAL_NUMBER_OF_PATCH][2] = {
    {RgAddr_dev1Eh_reg120h, 0x8014},
    {RgAddr_dev1Eh_reg122h, 0xFFFF},
    {RgAddr_dev1Eh_reg122h, 0xFFFF},
    {RgAddr_dev1Eh_reg144h, 0x0200},
    {RgAddr_dev1Eh_reg14Ah, 0xEE20},
    {RgAddr_dev1Eh_reg19Bh, 0x0111},
    {RgAddr_dev1Eh_reg234h, 0x1181},
    {RgAddr_dev1Eh_reg238h, 0x0120},
    {RgAddr_dev1Eh_reg239h, 0x0117},
    {RgAddr_dev1Fh_reg268h, 0x07F4},
    {RgAddr_dev1Eh_reg2D1h, 0x0733},
    {RgAddr_dev1Eh_reg323h, 0x0011},
    {RgAddr_dev1Eh_reg324h, 0x013F},
    {RgAddr_dev1Eh_reg326h, 0x0037}
};

#define TOTAL_NUMBER_OF_TR      (19)
static uint16_t tr_reg_table[TOTAL_NUMBER_OF_TR][3] = {
    {0x55A0, 0x0000, 0x83AA},
    {0xFF3F, 0x0007, 0x83AE},
    {0x001E, 0x0000, 0x8F80},
    {0xB90A, 0x006F, 0x8F82},
    {0x0671, 0x0006, 0x8FAE},
    {0x2F00, 0x000E, 0x8FB0},
    {0x4444, 0x0044, 0x8ECC},
    {0x0004, 0x0000, 0x9686},
    {0xBAEF, 0x002E, 0x968C},
    {0x000B, 0x0000, 0x9690},
    {0x504D, 0x0000, 0x9698},
    {0x314F, 0x0002, 0x969A},
    {0x3028, 0x0000, 0x969E},
    {0x5010, 0x0000, 0x96A0},
    {0x0001, 0x0004, 0x96A2},
    {0x8670, 0x0001, 0x96A6},
    {0x024A, 0x0000, 0x96A8},
    {0x0072, 0x0000, 0x96B6},
    {0x3210, 0x0000, 0x96B8}
};

void TR_RegWr(uint16_t phyadd, uint16_t tr_reg_addr, uint32_t tr_data);

uint16_t get_gphy_reg_cl22(uint8_t phyad, uint8_t reg)
{
    uint32_t rdata = 0;

    an8855_phy_read(phyad-g_smi_addr, reg, &rdata);

    return ((uint16_t)rdata);
    /*
    gsw_top_reg_REG_PHY_IAC REG_PHY_IAC_val;
    gsw_top_reg_REG_PHY_IAD REG_PHY_IAD_val;

    // Wait until done
    do
    {
      REG_PHY_IAC_val.Raw   = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    // Set address
    REG_PHY_IAC_val.Bits.csr_mdio_st = 1;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd   = 2;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = phyad;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = reg;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st =   1;
    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);
    // Wait until done
    do
    {
        REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    REG_PHY_IAD_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAD);

    return REG_PHY_IAD_val.Raw;
    */
}

/* EXPORTED SUBPROGRAM BODIES
 */
void gphy_config(void)
{
    uint8_t port = 1;
    uint8_t phy_base = 0, phys_in_chip = 8;

    for (port = 1; port <= phys_in_chip; port++)
    {
        set_gphy_reg_cl45(phy_base + port, 0x7, 0x3c, 0x0006); // Enable EEE
        set_gphy_reg_cl45(phy_base + port, 0x1e, 0x3e, 0xf000); // force on TXVLD
    }
}

static void set_gphy_TrReg(uint8_t prtid, uint16_t parm_1, uint16_t parm_2, uint16_t parm_3)
{
    set_gphy_reg_cl22(prtid, RgAddr_TrReg11h, parm_1);
    set_gphy_reg_cl22(prtid, RgAddr_TrReg12h, parm_2);
    set_gphy_reg_cl22(prtid, RgAddr_TrReg10h, parm_3);
}

static void gphy_eee_patch(uint8_t phy_base)
{
    UI8_T   port = 1, index = 0, phy_addr = 1;
    UI16_T  data = 0;

    for (port = 1; port <=8; port++)
    {
        phy_addr = phy_base + port;
        data = get_gphy_reg_cl22(phy_addr, MII_BMCR);
        set_gphy_reg_cl22(phy_addr, MII_BMCR, data & ~(BMCR_PDOWN));    /* PHY power on */

        /* Change EEE RG default value */
        for (index = 0; index < TOTAL_NUMBER_OF_PATCH; index++)
        {
            set_gphy_reg_cl45(phy_addr, DEVID_1E, eee_patch_table[index][0], eee_patch_table[index][1]);
        }

        set_gphy_reg_cl22(phy_addr, RgAddr_Reg1Fh, CL22_Page_TrReg);   /* change CL22page to LpiReg(0x3) */
        for (index = 0; index < TOTAL_NUMBER_OF_TR; index++)
        {
            set_gphy_TrReg(phy_addr, tr_reg_table[index][0], tr_reg_table[index][1], tr_reg_table[index][2]);
        }

        set_gphy_reg_cl22(phy_addr, RgAddr_Reg1Fh, CL22_Page_LpiReg);  /* change CL22page to LpiReg(0x3) */
        set_gphy_reg_cl22(phy_addr, RgAddr_LpiReg1Ch, 0x0c92);         /* Fine turn SigDet for B2B LPI link down issue */
        set_gphy_reg_cl22(phy_addr, RgAddr_LpiReg1Dh, 0x0001);         /* Enable "lpi_quit_waitafesigdet_en" for LPI link down issue */

        set_gphy_reg_cl22(phy_addr, RgAddr_Reg1Fh, CL22_Page_Reg);     /* change CL22page to Reg(0x0) */
    }
}

void gphy_calibration(uint8_t phy_base)
{
    uint8_t port = 1, phy_addr = 1 ,phy_group = 1, index = 0;
    uint8_t phys_in_chip = 5;

    BG_Calibration(phy_base, 0x1);
    if (phys_in_chip > 4)
    {
        BG_Calibration(phy_base + 0x4, 0x1);
    }

    for (port = 0; port < phys_in_chip; port++)
    {
        if (port < 4)
        {
            phy_group = phy_base;     /* PHY group 1 */
        }
        else
        {
            phy_group = phy_base + 0x04;     /* PHY group 2 */
        }
        phy_addr = phy_base + port;
        R50_Calibration(phy_addr, phy_group);
        TX_OFS_Calibration(phy_addr, phy_group);
        TX_AMP_Calibration(phy_addr, phy_group);
    }

    for (port = 0; port < phys_in_chip; port++)
    {
        phy_addr = phy_base + port;
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x017d, 0x0000);
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x017e, 0x0000);
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x017f, 0x0000);
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x0180, 0x0000);
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x0181, 0x0000);
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x0182, 0x0000);
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x0183, 0x0000);
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x0184, 0x0000);
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x00db, 0x0000);  // disable analog calibration circuit
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x00dc, 0x0000);  // disable Tx offset calibration circuit
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x003e, 0x0000);  // disable Tx VLD force mode
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x00dd, 0x0000);  // disable Tx offset/amplitude calibration circuit
        set_gphy_reg_cl45(phy_addr, 0x1e, 0x0145, 0x1000);  // enable auto MDI/MDIX

        set_gphy_reg_cl22(phy_addr, 0, 0x1200);
        /* GPHY Rx low pass filter */
        set_gphy_reg_cl45(phy_addr, 0x1e, 0xc7, 0xd000);
        /* patch */
        for (index = 0; index < TOTAL_PATCH_C45_ITEMS; index++)
        {
            set_gphy_reg_cl45(phy_addr, C45_PATCH_TABLE[index][0], C45_PATCH_TABLE[index][1], C45_PATCH_TABLE[index][2]);
        }
        for (index = 0; index < TOTAL_PATCH_TR_ITEMS; index++)
        {
            TR_RegWr(phy_addr, TR_PATCH_TABLE[index][0], TR_PATCH_TABLE[index][1]);
        }
        set_gphy_reg_cl22(phy_addr, 0x1f, 0x0  );
        set_gphy_reg_cl22(phy_addr, 0x1f, 0x3  );
        set_gphy_reg_cl22(phy_addr, 0x1c, 0xc92);
        set_gphy_reg_cl22(phy_addr, 0x1d, 0x01 );
        set_gphy_reg_cl22(phy_addr, 0x1f, 0x0  );
    }
    gphy_eee_patch(phy_base);
}

/* LOCAL SUBPROGRAM BODIES
 */
void TR_RegWr(uint16_t phyadd, uint16_t tr_reg_addr, uint32_t tr_data)
{
    set_gphy_reg_cl22(phyadd, 0x1F, 0x52b5);       /* page select */
    set_gphy_reg_cl22(phyadd, 0x11, (uint16_t)(tr_data & 0xffff));
    set_gphy_reg_cl22(phyadd, 0x12, (uint16_t)(tr_data >> 16));
    set_gphy_reg_cl22(phyadd, 0x10, (uint16_t)(tr_reg_addr | TrReg_WR));
    set_gphy_reg_cl22(phyadd, 0x1F, 0x0);          /* page resetore */
    return;
}

uint8_t BG_Calibration(uint8_t phyadd, int8_t calipolarity)
{
    int8_t rg_zcal_ctrl = 0, calibration_polarity = 0;
    uint8_t all_ana_cal_status = 1;
    uint16_t ad_cal_comp_out_init = 0;

    /* setting */
    set_gphy_reg_cl22(phyadd, RgAddr_Reg1Fh, CL22_Page_Reg);        // g0
    set_gphy_reg_cl22(phyadd, RgAddr_Reg00h, AN_disable_force_1000M);  // AN disable, force 1000M
    set_gphy_reg_cl45(phyadd, DEVID_1F, RgAddr_dev1Fh_reg100h, BG_voltage_output);// BG voltage output
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg145h, Fix_mdi);// fix mdi
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Fh_reg0FFh, 0x2);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Cal_control_BG);// 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Disable_all);// 1e_dc[0]:rg_txvos_calen
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0E1h, Disable_all);// 1e_e1[4]:rg_cal_refsel(0:1.2V) enable BG 1.2V to REXT PAD

    /* calibrate */
    rg_zcal_ctrl = ZCAL_MIDDLE;

    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0E0h, (uint16_t)rg_zcal_ctrl);

    anacal_exe(phyadd);
    if (all_ana_cal_status == 0)
    {
        all_ana_cal_status = ANACAL_ERROR;
    }
    ad_cal_comp_out_init = (get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Ah) >> 8) & 0x1;


    if (ad_cal_comp_out_init == 1)
    {
        calibration_polarity = -calipolarity;
    }
    else // ad_cal_comp_out_init == 0
    {
        calibration_polarity = calipolarity;
    }

    while (all_ana_cal_status < ANACAL_ERROR)
    {
        rg_zcal_ctrl += calibration_polarity;

        set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0E0h, (uint16_t)rg_zcal_ctrl);


        anacal_exe(phyadd);

        if (all_ana_cal_status == 0)
        {
            all_ana_cal_status = ANACAL_ERROR;
        }

        else if (((get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Ah) >> 8) & 0x1) != ad_cal_comp_out_init)
        {
            all_ana_cal_status = ANACAL_FINISH;
        }
        else
        {
            if ((rg_zcal_ctrl == 0x3F) || (rg_zcal_ctrl == 0x00))
            {
                all_ana_cal_status = ANACAL_SATURATION;  // need to FT
                rg_zcal_ctrl = ZCAL_MIDDLE;  // 0 dB
            }
        }
    }

    if (all_ana_cal_status == ANACAL_ERROR)
    {
        rg_zcal_ctrl = ZCAL_MIDDLE;  // 0 dB

        set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0E0h, (uint16_t)rg_zcal_ctrl);
    }
    else
    {
        // rg_zcal_ctrl[5:0] rg_rext_trim[13:8]
        set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0E0h, (uint16_t)((rg_zcal_ctrl << 8) | rg_zcal_ctrl));

        // 1f_115[2:0](rg_bg_rasel) = rg_zcal_ctrl[5:3]
        set_gphy_reg_cl45(phyadd, DEVID_1F, RgAddr_dev1Fh_reg115h, (uint16_t)((rg_zcal_ctrl & 0x3f) >> 3));
    }

    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Disable_all);
    return all_ana_cal_status;
}

uint8_t R50_Calibration(uint8_t phyadd, uint8_t phyadd_common)
{
    int8_t rg_zcal_ctrl = 0, rg_r50ohm_rsel_tx = 0, calibration_polarity = 0;
    uint8_t all_ana_cal_status = 1;
    int16_t backup_dev1e_e0 = 0, ad_cal_comp_out_init = 0, calibration_pair = 0;

    /* setting */
    set_gphy_reg_cl22(phyadd, RgAddr_Reg1Fh, CL22_Page_Reg);        // g0
    set_gphy_reg_cl22(phyadd, RgAddr_Reg00h, AN_disable_force_1000M);  // AN disable, force 1000M

    set_gphy_reg_cl45(phyadd_common, DEVID_1F, RgAddr_dev1Fh_reg100h, BG_voltage_output); // BG voltage output
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg145h, Fix_mdi); // fix mdi
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg185h, Disable_tx_slew_control); // disable tx slew control
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0FBh, LDO_control); // ldo
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Cal_control_R50); // 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Disable_all); // 1e_dc[0]:rg_txvos_calen
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0E1h, Disable_all); // 1e_e1[4]:rg_cal_refsel(0:1.2V) enable BG 1.2V to REXT PAD

    for (calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_D; calibration_pair++)
    {
        all_ana_cal_status = 1;

        if (calibration_pair == ANACAL_PAIR_A)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Cal_control_R50_pairA_ENABLE); // 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Zcalen_A_ENABLE);
        }
        else if (calibration_pair == ANACAL_PAIR_B)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Cal_control_R50); // 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Zcalen_B_ENABLE); // 1e_dc[12]:rg_zcalen_b
        }
        else if (calibration_pair == ANACAL_PAIR_C)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Cal_control_R50); // 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Zcalen_C_ENABLE); // 1e_dc[8]:rg_zcalen_c
        }
        else // if(calibration_pair == ANACAL_PAIR_D)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Cal_control_R50); // 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Zcalen_D_ENABLE); // 1e_dc[4]:rg_zcalen_d
        }

        /* calibrate */
        rg_zcal_ctrl = ZCAL_MIDDLE;             // start with 0 dB

        backup_dev1e_e0 = (get_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0E0h)&(~0x003f));
        set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0E0h, (backup_dev1e_e0 | rg_zcal_ctrl));

        anacal_exe(phyadd_common);
        if (all_ana_cal_status == 0)
        {
            all_ana_cal_status = ANACAL_ERROR;
        }

        ad_cal_comp_out_init = (get_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg17Ah) >> 8) & 0x1;  // 1e_17a[8]:ad_cal_comp_out

        if (ad_cal_comp_out_init == 1)
        {
            calibration_polarity = -1;
        }
        else
        {
            calibration_polarity = 1;
        }

        while (all_ana_cal_status < ANACAL_ERROR)
        {
            rg_zcal_ctrl += calibration_polarity;

            set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0E0h, (backup_dev1e_e0 | rg_zcal_ctrl));

            anacal_exe(phyadd_common);

            if (all_ana_cal_status == 0)
            {
                all_ana_cal_status = ANACAL_ERROR;
            }
            else if (((get_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg17Ah) >> 8) & 0x1) != ad_cal_comp_out_init)
            {
                all_ana_cal_status = ANACAL_FINISH;
            }
            else
            {
                if ((rg_zcal_ctrl == 0x3F) || (rg_zcal_ctrl == 0x00))
                {
                    all_ana_cal_status = ANACAL_SATURATION;  // need to FT
                    rg_zcal_ctrl = ZCAL_MIDDLE;  // 0 dB
                }
            }
        }

        if (all_ana_cal_status == ANACAL_ERROR)
        {
            rg_r50ohm_rsel_tx = ZCAL_MIDDLE;  // 0 dB
        }
        else
        {
            if (rg_zcal_ctrl > (0x3F - R50_OFFSET_VALUE))
            {
                all_ana_cal_status = ANACAL_SATURATION;  // need to FT
                rg_zcal_ctrl = ZCAL_MIDDLE;  // 0 dB
            }
            else
            {
                rg_zcal_ctrl += R50_OFFSET_VALUE;
            }

            rg_r50ohm_rsel_tx = ZCAL_TO_R50ohm_TBL[rg_zcal_ctrl];
        }

        if (calibration_pair == ANACAL_PAIR_A)
        {
            // cr_r50ohm_rsel_tx_a
            ad_cal_comp_out_init = get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg174h)&(~MASK_r50ohm_rsel_tx_a);
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg174h, (ad_cal_comp_out_init | (((rg_r50ohm_rsel_tx << 8) & MASK_MSB_8bit) | Rg_r50ohm_rsel_tx_a_en))); // 1e_174[15:8]
        }
        else if (calibration_pair == ANACAL_PAIR_B)
        {
            // cr_r50ohm_rsel_tx_b
            ad_cal_comp_out_init = get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg174h)&(~MASK_r50ohm_rsel_tx_b);
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg174h, (ad_cal_comp_out_init | (((rg_r50ohm_rsel_tx << 0) & MASK_LSB_8bit) | Rg_r50ohm_rsel_tx_b_en))); // 1e_174[7:0]
        }
        else if (calibration_pair == ANACAL_PAIR_C)
        {
            // cr_r50ohm_rsel_tx_c
            ad_cal_comp_out_init = get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg175h)&(~MASK_r50ohm_rsel_tx_c);
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg175h, (ad_cal_comp_out_init | (((rg_r50ohm_rsel_tx << 8) & MASK_MSB_8bit) | Rg_r50ohm_rsel_tx_c_en))); // 1e_175[15:8]
        }
        else // if(calibration_pair == ANACAL_PAIR_D)
        {
            // cr_r50ohm_rsel_tx_d
            ad_cal_comp_out_init = get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg175h)&(~MASK_r50ohm_rsel_tx_d);
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg175h, (ad_cal_comp_out_init | (((rg_r50ohm_rsel_tx << 0) & MASK_LSB_8bit) | Rg_r50ohm_rsel_tx_d_en))); // 1e_175[7:0]
        }
    }

    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Disable_all);
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Disable_all);

    return all_ana_cal_status;
}

uint8_t TX_OFS_Calibration(uint8_t phyadd, uint8_t phyadd_common)
{
    int8_t tx_offset_index = 0, calibration_polarity = 0;
    uint8_t all_ana_cal_status = 1, tx_offset_reg_shift = 0, tbl_idx = 0;
    int16_t ad_cal_comp_out_init = 0, calibration_pair = 0, tx_offset_reg = 0, reg_temp = 0;

    /* setting */
    set_gphy_reg_cl22(phyadd, RgAddr_Reg1Fh, CL22_Page_Reg);        // g0
    set_gphy_reg_cl22(phyadd, RgAddr_Reg00h, AN_disable_force_1000M);  // AN disable, force 1000M

    set_gphy_reg_cl45(phyadd, DEVID_1F, RgAddr_dev1Fh_reg100h, BG_voltage_output); // BG voltage output
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg145h, Fix_mdi); // fix mdi
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg185h, Disable_tx_slew_control); // disable tx slew control
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0FBh, LDO_control); // ldo
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Cal_control_TX_OFST); // 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Rg_txvos_calen_ENABLE); // 1e_dc[0]:rg_txvos_calen
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Cal_control_TX_OFST); // 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Rg_txvos_calen_ENABLE); // 1e_dc[0]:rg_txvos_calen
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0E1h, Disable_all); // 1e_e1[4]:rg_cal_refsel(0:1.2V) enable BG 1.2V to REXT PAD
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg096h, Bypass_tx_offset_cal); // 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg03Eh, Enable_Tx_VLD); // 1e_3e:enable Tx VLD

    for (calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_D; calibration_pair++)
    {
        all_ana_cal_status = 1;

        tbl_idx = TX_OFFSET_0mV_idx;
        tx_offset_index = EN753x_TX_OFS_TBL[tbl_idx];

        if (calibration_pair == ANACAL_PAIR_A)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DDh, Rg_txg_calen_a_ENABLE);       // 1e_dd[12]:rg_txg_calen_a
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Dh, (Force_dasn_dac_in0_ENABLE | DAC_IN_0V));  // 1e_17d:dac_in0_a
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg181h, (Force_dasn_dac_in1_ENABLE | DAC_IN_0V));  // 1e_181:dac_in1_a

            reg_temp = (get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg172h)&(~MASK_cr_tx_amp_offset_MSB));
            tx_offset_reg_shift = 8;  // 1e_172[13:8]
            tx_offset_reg = RgAddr_dev1Eh_reg172h;
        }
        else if (calibration_pair == ANACAL_PAIR_B)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DDh, Rg_txg_calen_b_ENABLE);       // 1e_dd[8]:rg_txg_calen_b
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Eh, (Force_dasn_dac_in0_ENABLE | DAC_IN_0V));  // 1e_17e:dac_in0_b
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg182h, (Force_dasn_dac_in1_ENABLE | DAC_IN_0V));  // 1e_182:dac_in1_b

            reg_temp = (get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg172h)&(~MASK_cr_tx_amp_offset_LSB));
            tx_offset_reg_shift = 0;  // 1e_172[5:0]
            tx_offset_reg = RgAddr_dev1Eh_reg172h;
        }
        else if (calibration_pair == ANACAL_PAIR_C)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DDh, Rg_txg_calen_c_ENABLE);       // 1e_dd[4]:rg_txg_calen_c
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Fh, (Force_dasn_dac_in0_ENABLE | DAC_IN_0V));  // 1e_17f:dac_in0_c
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg183h, (Force_dasn_dac_in1_ENABLE | DAC_IN_0V));  // 1e_183:dac_in1_c

            reg_temp = (get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg173h)&(~MASK_cr_tx_amp_offset_MSB));
            tx_offset_reg_shift = 8;  // 1e_173[13:8]
            tx_offset_reg = RgAddr_dev1Eh_reg173h;
        }
        else // if(calibration_pair == ANACAL_PAIR_D)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DDh, Rg_txg_calen_d_ENABLE);       // 1e_dd[0]:rg_txg_calen_d
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg180h, (Force_dasn_dac_in0_ENABLE | DAC_IN_0V));  // 1e_180:dac_in0_d
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg184h, (Force_dasn_dac_in1_ENABLE | DAC_IN_0V));  // 1e_184:dac_in1_d

            reg_temp = (get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg173h)&(~MASK_cr_tx_amp_offset_LSB));
            tx_offset_reg_shift = 0;  // 1e_173[5:0]
            tx_offset_reg = RgAddr_dev1Eh_reg173h;
        }

        /* calibrate */
        //tx_offset_index = TX_AMP_OFFSET_0mV;
        tbl_idx = TX_OFFSET_0mV_idx;
        tx_offset_index = EN753x_TX_OFS_TBL[tbl_idx];
        set_gphy_reg_cl45(phyadd, DEVID_1E, tx_offset_reg, (reg_temp | (tx_offset_index << tx_offset_reg_shift)));  // 1e_172, 1e_173

        anacal_exe(phyadd_common);
        if (all_ana_cal_status == 0)
        {
            all_ana_cal_status = ANACAL_ERROR;
        }

        ad_cal_comp_out_init = (get_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg17Ah) >> 8) & 0x1;  // 1e_17a[8]:ad_cal_comp_out

        if (ad_cal_comp_out_init == 1)
        {
            calibration_polarity = -1;
        }
        else
        {
            calibration_polarity = 1;
        }

        while (all_ana_cal_status < ANACAL_ERROR)
        {
            tbl_idx += calibration_polarity;
            tx_offset_index = EN753x_TX_OFS_TBL[tbl_idx];

            set_gphy_reg_cl45(phyadd, DEVID_1E, tx_offset_reg, (reg_temp | (tx_offset_index << tx_offset_reg_shift)));  // 1e_172, 1e_173

            anacal_exe(phyadd_common);

            if (all_ana_cal_status == 0)
            {
                all_ana_cal_status = ANACAL_ERROR;
            }
            else if (((get_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg17Ah) >> 8) & 0x1) != ad_cal_comp_out_init)
            {
                all_ana_cal_status = ANACAL_FINISH;
            }
            else
            {
                if ((tx_offset_index == 0x3f) || (tx_offset_index == 0x1f))
                {
                    all_ana_cal_status = ANACAL_SATURATION;  // need to FT
                }
            }
        }

        if (all_ana_cal_status == ANACAL_ERROR)
        {
            tbl_idx = TX_OFFSET_0mV_idx;
            tx_offset_index = EN753x_TX_OFS_TBL[tbl_idx];

            set_gphy_reg_cl45(phyadd, DEVID_1E, tx_offset_reg, (reg_temp | (tx_offset_index << tx_offset_reg_shift)));  // cr_tx_amp_offset_a/b/c/d, 1e_172, 1e_173
        }
    }

    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Dh, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Eh, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Fh, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg180h, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg181h, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg182h, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg183h, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg184h, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Disable_all); // disable analog calibration circuit
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Disable_all); // disable Tx offset calibration circuit
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Disable_all); // disable analog calibration circuit
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Disable_all); // disable Tx offset calibration circuit
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg03Eh, Disable_all); // disable Tx VLD force mode
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DDh, Disable_all); // disable Tx offset/amplitude calibration circuit

    return all_ana_cal_status;
}

uint8_t TX_AMP_Calibration(uint8_t phyadd, uint8_t phyadd_common)
{
    int8_t tx_amp_index = 0, calibration_polarity = 0;
    uint8_t all_ana_cal_status = 1, tx_amp_reg_shift = 0;
    uint8_t tx_amp_reg = 0, tx_amp_reg_100 = 0, tst_offset = 0, hbt_offset = 0, gbe_offset = 0, tbt_offset = 0;
    uint16_t ad_cal_comp_out_init = 0, calibration_pair = 0, reg_temp = 0;

  //phyadd_common = phyadd;

    /* setting */
    set_gphy_reg_cl22(phyadd, RgAddr_Reg1Fh, CL22_Page_Reg);        // g0
    set_gphy_reg_cl22(phyadd, RgAddr_Reg00h, AN_disable_force_1000M);  // AN disable, force 1000M

    set_gphy_reg_cl45(phyadd, DEVID_1F, RgAddr_dev1Fh_reg100h, BG_voltage_output); // BG voltage output
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg145h, Fix_mdi); // fix mdi
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg185h, Disable_tx_slew_control); // disable tx slew control
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0FBh, LDO_control); // ldo
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Cal_control_TX_AMP); // 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Rg_txvos_calen_ENABLE); // 1e_dc[0]:rg_txvos_calen
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0E1h, Rg_cal_refsel_ENABLE); // 1e_e1[4]:rg_cal_refsel(0:1.2V) enable BG 1.2V to REXT PAD
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Cal_control_TX_AMP); // 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Rg_txvos_calen_ENABLE); // 1e_dc[0]:rg_txvos_calen
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg0E1h, Rg_cal_refsel_ENABLE); // 1e_e1[4]:rg_cal_refsel(0:1.2V) enable BG 1.2V to REXT PAD
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg096h, Bypass_tx_offset_cal); // 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg03Eh, Enable_Tx_VLD); // 1e_3e:enable Tx VLD

    for (calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_D; calibration_pair++)
    //for (calibration_pair = ANACAL_PAIR_A; calibration_pair <= ANACAL_PAIR_B; calibration_pair++) // debugging
    {
        all_ana_cal_status = 1;

        /* calibrate */
        tx_amp_index = TX_AMP_MIDDLE;   // start with 0 dB
        if (calibration_pair == ANACAL_PAIR_A)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DDh, Rg_txg_calen_a_ENABLE);       // 1e_dd[12]:rg_txg_calen_a amp calibration enable
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Dh, (Force_dasn_dac_in0_ENABLE | DAC_IN_2V));  // 1e_17d:dac_in0_a
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg181h, (Force_dasn_dac_in1_ENABLE | DAC_IN_2V));  // 1e_181:dac_in1_a

            reg_temp = (get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg012h)&(~MASK_da_tx_i2mpb_a_gbe));
            tx_amp_reg_shift = 10;  // 1e_12[15:10]
            tx_amp_reg = RgAddr_dev1Eh_reg012h;
            tx_amp_reg_100 = 0x16;
        }
        else if (calibration_pair == ANACAL_PAIR_B)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DDh, Rg_txg_calen_b_ENABLE);       // 1e_dd[8]:rg_txg_calen_b amp calibration enable
            //Serial.println(Rg_txg_calen_b_ENABLE, HEX);
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Eh, (Force_dasn_dac_in0_ENABLE | DAC_IN_2V));  // 1e_17e:dac_in0_b
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg182h, (Force_dasn_dac_in1_ENABLE | DAC_IN_2V));  // 1e_182:dac_in1_b

            reg_temp = (get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg017h)&(~MASK_da_tx_i2mpb_b_c_d_gbe));
            tx_amp_reg_shift = 8; // 1e_17[13:8]
            tx_amp_reg = RgAddr_dev1Eh_reg017h;
            tx_amp_reg_100 = 0x18;
        }
        else if (calibration_pair == ANACAL_PAIR_C)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DDh, Rg_txg_calen_c_ENABLE);       // 1e_dd[4]:rg_txg_calen_c amp calibration enable
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Fh, (Force_dasn_dac_in0_ENABLE | DAC_IN_2V));  // 1e_17f:dac_in0_c
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg183h, (Force_dasn_dac_in1_ENABLE | DAC_IN_2V));  // 1e_183:dac_in1_c

            reg_temp = (get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg019h)&(~MASK_da_tx_i2mpb_b_c_d_gbe));
            tx_amp_reg_shift = 8; // 1e_19[13:8]
            tx_amp_reg = RgAddr_dev1Eh_reg019h;
            tx_amp_reg_100 = 0x20;
        }
        else //if(calibration_pair == ANACAL_PAIR_D)
        {
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DDh, Rg_txg_calen_d_ENABLE);       // 1e_dd[0]:rg_txg_calen_d amp calibration enable
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg180h, (Force_dasn_dac_in0_ENABLE | DAC_IN_2V));  // 1e_180:dac_in0_d
            set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg184h, (Force_dasn_dac_in1_ENABLE | DAC_IN_2V));  // 1e_184:dac_in1_d

            reg_temp = (get_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg021h)&(~MASK_da_tx_i2mpb_b_c_d_gbe));
            tx_amp_reg_shift = 8; // 1e_21[13:8]
            tx_amp_reg = RgAddr_dev1Eh_reg021h;
            tx_amp_reg_100 = 0x22;
        }

        /* calibrate */
        tx_amp_index = TX_AMP_MIDDLE; // start with 0 dB

        set_gphy_reg_cl45(phyadd, DEVID_1E, tx_amp_reg, (reg_temp | (tx_amp_index << tx_amp_reg_shift))); // 1e_12/17/19/21

        anacal_exe(phyadd_common);
        if (all_ana_cal_status == 0)
        {
            all_ana_cal_status = ANACAL_ERROR;
        }


        ad_cal_comp_out_init = (get_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg17Ah) >> 8) & 0x1;    // 1e_17a[8]:ad_cal_comp_out
        //Serial.println(ad_cal_comp_out_init, HEX);

        if (ad_cal_comp_out_init == 1)
        {
            calibration_polarity = -1;
        }
        else
        {
            calibration_polarity = 1;
        }
        while (all_ana_cal_status < ANACAL_ERROR)
        {
            tx_amp_index += calibration_polarity;
            //Serial.println(tx_amp_index, HEX);

            set_gphy_reg_cl45(phyadd, DEVID_1E, tx_amp_reg, (reg_temp | (tx_amp_index << tx_amp_reg_shift)));

            anacal_exe(phyadd_common);

            if (all_ana_cal_status == 0)
            {
                all_ana_cal_status = ANACAL_ERROR;
            }
            else if (((get_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg17Ah) >> 8) & 0x1) != ad_cal_comp_out_init)
            {
                all_ana_cal_status = ANACAL_FINISH;
                //Serial.print("    tx_amp_index: ");
                //Serial.println(tx_amp_index, HEX);
                //reg_temp = get_gphy_reg_cl45(phyadd, 0x1e, tx_amp_reg)&(~0xff00);
                //set_gphy_reg_cl45(phyadd, 0x1e, tx_amp_reg, (reg_temp|((tx_amp_index + tst_offset)<<tx_amp_reg_shift)));  // for gbe(DAC)
            }
            else
            {
                if ((tx_amp_index == 0x3f) || (tx_amp_index == 0x00))
                {
                    all_ana_cal_status = ANACAL_SATURATION;  // need to FT
                    tx_amp_index = TX_AMP_MIDDLE;
                }
            }
        }

        if (all_ana_cal_status == ANACAL_ERROR)
        {
            tx_amp_index = TX_AMP_MIDDLE;
        }

        // da_tx_i2mpb_a_gbe / b/c/d, only GBE for now
        set_gphy_reg_cl45(phyadd, DEVID_1E, tx_amp_reg, ((tx_amp_index - TXAMP_offset) | ((tx_amp_index - TXAMP_offset) << tx_amp_reg_shift)));  // // temp modify
        set_gphy_reg_cl45(phyadd, DEVID_1E, tx_amp_reg_100, ((tx_amp_index - TXAMP_offset) | ((tx_amp_index + TX_i2mpb_hbt_ofs) << tx_amp_reg_shift)));
    }

    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Dh, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Eh, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg17Fh, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg180h, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg181h, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg182h, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg183h, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg184h, Disable_all);
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DBh, Disable_all); // disable analog calibration circuit
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DCh, Disable_all); // disable Tx offset calibration circuit
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg03Eh, Disable_all); // disable Tx VLD force mode
    set_gphy_reg_cl45(phyadd, DEVID_1E, RgAddr_dev1Eh_reg0DDh, Disable_all); // disable Tx offset/amplitude calibration circuit

    return all_ana_cal_status;
}

void set_gphy_reg_cl22(uint8_t phyad, uint8_t reg, uint16_t value)
{
    an8855_phy_write(phyad-g_smi_addr, reg, value);
    /*
       gsw_top_reg_REG_PHY_IAC REG_PHY_IAC_val;
       gsw_top_reg_REG_PHY_IAD REG_PHY_IAD_val;

    // Wait until done
    do
    {
    REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    // Set address
    REG_PHY_IAC_val.Bits.csr_mdio_st = 1;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd = 1;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = phyad;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = reg;
    REG_PHY_IAC_val.Bits.csr_mdio_wr_data = value;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st = 1;

    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);
    */
}

UINT16 get_gphy_reg_cl45(uint8_t prtid, uint8_t devid, uint16_t reg)
{
    UINT32 rdata = 0;

    an8855_phy_read_cl45(prtid-g_smi_addr, devid, reg, &rdata);
    return ((UINT16)rdata);
    /*
    gsw_top_reg_REG_PHY_IAC REG_PHY_IAC_val;
    gsw_top_reg_REG_PHY_IAD REG_PHY_IAD_val;

    // Wait until done
    do
    {
    REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    // Set address
    REG_PHY_IAC_val.Bits.csr_mdio_st = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = prtid;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = devid;
    REG_PHY_IAC_val.Bits.csr_mdio_wr_data = reg;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st = 1;

    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);

    // Wait until done
    do
    {
    REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    // Read value
    REG_PHY_IAC_val.Bits.csr_mdio_st = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd = 3;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = prtid;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = devid;
    REG_PHY_IAC_val.Bits.csr_mdio_wr_data = 0;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st = 1;
    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);

    // Wait until done
    do
    {
    REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    REG_PHY_IAD_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAD);

    return REG_PHY_IAD_val.Raw;
    */
}

void set_gphy_reg_cl45(uint8_t prtid, uint8_t devid, uint16_t reg, uint16_t value)
{
    an8855_phy_write_cl45(prtid-g_smi_addr, devid, reg, value);
    /*
    gsw_top_reg_REG_PHY_IAC REG_PHY_IAC_val;

    // Wait until done
    do
    {
        REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    // Set address
    REG_PHY_IAC_val.Bits.csr_mdio_st = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = prtid;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = devid;
    REG_PHY_IAC_val.Bits.csr_mdio_wr_data = reg;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st = 1;

    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);

    // Wait until done
    do
    {
        REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    // Write value
    REG_PHY_IAC_val.Bits.csr_mdio_st = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd = 1;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = prtid;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = devid;
    REG_PHY_IAC_val.Bits.csr_mdio_wr_data = value;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st = 1;

    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);
    */
}

void anacal_exe(uint8_t phyadd_common)
{
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg17Ch, 1);// da_calin_flag pull high
    an8855_udelay(1000);
    set_gphy_reg_cl45(phyadd_common, DEVID_1E, RgAddr_dev1Eh_reg17Ch, 0);// da_calin_flag pull low
}


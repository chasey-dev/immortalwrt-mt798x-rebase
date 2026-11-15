/* FILE NAME: air_diag.c
 * PURPOSE:
 *      Define the diagnostic function in AIR SDK.
 * NOTES:
 *      None
 */

/* INCLUDE FILE DECLARATIONS
*/
#include "air.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
*/

/* STATIC VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
*/

/* FUNCTION NAME: air_diag_setTxComplyMode
 * PURPOSE:
 *      Set Ethernet TX Compliance mode.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      mode            --  Testing mode of Ethernet TX Compliance
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 */
AIR_ERROR_NO_T
air_diag_setTxComplyMode(
    const UI32_T unit,
    const UI8_T port,
    const AIR_DIAG_TXCOMPLY_MODE_T mode)
{
    UI32_T page = 0;
    UI32_T u32dat = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((mode >= AIR_DIAG_TXCOMPLY_MODE_LAST), AIR_E_BAD_PARAMETER);

    /* Backup page of CL22 */
    aml_readPhyReg(unit, port, PHY_PAGE, &page);

    switch(mode)
    {
        case AIR_DIAG_TXCOMPLY_MODE_10M_NLP:
            /* PHY 00h = 0x0100 */
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
            aml_writePhyReg(unit, port, PHY_MCR, MCR_MR_DUX);
            /* PHY dev 1Eh reg 145h = 0x5010 */
            u32dat = (FC_TDI_EN | FC_LITN_NO_COMP | FC_MDI_CO_MDI);
            aml_writePhyRegCL45(unit, port, 0x1e, 0x0145, u32dat);
            /* PHY dev 1Fh reg 17Bh = 0x1177 */
            u32dat = (CR_RG_TX_CM_10M(1) | CR_RG_DELAY_TX_10M(1) \
                    | CR_DA_TX_GAIN_10M_EEE(100) | CR_DA_TX_GAIN_10M(100));
            aml_writePhyRegCL45(unit, port, 0x1f, 0x027b, u32dat);
            break;
        case AIR_DIAG_TXCOMPLY_MODE_10M_RANDOM:
            /* PHY 00h = 0x0100 */
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
            aml_writePhyReg(unit, port, PHY_MCR, MCR_MR_DUX);
            /* PHY dev 1Eh reg 145h = 0x5010 */
            u32dat = (FC_TDI_EN | FC_LITN_NO_COMP | FC_MDI_CO_MDI);
            aml_writePhyRegCL45(unit, port, 0x1e, 0x0145, u32dat);
            /* PHY dev 1Fh reg 27Bh = 0x1177 */
            u32dat = (CR_RG_TX_CM_10M(1) | CR_RG_DELAY_TX_10M(1) \
                    | CR_DA_TX_GAIN_10M_EEE(100) | CR_DA_TX_GAIN_10M(100));
            aml_writePhyRegCL45(unit, port, 0x1f, 0x027b, u32dat);
            /* PHY 11dh = 0xf842 */
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_1);
            u32dat = (EPG_EN | EPG_RUN | EPG_TX_DUR | EPG_PKT_LEN_10KB \
                    | EPG_DES_ADDR(1) | EPG_PL_TYP_RANDOM);
            aml_writePhyReg(unit, port, PHY_EPG, u32dat);
            break;
        case AIR_DIAG_TXCOMPLY_MODE_10M_SINE:
            /* PHY 00h = 0x0100 */
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
            aml_writePhyReg(unit, port, PHY_MCR, MCR_MR_DUX);
            /* PHY dev 1Eh reg 145h = 0x5010 */
            u32dat = (FC_TDI_EN | FC_LITN_NO_COMP | FC_MDI_CO_MDI);
            aml_writePhyRegCL45(unit, port, 0x1e, 0x0145, u32dat);
            /* PHY dev 1Fh reg 27Bh = 0x1177 */
            u32dat = (CR_RG_TX_CM_10M(1) | CR_RG_DELAY_TX_10M(1) \
                    | CR_DA_TX_GAIN_10M_EEE(100) | CR_DA_TX_GAIN_10M(100));
            aml_writePhyRegCL45(unit, port, 0x1f, 0x027b, u32dat);
            /* PHY dev 1Eh reg 1A3h = 0x0000 */
            aml_writePhyRegCL45(unit, port, 0x1e, 0x01a3, 0x0000);
            /* PHY dev 1Eh reg 1A4h = 0x0000 */
            aml_writePhyRegCL45(unit, port, 0x1e, 0x01a4, 0x0000);
            /* PHY 11dh = 0xf840 */
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_1);
            u32dat = (EPG_EN | EPG_RUN | EPG_TX_DUR \
                    | EPG_PKT_LEN_10KB | EPG_DES_ADDR(1));
            aml_writePhyReg(unit, port, PHY_EPG, u32dat);
            break;
        case AIR_DIAG_TXCOMPLY_MODE_100M_PAIR_A:
            /* PHY 00h = 0x2100 */
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
            u32dat = (MCR_MR_FC_SPD_INT_0 | MCR_MR_DUX);
            aml_writePhyReg(unit, port, PHY_MCR, u32dat);
            /* PHY dev 1Eh reg 145h = 0x5010 */
            u32dat = (FC_TDI_EN | FC_LITN_NO_COMP | FC_MDI_CO_MDI);
            aml_writePhyRegCL45(unit, port, 0x1e, 0x0145, u32dat);
            break;
        case AIR_DIAG_TXCOMPLY_MODE_100M_PAIR_B:
            /* PHY 00h = 0x2100 */
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
            u32dat = (MCR_MR_FC_SPD_INT_0 | MCR_MR_DUX);
            aml_writePhyReg(unit, port, PHY_MCR, u32dat);
            /* PHY dev 1Eh reg 145h = 0x5018 */
            u32dat = (FC_TDI_EN | FC_LITN_NO_COMP | FC_MDI_CO_MDIX);
            aml_writePhyRegCL45(unit, port, 0x1e, 0x0145, u32dat);
            break;
        case AIR_DIAG_TXCOMPLY_MODE_1000M_TM1:
            /* PHY 09h = 0x2700 */
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
            u32dat = (CR1G_TEST_TM1 | CR1G_PORT_TYPE \
                    | CR1G_ADV_CAP1000_FDX | CR1G_ADV_CAP1000_HDX);
            aml_writePhyReg(unit, port, PHY_CR1G, u32dat);
            break;
        case AIR_DIAG_TXCOMPLY_MODE_1000M_TM2:
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
            /* PHY 09h = 0x4700 */
            u32dat = (CR1G_TEST_TM2 | CR1G_PORT_TYPE \
                    | CR1G_ADV_CAP1000_FDX | CR1G_ADV_CAP1000_HDX);
            aml_writePhyReg(unit, port, PHY_CR1G, u32dat);
            break;
        case AIR_DIAG_TXCOMPLY_MODE_1000M_TM3:
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
            /* PHY 09h = 0x6700 */
            u32dat = (CR1G_TEST_TM3 | CR1G_PORT_TYPE \
                    | CR1G_ADV_CAP1000_FDX | CR1G_ADV_CAP1000_HDX);
            aml_writePhyReg(unit, port, PHY_CR1G, u32dat);
            break;
        case AIR_DIAG_TXCOMPLY_MODE_1000M_TM4:
            aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
            /* PHY 09h = 0x8700 */
            u32dat = (CR1G_TEST_TM4 | CR1G_PORT_TYPE \
                    | CR1G_ADV_CAP1000_FDX | CR1G_ADV_CAP1000_HDX);
            aml_writePhyReg(unit, port, PHY_CR1G, u32dat);
            break;
        default:
            /* Unrecognized argument */
            return AIR_E_BAD_PARAMETER;
    }
    /* Restore page of CL22 */
    aml_writePhyReg(unit, port, PHY_PAGE, page);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_diag_getTxComplyMode
 * PURPOSE:
 *      Get Ethernet TX Compliance mode.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_mode        --  Testing mode of Ethernet TX Compliance
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_OTHERS
 *
 * NOTES:
 */
AIR_ERROR_NO_T
air_diag_getTxComplyMode(
    const UI32_T unit,
    const UI8_T port,
    AIR_DIAG_TXCOMPLY_MODE_T *ptr_mode)
{
    UI32_T page = 0;
    UI32_T curReg[4] = {0};
    UI32_T cmpReg[4] = {0};
    BOOL_T hit = FALSE;

    /* Mistake proofing */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_mode);

    /* Backup page of CL22 */
    aml_readPhyReg(unit, port, PHY_PAGE, &page);

    /* Test for AIR_DIAG_TXCOMPLY_MODE_1000M_TM1 */
    if( FALSE == hit )
    {
        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
        aml_readPhyReg(unit, port, PHY_CR1G, &curReg[0]);
        cmpReg[0] = (CR1G_TEST_TM1 | CR1G_PORT_TYPE | CR1G_ADV_CAP1000_FDX | CR1G_ADV_CAP1000_HDX);

        if( cmpReg[0] == curReg[0] )
        {
            (*ptr_mode) = AIR_DIAG_TXCOMPLY_MODE_1000M_TM1;
            hit = TRUE;
        }
    }

    /* Test for AIR_DIAG_TXCOMPLY_MODE_1000M_TM2 */
    if( FALSE == hit )
    {
        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
        aml_readPhyReg(unit, port, PHY_CR1G, &curReg[0]);
        cmpReg[0] = (CR1G_TEST_TM2 | CR1G_PORT_TYPE | CR1G_ADV_CAP1000_FDX | CR1G_ADV_CAP1000_HDX);

        if( cmpReg[0] == curReg[0] )
        {
            (*ptr_mode) = AIR_DIAG_TXCOMPLY_MODE_1000M_TM2;
            hit = TRUE;
        }
    }

    /* Test for AIR_DIAG_TXCOMPLY_MODE_1000M_TM3 */
    if( FALSE == hit )
    {
        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
        aml_readPhyReg(unit, port, PHY_CR1G, &curReg[0]);
        cmpReg[0] = (CR1G_TEST_TM3 | CR1G_PORT_TYPE | CR1G_ADV_CAP1000_FDX | CR1G_ADV_CAP1000_HDX);

        if( cmpReg[0] == curReg[0] )
        {
            (*ptr_mode) = AIR_DIAG_TXCOMPLY_MODE_1000M_TM3;
            hit = TRUE;
        }
    }

    /* Test for AIR_DIAG_TXCOMPLY_MODE_1000M_TM4 */
    if( FALSE == hit )
    {
        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
        aml_readPhyReg(unit, port, PHY_CR1G, &curReg[0]);
        cmpReg[0] = (CR1G_TEST_TM4 | CR1G_PORT_TYPE | CR1G_ADV_CAP1000_FDX | CR1G_ADV_CAP1000_HDX);

        if( cmpReg[0] == curReg[0] )
        {
            (*ptr_mode) = AIR_DIAG_TXCOMPLY_MODE_1000M_TM4;
            hit = TRUE;
        }
    }

    /* Test for AIR_DIAG_TXCOMPLY_MODE_100M_PAIR_A */
    if( FALSE == hit )
    {
        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
        aml_readPhyReg(unit, port, PHY_MCR, &curReg[0]);
        cmpReg[0] = (MCR_MR_FC_SPD_INT_0 | MCR_MR_DUX);

        aml_readPhyRegCL45(unit, port, 0x1e, 0x0145, &curReg[1]);
        cmpReg[1] = (FC_TDI_EN | FC_LITN_NO_COMP | FC_MDI_CO_MDI);

        if( (cmpReg[0] == curReg[0]) && (cmpReg[1] == curReg[1]) )
        {
            (*ptr_mode) = AIR_DIAG_TXCOMPLY_MODE_100M_PAIR_A;
            hit = TRUE;
        }
    }

    /* Test for AIR_DIAG_TXCOMPLY_MODE_100M_PAIR_B */
    if( FALSE == hit )
    {
        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
        aml_readPhyReg(unit, port, PHY_MCR, &curReg[0]);
        cmpReg[0] = (MCR_MR_FC_SPD_INT_0 | MCR_MR_DUX);

        aml_readPhyRegCL45(unit, port, 0x1e, 0x0145, &curReg[1]);
        cmpReg[1] = (FC_TDI_EN | FC_LITN_NO_COMP | FC_MDI_CO_MDIX);

        if( (cmpReg[0] == curReg[0]) && (cmpReg[1] == curReg[1]) )
        {
            (*ptr_mode) = AIR_DIAG_TXCOMPLY_MODE_100M_PAIR_B;
            hit = TRUE;
        }
    }

    /* Test for AIR_DIAG_TXCOMPLY_MODE_10M_SINE */
    if( FALSE == hit )
    {
        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
        aml_readPhyReg(unit, port, PHY_MCR, &curReg[0]);
        cmpReg[0] = MCR_MR_DUX;

        aml_readPhyRegCL45(unit, port, 0x1e, 0x0145, &curReg[1]);
        cmpReg[1] = (FC_TDI_EN | FC_LITN_NO_COMP | FC_MDI_CO_MDI);

        aml_readPhyRegCL45(unit, port, 0x1f, 0x027b, &curReg[2]);
        cmpReg[2] = (CR_RG_TX_CM_10M(1) \
                | CR_RG_DELAY_TX_10M(1) \
                | CR_DA_TX_GAIN_10M_EEE(100) \
                | CR_DA_TX_GAIN_10M(100));

        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_1);
        aml_readPhyReg(unit, port, PHY_EPG, &curReg[3]);
        cmpReg[3] = (EPG_EN \
                | EPG_RUN \
                | EPG_TX_DUR \
                | EPG_PKT_LEN_10KB \
                | EPG_DES_ADDR(1));

        if( (cmpReg[0] == curReg[0])    \
            && (cmpReg[1] == curReg[1]) \
            && (cmpReg[2] == curReg[2]) \
            && (cmpReg[3] == curReg[3]) )
        {
            (*ptr_mode) = AIR_DIAG_TXCOMPLY_MODE_10M_SINE;
            hit = TRUE;
        }
    }

    /* Test for AIR_DIAG_TXCOMPLY_MODE_10M_RANDOM */
    if( FALSE == hit )
    {
        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
        aml_readPhyReg(unit, port, PHY_MCR, &curReg[0]);
        cmpReg[0] = MCR_MR_DUX;

        aml_readPhyRegCL45(unit, port, 0x1e, 0x0145, &curReg[1]);
        cmpReg[1] = (FC_TDI_EN | FC_LITN_NO_COMP | FC_MDI_CO_MDI);

        aml_readPhyRegCL45(unit, port, 0x1f, 0x027b, &curReg[2]);
        cmpReg[2] = (CR_RG_TX_CM_10M(1) \
                | CR_RG_DELAY_TX_10M(1) \
                | CR_DA_TX_GAIN_10M_EEE(100) \
                | CR_DA_TX_GAIN_10M(100));

        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_1);
        aml_readPhyReg(unit, port, PHY_EPG, &curReg[3]);
        cmpReg[3] = (EPG_EN \
                | EPG_RUN \
                | EPG_TX_DUR \
                | EPG_PKT_LEN_10KB \
                | EPG_DES_ADDR(1) \
                | EPG_PL_TYP_RANDOM);

        if( (cmpReg[0] == curReg[0])    \
            && (cmpReg[1] == curReg[1]) \
            && (cmpReg[2] == curReg[2]) \
            && (cmpReg[3] == curReg[3]) )
        {
            (*ptr_mode) = AIR_DIAG_TXCOMPLY_MODE_10M_RANDOM;
            hit = TRUE;
        }
    }

    /* Test for AIR_DIAG_TXCOMPLY_MODE_10M_NLP */
    if( FALSE == hit )
    {
        aml_writePhyReg(unit, port, PHY_PAGE, PHY_PAGE_0);
        aml_readPhyReg(unit, port, PHY_MCR, &curReg[0]);
        cmpReg[0] = MCR_MR_DUX;

        aml_readPhyRegCL45(unit, port, 0x1e, 0x0145, &curReg[1]);
        cmpReg[1] = (FC_TDI_EN | FC_LITN_NO_COMP | FC_MDI_CO_MDI);

        aml_readPhyRegCL45(unit, port, 0x1f, 0x027b, &curReg[2]);
        cmpReg[2] = (CR_RG_TX_CM_10M(1) \
                | CR_RG_DELAY_TX_10M(1) \
                | CR_DA_TX_GAIN_10M_EEE(100) \
                | CR_DA_TX_GAIN_10M(100));

        if( (cmpReg[0] == curReg[0])    \
            && (cmpReg[1] == curReg[1]) \
            && (cmpReg[2] == curReg[2]) )
        {
            (*ptr_mode) = AIR_DIAG_TXCOMPLY_MODE_10M_NLP;
            hit = TRUE;
        }
    }

    /* Restore page of CL22 */
    aml_writePhyReg(unit, port, PHY_PAGE, page);

    if( TRUE == hit)
    {
        return AIR_E_OK;
    }
    else
    {
        return AIR_E_OTHERS;
    }
}


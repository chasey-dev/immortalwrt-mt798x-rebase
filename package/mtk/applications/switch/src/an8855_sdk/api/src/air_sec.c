/* FILE NAME: air_sec.c
 * PURPOSE:
 *      Define the security function in AIR SDK.
 *
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

/* FUNCTION NAME: air_sec_setStormEnable
 * PURPOSE:
 *      Enable or disable per port storm control for specific type.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      type            --  AIR_STORM_TYPE_BCST
 *                          AIR_STORM_TYPE_MCST
 *                          AIR_STORM_TYPE_UCST
 *      storm_en        --  TRUE
 *                          FALSE
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
air_sec_setStormEnable(
    const UI32_T unit,
    const UI32_T port,
    const AIR_STORM_TYPE_T type,
    const BOOL_T storm_en)
{
    UI32_T u32dat = 0, reg = 0, sp_en = 0;

    /* Check parameter */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((type >= AIR_STORM_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((storm_en != TRUE) && (storm_en != FALSE)), AIR_E_BAD_PARAMETER);

    /* Find register BSR:broadcast, BSR_EXT1:multicast, BSR_EXT2:unicast */
    switch(type)
    {
        case AIR_STORM_TYPE_BCST:
            reg = BSR(port);
            sp_en = BSR_STORM_BCST_EN;
            break;
        case AIR_STORM_TYPE_MCST:
            reg = BSR_EXT1(port);
            sp_en = BSR_STORM_MCST_EN;
            break;
        case AIR_STORM_TYPE_UCST:
            reg = BSR_EXT2(port);
            sp_en = BSR_STORM_UCST_EN;
            break;
        default:
            break;
    }

    /* Enable specific type */
    aml_readReg(unit, reg, &u32dat);
    if(TRUE == storm_en)
    {
        u32dat |= (BSR_STORM_DROP_EN | BSR_STORM_RATE_BASED);
        u32dat |= sp_en;
    }
    else
    {
        u32dat &= ~(BSR_STORM_DROP_EN | BSR_STORM_RATE_BASED);
        u32dat &= ~sp_en;
    }
    aml_writeReg(unit, reg, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_sec_getStormEnable
 * PURPOSE:
 *      Get per port status of storm control for specific type.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      type            --  AIR_STORM_TYPE_BCST
 *                          AIR_STORM_TYPE_MCST
 *                          AIR_STORM_TYPE_UCST
 * OUTPUT:
 *      ptr_storm_en    --  TRUE
 *                          FALSE
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getStormEnable(
    const UI32_T unit,
    const UI32_T port,
    const AIR_STORM_TYPE_T type,
    BOOL_T *ptr_storm_en)
{
    UI32_T u32dat = 0, reg = 0, en = 0;

    /* Check parameter */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((type >= AIR_STORM_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_storm_en);

    /* Find register BSR:broadcast, BSR_EXT1:multicast, BSR_EXT2:unicast */
    switch(type)
    {
        case AIR_STORM_TYPE_BCST:
            reg = BSR(port);
            break;
        case AIR_STORM_TYPE_MCST:
            reg = BSR_EXT1(port);
            break;
        case AIR_STORM_TYPE_UCST:
            reg = BSR_EXT2(port);
            break;
        default:
            break;
    }

    /* Enable specific type */
    aml_readReg(unit, reg, &u32dat);
    en = (u32dat & BSR_STORM_DROP_EN);
    if(FALSE == en)
    {
        *ptr_storm_en = FALSE;
    }
    else
    {
        *ptr_storm_en = TRUE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_sec_setStormRate
 * PURPOSE:
 *      Set per port storm rate limit control for specific type.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      type            --  AIR_STORM_TYPE_BCST
 *                          AIR_STORM_TYPE_MCST
 *                          AIR_STORM_TYPE_UCST
 *      count           --  Count of the unit
 *                          Range 0..255
 *                          Rate = (count * unit) bps
 *      unit            --  AIR_STORM_UNIT_64K
 *                          AIR_STORM_UNIT_256K
 *                          AIR_STORM_UNIT_1M
 *                          AIR_STORM_UNIT_4M
 *                          AIR_STORM_UNIT_16M
                            AIR_STORM_UNIT_32M
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
air_sec_setStormRate(
    const UI32_T unit,
    const UI32_T port,
    const AIR_STORM_TYPE_T type,
    const UI32_T count,
    const AIR_STORM_UNIT_T storm_unit)
{
    UI32_T u32dat = 0, reg = 0;

    /* Check parameter */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((type >= AIR_STORM_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((count > AIR_STORM_MAX_COUNT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((storm_unit >= AIR_STORM_UNIT_LAST), AIR_E_BAD_PARAMETER);

    /* Find register BSR:broadcast, BSR_EXT1:multicast, BSR_EXT2:unicast */
    switch(type)
    {
        case AIR_STORM_TYPE_BCST:
            reg = BSR(port);
            break;
        case AIR_STORM_TYPE_MCST:
            reg = BSR_EXT1(port);
            break;
        case AIR_STORM_TYPE_UCST:
            reg = BSR_EXT2(port);
            break;
        default:
            break;
    }
    /* Set storm rate limit unit */
    aml_readReg(unit, reg, &u32dat);
    u32dat &= ~(BSR_STORM_UNIT_MSK << BSR_STORM_UNIT_OFFT);
    u32dat |= (storm_unit << BSR_STORM_UNIT_OFFT);
    aml_writeReg(unit, reg, u32dat);

    /* Find register BSR1:broadcast, BSR1_EXT1:multicast, BSR1_EXT2:unicast */
    switch(type)
    {
        case AIR_STORM_TYPE_BCST:
            reg = BSR1(port);
            break;
        case AIR_STORM_TYPE_MCST:
            reg = BSR1_EXT1(port);
            break;
        case AIR_STORM_TYPE_UCST:
            reg = BSR1_EXT2(port);
            break;
        default:
            break;
    }
    /* Set storm rate limit count */
    u32dat &= ~(BSR_STORM_COUNT_MSK << BSR1_10M_COUNT_OFFT);
    u32dat |= (count << BSR1_10M_COUNT_OFFT);

    u32dat &= ~(BSR_STORM_COUNT_MSK << BSR1_100M_COUNT_OFFT);
    u32dat |= (count << BSR1_100M_COUNT_OFFT);

    u32dat &= ~(BSR_STORM_COUNT_MSK << BSR1_1000M_COUNT_OFFT);
    u32dat |= (count << BSR1_1000M_COUNT_OFFT);

    u32dat &= ~(BSR_STORM_COUNT_MSK << BSR1_2500M_COUNT_OFFT);
    u32dat |= (count << BSR1_2500M_COUNT_OFFT);
    aml_writeReg(unit, reg, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_sec_getStormRate
 * PURPOSE:
 *      Get per port storm rate limit control for specific type.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      type            --  AIR_STORM_TYPE_BCST
 *                          AIR_STORM_TYPE_MCST
 *                          AIR_STORM_TYPE_UCST
 * OUTPUT:
 *      ptr_count       --  Count of the unit
 *                          Range 0..255
 *                          Rate = (count * unit) bps
 *      ptr_unit        --  AIR_STORM_UNIT_64K
 *                          AIR_STORM_UNIT_256K
 *                          AIR_STORM_UNIT_1M
 *                          AIR_STORM_UNIT_4M
 *                          AIR_STORM_UNIT_16M
 *                          AIR_STORM_UNIT_32M
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getStormRate(
    const UI32_T unit,
    const UI32_T port,
    const AIR_STORM_TYPE_T type,
    UI32_T *ptr_count,
    AIR_STORM_UNIT_T *ptr_unit)
{
    UI32_T u32dat = 0, reg = 0;

    /* Check parameter */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((type >= AIR_STORM_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_count);
    AIR_CHECK_PTR(ptr_unit);

    /* Find register BSR:broadcast, BSR_EXT1:multicast, BSR_EXT2:unicast */
    switch(type)
    {
        case AIR_STORM_TYPE_BCST:
            reg = BSR(port);
            break;
        case AIR_STORM_TYPE_MCST:
            reg = BSR_EXT1(port);
            break;
        case AIR_STORM_TYPE_UCST:
            reg = BSR_EXT2(port);
            break;
        default:
            break;
    }
    aml_readReg(unit, reg, &u32dat);
    /* Get storm rate limit unit */
    *ptr_unit = (BSR_STORM_UNIT_MSK & (u32dat >> BSR_STORM_UNIT_OFFT));

    /* Find register BSR1:broadcast, BSR1_EXT1:multicast, BSR1_EXT2:unicast */
    switch(type)
    {
        case AIR_STORM_TYPE_BCST:
            reg = BSR1(port);
            break;
        case AIR_STORM_TYPE_MCST:
            reg = BSR1_EXT1(port);
            break;
        case AIR_STORM_TYPE_UCST:
            reg = BSR1_EXT2(port);
            break;
        default:
            break;
    }
    aml_readReg(unit, reg, &u32dat);
    /* Get storm rate limit count */
    *ptr_count = (u32dat & BSR_STORM_COUNT_MSK);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_sec_setFldMode
 * PURPOSE:
 *      Set per port flooding status for unknown type frame.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port to setting
 *      type            --  AIR_FLOOD_TYPE_BCST
 *                          AIR_FLOOD_TYPE_MCST
 *                          AIR_FLOOD_TYPE_UCST
 *                          AIR_FLOOD_TYPE_QURY
 *      fld_en          --  TRUE : flooding specific type frame for specific port
 *                          FALSE: drop specific type frame for specific port
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_setFldMode(
    const UI32_T unit,
    const UI32_T port,
    const AIR_FLOOD_TYPE_T type,
    const BOOL_T fld_en)
{
    UI32_T u32dat = 0, reg = 0;

    /* Check parameter */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((type >= AIR_FLOOD_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((fld_en != TRUE) && (fld_en != FALSE)), AIR_E_BAD_PARAMETER);

    /* Find register */
    switch(type)
    {
        case AIR_FLOOD_TYPE_BCST:
            reg = BCF;
            break;
        case AIR_FLOOD_TYPE_MCST:
            reg = UNMF;
            break;
        case AIR_FLOOD_TYPE_UCST:
            reg = UNUF;
            break;
        case AIR_FLOOD_TYPE_QURY:
            reg = QRYP;
            break;
        default:
            break;
    }

    aml_readReg(unit, reg, &u32dat);
    if(TRUE == fld_en)
    {
        u32dat |= BIT(port);
    }
    else
    {
        u32dat &= ~BIT(port);
    }
    aml_writeReg(unit, reg, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_sec_getFldMode
 * PURPOSE:
 *      Get per port flooding status for unknown type frame.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port to setting
 *      type            --  AIR_FLOOD_TYPE_BCST
 *                          AIR_FLOOD_TYPE_MCST
 *                          AIR_FLOOD_TYPE_UCST
 *                          AIR_FLOOD_TYPE_QURY
 * OUTPUT:
 *      ptr_fld_en      --  TRUE : flooding specific type frame for specific port
 *                          FALSE: drop specific type frame for specific port
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getFldMode(
    const UI32_T unit,
    const UI32_T port,
    const AIR_FLOOD_TYPE_T type,
    BOOL_T *ptr_fld_en)
{
    UI32_T u32dat = 0, reg = 0, value = 0;

    /* Check parameter */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((type >= AIR_FLOOD_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_fld_en);

    /* Find register */
    switch(type)
    {
        case AIR_FLOOD_TYPE_BCST:
            reg = BCF;
            break;
        case AIR_FLOOD_TYPE_MCST:
            reg = UNMF;
            break;
        case AIR_FLOOD_TYPE_UCST:
            reg = UNUF;
            break;
        case AIR_FLOOD_TYPE_QURY:
            reg = QRYP;
            break;
        default:
            break;
    }

    aml_readReg(unit, reg, &u32dat);
    value = u32dat & BIT(port);
    if(FALSE == value)
    {
        *ptr_fld_en = FALSE;
    }
    else
    {
        *ptr_fld_en = TRUE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_sec_setPortSecPortCfg
 * PURPOSE:
 *      Set port security configurations for specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      port_config     --  Structure of port configuration.
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
air_sec_setPortSecPortCfg(
    const UI32_T unit,
    const UI32_T port,
    const AIR_SEC_PORTSEC_PORT_CONFIG_T port_config)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T u32dat = 0, value = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != port_config.sa_lrn_en) && (FALSE != port_config.sa_lrn_en)), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != port_config.sa_lmt_en) && (FALSE != port_config.sa_lmt_en)), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port_config.sa_lmt_cnt > AIR_MAX_NUM_OF_MAC), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, PSC(port), &u32dat);

    if(FALSE == port_config.sa_lrn_en)
    {
        u32dat |= BITS_RANGE(PSC_DIS_LRN_OFFSET, PSC_DIS_LRN_LENGTH);
    }
    else
    {
        u32dat &= ~BITS_RANGE(PSC_DIS_LRN_OFFSET, PSC_DIS_LRN_LENGTH);
    }
    if(FALSE == port_config.sa_lmt_en)
    {
        u32dat &= ~BITS_RANGE(PSC_SA_CNT_EN_OFFSET, PSC_SA_CNT_EN_LENGTH);
        u32dat &= ~PSC_SA_CNT_LMT_MASK;
        u32dat |= (PSC_SA_CNT_LMT_MAX << PSC_SA_CNT_LMT_OFFSET);
    }
    else
    {
        u32dat |= BITS_RANGE(PSC_SA_CNT_EN_OFFSET, PSC_SA_CNT_EN_LENGTH);
        u32dat &= ~PSC_SA_CNT_LMT_MASK;
        value = (port_config.sa_lmt_cnt & PSC_SA_CNT_LMT_REALMASK);
        u32dat |= (((value > PSC_SA_CNT_LMT_MAX) ? PSC_SA_CNT_LMT_MAX : value) << PSC_SA_CNT_LMT_OFFSET);
    }

    aml_writeReg(unit, PSC(port), u32dat);

    return rc;
}

/* FUNCTION NAME: air_sec_getPortSecPortCfg
 * PURPOSE:
 *      Get port security configurations for specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *
 * OUTPUT:
 *      ptr_port_config --  Structure of port configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
 AIR_ERROR_NO_T
air_sec_getPortSecPortCfg(
    const UI32_T unit,
    const UI32_T port,
    AIR_SEC_PORTSEC_PORT_CONFIG_T *ptr_port_config)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T u32dat = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_port_config);

    aml_readReg(unit, PSC(port), &u32dat);

    ptr_port_config ->sa_lrn_en = ((~BITS_OFF_R(u32dat, PSC_DIS_LRN_OFFSET, PSC_DIS_LRN_LENGTH)) & BIT(0));
    ptr_port_config ->sa_lmt_en = BITS_OFF_R(u32dat, PSC_SA_CNT_EN_OFFSET, PSC_SA_CNT_EN_LENGTH);
    ptr_port_config ->sa_lmt_cnt = (u32dat >> PSC_SA_CNT_LMT_OFFSET) & PSC_SA_CNT_LMT_REALMASK;

    return rc;
}

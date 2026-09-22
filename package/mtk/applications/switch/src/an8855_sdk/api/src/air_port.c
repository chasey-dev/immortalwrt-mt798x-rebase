/* FILE NAME: air_port.c
 * PURPOSE:
 *      Define the port function in AIR SDK.
 *
 * NOTES:
 *      None
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "air.h"
#include "air_port.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define AIR_SET_REG_BIT(cond, reg, bit)    \
    do{                                     \
        if(TRUE == (cond))                  \
        {                                   \
            (reg) |= (bit);                 \
        }                                   \
        else                                \
        {                                   \
            (reg) &= ~(bit);                \
        }                                   \
    }while(0)

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

/* FUNCTION NAME: air_port_setAnMode
 * PURPOSE:
 *      Set the auto-negotiation mode for a specific port.(Auto or Forced)
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
air_port_setAnMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state)
{
    UI32_T u32CtrlReg = 0;
    UI32_T u32Pmcr = 0;
    UI32_T i = 0;
    UI32_T mii_port = 0;
    AIR_ERROR_NO_T ret = AIR_E_OK;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != state) && (FALSE != state)), AIR_E_BAD_PARAMETER);

    /* Read data from phy register */
    aml_readPhyReg(unit, port, 0x0, &u32CtrlReg);

    if(TRUE == state)
    {
        /* Enable AN mode of PHY port */
        u32CtrlReg |= BIT(12);
    }
    else
    {
        /* Disable AN mode of PHY port */
        u32CtrlReg &= ~BIT(12);
    }

    /* Restart AN */
    u32CtrlReg |= BIT(9);

    /* Write data to register */
    aml_writePhyReg(unit, port, 0x00, u32CtrlReg);

    return ret;
}

/* FUNCTION NAME: air_port_getAnMode
 * PURPOSE:
 *      Get the auto-negotiation mode for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getAnMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_state)
{
    UI32_T u32dat = 0;
    UI32_T i = 0, mii_port = 0;
    AIR_ERROR_NO_T ret = AIR_E_OK;

    /* Mistake proofing */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_state);

    /* Read data from register */
    aml_readPhyReg(unit, port, 0x0, &u32dat);
    (*ptr_state) = BITS_OFF_R(u32dat, 12, 1);

    return ret;
}

/* FUNCTION NAME: air_port_setLocalAdvAbility
 * PURPOSE:
 *      Set the auto-negotiation advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      adv             --  AN advertisement setting
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
air_port_setLocalAdvAbility(
    const UI32_T unit,
    const UI32_T port,
    const AIR_AN_ADV_T adv)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);

    /* Read AN Advertisement from register */
    aml_readPhyReg(unit, port, PHY_AN_ADV, &u32dat);

    /* Modify AN Advertisement */
    AIR_SET_REG_BIT(adv.advCap10HDX, u32dat, AN_ADV_CAP_10_HDX);
    AIR_SET_REG_BIT(adv.advCap10FDX, u32dat, AN_ADV_CAP_10_FDX);
    AIR_SET_REG_BIT(adv.advCap100HDX, u32dat, AN_ADV_CAP_100_HDX);
    AIR_SET_REG_BIT(adv.advCap100FDX, u32dat, AN_ADV_CAP_100_FDX);
    AIR_SET_REG_BIT(adv.advPause, u32dat, AN_ADV_CAP_PAUSE);

    /* Write AN Advertisement to register */
    aml_writePhyReg(unit, port, PHY_AN_ADV, u32dat);

    /* Write 1000BASE-T duplex capbility to  register */
    aml_readPhyReg(unit, port, PHY_CR1G, &u32dat);
    AIR_SET_REG_BIT(adv.advCap1000FDX, u32dat, CR1G_ADV_CAP1000_FDX);
    aml_writePhyReg(unit, port, PHY_CR1G, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_getLocalAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_adv         --  AN advertisement setting
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getLocalAdvAbility(
    const UI32_T unit,
    const UI32_T port,
    AIR_AN_ADV_T *ptr_adv)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);
    /* Mistake proofing checking */
    AIR_CHECK_PTR(ptr_adv);

    /* Read AN Advertisement from register */
    aml_readPhyReg(unit, port, PHY_AN_ADV, &u32dat);
    ptr_adv ->advCap10HDX = (u32dat & AN_ADV_CAP_10_HDX)?TRUE:FALSE;
    ptr_adv ->advCap10FDX = (u32dat & AN_ADV_CAP_10_FDX)?TRUE:FALSE;
    ptr_adv ->advCap100HDX = (u32dat & AN_ADV_CAP_100_HDX)?TRUE:FALSE;
    ptr_adv ->advCap100FDX = (u32dat & AN_ADV_CAP_100_FDX)?TRUE:FALSE;
    ptr_adv ->advPause = (u32dat & AN_ADV_CAP_PAUSE)?TRUE:FALSE;

    /* Read 1000BASE-T duplex capalibity from register */
    aml_readPhyReg(unit, port, PHY_CR1G, &u32dat);
    ptr_adv ->advCap1000FDX = (u32dat & CR1G_ADV_CAP1000_FDX)?TRUE:FALSE;

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_getRemoteAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation remote advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_lp_adv      --  AN advertisement of link partner
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getRemoteAdvAbility(
    const UI32_T unit,
    const UI32_T port,
    AIR_AN_ADV_T *ptr_lp_adv)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_lp_adv);

    /* Read AN LP Advertisement from register */
    aml_readPhyReg(unit, port, PHY_AN_LP_ADV, &u32dat);
    ptr_lp_adv ->advCap10HDX = (u32dat & AN_LP_CAP_10_HDX)?TRUE:FALSE;
    ptr_lp_adv ->advCap10FDX = (u32dat & AN_LP_CAP_10_FDX)?TRUE:FALSE;
    ptr_lp_adv ->advCap100HDX = (u32dat & AN_LP_CAP_100_HDX)?TRUE:FALSE;
    ptr_lp_adv ->advCap100FDX = (u32dat & AN_LP_CAP_100_FDX)?TRUE:FALSE;
    ptr_lp_adv ->advPause = (u32dat & AN_LP_CAP_PAUSE)?TRUE:FALSE;

    /* Read LP 1000BASE-T duplex capalibity from register */
    aml_readPhyReg(unit, port, PHY_SR1G, &u32dat);
    ptr_lp_adv ->advCap1000FDX = (u32dat & SR1G_CAP1000_FDX)?TRUE:FALSE;

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_setSpeed
 * PURPOSE:
 *      Set the speed for a specific port.
 *      This setting is used on force mode only.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      speed           --  AIR_PORT_SPEED_10M:  10Mbps
 *                          AIR_PORT_SPEED_100M: 100Mbps
 *                          AIR_PORT_SPEED_1000M:1Gbps
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setSpeed(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T speed)
{
    UI32_T u32dat = 0;
    UI32_T mii_port = 0;
    BOOL_T an = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for speed checking */
    AIR_PARAM_CHK((speed >= AIR_PORT_SPEED_2500M), AIR_E_BAD_PARAMETER);

    /* Read data from register */
    aml_readPhyReg(unit, port, 0x0, &u32dat);

    u32dat &= ~(BIT(13) | BIT(6));
    switch(speed)
    {
        case AIR_PORT_SPEED_10M:
            /* (bit6, bit13) = 2b'00 means 10M */
            break;
        case AIR_PORT_SPEED_100M:
            /* (bit6, bit13) = 2b'01 means 100M */
            u32dat |= BIT(13);
            break;
        case AIR_PORT_SPEED_1000M:
            /* (bit6, bit13) = 2b'10 means 1000M */
            u32dat |= BIT(6);
            break;
        default:
            /* (bit6, bit13) = 2b'11 means reverse,
             * other value is invalid */
            AIR_PRINT("argument 3: speed(%u) is invalid.\n", speed);
            return AIR_E_BAD_PARAMETER;
    }

    /* Write data to register */
    aml_writePhyReg(unit, port, 0x00, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_getSpeed
 * PURPOSE:
 *      Get the speed for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_speed       --  AIR_PORT_SPEED_10M:  10Mbps
 *                          AIR_PORT_SPEED_100M: 100Mbps
 *                          AIR_PORT_SPEED_1000M:1Gbps
 *                          AIR_PORT_SPEED_2500M:2.5Gbps
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getSpeed(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_speed)
{
    UI32_T u32dat = 0;
    UI32_T mii_port = 0, sp = 0;
    UI32_T ret = AIR_E_OK;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for speed checking */
    AIR_CHECK_PTR(ptr_speed);

    /* Read data from register */
	aml_readReg(unit, PMSR(port), &u32dat);
	(*ptr_speed) = BITS_OFF_R(u32dat, 28, 3);

    return ret;
}

/* FUNCTION NAME: air_port_setDuplex
 * PURPOSE:
 *      Get the duplex for a specific port.
 *      This setting is used on force mode only.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      duplex          --  AIR_PORT_DUPLEX_HALF
 *                          AIR_PORT_DUPLEX_FULL
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
air_port_setDuplex(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T duplex)
{
    UI32_T ret = AIR_E_OK;
    UI32_T u32dat = 0;
    UI32_T mii_port = 0, speed = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for duplex checking */
    AIR_PARAM_CHK(((AIR_PORT_DUPLEX_HALF != duplex) && (AIR_PORT_DUPLEX_FULL != duplex)), AIR_E_BAD_PARAMETER);

    /* Read data from register */
    aml_readPhyReg(unit, port, 0x0, &u32dat);
    speed = (BITS_OFF_R(u32dat, 6, 1) << 1) | BITS_OFF_R(u32dat, 13, 1);
    if(AIR_PORT_SPEED_100M >= speed)
    {
        if(TRUE == duplex)
        {
            u32dat |= BIT(8);
        }
        else
        {
            u32dat &= ~BIT(8);
        }
    }
    else
    {
        /* 1G support full duplex only */
        u32dat |= BIT(8);
    }

    /* Write data to register */
    aml_writePhyReg(unit, port, 0x0, u32dat);

    return ret;
}

/* FUNCTION NAME: air_port_getDuplex
 * PURPOSE:
 *      Get the duplex for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_duplex      --  AIR_PORT_DUPLEX_HALF
 *                          AIR_PORT_DUPLEX_FULL
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getDuplex(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_duplex)
{
    UI32_T u32dat = 0;
    UI32_T mii_port = 0, duplex = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);
    /* Mistake proofing for duplex checking */
    AIR_CHECK_PTR(ptr_duplex);

    /* Read data from register */
	aml_readReg(unit, PMSR(port), &u32dat);
	(*ptr_duplex) = BITS_OFF_R(u32dat, 25, 1);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_getLink
 * PURPOSE:
 *      Get the physical link status for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_ps          --  AIR_PORT_STATUS_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getLink(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_STATUS_T *ptr_ps)
{
    UI32_T ret = AIR_E_OK;
    UI32_T u32dat = 0;
    UI32_T mii_port = 0;
    BOOL_T an = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);
    /* Mistake proofing for duplex checking */
    AIR_CHECK_PTR(ptr_ps);

    /* Read data from register */
    aml_readReg(unit, PMSR(port), &u32dat);
    ptr_ps->link = BITS_OFF_R(u32dat, 24, 1);
    ptr_ps->duplex = BITS_OFF_R(u32dat, 25, 1);
    ptr_ps->speed = BITS_OFF_R(u32dat, 28, 3);

    return ret;
}

/* FUNCTION NAME: air_port_setBckPres
 * PURPOSE:
 *      Set the back pressure configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      bckPres         --  FALSE:Disable
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
air_port_setBckPres(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T bckPres)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    /* Mistake proofing for speed checking */
    AIR_PARAM_CHK(((TRUE != bckPres) && (FALSE != bckPres)), AIR_E_BAD_PARAMETER);

    /* Read data from register */
    aml_readReg(unit, PMCR(port), &u32dat);
    if(TRUE == bckPres)
    {
        u32dat |= BIT(11);
    }
    else
    {
        u32dat &= ~BIT(11);
    }

    /* Write data to register */
    aml_writeReg(unit, PMCR(port), u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_getBckPres
 * PURPOSE:
 *      Get the back pressure configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_bckPres     --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getBckPres(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_bckPres)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for speed checking */
    AIR_CHECK_PTR(ptr_bckPres);

    /* Read data from register */
    aml_readReg(unit, PMCR(port), &u32dat);
    (*ptr_bckPres) = BITS_OFF_R(u32dat, 11, 1);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_setFlowCtrl
 * PURPOSE:
 *      Set the flow control configuration for specific port.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number (0 - 6)
 *      dir             --  Directions of AIR_PORT_TX or AIR_PORT_RX
 *      fc_en           --  TRUE: Enable select port flow control
 *                          FALSE:Disable select port flow control
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
air_port_setFlowCtrl(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T dir,
    const BOOL_T fc_en)
{
    UI32_T u32dat = 0;

    /* Check port range */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    /* Check directions */
    if(dir != AIR_PORT_TX && dir != AIR_PORT_RX)
        return AIR_E_BAD_PARAMETER;;

    /* Check fc_en */
    AIR_PARAM_CHK(((TRUE != fc_en) && (FALSE != fc_en)), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, PMCR(port), &u32dat);
    if(TRUE == fc_en)
    {
        /* Enable port flow control */
        if(dir == AIR_PORT_TX)
        {
            u32dat |= FORCE_TX_FC;
        }
        else
        {
            u32dat |= FORCE_RX_FC;
        }
    }
    else
    {
        /* Disable port flow control */
        if(dir == AIR_PORT_TX)
        {
            u32dat &= ~(FORCE_TX_FC);
        }
        else
        {
            u32dat &= ~(FORCE_RX_FC);
        }
    }
    aml_writeReg(unit, PMCR(port), u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_getFlowCtrl
 * PURPOSE:
 *      Get the flow control configuration for specific port.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number (0..6)
 *      dir             --  AIR_PORT_TX
 *                          AIR_PORT_RX
 * OUTPUT:
 *      ptr_fc_en       --  FALSE: Port flow control disable
 *                          TRUE: Port flow control enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getFlowCtrl(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T dir,
    BOOL_T *ptr_fc_en)
{
    UI32_T u32dat = 0;

    /* Check port range */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_fc_en);

    /* Check directions */
    if(dir != AIR_PORT_TX && dir != AIR_PORT_RX)
        return AIR_E_BAD_PARAMETER;

    /* Read port flow control status*/
    aml_readReg(unit, PMCR(port), &u32dat);
    if(dir == AIR_PORT_TX)
    {
        if((u32dat & FORCE_TX_FC) == FORCE_TX_FC)
            *ptr_fc_en = TRUE;
        else
            *ptr_fc_en = FALSE;
    }
    else
    {
        if((u32dat & FORCE_RX_FC) == FORCE_RX_FC)
            *ptr_fc_en = TRUE;
        else
            *ptr_fc_en = FALSE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_setJumbo
 * PURPOSE:
 *      Set accepting jumbo frmes with specificied size.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      pkt_len         --  Select max packet length
 *                          RX_PKT_LEN_1518
 *                          RX_PKT_LEN_1536
 *                          RX_PKT_LEN_1552
 *                          RX_PKT_LEN_MAX_JUMBO
 *      frame_len       --  Select max lenght of jumbo frames
 *                          Range : 2 - 16
 *                          Units : K Bytes
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
air_port_setJumbo(
    const UI32_T unit,
    const UI32_T pkt_len,
    const UI32_T frame_len)
{
    UI32_T u32dat = 0;

    /* Check packet length */
    AIR_PARAM_CHK((pkt_len > 3), AIR_E_BAD_PARAMETER);

    /* Check frame length */
    AIR_PARAM_CHK((frame_len < 2), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((frame_len > 16), AIR_E_BAD_PARAMETER);

    /* Read and clear jumbo frame info */
    aml_readReg(unit, GMACCR, &u32dat);
    u32dat &= ~0x00F3;

    /* Set max packet length */
    u32dat |= pkt_len;

    /* Set jumbo frames max length */
    u32dat |= (frame_len << 4);

    aml_writeReg(unit, GMACCR, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_getJumbo
 * PURPOSE:
 *      Get accepting jumbo frmes with specificied size.
 *
 * INPUT:
 *      unit            --  Select device ID
 *
 * OUTPUT:
 *      ptr_pkt_len     --  Select max packet length
 *                          RX_PKT_LEN_1518
 *                          RX_PKT_LEN_1536
 *                          RX_PKT_LEN_1552
 *                          RX_PKT_LEN_MAX_JUMBO
 *      ptr_frame_len   --  Select max lenght of jumbo frames
 *                          Range : 2 - 16
 *                          Units : K Bytes
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getJumbo(
    const UI32_T unit,
    UI32_T *ptr_pkt_len,
    UI32_T *ptr_frame_len)
{
    UI32_T u32dat = 0;

    AIR_CHECK_PTR(ptr_pkt_len);
    AIR_CHECK_PTR(ptr_frame_len);

    /* Read and clear jumbo frame info */
    aml_readReg(unit, GMACCR, &u32dat);

    /* Set max packet length */
    *ptr_pkt_len = (0x03 & u32dat);

    /* Set jumbo frames max length */
    *ptr_frame_len = (0x0F & (u32dat >> 4));

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_setPsMode
 * PURPOSE:
 *      Set the power saving mode for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      mode            --  Bit-map:
 *                          AIR_PORT_PS_LINKSTATUS
 *                          AIR_PORT_PS_EEE
 *                          FALSE: Disable / TRUE: Enable
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
air_port_setPsMode(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T mode)
{
    UI32_T u32dat = 0;
    UI32_T u32cl45_1e_3c = 0;
    UI32_T u32cl45_1e_3d = 0;
    UI32_T u32cl45_1e_3e = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((mode & (~AIR_PORT_PS_MASK)), AIR_E_BAD_PARAMETER);

    /* Read data from register */
    aml_readPhyRegCL45(unit, port, PHY_DEV_1EH, BYPASS_POWER_DOWN_REG0, &u32cl45_1e_3c);
    aml_readPhyRegCL45(unit, port, PHY_DEV_1EH, BYPASS_POWER_DOWN_REG1, &u32cl45_1e_3d);
    aml_readPhyRegCL45(unit, port, PHY_DEV_1EH, BYPASS_POWER_DOWN_REG2, &u32cl45_1e_3e);

    if(mode & AIR_PORT_PS_LINKSTATUS)
    {
        /* Set Link Status
         * Disable bypass function to enable */
        u32cl45_1e_3c &= ~BITS(12, 15);
        aml_writePhyRegCL45(unit, port, PHY_DEV_1EH, BYPASS_POWER_DOWN_REG0, u32cl45_1e_3c);
        u32cl45_1e_3d &= ~BITS(12, 15);
        aml_writePhyRegCL45(unit, port, PHY_DEV_1EH, BYPASS_POWER_DOWN_REG1, u32cl45_1e_3d);
        u32cl45_1e_3e &= ~BITS(11, 15);
        aml_writePhyRegCL45(unit, port, PHY_DEV_1EH, BYPASS_POWER_DOWN_REG2, u32cl45_1e_3e);
    }
    else
    {
        /* Set Link Status
         * Enable bypass function to disable */
        u32cl45_1e_3c |= BITS(12, 15);
        aml_writePhyRegCL45(unit, port, PHY_DEV_1EH, BYPASS_POWER_DOWN_REG0, u32cl45_1e_3c);
        u32cl45_1e_3d |= BITS(12, 15);
        aml_writePhyRegCL45(unit, port, PHY_DEV_1EH, BYPASS_POWER_DOWN_REG1, u32cl45_1e_3d);
        u32cl45_1e_3e |= BITS(11, 15);
        aml_writePhyRegCL45(unit, port, PHY_DEV_1EH, BYPASS_POWER_DOWN_REG2, u32cl45_1e_3e);
    }

    if(mode & AIR_PORT_PS_EEE)
    {
        /* Enable EEE */
        u32dat = (EEE_ADV_1000BT | EEE_ADV_100BT );
        aml_writePhyRegCL45(unit, port, PHY_DEV_07H, EEE_ADV_REG, u32dat);
    }
    else
    {
        /* Disable EEE */
        aml_writePhyRegCL45(unit, port, PHY_DEV_07H, EEE_ADV_REG, 0);
    }
    return AIR_E_OK;

}

/* FUNCTION NAME: air_port_getPsMode
 * PURPOSE:
 *      Get the power saving mode for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 * OUTPUT:
 *      ptr_mode        --  Bit-map:
 *                          AIR_PORT_PS_LINKSTATUS
 *                          AIR_PORT_PS_EEE
 *                          FALSE: Disable / TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPsMode(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_mode)
{
    UI32_T u32cl45_1e_3e = 0;
    UI32_T u32cl45_07_3c = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for mode checking */
    AIR_CHECK_PTR(ptr_mode);

    (*ptr_mode) = 0;

    /* Check link-status power saving */
    aml_readPhyRegCL45(unit, port, PHY_DEV_1EH, BYPASS_POWER_DOWN_REG2, &u32cl45_1e_3e);
    if(!BITS_OFF_R(u32cl45_1e_3e, 11, 5))
    {
        /* Read Bypass the power-down TXVLD to check link-status
         * power saving function state */
        (*ptr_mode) |= AIR_PORT_PS_LINKSTATUS;
    }

    /* Check EEE */
    aml_readPhyRegCL45(unit, port, PHY_DEV_07H, EEE_ADV_REG, &u32cl45_07_3c);
    if( (u32cl45_07_3c & EEE_ADV_1000BT) && (u32cl45_07_3c & EEE_ADV_100BT) )
    {
        /* Read PMCR to check EEE ability */
        (*ptr_mode) |= AIR_PORT_PS_EEE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_setSmtSpdDwn
 * PURPOSE:
 *      Set Smart speed down feature for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
 *      time            --  AIR_PORT_SSD_2T
 *                          AIR_PORT_SSD_3T
 *                          AIR_PORT_SSD_4T
 *                          AIR_PORT_SSD_5T
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
air_port_setSmtSpdDwn(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state,
    const UI32_T time)
{
    UI32_T u32ext14 = 0;
    UI32_T page = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for state checking */
    AIR_PARAM_CHK(((TRUE != state) && (FALSE != state)), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for time checking */
    AIR_PARAM_CHK((time >= AIR_PORT_SSD_LAST), AIR_E_BAD_PARAMETER);

    /* Backup page */
    aml_readPhyReg(unit, port, 0x1F, &page);

    /* Switch to page 1*/
    aml_writePhyReg(unit, port, 0x1F, 0x1);
    /* Read data from register */
    aml_readPhyReg(unit, port, 0x14, &u32ext14);

    /* Write data to register */
    if(TRUE == state)
    {
        u32ext14 |= BIT(4);
    }
    else
    {
        u32ext14 &= ~BIT(4);
    }
    u32ext14 &= ~BITS(2,3);
    u32ext14 |= time << 2;

    /* Switch to page 1*/
    aml_writePhyReg(unit, port, 0x1F, 0x1);
    /* Read data from register */
    aml_writePhyReg(unit, port, 0x14, u32ext14);

    /* Restore page */
    aml_writePhyReg(unit, port, 0x1F, page);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_getSmtSpdDwn
 * PURPOSE:
 *      Get Smart speed down feature for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_state       --  FALSE:Disable
 *                          TRUE: Enable
 *      ptr_time        --  AIR_PORT_SSD_2T
 *                          AIR_PORT_SSD_3T
 *                          AIR_PORT_SSD_4T
 *                          AIR_PORT_SSD_5T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getSmtSpdDwn(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_state,
    UI32_T *ptr_time)
{
    UI32_T u32ext14 = 0;
    UI32_T page = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for state checking */
    AIR_CHECK_PTR(ptr_state);

    /* Mistake proofing for time checking */
    AIR_CHECK_PTR(ptr_time);

    /* Backup page */
    aml_readPhyReg(unit, port, 0x1F, &page);

    /* Switch to page 1*/
    aml_writePhyReg(unit, port, 0x1F, 0x1);
    /* Read data from register */
    aml_readPhyReg(unit, port, 0x14, &u32ext14);

    (*ptr_state) = BITS_OFF_R(u32ext14, 4, 1);
    (*ptr_time) = BITS_OFF_R(u32ext14, 2, 2);

    /* Restore page */
    aml_writePhyReg(unit, port, 0x1F, page);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_setEnable
 * PURPOSE:
 *      Set powerdown state for a specific port.
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
air_port_setEnable(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != state) && (FALSE != state)), AIR_E_BAD_PARAMETER);

    /* Read data from register */
    aml_readPhyReg(unit, port, 0x0, &u32dat);

    if(TRUE == state)
    {
        /* Enable port, so disable powerdown bit */
        u32dat &= ~BIT(11);
    }
    else
    {
        /* Disable port, so enable powerdown bit */
        u32dat |= BIT(11);
    }

    /* Write data to register */
    aml_writePhyReg(unit, port, 0x0, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_getEnable
 * PURPOSE:
 *      Get powerdown state for a specific port.
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
air_port_getEnable(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_GIGA_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for state checking */
    AIR_CHECK_PTR(ptr_state);

    /* Read data from register */
    aml_readPhyReg(unit, port, 0x0, &u32dat);

    (*ptr_state) = (~BITS_OFF_R(u32dat, 11, 1))&BIT(0);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_setPortMatrix
 * PURPOSE:
 *      Set port matrix from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      port_bitmap     --  Matrix port bitmap
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_port_setPortMatrix(
    const UI32_T    unit,
    const UI32_T    port,
    const UI32_T    port_bitmap)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port_bitmap & (~AIR_ALL_PORT_BITMAP)), AIR_E_BAD_PARAMETER);

    aml_writeReg(unit, PORTMATRIX(port), port_bitmap);

    return rc;
}

/* FUNCTION NAME: air_port_getPortMatrix
 * PURPOSE:
 *      Get port matrix from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *
 * OUTPUT:
 *      p_port_bitmap   --  Matrix port bitmap
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_port_getPortMatrix(
    const UI32_T    unit,
    const UI32_T    port,
    UI32_T          *p_port_bitmap)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(p_port_bitmap);

    aml_readReg(unit, PORTMATRIX(port), &val);
    *p_port_bitmap = val;

    return rc;
}

/* FUNCTION NAME: air_port_setVlanMode
 * PURPOSE:
 *      Set port-based vlan mechanism from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      mode            --  Port vlan mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_port_setVlanMode(
    const UI32_T    unit,
    const UI32_T    port,
    const AIR_PORT_VLAN_MODE_T mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((mode >= AIR_PORT_VLAN_MODE_LAST), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, PCR(port), &val);
    val &= ~PCR_PORT_VLAN_MASK;
    val |= (mode & PCR_PORT_VLAN_RELMASK) << PCR_PORT_VLAN_OFFT;
    aml_writeReg(unit, PCR(port), val);

    return rc;
}

/* FUNCTION NAME: air_port_getVlanMode
 * PURPOSE:
 *      Get port-based vlan mechanism from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *
 * OUTPUT:
 *      p_mode          --  Port vlan mode
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_port_getVlanMode(
    const UI32_T    unit,
    const UI32_T    port,
    AIR_PORT_VLAN_MODE_T *p_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(p_mode);

    aml_readReg(unit, PCR(port), &val);
    *p_mode = (val >> PCR_PORT_VLAN_OFFT) & PCR_PORT_VLAN_RELMASK;

    return rc;
}

/* FUNCTION NAME: air_port_setSpTag
 * PURPOSE:
 *      Set special tag state of a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      sptag_en        --  TRUE:  Enable special tag
 *                          FALSE: Disable special tag
 * OUTPUT:
 *        None
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setSpTag(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T sptag_en)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != sptag_en) && (FALSE != sptag_en)), AIR_E_BAD_PARAMETER);

    /* Read data from register */
    aml_readReg(unit, PVC(port), &u32dat);

    /* Write data to register */
    if(TRUE == sptag_en)
    {
        u32dat |= PVC_SPTAG_EN_MASK;
    }
    else
    {
        u32dat &= ~PVC_SPTAG_EN_MASK;
    }
    aml_writeReg(unit, PVC(port), u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_getSpTag
 * PURPOSE:
 *      Get special tag state of a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 * OUTPUT:
 *      ptr_sptag_en    --  TRUE:  Special tag enable
 *                          FALSE: Special tag disable
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getSpTag(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_sptag_en)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for state checking */
    AIR_CHECK_PTR(ptr_sptag_en);

    /* Read data from register */
    aml_readReg(unit, PVC(port), &u32dat);

    *ptr_sptag_en = (u32dat & PVC_SPTAG_EN_MASK) >> PVC_SPTAG_EN_OFFT;

    return AIR_E_OK;
}

/* FUNCTION NAME: air_port_set5GBaseRModeEnable
 * PURPOSE:
 *      Set the port5 5GBase-R mode enable
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
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_set5GBaseRModeEn(
    const UI32_T unit)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T u32dat = 0;

    /* PHYA Cal Enable (EFUSE) */
    aml_readReg(unit, INTF_CTRL_8, &u32dat);
    u32dat |= BIT(7);
    aml_writeReg(unit, INTF_CTRL_8, u32dat);

    aml_readReg(unit, INTF_CTRL_9, &u32dat);
    u32dat |= BIT(31);
    aml_writeReg(unit, INTF_CTRL_9, u32dat);

    /* PMA Init */
    /* PLL */
    aml_readReg(unit, RX_CTRL_26, &u32dat);
    u32dat |= BIT(23);
    u32dat &= (~BIT(24));
    u32dat |= BIT(26);
    aml_writeReg(unit, RX_CTRL_26, u32dat);

    aml_readReg(unit, QP_DIG_MODE_CTRL_1, &u32dat);
    u32dat |= BITS(2, 3);
    aml_writeReg(unit, QP_DIG_MODE_CTRL_1, u32dat);

    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat |= BITS(0, 1);
    u32dat &= ~(0x7 << 2);
    u32dat |= (0x5 << 2);
    u32dat &= ~(0x3 << 6);
    u32dat |= (0x1 << 6);
    u32dat &= ~(0x7 << 8);
    u32dat |= (0x3 << 8);
    u32dat |= BIT(29);
    u32dat &= ~BITS(12, 13);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    aml_readReg(unit, PLL_CTRL_4, &u32dat);
    u32dat &= ~BIT(2);
    aml_writeReg(unit, PLL_CTRL_4, u32dat);

    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~BIT(14);
    u32dat &= ~(0xf << 16);
    u32dat |= (0x8 << 16);
    u32dat &= ~BITS(20, 21);
    u32dat &= ~(0x3 << 24);
    u32dat |= (0x1 << 24);
    u32dat &= ~BIT(26);
    u32dat |= BIT(22);
    u32dat |= BIT(27);
    u32dat |= BIT(28);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    aml_readReg(unit, PLL_CTRL_4, &u32dat);
    u32dat &= ~(0x3 << 3);
    u32dat |= (0x1 << 3);
    aml_writeReg(unit, PLL_CTRL_4, u32dat);

    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~BIT(30);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    aml_readReg(unit, SS_LCPLL_PWCTL_SETTING_2, &u32dat);
    u32dat |= BITS(16, 17);
    aml_writeReg(unit, SS_LCPLL_PWCTL_SETTING_2, u32dat);

    aml_writeReg(unit, SS_LCPLL_TDC_FLT_2, 0x1c800000);
    aml_writeReg(unit, SS_LCPLL_TDC_PCW_1, 0x1c800000);

    aml_readReg(unit, SS_LCPLL_TDC_FLT_5, &u32dat);
    u32dat &= ~BIT(24);
    aml_writeReg(unit, SS_LCPLL_TDC_FLT_5, u32dat);

    aml_readReg(unit, PLL_CK_CTRL_0, &u32dat);
    u32dat &= ~BIT(8);
    aml_writeReg(unit, PLL_CK_CTRL_0, u32dat);

    aml_readReg(unit, PLL_CTRL_3, &u32dat);
    u32dat &= ~BITS(0, 15);
    aml_writeReg(unit, PLL_CTRL_3, u32dat);

    aml_readReg(unit, PLL_CTRL_4, &u32dat);
    u32dat &= ~BITS(0, 1);
    aml_writeReg(unit, PLL_CTRL_4, u32dat);

    aml_readReg(unit, PLL_CTRL_3, &u32dat);
    u32dat &= ~BITS(16, 31);
    aml_writeReg(unit, PLL_CTRL_3, u32dat);

    aml_readReg(unit, PLL_CK_CTRL_0, &u32dat);
    u32dat &= ~BIT(9);
    aml_writeReg(unit, PLL_CK_CTRL_0, u32dat);

    aml_readReg(unit, RG_QP_PLL_IPLL_DIG_PWR_SEL, &u32dat);
    u32dat &= ~(0x3 << 25);
    u32dat |= (0x1 << 25);
    aml_writeReg(unit, RG_QP_PLL_IPLL_DIG_PWR_SEL, u32dat);

    aml_readReg(unit, RG_QP_PLL_SDM_ORD, &u32dat);
    u32dat |= BIT(3);
    u32dat |= BIT(4);
    aml_writeReg(unit, RG_QP_PLL_SDM_ORD, u32dat);

    aml_readReg(unit, RG_QP_RX_DAC_EN, &u32dat);
    u32dat &= ~(0x3 << 16);
    u32dat |= (0x2 << 16);
    aml_writeReg(unit, RG_QP_RX_DAC_EN, u32dat);

    aml_readReg(unit, PON_RXFEDIG_CTRL_0, &u32dat);
    u32dat &= ~BIT(12);
    aml_writeReg(unit, PON_RXFEDIG_CTRL_0, u32dat);

    /* RX Control */
    aml_readReg(unit, RG_QP_CDR_LPF_MJV_LIM, &u32dat);
    u32dat &= ~BITS(4, 5);
    aml_writeReg(unit, RG_QP_CDR_LPF_MJV_LIM, u32dat);

    aml_readReg(unit, RG_QP_RXAFE_RESERVE, &u32dat);
    u32dat |= BIT(11);
    aml_writeReg(unit, RG_QP_RXAFE_RESERVE, u32dat);

    aml_readReg(unit, RG_QP_CDR_PR_CKREF_DIV1, &u32dat);
    u32dat &= ~(0x1f << 8);
    u32dat |= (0xc << 8);
    aml_writeReg(unit, RG_QP_CDR_PR_CKREF_DIV1, u32dat);

    aml_readReg(unit, RG_QP_CDR_FORCE_IBANDLPF_R_OFF, &u32dat);
    u32dat |= BIT(13);
    aml_writeReg(unit, RG_QP_CDR_FORCE_IBANDLPF_R_OFF, u32dat);

    aml_readReg(unit, RG_QP_CDR_PR_KBAND_DIV_PCIE, &u32dat);
    u32dat |= BIT(30);
    aml_writeReg(unit, RG_QP_CDR_PR_KBAND_DIV_PCIE, u32dat);

    aml_readReg(unit, PLL_CTRL_0, &u32dat);
    u32dat |= BIT(0);
    aml_writeReg(unit, PLL_CTRL_0, u32dat);

    aml_readReg(unit, RX_DLY_0, &u32dat);
    u32dat &= ~(0xff << 0);
    u32dat |= (0x6f << 0);
    u32dat |= BITS(8, 13);
    aml_writeReg(unit, RX_DLY_0, u32dat);

    aml_readReg(unit, RX_CTRL_42, &u32dat);
    u32dat &= ~(0x1fff << 0);
    u32dat |= (0x150 << 0);
    aml_writeReg(unit, RX_CTRL_42, u32dat);

    aml_readReg(unit, RX_CTRL_2, &u32dat);
    u32dat &= ~(0x1fff << 16);
    u32dat |= (0x150 << 16);
    aml_writeReg(unit, RX_CTRL_2, u32dat);

    aml_readReg(unit, PON_RXFEDIG_CTRL_9, &u32dat);
    u32dat |= BITS(0, 2);
    aml_writeReg(unit, PON_RXFEDIG_CTRL_9, u32dat);

    aml_readReg(unit, RX_CTRL_8, &u32dat);
    u32dat &= ~(0xfff << 16);
    u32dat |= (0x200 << 16);
    aml_writeReg(unit, RX_CTRL_8, u32dat);

    /* Frequency memter */
    aml_readReg(unit, RX_CTRL_5, &u32dat);
    u32dat &= ~(0xfffff << 10);
    u32dat |= (0x9 << 10);
    aml_writeReg(unit, RX_CTRL_5, u32dat);

    aml_readReg(unit, RX_CTRL_6, &u32dat);
    u32dat &= ~(0xfffff << 0);
    u32dat |= (0x64 << 0);
    aml_writeReg(unit, RX_CTRL_6, u32dat);

    aml_readReg(unit, RX_CTRL_7, &u32dat);
    u32dat &= ~(0xfffff << 0);
    u32dat |= (0x2710 << 0);
    aml_writeReg(unit, RX_CTRL_7, u32dat);

    /* PCS Init */
    aml_readReg(unit, RG_USXGMII_AN_CONTROL_0, &u32dat);
    u32dat &= ~BIT(0);
    aml_writeReg(unit, RG_USXGMII_AN_CONTROL_0, u32dat);

    aml_readReg(unit, USGMII_CTRL_0, &u32dat);
    u32dat |= BIT(2);
    aml_writeReg(unit, USGMII_CTRL_0, u32dat);

    aml_readReg(unit, MSG_RX_CTRL_0, &u32dat);
    u32dat |= BIT(28);
    aml_writeReg(unit, MSG_RX_CTRL_0, u32dat);

    aml_readReg(unit, QP_CK_RST_CTRL_4, &u32dat);
    u32dat |= BITS(14, 20);
    aml_writeReg(unit, QP_CK_RST_CTRL_4, u32dat);

    /* bypass flow control to MAC */
    aml_writeReg(unit, MSG_RX_LIK_STS_0, 0x01010107);
    aml_writeReg(unit, MSG_RX_LIK_STS_2, 0x00000EEF);

    return ret;
}

/* FUNCTION NAME: air_port_setHsgmiiModeEnable
 * PURPOSE:
 *      Set the port5 HSGMII mode enable
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
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setHsgmiiModeEn(
    const UI32_T unit)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T u32dat = 0;

    /* PLL */
    aml_readReg(unit, QP_DIG_MODE_CTRL_1, &u32dat);
    u32dat &= ~(0x3 << 2);
    u32dat |= (0x1 << 2);
    aml_writeReg(unit, QP_DIG_MODE_CTRL_1, u32dat);

    /* PLL - LPF */
    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~(0x3 << 0);
    u32dat |= (0x1 << 0);
    u32dat &= ~(0x7 << 2);
    u32dat |= (0x5 << 2);
    u32dat &= ~BITS(6, 7);
    u32dat &= ~(0x7 << 8);
    u32dat |= (0x3 << 8);
    u32dat |= BIT(29);
    u32dat &= ~BITS(12, 13);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    /* PLL - ICO */
    aml_readReg(unit, PLL_CTRL_4, &u32dat);
    u32dat |= BIT(2);
    aml_writeReg(unit, PLL_CTRL_4, u32dat);

    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~BIT(14);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    /* PLL - CHP */
    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~(0xf << 16);
    u32dat |= (0x6 << 16);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);


    /* PLL - PFD */
    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~(0x3 << 20);
    u32dat |= (0x1 << 20);
    u32dat &= ~(0x3 << 24);
    u32dat |= (0x1 << 24);
    u32dat &= ~BIT(26);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    /* PLL - POSTDIV */
    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat |= BIT(22);
    u32dat &= ~BIT(27);
    u32dat &= ~BIT(28);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    /* PLL - SDM */
    aml_readReg(unit, PLL_CTRL_4, &u32dat);
    u32dat &= ~BITS(3, 4);
    aml_writeReg(unit, PLL_CTRL_4, u32dat);

    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~BIT(30);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    aml_readReg(unit, SS_LCPLL_PWCTL_SETTING_2, &u32dat);
    u32dat &= ~(0x3 << 16);
    u32dat |= (0x1 << 16);
    aml_writeReg(unit, SS_LCPLL_PWCTL_SETTING_2, u32dat);

    aml_writeReg(unit, SS_LCPLL_TDC_FLT_2, 0x7a000000);
    aml_writeReg(unit, SS_LCPLL_TDC_PCW_1, 0x7a000000);

    aml_readReg(unit, SS_LCPLL_TDC_FLT_5, &u32dat);
    u32dat &= ~BIT(24);
    aml_writeReg(unit, SS_LCPLL_TDC_FLT_5, u32dat);

    aml_readReg(unit, PLL_CK_CTRL_0, &u32dat);
    u32dat &= ~BIT(8);
    aml_writeReg(unit, PLL_CK_CTRL_0, u32dat);

    /* PLL - SS */
    aml_readReg(unit, PLL_CTRL_3, &u32dat);
    u32dat &= ~BITS(0, 15);
    aml_writeReg(unit, PLL_CTRL_3, u32dat);

    aml_readReg(unit, PLL_CTRL_4, &u32dat);
    u32dat &= ~BITS(0, 1);
    aml_writeReg(unit, PLL_CTRL_4, u32dat);

    aml_readReg(unit, PLL_CTRL_3, &u32dat);
    u32dat &= ~BITS(16, 31);
    aml_writeReg(unit, PLL_CTRL_3, u32dat);

    /* PLL - TDC */
    aml_readReg(unit, PLL_CK_CTRL_0, &u32dat);
    u32dat &= ~BIT(9);
    aml_writeReg(unit, PLL_CK_CTRL_0, u32dat);

    aml_readReg(unit, RG_QP_PLL_SDM_ORD, &u32dat);
    u32dat |= BIT(3);
    u32dat |= BIT(4);
    aml_writeReg(unit, RG_QP_PLL_SDM_ORD, u32dat);

    aml_readReg(unit, RG_QP_RX_DAC_EN, &u32dat);
    u32dat &= ~(0x3 << 16);
    u32dat |= (0x2 << 16);
    aml_writeReg(unit, RG_QP_RX_DAC_EN, u32dat);

    /* TCL Disable (only for Co-SIM) */
    aml_readReg(unit, PON_RXFEDIG_CTRL_0, &u32dat);
    u32dat &= ~BIT(12);
    aml_writeReg(unit, PON_RXFEDIG_CTRL_0, u32dat);

    /* TX Init */
    aml_readReg(unit, RG_QP_TX_MODE_16B_EN, &u32dat);
    u32dat &= ~BIT(0);
    u32dat &= ~(0xffff << 16);
    u32dat |= (0x4 << 16);
    aml_writeReg(unit, RG_QP_TX_MODE_16B_EN, u32dat);

    /* RX Control */
    aml_readReg(unit, RG_QP_RXAFE_RESERVE, &u32dat);
    u32dat |= BIT(11);
    aml_writeReg(unit, RG_QP_RXAFE_RESERVE, u32dat);

    aml_readReg(unit, RG_QP_CDR_LPF_MJV_LIM, &u32dat);
    u32dat &= ~(0x3 << 4);
    u32dat |= (0x1 << 4);
    aml_writeReg(unit, RG_QP_CDR_LPF_MJV_LIM, u32dat);

    aml_readReg(unit, RG_QP_CDR_LPF_SETVALUE, &u32dat);
    u32dat &= ~(0xf << 25);
    u32dat |= (0x1 << 25);
    u32dat &= ~(0x7 << 29);
    u32dat |= (0x3 << 29);
    aml_writeReg(unit, RG_QP_CDR_LPF_SETVALUE, u32dat);

    aml_readReg(unit, RG_QP_CDR_PR_CKREF_DIV1, &u32dat);
    u32dat &= ~(0x1f << 8);
    u32dat |= (0xf << 8);
    aml_writeReg(unit, RG_QP_CDR_PR_CKREF_DIV1, u32dat);

    aml_readReg(unit, RG_QP_CDR_PR_KBAND_DIV_PCIE, &u32dat);
    u32dat &= ~(0x3f << 0);
    u32dat |= (0x19 << 0);
    u32dat &= ~BIT(6);
    aml_writeReg(unit, RG_QP_CDR_PR_KBAND_DIV_PCIE, u32dat);

    aml_readReg(unit, RG_QP_CDR_FORCE_IBANDLPF_R_OFF, &u32dat);
    u32dat &= ~(0x7f << 6);
    u32dat |= (0x21 << 6);
    u32dat &= ~(0x3 << 16);
    u32dat |= (0x2 << 16);
    u32dat &= ~BIT(13);
    aml_writeReg(unit, RG_QP_CDR_FORCE_IBANDLPF_R_OFF, u32dat);

    aml_readReg(unit, RG_QP_CDR_PR_KBAND_DIV_PCIE, &u32dat);
    u32dat &= ~BIT(30);
    aml_writeReg(unit, RG_QP_CDR_PR_KBAND_DIV_PCIE, u32dat);

    aml_readReg(unit, RG_QP_CDR_PR_CKREF_DIV1, &u32dat);
    u32dat &= ~(0x7 << 24);
    u32dat |= (0x4 << 24);
    aml_writeReg(unit, RG_QP_CDR_PR_CKREF_DIV1, u32dat);

    aml_readReg(unit, PLL_CTRL_0, &u32dat);
    u32dat |= BIT(0);
    aml_writeReg(unit, PLL_CTRL_0, u32dat);

    aml_readReg(unit, RX_CTRL_26, &u32dat);
    u32dat &= ~BIT(23);
    u32dat |= BIT(26);
    aml_writeReg(unit, RX_CTRL_26, u32dat);

    aml_readReg(unit, RX_DLY_0, &u32dat);
    u32dat &= ~(0xff << 0);
    u32dat |= (0x6f << 0);
    u32dat |= BITS(8, 13);
    aml_writeReg(unit, RX_DLY_0, u32dat);

    aml_readReg(unit, RX_CTRL_42, &u32dat);
    u32dat &= ~(0x1fff << 0);
    u32dat |= (0x150 << 0);
    aml_writeReg(unit, RX_CTRL_42, u32dat);

    aml_readReg(unit, RX_CTRL_2, &u32dat);
    u32dat &= ~(0x1fff << 16);
    u32dat |= (0x150 << 16);
    aml_writeReg(unit, RX_CTRL_2, u32dat);

    aml_readReg(unit, PON_RXFEDIG_CTRL_9, &u32dat);
    u32dat &= ~(0x7 << 0);
    u32dat |= (0x1 << 0);
    aml_writeReg(unit, PON_RXFEDIG_CTRL_9, u32dat);

    aml_readReg(unit, RX_CTRL_8, &u32dat);
    u32dat &= ~(0xfff << 16);
    u32dat |= (0x200 << 16);
    u32dat &= ~(0x7fff << 14);
    u32dat |= (0xfff << 14);
    aml_writeReg(unit, RX_CTRL_8, u32dat);

    /* Frequency memter */
    aml_readReg(unit, RX_CTRL_5, &u32dat);
    u32dat &= ~(0xfffff << 10);
    u32dat |= (0x10 << 10);
    aml_writeReg(unit, RX_CTRL_5, u32dat);

    aml_readReg(unit, RX_CTRL_6, &u32dat);
    u32dat &= ~(0xfffff << 0);
    u32dat |= (0x64 << 0);
    aml_writeReg(unit, RX_CTRL_6, u32dat);

    aml_readReg(unit, RX_CTRL_7, &u32dat);
    u32dat &= ~(0xfffff << 0);
    u32dat |= (0x2710 << 0);
    aml_writeReg(unit, RX_CTRL_7, u32dat);

    /* PCS Init */
    aml_readReg(unit, RG_HSGMII_PCS_CTROL_1, &u32dat);
    u32dat &= ~BIT(30);
    aml_writeReg(unit, RG_HSGMII_PCS_CTROL_1, u32dat);

    /* Rate Adaption */
    aml_readReg(unit, RATE_ADP_P0_CTRL_0, &u32dat);
    u32dat &= ~BIT(31);
    aml_writeReg(unit, RATE_ADP_P0_CTRL_0, u32dat);

    aml_readReg(unit, RG_RATE_ADAPT_CTRL_0, &u32dat);
    u32dat |= BIT(0);
    u32dat |= BIT(4);
    u32dat |= BITS(26, 27);
    aml_writeReg(unit, RG_RATE_ADAPT_CTRL_0, u32dat);

    /* Disable AN */
    aml_readReg(unit, SGMII_REG_AN0, &u32dat);
    u32dat &= ~BIT(12);
    aml_writeReg(unit, SGMII_REG_AN0, u32dat);

    /* Force Speed */
    aml_readReg(unit, SGMII_STS_CTRL_0, &u32dat);
    u32dat |= BIT(2);
    u32dat |= BITS(4, 5);
    aml_writeReg(unit, SGMII_STS_CTRL_0, u32dat);

    /* bypass flow control to MAC */
    aml_writeReg(unit, MSG_RX_LIK_STS_0, 0x01010107);
    aml_writeReg(unit, MSG_RX_LIK_STS_2, 0x00000EEF);

    return ret;
}

/* FUNCTION NAME: air_port_setSgmiiMode
 * PURPOSE:
 *      Set the port5 SGMII mode for AN or force
 *
 * INPUT:
 *      unit            --  Device ID
 *      mode            --  AIR_PORT_SGMII_MODE_AN
 *                          AIR_PORT_SGMII_MODE_FORCE
 *      speed           --  AIR_PORT_SPEED_10M:   10Mbps
 *                          AIR_PORT_SPEED_100M:  100Mbps
 *                          AIR_PORT_SPEED_1000M: 1Gbps
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
air_port_setSgmiiMode(
    const UI32_T unit,
    const UI32_T mode,
    const UI32_T speed)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T u32dat = 0;

    AIR_PARAM_CHK(((AIR_PORT_SGMII_MODE_AN != mode) && (AIR_PORT_SGMII_MODE_FORCE != mode)), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((speed >= AIR_PORT_SPEED_2500M), AIR_E_BAD_PARAMETER);

    /* PMA Init */
    /* PLL */
    aml_readReg(unit, QP_DIG_MODE_CTRL_1, &u32dat);
    u32dat &= ~BITS(2, 3);
    aml_writeReg(unit, QP_DIG_MODE_CTRL_1, u32dat);

    /* PLL - LPF */
    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~(0x3 << 0);
    u32dat |= (0x1 << 0);
    u32dat &= ~(0x7 << 2);
    u32dat |= (0x5 << 2);
    u32dat &= ~BITS(6, 7);
    u32dat &= ~(0x7 << 8);
    u32dat |= (0x3 << 8);
    u32dat |= BIT(29);
    u32dat &= ~BITS(12, 13);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    /* PLL - ICO */
    aml_readReg(unit, PLL_CTRL_4, &u32dat);
    u32dat |= BIT(2);
    aml_writeReg(unit, PLL_CTRL_4, u32dat);

    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~BIT(14);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    /* PLL - CHP */
    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~(0xf << 16);
    u32dat |= (0x4 << 16);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);


    /* PLL - PFD */
    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~(0x3 << 20);
    u32dat |= (0x1 << 20);
    u32dat &= ~(0x3 << 24);
    u32dat |= (0x1 << 24);
    u32dat &= ~BIT(26);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    /* PLL - POSTDIV */
    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat |= BIT(22);
    u32dat &= ~BIT(27);
    u32dat &= ~BIT(28);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    /* PLL - SDM */
    aml_readReg(unit, PLL_CTRL_4, &u32dat);
    u32dat &= ~BITS(3, 4);
    aml_writeReg(unit, PLL_CTRL_4, u32dat);

    aml_readReg(unit, PLL_CTRL_2, &u32dat);
    u32dat &= ~BIT(30);
    aml_writeReg(unit, PLL_CTRL_2, u32dat);

    aml_readReg(unit, SS_LCPLL_PWCTL_SETTING_2, &u32dat);
    u32dat &= ~(0x3 << 16);
    u32dat |= (0x1 << 16);
    aml_writeReg(unit, SS_LCPLL_PWCTL_SETTING_2, u32dat);

    aml_writeReg(unit, SS_LCPLL_TDC_FLT_2, 0x48000000);
    aml_writeReg(unit, SS_LCPLL_TDC_PCW_1, 0x48000000);

    aml_readReg(unit, SS_LCPLL_TDC_FLT_5, &u32dat);
    u32dat &= ~BIT(24);
    aml_writeReg(unit, SS_LCPLL_TDC_FLT_5, u32dat);

    aml_readReg(unit, PLL_CK_CTRL_0, &u32dat);
    u32dat &= ~BIT(8);
    aml_writeReg(unit, PLL_CK_CTRL_0, u32dat);

    /* PLL - SS */
    aml_readReg(unit, PLL_CTRL_3, &u32dat);
    u32dat &= ~BITS(0, 15);
    aml_writeReg(unit, PLL_CTRL_3, u32dat);

    aml_readReg(unit, PLL_CTRL_4, &u32dat);
    u32dat &= ~BITS(0, 1);
    aml_writeReg(unit, PLL_CTRL_4, u32dat);

    aml_readReg(unit, PLL_CTRL_3, &u32dat);
    u32dat &= ~BITS(16, 31);
    aml_writeReg(unit, PLL_CTRL_3, u32dat);

    /* PLL - TDC */
    aml_readReg(unit, PLL_CK_CTRL_0, &u32dat);
    u32dat &= ~BIT(9);
    aml_writeReg(unit, PLL_CK_CTRL_0, u32dat);

    aml_readReg(unit, RG_QP_PLL_SDM_ORD, &u32dat);
    u32dat |= BIT(3);
    u32dat |= BIT(4);
    aml_writeReg(unit, RG_QP_PLL_SDM_ORD, u32dat);

    aml_readReg(unit, RG_QP_RX_DAC_EN, &u32dat);
    u32dat &= ~(0x3 << 16);
    u32dat |= (0x2 << 16);
    aml_writeReg(unit, RG_QP_RX_DAC_EN, u32dat);

    /* PLL - TCL Disable (only for Co-SIM) */
    aml_readReg(unit, PON_RXFEDIG_CTRL_0, &u32dat);
    u32dat &= ~BIT(12);
    aml_writeReg(unit, PON_RXFEDIG_CTRL_0, u32dat);

    /* TX Init */
    aml_readReg(unit, RG_QP_TX_MODE_16B_EN, &u32dat);
    u32dat &= ~BIT(0);
    u32dat &= ~BITS(16, 31);
    aml_writeReg(unit, RG_QP_TX_MODE_16B_EN, u32dat);

    /* RX Init */
    aml_readReg(unit, RG_QP_RXAFE_RESERVE, &u32dat);
    u32dat |= BIT(11);
    aml_writeReg(unit, RG_QP_RXAFE_RESERVE, u32dat);

    aml_readReg(unit, RG_QP_CDR_LPF_MJV_LIM, &u32dat);
    u32dat &= ~(0x3 << 4);
    u32dat |= (0x2 << 4);
    aml_writeReg(unit, RG_QP_CDR_LPF_MJV_LIM, u32dat);

    aml_readReg(unit, RG_QP_CDR_LPF_SETVALUE, &u32dat);
    u32dat &= ~(0xf << 25);
    u32dat |= (0x1 << 25);
    u32dat &= ~(0x7 << 29);
    u32dat |= (0x6 << 29);
    aml_writeReg(unit, RG_QP_CDR_LPF_SETVALUE, u32dat);

    aml_readReg(unit, RG_QP_CDR_PR_CKREF_DIV1, &u32dat);
    u32dat &= ~(0x1f << 8);
    u32dat |= (0xc << 8);
    aml_writeReg(unit, RG_QP_CDR_PR_CKREF_DIV1, u32dat);

    aml_readReg(unit, RG_QP_CDR_PR_KBAND_DIV_PCIE, &u32dat);
    u32dat &= ~(0x3f << 0);
    u32dat |= (0x19 << 0);
    u32dat &= ~BIT(6);
    aml_writeReg(unit, RG_QP_CDR_PR_KBAND_DIV_PCIE, u32dat);

    aml_readReg(unit, RG_QP_CDR_FORCE_IBANDLPF_R_OFF, &u32dat);
    u32dat &= ~(0x7f << 6);
    u32dat |= (0x21 << 6);
    u32dat &= ~(0x3 << 16);
    u32dat |= (0x2 << 16);
    u32dat &= ~BIT(13);
    aml_writeReg(unit, RG_QP_CDR_FORCE_IBANDLPF_R_OFF, u32dat);

    aml_readReg(unit, RG_QP_CDR_PR_KBAND_DIV_PCIE, &u32dat);
    u32dat &= ~BIT(30);
    aml_writeReg(unit, RG_QP_CDR_PR_KBAND_DIV_PCIE, u32dat);

    aml_readReg(unit, RG_QP_CDR_PR_CKREF_DIV1, &u32dat);
    u32dat &= ~(0x7 << 24);
    u32dat |= (0x4 << 24);
    aml_writeReg(unit, RG_QP_CDR_PR_CKREF_DIV1, u32dat);

    aml_readReg(unit, PLL_CTRL_0, &u32dat);
    u32dat |= BIT(0);
    aml_writeReg(unit, PLL_CTRL_0, u32dat);

    aml_readReg(unit, RX_CTRL_26, &u32dat);
    u32dat &= ~BIT(23);
    if(AIR_PORT_SGMII_MODE_AN == mode)
    {
        u32dat |= BIT(26);
    }
    aml_writeReg(unit, RX_CTRL_26, u32dat);

    aml_readReg(unit, RX_DLY_0, &u32dat);
    u32dat &= ~(0xff << 0);
    u32dat |= (0x6f << 0);
    u32dat |= BITS(8, 13);
    aml_writeReg(unit, RX_DLY_0, u32dat);

    aml_readReg(unit, RX_CTRL_42, &u32dat);
    u32dat &= ~(0x1fff << 0);
    u32dat |= (0x150 << 0);
    aml_writeReg(unit, RX_CTRL_42, u32dat);

    aml_readReg(unit, RX_CTRL_2, &u32dat);
    u32dat &= ~(0x1fff << 16);
    u32dat |= (0x150 << 16);
    aml_writeReg(unit, RX_CTRL_2, u32dat);

    aml_readReg(unit, PON_RXFEDIG_CTRL_9, &u32dat);
    u32dat &= ~(0x7 << 0);
    u32dat |= (0x1 << 0);
    aml_writeReg(unit, PON_RXFEDIG_CTRL_9, u32dat);

    aml_readReg(unit, RX_CTRL_8, &u32dat);
    u32dat &= ~(0xfff << 16);
    u32dat |= (0x200 << 16);
    u32dat &= ~(0x7fff << 0);
    u32dat |= (0xfff << 0);
    aml_writeReg(unit, RX_CTRL_8, u32dat);

    /* Frequency memter */
    aml_readReg(unit, RX_CTRL_5, &u32dat);
    u32dat &= ~(0xfffff << 10);
    u32dat |= (0x28 << 10);
    aml_writeReg(unit, RX_CTRL_5, u32dat);

    aml_readReg(unit, RX_CTRL_6, &u32dat);
    u32dat &= ~(0xfffff << 0);
    u32dat |= (0x64 << 0);
    aml_writeReg(unit, RX_CTRL_6, u32dat);

    aml_readReg(unit, RX_CTRL_7, &u32dat);
    u32dat &= ~(0xfffff << 0);
    u32dat |= (0x2710 << 0);
    aml_writeReg(unit, RX_CTRL_7, u32dat);

    if(AIR_PORT_SGMII_MODE_FORCE == mode)
    {
        /* PCS Init */
        aml_readReg(unit, QP_DIG_MODE_CTRL_0, &u32dat);
        u32dat &= ~BIT(0);
        if(AIR_PORT_SPEED_1000M == speed)
        {
            u32dat &= ~BITS(4, 5);
        }
        else if(AIR_PORT_SPEED_100M == speed)
        {
            u32dat &= ~(0x3 << 4);
            u32dat |= (0x1 << 4);
        }
        else
        {
            u32dat &= ~(0x3 << 4);
            u32dat |= (0x2 << 4);
        }
        aml_writeReg(unit, QP_DIG_MODE_CTRL_0, u32dat);

        aml_readReg(unit, RG_HSGMII_PCS_CTROL_1, &u32dat);
        u32dat &= ~BIT(30);
        aml_writeReg(unit, RG_HSGMII_PCS_CTROL_1, u32dat);

        /* Rate Adaption - GMII path config. */
        aml_readReg(unit, RG_AN_SGMII_MODE_FORCE, &u32dat);
        u32dat |= BIT(0);
        if(AIR_PORT_SPEED_1000M == speed)
        {
            u32dat &= ~BITS(4, 5);
        }
        else if(AIR_PORT_SPEED_100M == speed)
        {
            u32dat &= ~(0x3 << 4);
            u32dat |= (0x1 << 4);
        }
        else
        {
            u32dat &= ~(0x3 << 4);
            u32dat |= (0x2 << 4);
        }
        aml_writeReg(unit, RG_AN_SGMII_MODE_FORCE, u32dat);

        aml_readReg(unit, SGMII_STS_CTRL_0, &u32dat);
        u32dat |= BIT(2);
        if(AIR_PORT_SPEED_1000M == speed)
        {
            u32dat &= ~(0x3 << 4);
            u32dat |= (0x2 << 4);
        }
        else if(AIR_PORT_SPEED_100M == speed)
        {
            u32dat &= ~(0x3 << 4);
            u32dat |= (0x1 << 4);
        }
        else
        {
            u32dat &= ~BITS(4, 5);
        }
        aml_writeReg(unit, SGMII_STS_CTRL_0, u32dat);

        aml_readReg(unit, SGMII_REG_AN0, &u32dat);
        u32dat &= ~BIT(12);
        aml_writeReg(unit, SGMII_REG_AN0, u32dat);

        aml_readReg(unit, PHY_RX_FORCE_CTRL_0, &u32dat);
        u32dat |= BIT(4);
        aml_writeReg(unit, PHY_RX_FORCE_CTRL_0, u32dat);

        aml_readReg(unit, RATE_ADP_P0_CTRL_0, &u32dat);
        if(AIR_PORT_SPEED_1000M == speed)
        {
            u32dat &= ~BITS(0, 3);
        }
        else if(AIR_PORT_SPEED_100M == speed)
        {
            u32dat &= ~(0xf << 0);
            u32dat |= (0xc << 0);
        }
        else
        {
            u32dat |= BITS(0, 3);
        }
        u32dat |= BIT(28);
        aml_writeReg(unit, RATE_ADP_P0_CTRL_0, u32dat);

        aml_readReg(unit, RG_RATE_ADAPT_CTRL_0, &u32dat);
        u32dat |= BIT(0);
        u32dat |= BIT(4);
        if(AIR_PORT_SPEED_1000M == speed)
        {
            u32dat |= BITS(26, 27);
        }
        else
        {
            u32dat &= ~BITS(26, 27);
        }
        aml_writeReg(unit, RG_RATE_ADAPT_CTRL_0, u32dat);

    }
    else
    {
        /* PCS Init */
        aml_readReg(unit, RG_HSGMII_PCS_CTROL_1, &u32dat);
        u32dat &= ~BIT(30);
        aml_writeReg(unit, RG_HSGMII_PCS_CTROL_1, u32dat);

        /* Set AN Ability - Interrupt */
        aml_readReg(unit, SGMII_REG_AN_FORCE_CL37, &u32dat);
        u32dat |= BIT(0);
        aml_writeReg(unit, SGMII_REG_AN_FORCE_CL37, u32dat);

        aml_readReg(unit, SGMII_REG_AN_13, &u32dat);
        u32dat &= ~(0x3f << 0);
        u32dat |= (0xb << 0);
        u32dat |= BIT(8);
        aml_writeReg(unit, SGMII_REG_AN_13, u32dat);

        /* Rate Adaption - GMII path config. */
        aml_readReg(unit, SGMII_REG_AN0, &u32dat);
        u32dat |= BIT(12);
        aml_writeReg(unit, SGMII_REG_AN0, u32dat);

        aml_readReg(unit, MII_RA_AN_ENABLE, &u32dat);
        u32dat |= BIT(0);
        aml_writeReg(unit, MII_RA_AN_ENABLE, u32dat);

        aml_readReg(unit, RATE_ADP_P0_CTRL_0, &u32dat);
        u32dat |= BIT(28);
        aml_writeReg(unit, RATE_ADP_P0_CTRL_0, u32dat);

        aml_readReg(unit, RG_RATE_ADAPT_CTRL_0, &u32dat);
        u32dat |= BIT(0);
        u32dat |= BIT(4);
        u32dat |= BITS(26, 27);
        aml_writeReg(unit, RG_RATE_ADAPT_CTRL_0, u32dat);

        /* Only for Co-SIM */

        /* AN Speed up (Only for Co-SIM) */

        /* Restart AN */
        aml_readReg(unit, SGMII_REG_AN0, &u32dat);
        u32dat |= BIT(9);
        u32dat |= BIT(15);
        aml_writeReg(unit, SGMII_REG_AN0, u32dat);
    }

    /* bypass flow control to MAC */
    aml_writeReg(unit, MSG_RX_LIK_STS_0, 0x01010107);
    aml_writeReg(unit, MSG_RX_LIK_STS_2, 0x00000EEF);

    return ret;
}

/* FUNCTION NAME: air_port_setRmiiMode
 * PURPOSE:
 *      Set the port5 RMII mode for 100Mbps or 10Mbps
 *
 * INPUT:
 *      unit            --  Device ID
 *      speed           --  AIR_PORT_SPEED_10M:  10Mbps
 *                          AIR_PORT_SPEED_100M: 100Mbps
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
air_port_setRmiiMode(
    const UI32_T unit,
    const UI32_T speed)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T u32dat = 0;

    /* Mistake proofing for speed checking */
    AIR_PARAM_CHK((speed >= AIR_PORT_SPEED_1000M), AIR_E_BAD_PARAMETER);

    if(AIR_PORT_SPEED_100M == speed)
    {
        aml_writeReg(unit, PMCR(5), 0x93159000);
        aml_writeReg(unit, RG_P5MUX_MODE, 0x301);
        aml_writeReg(unit, RG_FORCE_CKDIR_SEL, 0x101);
        aml_writeReg(unit, RG_SWITCH_MODE, 0x101);
        aml_writeReg(unit, RG_FORCE_MAC5_SB, 0x1010101);
        aml_writeReg(unit, CSR_RMII, 0x420102);
        aml_writeReg(unit, RG_RGMII_TXCK_C, 0x1100910);
    }
    else
    {
        aml_writeReg(unit, PMCR(5), 0x83159000);
        aml_writeReg(unit, RG_P5MUX_MODE, 0x301);
        aml_writeReg(unit, RG_FORCE_CKDIR_SEL, 0x101);
        aml_writeReg(unit, RG_SWITCH_MODE, 0x101);
        aml_writeReg(unit, RG_FORCE_MAC5_SB, 0x1000101);
        aml_writeReg(unit, CSR_RMII, 0x420102);
        aml_writeReg(unit, RG_RGMII_TXCK_C, 0x1100910);
    }

    return ret;
}

/* FUNCTION NAME: air_port_setRgmiiMode
 * PURPOSE:
 *      Set the port5 RGMII mode for 1Gbps or 100Mbps or 10Mbps
 *
 * INPUT:
 *      unit            --  Device ID
 *      speed           --  AIR_PORT_SPEED_10M:   10Mbps
 *                          AIR_PORT_SPEED_100M:  100Mbps
 *                          AIR_PORT_SPEED_1000M: 1Gbps
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
air_port_setRgmiiMode(
    const UI32_T unit,
    const UI32_T speed)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T u32dat = 0;

    /* Mistake proofing for speed checking */
    AIR_PARAM_CHK((speed >= AIR_PORT_SPEED_2500M), AIR_E_BAD_PARAMETER);

    if(AIR_PORT_SPEED_1000M == speed)
    {
        aml_writeReg(unit, PMCR(5), 0xa3159000);
        aml_writeReg(unit, RG_FORCE_MAC5_SB, 0x20101);
    }
    else if(AIR_PORT_SPEED_100M == speed)
    {
        aml_writeReg(unit, PMCR(5), 0x93159000);
        aml_writeReg(unit, RG_FORCE_MAC5_SB, 0x10101);
    }
    else
    {
        aml_writeReg(unit, PMCR(5), 0x83159000);
        aml_writeReg(unit, RG_FORCE_MAC5_SB, 0x101);
    }

    return ret;
}


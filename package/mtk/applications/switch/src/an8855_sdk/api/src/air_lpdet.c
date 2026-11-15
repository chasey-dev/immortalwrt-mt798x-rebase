/* FILE NAME: air_lpdet.c
 * PURPOSE:
 *      Define the loop detect function in AIR SDK.
 *
 * NOTES:
 *      None
 */

/* INCLUDE FILE DECLARATIONS
*/
#include "air.h"

/* NAMING CONSTANT DECLARATIONS
*/
#define AIR_MAC_ADDR_LEN       (6)

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
*/
/* FUNCTION NAME: _air_lpdet_checkMac
 * PURPOSE:
 *      Check MAC address is valid to be loop detect frame source mac.
 * INPUT:
 *      unit                     -- Device ID
 *      mac                      -- Source MAC address
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Mac address is valid.
 *      AIR_E_BAD_PARAMETER      -- Mac address is invalid.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_air_lpdet_checkMac(
    const AIR_MAC_T mac)
{
    AIR_MAC_T bc_mac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    AIR_MAC_T all_zero_mac = {0, 0, 0, 0, 0, 0};

    if (0 == memcmp(mac, bc_mac, AIR_MAC_ADDR_LEN))
    {
        return AIR_E_BAD_PARAMETER;
    }
    else if (0 == memcmp(mac, all_zero_mac, AIR_MAC_ADDR_LEN))
    {
        return AIR_E_BAD_PARAMETER;
    }
    else if (((0x01) & (mac[0])))
    {
        /* multicast */
        return AIR_E_BAD_PARAMETER;
    }
    else
    {
        return AIR_E_OK;
    }
}
/* FUNCTION NAME:   air_lpdet_setLdfSrcMac
 * PURPOSE:
 *      Set the loop detect frame source MAC address.
 * INPUT:
 *      unit                     -- Device ID
 *      mac                      -- Source MAC address
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      It's unique and specified for loop frame.
 */
AIR_ERROR_NO_T
air_lpdet_setLdfSrcMac(
    const UI32_T    unit,
    const AIR_MAC_T mac)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          u32dat = 0;

    /* Check and set source mac */
    rc = _air_lpdet_checkMac(mac);
    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, LPDET_SA_MSB, &u32dat);
        if (AIR_E_OK == rc)
        {
            u32dat &= ~LPDET_SMAC_MASK;
            u32dat |= BITS_OFF_L(mac[0], 8, 8);
            u32dat |= BITS_OFF_L(mac[1], 0, 8);
            rc = aml_writeReg(unit, LPDET_SA_MSB, u32dat);
            if (AIR_E_OK == rc)
            {
                rc = aml_readReg(unit, LPDET_SA_LSB, &u32dat);
                if (AIR_E_OK == rc)
                {
                    u32dat = BITS_OFF_L(mac[2], 24, 8);
                    u32dat |= BITS_OFF_L(mac[3], 16, 8);
                    u32dat |= BITS_OFF_L(mac[4], 8, 8);
                    u32dat |= BITS_OFF_L(mac[5], 0, 8);
                    rc = aml_writeReg(unit, LPDET_SA_LSB, u32dat);
                }
            }
        }
    }
    return rc;
}

/* FUNCTION NAME:   air_lpdet_getLdfSrcMac
 * PURPOSE:
 *      Get loop detect frame source MAC address.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      mac                      -- Source MAC address
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      It's unique and specified for loop frame.
 */
AIR_ERROR_NO_T
air_lpdet_getLdfSrcMac(
    const UI32_T unit,
    AIR_MAC_T    mac)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          u32dat = 0;

    rc = aml_readReg(unit, LPDET_SA_MSB, &u32dat);
    if (AIR_E_OK == rc)
    {
        /* Get source mac address */
        mac[0] = BITS_OFF_R(u32dat, 8, 8);
        mac[1] = BITS_OFF_R(u32dat, 0, 8);

        rc = aml_readReg(unit, LPDET_SA_LSB, &u32dat);
        if (AIR_E_OK == rc)
        {
            mac[2] = BITS_OFF_R(u32dat, 24, 8);
            mac[3] = BITS_OFF_R(u32dat, 16, 8);
            mac[4] = BITS_OFF_R(u32dat, 8, 8);
            mac[5] = BITS_OFF_R(u32dat, 0, 8);
        }
    }
    return rc;
}

/* FUNCTION NAME:   air_lpdet_setCtrl
 * PURPOSE:
 *      Set the loop detect control.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 *      enable                   -- FALSE: Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lpdet_setCtrl(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const BOOL_T                    enable)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          u32dat = 0, mac_port = 0, reg_addr = 0;

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = LPDETTXCR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = LPDETRXCR;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, reg_addr, &u32dat);
        if (AIR_E_OK == rc)
        {
            if (TRUE == enable)
            {
                u32dat |= (1 << port);
            }
            else
            {
                u32dat &= ~(1 << port);
            }
            rc = aml_writeReg(unit, reg_addr, u32dat);
        }
    }
    return rc;
}

/* FUNCTION NAME:   air_lpdet_getCtrl
 * PURPOSE:
 *      Get the loop detect control.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 * OUTPUT:
 *      ptr_enable               -- FALSE: Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lpdet_getCtrl(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    BOOL_T                          *ptr_enable)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI32_T          u32dat = 0, mac_port = 0, reg_addr = 0;

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = LPDETTXCR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = LPDETRXCR;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, reg_addr, &u32dat);
        if (AIR_E_OK == rc)
        {
            *ptr_enable = (u32dat & (1 << port)) ? TRUE : FALSE;
        }
    }
    return rc;
}

/* FUNCTION NAME:   air_lpdet_clearStatus
 * PURPOSE:
 *      Clear the loop detect status.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 *      port_bitmap              -- Loop status port bitmap
 *                                  AIR_PORT_BITMAP_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lpdet_clearStatus(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const AIR_PORT_BITMAP_T         port_bitmap)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              reg_addr = 0;

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = LPDETTXSR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = LPDETRXSR;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        rc = aml_writeReg(unit, reg_addr, port_bitmap[0]);
    }
    return rc;
}

/* FUNCTION NAME:   air_lpdet_getStatus
 * PURPOSE:
 *      Get the loop detect status.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 * OUTPUT:
 *      port_bitmap              -- Loop status port bitmap
 *                                  AIR_PORT_BITMAP_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lpdet_getStatus(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    AIR_PORT_BITMAP_T               port_bitmap)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              reg_addr = 0;

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = LPDETTXSR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = LPDETRXSR;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, reg_addr, &(port_bitmap[0]));
    }
    return rc;
}
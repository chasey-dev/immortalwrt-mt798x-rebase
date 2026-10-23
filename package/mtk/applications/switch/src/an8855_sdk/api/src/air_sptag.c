/* FILE NAME: air_sptag.c
 * PURPOSE:
 *      Define the Special Tag function in AIR SDK.
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
/* FUNCTION NAME: air_sptag_setState
 * PURPOSE:
 *      Set special tag enable/disable for port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Special tag Port
 *      sp_en           --  special tag Enable or Disable
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
air_sptag_setState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T sp_en)
{
    UI32_T udat32 = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK(port > AIR_MAX_NUM_OF_PORTS, AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != sp_en) && (FALSE != sp_en)), AIR_E_BAD_PARAMETER);

    /* Read PVC */
    aml_readReg(unit, PVC(port), &udat32);
    AIR_PRINT("PVC REG:%x. val:%x\n", PVC(port),udat32);

    /* Set special tag enable or disable */
    udat32 &= ~BITS_RANGE(PVC_SPTAG_EN_OFFT, PVC_SPTAG_EN_LENG);
    udat32 |= (sp_en << PVC_SPTAG_EN_OFFT);

    /* Write PVC */
    aml_writeReg(unit, PVC(port), udat32);
    AIR_PRINT("PVC REG:%x. val:%x\n", PVC(port),udat32);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_switch_getCpuPortEn
 * PURPOSE:
 *      Get CPU port member
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Special tag Port
 *
 * OUTPUT:
 *      sp_en           --  special tag enable or disable
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sptag_getState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *sp_en)
{
    UI32_T udat32 = 0;

    /* Mistake proofing */
    AIR_CHECK_PTR(sp_en);
    AIR_PARAM_CHK(port > AIR_MAX_NUM_OF_PORTS, AIR_E_BAD_PARAMETER);

    /* Read PVC */
    aml_readReg(unit, PVC(port), &udat32);

    /* Get special tag state */
    (*sp_en) = BITS_OFF_R(udat32, PVC_SPTAG_EN_OFFT, PVC_SPTAG_EN_LENG);

    return AIR_E_OK;
}


/* FUNCTION NAME: air_sptag_setMode
 * PURPOSE:
 *      Set special tag enable/disable for port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Special tag Port
 *      mode            --  insert mode or replace mode
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
air_sptag_setMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T mode)
{
    UI32_T udat32 = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK(port > AIR_MAX_NUM_OF_PORTS, AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != mode) && (FALSE != mode)), AIR_E_BAD_PARAMETER);

    /* Read PVC */
    aml_readReg(unit, PVC(port), &udat32);
    AIR_PRINT("PVC REG:%x. val:%x\n", PVC(port),udat32);

    /* Set special tag enable or disable */
    udat32 &= ~BITS_RANGE(PVC_SPTAG_MODE_OFFT, PVC_SPTAG_MODE_LENG);
    udat32 |= (mode << PVC_SPTAG_MODE_OFFT);

    /* Write PVC */
    aml_writeReg(unit, PVC(port), udat32);
    AIR_PRINT("PVC REG:%x. val:%x\n", PVC(port),udat32);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_sptag_getMode
 * PURPOSE:
 *      Get CPU port member
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Special tag Port
 *
 * OUTPUT:
 *      mode            --  insert or replace mode
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sptag_getMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *mode)
{
    UI32_T udat32 = 0;

    /* Mistake proofing */
    AIR_CHECK_PTR(mode);
    AIR_PARAM_CHK(port > AIR_MAX_NUM_OF_PORTS, AIR_E_BAD_PARAMETER);

    /* Read PVC */
    aml_readReg(unit, PVC(port), &udat32);

    /* Get special tag mode */
    (*mode) = BITS_OFF_R(udat32, PVC_SPTAG_MODE_OFFT, PVC_SPTAG_MODE_LENG);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_sptag_encodeTx
 * PURPOSE:
 *      Encode tx special tag into buffer.
 * INPUT:
 *      unit            --  Device ID
 *      ptr_sptag_tx    --  Special tag parameters
 *      ptr_buf         --  Buffer address
 *      ptr_len         --  Buffer length
 * OUTPUT:
 *      ptr_len         --  Written buffer length
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sptag_encodeTx(
    const UI32_T unit,
    const AIR_STAG_MODE_T mode,
    AIR_STAG_TX_PARA_T *ptr_stag_tx,
    UI8_T *ptr_buf,
    UI32_T *ptr_len)
{
    UI32_T port = 0, byte_off = 0, bit_off = 0;
    BOOL_T found = FALSE;
    UI16_T mac_pbmp;

    AIR_PARAM_CHK(((ptr_stag_tx->opc <  AIR_STAG_OPC_PORTMAP) ||(ptr_stag_tx->opc > AIR_STAG_OPC_LOOKUP)),AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((ptr_stag_tx->vpm <  AIR_STAG_VPM_UNTAG) ||(ptr_stag_tx->opc > AIR_STAG_VPM_TPID_PRE_DEFINED)),AIR_E_BAD_PARAMETER);

    mac_pbmp = ptr_stag_tx->pbm;

    /* insert mode only support port map */
    if ((AIR_STAG_MODE_INSERT == mode)
        && ((ptr_stag_tx->opc != AIR_STAG_OPC_PORTMAP) && (ptr_stag_tx->opc != AIR_STAG_OPC_LOOKUP)))
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* clear output buffer */
    memset(ptr_buf, 0, AIR_STAG_BUF_LEN);
    AIR_PRINT("air_sptag_encode:mac_pbmp=%x\n", mac_pbmp);

    ptr_buf[0] |= BITS_OFF_L(ptr_stag_tx->opc, AIR_STAG_TX_OPC_BIT_OFFSET, AIR_STAG_TX_OPC_BIT_WIDTH);
    if (AIR_STAG_MODE_INSERT == mode)
    {   /*insert only support bitmap , opc always 000*/
        AIR_PORT_FOREACH(mac_pbmp, port)
        {
            ptr_buf[1] |= (0x1 << port);
            AIR_PRINT("air_sptag_encode:port=%d,value = %x\n", port,(0x1 << port));
        }
    }
    else
    {
        ptr_buf[0] |= BITS_OFF_L(ptr_stag_tx->vpm, AIR_STAG_TX_VPM_BIT_OFFSET, AIR_STAG_TX_VPM_BIT_WIDTH);
        ptr_buf[0] |= BITS_OFF_L(ptr_stag_tx->opc, AIR_STAG_TX_OPC_BIT_OFFSET, AIR_STAG_TX_OPC_BIT_WIDTH);
        if (AIR_STAG_OPC_PORTMAP == ptr_stag_tx->opc)
        {
            AIR_PORT_FOREACH(mac_pbmp, port)
            {
                ptr_buf[1] |= 0x1 << port;

            }
        }
        else if (AIR_STAG_OPC_PORTID == ptr_stag_tx->opc)
        {
            AIR_PORT_FOREACH(mac_pbmp, port)
            {
                if (TRUE ==found)
                {
                    return AIR_E_BAD_PARAMETER;
                }
                ptr_buf[1] |= port;
                found = TRUE;
            }
        }
        AIR_PRINT("air_sptag_encode:pri = %d,cfi = %d,vid = %d\n", ptr_stag_tx->pri,ptr_stag_tx->cfi,ptr_stag_tx->vid);

        ptr_buf[2] |= BITS_OFF_L(ptr_stag_tx->pri, AIR_STAG_TX_PCP_BIT_OFFSET, AIR_STAG_TX_PCP_BIT_WIDTH);
        ptr_buf[2] |= BITS_OFF_L(ptr_stag_tx->cfi, AIR_STAG_TX_DEI_BIT_OFFSET, AIR_STAG_TX_DEI_BIT_WIDTH);
        ptr_buf[2] |= BITS_OFF_L((ptr_stag_tx->vid >> AIR_STAG_ALIGN_BIT_WIDTH), 0,
            (AIR_STAG_ALIGN_BIT_WIDTH - AIR_STAG_TX_PCP_BIT_WIDTH - AIR_STAG_TX_DEI_BIT_WIDTH));
        AIR_PRINT("air_sptag_encode:pbuf[2] %02x\n", ptr_buf[2]);
        ptr_buf[3] |= BITS_OFF_L((ptr_stag_tx->vid & 0xFF), 0, AIR_STAG_ALIGN_BIT_WIDTH);
        AIR_PRINT("air_sptag_encode:pbuf[3] %02x\n", ptr_buf[3]);
    }

    *ptr_len = AIR_STAG_BUF_LEN;

    return AIR_E_OK;
}

/* FUNCTION NAME: air_sptag_decodeRx
 * PURPOSE:
 *      Decode rx special tag from buffer.
 * INPUT:
 *      unit            --  Device ID
 *      ptr_buf         --  Buffer address
 *      len             --  Buffer length
 * OUTPUT:
 *      ptr_sptag_rx    --  Special tag parameters
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sptag_decodeRx(
    const UI32_T unit,
    const UI8_T *ptr_buf,
    const UI32_T len,
    AIR_SPTAG_RX_PARA_T *ptr_sptag_rx)
{
    AIR_CHECK_PTR(ptr_buf);
    AIR_CHECK_PTR(ptr_sptag_rx);
    AIR_PARAM_CHK((len != AIR_STAG_BUF_LEN), AIR_E_BAD_PARAMETER);

    ptr_sptag_rx->vpm  = BITS_OFF_R(ptr_buf[0], 0, 2);
    ptr_sptag_rx->rsn  = BITS_OFF_R(ptr_buf[0], 2, 3);
    ptr_sptag_rx->spn  = BITS_OFF_R(ptr_buf[1], 0, 5);
    ptr_sptag_rx->pri  = BITS_OFF_R(ptr_buf[2], 5, 3);
    ptr_sptag_rx->cfi  = BITS_OFF_R(ptr_buf[2], 4, 1);
    ptr_sptag_rx->vid  = BITS_OFF_R(ptr_buf[2], 0, 4);
    ptr_sptag_rx->vid  = (ptr_sptag_rx->vid << 8) | ptr_buf[3];

    AIR_PARAM_CHK((ptr_sptag_rx->vpm >= AIR_STAG_REASON_CODE_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((ptr_sptag_rx->rsn >= AIR_STAG_VPM_LAST), AIR_E_BAD_PARAMETER);

    return AIR_E_OK;
}


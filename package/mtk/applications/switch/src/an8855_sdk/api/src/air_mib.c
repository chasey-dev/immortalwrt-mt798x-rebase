/* FILE NAME: air_mib.c
 * PURPOSE:
 *      Define the MIB counter function in AIR SDK.
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
#define MIB_READ_DATA(unit, port, mib, reg, val)    \
    do{                                             \
        aml_readReg(unit, MIB_##reg(port), &val );  \
        mib -> reg = val;                           \
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

/* EXPORTED SUBPROGRAM BODIES
*/
/* FUNCTION NAME: air_mib_setEnable
 * PURPOSE:
 *      Enable or Disable mib count fucntion.
 *
 * INPUT:
 *      unit           --   Device ID
 *      mib_en         --   enable or disable mib_en
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
air_mib_setEnable(
    const UI32_T unit,
    const BOOL_T mib_en)
{
    UI32_T u32dat = 0;
    AIR_PARAM_CHK(((TRUE != mib_en) && (FALSE != mib_en)), AIR_E_BAD_PARAMETER);

    /* Write data to register */
    aml_readReg(unit, MIB_CCR, &u32dat);
    if(mib_en)
    {
        u32dat |= MIB_CCR_MIB_ENABLE;
    }
    else
    {
        u32dat &= ~MIB_CCR_MIB_ENABLE;
    }
    aml_writeReg(unit, MIB_CCR, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_mib_getEnable
 * PURPOSE:
 *      Enable or Disable mib count fucntion.
 *
 * INPUT:
 *      unit           --   Device ID
 *
 * OUTPUT:
 *      mib_en         --   enable or disable mib_en

 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mib_getEnable(
    const UI32_T unit,
    BOOL_T *mib_en)
{
    UI32_T u32dat = 0;
    /* Mistake proofing */
    AIR_CHECK_PTR(mib_en);


    /* Write data to register */
    aml_readReg(unit, MIB_CCR, &u32dat);
    (*mib_en) = BITS_OFF_R(u32dat, MIB_CCR_MIB_ENABLE_OFFSET, MIB_CCR_MIB_ENABLE_LENGTH);


    return AIR_E_OK;
}
/* FUNCTION NAME: air_mib_clear
 * PURPOSE:
 *      Clear all counters of all MIB counters.
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
air_mib_clear(
    const UI32_T unit)
{
    UI32_T u32dat = 0;

    /* Write data to register */
    aml_readReg(unit, MIB_CCR, &u32dat);
    /* Restart MIB counter */
    u32dat &= ~MIB_CCR_MIB_ENABLE;
    aml_writeReg(unit, MIB_CCR, u32dat);
    u32dat |= MIB_CCR_MIB_ENABLE;
    aml_writeReg(unit, MIB_CCR, u32dat);

    return AIR_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
*/
/* FUNCTION NAME: air_mib_clear_by_port
 * PURPOSE:
 *      Clear all counters of all MIB counters.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  clear port number
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
air_mib_clear_by_port(
    const UI32_T unit,
    const UI32_T port)
{
    /* Mistake proofing */
    AIR_PARAM_CHK(port > AIR_MAX_NUM_OF_PORTS, AIR_E_BAD_PARAMETER);

    /* Write data to register */
    aml_writeReg(unit, MIB_PCLR, 1 << port);
    aml_writeReg(unit, MIB_PCLR, 0);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_mib_get
 * PURPOSE:
 *      Get the structure of MIB counter for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_rx_mib      --  MIB Counters of Rx Event
 *      ptr_tx_mib      --  MIB Counters of Tx Event
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mib_get(
    const UI32_T unit,
    const UI32_T port,
    AIR_MIB_CNT_RX_T *ptr_rx_mib,
    AIR_MIB_CNT_TX_T *ptr_tx_mib)
{
    UI32_T u32dat = 0, u32dat_h = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_rx_mib);
    AIR_CHECK_PTR(ptr_tx_mib);

    /* Read data from register */

    /* Read Tx MIB Counter */
    MIB_READ_DATA(unit, port, ptr_tx_mib, TDPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TCRC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TUPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TMPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TBPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TCEC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TSCEC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TMCEC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TDEC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TLCEC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TXCEC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TPPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TL64PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TL65PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TL128PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TL256PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TL512PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TL1024PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TL1519PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_tx_mib, TODPC, u32dat);
    aml_readReg(unit, MIB_TOCL(port), &u32dat);
    aml_readReg(unit, MIB_TOCH(port), &u32dat_h);
    ptr_tx_mib->TOC = u32dat | ((UI64_T)(u32dat_h) << 32);
    u32dat = 0;
    u32dat_h = 0;
    aml_readReg(unit, MIB_TOCL2(port), &u32dat);
    aml_readReg(unit, MIB_TOCH2(port), &u32dat_h);
    ptr_tx_mib->TOC2 = u32dat | ((UI64_T)(u32dat_h) << 32);

    /* Read Rx MIB Counter */
    MIB_READ_DATA(unit, port, ptr_rx_mib, RDPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RFPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RUPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RMPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RBPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RAEPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RCEPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RUSPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RFEPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, ROSPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RJEPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RPPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RL64PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RL65PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RL128PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RL256PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RL512PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RL1024PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RL1519PC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RCDPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RIDPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RADPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, FCDPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, WRDPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, MRDPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, SFSPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, SFTPC, u32dat);
    MIB_READ_DATA(unit, port, ptr_rx_mib, RXC_DPC, u32dat);
    u32dat = 0;
    u32dat_h = 0;
    aml_readReg(unit, MIB_ROCL(port), &u32dat);
    aml_readReg(unit, MIB_ROCH(port), &u32dat_h);
    ptr_rx_mib->ROC = u32dat | ((UI64_T)(u32dat_h) << 32);
    u32dat = 0;
    u32dat_h = 0;
    aml_readReg(unit, MIB_ROCL2(port), &u32dat);
    aml_readReg(unit, MIB_ROCH2(port), &u32dat_h);
    ptr_rx_mib->ROC2 = u32dat | ((UI64_T)(u32dat_h) << 32);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_mib_clearAclEvent
 * PURPOSE:
 *      Clear all counters of ACL event
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
air_mib_clearAclEvent(
    const UI32_T unit)
{
    UI32_T u32dat = 0;

    aml_readReg(unit, ACL_MIB_CNT_CFG, &u32dat);
    u32dat |= CSR_ACL_MIB_CLEAR;
    aml_writeReg(unit, ACL_MIB_CNT_CFG, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_mib_getAclEvent
 * PURPOSE:
 *      Get the total number of ACL event occurred.
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Index of ACL event
 *
 * OUTPUT:
 *      ptr_cnt         --  The total number of ACL event occured
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mib_getAclEvent(
    const UI32_T unit,
    const UI32_T idx,
    UI32_T *ptr_cnt)
{
    UI32_T reg = 0;
    UI32_T u32dat = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK((idx >= AIR_MIB_MAX_ACL_EVENT_NUM), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_cnt);

    aml_readReg(unit, ACL_MIB_CNT_CFG, &u32dat);
    u32dat = u32dat | (idx << CSR_ACL_MIB_SEL_OFFSET);
    aml_writeReg(unit, ACL_MIB_CNT_CFG, u32dat);

    aml_readReg(unit, ACL_MIB_CNT, &u32dat);
    (*ptr_cnt) = u32dat;

    return AIR_E_OK;
}


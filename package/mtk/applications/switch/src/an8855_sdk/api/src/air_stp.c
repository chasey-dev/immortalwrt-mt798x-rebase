/* FILE NAME: air_stp.c
 * PURPOSE:
 *      Define the STP function in AIR SDK.
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
/* FUNCTION NAME: air_stp_setPortstate
 * PURPOSE:
 *      Set the STP port state for a specifiec port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      fid             --  Filter ID for MSTP
 *      state           --  AIR_STP_STATE_DISABLE
 *                          AIR_STP_STATE_LISTEN
 *                          AIR_STP_STATE_LEARN
 *                          AIR_STP_STATE_FORWARD
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
air_stp_setPortstate(
    const UI32_T unit,
    const UI8_T port,
    const UI8_T fid,
    const AIR_STP_STATE_T state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for fid checking */
    AIR_PARAM_CHK((fid >= AIR_STP_FID_NUMBER), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for state checking */
    AIR_PARAM_CHK((state >= AIR_STP_STATE_LAST), AIR_E_BAD_PARAMETER);

    /* Read data from register */
    aml_readReg(unit, SSC(port), &u32dat);

    /* Write data to register */
    u32dat &= ~BITS(fid*2, (fid*2)+1);
    u32dat |= BITS_OFF_L(state, (fid*2), 2);
    aml_writeReg(unit, SSC(port), u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_stp_getPortstate
 * PURPOSE:
 *      Get the STP port state for a specifiec port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      fid             --  Filter ID for MSTP
 *
 * OUTPUT:
 *      ptr_state       --  AIR_STP_STATE_DISABLE
 *                          AIR_STP_STATE_LISTEN
 *                          AIR_STP_STATE_LEARN
 *                          AIR_STP_STATE_FORWARD
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */

AIR_ERROR_NO_T
air_stp_getPortstate(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T fid,
    AIR_STP_STATE_T *ptr_state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for fid checking */
    AIR_PARAM_CHK((fid >= AIR_STP_FID_NUMBER), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for state checking */
    AIR_CHECK_PTR(ptr_state);

    /* Read data from register */
    aml_readReg(unit, SSC(port), &u32dat);
    (*ptr_state) = BITS_OFF_R(u32dat, fid*2, 2);

    return AIR_E_OK;
}


/* FILE NAME: air_stp.h
 * PURPOSE:
 *      Define the STP function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_STP_H
#define AIR_STP_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/

/* Definition of STP state
 * 2'b00: Disable(STP)    / Discard(RSTP)
 * 2'b01: Listening(STP)  / Discard(RSTP)
 * 2'b10: Learning(STP)   / Learning(RSTP)
 * 2'b11: Forwarding(STP) / Forwarding(RSTP)
 * */
typedef enum
{
    AIR_STP_STATE_DISABLE,
    AIR_STP_STATE_LISTEN,
    AIR_STP_STATE_LEARN,
    AIR_STP_STATE_FORWARD,
    AIR_STP_STATE_LAST
}AIR_STP_STATE_T;

#define AIR_STP_FID_NUMBER    (16)

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const AIR_STP_STATE_T state);

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
    AIR_STP_STATE_T *ptr_state);

#endif /* End of AIR_STP_H */

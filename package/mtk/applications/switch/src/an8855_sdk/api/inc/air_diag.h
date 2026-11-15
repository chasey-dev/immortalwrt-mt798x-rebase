/* FILE NAME: air_diag.h
 * PURPOSE:
 *      Define the diagnostic function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_DIAG_H
#define AIR_DIAG_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/
typedef enum
{
    AIR_DIAG_TXCOMPLY_MODE_10M_NLP,
    AIR_DIAG_TXCOMPLY_MODE_10M_RANDOM,
    AIR_DIAG_TXCOMPLY_MODE_10M_SINE,
    AIR_DIAG_TXCOMPLY_MODE_100M_PAIR_A,
    AIR_DIAG_TXCOMPLY_MODE_100M_PAIR_B,
    AIR_DIAG_TXCOMPLY_MODE_1000M_TM1,
    AIR_DIAG_TXCOMPLY_MODE_1000M_TM2,
    AIR_DIAG_TXCOMPLY_MODE_1000M_TM3,
    AIR_DIAG_TXCOMPLY_MODE_1000M_TM4,
    AIR_DIAG_TXCOMPLY_MODE_LAST
}AIR_DIAG_TXCOMPLY_MODE_T;

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const AIR_DIAG_TXCOMPLY_MODE_T mode);

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
    AIR_DIAG_TXCOMPLY_MODE_T *ptr_mode);

#endif /* End of AIR_DIAG_H */

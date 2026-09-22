/* FILE NAME:   air_cmd.h
 * PURPOSE:
 *      Define the command line function in AIR SDK.
 * NOTES:
 */

#ifndef AIR_CMD_H
#define AIR_CMD_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   air_parse_cmd
 * PURPOSE:
 *      This function is used process diagnostic cmd
 * INPUT:
 *      argc         -- parameter number
 *      argv         -- parameter strings
 * OUTPUT:
 *      None
 * RETURN:
 *      NPS_E_OK     -- Successfully read the data.
 *      NPS_E_OTHERS -- Failed to read the data.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
air_parse_cmd(
    const UI32_T argc,
    const C8_T **argv);

UI32_T
_strtoul(
    const C8_T *cp,
    C8_T **endp,
    UI32_T base);

#endif  /* AIR_CMD_H */


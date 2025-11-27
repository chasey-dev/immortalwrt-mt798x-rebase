/* FILE NAME:   air_error.c
 * PURPOSE:
 *      Define the software modules in AIR SDK.
 * NOTES:
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
static C8_T *_air_error_cause[AIR_E_LAST] =
{
    "OK",
    "NOT_OK",
    "BAD_PARAMETER",
    "TABLE_FULL",
    "ENTRY_NOT_FOUND",
    "ENTRY_EXISTS",
    "NOT_SUPPORT",
    "TIMEOUT",
};

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   air_error_getString
 * PURPOSE:
 *      To obtain the error string of the specified error code
 *
 * INPUT:
 *      The specified error code
 * OUTPUT:
 *      None
 * RETURN:
 *      Pointer to the target error string
 *
 * NOTES:
 *
 *
 */
C8_T *
air_error_getString(
const AIR_ERROR_NO_T cause )
{
    if(cause < AIR_E_LAST)
    {
        return _air_error_cause[cause];
    }
    else
    {
        return "";
    }
}


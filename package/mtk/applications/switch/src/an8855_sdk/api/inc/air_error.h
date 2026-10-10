/* FILE NAME:   air_error.h
 * PURPOSE:
 *      Define the error code in AIR SDK.
 * NOTES:
 */

#ifndef AIR_ERROR_H
#define AIR_ERROR_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    AIR_E_OK = 0,              /* Ok and no error */
    AIR_E_OTHERS,              /* Operation is unsuccessful */
    AIR_E_BAD_PARAMETER,       /* Parameter is wrong */
    AIR_E_TABLE_FULL,          /* Table is full */
    AIR_E_ENTRY_NOT_FOUND,     /* Entry is not found */
    AIR_E_ENTRY_EXISTS,        /* Entry already exists */
    AIR_E_NOT_SUPPORT,         /* Feature is not supported */
    AIR_E_TIMEOUT,             /* Time out error */
    AIR_E_NO_MEMORY,           /* No memory is available. */
    AIR_E_LAST
} AIR_ERROR_NO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   air_error_getString
 * PURPOSE:
 *      To obtain the error string of the specified error code
 *
 * INPUT:
 *      cause  -- The specified error code
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
    const AIR_ERROR_NO_T cause );

#endif  /* AIR_ERROR_H */


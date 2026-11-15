/* FILE NAME: air_mirror.h
 * PURPOSE:
 *      Define the port mirror function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_MIRROR_H
#define AIR_MIRROR_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/
#define AIR_MAX_MIRROR_SESSION  (2)
/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* mirror session */
typedef struct AIR_MIR_SESSION_S
{
#define AIR_MIR_SESSION_FLAGS_ENABLE            (1U << 0)
#define AIR_MIR_SESSION_FLAGS_DIR_TX            (1U << 1)
#define AIR_MIR_SESSION_FLAGS_DIR_RX            (1U << 2)
#define AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG   (1U << 3)

    /* flags refer to AIR_MIR_SESSION_FLAGS_XXX */
    UI32_T                              flags;
    UI32_T                              dst_port;
    UI32_T                              src_port;
} AIR_MIR_SESSION_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: air_mir_addSession
* PURPOSE:
*      This API is used to add or set a mirror session.
* INPUT:
*      unit        --   Device unit number
*      session_id  --   The session information
*      ptr_session --   The session information
* OUTPUT:
*       None
*
* RETURN:
*       AIR_E_OK
*       AIR_E_BAD_PARAMETER
*
* NOTES:
*       None
*/
AIR_ERROR_NO_T
air_mir_addSession(
    const UI32_T unit,
    const UI32_T session_id,
    const AIR_MIR_SESSION_T *ptr_session);

/* FUNCTION NAME: air_mir_delSession
* PURPOSE:
*      This API is used to delete a mirror session.
* INPUT:
*      unit        --   Device unit number
*      session_id  --   The session information
* OUTPUT:
*       None
*
* RETURN:
*       AIR_E_OK
*       AIR_E_BAD_PARAMETER
*
* NOTES:
*       None
*/
AIR_ERROR_NO_T
air_mir_delSession(
    const UI32_T unit,
    const UI32_T session_id);


/* FUNCTION NAME: air_mir_getSession
* PURPOSE:
*      This API is used to get mirror session information.
* INPUT:
*      unit         --  Device unit number
*      session_id   --   The session information
* OUTPUT:
*      ptr_session  --  The information of this session to be obtained
* RETURN:
*       AIR_E_OK
*       AIR_E_BAD_PARAMETER
*
* NOTES:
*       None
*/
AIR_ERROR_NO_T
air_mir_getSession(
    const UI32_T unit,
    const UI32_T session_id,
    AIR_MIR_SESSION_T *ptr_session);


/* FUNCTION NAME: air_mir_setSessionAdminMode
* PURPOSE:
*      This API is used to set mirror session state
* INPUT:
*      unit         --  Device unit number
*      session_id   --  mirror session id
*      state        --  FALSE: disable
*                       TRUE:  enable
* OUTPUT:
*      None
* RETURN:
*       AIR_E_OK
*       AIR_E_BAD_PARAMETER
*
* NOTES:
*       None
*/
AIR_ERROR_NO_T
air_mir_setSessionAdminMode(
    const UI32_T unit,
    const UI32_T session_id,
    const BOOL_T state);


/* FUNCTION NAME: air_mir_getSessionAdminMode
* PURPOSE:
*      This API is used to get mirror session state
* INPUT:
*      unit         --  Device unit number
*      session_id   --  mirror session id
* OUTPUT:
*      state        --  FALSE: disable
*                       TRUE:  enable
* RETURN:
*       AIR_E_OK
*       AIR_E_BAD_PARAMETER
*
* NOTES:
*       None
*/
AIR_ERROR_NO_T
air_mir_getSessionAdminMode(
    const UI32_T unit,
    const UI32_T session_id,
    BOOL_T *state);


/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: air_mir_setMirrorPort
* PURPOSE:
*      This API is used to set mirror port mirroring type
* INPUT:
*      unit        --   Device unit number
*      session_id  --  mirror session id
*      ptr_session --   The session information
* OUTPUT:
*       None
*
* RETURN:
*       AIR_E_OK
*       AIR_E_BAD_PARAMETER
*
* NOTES:
*       None
*/
AIR_ERROR_NO_T
air_mir_setMirrorPort(
    const UI32_T unit,
    const UI32_T session_id,
    const AIR_MIR_SESSION_T *ptr_session);


/* FUNCTION NAME: air_mir_getMirrorPort
* PURPOSE:
*      This API is used to get mirror port mirroring type
* INPUT:
*      unit         --  Device unit number
*      session_id   --  mirror session id
* OUTPUT:
*      ptr_session  --  The information of this session to be obtained
* RETURN:
*       AIR_E_OK
*       AIR_E_BAD_PARAMETER
*
* NOTES:
*       None
*/
AIR_ERROR_NO_T
air_mir_getMirrorPort(
    const UI32_T unit,
    const UI32_T session_id,
    AIR_MIR_SESSION_T *ptr_session);


#endif /* End of AIR_MIRROR_H */

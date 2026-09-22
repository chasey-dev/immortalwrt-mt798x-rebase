/* FILE NAME: air_mirror.c
 * PURPOSE:
 *      Define the port mirror function in ECNT SDK.
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
/* FUNCTION NAME:   air_mir_addSession
 * PURPOSE:
 *      This API is used to add or set a mirror session.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 *      ptr_session          -- Session information
 *                              AIR_MIR_SESSION_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_addSession(
    const UI32_T    unit,
    const UI32_T    session_id,
    const AIR_MIR_SESSION_T   *ptr_session)
{
    UI32_T regMIR = 0, regPCR = 0;
    UI32_T dst_mac_port = 0, src_mac_port = 0;
    BOOL_T enable=FALSE, tx_tag_enable=FALSE;
    
    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((session_id >= AIR_MAX_MIRROR_SESSION), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_session);
    AIR_PARAM_CHK((ptr_session->src_port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((ptr_session->dst_port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((ptr_session->dst_port == ptr_session->src_port), AIR_E_BAD_PARAMETER);

    src_mac_port = ptr_session->src_port;
    dst_mac_port = ptr_session->dst_port;
    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR);

    /* Set mirroring port */
    regMIR &= ~ BITS_RANGE(MIR_MIRROR_PORT_OFFSER(session_id), MIR_MIRROR_PORT_LEN);
    regMIR |= BITS_OFF_L(dst_mac_port, MIR_MIRROR_PORT_OFFSER(session_id), MIR_MIRROR_PORT_LEN);

    /* Set mirroring port tx tag state */
    if(ptr_session->flags & AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG)
    {
        tx_tag_enable = TRUE;
    }
    regMIR &= ~ BITS_RANGE(MIR_MIRROR_TAG_TX_EN_OFFSER(session_id), MIR_MIRROR_TAG_TX_EN_LEN);
    regMIR |= BITS_OFF_L(tx_tag_enable, MIR_MIRROR_TAG_TX_EN_OFFSER(session_id), MIR_MIRROR_TAG_TX_EN_LEN);

    /* Set mirroring port state */
    if(ptr_session->flags & AIR_MIR_SESSION_FLAGS_ENABLE)
    {
        enable = TRUE;
    }
    regMIR &= ~ BITS_RANGE(MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);
    regMIR |= BITS_OFF_L(enable, MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);

    /* Write MIR */
    aml_writeReg(unit, MIR, regMIR);

    /* Read PCR */
    aml_readReg(unit, PCR(src_mac_port), &regPCR);

    /* Set mirroring source port */
    regPCR &= ~ BIT(PCR_PORT_TX_MIR_OFFT + session_id);
    regPCR &= ~ BIT(PCR_PORT_RX_MIR_OFFT + session_id);
    if(ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_TX)
    {
        regPCR |= BIT(PCR_PORT_TX_MIR_OFFT + session_id);
    }

    if(ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_RX)
    {
        regPCR |= BIT(PCR_PORT_RX_MIR_OFFT + session_id);
    }

    /* Write PCR */
    aml_writeReg(unit, PCR(src_mac_port), regPCR);
    return AIR_E_OK;
}

/* FUNCTION NAME:   air_mir_delSession
 * PURPOSE:
 *      This API is used to delete a mirror session.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_delSession(
    const UI32_T    unit,
    const UI32_T    session_id)
{
    UI32_T regMIR = 0;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((session_id >= AIR_MAX_MIRROR_SESSION), AIR_E_BAD_PARAMETER);

    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR);

    /* Set mirroring port */
    regMIR &= ~ BITS_RANGE(MIR_MIRROR_PORT_OFFSER(session_id), MIR_MIRROR_PORT_LEN);
    regMIR |= BITS_OFF_L(AIR_DST_DEFAULT_PORT, MIR_MIRROR_PORT_OFFSER(session_id), MIR_MIRROR_PORT_LEN);
    /* Set mirroring port tx tag state */
    regMIR &= ~ BITS_RANGE(MIR_MIRROR_TAG_TX_EN_OFFSER(session_id), MIR_MIRROR_TAG_TX_EN_LEN);
    /* Set mirroring port state */
    regMIR &= ~ BITS_RANGE(MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);

    /* Write MIR */
    aml_writeReg(unit, MIR, regMIR);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_mir_getSession
 * PURPOSE:
 *      This API is used to get mirror session information.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 * OUTPUT:
 *      ptr_session          -- The information of the session to be
 *                              obtained
 *                              AIR_MIR_SESSION_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_getSession(
    const UI32_T        unit,
    const UI32_T        session_id,
    AIR_MIR_SESSION_T   *ptr_session)
{
    UI32_T regMIR = 0;
    UI32_T dst_mac_port = 0;
    BOOL_T enable = FALSE, tx_tag_enable = FALSE;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((session_id >= AIR_MAX_MIRROR_SESSION), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_session);

    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR);
    /* Get mirroring port */
    dst_mac_port = BITS_OFF_R(regMIR, MIR_MIRROR_PORT_OFFSER(session_id), MIR_MIRROR_PORT_LEN);
    /* Get mirroring port state */
    enable = BITS_OFF_R(regMIR, MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);
    /* Get mirroring tx tag state*/
    tx_tag_enable = BITS_OFF_R(regMIR, MIR_MIRROR_TAG_TX_EN_OFFSER(session_id), MIR_MIRROR_TAG_TX_EN_LEN);
    ptr_session->dst_port = dst_mac_port;
    if(enable)
    {
        ptr_session->flags |= AIR_MIR_SESSION_FLAGS_ENABLE;
    }
    if(tx_tag_enable)
    {
        ptr_session->flags |= AIR_MIR_SESSION_FLAGS_TX_TAG_OBEY_CFG;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_mir_setSessionAdminMode
 * PURPOSE:
 *      This API is used to set mirror session state.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 *      enable               -- State of session
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_setSessionAdminMode(
    const UI32_T    unit,
    const UI32_T    session_id,
    const BOOL_T    enable)
{
    UI32_T regMIR = 0;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((session_id >= AIR_MAX_MIRROR_SESSION), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((enable != TRUE && enable != FALSE), AIR_E_BAD_PARAMETER);

    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR);

    /* Set mirroring port state */
    regMIR &= ~ BITS_RANGE(MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);
    regMIR |= BITS_OFF_L(enable, MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);

    /* Write MIR */
    aml_writeReg(unit, MIR, regMIR);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_mir_getSessionAdminMode
 * PURPOSE:
 *      This API is used to get mirror session state.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- mirror session id
 * OUTPUT:
 *      ptr_enable           -- State of session
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_getSessionAdminMode(
    const UI32_T    unit,
    const UI32_T    session_id,
    BOOL_T          *ptr_enable)
{
    UI32_T regMIR = 0;
    BOOL_T enable = FALSE;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((session_id >= AIR_MAX_MIRROR_SESSION), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_enable);

    /* Read MIR */
    aml_readReg(unit, MIR, &regMIR);

    /* Get mirroring port state */
    enable = BITS_OFF_R(regMIR, MIR_MIRROR_EN_OFFSER(session_id), MIR_MIRROR_EN_LEN);

    *ptr_enable = enable;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_mir_setMirrorPort
 * PURPOSE:
 *      This API is used to set mirror port mirroring type.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 *      ptr_session          -- Session information
 *                              AIR_MIR_SESSION_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_setMirrorPort(
    const UI32_T            unit,
    const UI32_T            session_id,
    const AIR_MIR_SESSION_T *ptr_session)
{
    UI32_T regPCR = 0;
    UI32_T src_mac_port = 0;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((session_id >= AIR_MAX_MIRROR_SESSION), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_session);
    AIR_PARAM_CHK((ptr_session->src_port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    src_mac_port = ptr_session->src_port;
    /* Read data from register */
    aml_readReg(unit, PCR(src_mac_port), &regPCR);

    regPCR &= ~ BIT(PCR_PORT_TX_MIR_OFFT + session_id);
    regPCR &= ~ BIT(PCR_PORT_RX_MIR_OFFT + session_id);

    if(ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_TX)
    {
        regPCR |= BIT(PCR_PORT_TX_MIR_OFFT + session_id);
    }

    if(ptr_session->flags & AIR_MIR_SESSION_FLAGS_DIR_RX)
    {
        regPCR |= BIT(PCR_PORT_RX_MIR_OFFT + session_id);
    }
    /* Write data to register */
    aml_writeReg(unit, PCR(src_mac_port), regPCR);
    return AIR_E_OK;
}

/* FUNCTION NAME:   air_mir_getMirrorPort
 * PURPOSE:
 *      This API is used to get mirror port mirroring type.
 * INPUT:
 *      unit                 -- Device unit number
 *      session_id           -- Session id
 * OUTPUT:
 *      ptr_session          -- The information of this session to be
 *                              obtained.
 *                              AIR_MIR_SESSION_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mir_getMirrorPort(
    const UI32_T        unit,
    const UI32_T        session_id,
    AIR_MIR_SESSION_T   *ptr_session)
{
    UI32_T regPCR = 0;
    UI32_T src_mac_port = 0;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((session_id >= AIR_MAX_MIRROR_SESSION), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_session);
    AIR_PARAM_CHK((ptr_session->src_port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    
    src_mac_port = ptr_session->src_port;
    /* Read data from register */
    aml_readReg(unit, PCR(src_mac_port), &regPCR);

    if(regPCR & BIT(PCR_PORT_TX_MIR_OFFT + session_id))
    {
        ptr_session->flags |= AIR_MIR_SESSION_FLAGS_DIR_TX;
    }

    if(regPCR & BIT(PCR_PORT_RX_MIR_OFFT + session_id))
    {
        ptr_session->flags |= AIR_MIR_SESSION_FLAGS_DIR_RX;
    }

    return AIR_E_OK;
}




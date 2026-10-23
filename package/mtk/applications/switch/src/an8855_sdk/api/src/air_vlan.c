/* FILE NAME:   air_vlan.c
 * PURPOSE:
 *      Define the VLAN function in AIR SDK.
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

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
*/
void
_air_vlan_readEntry(
    const UI32_T unit,
    const UI16_T vid,
    AIR_VLAN_ENTRY_T* vlan_entry)
{
    UI32_T val = 0;
    val = (0x80000000 + vid); //r_vid_cmd
    aml_writeReg(unit, VTCR, val);

    for (;;)
    {
        aml_readReg(unit, VTCR, &val);
        if ((val & 0x80000000) == 0)
            break;
        AIR_UDELAY(10);
    }

    aml_readReg(unit, VLNRDATA0, &(vlan_entry->vlan_table.vlan_table0));
    aml_readReg(unit, VLNRDATA1, &(vlan_entry->vlan_table.vlan_table1));
}

void
_air_vlan_writeEntry(
    const UI32_T unit,
    const UI16_T vid,
    AIR_VLAN_ENTRY_T* vlan_entry)
{
    UI32_T val = 0;

    aml_writeReg(unit, VLNWDATA0, vlan_entry->vlan_table.vlan_table0);
    aml_writeReg(unit, VLNWDATA1, vlan_entry->vlan_table.vlan_table1);
    aml_writeReg(unit, VLNWDATA2, 0);
    aml_writeReg(unit, VLNWDATA3, 0);
    aml_writeReg(unit, VLNWDATA4, 0);

    val = (0x80001000 + vid); //w_vid_cmd
    aml_writeReg(unit, VTCR, val);

    for (;;)
    {
        aml_readReg(unit, VTCR, &val);
        if ((val & 0x80000000) == 0)
            break;
        AIR_UDELAY(10);
    }
}

void
_air_untagged_vlan_readEntry(
    const UI32_T unit,
    const UI16_T vid,
    AIR_VLAN_ENTRY_ATTR_T* vlan_entry)
{
    UI32_T val = 0;
    val = (0x80000000 + vid); //r_vid_cmd
    aml_writeReg(unit, VTCR, val);

    for (;;)
    {
        aml_readReg(unit, VTCR, &val);
        if ((val & 0x80000000) == 0)
            break;
        AIR_UDELAY(10);
    }

    aml_readReg(unit, VLNRDATA0, &(vlan_entry->vlan_table.vlan_table0));
    aml_readReg(unit, VLNRDATA1, &(vlan_entry->vlan_table.vlan_table1));
    aml_readReg(unit, VLNRDATA2, &(vlan_entry->vlan_table.vlan_table2));
    aml_readReg(unit, VLNRDATA3, &(vlan_entry->vlan_table.vlan_table3));
    aml_readReg(unit, VLNRDATA4, &(vlan_entry->vlan_table.vlan_table4));
}

void
_air_untagged_vlan_writeEntry(
    const UI32_T unit,
    const UI16_T vid,
    AIR_VLAN_ENTRY_ATTR_T* vlan_entry)
{
    UI32_T val = 0;

    aml_writeReg(unit, VLNWDATA0, vlan_entry->vlan_table.vlan_table0);
    aml_writeReg(unit, VLNWDATA1, vlan_entry->vlan_table.vlan_table1);
    aml_writeReg(unit, VLNWDATA2, vlan_entry->vlan_table.vlan_table2);
    aml_writeReg(unit, VLNWDATA3, vlan_entry->vlan_table.vlan_table3);
    aml_writeReg(unit, VLNWDATA4, vlan_entry->vlan_table.vlan_table4);

    val = (0x80001000 + vid); //w_vid_cmd
    aml_writeReg(unit, VTCR, val);

    for (;;)
    {
        aml_readReg(unit, VTCR, &val);
        if ((val & 0x80000000) == 0)
            break;
        AIR_UDELAY(10);
    }
}

/* FUNCTION NAME:   air_vlan_create
 * PURPOSE:
 *      Create the vlan in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      p_attr      -- vlan attr
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Vlan creation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_create(
    const UI32_T    unit,
    const UI16_T    vid,
    AIR_VLAN_ENTRY_ATTR_T *p_attr)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};
    AIR_VLAN_ENTRY_ATTR_T vlan_attr_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (vlan_entry.valid)
        return AIR_E_ENTRY_EXISTS;

    if (NULL != p_attr)
    {
        p_attr->valid = 1;
        _air_untagged_vlan_writeEntry(unit, vid, p_attr);
    }
    else
    {
        memset(&vlan_attr_entry, 0, sizeof(vlan_attr_entry));
        vlan_attr_entry.valid = 1;
        _air_untagged_vlan_writeEntry(unit, vid, &vlan_attr_entry);
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_destroy
 * PURPOSE:
 *      Destroy the vlan in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK     -- Successfully read the data.
 *      AIR_E_OTHERS -- Vlan destroy failed.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_destroy(
    const UI32_T    unit,
    const UI16_T    vid)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);

    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_destroyAll
 * PURPOSE:
 *      Destroy the vlan in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK     -- Successfully read the data.
 *      AIR_E_OTHERS -- Vlan destroy failed.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_destroyAll(
    const UI32_T    unit,
    const UI32_T    keep_and_restore_default_vlan)
{
    UI16_T vid = 0;

    for (vid = AIR_VLAN_ID_MIN; vid <= AIR_VLAN_ID_MAX; vid++)
    {
        if (keep_and_restore_default_vlan)
        {
            air_vlan_reset(unit, vid);
        }
        else
        {
            air_vlan_destroy(unit, vid);
        }
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_reset
 * PURPOSE:
 *      Destroy the vlan in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK     -- Successfully reset the data.
 *      AIR_E_OTHERS -- Vlan reset failed.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_reset(
    const UI32_T    unit,
    const UI16_T    vid)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);

    vlan_entry.vlan_entry_format.port_mem = AIR_ALL_PORT_BITMAP;
    vlan_entry.valid = TRUE;

    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setFid
 * PURPOSE:
 *      Set the filter id of the vlan to the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      fid         -- filter id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setFid(
    const UI32_T    unit,
    const UI16_T    vid,
    const UI8_T     fid)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    /* VID check */
    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((fid > AIR_FILTER_ID_MAX), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    vlan_entry.vlan_entry_format.fid = fid;
    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getFid
 * PURPOSE:
 *      Get the filter id of the vlan from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id to be created
 * OUTPUT:
 *      ptr_fid     -- filter id
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getFid(
    const UI32_T    unit,
    const UI16_T    vid,
    UI8_T           *ptr_fid)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_fid);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    *ptr_fid = vlan_entry.vlan_entry_format.fid;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_addMemberPort
 * PURPOSE:
 *      Add one vlan member to the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port        -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_addMemberPort(
    const UI32_T    unit,
    const UI16_T    vid,
    const UI32_T    port)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    vlan_entry.vlan_entry_format.port_mem |= 1 << port;
    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_delMemberPort
 * PURPOSE:
 *      Delete one vlan member from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port        -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_delMemberPort(
    const UI32_T    unit,
    const UI16_T    vid,
    const UI32_T    port)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    vlan_entry.vlan_entry_format.port_mem &= ~(1 << port);
    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setMemberPort
 * PURPOSE:
 *      Replace the vlan members in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port_bitmap -- member port bitmap
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setMemberPort(
    const UI32_T    unit,
    const UI16_T    vid,
    const UI32_T    port_bitmap)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port_bitmap & (~AIR_ALL_PORT_BITMAP)), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    vlan_entry.vlan_entry_format.port_mem = port_bitmap;
    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getMemberPort
 * PURPOSE:
 *      Get the vlan members from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      port_bitmap -- member port bitmap
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getMemberPort(
    const UI32_T    unit,
    const UI16_T    vid,
    UI32_T          *ptr_port_bitmap)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_port_bitmap);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    *ptr_port_bitmap = vlan_entry.vlan_entry_format.port_mem;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setIVL
 * PURPOSE:
 *      Set L2 lookup mode IVL/SVL for L2 traffic.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable IVL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setIVL(
    const UI32_T    unit,
    const UI16_T    vid,
    const BOOL_T    enable)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    vlan_entry.vlan_entry_format.ivl = enable ? 1 : 0;
    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getIVL
 * PURPOSE:
 *      Get L2 lookup mode IVL/SVL for L2 traffic.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable IVL
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getIVL(
    const UI32_T    unit,
    const UI16_T    vid,
    BOOL_T          *ptr_enable)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_enable);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    *ptr_enable = vlan_entry.vlan_entry_format.ivl;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setPortAcceptFrameType
 * PURPOSE:
 *      Set vlan accept frame type of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      type        -- accept frame type
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setPortAcceptFrameType(
    const UI32_T    unit,
    const UI32_T    port,
    const AIR_VLAN_ACCEPT_FRAME_TYPE_T type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((type >= AIR_VLAN_ACCEPT_FRAME_TYPE_LAST), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, PVC(port), &val);
    val &= ~PVC_ACC_FRM_MASK;
    val |= (type & PVC_ACC_FRM_RELMASK) << PVC_ACC_FRM_OFFT;
    aml_writeReg(unit, PVC(port), val);

    return rc;
}

/* FUNCTION NAME:   air_vlan_getPortAcceptFrameType
 * PURPOSE:
 *      Get vlan accept frame type of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_type    -- accept frame type
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getPortAcceptFrameType(
    const UI32_T    unit,
    const UI32_T    port,
    AIR_VLAN_ACCEPT_FRAME_TYPE_T *ptr_type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_type);

    aml_readReg(unit, PVC(port), &val);
    *ptr_type = (val >> PVC_ACC_FRM_OFFT) & PVC_ACC_FRM_RELMASK;

    return rc;
}

/* FUNCTION NAME:   air_vlan_setPortLeakyVlanEnable
 * PURPOSE:
 *      Set leaky vlan enable of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      pkt_type    -- packet type
 *      enable      -- enable leaky
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setPortLeakyVlanEnable(
    const UI32_T    unit,
    const UI32_T    port,
    AIR_LEAKY_PKT_TYPE_T   pkt_type,
    const BOOL_T    enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((pkt_type >= AIR_LEAKY_PKT_TYPE_LAST), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, PVC(port), &val);

    if (pkt_type == AIR_LEAKY_PKT_TYPE_UNICAST)
    {
        if (enable)
        {
            val |= PVC_UC_LKYV_EN_MASK;
        }
        else
        {
            val &= ~PVC_UC_LKYV_EN_MASK;
        }
    }
    else if (pkt_type == AIR_LEAKY_PKT_TYPE_MULTICAST)
    {
        if (enable)
        {
            val |= PVC_MC_LKYV_EN_MASK;
        }
        else
        {
            val &= ~PVC_MC_LKYV_EN_MASK;
        }
    }
    else
    {
        if (enable)
        {
            val |= PVC_BC_LKYV_EN_MASK;
        }
        else
        {
            val &= ~PVC_BC_LKYV_EN_MASK;
        }
    }

    aml_writeReg(unit, PVC(port), val);

    return rc;
}

/* FUNCTION NAME:   air_vlan_getPortLeakyVlanEnable
 * PURPOSE:
 *      Get leaky vlan enable of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      pkt_type    -- packet type
 * OUTPUT:
 *      ptr_enable  -- enable leaky
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getPortLeakyVlanEnable(
    const UI32_T    unit,
    const UI32_T    port,
    AIR_LEAKY_PKT_TYPE_T   pkt_type,
    BOOL_T          *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((pkt_type >= AIR_LEAKY_PKT_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_enable);

    aml_readReg(unit, PVC(port), &val);

    if (pkt_type == AIR_LEAKY_PKT_TYPE_UNICAST)
    {
        *ptr_enable = val & PVC_UC_LKYV_EN_MASK ? TRUE : FALSE;
    }
    else if (pkt_type == AIR_LEAKY_PKT_TYPE_MULTICAST)
    {
        *ptr_enable = val & PVC_MC_LKYV_EN_MASK ? TRUE : FALSE;
    }
    else
    {
        *ptr_enable = val & PVC_BC_LKYV_EN_MASK ? TRUE : FALSE;
    }

    return rc;
}

/* FUNCTION NAME:   air_vlan_setPortAttr
 * PURPOSE:
 *      Set vlan port attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      attr        -- vlan port attr
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setPortAttr(
    const UI32_T    unit,
    const UI32_T    port,
    const AIR_VLAN_PORT_ATTR_T attr)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((attr >= AIR_VLAN_PORT_ATTR_LAST), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, PVC(port), &val);
    val &= ~PVC_VLAN_ATTR_MASK;
    val |= (attr & PVC_VLAN_ATTR_RELMASK) << PVC_VLAN_ATTR_OFFT;
    aml_writeReg(unit, PVC(port), val);

    return rc;
}

/* FUNCTION NAME:   air_vlan_getPortAttr
 * PURPOSE:
 *      Get vlan port attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_attr    -- vlan port attr
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getPortAttr(
    const UI32_T    unit,
    const UI32_T    port,
    AIR_VLAN_PORT_ATTR_T *ptr_attr)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_attr);

    aml_readReg(unit, PVC(port), &val);
    *ptr_attr = (val >> PVC_VLAN_ATTR_OFFT) & PVC_VLAN_ATTR_RELMASK;

    return rc;
}

/* FUNCTION NAME:   air_vlan_setIgrPortTagAttr
 * PURPOSE:
 *      Set vlan incoming port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      attr        -- egress tag attr
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setIgrPortTagAttr(
    const UI32_T    unit,
    const UI32_T    port,
    const AIR_IGR_PORT_EG_TAG_ATTR_T attr)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((attr >= AIR_IGR_PORT_EG_TAG_ATTR_LAST), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, PVC(port), &val);
    val &= ~PVC_EG_TAG_MASK;
    val |= (attr & PVC_EG_TAG_RELMASK) << PVC_EG_TAG_OFFT;
    aml_writeReg(unit, PVC(port), val);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getIgrPortTagAttr
 * PURPOSE:
 *      Get vlan incoming port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_attr    -- egress tag attr
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getIgrPortTagAttr(
    const UI32_T    unit,
    const UI32_T    port,
    AIR_IGR_PORT_EG_TAG_ATTR_T *ptr_attr)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_attr);

    aml_readReg(unit, PVC(port), &val);
    *ptr_attr = (val >> PVC_EG_TAG_OFFT) & PVC_EG_TAG_RELMASK;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setPortEgsTagAttr
 * PURPOSE:
 *      Set vlan port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      attr        -- egress tag attr
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setPortEgsTagAttr(
    const UI32_T    unit,
    const UI32_T    port,
    const AIR_PORT_EGS_TAG_ATTR_T attr)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((attr >= AIR_PORT_EGS_TAG_ATTR_LAST), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, PCR(port), &val);
    val &= ~PCR_EG_TAG_MASK;
    val |= (attr & PCR_EG_TAG_RELMASK) << PCR_EG_TAG_OFFT;
    aml_writeReg(unit, PCR(port), val);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getPortEgsTagAttr
 * PURPOSE:
 *      Get vlan port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_attr    -- egress tag attr
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getPortEgsTagAttr(
    const UI32_T    unit,
    const UI32_T    port,
    AIR_PORT_EGS_TAG_ATTR_T *ptr_attr)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_attr);

    aml_readReg(unit, PCR(port), &val);
    *ptr_attr = (val >> PCR_EG_TAG_OFFT) & PCR_EG_TAG_RELMASK;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setPortOuterTPID
 * PURPOSE:
 *      Set stack tag TPID of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      tpid        -- TPID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setPortOuterTPID(
    const UI32_T    unit,
    const UI32_T    port,
    const UI16_T    tpid)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, PVC(port), &val);
    val &= ~PVC_STAG_VPID_MASK;
    val |= (tpid & PVC_STAG_VPID_RELMASK) << PVC_STAG_VPID_OFFT;
    aml_writeReg(unit, PVC(port), val);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getPortOuterTPID
 * PURPOSE:
 *      Get stack tag TPID of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_tpid    -- TPID
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getPortOuterTPID(
    const UI32_T    unit,
    const UI32_T    port,
    UI16_T          *ptr_tpid)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_tpid);

    aml_readReg(unit, PVC(port), &val);
    *ptr_tpid = (val >> PVC_STAG_VPID_OFFT) & PVC_STAG_VPID_RELMASK;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setPortPVID
 * PURPOSE:
 *      Set PVID of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      pvid        -- native vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setPortPVID(
    const UI32_T    unit,
    const UI32_T    port,
    const UI16_T    pvid)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((pvid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, PVID(port), &val);
    val &= ~PVID_PCVID_MASK;
    val |= (pvid & PVID_PCVID_RELMASK) << PVID_PCVID_OFFT;
    aml_writeReg(unit, PVID(port), val);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getPortPVID
 * PURPOSE:
 *      Get PVID of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_pvid    -- native vlan id
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getPortPVID(
    const UI32_T    unit,
    const UI32_T    port,
    UI16_T          *ptr_pvid)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_pvid);

    aml_readReg(unit, PVID(port), &val);
    *ptr_pvid = (val >> PVID_PCVID_OFFT) & PVID_PCVID_RELMASK;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setServiceTag
 * PURPOSE:
 *      Set Vlan service tag.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      stag        -- service stag
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setServiceTag(
    const UI32_T    unit,
    const UI16_T    vid,
    const UI16_T    stag)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((stag > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    vlan_entry.vlan_entry_format.stag = stag;
    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getServiceTag
 * PURPOSE:
 *      Get Vlan service tag.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_stag    -- service stag
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getServiceTag(
    const UI32_T    unit,
    const UI16_T    vid,
    UI16_T          *ptr_stag)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_stag);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    *ptr_stag = vlan_entry.vlan_entry_format.stag;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setEgsTagCtlEnable
 * PURPOSE:
 *      Set per vlan egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable vlan egress tag control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setEgsTagCtlEnable(
    const UI32_T    unit,
    const UI16_T    vid,
    const BOOL_T    enable)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    vlan_entry.vlan_entry_format.eg_ctrl_en = enable ? 1 : 0;
    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getEgsTagCtlEnable
 * PURPOSE:
 *      Get per vlan egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable vlan egress tag control
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getEgsTagCtlEnable(
    const UI32_T    unit,
    const UI16_T    vid,
    BOOL_T          *ptr_enable)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_enable);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    *ptr_enable = vlan_entry.vlan_entry_format.eg_ctrl_en;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setEgsTagConsistent
 * PURPOSE:
 *      Set per vlan egress tag consistent.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable vlan egress tag consistent
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setEgsTagConsistent(
    const UI32_T    unit,
    const UI16_T    vid,
    const BOOL_T    enable)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    vlan_entry.vlan_entry_format.eg_con = enable;
    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getEgsTagConsistent
 * PURPOSE:
 *      Get per vlan egress tag consistent.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable vlan egress tag consistent
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getEgsTagConsistent(
    const UI32_T    unit,
    const UI16_T    vid,
    BOOL_T          *ptr_enable)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_enable);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    *ptr_enable = vlan_entry.vlan_entry_format.eg_con;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setPortBasedStag
 * PURPOSE:
 *      Set vlan port based stag enable.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- vlan port based stag enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setPortBasedStag(
    const UI32_T    unit,
    const UI16_T    vid,
    const BOOL_T    enable)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    vlan_entry.vlan_entry_format.port_stag = enable;
    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getPortBasedStag
 * PURPOSE:
 *      Get vlan port based stag enable.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- vlan port based stag enable
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getPortBasedStag(
    const UI32_T    unit,
    const UI16_T    vid,
    BOOL_T          *ptr_enable)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_enable);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    *ptr_enable = vlan_entry.vlan_entry_format.port_stag;

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_setPortEgsTagCtl
 * PURPOSE:
 *      Set vlan port egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port        -- port id
 *      tag_ctl     -- egress tag control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_setPortEgsTagCtl(
    const UI32_T    unit,
    const UI16_T    vid,
    const UI32_T    port,
    const AIR_VLAN_PORT_EGS_TAG_CTL_TYPE_T    tag_ctl)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((tag_ctl >= AIR_PORT_EGS_TAG_ATTR_LAST), AIR_E_BAD_PARAMETER);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    vlan_entry.vlan_entry_format.eg_ctrl &= ~(0x3 << (port * 2));
    vlan_entry.vlan_entry_format.eg_ctrl |= (tag_ctl & 0x3) << (port * 2);
    _air_vlan_writeEntry(unit, vid, &vlan_entry);

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_vlan_getPortEgsTagCtl
 * PURPOSE:
 *      Get vlan port egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_tag_ctl -- egress tag control
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getPortEgsTagCtl(
    const UI32_T    unit,
    const UI16_T    vid,
    const UI32_T    port,
    AIR_VLAN_PORT_EGS_TAG_CTL_TYPE_T   *ptr_tag_ctl)
{
    AIR_VLAN_ENTRY_T vlan_entry = {0};

    AIR_PARAM_CHK((vid > AIR_VLAN_ID_MAX), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_tag_ctl);

    _air_vlan_readEntry(unit, vid, &vlan_entry);
    if (!vlan_entry.valid)
        return AIR_E_ENTRY_NOT_FOUND;

    *ptr_tag_ctl = (vlan_entry.vlan_entry_format.eg_ctrl >> (port * 2)) & 0x3;

    return AIR_E_OK;
}

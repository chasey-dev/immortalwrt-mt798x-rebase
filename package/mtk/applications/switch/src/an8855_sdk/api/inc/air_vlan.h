/* FILE NAME:   air_vlan.h
 * PURPOSE:
 *      Define the vlan functions in AIR SDK.
 * NOTES:
 */

#ifndef AIR_VLAN_H
#define AIR_VLAN_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_VLAN_ID_MIN                             0
#define AIR_VLAN_ID_MAX                             4095
#define AIR_DEFAULT_VLAN_ID                         1

#define AIR_FILTER_ID_MAX                           7

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    AIR_VLAN_PORT_EGS_TAG_CTL_TYPE_UNTAGGED = 0,
    AIR_VLAN_PORT_EGS_TAG_CTL_TYPE_TAGGED = 2,
    AIR_VLAN_PORT_EGS_TAG_CTL_TYPE_LAST,
} AIR_VLAN_PORT_EGS_TAG_CTL_TYPE_T;

typedef enum
{
    AIR_PORT_EGS_TAG_ATTR_UNTAGGED = 0,
    AIR_PORT_EGS_TAG_ATTR_SWAP,
    AIR_PORT_EGS_TAG_ATTR_TAGGED,
    AIR_PORT_EGS_TAG_ATTR_STACK,
    AIR_PORT_EGS_TAG_ATTR_LAST
} AIR_PORT_EGS_TAG_ATTR_T;

typedef enum
{
    AIR_VLAN_ACCEPT_FRAME_TYPE_ALL = 0,            /* untagged, priority-tagged and tagged  */
    AIR_VLAN_ACCEPT_FRAME_TYPE_TAG_ONLY,           /* tagged                                */
    AIR_VLAN_ACCEPT_FRAME_TYPE_UNTAG_ONLY,         /* untagged and priority-tagged          */
    AIR_VLAN_ACCEPT_FRAME_TYPE_RESERVED,           /* reserved                              */
    AIR_VLAN_ACCEPT_FRAME_TYPE_LAST
} AIR_VLAN_ACCEPT_FRAME_TYPE_T;

typedef enum
{
    AIR_LEAKY_PKT_TYPE_UNICAST = 0,                /* unicast pkt      */
    AIR_LEAKY_PKT_TYPE_MULTICAST,                  /* multicast pkt    */
    AIR_LEAKY_PKT_TYPE_BROADCAST,                  /* broadcast pkt    */
    AIR_LEAKY_PKT_TYPE_LAST
} AIR_LEAKY_PKT_TYPE_T;

typedef enum
{
    AIR_VLAN_PORT_ATTR_USER_PORT = 0,              /* user port        */
    AIR_VLAN_PORT_ATTR_STACK_PORT,                 /* stack port       */
    AIR_VLAN_PORT_ATTR_TRANSLATION_PORT,           /* translation port */
    AIR_VLAN_PORT_ATTR_TRANSPARENT_PORT,           /* transparent port */
    AIR_VLAN_PORT_ATTR_LAST
} AIR_VLAN_PORT_ATTR_T;

typedef enum
{
    AIR_IGR_PORT_EG_TAG_ATTR_DISABLE = 0,
    AIR_IGR_PORT_EG_TAG_ATTR_CONSISTENT,
    AIR_IGR_PORT_EG_TAG_ATTR_UNTAGGED = 4,
    AIR_IGR_PORT_EG_TAG_ATTR_SWAP,
    AIR_IGR_PORT_EG_TAG_ATTR_TAGGED,
    AIR_IGR_PORT_EG_TAG_ATTR_STACK,
    AIR_IGR_PORT_EG_TAG_ATTR_LAST
} AIR_IGR_PORT_EG_TAG_ATTR_T;

typedef union AIR_VLAN_ENTRY_S
{
    UI8_T valid : 1;
    struct
    {
        UI32_T  vlan_table0;
        UI32_T  vlan_table1;
    } vlan_table;
    struct
    {
        UI64_T   valid             : 1;
        UI64_T   fid               : 4;
        UI64_T   ivl               : 1;
        UI64_T   copy_pri          : 1;
        UI64_T   user_pri          : 3;
        UI64_T   eg_ctrl_en        : 1;
        UI64_T   eg_con            : 1;
        UI64_T   eg_ctrl           : 14;
        UI64_T   port_mem          : 7;
        UI64_T   port_stag         : 1;
        UI64_T   stag              : 12;
        UI64_T   unm_vlan_drop     : 1;
    } vlan_entry_format;
} AIR_VLAN_ENTRY_T;

typedef union AIR_VLAN_ENTRY_ATTR_S
{
    UI8_T valid : 1;
    struct
    {
        UI32_T  vlan_table0;
        UI32_T  vlan_table1;
        UI32_T  vlan_table2;
        UI32_T  vlan_table3;
        UI32_T  vlan_table4;
    } vlan_table;
    struct
    {
        UI64_T   valid             : 1;
        UI64_T   fid               : 4;
        UI64_T   ivl               : 1;
        UI64_T   copy_pri          : 1;
        UI64_T   user_pri          : 3;
        UI64_T   eg_ctrl_en        : 1;
        UI64_T   eg_con            : 1;
        UI64_T   eg_ctrl           : 14;
        UI64_T   port_mem          : 7;
        UI64_T   port_stag         : 1;
        UI64_T   stag              : 12;
        UI64_T   unm_vlan_drop     : 1;
    } vlan_entry_format;
#if 0
    struct
    {
        UI64_T   valid             : 1;
        UI64_T   type              : 3;
        UI64_T   mac_addr          : 48;
        UI64_T   mac_mask_len      : 6;
        UI64_T   priority          : 3;
        UI64_T   :0;
        UI64_T   vid               : 12;
    } mac_based_vlan_entry_format;
    struct
    {
        UI64_T   valid             : 1;
        UI64_T   type              : 3;
        UI64_T   check_field       : 36;
        UI64_T   :0;
        UI64_T   check_field_mask  : 36;
        UI64_T   untagged_packet   : 1;
        UI64_T   vlan_priority     : 3;
        UI64_T   svid              : 12;
    } qinq_based_vlan_entry_format;
    struct
    {
        UI64_T   valid             : 1;
        UI64_T   type              : 3;
        UI64_T   ipv4              : 32;
        UI64_T   :0;
        UI64_T   subnetmask        : 32;
        UI64_T   priority          : 3;
        UI64_T   cvid              : 12;
    } ipv4_based_vlan_entry_format;
    struct
    {
        UI64_T   valid             : 1;
        UI64_T   :0;
        UI64_T   ipv6_high         : 64;
        UI64_T   ipv6_low          : 64;
        UI64_T   subnetmask        : 8;
        UI64_T   priority          : 3;
        UI64_T   cvid              : 12;
    } ipv6_based_vlan_entry_format;
#endif
} AIR_VLAN_ENTRY_ATTR_T;

void
_air_vlan_readEntry(
    const UI32_T unit,
    const UI16_T vid,
    AIR_VLAN_ENTRY_T* vlan_entry);

void
_air_vlan_writeEntry(
    const UI32_T unit,
    const UI16_T vid,
    AIR_VLAN_ENTRY_T* vlan_entry);

void
_air_untagged_vlan_readEntry(
    const UI32_T unit,
    const UI16_T vid,
    AIR_VLAN_ENTRY_ATTR_T* vlan_entry);

void
_air_untagged_vlan_writeEntry(
    const UI32_T unit,
    const UI16_T vid,
    AIR_VLAN_ENTRY_ATTR_T* vlan_entry);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
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
    AIR_VLAN_ENTRY_ATTR_T *p_attr);

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
    const UI16_T    vid);

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
    const UI32_T    keep_and_restore_default_vlan);

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
    const UI16_T    vid);

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
    const UI8_T     fid);

/* FUNCTION NAME:   air_vlan_getFid
 * PURPOSE:
 *      Get the filter id of the vlan from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id to be created
 * OUTPUT:
 *      p_fid       -- filter id
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
    UI8_T           *p_fid);

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
    const UI32_T    port);

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
    const UI32_T    port);

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
    const UI32_T    port_bitmap);

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
    UI32_T          *p_port_bitmap);

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
    const BOOL_T    enable);

/* FUNCTION NAME:   air_vlan_getIVL
 * PURPOSE:
 *      Get L2 lookup mode IVL/SVL for L2 traffic.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      p_enable    -- enable IVL
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
    BOOL_T          *p_enable);

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
    const AIR_VLAN_ACCEPT_FRAME_TYPE_T type);

/* FUNCTION NAME:   air_vlan_getPortAcceptFrameType
 * PURPOSE:
 *      Get vlan accept frame type of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      p_type      -- accept frame type
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
    AIR_VLAN_ACCEPT_FRAME_TYPE_T *p_type);

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
    const BOOL_T    enable);

/* FUNCTION NAME:   air_vlan_getPortLeakyVlanEnable
 * PURPOSE:
 *      Get leaky vlan enable of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      pkt_type    -- packet type
 * OUTPUT:
 *      p_enable    -- enable leaky
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
    BOOL_T          *p_enable);

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
    const AIR_VLAN_PORT_ATTR_T attr);

/* FUNCTION NAME:   air_vlan_getPortAttr
 * PURPOSE:
 *      Get vlan port attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      p_attr      -- vlan port attr
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
    AIR_VLAN_PORT_ATTR_T *p_attr);

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
    const AIR_IGR_PORT_EG_TAG_ATTR_T attr);

/* FUNCTION NAME:   air_vlan_getIgrPortTagAttr
 * PURPOSE:
 *      Get vlan incoming port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      p_attr      -- egress tag attr
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
    AIR_IGR_PORT_EG_TAG_ATTR_T *p_attr);

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
    const AIR_PORT_EGS_TAG_ATTR_T attr);

/* FUNCTION NAME:   air_vlan_getPortEgsTagAttr
 * PURPOSE:
 *      Get vlan port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      p_attr      -- egress tag attr
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
    AIR_PORT_EGS_TAG_ATTR_T *p_attr);

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
    const UI16_T    tpid);

/* FUNCTION NAME:   air_vlan_getPortOuterTPID
 * PURPOSE:
 *      Get stack tag TPID of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      p_tpid        -- TPID
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
    UI16_T          *p_tpid);

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
    const UI16_T    pvid);

/* FUNCTION NAME:   air_vlan_getPortPVID
 * PURPOSE:
 *      Get PVID of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      p_pvid      -- native vlan id
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
    UI16_T          *p_pvid);

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
    const UI16_T    stag);

/* FUNCTION NAME:   air_vlan_getServiceTag
 * PURPOSE:
 *      Get Vlan service tag.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      p_stag      -- service stag
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
    UI16_T          *p_stag);

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
    const BOOL_T    enable);

/* FUNCTION NAME:   air_vlan_getEgsTagCtlEnable
 * PURPOSE:
 *      Get per vlan egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      p_enable    -- enable vlan egress tag control
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
    BOOL_T          *p_enable);

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
    const BOOL_T    enable);

/* FUNCTION NAME:   air_vlan_getEgsTagConsistent
 * PURPOSE:
 *      Get per vlan egress tag consistent.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      p_enable    -- enable vlan egress tag consistent
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
    BOOL_T          *p_enable);

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
    const BOOL_T    enable);

/* FUNCTION NAME:   air_vlan_getPortBasedStag
 * PURPOSE:
 *      Get vlan port based stag enable.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      p_enable    -- vlan port based stag enable
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
    BOOL_T          *p_enable);

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
    const AIR_VLAN_PORT_EGS_TAG_CTL_TYPE_T    tag_ctl);

/* FUNCTION NAME:   air_vlan_getPortEgsTagCtl
 * PURPOSE:
 *      Get vlan port egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      p_tag_ctl   -- egress tag control
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
    AIR_VLAN_PORT_EGS_TAG_CTL_TYPE_T   *ptr_tag_ctl);

#endif  /* AIR_VLAN_H */


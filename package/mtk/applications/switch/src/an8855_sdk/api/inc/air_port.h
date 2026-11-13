/* FILE NAME:   air_port.h
 * PURPOSE:
 *      Define port function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_PORT_H
#define AIR_PORT_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_MAX_NUM_OF_UNIT            (1)
#define AIR_DST_DEFAULT_PORT           (31)
#define AIR_PORT_TX                    (0x00)
#define AIR_PORT_RX                    (0x01)
#define AIR_MAX_NUM_OF_PORTS           (7)
#define AIR_MAX_NUM_OF_GIGA_PORTS      (5)
#define AIR_SGMII_PORT_OFFSET_BEGIN    (5)
#define AIR_SGMII_PORT_OFFSET_END      (6)
#define AIR_ALL_PORT_BITMAP            (0x7F)

/* Definition of Power Saving mode */
#define AIR_PORT_PS_LINKSTATUS         (0x1 << 0)
#define AIR_PORT_PS_EEE                (0x1 << 1)
#define AIR_PORT_PS_MASK               (0x3)

/* MACRO FUNCTION DECLARATIONS
 */

#define AIR_PORT_ADD(bitmap, port) (((bitmap)[(port)/32]) |=  (1U << ((port)%32)))
#define AIR_PORT_DEL(bitmap, port) (((bitmap)[(port)/32]) &= ~(1U << ((port)%32)))
#define AIR_PORT_CHK(bitmap, port) ((((bitmap)[(port)/32] &   (1U << ((port)%32)))) != 0)

/* DATA TYPE DECLARATIONS
 */
/* AIR_PORT_BITMAP_T is the data type for physical port bitmap. */
#define AIR_BITMAP_SIZE(bit_num)                    ((((bit_num) - 1) / AIR_MAX_NUM_OF_PORTS) + 1)
#define AIR_PORT_BITMAP_SIZE           AIR_BITMAP_SIZE(AIR_MAX_NUM_OF_PORTS)
typedef UI32_T   AIR_PORT_BITMAP_T[AIR_PORT_BITMAP_SIZE];

#define AIR_INVALID_ID      (0xFFFFFFFF)
#define AIR_PORT_INVALID    (AIR_INVALID_ID)

/* Definition of SGMII mode */
typedef enum
{
    AIR_PORT_SGMII_MODE_AN,
    AIR_PORT_SGMII_MODE_FORCE,
    AIR_PORT_SGMII_MODE_LAST
}AIR_PORT_SGMII_MODE_T;

/* Definition of port speed */
typedef enum
{
    AIR_PORT_SPEED_10M,
    AIR_PORT_SPEED_100M,
    AIR_PORT_SPEED_1000M,
    AIR_PORT_SPEED_2500M,
    AIR_PORT_SPEED_LAST
}AIR_PORT_SPEED_T;

typedef enum
{
    AIR_PORT_DUPLEX_HALF,
    AIR_PORT_DUPLEX_FULL,
    AIR_PORT_DUPLEX_LAST
}AIR_PORT_DUPLEX_T;

typedef enum
{
    AIR_PORT_LINK_DOWN,
    AIR_PORT_LINK_UP,
    AIR_PORT_LINK_LAST
}AIR_PORT_LINK_T;

/* Definition of Smart speed down will occur after AN failed how many times */
typedef enum
{
    AIR_PORT_SSD_2T,
    AIR_PORT_SSD_3T,
    AIR_PORT_SSD_4T,
    AIR_PORT_SSD_5T,
    AIR_PORT_SSD_LAST
}AIR_PORT_SSD_T;

typedef enum
{
    AIR_PORT_VLAN_MODE_PORT_MATRIX = 0,    /* Port matrix mode  */
    AIR_PORT_VLAN_MODE_FALLBACK,           /* Fallback mode  */
    AIR_PORT_VLAN_MODE_CHECK,              /* Check mode  */
    AIR_PORT_VLAN_MODE_SECURITY,           /* Security mode  */
    AIR_PORT_VLAN_MODE_LAST
} AIR_PORT_VLAN_MODE_T;

/* Definition of AN Advertisement Register */
typedef struct AIR_AN_ADV_S
{
    BOOL_T advCap10HDX;         /* Advertises 10 BASE-T HDX */
    BOOL_T advCap10FDX;         /* Advertises 10 BASE-T FDX */
    BOOL_T advCap100HDX;        /* Advertises 100 BASE-T HDX */
    BOOL_T advCap100FDX;        /* Advertises 100 BASE-T FDX */
    BOOL_T advCap1000FDX;       /* Advertises 1000 BASE-T FDX */
    BOOL_T advPause;            /* Advertieses Asynchronous Pause */
}AIR_AN_ADV_T;

/* Definition of Link Status of a specific port */
typedef struct AIR_PORT_STATUS_S
{
    BOOL_T link;
    BOOL_T duplex;
    UI32_T speed;
}AIR_PORT_STATUS_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: air_port_setPortMatrix
 * PURPOSE:
 *      Set port matrix from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      port_bitmap     --  Matrix port bitmap
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_port_setPortMatrix(
    const UI32_T    unit,
    const UI32_T    port,
    const UI32_T    port_bitmap);

/* FUNCTION NAME: air_port_getPortMatrix
 * PURPOSE:
 *      Get port matrix from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *
 * OUTPUT:
 *      p_port_bitmap   --  Matrix port bitmap
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_port_getPortMatrix(
    const UI32_T    unit,
    const UI32_T    port,
    UI32_T          *p_port_bitmap);

/* FUNCTION NAME: air_port_setVlanMode
 * PURPOSE:
 *      Set port-based vlan mechanism from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      mode            --  Port vlan mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_port_setVlanMode(
    const UI32_T    unit,
    const UI32_T    port,
    const AIR_PORT_VLAN_MODE_T mode);

/* FUNCTION NAME: air_port_getVlanMode
 * PURPOSE:
 *      Get port-based vlan mechanism from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *
 * OUTPUT:
 *      p_mode          --  Port vlan mode
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_port_getVlanMode(
    const UI32_T    unit,
    const UI32_T    port,
    AIR_PORT_VLAN_MODE_T *p_mode);

/* FUNCTION NAME: air_port_setAnMode
 * PURPOSE:
 *      Set the auto-negotiation mode for a specific port.(Auto or Forced)
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setAnMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state);

/* FUNCTION NAME: air_port_getAnMode
 * PURPOSE:
 *      Get the auto-negotiation mode for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getAnMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_state);

/* FUNCTION NAME: air_port_setLocalAdvAbility
 * PURPOSE:
 *      Set the auto-negotiation advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      adv             --  AN advertisement setting
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setLocalAdvAbility(
    const UI32_T unit,
    const UI32_T port,
    const AIR_AN_ADV_T adv);

/* FUNCTION NAME: air_port_getLocalAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_adv         --  AN advertisement setting
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getLocalAdvAbility(
    const UI32_T unit,
    const UI32_T port,
    AIR_AN_ADV_T *ptr_adv);

/* FUNCTION NAME: air_port_getRemoteAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation remote advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_lp_adv      --  AN advertisement of link partner
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getRemoteAdvAbility(
    const UI32_T unit,
    const UI32_T port,
    AIR_AN_ADV_T *ptr_lp_adv);

/* FUNCTION NAME: air_port_setSpeed
 * PURPOSE:
 *      Set the speed for a specific port.
 *      This setting is used on force mode only.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      speed           --  AIR_PORT_SPEED_10M:  10Mbps
 *                          AIR_PORT_SPEED_100M: 100Mbps
 *                          AIR_PORT_SPEED_1000M:1Gbps
 *                          AIR_PORT_SPEED_2500M:2.5Gbps (Port5, Port6)
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setSpeed(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T speed);

/* FUNCTION NAME: air_port_getSpeed
 * PURPOSE:
 *      Get the speed for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_speed       --  AIR_PORT_SPEED_10M:  10Mbps
 *                          AIR_PORT_SPEED_100M: 100Mbps
 *                          AIR_PORT_SPEED_1000M:1Gbps
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getSpeed(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_speed);

/* FUNCTION NAME: air_port_setDuplex
 * PURPOSE:
 *      Get the duplex for a specific port.
 *      This setting is used on force mode only.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      duplex          --  AIR_PORT_DUPLEX_HALF
 *                          AIR_PORT_DUPLEX_FULL
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setDuplex(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T duplex);

/* FUNCTION NAME: air_port_getDuplex
 * PURPOSE:
 *      Get the duplex for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_duplex      --  AIR_PORT_DUPLEX_HALF
 *                          AIR_PORT_DUPLEX_FULL
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getDuplex(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_duplex);

/* FUNCTION NAME: air_port_getLink
 * PURPOSE:
 *      Get the physical link status for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_ps          --  AIR_PORT_STATUS_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getLink(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_STATUS_T *ptr_ps);

/* FUNCTION NAME: air_port_setBckPres
 * PURPOSE:
 *      Set the back pressure configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      bckPres         --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setBckPres(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T bckPres);

/* FUNCTION NAME: air_port_getBckPres
 * PURPOSE:
 *      Get the back pressure configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_bckPres     --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getBckPres(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_bckPres);

/* FUNCTION NAME: air_port_setFlowCtrl
 * PURPOSE:
 *      Set the flow control configuration for specific port.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number (0 - 6)
 *      dir             --  Directions of AIR_PORT_TX or AIR_PORT_RX
 *      fc_en           --  TRUE: Enable select port flow control
 *                          FALSE:Disable select port flow control
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setFlowCtrl(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T dir,
    const BOOL_T fc_en);

/* FUNCTION NAME: air_port_getFlowCtrl
 * PURPOSE:
 *      Get the flow control configuration for specific port.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number (0..6)
 *      dir             --  AIR_PORT_TX
 *                          AIR_PORT_RX
 * OUTPUT:
 *      ptr_fc_en       --  FALSE: Port flow control disable
 *                          TRUE: Port flow control enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getFlowCtrl(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T dir,
    BOOL_T *ptr_fc_en);

/* FUNCTION NAME: air_port_setJumbo
 * PURPOSE:
 *      Set accepting jumbo frmes with specificied size.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      pkt_len         --  Select max packet length
 *                          RX_PKT_LEN_1518
 *                          RX_PKT_LEN_1536
 *                          RX_PKT_LEN_1552
 *                          RX_PKT_LEN_MAX_JUMBO
 *      frame_len       --  Select max lenght of jumbo frames
 *                          Range : 2 - 15
 *                          Units : K Bytes
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setJumbo(
    const UI32_T unit,
    const UI32_T pkt_len,
    const UI32_T frame_len);

/* FUNCTION NAME: air_port_getJumbo
 * PURPOSE:
 *      Get accepting jumbo frmes with specificied size.
 *
 * INPUT:
 *      unit            --  Select device ID
 *
 * OUTPUT:
 *      ptr_pkt_len     --  Select max packet length
 *                          RX_PKT_LEN_1518
 *                          RX_PKT_LEN_1536
 *                          RX_PKT_LEN_1552
 *                          RX_PKT_LEN_MAX_JUMBO
 *      ptr_frame_len   --  Select max lenght of jumbo frames
 *                          Range : 2 - 15
 *                          Units : K Bytes
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getJumbo(
    const UI32_T unit,
    UI32_T *ptr_pkt_len,
    UI32_T *ptr_frame_len);


/* FUNCTION NAME: air_port_setPsMode
 * PURPOSE:
 *      Set the power saving mode for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      mode            --  Bit-map:
 *                          AIR_PORT_PS_LINKSTATUS
 *                          AIR_PORT_PS_EEE
 *                          FALSE: Disable / TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPsMode(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T mode);

/* FUNCTION NAME: air_port_getPsMode
 * PURPOSE:
 *      Get the power saving mode for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 * OUTPUT:
 *      ptr_mode        --  Bit-map:
 *                          AIR_PORT_PS_LINKSTATUS
 *                          AIR_PORT_PS_EEE
 *                          FALSE: Disable / TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPsMode(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_mode);

/* FUNCTION NAME: air_port_setSmtSpdDwn
 * PURPOSE:
 *      Set Smart speed down feature for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
 *      time            --  AIR_PORT_SSD_2T
 *                          AIR_PORT_SSD_3T
 *                          AIR_PORT_SSD_4T
 *                          AIR_PORT_SSD_5T
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setSmtSpdDwn(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state,
    const UI32_T time);

/* FUNCTION NAME: air_port_getSmtSpdDwn
 * PURPOSE:
 *      Get Smart speed down feature for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_state       --  FALSE:Disable
 *                          TRUE: Enable
 *      ptr_time        --  AIR_PORT_SSD_2T
 *                          AIR_PORT_SSD_3T
 *                          AIR_PORT_SSD_4T
 *                          AIR_PORT_SSD_5T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getSmtSpdDwn(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_state,
    UI32_T *ptr_time);

/* FUNCTION NAME: air_port_setEnable
 * PURPOSE:
 *      Set powerdown state for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setEnable(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state);

/* FUNCTION NAME: air_port_getEnable
 * PURPOSE:
 *      Get powerdown state for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_state       --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getEnable(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_state);

/* FUNCTION NAME: air_port_setSpTag
 * PURPOSE:
 *      Set special tag state of a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      sptag_en        --  TRUE:  Enable special tag
 *                          FALSE: Disable special tag
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
air_port_setSpTag(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T sptag_en);

/* FUNCTION NAME: air_port_getSpTag
 * PURPOSE:
 *      Get special tag state of a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 * OUTPUT:
 *      ptr_sptag_en    --  TRUE:  Special tag enable
 *                          FALSE: Special tag disable
 *
 * RETURN:
 *        AIR_E_OK
 *        AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getSpTag(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_sptag_en);

/* FUNCTION NAME: air_port_set5GBaseRModeEnable
 * PURPOSE:
 *      Set the port5 5GBase-R mode enable
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_set5GBaseRModeEn(
    const UI32_T unit);

/* FUNCTION NAME: air_port_setHsgmiiModeEnable
 * PURPOSE:
 *      Set the port5 HSGMII mode enable
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setHsgmiiModeEn(
    const UI32_T unit);

/* FUNCTION NAME: air_port_setSgmiiMode
 * PURPOSE:
 *      Set the port5 SGMII mode for AN or force
 *
 * INPUT:
 *      unit            --  Device ID
 *      mode            --  AIR_PORT_SGMII_MODE_AN
 *                          AIR_PORT_SGMII_MODE_FORCE
 *      speed           --  AIR_PORT_SPEED_10M:   10Mbps
 *                          AIR_PORT_SPEED_100M:  100Mbps
 *                          AIR_PORT_SPEED_1000M: 1Gbps
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setSgmiiMode(
    const UI32_T unit,
    const UI32_T mode,
    const UI32_T speed);


/* FUNCTION NAME: air_port_setRmiiMode
 * PURPOSE:
 *      Set the port5 RMII mode for 100Mbps or 10Mbps
 *
 * INPUT:
 *      unit            --  Device ID
 *      speed           --  AIR_PORT_SPEED_10M:  10Mbps
 *                          AIR_PORT_SPEED_100M: 100Mbps
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setRmiiMode(
    const UI32_T unit,
    const UI32_T speed);

/* FUNCTION NAME: air_port_setRgmiiMode
 * PURPOSE:
 *      Set the port5 RGMII mode for 1Gbps or 100Mbps or 10Mbps
 *
 * INPUT:
 *      unit            --  Device ID
 *      speed           --  AIR_PORT_SPEED_10M:   10Mbps
 *                          AIR_PORT_SPEED_100M:  100Mbps
 *                          AIR_PORT_SPEED_1000M: 1Gbps
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setRgmiiMode(
    const UI32_T unit,
    const UI32_T speed);

#endif  /* AIR_PORT_H */


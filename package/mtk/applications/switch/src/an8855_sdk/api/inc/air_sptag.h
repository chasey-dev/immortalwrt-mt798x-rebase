/* FILE NAME: air_sptag.h
 * PURPOSE:
 *      Define the Special Tag function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_SPTAG_H
#define AIR_SPTAG_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/

#define AIR_STAG_BUF_LEN                (4)
#define AIR_STAG_ALIGN_BIT_WIDTH        (8)
#define AIR_STAG_REPLACE_MODE_MAX_DP    (10)

/* cpu tx stag offset */
#define AIR_STAG_TX_OPC_BIT_OFFSET      (5)
#define AIR_STAG_TX_OPC_BIT_WIDTH       (3)
#define AIR_STAG_TX_VPM_BIT_OFFSET      (0)
#define AIR_STAG_TX_VPM_BIT_WIDTH       (2)
#define AIR_STAG_TX_PCP_BIT_OFFSET      (5)
#define AIR_STAG_TX_PCP_BIT_WIDTH       (3)
#define AIR_STAG_TX_DEI_BIT_OFFSET      (4)
#define AIR_STAG_TX_DEI_BIT_WIDTH       (1)

/* cpu rx stag offset */
#define AIR_STAG_RX_RSN_BIT_OFFSET      (2)
#define AIR_STAG_RX_RSN_BIT_WIDTH       (3)
#define AIR_STAG_RX_VPM_BIT_OFFSET      (0)
#define AIR_STAG_RX_VPM_BIT_WIDTH       (2)
#define AIR_STAG_RX_SP_BIT_OFFSET       (0)
#define AIR_STAG_RX_SP_BIT_WIDTH        (5)
#define AIR_STAG_RX_PCP_BIT_OFFSET      (5)
#define AIR_STAG_RX_PCP_BIT_WIDTH       (3)
#define AIR_STAG_RX_DEI_BIT_OFFSET      (4)
#define AIR_STAG_RX_DEI_BIT_WIDTH       (1)
#define AIR_PORT_NUM                    (6)

#define AIR_PORT_FOREACH(bitmap, port)                 \
            for(port = 0; port < AIR_PORT_NUM; port++) \
                if(bitmap & BIT(port))


/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef enum
{
    AIR_STAG_MODE_INSERT,
    AIR_STAG_MODE_REPLACE,
    AIR_STAG_MODE_LAST
} AIR_STAG_MODE_T;

typedef enum
{
    /* Egress DP is port map */
    AIR_STAG_OPC_PORTMAP,

    /* Egress DP is port id */
    AIR_STAG_OPC_PORTID,

    /* Forward the packet according to lookup result */
    AIR_STAG_OPC_LOOKUP,
    AIR_STAG_OPC_LAST
} AIR_STAG_OPC_T;

typedef enum
{
    AIR_STAG_REASON_CODE_NORMAL,
    AIR_STAG_REASON_CODE_SFLOW,
    AIR_STAG_REASON_CODE_TTL_ERR,
    AIR_STAG_REASON_CODE_ACL,
    AIR_STAG_REASON_CODE_SA_FULL,
    AIR_STAG_REASON_CODE_PORT_MOVE_ERR,
    AIR_STAG_REASON_CODE_LAST,
} AIR_STAG_REASON_CODE_T;

typedef enum
{
    AIR_STAG_VPM_UNTAG,
    AIR_STAG_VPM_TPID_8100,
    AIR_STAG_VPM_TPID_88A8,
    AIR_STAG_VPM_TPID_PRE_DEFINED,
    AIR_STAG_VPM_LAST,
} AIR_STAG_VPM_T;

typedef struct AIR_STAG_TX_PARA_S
{
    /* destination port operation code */
    AIR_STAG_OPC_T      opc;

    /* tag attribute */
    AIR_STAG_VPM_T      vpm;

    /* destination port map */
    UI32_T              pbm;

    /* PRI in vlan tag */
    UI32_T              pri :3;

    /* CFI in vlan tag */
    UI32_T              cfi :1;

    /* VID in vlan tag */
    UI32_T              vid :12;
} AIR_STAG_TX_PARA_T;


typedef struct AIR_SPTAG_RX_PARA_S
{
    AIR_STAG_REASON_CODE_T rsn;        /* tag attribute */
    AIR_STAG_VPM_T         vpm;        /* tag attribute */
    UI32_T                 spn;        /* source port */
    UI32_T                 pri;        /* PRI in vlan tag */
    UI32_T                 cfi;        /* CFI in vlan tag */
    UI32_T                 vid;        /* VID in vlan tag */
}AIR_SPTAG_RX_PARA_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: air_sptag_setState
 * PURPOSE:
 *      Set special tag enable/disable for port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Special tag Port
 *      sp_en           --  special tag Enable or Disable
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
air_sptag_setState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T sp_en);

/* FUNCTION NAME: air_switch_getCpuPortEn
 * PURPOSE:
 *      Get CPU port member
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Special tag Port
 *
 * OUTPUT:
 *      sp_en           --  special tag enable or disable
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sptag_getState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *sp_en);

/* FUNCTION NAME: air_sptag_setMode
 * PURPOSE:
 *      Set special tag enable/disable for port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Special tag Port
 *      mode            --  insert mode or replace mode
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
air_sptag_setMode(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T mode);

/* FUNCTION NAME: air_sptag_getMode
 * PURPOSE:
 *      Get CPU port member
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Special tag Port
 *
 * OUTPUT:
 *      mode            --  insert or replace mode
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sptag_getMode(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *mode);

/* FUNCTION NAME: air_sptag_encodeTx
 * PURPOSE:
 *      Encode tx special tag into buffer.
 * INPUT:
 *      unit            --  Device ID
 *      ptr_sptag_tx    --  Special tag parameters
 *      ptr_buf         --  Buffer address
 *      ptr_len         --  Buffer length
 * OUTPUT:
 *      ptr_len         --  Written buffer length
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sptag_encodeTx(
    const UI32_T unit,
    const AIR_STAG_MODE_T mode,
    AIR_STAG_TX_PARA_T *ptr_sptag_tx,
    UI8_T *ptr_buf,
    UI32_T *ptr_len);

/* FUNCTION NAME: air_sptag_decodeRx
 * PURPOSE:
 *      Decode rx special tag from buffer.
 * INPUT:
 *      unit            --  Device ID
 *      ptr_buf         --  Buffer address
 *      len             --  Buffer length
 * OUTPUT:
 *      ptr_sptag_rx    --  Special tag parameters
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sptag_decodeRx(
    const UI32_T unit,
    const UI8_T *ptr_buf,
    const UI32_T len,
    AIR_SPTAG_RX_PARA_T *ptr_sptag_rx);

#endif  /* AIR_SPTAG_H */

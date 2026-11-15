/* FILE NAME: air_mib.h
 * PURPOSE:
 *      Define the MIB counter function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_MIB_H
#define AIR_MIB_H

/* INCLUDE FILE DECLARATIONS
*/
#define MIB_ALL_ITEM    0xFFFFFFFF
#define AIR_MIB_MAX_ACL_EVENT_NUM      (64)
#define CSR_ACL_MIB_SEL_OFFSET          (4)

/* NAMING CONSTANT DECLARATIONS
*/
typedef struct AIR_MIB_CNT_TX_S
{
    UI32_T TDPC;      /* TX Drop Packet */
    UI32_T TCRC;      /* TX CRC Packet */
    UI32_T TUPC;      /* TX Unicast Packet */
    UI32_T TMPC;      /* TX Multicast Packet */
    UI32_T TBPC;      /* TX Broadcast Packet */
    UI32_T TCEC;      /* TX Collision Event Count */
    UI32_T TSCEC;     /* TX Single Collision Event Count */
    UI32_T TMCEC;     /* TX Multiple Conllision Event Count */
    UI32_T TDEC;      /* TX Deferred Event Count */
    UI32_T TLCEC;     /* TX Late Collision Event Count */
    UI32_T TXCEC;     /* TX Excessive Collision Event Count */
    UI32_T TPPC;      /* TX Pause Packet */
    UI32_T TL64PC;    /* TX Packet Length 64 bytes */
    UI32_T TL65PC;    /* TX Packet Length 65 ~ 127 bytes */
    UI32_T TL128PC;   /* TX Packet Length 128 ~ 255 bytes */
    UI32_T TL256PC;   /* TX Packet Length 256 ~ 511 bytes */
    UI32_T TL512PC;   /* TX Packet Length 512 ~ 1023 bytes */
    UI32_T TL1024PC;  /* TX Packet Length 1024 ~ 1518 bytes */
    UI32_T TL1519PC;  /* TX Packet Length 1519 ~ max bytes */
    UI32_T TODPC;     /* TX Oversize Drop Packet */
    UI64_T TOC;       /* TX Octets good or bad packtes determined by TX_OCT_CNT_GOOD or TX_OCT_CNT_BAD(64 bit-width)*/
    UI64_T TOC2;      /* TX Octets bad packets (64 bit-width)*/
}AIR_MIB_CNT_TX_T;

typedef struct AIR_MIB_CNT_RX_S
{
    UI32_T RDPC;      /* RX Drop Packet */
    UI32_T RFPC;      /* RX filtering Packet */
    UI32_T RUPC;      /* RX Unicast Packet */
    UI32_T RMPC;      /* RX Multicast Packet */
    UI32_T RBPC;      /* RX Broadcast Packet */
    UI32_T RAEPC;     /* RX Alignment Error Packet */
    UI32_T RCEPC;     /* RX CRC Packet */
    UI32_T RUSPC;     /* RX Undersize Packet */
    UI32_T RFEPC;     /* RX Fragment Error Packet */
    UI32_T ROSPC;     /* RX Oversize Packet */
    UI32_T RJEPC;     /* RX Jabber Error Packet */
    UI32_T RPPC;      /* RX Pause Packet */
    UI32_T RL64PC;    /* RX Packet Length 64 bytes */
    UI32_T RL65PC;    /* RX Packet Length 65 ~ 127 bytes */
    UI32_T RL128PC;   /* RX Packet Length 128 ~ 255 bytes */
    UI32_T RL256PC;   /* RX Packet Length 256 ~ 511 bytes */
    UI32_T RL512PC;   /* RX Packet Length 512 ~ 1023 bytes */
    UI32_T RL1024PC;  /* RX Packet Length 1024 ~ 1518 bytes */
    UI32_T RL1519PC;  /* RX Packet Length 1519 ~ max bytes */
    UI32_T RCDPC;     /* RX_CTRL Drop Packet */
    UI32_T RIDPC;     /* RX Ingress Drop Packet */
    UI32_T RADPC;     /* RX ARL Drop Packet */
    UI32_T FCDPC;     /* FLow Control Drop Packet */
    UI32_T WRDPC;     /* WRED Drop Packtet */
    UI32_T MRDPC;     /* Mirror Drop Packet */
    UI32_T SFSPC;     /* RX  sFlow Sampling Packet */
    UI32_T SFTPC;     /* Rx sFlow Total Packet */
    UI32_T RXC_DPC;   /* Port Control Drop Packet */
    UI64_T ROC;       /* RX Octets good or bad packtes determined by TX_OCT_CNT_GOOD or TX_OCT_CNT_BAD (64 bit-width)*/
    UI64_T ROC2;      /* RX Octets bad packets (64 bit-width)*/

}AIR_MIB_CNT_RX_T;

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: air_mib_setEnable
 * PURPOSE:
 *      Enable or Disable mib count fucntion.
 *
 * INPUT:
 *      unit           --   Device ID
 *      mib_en         --   enable or disable mib_en
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
air_mib_setEnable(
    const UI32_T unit,
    const BOOL_T mib_en);
/* FUNCTION NAME: air_mib_getEnable
 * PURPOSE:
 *      Enable or Disable mib count fucntion.
 *
 * INPUT:
 *      unit           --   Device ID
 *
 * OUTPUT:
 *      mib_en         --   enable or disable mib_en

 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mib_getEnable(
    const UI32_T unit,
    BOOL_T *mib_en);

/* FUNCTION NAME: air_mib_clear
 * PURPOSE:
 *      Clear all counters of all MIB counters.
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
air_mib_clear(
    const UI32_T unit);

/* FUNCTION NAME: air_mib_clear_by_port
 * PURPOSE:
 *      Clear all counters of all MIB counters.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  clear port number
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
air_mib_clear_by_port(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME: air_mib_clearAclEvent
 * PURPOSE:
 *      Clear all counters of ACL event
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

/* FUNCTION NAME: air_mib_get
 * PURPOSE:
 *      Get the structure of MIB counter for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_rx_mib      --  MIB Counters of Rx Event
 *      ptr_tx_mib      --  MIB Counters of Tx Event
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mib_get(
    const UI32_T unit,
    const UI32_T port,
    AIR_MIB_CNT_RX_T *ptr_rx_mib,
    AIR_MIB_CNT_TX_T *ptr_tx_mib);

AIR_ERROR_NO_T
air_mib_clearAclEvent(
    const UI32_T unit);

/* FUNCTION NAME: air_mib_getAclEvent
 * PURPOSE:
 *      Get the total number of ACL event occurred.
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Index of ACL event
 *
 * OUTPUT:
 *      ptr_cnt         --  The total number of ACL event occured
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mib_getAclEvent(
    const UI32_T unit,
    const UI32_T idx,
    UI32_T *ptr_cnt);

#endif /* End of AIR_MIB_H */

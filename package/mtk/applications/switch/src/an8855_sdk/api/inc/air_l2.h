/* FILE NAME: air_l2.h
* PURPOSE:
*       Define the layer 2 functions in AIR SDK.
*
* NOTES:
*       None
*/

#ifndef AIR_L2_H
#define AIR_L2_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/
#define AIR_L2_MAC_MAX_AGE_OUT_TIME (1000000)

#define AIR_L2_MAX_BUSY_TIME    (200)
#define AIR_L2_MAX_AGE_CNT      (BITS(0, (AAC_AGE_CNT_LENGTH - 1)))
#define AIR_L2_MAX_AGE_UNIT     (BITS(0, (AAC_AGE_UNIT_LENGTH - 1)))
#define AIR_L2_MAC_SET_NUM      (4)
#define AIR_L2_MAX_SIZE         (512)

/* Field for MAC Address Table */
/* Field for MAC entry forward control when hit source MAC */
typedef enum
{
    AIR_L2_FWD_CTRL_DEFAULT,
    AIR_L2_FWD_CTRL_CPU_INCLUDE,
    AIR_L2_FWD_CTRL_CPU_EXCLUDE,
    AIR_L2_FWD_CTRL_CPU_ONLY,
    AIR_L2_FWD_CTRL_DROP,
    AIR_L2_FWD_CTRL_LAST
} AIR_L2_FWD_CTRL_T;
/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
/* Entry structure of MAC table */
typedef struct AIR_MAC_ENTRY_S
{
    /* L2 MAC entry keys */
    /* MAC Address */
    AIR_MAC_T mac;

    /* Customer VID (12bits) */
    UI16_T cvid;

    /* Filter ID */
    UI16_T fid;

    /* l2 mac entry attributes */
#define AIR_L2_MAC_ENTRY_FLAGS_IVL     (1U << 0)
#define AIR_L2_MAC_ENTRY_FLAGS_STATIC  (1U << 1)
#define AIR_L2_MAC_ENTRY_FLAGS_UNAUTH  (1U << 2)
    UI32_T flags;

    /* Destination Port Map */
    AIR_PORT_BITMAP_T port_bitmap;

    /* Source MAC address hit forward control */
    AIR_L2_FWD_CTRL_T sa_fwd;

    /* Age Timer only for getting information */
    UI32_T timer;
} AIR_MAC_ENTRY_T;

/* FUNCTION NAME: air_l2_addMacAddr
 * PURPOSE:
 *      Add or set a L2 unicast MAC address entry.
 *      If the address entry doesn't exist, it will add the entry.
 *      If the address entry already exists, it will set the entry
 *      with user input value.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  Structure of MAC Address table
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TABLE_FULL
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_addMacAddr(
    const UI32_T unit,
    const AIR_MAC_ENTRY_T   *ptr_mac_entry);

/* FUNCTION NAME: air_l2_delMacAddr
 * PURPOSE:
 *      Delete a L2 unicast MAC address entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
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
air_l2_delMacAddr(
    const UI32_T unit,
    const AIR_MAC_ENTRY_T   *ptr_mac_entry);

/* FUNCTION NAME:  air_l2_getMacAddr
 * PURPOSE:
 *      Get a L2 unicast MAC address entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *
 * OUTPUT:
 *      ptr_count                -- The number of returned MAC entries
 *      ptr_mac_entry            -- Structure of MAC Address table for
 *                                  searching result.
 *                                  The size of ptr_mac_entry depends
 *                                  on the maximun number of bank.
 *                                  The memory size should greater than
 *                                  ((# of Bank) * (Size of entry
 *                                  structure))
 *                                  AIR_MAC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_TIMEOUT            -- Timeout error.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      If the parameter:mac in input argument ptr_mac_entry[0] is
 *      empty. It means to search the first valid MAC address entry
 *      in MAC address table. Otherwise, to search the specific MAC
 *      address entry in input argument ptr_mac_entry[0].
 *      Input argument ptr_mac_entry[0] needs include mac, ivl and
 *      (fid or cvid) depends on ivl.
 *      If argument ivl is TRUE, cvid is necessary, or fid is.
 */
AIR_ERROR_NO_T
air_l2_getMacAddr(
    const UI32_T unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mac_entry);

/* FUNCTION NAME: hal_sco_l2_getMacBucketSize
 * PURPOSE:
 *      Get the bucket size of one MAC address set when searching L2 table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_size        --  The bucket size
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_getMacBucketSize(
    const UI32_T    unit,
    UI32_T          *ptr_size);


/* FUNCTION NAME: air_l2_getNextMacAddr
 * PURPOSE:
 *      Get the next L2 unicast MAC address entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  The structure of MAC Address table
 *
 * OUTPUT:
 *      ptr_count       --  The number of returned MAC entries
 *      ptr_mac_entry   --  Structure of MAC Address table for searching result.
 *                          The size of ptr_mac_entry depends on the max. number of bank.
 *                          The memory size should greater than ((# of Bank) * (Table size))
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *      AIR_E_ENTRY_NOT_FOUND
 * NOTES:
 *      If the parameter:mac in input argument ptr_mac_entry[0] is empty.
 *      It means to search the next valid MAC address entries of last searching result.
 *      Otherwise, to search the next valid MAC address entry of the specific MAC address
 *      entry in input argument ptr_mac_entry[0].
 *      Input argument ptr_mac_entry[0] needs include mac, ivl and (fid or cvid) depends on ivl.
 *      If argument ivl is TRUE, cvid is necessary, or fid is.
 */
AIR_ERROR_NO_T
air_l2_getNextMacAddr(
    const UI32_T unit,
    UI8_T           *ptr_count,
    AIR_MAC_ENTRY_T *ptr_mac_entry);

/* FUNCTION NAME: air_l2_clearMacAddr
 * PURPOSE:
 *      Clear all L2 unicast MAC address entry.
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
air_l2_clearMacAddr(
    const UI32_T unit);

/* FUNCTION NAME:   air_l2_setMacAddrAgeOut
 * PURPOSE:
 *      Set the age out time of L2 MAC address entries.
 * INPUT:
 *      unit                     -- Device ID
 *      age_time                 -- Age out time (second)
 *                                  (1..AIR_L2_MAC_MAX_AGE_OUT_TIME)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */

AIR_ERROR_NO_T
air_l2_setMacAddrAgeOut(
    const UI32_T    unit,
    const UI32_T    age_time);

/* FUNCTION NAME:   air_l2_getMacAddrAgeOut
 * PURPOSE:
 *      Get the age out time of unicast MAC address.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_age_time             -- age out time
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_l2_getMacAddrAgeOut(
    const UI32_T    unit,
    UI32_T          *ptr_age_time);

/* FUNCTION NAME: air_l2_setAgeEnable
 * PURPOSE:
 *      Set aging state for a specific port.
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
air_l2_setAgeEnable(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state);

/* FUNCTION NAME: air_l2_getAgeEnable
 * PURPOSE:
 *      Get age state for a specific port.
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
air_l2_getAgeEnable(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_state);

#endif /* End of AIR_L2_H */

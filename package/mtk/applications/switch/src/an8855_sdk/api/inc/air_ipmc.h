/* FILE NAME: air_stp.h
 * PURPOSE:
 *      Define the IP multicast function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_IPMC_H
#define AIR_IPMC_H

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
    AIR_IPMC_MATCH_TYPE_IPV4_GRP = 0,
    AIR_IPMC_MATCH_TYPE_IPV4_GRP_SRC,
    AIR_IPMC_MATCH_TYPE_IPV6_GRP,
    AIR_IPMC_MATCH_TYPE_IPV6_GRP_SRC,
    AIR_IPMC_MATCH_TYPE_LAST,
} AIR_IPMC_MATCH_TYPE_T;

typedef enum
{
    AIR_IPMC_TYPE_GRP = 0,
    AIR_IPMC_TYPE_GRP_SRC,
    AIR_IPMC_TYPE_GRP_SRC_AND_GRP,
    AIR_IPMC_TYPE_LAST,
} AIR_IPMC_TYPE_T;

typedef struct AIR_IPMC_ENTRY_S
{
    AIR_IPMC_TYPE_T type;

    /* keys */
    UI16_T vid;

    /* Multicast Group Address */
    AIR_IP_ADDR_T group_addr;

    /* Source IP Address for IGMPv3 and MLDv2 */
    AIR_IP_ADDR_T source_addr;

    /* IP Address entry attributes */
#define AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER    (1U << 0)
    UI32_T flags;

    /* Port member */
    AIR_PORT_BITMAP_T port_bitmap;
} AIR_IPMC_ENTRY_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME:   air_ipmc_addMcastAddr
 * PURPOSE:
 *      This API is used to add a multicast MAC address entry.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast Info
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_EXISTS       -- Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_addMcastAddr(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T  *ptr_entry);

/* FUNCTION NAME:   air_ipmc_getMcastAddr
 * PURPOSE:
 *      This API is used to get a multicast address entry.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast key
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      ptr_entry                -- The multicast info
 *                                  AIR_IPMC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getMcastAddr(
    const UI32_T        unit,
    AIR_IPMC_ENTRY_T    *ptr_entry);

/* FUNCTION NAME:   air_ipmc_delMcastAddr
 * PURPOSE:
 *      This API is used to delete a multicast address entry.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast key
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_delMcastAddr(
    const UI32_T            unit,
    const AIR_IPMC_ENTRY_T  *ptr_entry);

/* FUNCTION NAME: air_ipmc_delAllMcastAddr
 * PURPOSE:
 *      Delete all multicast address entries.
 * INPUT:
 *      unit                     -- Select device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_TIMEOUT            -- Timeout error.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_delAllMcastAddr(
    const UI32_T unit);

/* FUNCTION NAME:   air_ipmc_addMcastMember
 * PURPOSE:
 *      This API is used to add member for a multicast ID.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast Info
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_addMcastMember(
    const UI32_T        unit,
    AIR_IPMC_ENTRY_T    *ptr_entry);

/* FUNCTION NAME:   air_ipmc_delMcastMember
 * PURPOSE:
 *      This API is used to delete member for a multicast ID.
 * INPUT:
 *      unit                     -- Device unit number
 *      ptr_entry                -- The multicast Info
 *                                  AIR_IPMC_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_delMcastMember(
    const UI32_T        unit,
    AIR_IPMC_ENTRY_T    *ptr_entry);

/* FUNCTION NAME:   air_ipmc_getMcastBucketSize
 * PURPOSE:
 *      Get the bucket size of one multicast address set when searching
 *      multicast.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_size                 -- The bucket size
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getMcastBucketSize(
    const UI32_T    unit,
    UI32_T          *ptr_size);

/* FUNCTION NAME:   air_ipmc_getFirstMcastAddr
 * PURPOSE:
 *      This API is used to get the first multicast address entry.
 * INPUT:
 *      unit                     -- Device unit number
 *      match_type               -- The type to search multicast entry.
 *                                  AIR_IPMC_MATCH_TYPE_T
 * OUTPUT:
 *      ptr_entry_cnt            -- The number of multicast address
 *                                  entries.
 *      ptr_entry                -- The multicast entry
 *                                  AIR_IPMC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getFirstMcastAddr(
    const UI32_T            unit,
    AIR_IPMC_MATCH_TYPE_T   match_type,
    UI32_T                  *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T        *ptr_entry);

/* FUNCTION NAME:   air_ipmc_getNextMcastAddr
 * PURPOSE:
 *      This API is used to get next multicast address entry.
 * INPUT:
 *      unit                     -- Select device ID
 *      match_type               -- The type to search multicast entry.
 *                                  AIR_IPMC_MATCH_TYPE_T
 * OUTPUT:
 *      ptr_entry_cnt            -- The number of returned multicast
 *                                  entries.
 *      ptr_entry                -- The multicast searching result.
 *                                  AIR_IPMC_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getNextMcastAddr(
    const UI32_T            unit,
    AIR_IPMC_MATCH_TYPE_T   match_type,
    UI32_T                  *ptr_entry_cnt,
    AIR_IPMC_ENTRY_T        *ptr_entry);

/* FUNCTION NAME:   air_ipmc_setPortIpmcMode
 * PURPOSE:
 *      This API is used to set IPMC mode.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port number
 *      enable                   -- IPMC mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_setPortIpmcMode(
    const UI32_T    unit,
    const UI32_T    port,
    const BOOL_T    enable);

/* FUNCTION NAME:   air_ipmc_getPortIpmcMode
 * PURPOSE:
 *      This API is used to get IPMC mode.
 * INPUT:
 *      unit                     -- Device unit number
 *      port                     -- Port number
 * OUTPUT:
 *      ptr_enable               -- IPMC mode
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ipmc_getPortIpmcMode(
    const UI32_T    unit,
    const UI32_T    port,
    BOOL_T          *ptr_enable);
#endif /* End of AIR_IPMC_H */

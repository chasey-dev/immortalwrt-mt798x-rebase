/* FILE NAME: air_ipmc.c
 * PURPOSE:
 *      Define the IP multicast function in AIR SDK.
 *
 * NOTES:
 *      None
 */

/* INCLUDE FILE DECLARATIONS
*/
#include "air.h"

/* NAMING CONSTANT DECLARATIONS
*/
#define AIR_TABLE_PORT_MSK         (0x0FFFFFFF)

/* MACRO FUNCTION DECLARATIONS
*/
#define AIR_IPMC_WRITE_IPV6_GROUP(unit, __ip__, __size__)                          \
{                                                                                  \
    UI32_T reversed_addr = 0, ip6addr = 0;                                         \
    memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[0], __size__);                      \
    reversed_addr = (ip6addr >> 24) |                                              \
                   ((ip6addr >> 8) & 0x0000FF00) |                                 \
                   ((ip6addr << 8) & 0x00FF0000) |                                 \
                   (ip6addr << 24);                                                \
    aml_writeReg(unit, ATA7, reversed_addr);                                        \
    memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[4], __size__);                      \
    reversed_addr = (ip6addr >> 24) |                                              \
                   ((ip6addr >> 8) & 0x0000FF00) |                                 \
                   ((ip6addr << 8) & 0x00FF0000) |                                 \
                   (ip6addr << 24);                                                \
    aml_writeReg(unit, ATA5, reversed_addr);                                        \
    memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[8], __size__);                      \
    reversed_addr = (ip6addr >> 24) |                                              \
                   ((ip6addr >> 8) & 0x0000FF00) |                                 \
                   ((ip6addr << 8) & 0x00FF0000) |                                 \
                   (ip6addr << 24);                                                \
    aml_writeReg(unit, ATA3, reversed_addr);                                        \
    memcpy(&ip6addr, &__ip__.ip_addr.ipv6_addr[12], __size__);                     \
    reversed_addr = (ip6addr >> 24) |                                              \
                   ((ip6addr >> 8) & 0x0000FF00) |                                 \
                   ((ip6addr << 8) & 0x00FF0000) |                                 \
                   (ip6addr << 24);                                                \
    aml_writeReg(unit, ATA1, reversed_addr);                                        \
}
#define AIR_IPMC_IPV4_IS_MULTICAST(addr)                 (0xE0000000 == ((addr) & 0xF0000000))
#define AIR_IPMC_IPV6_IS_MULTICAST(addr)                 (0xFF == (((UI8_T *)(addr))[0]))

#define AIR_IPMC_U32_ENDIAN_XCHG( __ip__)                                      \
    __ip__ = (__ip__ >> 24) |                                                      \
             ((__ip__ >> 8) & 0x0000FF00) |                                        \
             ((__ip__ << 8) & 0x00FF0000) |                                        \
             (__ip__ << 24);

#define AIR_L3_IP_IS_MULTICAST(ptr_ip)                           \
        ((TRUE == (ptr_ip)->ipv4)?                               \
            AIR_IPMC_IPV4_IS_MULTICAST((ptr_ip)->ip_addr.ipv4_addr) : \
            AIR_IPMC_IPV6_IS_MULTICAST((ptr_ip)->ip_addr.ipv6_addr))

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
*/
/* FUNCTION NAME: _findDIPEntry
 * PURPOSE:
 *      Find DIP table on specific group address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      gaddr           --  Specific groupe address
 *      vid             --  Specific VLAN ID
 *
 * OUTPUT:
 *      ptr_mcstinfo    --  MCASTINFO_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_findDIPEntry(
    const UI32_T        unit,
    AIR_IPMC_TYPE_T     type,
    const UI32_T        gaddr,
    const UI32_T        vid,
    AIR_PORT_BITMAP_T   *p_portmap)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T u32dat = 0, i = 0, banks = 0;

    aml_writeReg(unit, ATA1, gaddr);
    u32dat |= (vid << 16);
    aml_writeReg(unit, ATWD, u32dat);

    u32dat = (ATC_CMD_READ | ATC_SAT_DIP | ATC_START_BUSY);
    aml_writeReg(unit, ATC, u32dat);

    /* Check timeout */
    for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ATC, &u32dat);
        if(FALSE == (u32dat & ATC_START_BUSY))
        {
            break;
        }
        AIR_UDELAY(1000);
    }
    if(i == AIR_L2_MAX_BUSY_TIME)
    {
        rc = AIR_E_TIMEOUT;
    }

    if(AIR_E_OK == rc)
    {
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        if(banks)
        {
            for(i= 0; i < AIR_L2_MAC_SET_NUM; i++)
            {
                if(TRUE == BITS_OFF_R(banks, i, 1))
                {
                    /* Select bank */
                    u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                    aml_writeReg(unit, ATRDS, u32dat);
                    aml_readReg(unit, ATRD1, &u32dat);
                    if(u32dat == gaddr)
                    {
                        /* Check vid and save the group member */
                        aml_readReg(unit, ATRD0, &u32dat);
                        if(BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE) == vid)
                        {
                            aml_readReg(unit, ATRD3, &u32dat);
                            *p_portmap[0] |= (u32dat & AIR_TABLE_PORT_MSK);
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        /* Source address mismatch, search next */
                        continue;
                    }
                }
            }
            if(AIR_L2_MAC_SET_NUM == i)
            {
                AIR_PRINT("gaddr=(0x%x), vid=(0x%x), entry not found.\n", gaddr, vid);
                rc = AIR_E_ENTRY_NOT_FOUND;
            }
        }
        else
        {
            rc = AIR_E_ENTRY_NOT_FOUND;
        }
    }

    return rc;
}

/* FUNCTION NAME: _findDIP6Entry
 * PURPOSE:
 *      Find DIP6 table on specific group address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      gaddr           --  Specific groupe address
 *      vid             --  Specific VLAN ID
 *
 * OUTPUT:
 *      ptr_mcstinfo    --  MCASTINFO_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_findDIP6Entry(
    const UI32_T        unit,
    AIR_IPMC_TYPE_T     type,
    const AIR_IP_ADDR_T gaddr,
    const UI32_T        vid,
    AIR_PORT_BITMAP_T   *p_portmap)
{
    UI32_T u32dat = 0, i = 0, bank = 0;
    C8_T   hit = FALSE;
    UI32_T group_addr6[4];
    UI32_T addr, banks;

    memset(group_addr6, 0, sizeof(UI32_T)*4);
    memcpy(&group_addr6[0], &gaddr.ip_addr.ipv6_addr, sizeof(u32dat)*4);

    if(type == AIR_IPMC_TYPE_GRP_SRC)
    {
        AIR_IPMC_WRITE_IPV6_GROUP(unit, gaddr, sizeof(u32dat));
    }

    u32dat = (ATC_CMD_SEARCH | ATC_MAT_DIPV6 | ATC_START_BUSY);
    aml_writeReg(unit, ATC, u32dat);
    (*p_portmap)[0] = 0;
    while(1)
    {
        while(1)
        {
            /* Check timeout */
            for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
            {
                aml_readReg(unit, ATC, &u32dat);
                if(FALSE == (u32dat & ATC_START_BUSY))
                {
                    break;
                }
                AIR_UDELAY(1000);
            }
            if(i == AIR_L2_MAX_BUSY_TIME)
                return AIR_E_TIMEOUT;

            /* Get address */
            addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
            /* Get banks */
            banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
            if(banks == 0xF)/*IPv6 occupy all banks*/
            {
                aml_readReg(unit, ATRD0, &u32dat);
                if(BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE) != vid)
                {
                    break;
                }

                for(bank = 0; bank < AIR_L2_MAC_SET_NUM; bank++)
                {
                    aml_writeReg(unit, ATRDS, bank);
                    aml_readReg(unit, ATRD1, &u32dat);
                    AIR_IPMC_U32_ENDIAN_XCHG(u32dat);
                    if(u32dat == group_addr6[AIR_L2_MAC_SET_NUM-(bank+1)])
                    {
                        hit++;
                    }
                    else
                    {
                        /* Source address mismatch, search next */
                        break;
                    }
                }
                if(hit == AIR_L2_MAC_SET_NUM)
                {
                    aml_readReg(unit, ATRD3, &u32dat);
                    *p_portmap[0] |= (u32dat & AIR_TABLE_PORT_MSK);
                    return AIR_E_OK;
                }
                else
                {
                    if((AIR_L2_MAX_SIZE - 1) == (addr))
                    {
                        return AIR_E_ENTRY_NOT_FOUND;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
        u32dat = (ATC_CMD_SEARCH_NEXT | ATC_MAT_DIPV6 | ATC_START_BUSY);
        aml_writeReg(unit, ATC, u32dat);
    }

    return AIR_E_OK;
}

/* STATIC VARIABLE DECLARATIONS
*/
static BOOL_T _search_end = FALSE;

/* LOCAL SUBPROGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
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
    const AIR_IPMC_ENTRY_T  *ptr_entry)
{
    UI32_T          u32dat = 0, i = 0, banks = 0, port = 0;
    AIR_PORT_BITMAP_T p_portmap = {0}, curr_portmap = {0};
    AIR_ERROR_NO_T ret = 0;

    if(AIR_IP_ADDR_IS_ZERO(ptr_entry->group_addr) ||
       !AIR_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* parameter sanity check */
    if(ptr_entry->group_addr.ipv4 == TRUE)
    {
        ret = _findDIPEntry(unit, ptr_entry->type, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->vid, &curr_portmap);
        if((ret == AIR_E_ENTRY_NOT_FOUND))
        {
            /* Create new entry or update v3 group filter */
            /* Set entry type DIP */
            u32dat = 0;
            u32dat |= (TRUE << ATWD_IPM_VLD_OFFSET);
            /* Set attributes */
            if (ptr_entry->flags & AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER)
            {
                u32dat |= (1UL << ATWD_IPM_LEAKY_OFFSET);
            }
            u32dat |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
            aml_writeReg(unit, ATWD, u32dat);

            /* Set member ports of IPMC entry */
            u32dat = 0;
            AIR_PORT_FOREACH(ptr_entry->port_bitmap[0], port)
            {
                u32dat |= (1 << port);
            }
            aml_writeReg(unit, ATWD2, u32dat);

            /* Set DIP address */
            aml_writeReg(unit, ATA1, ptr_entry->group_addr.ip_addr.ipv4_addr);

            /* Write DIP table */
            u32dat = (ATC_SAT_DIP | ATC_CMD_WRITE | ATC_START_BUSY);
            aml_writeReg(unit, ATC, u32dat);
            /* Check write state */
            for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
            {
                aml_readReg(unit, ATC, &u32dat);
                if(FALSE == (u32dat & ATC_START_BUSY))
                {
                    break;
                }
                AIR_UDELAY(1000);
            }
            if(i == AIR_L2_MAX_BUSY_TIME)
            {
                return AIR_E_TIMEOUT;
            }
            /* Get banks */
            banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
            if(!banks)
            {
                return AIR_E_OTHERS;
            }
        }
        else
        {
            /* IGMPv2 group already exist */
            return AIR_E_ENTRY_EXISTS;
        }
    }
    else
    {
        ret = _findDIP6Entry(unit, ptr_entry->type, ptr_entry->group_addr, ptr_entry->vid, &curr_portmap);
        if((ret == AIR_E_ENTRY_NOT_FOUND))
        {
            /* Create new entry or update v3 group filter */
            /* Set entry type DIP */
            u32dat = 0;
            u32dat |= (TRUE << ATWD_IPM_VLD_OFFSET);
            u32dat |= (TRUE << ATWD_IPM_IPV6_OFFSET);
            /* Set attributes */
            if (ptr_entry->flags & AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER)
            {
                u32dat |= (1UL << ATWD_IPM_LEAKY_OFFSET);
            }
            u32dat |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
            aml_writeReg(unit, ATWD, u32dat);

            /* Set member ports of IPMC entry */
            u32dat = 0;
            AIR_PORT_FOREACH(ptr_entry->port_bitmap[0], port)
            {
                u32dat |= (1 << port);
            }
            aml_writeReg(unit, ATWD2, u32dat);

            /* Set DIP address */
            AIR_IPMC_WRITE_IPV6_GROUP(unit, ptr_entry->group_addr, sizeof(u32dat));

            /* Write DIP table */
            u32dat = (ATC_SAT_DIP | ATC_CMD_WRITE | ATC_START_BUSY);
            aml_writeReg(unit, ATC, u32dat);
            /* Check write state */
            for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
            {
                aml_readReg(unit, ATC, &u32dat);
                if(FALSE == (u32dat & ATC_START_BUSY))
                {
                    break;
                }
                AIR_UDELAY(1000);
            }
            if(i == AIR_L2_MAX_BUSY_TIME)
            {
                return AIR_E_TIMEOUT;
            }
            /* Get banks */
            banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
            if(!banks)
            {
                return AIR_E_OTHERS;
            }
        }
        else
        {
            return AIR_E_ENTRY_EXISTS;
        }
    }

    return AIR_E_OK;
}

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
    AIR_IPMC_ENTRY_T    *ptr_entry)
{
    UI32_T u32dat = 0, atwd = 0, i = 0, addr = 0, banks = 0, port = 0;
    AIR_PORT_BITMAP_T portmap = {0};
    UI16_T vid=0;
    AIR_IP_ADDR_T group_addr, source_addr;
    UI32_T mac_port = 0;

    memset(&group_addr, 0, sizeof(AIR_IP_ADDR_T));

    if(AIR_IP_ADDR_IS_ZERO(ptr_entry->group_addr) ||
       !AIR_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        return AIR_E_BAD_PARAMETER;
    }
    if(ptr_entry->group_addr.ipv4 == TRUE)
    {
        atwd |= (((UI32_T)ptr_entry->vid) << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, atwd);
        aml_writeReg(unit, ATA1, ptr_entry->group_addr.ip_addr.ipv4_addr);

        u32dat = (ATC_CMD_READ | ATC_SAT_DIP | ATC_START_BUSY);


        aml_writeReg(unit, ATC, u32dat);
        /* Check timeout */
        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if(FALSE == (u32dat & ATC_START_BUSY))
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }
        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        if(banks)
        {
            for(i = 0; i < AIR_L2_MAC_SET_NUM; i++)
            {
                if(TRUE == BITS_OFF_R(banks, i, 1))
                {
                    /* Select bank */
                    u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                    aml_writeReg(unit, ATRDS, u32dat);
                    /* Get attributes */
                    aml_readReg(unit, ATRD0, &u32dat);
                    vid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
                    if(ptr_entry->vid != vid)
                    {
                        continue;
                    }
                    if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
                    {
                        ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
                    }
                    else
                    {
                        ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
                    }
                    aml_readReg(unit, ATRD1, &u32dat);
                    group_addr.ipv4 = TRUE;
                    group_addr.ip_addr.ipv4_addr=u32dat;
                    if(AIR_IPV4_ZERO != ptr_entry->group_addr.ip_addr.ipv4_addr)
                    {
                        if(ptr_entry->group_addr.ip_addr.ipv4_addr != group_addr.ip_addr.ipv4_addr)
                        {
                            continue;
                        }
                    }

                    /* group address match */
                    aml_readReg(unit, ATRD3, &u32dat);
                    ptr_entry->port_bitmap[0] = BITS_OFF_R(u32dat, ATRD3_MAC_PORT_OFFSET, ATRD3_MAC_PORT_LENGTH);
               }
            }
        }
        else
        {
            return AIR_E_ENTRY_NOT_FOUND;
        }
    }
    else
    {
        atwd |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        atwd |= (TRUE << ATWD_IPM_IPV6_OFFSET);
        aml_writeReg(unit, ATWD, atwd);
        AIR_IPMC_WRITE_IPV6_GROUP(unit, ptr_entry->group_addr, sizeof(u32dat));

        u32dat = (ATC_CMD_READ | ATC_SAT_DIP | ATC_START_BUSY);

        aml_writeReg(unit, ATC, u32dat);

        /* Check timeout */
        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if(FALSE == (u32dat & ATC_START_BUSY))
            {
                break;
            }
            AIR_UDELAY(1000);
        }

        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        if(banks==0xF)/*IPv6 occupy all banks*/
        {
            aml_readReg(unit, ATRD0, &u32dat);
            vid = BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
            if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
            {
                ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
            }
            else
            {
                ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
            }
            for(i = 0; i < AIR_L2_MAC_SET_NUM; i++)
            {
                /* Select bank */
                u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                aml_writeReg(unit, ATRDS, u32dat);
                aml_readReg(unit, ATRD1, &u32dat);
                AIR_IPMC_U32_ENDIAN_XCHG(u32dat);
                group_addr.ipv4 = FALSE;
                memcpy(&group_addr.ip_addr.ipv6_addr[(AIR_L2_MAC_SET_NUM-(i+1))*4], &u32dat, sizeof(u32dat));
            }
            /* group address match */
            aml_readReg(unit, ATRD3, &u32dat);
            ptr_entry->port_bitmap[0] = BITS_OFF_R(u32dat, ATRD3_MAC_PORT_OFFSET, ATRD3_MAC_PORT_LENGTH);
        }
        else
        {
            return AIR_E_ENTRY_NOT_FOUND;
        }
    }

    return AIR_E_OK;
}

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
    const AIR_IPMC_ENTRY_T  *ptr_entry)
{
    UI32_T u32dat = 0;
    UI32_T i = 0;

    /* Check group address */
    if(AIR_IP_ADDR_IS_ZERO(ptr_entry->group_addr) ||
       !AIR_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        return AIR_E_BAD_PARAMETER;
    }

    if(ptr_entry->group_addr.ipv4 == TRUE)
    {
        /* Set group address */
        aml_writeReg(unit, ATA1, ptr_entry->group_addr.ip_addr.ipv4_addr);

        /* Set DIP STATUS = 0 */
        u32dat = 0;
        u32dat |= (FALSE << ATWD_IPM_VLD_OFFSET);
        u32dat |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, u32dat);

        /* Write DIP table */
        u32dat = (ATC_SAT_DIP | ATC_CMD_WRITE | ATC_START_BUSY);
        aml_writeReg(unit, ATC, u32dat);
        /* Check write state */
        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if(FALSE == (u32dat & ATC_START_BUSY))
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }
    }
    else
    {
        /* Set group address */
        AIR_IPMC_WRITE_IPV6_GROUP(unit, ptr_entry->group_addr, sizeof(u32dat));

        /* Set DIP STATUS = 0 */
        u32dat = 0;
        u32dat |= (FALSE << ATWD_IPM_VLD_OFFSET);
        u32dat |= (TRUE << ATWD_IPM_IPV6_OFFSET);
        u32dat |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, u32dat);

        /* Write DIP table */
        u32dat = (ATC_SAT_DIP | ATC_CMD_WRITE | ATC_START_BUSY);
        aml_writeReg(unit, ATC, u32dat);
        /* Check write state */
        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if(FALSE == (u32dat & ATC_START_BUSY))
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }
    }

    if(!BITS_OFF_R(u32dat, ATC_SINGLE_HIT_OFFSET, ATC_SINGLE_HIT_LENGTH))
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    return AIR_E_OK;
}

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
    const UI32_T unit)
{
    UI32_T u32dat = 0;
    UI32_T i = 0;

    /* Clear all SIP entry */
    u32dat = (ATC_CMD_CLEAN | ATC_MAT_SIP | ATC_START_BUSY);
    aml_writeReg(unit, ATC, u32dat);
    /* Check write state */
    for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ATC, &u32dat);
        if(FALSE == (u32dat & ATC_START_BUSY))
        {
            break;
        }
        AIR_UDELAY(1000);
    }
    if(i == AIR_L2_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }

    /* Clear all DIP entry*/
    u32dat = (ATC_CMD_CLEAN | ATC_MAT_DIP | ATC_START_BUSY);
    aml_writeReg(unit, ATC, u32dat);
    /* Check write state */
    for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ATC, &u32dat);
        if(FALSE == (u32dat & ATC_START_BUSY))
        {
            break;
        }
        AIR_UDELAY(1000);
    }
    if(i == AIR_L2_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }

    return AIR_E_OK;
}

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
    AIR_IPMC_ENTRY_T    *ptr_entry)
{
    UI32_T      u32dat = 0, atwd=0, i = 0, port = 0;
    AIR_PORT_BITMAP_T	map={0}, portmask={0}, p_portmap={0}, mt_portmap={0}, st_portmap={0};

    p_portmap[0] = ptr_entry->port_bitmap[0];
    /* Check group address */
    if(AIR_IP_ADDR_IS_ZERO(ptr_entry->group_addr) ||
       !AIR_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        return AIR_E_BAD_PARAMETER;
    }

    if(ptr_entry->group_addr.ipv4 == TRUE)
    {
        /* Check group exist or not */
        if(ptr_entry->type == AIR_IPMC_TYPE_GRP)
        {
            if(_findDIPEntry(unit, ptr_entry->type, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->vid, &mt_portmap) == AIR_E_ENTRY_NOT_FOUND)
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }

        /* Check version */
        if(ptr_entry->type != AIR_IPMC_TYPE_GRP)
        {
            return AIR_E_BAD_PARAMETER;
        }

        /* Get portmask by max port number */
        for(i = 0; i < (AIR_MAX_NUM_OF_PORTS - 1); i++)
            AIR_PORT_ADD(portmask, i);

        /* Set DIP table to new member */
        aml_writeReg(unit, ATA1, ptr_entry->group_addr.ip_addr.ipv4_addr);

        /* Add member to DIP specitic group entry */
        AIR_PORT_FOREACH(mt_portmap[0], port)
        {
            AIR_PORT_ADD(p_portmap, port);
        }

        AIR_PORT_FOREACH(portmask[0], port)
        {
            if(AIR_PORT_CHK(p_portmap, port))
            {
                AIR_PORT_ADD(map, port);
            }
        }

        atwd |= (TRUE << ATWD_IPM_VLD_OFFSET);
        atwd |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, atwd);
        u32dat = 0;
        AIR_PORT_FOREACH(map[0], port)
        {
            u32dat |= (1 << port);
        }
        aml_writeReg(unit, ATWD2, u32dat);

        /* Write DIP table */
        u32dat = (ATC_SAT_DIP | ATC_CMD_WRITE | ATC_START_BUSY);
        aml_writeReg(unit, ATC, u32dat);
        /* Check write state */
        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if((u32dat & ATC_START_BUSY) == 0)
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }

    }
    else
    {
        /* Check group exist or not */
        if(_findDIP6Entry(unit, ptr_entry->type, ptr_entry->group_addr, ptr_entry->vid, &mt_portmap) == AIR_E_ENTRY_NOT_FOUND)
        {
            return AIR_E_ENTRY_NOT_FOUND;
        }

        /* Check version */
        if(ptr_entry->type != AIR_IPMC_TYPE_GRP)
        {
            return AIR_E_BAD_PARAMETER;
        }

        /* Get portmask by max port number */
        for(i = 0; i < (AIR_MAX_NUM_OF_PORTS - 1); i++)
            AIR_PORT_ADD(portmask, i);

        /* Set DIP table to new member */
        AIR_IPMC_WRITE_IPV6_GROUP(unit, ptr_entry->group_addr, sizeof(u32dat));

        /* Assign new member*/
        /* Add member to DIP specitic group entry */
        AIR_PORT_FOREACH(mt_portmap[0], port)
        {
            AIR_PORT_ADD(p_portmap, port);
        }

        AIR_PORT_FOREACH(portmask[0], port)
        {
            if(AIR_PORT_CHK(p_portmap, port))
            {
                AIR_PORT_ADD(map, port);
            }
        }

        atwd |= (TRUE << ATWD_IPM_VLD_OFFSET);
        atwd |= (TRUE << ATWD_IPM_IPV6_OFFSET);
        atwd |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, atwd);
        u32dat = 0;
        AIR_PORT_FOREACH(map[0], port)
        {
            u32dat |= (1 << port);
        }
        aml_writeReg(unit, ATWD2, u32dat);

        /* Write DIP table */
        u32dat = (ATC_SAT_DIP | ATC_CMD_WRITE | ATC_START_BUSY);
        aml_writeReg(unit, ATC, u32dat);
        /* Check write state */
        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if((u32dat & ATC_START_BUSY) == 0)
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }
    }
    return AIR_E_OK;
}

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
    AIR_IPMC_ENTRY_T    *ptr_entry)
{
    UI32_T      u32dat = 0, atwd=0, i = 0, port = 0;
    AIR_PORT_BITMAP_T portmask={0}, map={0}, p_portmap={0}, mt_portmap={0}, st_portmap={0};


    p_portmap[0] = ptr_entry->port_bitmap[0];

    /* Check group address */
    if(AIR_IP_ADDR_IS_ZERO(ptr_entry->group_addr) ||
       !AIR_L3_IP_IS_MULTICAST(&ptr_entry->group_addr))
    {
        return AIR_E_BAD_PARAMETER;
    }

    if(ptr_entry->group_addr.ipv4 == TRUE)
    {
        /* Check group exist or not */
        if(ptr_entry->type == AIR_IPMC_TYPE_GRP)
        {
            if(_findDIPEntry(unit, ptr_entry->type, ptr_entry->group_addr.ip_addr.ipv4_addr, ptr_entry->vid, &mt_portmap) == AIR_E_ENTRY_NOT_FOUND)
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }

        /* Check version */
        if(ptr_entry->type != AIR_IPMC_TYPE_GRP)
        {
            return AIR_E_BAD_PARAMETER;
        }

        /* Get portmask by max port number */
        for(i = 0; i < (AIR_MAX_NUM_OF_PORTS - 1); i++)
            AIR_PORT_ADD(portmask, i);

        /* Set DIP table to new member */
        aml_writeReg(unit, ATA1, ptr_entry->group_addr.ip_addr.ipv4_addr);

        /* Assign new member*/
        /* Delete member to DIP specitic group entry */
        AIR_PORT_FOREACH(p_portmap[0], port)
        {
            AIR_PORT_DEL(mt_portmap, port);
        }
        AIR_PORT_FOREACH(mt_portmap[0], port)
        {
            if(AIR_PORT_CHK(portmask, port))
            {
                AIR_PORT_ADD(map, port);
            }
        }

        atwd |= (TRUE << ATWD_IPM_VLD_OFFSET);
        atwd |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, atwd);
        u32dat = 0;
        AIR_PORT_FOREACH(map[0], port)
        {
            u32dat |= (1 << port);
        }
        aml_writeReg(unit, ATWD2, u32dat);

        /* Write DIP table */
        u32dat = (ATC_SAT_DIP | ATC_CMD_WRITE | ATC_START_BUSY);
        aml_writeReg(unit, ATC, u32dat);
        /* Check write state */
        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if((u32dat & ATC_START_BUSY) == 0)
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }
    }
    else
    {
        /* Check group exist or not */
        if(_findDIP6Entry(unit, ptr_entry->type, ptr_entry->group_addr, ptr_entry->vid, &mt_portmap) == AIR_E_ENTRY_NOT_FOUND)
            return AIR_E_ENTRY_NOT_FOUND;

        /* Check version */
        if(ptr_entry->type != AIR_IPMC_TYPE_GRP)
        {
            return AIR_E_BAD_PARAMETER;
        }

        /* Get portmask by max port number */
        for(i = 0; i < (AIR_MAX_NUM_OF_PORTS - 1); i++)
            AIR_PORT_ADD(portmask, i);

        /* Set DIP table to new member */
        AIR_IPMC_WRITE_IPV6_GROUP(unit, ptr_entry->group_addr, sizeof(u32dat));

        /* Assign new member*/
        /* Delete member to DIP specitic group entry */
        AIR_PORT_FOREACH(p_portmap[0],port)
        {
            AIR_PORT_DEL(mt_portmap, port);
        }
        AIR_PORT_FOREACH(mt_portmap[0], port)
        {
            if(AIR_PORT_CHK(portmask, port))
            {
                AIR_PORT_ADD(map, port);
            }
        }

        atwd |= (TRUE << ATWD_IPM_VLD_OFFSET);
        atwd |= (TRUE << ATWD_IPM_IPV6_OFFSET);
        atwd |= (ptr_entry->vid << ATWD_IPM_VID_OFFSET);
        aml_writeReg(unit, ATWD, atwd);
        u32dat = 0;
        AIR_PORT_FOREACH(map[0], port)
        {
            u32dat |= (1 << port);
        }
        aml_writeReg(unit, ATWD2, u32dat);

        /* Write DIP table */
        u32dat = (ATC_SAT_DIP | ATC_CMD_WRITE | ATC_START_BUSY);
        aml_writeReg(unit, ATC, u32dat);
        /* Check write state */
        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if((u32dat & ATC_START_BUSY) == 0)
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }
    }

    return AIR_E_OK;
}

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
    UI32_T          *ptr_size)
{
    /* Access regiser */
    (*ptr_size) = AIR_L2_MAC_SET_NUM;

    return AIR_E_OK;
}

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
    AIR_IPMC_ENTRY_T        *ptr_entry)
{
    UI32_T u32dat = 0, i=0, port=0;
    UI32_T addr = 0, banks = 0;
    AIR_PORT_BITMAP_T portmap;
    UI32_T mac_port = 0;

    portmap[0] = 0;

    _search_end = FALSE;
    if(AIR_IPMC_MATCH_TYPE_IPV4_GRP == match_type)
    {
        if(AIR_IPMC_MATCH_TYPE_IPV4_GRP == match_type)
        {
            u32dat = (ATC_CMD_SEARCH | ATC_MAT_DIPV4 | ATC_START_BUSY);
        }
        aml_writeReg(unit, ATC, u32dat);

        /* Check timeout */
        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if(FALSE == (u32dat & ATC_START_BUSY))
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        if(banks)
        {
            for(i = 0; i < AIR_L2_MAC_SET_NUM; i++)
            {
                portmap[0] = 0;
                if(TRUE == BITS_OFF_R(banks, i, 1))
                {
                    /* Select bank */
                    u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                    aml_writeReg(unit, ATRDS, u32dat);
                    /* Get attributes */
                    aml_readReg(unit, ATRD0, &u32dat);
                    ptr_entry->vid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
                    if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
                    {
                        ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
                    }
                    else
                    {
                        ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
                    }
                    aml_readReg(unit, ATRD1, &u32dat);
                    ptr_entry->group_addr.ipv4 = TRUE;
                    ptr_entry->group_addr.ip_addr.ipv4_addr = u32dat;

                    aml_readReg(unit, ATRD3, &u32dat);
                    ptr_entry->port_bitmap[0] = BITS_OFF_R(u32dat, ATRD3_MAC_PORT_OFFSET, ATRD3_MAC_PORT_LENGTH);
                    (*ptr_entry_cnt)++;
                    ptr_entry++;
                }
            }
            if((AIR_L2_MAX_SIZE - 1) == (addr) && (*ptr_entry_cnt) == 0)
            {
                AIR_PRINT("u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
        else
        {
            if((AIR_L2_MAX_SIZE - 1) == (addr))
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else if(AIR_IPMC_MATCH_TYPE_IPV6_GRP == match_type)
    {
        if(AIR_IPMC_MATCH_TYPE_IPV6_GRP == match_type)
        {
            u32dat = (ATC_CMD_SEARCH | ATC_MAT_DIPV6 | ATC_START_BUSY);
        }
        aml_writeReg(unit, ATC, u32dat);

        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if(FALSE == (u32dat & ATC_START_BUSY))
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        if(banks == 0xF)
        {
            aml_readReg(unit, ATRD0, &u32dat);
            ptr_entry->vid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
            if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
            {
                ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
            }
            else
            {
                ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
            }
            for(i = 0; i < AIR_L2_MAC_SET_NUM; i++)
            {
                /* select bank */
                u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                aml_writeReg(unit, ATRDS, u32dat);
                aml_readReg(unit, ATRD1, &u32dat);
                AIR_IPMC_U32_ENDIAN_XCHG(u32dat);
                ptr_entry->group_addr.ipv4 = FALSE;
                memcpy(&ptr_entry->group_addr.ip_addr.ipv6_addr[(AIR_L2_MAC_SET_NUM-(i+1))*4], &u32dat, sizeof(u32dat));
            }
            aml_readReg(unit, ATRD3, &u32dat);
            ptr_entry->port_bitmap[0] = BITS_OFF_R(u32dat, ATRD3_MAC_PORT_OFFSET, ATRD3_MAC_PORT_LENGTH);
            (*ptr_entry_cnt)++;
            ptr_entry++;
        }
        else
        {
            if((AIR_L2_MAX_SIZE - 1) == (addr))
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    return AIR_E_OK;
}

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
    AIR_IPMC_ENTRY_T        *ptr_entry)
{
    UI32_T u32dat = 0, i=0, port=0;
    UI32_T addr = 0, banks = 0;
    AIR_PORT_BITMAP_T portmap;
    UI32_T mac_port = 0;

    portmap[0] = 0;

    /* If found the lastest entry last time, we couldn't keep to search the next entry */
    if (TRUE == _search_end)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    if(AIR_IPMC_MATCH_TYPE_IPV4_GRP == match_type)
    {
        if(AIR_IPMC_MATCH_TYPE_IPV4_GRP == match_type)
        {
            u32dat = (ATC_CMD_SEARCH_NEXT | ATC_MAT_DIPV4 | ATC_START_BUSY);
        }
        aml_writeReg(unit, ATC, u32dat);

        /* Check timeout */
        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if(FALSE == (u32dat & ATC_START_BUSY))
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        if ((AIR_L2_MAX_SIZE - 1) == addr)
        {
            _search_end = TRUE;
        }
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        if(banks)
        {
            for(i = 0; i < AIR_L2_MAC_SET_NUM; i++)
            {
                portmap[0] = 0;
                if(TRUE == BITS_OFF_R(banks, i, 1))
                {
                    /* Select bank */
                    u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                    aml_writeReg(unit, ATRDS, u32dat);
                    /* Get attributes */
                    aml_readReg(unit, ATRD0, &u32dat);
                    ptr_entry->vid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
                    if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
                    {
                        ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
                    }
                    else
                    {
                        ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
                    }
                    aml_readReg(unit, ATRD1, &u32dat);
                    ptr_entry->group_addr.ipv4 = TRUE;
                    ptr_entry->group_addr.ip_addr.ipv4_addr = u32dat;

                    aml_readReg(unit, ATRD3, &u32dat);
                    ptr_entry->port_bitmap[0] = BITS_OFF_R(u32dat, ATRD3_MAC_PORT_OFFSET, ATRD3_MAC_PORT_LENGTH);
                    (*ptr_entry_cnt)++;
                    ptr_entry++;
                }
            }
            if((AIR_L2_MAX_SIZE - 1) == (addr) && (*ptr_entry_cnt) == 0)
            {
                AIR_PRINT("u32dat=(0x%x), addr=(0x%x), banks=(0x%x)\n", u32dat, addr, banks);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
        else
        {
            if((AIR_L2_MAX_SIZE - 1) == (addr))
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else if(AIR_IPMC_MATCH_TYPE_IPV6_GRP == match_type)
    {
        if(AIR_IPMC_MATCH_TYPE_IPV6_GRP == match_type)
        {
            u32dat = (ATC_CMD_SEARCH_NEXT | ATC_MAT_DIPV6 | ATC_START_BUSY);
        }
        aml_writeReg(unit, ATC, u32dat);

        for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
        {
            aml_readReg(unit, ATC, &u32dat);
            if(FALSE == (u32dat & ATC_START_BUSY))
            {
                break;
            }
            AIR_UDELAY(1000);
        }
        if(i == AIR_L2_MAX_BUSY_TIME)
        {
            return AIR_E_TIMEOUT;
        }

        /* Get address */
        addr = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
        if ((AIR_L2_MAX_SIZE - 1) == addr)
        {
            _search_end = TRUE;
        }
        /* Get banks */
        banks = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
        if(banks == 0xF)
        {
            aml_readReg(unit, ATRD0, &u32dat);
            ptr_entry->vid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_IPM_VID_OFFSET, ATRD0_IPM_VID_RANGE);
            if (BITS_OFF_R(u32dat, ATRD0_IPM_LEAKY_OFFSET, ATRD0_IPM_LEAKY_RANGE))
            {
                ptr_entry->flags |= AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER;
            }
            else
            {
                ptr_entry->flags &= ~(AIR_IPMC_ENTRY_FLAGS_DISABLE_EGRESS_VLAN_FILTER);
            }
            for(i = 0; i < AIR_L2_MAC_SET_NUM; i++)
            {
                /* select bank */
                u32dat = BITS_OFF_L(i, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
                aml_writeReg(unit, ATRDS, u32dat);
                aml_readReg(unit, ATRD1, &u32dat);
                AIR_IPMC_U32_ENDIAN_XCHG(u32dat);
                ptr_entry->group_addr.ipv4 = FALSE;
                memcpy(&ptr_entry->group_addr.ip_addr.ipv6_addr[(AIR_L2_MAC_SET_NUM-(i+1))*4], &u32dat, sizeof(u32dat));
            }
            aml_readReg(unit, ATRD3, &u32dat);
            ptr_entry->port_bitmap[0] = BITS_OFF_R(u32dat, ATRD3_MAC_PORT_OFFSET, ATRD3_MAC_PORT_LENGTH);
            (*ptr_entry_cnt)++;
            ptr_entry++;
        }
        else
        {
            if((AIR_L2_MAX_SIZE - 1) == (addr))
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    return AIR_E_OK;
}

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
    const BOOL_T    enable)
{
    UI32_T u32dat = 0;

    if(TRUE == enable)
    {
        /* Enable igmp snooping */
        u32dat |= (PIC_PORT_IGMP_CTRL_CSR_IPM_01| PIC_PORT_IGMP_CTRL_CSR_IPM_33 | PIC_PORT_IGMP_CTRL_CSR_IPM_224);
    }
    else
    {
        /* Disable igmp snooping */
        u32dat = 0;
    }

    aml_writeReg(unit, PIC(port), u32dat);

    return AIR_E_OK;
}

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
    BOOL_T          *ptr_enable)
{
    UI32_T u32dat = 0;

    aml_readReg(unit, PIC(port), &u32dat);
    if(u32dat == (PIC_PORT_IGMP_CTRL_CSR_IPM_01| PIC_PORT_IGMP_CTRL_CSR_IPM_33 | PIC_PORT_IGMP_CTRL_CSR_IPM_224))
    {
        *ptr_enable = TRUE;
    }else
    {
        *ptr_enable = FALSE;
    }

    return AIR_E_OK;
}

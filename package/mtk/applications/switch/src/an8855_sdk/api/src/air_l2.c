/* FILE NAME: air_l2.c
 * PURPOSE:
 *      Define the layer 2 function in AIR SDK.
 *
 * NOTES:
 *      None
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "air.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_L2_DELAY_US             (1000)
#define AIR_L2_WDOG_KICK_NUM        (1000)
#define AIR_L2_FORWARD_VALUE        (0xFFFFFFFF)
#define AIR_L2_AGING_MS_CONSTANT    (1024)
#define AIR_L2_AGING_1000MS         (1000)

typedef enum
{
    AIR_L2_EXEC_FWD_CTRL_DEFAULT =      0x0,
    AIR_L2_EXEC_FWD_CTRL_CPU_EXCLUDE =  0x4,
    AIR_L2_EXEC_FWD_CTRL_CPU_INCLUDE =  0x5,
    AIR_L2_EXEC_FWD_CTRL_CPU_ONLY =     0x6,
    AIR_L2_EXEC_FWD_CTRL_DROP =         0x7,
    AIR_L2_EXEC_FWD_CTRL_LAST
}AIR_L2_EXEC_FWD_CTRL_T;

typedef enum
{
    AIR_L2_MAC_MAT_MAC,
    AIR_L2_MAC_MAT_DYNAMIC_MAC,
    AIR_L2_MAC_MAT_STATIC_MAC,
    AIR_L2_MAC_MAT_MAC_BY_VID,
    AIR_L2_MAC_MAT_MAC_BY_FID,
    AIR_L2_MAC_MAT_MAC_BY_PORT,
    AIR_L2_MAC_MAT_MAC_BY_LAST
}AIR_L2_MAC_MAT_T;

/* L2 MAC table multi-searching */
typedef enum
{
    AIR_L2_MAC_MS_START,   /* Start search */
    AIR_L2_MAC_MS_NEXT,    /* Next search */
    AIR_L2_MAC_MS_LAST
}AIR_L2_MAC_MS_T;

typedef enum
{
    AIR_L2_MAC_TB_TY_MAC,
    AIR_L2_MAC_TB_TY_DIP,
    AIR_L2_MAC_TB_TY_DIP_SIP,
    AIR_L2_MAC_TB_TY_LAST
}AIR_L2_MAC_TB_TY_T;

/*
typedef enum
{
    AIR_MAC_MAT_MAC,
    AIR_MAC_MAT_DYNAMIC_MAC,
    AIR_MAC_MAT_STATIC_MAC,
    AIR_MAC_MAT_MAC_BY_VID,
    AIR_MAC_MAT_MAC_BY_FID,
    AIR_MAC_MAT_MAC_BY_PORT,
    AIR_MAC_MAT_MAC_BY_LAST
}AIR_MAC_MAT_T;
*/

/* MACRO FUNCTION DECLARATIONS
 */
#define AIR_L2_AGING_TIME(__cnt__, __unit__)   \
        (((__cnt__) + 1) * ((__unit__) + 1) * AIR_L2_AGING_MS_CONSTANT / AIR_L2_AGING_1000MS)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
static BOOL_T _search_end = FALSE;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T
_cmpMac(
    const UI8_T mac1[6],
    const UI8_T mac2[6]);

static AIR_ERROR_NO_T
_searchMacEntry(
    const UI32_T unit,
    const AIR_L2_MAC_MS_T ms,
    const AIR_L2_MAC_MAT_T multi_target,
    UI32_T *ptr_addr,
    UI32_T *ptr_bank);

static AIR_ERROR_NO_T
_checkL2Busy(
    const UI32_T unit);

static void
_fill_MAC_ATA(
    const UI32_T unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry);

static void
_fill_MAC_ATWD(
    const UI32_T            unit,
    const AIR_MAC_ENTRY_T   *ptr_mac_entry,
    const BOOL_T            valid);

static UI32_T
_checkL2EntryHit(
    const UI32_T unit);

static void
_fill_MAC_ATRDS(
    const UI32_T unit,
    UI8_T bank);

static AIR_ERROR_NO_T
_read_MAC_ATRD(
    const UI32_T unit,
    AIR_MAC_ENTRY_T *ptr_mac_entry);

/* FUNCTION NAME: _cmpMac
 * PURPOSE:
 *      Compare MAC address to check whether those MAC is same.
 *
 * INPUT:
 *      mac1            --  1st MAC address
 *      mac2            --  2nd MAC address
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None
 */
static BOOL_T
_cmpMac(
    const UI8_T mac1[6],
    const UI8_T mac2[6])
{
    UI32_T i;
    for(i=0; i<6; i++)
    {
        if(mac1[i] != mac2[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}
/* FUNCTION NAME: _searchMacEntry
 * PURPOSE:
 *      Search MAC Address table.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ms              --  _HAL_SCO_L2_MAC_MS_START:           Start search command
 *                          _HAL_SCO_L2_MAC_MS_NEXT:            Next search command
 *      multi_target    --  _HAL_SCO_L2_MAC_MAT_MAC:            MAC address entries
 *                          _HAL_SCO_L2_MAC_MAT_DYNAMIC_MAC:    Dynamic MAC address entries
 *                          _HAL_SCO_L2_MAC_MAT_STATIC_MAC:     Static MAC address entries
 *                          _HAL_SCO_L2_MAC_MAT_MAC_BY_VID:     MAC address entries with specific CVID
 *                          _HAL_SCO_L2_MAC_MAT_MAC_BY_FID:     MAC address entries with specific FID
 *                          _HAL_SCO_L2_MAC_MAT_MAC_BY_PORT:    MAC address entries with specific port
 * OUTPUT:
 *      ptr_addr        --  MAC Table Access Index
 *      ptr_bank        --  Searching result in which bank
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_searchMacEntry(
    const UI32_T unit,
    const AIR_L2_MAC_MS_T ms,
    const AIR_L2_MAC_MAT_T multi_target,
    UI32_T *ptr_addr,
    UI32_T *ptr_bank)
{
    UI32_T u32dat = 0;

    u32dat = (ATC_SAT_MAC | ATC_START_BUSY);
    if (AIR_L2_MAC_MS_START == ms)
    {
        /* Start search 1st valid entry */
        u32dat |= ATC_CMD_SEARCH;
    }
    else if (AIR_L2_MAC_MS_NEXT == ms)
    {
        /* Search next valid entry */
        u32dat |= ATC_CMD_SEARCH_NEXT;
    }
    else
    {
        /* Unknown commnad */
        return AIR_E_BAD_PARAMETER;
    }

    switch(multi_target)
    {
        case AIR_L2_MAC_MAT_MAC:
            u32dat |= ATC_MAT_MAC;
            break;
        case AIR_L2_MAC_MAT_DYNAMIC_MAC:
            u32dat |= ATC_MAT_DYNAMIC_MAC;
            break;
        case AIR_L2_MAC_MAT_STATIC_MAC:
            u32dat |= ATC_MAT_STATIC_MAC;
            break;
        case AIR_L2_MAC_MAT_MAC_BY_VID:
            u32dat |= ATC_MAT_MAC_BY_VID;
            break;
        case AIR_L2_MAC_MAT_MAC_BY_FID:
            u32dat |= ATC_MAT_MAC_BY_FID;
            break;
        case AIR_L2_MAC_MAT_MAC_BY_PORT:
            u32dat |= ATC_MAT_MAC_BY_PORT;
            break;
        default:
            /* Unknown searching mode */
            return AIR_E_BAD_PARAMETER;
    }
    aml_writeReg(unit, ATC, u32dat);
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        return AIR_E_TIMEOUT;
    }

    aml_readReg(unit, ATC, &u32dat);
    /* Get address */
    (*ptr_addr) = BITS_OFF_R(u32dat, ATC_ADDR_OFFSET, ATC_ADDR_LENGTH);
    /* Get banks */
    (*ptr_bank) = BITS_OFF_R(u32dat, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
    if ((AIR_L2_MAX_SIZE - 1) == (*ptr_addr))
    {
        _search_end = TRUE;
    }
    else
    {
        _search_end = FALSE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: _checkL2Busy
 * PURPOSE:
 *      Check BUSY bit of ATC
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_checkL2Busy(
    const UI32_T unit)
{
    UI32_T i = 0;
    UI32_T reg_atc = 0;

    /* Check BUSY bit is 0 */
    for(i = 0; i < AIR_L2_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, ATC, &reg_atc);
        if(!BITS_OFF_R(reg_atc, ATC_BUSY_OFFSET, ATC_BUSY_LENGTH))
        {
            break;
        }
        AIR_UDELAY(AIR_L2_DELAY_US);
    }
    if(i >= AIR_L2_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }
    return AIR_E_OK;
}
/***************************************************************/
/* FUNCTION NAME: _fill_MAC_ATA
 * PURPOSE:
 *      Fill register ATA for MAC Address table.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  Structure of MAC Address table
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
static void
_fill_MAC_ATA(
    const UI32_T unit,
    const AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    UI32_T u32dat = 0;
    UI32_T i = 0;

    /* Fill ATA1 */
    for (i = 0; i < 4; i++)
    {
        u32dat |= ((UI32_T)(ptr_mac_entry->mac[i] & BITS(0,7))) << ( (3-i) * 8);
    }
    aml_writeReg(unit, ATA1, u32dat);
    AIR_UDELAY(AIR_L2_DELAY_US);

    /* Fill ATA2 */
    u32dat=0;
    for (i = 4; i < 6; i++)
    {
        u32dat |= ((UI32_T)(ptr_mac_entry->mac[i] & BITS(0,7))) << ( (7-i) * 8);
    }
    if (!(ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_STATIC))
    {
        /* type is dynamic */
        u32dat |= BITS_OFF_L(1UL, ATA2_MAC_LIFETIME_OFFSET, ATA2_MAC_LIFETIME_LENGTH);
        /* set aging counter as system aging conuter */
        u32dat |= BITS_OFF_L(ptr_mac_entry->timer, ATA2_MAC_AGETIME_OFFSET, ATA2_MAC_AGETIME_LENGTH);
    }
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_UNAUTH)
    {
        u32dat |= BITS_OFF_L(1UL, ATA2_MAC_UNAUTH_OFFSET, ATA2_MAC_UNAUTH_LENGTH);
    }

    aml_writeReg(unit, ATA2, u32dat);
    AIR_UDELAY(AIR_L2_DELAY_US);
}

/* FUNCTION NAME: _fill_MAC_ATWD
 * PURPOSE:
 *      Fill register ATWD for MAC Address table.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_entry   --  Structure of MAC Address table
 *      valid           --  TRUE
 *                          FALSE
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
static void
_fill_MAC_ATWD(
    const UI32_T            unit,
    const AIR_MAC_ENTRY_T   *ptr_mac_entry,
    const BOOL_T            valid)
{
    UI32_T u32dat = 0;
    UI32_T fwd_val = 0;

    u32dat = 0;
    /* Fill ATWD */
    /* set valid bit */
    if (TRUE == valid)
    {
        u32dat |= BITS_OFF_L(1UL, ATWD_MAC_LIVE_OFFSET, ATWD_MAC_LIVE_LENGTH);
    }

    /* set IVL */
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        u32dat |= BITS_OFF_L(1UL, ATWD_MAC_IVL_OFFSET, ATWD_MAC_IVL_LENGTH);
    }
    /* set VID */
    u32dat |= BITS_OFF_L(ptr_mac_entry->cvid, ATWD_MAC_VID_OFFSET, ATWD_MAC_VID_LENGTH);
    /* set FID */
    u32dat |= BITS_OFF_L(ptr_mac_entry->fid, ATWD_MAC_FID_OFFSET, ATWD_MAC_FID_LENGTH);

    /* Set forwarding control */
    switch (ptr_mac_entry->sa_fwd)
    {
        case AIR_L2_FWD_CTRL_DEFAULT:
            fwd_val = AIR_L2_EXEC_FWD_CTRL_DEFAULT;
            break;
        case AIR_L2_FWD_CTRL_CPU_INCLUDE:
            fwd_val = AIR_L2_EXEC_FWD_CTRL_CPU_INCLUDE;
            break;
        case AIR_L2_FWD_CTRL_CPU_EXCLUDE:
            fwd_val = AIR_L2_EXEC_FWD_CTRL_CPU_EXCLUDE;
            break;
        case AIR_L2_FWD_CTRL_CPU_ONLY:
            fwd_val = AIR_L2_EXEC_FWD_CTRL_CPU_ONLY;
            break;
        case AIR_L2_FWD_CTRL_DROP:
            fwd_val = AIR_L2_EXEC_FWD_CTRL_DROP;
            break;
        default:
            break;
    }
    u32dat |= BITS_OFF_L(fwd_val, ATWD_MAC_FWD_OFFSET, ATWD_MAC_FWD_LENGTH);
    aml_writeReg(unit, ATWD, u32dat);
    AIR_UDELAY(AIR_L2_DELAY_US);

    /* Fill ATWD2 */
    u32dat = BITS_OFF_L(ptr_mac_entry->port_bitmap[0], ATWD2_MAC_PORT_OFFSET, ATWD2_MAC_PORT_LENGTH);
    aml_writeReg(unit, ATWD2, u32dat);
    AIR_UDELAY(AIR_L2_DELAY_US);
}

/* FUNCTION NAME: _checkL2EntryHit
 * PURPOSE:
 *      Check entry hit of ATC
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The entry hit bitmap
 *
 * NOTES:
 *      None
 */
static UI32_T
_checkL2EntryHit(
    const UI32_T unit)
{
    UI32_T reg_atc = 0;
    aml_readReg(unit, ATC, &reg_atc);
    return BITS_OFF_R(reg_atc, ATC_ENTRY_HIT_OFFSET, ATC_ENTRY_HIT_LENGTH);
}

/* FUNCTION NAME: _fill_MAC_ATRDS
 * PURPOSE:
 *      Fill register ATRDS for select bank after ATC search L2 table.
 *
 * INPUT:
 *      unit            --  Device ID
 *      bank            --  Selected index of bank
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
static void
_fill_MAC_ATRDS(
    const UI32_T unit,
    UI8_T bank)
{
    UI32_T u32dat = 0;

    /* Fill ATRDS */
    u32dat = BITS_OFF_L(bank, ATRD0_MAC_SEL_OFFSET, ATRD0_MAC_SEL_LENGTH);
    aml_writeReg(unit, ATRDS, u32dat);
    AIR_UDELAY(AIR_L2_DELAY_US);
}

/* FUNCTION NAME: _read_MAC_ATRD
 * PURPOSE:
 *      Read register ATRD for MAC Address table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_mac_entry   --  Structure of MAC Address table
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_read_MAC_ATRD(
    const UI32_T unit,
    AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    UI32_T u32dat = 0;
    UI32_T i = 0;
    BOOL_T live = FALSE;
    UI32_T type = 0;
    UI32_T age_unit = 0;
    UI32_T age_cnt = 0;
    UI32_T sa_fwd = 0;

    /* Read ATRD0 */
    aml_readReg(unit, ATRD0, &u32dat);
    live = BITS_OFF_R(u32dat, ATRD0_MAC_LIVE_OFFSET, ATRD0_MAC_LIVE_LENGTH);
    type = BITS_OFF_R(u32dat, ATRD0_MAC_TYPE_OFFSET, ATRD0_MAC_TYPE_LENGTH);
    if (FALSE == live)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }
    if (AIR_L2_MAC_TB_TY_MAC != type)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }
    /* Clear table */
    memset(ptr_mac_entry, 0, sizeof(AIR_MAC_ENTRY_T));

    ptr_mac_entry->cvid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_MAC_VID_OFFSET, ATRD0_MAC_VID_LENGTH);
    ptr_mac_entry->fid = (UI16_T)BITS_OFF_R(u32dat, ATRD0_MAC_FID_OFFSET, ATRD0_MAC_FID_LENGTH);
    if (!!BITS_OFF_R(u32dat, ATRD0_MAC_LIFETIME_OFFSET, ATRD0_MAC_LIFETIME_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_STATIC;
    }
    if (!!BITS_OFF_R(u32dat, ATRD0_MAC_IVL_OFFSET, ATRD0_MAC_IVL_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_IVL;
    }
    if (!!BITS_OFF_R(u32dat, ATRD1_MAC_UNAUTH_OFFSET, ATRD1_MAC_UNAUTH_LENGTH))
    {
        ptr_mac_entry->flags |= AIR_L2_MAC_ENTRY_FLAGS_UNAUTH;
    }

    /* Get the L2 MAC aging unit */
    aml_readReg(unit, AAC, &u32dat);
    age_unit = BITS_OFF_R(u32dat, AAC_AGE_UNIT_OFFSET, AAC_AGE_UNIT_LENGTH);

    /* Read ATRD1 */
    aml_readReg(unit, ATRD1, &u32dat);
    for (i = 4; i < 6; i++)
    {
        ptr_mac_entry->mac[i] = BITS_OFF_R(u32dat, (7 - i)*8, 8);
    }
    /* Aging time */
    age_cnt = BITS_OFF_R(u32dat, ATRD1_MAC_AGETIME_OFFSET, ATRD1_MAC_AGETIME_LENGTH);
    ptr_mac_entry->timer = AIR_L2_AGING_TIME(age_cnt, age_unit);
    /* SA forwarding */
    sa_fwd = BITS_OFF_R(u32dat, ATRD1_MAC_FWD_OFFSET, ATRD1_MAC_FWD_LENGTH);
    switch (sa_fwd)
    {
        case AIR_L2_EXEC_FWD_CTRL_DEFAULT:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_DEFAULT;
            break;
        case AIR_L2_EXEC_FWD_CTRL_CPU_INCLUDE:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_CPU_INCLUDE;
            break;
        case AIR_L2_EXEC_FWD_CTRL_CPU_EXCLUDE:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_CPU_EXCLUDE;
            break;
        case AIR_L2_EXEC_FWD_CTRL_CPU_ONLY:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_CPU_ONLY;
            break;
        case AIR_L2_EXEC_FWD_CTRL_DROP:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_DROP;
            break;
        default:
            ptr_mac_entry->sa_fwd = AIR_L2_FWD_CTRL_DEFAULT;
            break;
    }

    /* Read ATRD2 */
    aml_readReg(unit, ATRD2, &u32dat);
    for (i = 0; i < 4; i++)
    {
        ptr_mac_entry->mac[i] = BITS_OFF_R(u32dat, (3 - i)*8, 8);
    }

    /* Read ATRD3 */
    aml_readReg(unit, ATRD3, &u32dat);
    ptr_mac_entry->port_bitmap[0] = BITS_OFF_R(u32dat, ATRD3_MAC_PORT_OFFSET, ATRD3_MAC_PORT_LENGTH);

    return AIR_E_OK;
}

static AIR_ERROR_NO_T _mac_entry_init(AIR_MAC_ENTRY_T   *ptr_mac_entry)
{
    AIR_CHECK_PTR(ptr_mac_entry);
    memset(ptr_mac_entry->mac, 0, sizeof(ptr_mac_entry->mac));
    ptr_mac_entry->cvid = 0;
    ptr_mac_entry->fid = 0;
    ptr_mac_entry->flags = 0;
    ptr_mac_entry->port_bitmap[0] = 0;
    ptr_mac_entry->sa_fwd = 0;
    ptr_mac_entry->timer = 0;
    return AIR_E_OK;
}
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
    const AIR_MAC_ENTRY_T   *ptr_mac_entry)
{
    UI32_T u32dat = 0;
    UI32_T reg_aac = 0;
    AIR_MAC_ENTRY_T set_mac_entry;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_mac_entry);
    AIR_PARAM_CHK(((ptr_mac_entry->port_bitmap[0] & AIR_ALL_PORT_BITMAP) == 0), AIR_E_BAD_PARAMETER);
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        AIR_PARAM_CHK(((ptr_mac_entry->cvid < 1) || (ptr_mac_entry->cvid > 4095)), AIR_E_BAD_PARAMETER);
    }
    else
    {
        AIR_PARAM_CHK(((ptr_mac_entry->fid > (AIR_STP_FID_NUMBER - 1))), AIR_E_BAD_PARAMETER);
    }
    _mac_entry_init(&set_mac_entry);
    /* Set the target MAC entry as setting entry no mater the hash addrees is existed or not */
    memcpy(&set_mac_entry, ptr_mac_entry, sizeof(AIR_MAC_ENTRY_T));
    /* Translate port bitmap type */
    /* set aging counter as system aging conuter */
    aml_readReg(unit, AAC, &reg_aac);
    set_mac_entry.timer = BITS_OFF_R(reg_aac, AAC_AGE_CNT_OFFSET, AAC_AGE_CNT_LENGTH);

    /* Fill MAC address entry */
    _fill_MAC_ATA(unit, &set_mac_entry);
    _fill_MAC_ATWD(unit, &set_mac_entry, TRUE);

    /* Write data by ATC */
    u32dat = (ATC_SAT_MAC | ATC_CMD_WRITE | ATC_START_BUSY);
    aml_writeReg(unit, ATC, u32dat);
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        return AIR_E_TIMEOUT;
    }
    if ( !_checkL2EntryHit(unit))
    {
        return AIR_E_TABLE_FULL;
    }
    return AIR_E_OK;
}

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
    const AIR_MAC_ENTRY_T   *ptr_mac_entry)
{
    UI32_T u32dat = 0;
    AIR_MAC_ENTRY_T del_mac_entry;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_mac_entry);
    //AIR_PARAM_CHK(((ptr_mac_entry->port_bitmap[0] & AIR_ALL_PORT_BITMAP) == 0), AIR_E_BAD_PARAMETER);
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        AIR_PARAM_CHK(((ptr_mac_entry->cvid < 1) || (ptr_mac_entry->cvid > 4095)), AIR_E_BAD_PARAMETER);
    }
    else
    {
        AIR_PARAM_CHK(((ptr_mac_entry->fid > (AIR_STP_FID_NUMBER - 1))), AIR_E_BAD_PARAMETER);
    }
    _mac_entry_init(&del_mac_entry);
    memcpy(&del_mac_entry, ptr_mac_entry, sizeof(AIR_MAC_ENTRY_T));

    /* Fill MAC address entry */
    _fill_MAC_ATA(unit, &del_mac_entry);
    _fill_MAC_ATWD(unit, &del_mac_entry, FALSE);

    /* Write data by ATC to delete entry */
    u32dat = (ATC_SAT_MAC | ATC_CMD_WRITE | ATC_START_BUSY);
    aml_writeReg(unit, ATC, u32dat);
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        return AIR_E_TIMEOUT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:   air_l2_getMacBucketSize
 * PURPOSE:
 *      Get the bucket size of one MAC address set when searching L2
 *      table.
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
air_l2_getMacBucketSize(
    const UI32_T    unit,
    UI32_T          *ptr_size)
{
    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_size);

    /* Access regiser */
    (*ptr_size) = AIR_L2_MAC_SET_NUM;

    return AIR_E_OK;
}


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
    AIR_MAC_ENTRY_T *ptr_mac_entry)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T i = 0;
    BOOL_T is_mac_empty = TRUE;
    BOOL_T found_target = FALSE;
    AIR_MAC_ENTRY_T mt_read;
    UI32_T addr = 0;
    UI32_T banks = 0;
    AIR_L2_MAC_MAT_T mat = 0;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_mac_entry);
    //AIR_PARAM_CHK(((ptr_mac_entry->port_bitmap[0] & AIR_ALL_PORT_BITMAP) == 0), AIR_E_BAD_PARAMETER);
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        AIR_PARAM_CHK(((ptr_mac_entry->cvid < 1) || (ptr_mac_entry->cvid > 4095)), AIR_E_BAD_PARAMETER);
    }
    else
    {
        AIR_PARAM_CHK(((ptr_mac_entry->fid > (AIR_STP_FID_NUMBER - 1))), AIR_E_BAD_PARAMETER);
    }
    _mac_entry_init(&mt_read);
    /* Check MAC Address field of input data */
    for (i = 0; i < 6; i++)
    {
        if (0 != ptr_mac_entry->mac[i])
        {
            is_mac_empty = FALSE;
            break;
        }
    }

    (*ptr_count) = 0;
    if (FALSE == is_mac_empty)
    {
        /* MAC address isn't empty, means to search a specific MAC entry */
        if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
        {
            mat = AIR_L2_MAC_MAT_MAC_BY_VID;
        }
        else
        {
            mat = AIR_L2_MAC_MAT_MAC_BY_FID;
        }
        _fill_MAC_ATA(unit, ptr_mac_entry);
        _fill_MAC_ATWD(unit, ptr_mac_entry, TRUE);

        rc = _searchMacEntry(unit, AIR_L2_MAC_MS_START, mat, &addr, &banks);

        while(AIR_E_OK == rc)
        {
            AIR_PRINT("banks=(%d)\n", banks);
            if (0 == banks)
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
            for (i = 0; i < AIR_L2_MAC_SET_NUM; i++)
            {
                if (!!BITS_OFF_R(banks, i, 1))
                {
                    /* Found a valid MAC entry */
                    /* Select bank */
                    _fill_MAC_ATRDS(unit, i);

                    /* Read MAC entry */
                    memset(&mt_read, 0, sizeof(AIR_MAC_ENTRY_T));
                    rc = _read_MAC_ATRD(unit, &mt_read);
                    if (AIR_E_OK != rc)
                    {
                        AIR_PRINT("rc=(%d)\n", rc);
                        continue;
                    }
                    if (TRUE == _cmpMac(ptr_mac_entry->mac, mt_read.mac))
                    {
                        /* The found MAC is the target, restore data and leave */
                        memcpy(ptr_mac_entry, &mt_read, sizeof(AIR_MAC_ENTRY_T));
                        /* Translate port bitmap type */
                        found_target = TRUE;
                        (*ptr_count)++;
                        break;
                    }
                }
            }

            if ( TRUE == found_target)
            {
                break;
            }

            /* The found MAC isn't the target, keep searching or leave
             * when found the last entry */
            if (TRUE == _search_end)
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
            else
            {
                rc = _searchMacEntry(unit, AIR_L2_MAC_MS_NEXT, mat, &addr, &banks);
            }
        }
        return rc;
    }
    else
    {
        /* MAC address is empty, means to search the 1st MAC entry */
        rc = _searchMacEntry(unit, AIR_L2_MAC_MS_START, AIR_L2_MAC_MAT_MAC, &addr, &banks);

        switch(rc)
        {
            case AIR_E_OK:
                /* Searching bank and read data */
                AIR_PRINT("banks=(%d)\n", banks);
                if (0 == banks)
                {
                    return AIR_E_ENTRY_NOT_FOUND;
                }
                for (i = 0; i < AIR_L2_MAC_SET_NUM; i++)
                {
                    if (!!BITS_OFF_R(banks, i, 1))
                    {
                        /* Found a valid MAC entry */
                        /* Select bank */
                        _fill_MAC_ATRDS(unit, i);

                        /* Read MAC entry */
                        memset(&mt_read, 0, sizeof(AIR_MAC_ENTRY_T));
                        rc = _read_MAC_ATRD(unit, &mt_read);
                        if (AIR_E_OK != rc)
                        {
                            AIR_PRINT("rc=(%d)\n", rc);
                            continue;
                        }
                        memcpy(&ptr_mac_entry[(*ptr_count)], &mt_read, sizeof(AIR_MAC_ENTRY_T));
                        /* Translate port bitmap type */
                        (*ptr_count)++;
                    }
                }
                return AIR_E_OK;
            case AIR_E_TIMEOUT:
                /* Searching over time */
                return AIR_E_TIMEOUT;
            default:
                AIR_PRINT("rc=(%d)\n", rc);
                return AIR_E_ENTRY_NOT_FOUND;
        }
    }
}

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
    AIR_MAC_ENTRY_T *ptr_mac_entry)

{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T i = 0;
    BOOL_T is_mac_empty = TRUE;
    BOOL_T found_target = FALSE;
    AIR_MAC_ENTRY_T mt_read;
    UI32_T addr = 0;
    UI32_T banks = 0;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_mac_entry);
    //AIR_PARAM_CHK(((ptr_mac_entry->port_bitmap[0] & AIR_ALL_PORT_BITMAP) == 0), AIR_E_BAD_PARAMETER);
    if (ptr_mac_entry->flags & AIR_L2_MAC_ENTRY_FLAGS_IVL)
    {
        AIR_PARAM_CHK(((ptr_mac_entry->cvid < 1) || (ptr_mac_entry->cvid > 4095)), AIR_E_BAD_PARAMETER);
    }
    else
    {
        AIR_PARAM_CHK(((ptr_mac_entry->fid > (AIR_STP_FID_NUMBER - 1))), AIR_E_BAD_PARAMETER);
    }
    _mac_entry_init(&mt_read);
    /* If found the lastest entry last time, we couldn't keep to search the next entry */
    if(TRUE == _search_end)
    {
        return AIR_E_ENTRY_NOT_FOUND;
    }

    /* Check MAC Address field of input data */
    for (i = 0; i < 6; i++)
    {
        if (0 != ptr_mac_entry->mac[i])
        {
            is_mac_empty = FALSE;
            break;
        }
    }
    (*ptr_count)=0;

    if (FALSE == is_mac_empty)
    {
        /* MAC address isn't empty, means to search the next entries of input MAC Address */
        /* Search the target MAC entry */
        _fill_MAC_ATA(unit, ptr_mac_entry);
        rc = _searchMacEntry(unit, AIR_L2_MAC_MS_START, AIR_L2_MAC_MAT_MAC, &addr, &banks);
        while(AIR_E_OK == rc)
        {
            AIR_PRINT("banks=(%d)\n", banks);
            if (0 == banks)
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
            for (i = 0; i < AIR_L2_MAC_SET_NUM; i++)
            {
                if (!!BITS_OFF_R(banks, i, 1))
                {
                    /* Found a valid MAC entry */
                    /* Select bank */
                    _fill_MAC_ATRDS(unit, i);

                    /* Read MAC entry */
                    memset(&mt_read, 0, sizeof(AIR_MAC_ENTRY_T));
                    rc = _read_MAC_ATRD(unit, &mt_read);
                    if (AIR_E_OK != rc)
                    {
                        AIR_PRINT("rc=(%d)\n", rc);
                        continue;
                    }
                    if (TRUE == _cmpMac(ptr_mac_entry->mac, mt_read.mac))
                    {
                        /* The found MAC is the target, restore data and leave */
                        found_target = TRUE;
                        break;
                    }
                }
            }

            if ( TRUE == found_target)
            {
                break;
            }

            /* The found MAC isn't the target, keep searching or leave
             * when found the last entry */
            if (TRUE == _search_end)
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
            else
            {
                rc = _searchMacEntry(unit, AIR_L2_MAC_MS_NEXT, AIR_L2_MAC_MAT_MAC, &addr, &banks);
            }
        }

        if ( FALSE == found_target )
        {
            /* Entry not bank */
            return AIR_E_ENTRY_NOT_FOUND;
        }
        else
        {
            /* Found the target MAC entry, and try to search the next address */
            rc = _searchMacEntry(unit, AIR_L2_MAC_MS_NEXT, AIR_L2_MAC_MAT_MAC, &addr, &banks);
            if (AIR_E_OK == rc)
            {
                AIR_PRINT("banks=(%d)\n", banks);
                if (0 == banks)
                {
                    return AIR_E_ENTRY_NOT_FOUND;
                }
                for (i = 0; i < AIR_L2_MAC_SET_NUM; i++)
                {
                    if (!!BITS_OFF_R(banks, i, 1))
                    {
                        /* Found a valid MAC entry */
                        /* Select bank */
                        _fill_MAC_ATRDS(unit, i);

                        /* Read MAC entry */
                        memset(&mt_read, 0, sizeof(AIR_MAC_ENTRY_T));
                        rc = _read_MAC_ATRD(unit, &mt_read);
                        if (AIR_E_OK != rc)
                        {
                            AIR_PRINT("rc=(%d)\n", rc);
                            continue;
                        }
                        memcpy(&ptr_mac_entry[(*ptr_count)], &mt_read, sizeof(AIR_MAC_ENTRY_T));
                        /* Translate port bitmap type */
                        (*ptr_count)++;
                    }
                }
                return AIR_E_OK;
            }
            else
            {
                AIR_PRINT("rc=(%d)\n", rc);
                return AIR_E_ENTRY_NOT_FOUND;
            }
        }
    }
    else
    {
        /* MAC address is empty, means to search next entry */
        rc = _searchMacEntry(unit, AIR_L2_MAC_MS_NEXT, AIR_L2_MAC_MAT_MAC, &addr, &banks);
        if (AIR_E_OK == rc)
        {
            AIR_PRINT("banks=(%d)\n", banks);
            if (0 == banks)
            {
                return AIR_E_ENTRY_NOT_FOUND;
            }
            for (i = 0; i < AIR_L2_MAC_SET_NUM; i++)
            {
                if (!!BITS_OFF_R(banks, i, 1))
                {
                    /* Found a valid MAC entry */
                    /* Select bank */
                    _fill_MAC_ATRDS(unit, i);

                    /* Read MAC entry */
                    memset(&mt_read, 0, sizeof(AIR_MAC_ENTRY_T));
                    rc = _read_MAC_ATRD(unit, &mt_read);
                    if (AIR_E_OK != rc)
                    {
                        AIR_PRINT("rc=(%d)\n", rc);
                        continue;
                    }
                    memcpy(&ptr_mac_entry[(*ptr_count)], &mt_read, sizeof(AIR_MAC_ENTRY_T));
                    /* Translate port bitmap type */
                    (*ptr_count)++;
                }
            }
            return AIR_E_OK;
        }
        else
        {
            AIR_PRINT("rc=(%d)\n", rc);
            return AIR_E_ENTRY_NOT_FOUND;
        }
    }
}

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
    const UI32_T unit)
{
    UI32_T u32dat = 0;

    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    /* Write data by ATC to clear all MAC address entries */
    u32dat = (ATC_SAT_MAC | ATC_CMD_CLEAN | ATC_START_BUSY);
    aml_writeReg(unit, ATC, u32dat);
    if (AIR_E_TIMEOUT == _checkL2Busy(unit))
    {
        return AIR_E_TIMEOUT;
    }

    return AIR_E_OK;
}

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
    const UI32_T    age_time)
{
    
    UI32_T u32dat = 0;
    UI32_T age_cnt = 0, age_unit = 0, age_value = 0;
    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((age_time > AIR_L2_MAC_MAX_AGE_OUT_TIME) || (age_time < 1)), AIR_E_BAD_PARAMETER);

    /* Read the old register value */
    aml_readReg(unit, AAC, &u32dat);

    u32dat &= ~ BITS_RANGE(AAC_AGE_UNIT_OFFSET, AAC_AGE_UNIT_LENGTH);
    u32dat &= ~ BITS_RANGE(AAC_AGE_CNT_OFFSET, AAC_AGE_CNT_LENGTH);

    /* Calcuate the aging count/unit */
    age_value = age_time * AIR_L2_AGING_1000MS / AIR_L2_AGING_MS_CONSTANT;
    age_unit = (age_value / BIT(AAC_AGE_CNT_LENGTH) + 1);
    age_cnt = (age_value / age_unit + 1);

    /* Write the new register value */
    u32dat |= BITS_OFF_L((age_unit - 1), AAC_AGE_UNIT_OFFSET, AAC_AGE_UNIT_LENGTH);
    u32dat |= BITS_OFF_L((age_cnt - 1), AAC_AGE_CNT_OFFSET, AAC_AGE_CNT_LENGTH);

    aml_writeReg(unit, AAC, u32dat);

    return AIR_E_OK;
}

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
    UI32_T          *ptr_age_time)
{
    UI32_T u32dat = 0;
    UI32_T age_cnt = 0, age_unit = 0;
    /* parameter sanity check */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_age_time);

    /* Read data from register */
    aml_readReg(unit, AAC, &u32dat);

    age_cnt = BITS_OFF_R(u32dat, AAC_AGE_CNT_OFFSET, AAC_AGE_CNT_LENGTH);
    age_unit = BITS_OFF_R(u32dat, AAC_AGE_UNIT_OFFSET, AAC_AGE_UNIT_LENGTH);
    (*ptr_age_time) = AIR_L2_AGING_TIME(age_cnt, age_unit);

    return AIR_E_OK;
}

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
    const BOOL_T state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != state) && (FALSE != state)), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, AGDIS, &u32dat);
    if (state)
    {
        u32dat &= ~BIT(port);
    }
    else
    {
        u32dat |= BIT(port);
    }
    aml_writeReg(unit, AGDIS, u32dat);

    return AIR_E_OK;
}

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
    UI32_T *ptr_state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for state checking */
    AIR_CHECK_PTR(ptr_state);

    /* Read data from register */
    aml_readReg(unit, AGDIS, &u32dat);

    (*ptr_state) = (u32dat & BIT(port)) ? TRUE : FALSE;

    return AIR_E_OK;
}


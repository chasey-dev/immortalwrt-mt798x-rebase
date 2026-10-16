/* FILE NAME:   air.h
 * PURPOSE:
 *      Define the initialization in AIR SDK.
 * NOTES:
 */

#ifndef AIR_H
#define AIR_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "air_types.h"
#include "air_error.h"
#include "air_reg.h"
#include "air_aml.h"
#include "air_port.h"
#include "air_vlan.h"
#include "air_lag.h"
#include "air_l2.h"
#include "air_stp.h"
#include "air_mirror.h"
#include "air_mib.h"
#include "air_diag.h"
#include "air_led.h"
#include "air_cmd.h"
#include "air_qos.h"
#include "air_switch.h"
#include "air_ver.h"
#include "air_sec.h"
#include "air_acl.h"
#include "air_sptag.h"
#include "air_ipmc.h"
#include "air_lpdet.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define AIR_CHECK_PTR(__ptr__) do                                          \
{                                                                           \
    if (NULL == (__ptr__))                                                  \
    {                                                                       \
        return  AIR_E_BAD_PARAMETER;                                       \
    }                                                                       \
} while (0)

#define AIR_PARAM_CHK(expr, errCode) do                                    \
{                                                                           \
    if ((I32_T)(expr))                                                      \
    {                                                                       \
        return errCode;                                                     \
    }                                                                       \
} while (0)

#define AIR_PRINT(fmt,...) do                                              \
{                                                                           \
    if (_ext_printf)                                                        \
    {                                                                       \
        _ext_printf(fmt, ##__VA_ARGS__);                                    \
    }                                                                       \
}while (0)

#define AIR_UDELAY(us) do                                                  \
{                                                                           \
    if (_ext_udelay)                                                        \
    {                                                                       \
        _ext_udelay(us);                                                    \
    }                                                                       \
}while (0)

#define AIR_MALLOC(size) (_ext_malloc ? _ext_malloc(size) : NULL)

#define AIR_FREE(ptr) do                                                   \
    {                                                                       \
        if (ptr && _ext_free)                                               \
        {                                                                   \
            _ext_free(ptr);                                                 \
        }                                                                   \
    }while (0)

#define AIR_IP_ADDR_IS_ZERO(__ip__)                                                            \
                        (__ip__.ipv4 == TRUE)?                                                        \
                        (__ip__.ip_addr.ipv4_addr == AIR_IPV4_ZERO):                                 \
                        (__ip__.ip_addr.ipv6_addr[0] == 0 && __ip__.ip_addr.ipv6_addr[1] == 0 &&      \
                         __ip__.ip_addr.ipv6_addr[2] == 0 && __ip__.ip_addr.ipv6_addr[3] == 0 &&      \
                         __ip__.ip_addr.ipv6_addr[4] == 0 && __ip__.ip_addr.ipv6_addr[5] == 0 &&      \
                         __ip__.ip_addr.ipv6_addr[6] == 0 && __ip__.ip_addr.ipv6_addr[7] == 0 &&      \
                         __ip__.ip_addr.ipv6_addr[8] == 0 && __ip__.ip_addr.ipv6_addr[9] == 0 &&      \
                         __ip__.ip_addr.ipv6_addr[10] == 0 && __ip__.ip_addr.ipv6_addr[11] == 0 &&    \
                         __ip__.ip_addr.ipv6_addr[12] == 0 && __ip__.ip_addr.ipv6_addr[13] == 0 &&    \
                         __ip__.ip_addr.ipv6_addr[14] == 0 && __ip__.ip_addr.ipv6_addr[15] == 0)

#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif	/* End of BIT */

/* bits range: for example BITS(16,23) = 0xFF0000*/
#ifndef BITS
#define BITS(m, n)   (~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))
#endif	/* End of BITS */

/* bits range: for example BITS_RANGE(16,4) = 0x0F0000*/
#ifndef BITS_RANGE
#define BITS_RANGE(offset, range)   BITS(offset, ((offset)+(range)-1))
#endif	/* End of BITS_RANGE */

/* bits offset right: for example BITS_OFF_R(0x1234, 8, 4) = 0x2 */
#ifndef BITS_OFF_R
#define BITS_OFF_R(val, offset, range)   ((val >> offset) & (BIT(range) - 1))
#endif	/* End of BITS_OFF_R */

/* bits offset left: for example BITS_OFF_L(0x1234, 8, 4) = 0x400 */
#ifndef BITS_OFF_L
#define BITS_OFF_L(val, offset, range)   ((val & (BIT(range) - 1)) << offset)
#endif	/* End of BITS_OFF_L */

#ifndef MAX
#define MAX(a, b)   (((a)>(b))?(a):(b))
#endif	/* End of MAX */

#ifndef MIN
#define MIN(a, b)   (((a)<(b))?(a):(b))
#endif	/* End of MIN */

/* DATA TYPE DECLARATIONS
 */
typedef int
(*AIR_PRINTF)(
    const C8_T* fmt,
    ...);

typedef int
(*AIR_UDELAY)(
    UI32_T us);

typedef void*
(*AIR_MALLOC)(
    long unsigned int size);

typedef void
(*AIR_FREE)(
    void *ptr);

typedef struct
{
    AML_DEV_ACCESS_T    dev_access;
    AIR_PRINTF         printf;
    AIR_UDELAY         udelay;
    AIR_MALLOC         malloc;
    AIR_FREE           free;
}AIR_INIT_PARAM_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
extern AIR_PRINTF _ext_printf;
extern AIR_UDELAY _ext_udelay;
extern AIR_MALLOC _ext_malloc;
extern AIR_FREE   _ext_free;


/* FUNCTION NAME:   air_init
 * PURPOSE:
 *      This API is used to initialize the SDK.
 *
 * INPUT:
 *      unit            --  The device unit
 *      ptr_init_param  --  The sdk callback functions.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init(
    const UI32_T unit,
    AIR_INIT_PARAM_T *ptr_init_param);

/* FUNCTION NAME:   air_hw_reset
 * PURPOSE:
 *      This API is used to reset hardware.
 *
 * INPUT:
 *      unit            --  The device unit
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_hw_reset(
    const UI32_T unit);

/* FUNCTION NAME:   air_set_gpio_pin_mux
 * PURPOSE:
 *      This API is used to set gpio pin mux.
 *
 * INPUT:
 *      unit            --  The device unit
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_set_gpio_pin_mux(
    const UI32_T unit);

#endif  /* AIR_H */


/* FILE NAME:   air_types.h
 * PURPOSE:
 *      Define the commom data type in AIR SDK.
 * NOTES:
 */

#ifndef AIR_TYPES_H
#define AIR_TYPES_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef NULL
#define NULL                (void *)0
#endif

#ifndef LOW
#define LOW                 0
#endif

#ifndef HIGH
#define HIGH                1
#endif

/* MACRO FUNCTION DECLARATIONS
 */

#define AIR_IPV4_ZERO      0

/* DATA TYPE DECLARATIONS
 */
typedef int                 BOOL_T;
typedef signed char         I8_T;
typedef unsigned char       UI8_T;
typedef signed short        I16_T;
typedef unsigned short      UI16_T;
typedef signed int          I32_T;
typedef unsigned int        UI32_T;
typedef char                C8_T;
typedef unsigned long long  UI64_T;

typedef UI8_T   AIR_MAC_T[6];
typedef UI32_T  AIR_IPV4_T;
typedef UI8_T   AIR_IPV6_T[16];

typedef union AIR_IP_U
{

    AIR_IPV4_T     ipv4_addr;
    AIR_IPV6_T     ipv6_addr;

}AIR_IP_T;

typedef struct AIR_IP_ADDR_S
{
   AIR_IP_T      ip_addr;
   BOOL_T        ipv4 ;
}AIR_IP_ADDR_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif  /* AIR_TYPES_H */


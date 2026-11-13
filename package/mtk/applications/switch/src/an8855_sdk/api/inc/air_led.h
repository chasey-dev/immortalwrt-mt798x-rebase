/* FILE NAME: air_led.h
 * PURPOSE:
 * 1. Define information for air_led.c
 * NOTES:
 */

#ifndef AIR_LED_H
#define AIR_LED_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/

#define MAX_NUM_LED_ENTITY          2
#define UNIT_LED_BLINK_DURATION     1024

typedef enum
{
    AIR_LED_MODE_DISABLE,      /* ALL LED outputs ard disabled */
    AIR_LED_MODE_2LED_MODE0,   /* 2 LED pins are enabled. Active high.
                                   LED 0: Link.
                                   LED 1: Activity. */
    AIR_LED_MODE_2LED_MODE1,   /* 2 LED pins are enabled. Active high.
                                   LED 0: Link 1000 Activity.
                                   LED 1: Link 100 Activity. */
    AIR_LED_MODE_2LED_MODE2,   /* 2 LED pins are enabled. Active high.
                                   LED 0: Link 1000 Activity.
                                   LED 1: Link 10/100 Activity. */
    AIR_LED_MODE_USER_DEFINE,  /* LED functions of each pin are user-defined */
    AIR_LED_MODE_LAST
}AIR_LED_MODE_T;

typedef enum
{
    AIR_LED_BLK_DUR_32M,           /* Blink duration: 32 ms */
    AIR_LED_BLK_DUR_64M,           /* Blink duration: 64 ms */
    AIR_LED_BLK_DUR_128M,          /* Blink duration: 128 ms */
    AIR_LED_BLK_DUR_256M,          /* Blink duration: 256 ms */
    AIR_LED_BLK_DUR_512M,          /* Blink duration: 512 ms */
    AIR_LED_BLK_DUR_1024M,         /* Blink duration: 1024 ms */
    AIR_LED_BLK_DUR_LAST
}AIR_LED_BLK_DUR_T;

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef struct AIR_LED_ON_EVT_S
{
    BOOL_T link_1000m;     /* Link 1000M */
    BOOL_T link_100m;      /* Link 100M */
    BOOL_T link_10m;       /* Link 10M */
    BOOL_T link_dn;        /* Link Down */
    BOOL_T fdx;            /* Full Duplex */
    BOOL_T hdx;            /* Half Duplex */
    BOOL_T force;          /* Force on (logic 1) */
}AIR_LED_ON_EVT_T;

typedef struct AIR_LED_BLK_EVT_S
{
    BOOL_T tx_act_1000m;  /* 1000Mbps TX Activity */
    BOOL_T rx_act_1000m;  /* 1000Mbps RX Activity */
    BOOL_T tx_act_100m;   /* 100Mbps TX Activity */
    BOOL_T rx_act_100m;   /* 100Mbps RX Activity */
    BOOL_T tx_act_10m;    /* 10Mbps TX Activity */
    BOOL_T rx_act_10m;    /* 10Mbps RX Activity */
    BOOL_T cls;           /* Collision */
    BOOL_T rx_crc;        /* Rx CRC Error */
    BOOL_T rx_idle;       /* Rx Idle Error */
    BOOL_T force;         /* Force blinks (logic 1) */
}AIR_LED_BLK_EVT_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: air_led_setMode
 * PURPOSE:
 *      Set the LED processing mode for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      mode            --  Setting mode of LED
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      The LED control register is shared with all port on AN8855.
 *      Setting LED on any one port will also set to each other ports.
 */
AIR_ERROR_NO_T
air_led_setMode(
    const UI32_T unit,
    const UI8_T port,
    const AIR_LED_MODE_T mode);

/* FUNCTION NAME:air_led_getMode
 * PURPOSE:
 *      Get the LED processing mode for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_mode        --  Setting mode of LED
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_led_getMode(
    const UI32_T unit,
    const UI8_T port,
    AIR_LED_MODE_T *ptr_mode);

/* FUNCTION NAME: air_led_setState
 * PURPOSE:
 *      Set the enable state for a specific LED.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      entity          --  Entity of LED
 *      state           --  TRUE: Enable
 *                          FALSE: Disable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      The LED control register is shared with all port on AN8855.
 *      Setting LED on any one port will also set to each other ports.
 */
AIR_ERROR_NO_T
air_led_setState(
    const UI32_T unit,
    const UI8_T port,
    const UI8_T entity,
    const BOOL_T state);

/* FUNCTION NAME: air_led_getState
 * PURPOSE:
 *      Get the enable state for a specific LED.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      entity          --  Entity of LED
 *
 * OUTPUT:
 *      ptr_state       --  TRUE: Enable
 *                          FALSE: Disable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_led_getState(
    const UI32_T unit,
    const UI8_T port,
    const UI8_T entity,
    BOOL_T *ptr_state);

/* FUNCTION NAME: air_led_setUsrDef
 * PURPOSE:
 *      Set the user-defined configuration of a speficic LED.
 *      It only work when air_led_setState() set to AIR_LED_MODE_USER_DEFINE.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      entity          --  Entity of LED
 *      polar           --  LOW: Active low
 *                          HIGH: Active high
 *      on_evt          --  AIR_LED_ON_EVT_T
 *                          LED turns on if any event is detected
 *      blk_evt         --  AIR_LED_BLK_EVT_T
 *                          LED blinks blink if any event is detected
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      The LED control register is shared with all port on AN8855.
 *      Setting LED on any one port will also set to each other ports.
 */
AIR_ERROR_NO_T
air_led_setUsrDef(
    const UI32_T unit,
    const UI8_T port,
    const UI8_T entity,
    const BOOL_T polar,
    const AIR_LED_ON_EVT_T on_evt,
    const AIR_LED_BLK_EVT_T blk_evt);

/* FUNCTION NAME: air_led_getUsrDef
 * PURPOSE:
 *      Get the user-defined configuration of a speficic LED.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      entity          --  Entity of LED
 * OUTPUT:
 *      ptr_polar       --  LOW: Active low
 *                          HIGH: Active high
 *      ptr_on_evt      --  AIR_LED_ON_EVT_T
 *                          LED turns on if any event is detected
 *      ptr_blk_evt     --  AIR_LED_BLK_EVT_T
 *                          LED blinks if any event is detected
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_led_getUsrDef(
    const UI32_T unit,
    const UI8_T port,
    const UI8_T entity,
    BOOL_T *ptr_polar,
    AIR_LED_ON_EVT_T *ptr_on_evt,
    AIR_LED_BLK_EVT_T *ptr_blk_evt);

/* FUNCTION NAME: air_led_setBlkTime
 * PURPOSE:
 *      Set the Blinking duration of a speficic LED.
 *      It only work when air_led_setState() set to AIR_LED_MODE_USER_DEFINE.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      dur             --  Blink duration
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      The LED control register is shared with all port on AN8855.
 *      Setting LED on any one port will also set to each other ports.
 */
AIR_ERROR_NO_T
air_led_setBlkTime(
    const UI32_T unit,
    const UI8_T port,
    const AIR_LED_BLK_DUR_T dur);

/* FUNCTION NAME: air_led_getBlkTime
 * PURPOSE:
 *      Get the Blinking duration of a speficic LED.
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_dur         --  Blink duration
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_led_getBlkTime(
    const UI32_T unit,
    const UI8_T port,
    AIR_LED_BLK_DUR_T *ptr_dur);

#endif /* End of AIR_LED_H */

/* FILE NAME: air_led.c
 * PURPOSE:
 *      Define the LED function in AIR SDK.
 *
 * NOTES:
 *      None
 */

/* INCLUDE FILE DECLARATIONS
*/
#include "air.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/
#define LED_SET_EVT(evt, reg, bit)          \
    do{                                     \
        if( TRUE == evt)                    \
        {                                   \
            reg |= bit;                     \
        }                                   \
    }while(0)

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
*/

/* STATIC VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
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
    const AIR_LED_MODE_T mode)
{
    UI32_T u32dat = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK( ( port >= AIR_MAX_NUM_OF_GIGA_PORTS ), AIR_E_BAD_PARAMETER );
    AIR_PARAM_CHK( ( mode >= AIR_LED_BLK_DUR_LAST ), AIR_E_BAD_PARAMETER );

    /* Read data from register */
    aml_readPhyRegCL45( unit, port, 0x1f, LED_BCR, &u32dat );

    /* Set LED mode */
    switch( mode )
    {
        case AIR_LED_MODE_DISABLE:
            u32dat &= ~LED_BCR_EXT_CTRL;
            u32dat &= ~LED_BCR_MODE_MASK;
            u32dat |= LED_BCR_MODE_DISABLE;
            break;
        case AIR_LED_MODE_2LED_MODE0:
            u32dat &= ~LED_BCR_EXT_CTRL;
            u32dat &= ~LED_BCR_MODE_MASK;
            u32dat |= LED_BCR_MODE_2LED;
            break;
        case AIR_LED_MODE_2LED_MODE1:
            u32dat &= ~LED_BCR_EXT_CTRL;
            u32dat &= ~LED_BCR_MODE_MASK;
            u32dat |= LED_BCR_MODE_3LED_1;
            break;
        case AIR_LED_MODE_2LED_MODE2:
            u32dat &= ~LED_BCR_EXT_CTRL;
            u32dat &= ~LED_BCR_MODE_MASK;
            u32dat |= LED_BCR_MODE_3LED_2;
            break;
        case AIR_LED_MODE_USER_DEFINE:
            u32dat |= LED_BCR_EXT_CTRL;
            break;
    }

    /* Write data to register */
    aml_writePhyRegCL45( unit, port, 0x1f, LED_BCR, u32dat );

    return AIR_E_OK;
}

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
    AIR_LED_MODE_T *ptr_mode)
{
    UI32_T u32dat = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK( ( port >= AIR_MAX_NUM_OF_GIGA_PORTS ), AIR_E_BAD_PARAMETER );
    AIR_CHECK_PTR( ptr_mode );

    /* Read data from register */
    aml_readPhyRegCL45( unit, port, 0x1f, LED_BCR, &u32dat );

    /* Get LED mode */
    if( LED_BCR_EXT_CTRL & u32dat )
    {
        (*ptr_mode ) = AIR_LED_MODE_USER_DEFINE;
    }
    else
    {
        switch( u32dat & LED_BCR_MODE_MASK )
        {
            case LED_BCR_MODE_DISABLE:
                (*ptr_mode ) = AIR_LED_MODE_DISABLE;
                break;
            case LED_BCR_MODE_2LED:
                (*ptr_mode ) = AIR_LED_MODE_2LED_MODE0;
                break;
            case LED_BCR_MODE_3LED_1:
                (*ptr_mode ) = AIR_LED_MODE_2LED_MODE1;
                break;
            case LED_BCR_MODE_3LED_2:
                (*ptr_mode ) = AIR_LED_MODE_2LED_MODE2;
                break;
            default:
                return AIR_E_OTHERS;
        }
    }

    return AIR_E_OK;
}

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
    const BOOL_T state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK( ( port >= AIR_MAX_NUM_OF_GIGA_PORTS ), AIR_E_BAD_PARAMETER );
    AIR_PARAM_CHK( ( entity >= MAX_NUM_LED_ENTITY ), AIR_E_BAD_PARAMETER );
    AIR_PARAM_CHK( ( ( TRUE != state ) && ( FALSE != state ) ), AIR_E_BAD_PARAMETER );

    /* Read data from register */
    aml_readPhyRegCL45( unit, port, 0x1f, LED_ON_CTRL(entity), &u32dat );

    /* Set LED state */
    if( TRUE == state)
    {
        u32dat |= LED_ON_EN;
    }
    else
    {
        u32dat &= ~LED_ON_EN;
    }

    /* Write data to register */
    aml_writePhyRegCL45( unit, port, 0x1f, LED_ON_CTRL(entity), u32dat );

    return AIR_E_OK;
}

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
    BOOL_T *ptr_state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK( ( port >= AIR_MAX_NUM_OF_GIGA_PORTS ), AIR_E_BAD_PARAMETER );
    AIR_PARAM_CHK( ( entity >= MAX_NUM_LED_ENTITY ), AIR_E_BAD_PARAMETER );
    AIR_CHECK_PTR( ptr_state );

    /* Read data from register */
    aml_readPhyRegCL45( unit, port, 0x1f, LED_ON_CTRL(entity), &u32dat );

    /* Get LED state */
    (*ptr_state) = ( LED_ON_EN & u32dat )?TRUE:FALSE;

    return AIR_E_OK;
}

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
    const AIR_LED_BLK_EVT_T blk_evt)
{
    UI32_T on_reg = 0;
    UI32_T blk_reg = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK( ( port >= AIR_MAX_NUM_OF_GIGA_PORTS ), AIR_E_BAD_PARAMETER );
    AIR_PARAM_CHK( ( entity >= MAX_NUM_LED_ENTITY ), AIR_E_BAD_PARAMETER );
    AIR_PARAM_CHK( ( ( HIGH != polar ) && ( LOW != polar ) ), AIR_E_BAD_PARAMETER );

    /* Read data from register */
    aml_readPhyRegCL45( unit, port, 0x1f, LED_ON_CTRL(entity), &on_reg );
    aml_readPhyRegCL45( unit, port, 0x1f, LED_BLK_CTRL(entity), &blk_reg );

    /* Set LED polarity */
    if( HIGH == polar)
    {
        on_reg |= LED_ON_POL;
    }
    else
    {
        on_reg &= ~LED_ON_POL;
    }

    /* Set LED On Event */
    on_reg &= ~LED_ON_EVT_MASK;
    LED_SET_EVT(on_evt.link_1000m, on_reg, LED_ON_EVT_LINK_1000M);
    LED_SET_EVT(on_evt.link_100m, on_reg, LED_ON_EVT_LINK_100M);
    LED_SET_EVT(on_evt.link_10m, on_reg, LED_ON_EVT_LINK_10M);
    LED_SET_EVT(on_evt.link_dn, on_reg, LED_ON_EVT_LINK_DN);
    LED_SET_EVT(on_evt.fdx, on_reg, LED_ON_EVT_FDX);
    LED_SET_EVT(on_evt.hdx, on_reg, LED_ON_EVT_HDX);
    LED_SET_EVT(on_evt.force, on_reg, LED_ON_EVT_FORCE);

    /* Set LED Blinking Event */
    blk_reg &= ~LED_BLK_EVT_MASK;
    LED_SET_EVT(blk_evt.tx_act_1000m, blk_reg, LED_BLK_EVT_1000M_TX_ACT);
    LED_SET_EVT(blk_evt.rx_act_1000m, blk_reg, LED_BLK_EVT_1000M_RX_ACT);
    LED_SET_EVT(blk_evt.tx_act_100m, blk_reg, LED_BLK_EVT_100M_TX_ACT);
    LED_SET_EVT(blk_evt.rx_act_100m, blk_reg, LED_BLK_EVT_100M_RX_ACT);
    LED_SET_EVT(blk_evt.tx_act_10m, blk_reg, LED_BLK_EVT_10M_TX_ACT);
    LED_SET_EVT(blk_evt.rx_act_10m, blk_reg, LED_BLK_EVT_10M_RX_ACT);
    LED_SET_EVT(blk_evt.cls, blk_reg, LED_BLK_EVT_CLS);
    LED_SET_EVT(blk_evt.rx_crc, blk_reg, LED_BLK_EVT_RX_CRC);
    LED_SET_EVT(blk_evt.rx_idle, blk_reg, LED_BLK_EVT_RX_IDL);
    LED_SET_EVT(blk_evt.force, blk_reg, LED_BLK_EVT_FORCE);

    /* Write data to register */
    aml_writePhyRegCL45( unit, port, 0x1f, LED_ON_CTRL(entity), on_reg );
    aml_writePhyRegCL45( unit, port, 0x1f, LED_BLK_CTRL(entity), blk_reg );

    return AIR_E_OK;
}

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
    AIR_LED_BLK_EVT_T *ptr_blk_evt)
{
    UI32_T on_reg = 0;
    UI32_T blk_reg = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK( ( port >= AIR_MAX_NUM_OF_GIGA_PORTS ), AIR_E_BAD_PARAMETER );
    AIR_PARAM_CHK( ( entity >= MAX_NUM_LED_ENTITY ), AIR_E_BAD_PARAMETER );
    AIR_CHECK_PTR( ptr_polar );
    AIR_CHECK_PTR( ptr_on_evt );
    AIR_CHECK_PTR( ptr_blk_evt );

    /* Read data from register */
    aml_readPhyRegCL45( unit, port, 0x1f, LED_ON_CTRL(entity), &on_reg );
    aml_readPhyRegCL45( unit, port, 0x1f, LED_BLK_CTRL(entity), &blk_reg );

    /* Get LED polarity */
    (*ptr_polar) = ( on_reg & LED_ON_POL)?TRUE:FALSE;

    /* Get LED On Event */
    ptr_on_evt ->link_1000m = (on_reg & LED_ON_EVT_LINK_1000M)?TRUE:FALSE;
    ptr_on_evt ->link_100m = (on_reg & LED_ON_EVT_LINK_100M)?TRUE:FALSE;
    ptr_on_evt ->link_10m = (on_reg & LED_ON_EVT_LINK_10M)?TRUE:FALSE;
    ptr_on_evt ->link_dn = (on_reg & LED_ON_EVT_LINK_DN)?TRUE:FALSE;
    ptr_on_evt ->fdx = (on_reg & LED_ON_EVT_FDX)?TRUE:FALSE;
    ptr_on_evt ->hdx = (on_reg & LED_ON_EVT_HDX)?TRUE:FALSE;
    ptr_on_evt ->force = (on_reg & LED_ON_EVT_FORCE)?TRUE:FALSE;

    /* Set LED Blinking Event */
    ptr_blk_evt ->tx_act_1000m = (blk_reg & LED_BLK_EVT_1000M_TX_ACT)?TRUE:FALSE;
    ptr_blk_evt ->rx_act_1000m = (blk_reg & LED_BLK_EVT_1000M_RX_ACT)?TRUE:FALSE;
    ptr_blk_evt ->tx_act_100m = (blk_reg & LED_BLK_EVT_100M_TX_ACT)?TRUE:FALSE;
    ptr_blk_evt ->rx_act_100m = (blk_reg & LED_BLK_EVT_100M_RX_ACT)?TRUE:FALSE;
    ptr_blk_evt ->tx_act_10m = (blk_reg & LED_BLK_EVT_10M_TX_ACT)?TRUE:FALSE;
    ptr_blk_evt ->rx_act_10m = (blk_reg & LED_BLK_EVT_10M_RX_ACT)?TRUE:FALSE;
    ptr_blk_evt ->cls = (blk_reg & LED_BLK_EVT_CLS)?TRUE:FALSE;
    ptr_blk_evt ->rx_crc = (blk_reg & LED_BLK_EVT_RX_CRC)?TRUE:FALSE;
    ptr_blk_evt ->rx_idle = (blk_reg & LED_BLK_EVT_RX_IDL)?TRUE:FALSE;
    ptr_blk_evt ->force = (blk_reg & LED_BLK_EVT_FORCE)?TRUE:FALSE;

    return AIR_E_OK;
}

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
    const AIR_LED_BLK_DUR_T dur)
{
    UI32_T on_dur = 0;
    UI32_T blk_dur = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK( ( port >= AIR_MAX_NUM_OF_GIGA_PORTS ), AIR_E_BAD_PARAMETER );
    AIR_PARAM_CHK( ( dur >= AIR_LED_BLK_DUR_LAST ), AIR_E_BAD_PARAMETER );

    /* Read data from register */
    aml_readPhyRegCL45( unit, port, 0x1f, LED_ON_DUR, &on_dur );
    aml_readPhyRegCL45( unit, port, 0x1f, LED_BLK_DUR, &blk_dur );

    /* Set LED Blinking duration */
    /* Setting unit = 32ms, register unit = 32.768 us */
    blk_dur = UNIT_LED_BLINK_DURATION << dur;
    /* On duration should be half of blinking duration */
    on_dur  = blk_dur >> 1;

    /* Write data to register */
    aml_writePhyRegCL45( unit, port, 0x1f, LED_ON_DUR, on_dur );
    aml_writePhyRegCL45( unit, port, 0x1f, LED_BLK_DUR, blk_dur );

    return AIR_E_OK;
}

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
    AIR_LED_BLK_DUR_T *ptr_dur)
{
    UI32_T blk_dur = 0;
    UI32_T u32dat = 0;
    I8_T i = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK( ( port >= AIR_MAX_NUM_OF_GIGA_PORTS ), AIR_E_BAD_PARAMETER );
    AIR_CHECK_PTR( ptr_dur );

    /* Read data from register */
    aml_readPhyRegCL45( unit, port, 0x1f, LED_BLK_DUR, &blk_dur );

    /* Get LED Blinking duration */
    u32dat = blk_dur / UNIT_LED_BLINK_DURATION;
    for(i = AIR_LED_BLK_DUR_LAST; i>=0; i--)
    {
        if( (u32dat >> i) & 0x1 )
        {
            break;
        }
    }
    (*ptr_dur) = i;

    return AIR_E_OK;
}


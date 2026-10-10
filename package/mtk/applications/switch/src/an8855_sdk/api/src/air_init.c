/* FILE NAME:   air_init.c
 * PURPOSE:
 *      Define the initialization function in AIR SDK.
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

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
AIR_PRINTF _ext_printf;
AIR_UDELAY _ext_udelay;
AIR_MALLOC _ext_malloc;
AIR_FREE   _ext_free;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
 */

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
    AIR_INIT_PARAM_T *ptr_init_param)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T u32dat = 0;
    UI8_T port = 0;
    AIR_LED_ON_EVT_T on_evt;
    AIR_LED_BLK_EVT_T blk_evt;

    /* check point */
    AIR_CHECK_PTR(ptr_init_param);

    _ext_dev_access.read_callback = ptr_init_param->dev_access.read_callback;
    _ext_dev_access.write_callback = ptr_init_param->dev_access.write_callback;
    _ext_dev_access.phy_read_callback = ptr_init_param->dev_access.phy_read_callback;
    _ext_dev_access.phy_write_callback = ptr_init_param->dev_access.phy_write_callback;
    _ext_dev_access.phy_cl45_read_callback = ptr_init_param->dev_access.phy_cl45_read_callback;
    _ext_dev_access.phy_cl45_write_callback = ptr_init_param->dev_access.phy_cl45_write_callback;
    _ext_printf = ptr_init_param->printf;
    _ext_udelay = ptr_init_param->udelay;
    _ext_malloc = ptr_init_param->malloc;
    _ext_free   = ptr_init_param->free;
    
    return rc;
}

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
    const UI32_T unit)
{
    AIR_PRINT(">>>>> enct_hw_reset\n");
    /* Set an8855 reset pin to 0 */

    /* Delay 100ms */

    /* Set an8855 reset pin to 1 */

    /* Delay 600ms */

    return AIR_E_OK;
}

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
    const UI32_T unit)
{
    AIR_PRINT(">>>>> enct_set_gpio_pin_mux\n");
    /* Set GPIO_MODE0 */
    /* Implementation for SLT HW */
    aml_writeReg(unit, GPIO_MODE0, 0x11111111);

    return AIR_E_OK;
}

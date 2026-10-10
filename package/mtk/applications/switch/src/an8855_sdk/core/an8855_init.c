/* FILE NAME:  an8855_init.c
 * PURPOSE:
 *    It provides an8855 switch intialize flow.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "an8855_reg.h"
#include "an8855_mdio.h"
#include "an8855_phy.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
*/

/* STATIC VARIABLE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
*/

/* FUNCTION NAME:   an8855_hw_reset
 * PURPOSE:
 *      This API is used to reset an8855 hw.
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *      Attention!! Customer should implement this func
 */
void
an8855_hw_reset(void)
{
    //dbg_print(">>>>> an8855_hw_reset\n");
    /* set an8855 reset pin to 0 */

    /* delay 100ms */

    /* set an8855 reset pin to 1 */

    /* delay 600ms */

}

/* FUNCTION NAME:   an8855_sw_reset
 * PURPOSE:
 *      This API is used to reset an8855 system.
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 */
void
an8855_sw_reset(void)
{
    //dbg_print(">>>>> an8855_sw_reset\n");
    an8855_reg_write(0x100050c0, 0x80000000);
    an8855_udelay(100000);
}

/* FUNCTION NAME:   an8855_phy_calibration_setting
 * PURPOSE:
 *      This API is used to set an8855 phy calibration.
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *      None
 */
void
an8855_phy_calibration_setting(void)
{
    int i = 0;

    //dbg_print("\nSMI IOMUX initial ...");
    an8855_reg_write(0x10000070, 0x2);
    an8855_udelay(10000);
    //dbg_print("\nGPHY initial ...");
    an8855_reg_write(0x1028C840, 0x0);
    for(i = 0; i <= 4; i++)
    {
        an8855_phy_write(i, 0, 0x1040);
    }
    an8855_udelay(10000);
    //dbg_print("Done");
    //dbg_print("\nSw calibration ... ");
    gphy_calibration(g_smi_addr);
    //dbg_print("\nDone");
}

/* FUNCTION NAME:   an8855_init
 * PURPOSE:
 *      This API is used to init an8855.
 * INPUT:
 * OUTPUT:
 * RETURN:
 *      0 -- init success
 *      -1 -- init failure
 * NOTES:
 *      Attention!! Customer should implement part of this func
 */
int
an8855_init(void)
{
    u32 data = 0;

    /* an8855 hw reset */
    an8855_hw_reset();

    /* an8855 system reset */
    an8855_sw_reset();

    /* Keep the clock ticking when all ports link down */
    data = an8855_reg_read(0x10213e1c);
    data &= ~(0x3);
    an8855_reg_write(0x10213e1c, data);

    /* internal phy calibration */
    /* please comment out this func after calibration data loaded from ROM code */
    an8855_phy_calibration_setting();

    return 0;
}


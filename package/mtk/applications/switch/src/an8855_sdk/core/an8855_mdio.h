/* FILE NAME:  an8855_mdio.h
 * PURPOSE:
 *      It provides AN8855 mdio access API.
 * NOTES:
 *
 */

#ifndef AN8855_MDIO_H
#define AN8855_MDIO_H

/* INCLUDE FILE DECLARATIONS
 */
//#include "CTP_type.h"
//#include "CTP_shell.h"
//#include "common.h"
//#include "eth.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
/* Attention!! Customer should define udelay function */
void delayUs(int usecond);
#define an8855_udelay(us) delayUs(us)

/* Attention!! Customer should define dbg_print to get dbg output */
#ifndef dbg_print
#define dbg_print(...)
#endif

#define AN8855_PHY_NUM                      5

/* DATA TYPE DECLARATIONS
 */
#ifndef NULL
#define NULL 0L
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef u8
#define u8 unsigned char
#endif

typedef u32 (*AIR_MII_READ_FUNC_T) (u32 phy_addr, u32 reg, u32 *p_data);

typedef u32 (*AIR_MII_WRITE_FUNC_T) (u32 phy_addr, u32 reg, u32 data);

typedef u32 (*AIR_MII_C45_READ_FUNC_T) (u32 phy_addr, u32 dev, u32 reg, u32 *p_data);

typedef u32 (*AIR_MII_C45_WRITE_FUNC_T) (u32 phy_addr, u32 dev, u32 reg, u32 data);

extern u32 g_smi_addr;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   an8855_set_smi_addr
 * PURPOSE:
 *      This API is used to set an8855 smi address.
 * INPUT:
 *      smi_addr -- AN8855 smi address
 * OUTPUT:
 * RETURN:
 * NOTES:
 *      None
 */
void
an8855_set_smi_addr(u32 smi_addr);

/* FUNCTION NAME:   an8855_set_mii_callback
 * PURPOSE:
 *      This API is used to set an8855 mii access callbacks.
 * INPUT:
 *      mii_read -- mii read api function
 *      mii_write -- mii write api function
 * OUTPUT:
 * RETURN:
 *      0     -- Successfully set callback.
 *      -1    -- Setting callback failed.
 * NOTES:
 *      None
 */
int
an8855_set_mii_callback(
    AIR_MII_READ_FUNC_T mii_read, 
    AIR_MII_WRITE_FUNC_T mii_write,
    AIR_MII_C45_READ_FUNC_T mii_c45_read, 
    AIR_MII_C45_WRITE_FUNC_T mii_c45_write);

/* FUNCTION NAME:   an8855_reg_read
 * PURPOSE:
 *      This API is used read an8855 registers.
 * INPUT:
 *      reg -- register offset
 * OUTPUT:
 * RETURN:
 *      Register value
 * NOTES:
 *      Attention!! Customer should implement mdio mutex
 *      lock in this func
 */
u32
an8855_reg_read(u32 reg);

/* FUNCTION NAME:   an8855_reg_write
 * PURPOSE:
 *      This API is used write an8855 registers.
 * INPUT:
 *      reg -- register offset
 *      val -- register value
 * OUTPUT:
 * RETURN:
 * NOTES:
 *      Attention!! Customer should implement mdio mutex
 *      lock in this func
 */
void
an8855_reg_write(u32 reg, u32 val);

/* FUNCTION NAME:   an8855_phy_read
 * PURPOSE:
 *      This API is used read an8855 phy registers.
 * INPUT:
 *      port_num -- port number, 0~4
 *      reg -- phy register offset
 * OUTPUT:
 *      p_val -- phy register value
 * RETURN:
 *      0 -- read success
 *      -1 -- read failure
 * NOTES:
 *      Attention!! Customer should implement mii mutex
 *      lock in this func
 */
int
an8855_phy_read(u32 port_num, u32 reg, u32 *p_val);

/* FUNCTION NAME:   an8855_phy_write
 * PURPOSE:
 *      This API is used write an8855 phy registers.
 * INPUT:
 *      port_num -- port number, 0~4
 *      reg -- phy register offset
 *      val -- phy register value
 * OUTPUT:
 * RETURN:
 *      0 -- write success
 *      -1 -- write failure
 * NOTES:
 *      Attention!! Customer should implement mii mutex
 *      lock in this func
 */
int
an8855_phy_write(u32 port_num, u32 reg, u32 val);

/* FUNCTION NAME:   an8855_phy_read_cl45
 * PURPOSE:
 *      This API is used read an8855 phy registers.
 * INPUT:
 *      port_num -- port number, 0~4
 *      dev_addr -- phy device type
 *      reg_addr -- phy register offset
 * OUTPUT:
 *      p_val -- phy register value
 * RETURN:
 *      0 -- read success
 *      -1 -- read failure
 * NOTES:
 *      Attention!! Customer should implement mii mutex
 *      lock in this func or before/after calling this func
 */
u32
an8855_phy_read_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 *p_val);

/* FUNCTION NAME:   an8855_phy_write_cl45
 * PURPOSE:
 *      This API is used write an8855 phy registers.
 * INPUT:
 *      port_num -- port number, 0~4
 *      dev_addr -- phy device type
 *      reg_addr -- phy register offset
 *      val -- phy register value
 * OUTPUT:
 * RETURN:
 *      0 -- write success
 *      -1 -- write failure
 * NOTES:
 *      Attention!! Customer should implement mii mutex
 *      lock in this func or before/after calling this func
 */
int
an8855_phy_write_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 val);

#endif  /* End of AN8855_MDIO_H */


/* FILE NAME:  air_aml.c
 * PURPOSE:
 *      It provides access management layer function.
 * NOTES:
 *
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
AML_DEV_ACCESS_T _ext_dev_access;

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   aml_readReg
 * PURPOSE:
 *      To read data from the register of the specified chip unit.
 * INPUT:
 *      unit        -- the device unit
 *      addr_offset -- the address of register
 * OUTPUT:
 *      ptr_data    -- pointer for the register data
 * RETURN:
 *      NPS_E_OK     -- Successfully read the data.
 *      NPS_E_OTHERS -- Failed to read the data.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_readReg(
    const UI32_T    unit,
    const UI32_T    addr_offset,
    UI32_T          *ptr_data)
{
    AIR_CHECK_PTR(ptr_data);

    if (!_ext_dev_access.read_callback)
    {
        return AIR_E_OTHERS;
    }

    return _ext_dev_access.read_callback(unit, addr_offset, ptr_data);
}

/* FUNCTION NAME:   aml_writeReg
 * PURPOSE:
 *      To write data to the register of the specified chip unit.
 * INPUT:
 *      unit        -- the device unit
 *      addr_offset -- the address of register
 *      data        -- written data
 * OUTPUT:
 *      none
 * RETURN:
 *      NPS_E_OK     -- Successfully write the data.
 *      NPS_E_OTHERS -- Failed to write the data.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_writeReg(
    const UI32_T    unit,
    const UI32_T    addr_offset,
    const UI32_T    data)
{
    if (!_ext_dev_access.write_callback)
    {
        return AIR_E_OTHERS;
    }

    return _ext_dev_access.write_callback(unit, addr_offset, data);
}

/* FUNCTION NAME:   aml_readPhyReg
 * PURPOSE:
 *      To read data from the phy register of the specified chip unit in Clause22.
 * INPUT:
 *      unit        -- the device unit
 *      port_id     -- physical port number
 *      addr_offset -- the address of phy register
 * OUTPUT:
 *      ptr_data    -- pointer for the register data
 * RETURN:
 *      NPS_E_OK     -- Successfully read the data.
 *      NPS_E_OTHERS -- Failed to read the data.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_readPhyReg(
    const UI32_T    unit,
    const UI32_T    port_id,
    const UI32_T    addr_offset,
    UI32_T          *ptr_data)
{
    AIR_CHECK_PTR(ptr_data);

    if (!_ext_dev_access.phy_read_callback)
    {
        return AIR_E_OTHERS;
    }

    return _ext_dev_access.phy_read_callback(unit, port_id, addr_offset, ptr_data);
}

/* FUNCTION NAME:   aml_writePhyReg
 * PURPOSE:
 *      To write data to the phy register of the specified chip unit in Clause22.
 * INPUT:
 *      unit        -- the device unit
 *      port_id     -- physical port number
 *      addr_offset -- the address of phy register
 *      data        -- written data
 * OUTPUT:
 *      none
 * RETURN:
 *      NPS_E_OK     -- Successfully write the data.
 *      NPS_E_OTHERS -- Failed to write the data.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_writePhyReg(
    const UI32_T    unit,
    const UI32_T    port_id,
    const UI32_T    addr_offset,
    const UI32_T    data)
{
    if (!_ext_dev_access.phy_write_callback)
    {
        return AIR_E_OTHERS;
    }

    return _ext_dev_access.phy_write_callback(unit, port_id, addr_offset, data);
}

/* FUNCTION NAME:   aml_readPhyRegCL45
 * PURPOSE:
 *      To read data from the phy register of the specified chip unit in Clause45.
 * INPUT:
 *      unit        -- the device unit
 *      port_id     -- physical port number
 *      dev_type    -- phy register type
 *      addr_offset -- the address of phy register
 * OUTPUT:
 *      ptr_data    -- pointer for the register data
 * RETURN:
 *      NPS_E_OK     -- Successfully read the data.
 *      NPS_E_OTHERS -- Failed to read the data.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_readPhyRegCL45(
    const UI32_T    unit,
    const UI32_T    port_id,
    const UI32_T    dev_type,
    const UI32_T    addr_offset,
    UI32_T          *ptr_data)
{
    AIR_CHECK_PTR(ptr_data);

    if (!_ext_dev_access.phy_cl45_read_callback)
    {
        return AIR_E_OTHERS;
    }

    return _ext_dev_access.phy_cl45_read_callback(unit, port_id, dev_type, addr_offset, ptr_data);
}

/* FUNCTION NAME:   aml_writePhyRegCL45
 * PURPOSE:
 *      To write data to the phy register of the specified chip unit in Clause45.
 * INPUT:
 *      unit        -- the device unit
 *      port_id     -- physical port number
 *      dev_type    -- phy register offset
 *      addr_offset -- the address of phy register
 *      data        -- written data
 * OUTPUT:
 *      none
 * RETURN:
 *      NPS_E_OK     -- Successfully write the data.
 *      NPS_E_OTHERS -- Failed to write the data.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_writePhyRegCL45(
    const UI32_T    unit,
    const UI32_T    port_id,
    const UI32_T    dev_type,
    const UI32_T    addr_offset,
    const UI32_T    data)
{
    if (!_ext_dev_access.phy_cl45_write_callback)
    {
        return AIR_E_OTHERS;
    }

    return _ext_dev_access.phy_cl45_write_callback(unit, port_id, dev_type, addr_offset, data);
}


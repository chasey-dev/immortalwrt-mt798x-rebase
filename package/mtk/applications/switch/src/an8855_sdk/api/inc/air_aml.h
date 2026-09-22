/* FILE NAME:   air_aml.h
 * PURPOSE:
 *      Define the access management layer function in AIR SDK.
 * NOTES:
 */

#ifndef AIR_AML_H
#define AIR_AML_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef AIR_ERROR_NO_T
(*AML_DEV_READ_FUNC_T)(
    const UI32_T    unit,
    const UI32_T    addr_offset,
    UI32_T          *ptr_data);

typedef AIR_ERROR_NO_T
(*AML_DEV_WRITE_FUNC_T)(
    const UI32_T    unit,
    const UI32_T    addr_offset,
    const UI32_T    data);

typedef AIR_ERROR_NO_T
(*AML_DEV_PHY_READ_FUNC_T)(
    const UI32_T    unit,
    const UI32_T    port_id,
    const UI32_T    addr_offset,
    UI32_T          *ptr_data);

typedef AIR_ERROR_NO_T
(*AML_DEV_PHY_WRITE_FUNC_T)(
    const UI32_T    unit,
    const UI32_T    port_id,
    const UI32_T    addr_offset,
    const UI32_T    data);

typedef AIR_ERROR_NO_T
(*AML_DEV_PHY_READ_CL45_FUNC_T)(
    const UI32_T    unit,
    const UI32_T    port_id,
    const UI32_T    dev_type,
    const UI32_T    addr_offset,
    UI32_T          *ptr_data);

typedef AIR_ERROR_NO_T
(*AML_DEV_PHY_WRITE_CL45_FUNC_T)(
    const UI32_T    unit,
    const UI32_T    port_id,
    const UI32_T    dev_type,
    const UI32_T    addr_offset,
    const UI32_T    data);

/* To read or write the HW-intf registers. */
typedef struct
{
    AML_DEV_READ_FUNC_T             read_callback;
    AML_DEV_WRITE_FUNC_T            write_callback;
    AML_DEV_PHY_READ_FUNC_T         phy_read_callback;
    AML_DEV_PHY_WRITE_FUNC_T        phy_write_callback;
    AML_DEV_PHY_READ_CL45_FUNC_T    phy_cl45_read_callback;
    AML_DEV_PHY_WRITE_CL45_FUNC_T   phy_cl45_write_callback;
}AML_DEV_ACCESS_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
extern AML_DEV_ACCESS_T _ext_dev_access;

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
    UI32_T          *ptr_data);

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
    const UI32_T    data);

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
    UI32_T          *ptr_data);

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
    const UI32_T    ptr_data);

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
    UI32_T          *ptr_data);

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
    const UI32_T    data);

#endif  /* AIR_AML_H */


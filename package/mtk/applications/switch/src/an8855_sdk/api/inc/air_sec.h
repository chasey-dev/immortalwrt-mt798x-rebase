/* FILE NAME: air_sec.h
 * PURPOSE:
 *      Define the security function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_SEC_H
#define AIR_SEC_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/

/* Field for storm control */
#define AIR_STORM_MAX_COUNT    (255)

#define AIR_MAX_NUM_OF_MAC     (2048)

typedef enum
{
    AIR_STORM_TYPE_BCST,
    AIR_STORM_TYPE_MCST,
    AIR_STORM_TYPE_UCST,
    AIR_STORM_TYPE_LAST
}AIR_STORM_TYPE_T;

typedef enum
{
    AIR_STORM_UNIT_64K,
    AIR_STORM_UNIT_256K,
    AIR_STORM_UNIT_1M,
    AIR_STORM_UNIT_4M,
    AIR_STORM_UNIT_16M,
    AIR_STORM_UNIT_32M,
    AIR_STORM_UNIT_LAST
}AIR_STORM_UNIT_T;

/* Field for flooding port */
typedef enum
{
    AIR_FLOOD_TYPE_BCST,
    AIR_FLOOD_TYPE_MCST,
    AIR_FLOOD_TYPE_UCST,
    AIR_FLOOD_TYPE_QURY,
    AIR_FLOOD_TYPE_LAST
}AIR_FLOOD_TYPE_T;

/* Port security port control configurations */
typedef struct AIR_SEC_PORTSEC_PORT_CONFIG_S
{
    /* Source MAC address learning mode */
    BOOL_T sa_lrn_en;

    /* Learned source MAC address counter */
    BOOL_T sa_lmt_en;

    /* Rx SA allowable learning limit number */
    UI32_T sa_lmt_cnt;

}AIR_SEC_PORTSEC_PORT_CONFIG_T;

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: air_sec_setStormEnable
 * PURPOSE:
 *      Enable or disable per port storm control for specific type.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      type            --  AIR_STORM_TYPE_BCST
 *                          AIR_STORM_TYPE_MCST
 *                          AIR_STORM_TYPE_UCST
 *      storm_en        --  TRUE
 *                          FALSE
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
air_sec_setStormEnable(
    const UI32_T unit,
    const UI32_T port,
    const AIR_STORM_TYPE_T type,
    const BOOL_T storm_en);

/* FUNCTION NAME: air_sec_getStormEnable
 * PURPOSE:
 *      Get per port status of storm control for specific type.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      type            --  AIR_STORM_TYPE_BCST
 *                          AIR_STORM_TYPE_MCST
 *                          AIR_STORM_TYPE_UCST
 * OUTPUT:
 *      ptr_storm_en    --  TRUE
 *                          FALSE
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getStormEnable(
    const UI32_T unit,
    const UI32_T port,
    const AIR_STORM_TYPE_T type,
    BOOL_T *ptr_storm_en);

/* FUNCTION NAME: air_sec_setStormRate
 * PURPOSE:
 *      Set per port storm rate limit control for specific type.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      type            --  AIR_STORM_TYPE_BCST
 *                          AIR_STORM_TYPE_MCST
 *                          AIR_STORM_TYPE_UCST
 *      count           --  Count of the unit
 *                          Range 0..255
 *                          Rate = (count * unit) bps
 *      unit            --  AIR_STORM_UNIT_64K
 *                          AIR_STORM_UNIT_256K
 *                          AIR_STORM_UNIT_1M
 *                          AIR_STORM_UNIT_4M
 *                          AIR_STORM_UNIT_16M
 *                          AIR_STORM_UNIT_32M
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
air_sec_setStormRate(
    const UI32_T unit,
    const UI32_T port,
    const AIR_STORM_TYPE_T type,
    const UI32_T count,
    const AIR_STORM_UNIT_T storm_unit);

/* FUNCTION NAME: air_sec_getStormRate
 * PURPOSE:
 *      Get per port storm rate limit control for specific type.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      type            --  AIR_STORM_TYPE_BCST
 *                          AIR_STORM_TYPE_MCST
 *                          AIR_STORM_TYPE_UCST
 * OUTPUT:
 *      ptr_count       --  Count of the unit
 *                          Range 0..255
 *                          Rate = (count * unit) bps
 *      ptr_unit        --  AIR_STORM_UNIT_64K
 *                          AIR_STORM_UNIT_256K
 *                          AIR_STORM_UNIT_1M
 *                          AIR_STORM_UNIT_4M
 *                          AIR_STORM_UNIT_16M
 *                          AIR_STORM_UNIT_32M
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getStormRate(
    const UI32_T unit,
    const UI32_T port,
    const AIR_STORM_TYPE_T type,
    UI32_T *ptr_count,
    AIR_STORM_UNIT_T *ptr_unit);

/* FUNCTION NAME: air_sec_setFldMode
 * PURPOSE:
 *      Set per port flooding status for unknown type frame.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port to setting
 *      type            --  AIR_FLOOD_TYPE_BCST
 *                          AIR_FLOOD_TYPE_MCST
 *                          AIR_FLOOD_TYPE_UCST
 *                          AIR_FLOOD_TYPE_QURY
 *      fld_en          --  TRUE : flooding specific type frame for specific port
 *                          FALSE: drop specific type frame for specific port
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_setFldMode(
    const UI32_T unit,
    const UI32_T port,
    const AIR_FLOOD_TYPE_T type,
    const BOOL_T fld_en);

/* FUNCTION NAME: air_sec_getFldMode
 * PURPOSE:
 *      Get per port flooding status for unknown type frame.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port to setting
 *      type            --  AIR_FLOOD_TYPE_BCST
 *                          AIR_FLOOD_TYPE_MCST
 *                          AIR_FLOOD_TYPE_UCST
 *                          AIR_FLOOD_TYPE_QURY
 * OUTPUT:
 *      ptr_fld_en      --  TRUE : flooding specific type frame for specific port
 *                          FALSE: drop specific type frame for specific port
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getFldMode(
    const UI32_T unit,
    const UI32_T port,
    const AIR_FLOOD_TYPE_T type,
    BOOL_T *ptr_fld_en);

/* FUNCTION NAME: air_sec_setPortSecPortCfg
 * PURPOSE:
 *      Set port security configurations for specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      port_config     --  Structure of port configuration.
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
air_sec_setPortSecPortCfg(
    const UI32_T unit,
    const UI32_T port,
    const AIR_SEC_PORTSEC_PORT_CONFIG_T port_config);

/* FUNCTION NAME: air_sec_getPortSecPortCfg
 * PURPOSE:
 *      Get port security configurations for specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *
 * OUTPUT:
 *      ptr_port_config --  Structure of port configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getPortSecPortCfg(
    const UI32_T unit,
    const UI32_T port,
    AIR_SEC_PORTSEC_PORT_CONFIG_T *ptr_port_config);

#endif /* End of AIR_SEC_H */

/* FILE NAME: air_switch.h
 * PURPOSE:
 *      Define the switch function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_SWITCH_H
#define AIR_SWITCH_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/
#define SYS_INT_EN                 0x1021C010
#define SYS_INT_STS                0x1021C014

/* DATA TYPE DECLARATIONS
*/
typedef enum
{
    AIR_SYS_INTR_TYPE_PHY0_LC = 0,
    AIR_SYS_INTR_TYPE_PHY1_LC,
    AIR_SYS_INTR_TYPE_PHY2_LC,
    AIR_SYS_INTR_TYPE_PHY3_LC,
    AIR_SYS_INTR_TYPE_PHY4_LC,
    AIR_SYS_INTR_TYPE_PHY5_LC,
    AIR_SYS_INTR_TYPE_PHY6_LC,
    AIR_SYS_INTR_TYPE_PHY7_LC,
    AIR_SYS_INTR_TYPE_MAC_PC = 16,
    AIR_SYS_INTR_TYPE_LAST
}AIR_SYS_INTR_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/

/* FUNCTION NAME: air_switch_setCpuPort
 * PURPOSE:
 *      Set CPU port member
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  CPU port index
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
air_switch_setCpuPort(
    const UI32_T unit,
    const UI32_T portmap);

/* FUNCTION NAME: air_switch_getCpuPort
 * PURPOSE:
 *      Get CPU port member
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_port        --  CPU port index
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_switch_getCpuPort(
    const UI32_T unit,
    UI32_T *ptr_portmap);

/* FUNCTION NAME: air_switch_setCpuPortEN
 * PURPOSE:
 *      Set CPU port Enable
 *
 * INPUT:
 *      unit            --  Device ID
 *      cpu_en          --  CPU Port Enable
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
air_switch_setCpuPortEn(
    const UI32_T unit,
    const BOOL_T cpu_en);

/* FUNCTION NAME: air_switch_getCpuPortEn
 * PURPOSE:
 *      Get CPU port member
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      cpu_en          --  CPU Port enable
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_switch_getCpuPortEn(
    const UI32_T unit,
    BOOL_T *cpu_en);

/* FUNCTION NAME: air_switch_setSysIntrEn
 * PURPOSE:
 *      Set system interrupt enable
 *
 * INPUT:
 *      unit            --  Device ID
 *      intr            --  system interrupt type
 *      enable          --  system interrupt enable/disable
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
air_switch_setSysIntrEn(
    const UI32_T unit,
    const AIR_SYS_INTR_TYPE_T intr,
    const BOOL_T enable);

/* FUNCTION NAME: air_switch_getSysIntrEn
 * PURPOSE:
 *      Get system interrupt enable
 *
 * INPUT:
 *      unit            --  Device ID
 *      intr            --  system interrupt type
 *
 * OUTPUT:
 *      ptr_enable      --  system interrupt enable/disable
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_switch_getSysIntrEn(
    const UI32_T unit,
    const AIR_SYS_INTR_TYPE_T intr,
    BOOL_T *ptr_enable);

/* FUNCTION NAME: air_switch_setSysIntrStatus
 * PURPOSE:
 *      Set system interrupt status
 *
 * INPUT:
 *      unit            --  Device ID
 *      intr            --  system interrupt type
 *      enable          --  write TRUE to clear interrupt status
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
air_switch_setSysIntrStatus(
    const UI32_T unit,
    const AIR_SYS_INTR_TYPE_T intr,
    const BOOL_T enable);

/* FUNCTION NAME: air_switch_getSysIntrStatus
 * PURPOSE:
 *      Get system interrupt status
 *
 * INPUT:
 *      unit            --  Device ID
 *      intr            --  system interrupt type
 *
 * OUTPUT:
 *      ptr_enable      --  system interrupt status
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_switch_getSysIntrStatus(
    const UI32_T unit,
    const AIR_SYS_INTR_TYPE_T intr,
    BOOL_T *ptr_enable);

/* FUNCTION NAME: air_switch_reset
 * PURPOSE:
 *      Reset whole system
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_switch_reset(
    const UI32_T unit);

#endif /* End of AIR_SWITCH_H */

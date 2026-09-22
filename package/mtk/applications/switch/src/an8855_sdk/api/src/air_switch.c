/* FILE NAME: air_switch.c
 * PURPOSE:
 *      Define the switch function in AIR SDK.
 *
 * NOTES:
 *      None
 */

/* INCLUDE FILE DECLARATIONS
*/
#include "air.h"

/* NAMING CONSTANT DECLARATIONS
*/
#define AIR_SYS_RST_WAIT_TIME       (100000)

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

/* LOCAL SUBPROGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
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
    const UI32_T port)
{
    UI32_T regMFC = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK(port > AIR_MAX_NUM_OF_PORTS, AIR_E_BAD_PARAMETER);

    /* Read CFC */
    aml_readReg(unit, MFC, &regMFC);
    AIR_PRINT("PTC REG:%x. val:%x\n", MFC,regMFC);

    /* Set CPU portmap */
    regMFC &= ~BITS_RANGE(MFC_CPU_PORT_OFFSET, MFC_CPU_PORT_LENGTH);
    regMFC |= (port << MFC_CPU_PORT_OFFSET);

    /* Write CFC */
    aml_writeReg(unit, MFC, regMFC);
    AIR_PRINT("PTC REG:%x. val:%x\n", MFC,regMFC);

    return AIR_E_OK;
}

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
    UI32_T *ptr_port)
{
    UI32_T regMFC = 0;

    /* Mistake proofing */
    AIR_CHECK_PTR(ptr_port);

    /* Read CFC */
    aml_readReg(unit, MFC, &regMFC);

    /* Get CPU portmap */
    (*ptr_port) = BITS_OFF_R(regMFC, MFC_CPU_PORT_OFFSET, MFC_CPU_PORT_LENGTH);

    return AIR_E_OK;
}


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
    const BOOL_T cpu_en)
{
    UI32_T regMFC = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK(((TRUE != cpu_en) && (FALSE != cpu_en)), AIR_E_BAD_PARAMETER);

    /* Read CFC */
    aml_readReg(unit, MFC, &regMFC);

    /* Set CPU portmap */
    regMFC &= ~BITS_RANGE(MFC_CPU_EN_OFFSET, MFC_CPU_EN_LENGTH);
    regMFC |= cpu_en << MFC_CPU_EN_OFFSET ;

    /* Write CFC */
    aml_writeReg(unit, MFC, regMFC);

    return AIR_E_OK;
}

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
    BOOL_T *cpu_en)
{
    UI32_T regMFC = 0;

    /* Mistake proofing */
    AIR_CHECK_PTR(cpu_en);

    /* Read CFC */
    aml_readReg(unit, MFC, &regMFC);

    /* Get CPU portmap */
    (*cpu_en) = BITS_OFF_R(regMFC, MFC_CPU_EN_OFFSET, MFC_CPU_EN_LENGTH);

    return AIR_E_OK;
}

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
    const BOOL_T enable)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((intr >= AIR_SYS_INTR_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((intr > AIR_SYS_INTR_TYPE_PHY7_LC) && (intr < AIR_SYS_INTR_TYPE_MAC_PC)), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != enable) && (FALSE != enable)), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, SYS_INT_EN, &val);
    val &= ~BIT(intr);
    val |= (TRUE == enable) ? BIT(intr) : 0;
    aml_writeReg(unit, SYS_INT_EN, val);

    return AIR_E_OK;
}

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
    BOOL_T *ptr_enable)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((intr >= AIR_SYS_INTR_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((intr > AIR_SYS_INTR_TYPE_PHY7_LC) && (intr < AIR_SYS_INTR_TYPE_MAC_PC)), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_enable);

    aml_readReg(unit, SYS_INT_EN, &val);
    *ptr_enable = (val & BIT(intr)) ? TRUE : FALSE;

    return AIR_E_OK;
}

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
    const BOOL_T enable)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((intr >= AIR_SYS_INTR_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((intr > AIR_SYS_INTR_TYPE_PHY6_LC) && (intr < AIR_SYS_INTR_TYPE_MAC_PC)), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != enable), AIR_E_BAD_PARAMETER);

    aml_writeReg(unit, SYS_INT_STS, BIT(intr));

    return AIR_E_OK;
}

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
    BOOL_T *ptr_enable)
{
    UI32_T val = 0;

    AIR_PARAM_CHK((intr >= AIR_SYS_INTR_TYPE_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((intr > AIR_SYS_INTR_TYPE_PHY6_LC) && (intr < AIR_SYS_INTR_TYPE_MAC_PC)), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_enable);

    aml_readReg(unit, SYS_INT_STS, &val);
    *ptr_enable = (val & BIT(intr)) ? TRUE : FALSE;

    return AIR_E_OK;
}

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
    const UI32_T unit)
{
    UI32_T val = 0;

    aml_writeReg(unit, RST_CTRL1, BIT(SYS_SW_RST_OFFT));
    AIR_UDELAY(AIR_SYS_RST_WAIT_TIME);

    return AIR_E_OK;
}

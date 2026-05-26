/* FILE NAME:  air_lag.c
 * PURPOSE:
 *      Define the Link Agrregation function in AIR SDK.
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

/* LOCAL SUBPROGRAM DECLARATIONS
*/

/* STATIC VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
*/

/* FUNCTION NAME: air_lag_setMember
 * PURPOSE:
 *      Set LAG member(s) for a specific LAG port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptg_index       --  Port trunk index
 *      mem_index       --  Member index
 *      mem_en          --  enable Member
 *      port_index      --  Member port
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
air_lag_setMember(
    const UI32_T        unit,
    const UI32_T        ptg_index,
    const UI32_T        mem_index,
    const UI32_T        mem_en,
    const UI32_T        port_index)
{
    UI32_T val = 0;
    UI32_T i = 0, offset = 0;
    UI32_T reg = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK((ptg_index >= AIR_LAG_MAX_PTG_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((mem_index > AIR_LAG_MAX_MEM_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((mem_en !=0 && mem_en !=1), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port_index > AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    offset = mem_index;
    reg = (UI32_T)PTG(ptg_index);

    AIR_PRINT("PTC REG:%x.\n", reg);

    aml_readReg(unit,reg, &val);
    AIR_PRINT("PTC REG val:%x.---1\n", val);
    if(mem_en == 0)
    {
        val = val & ~(BIT(7 + 8*offset)); //port turnk group ptg_index; port port_index
    }
    else
    {
        val = val  | (BIT(7 + 8*offset)); //port turnk group ptg_index; port port_index
    }
    AIR_PRINT("PTC REG val:%x.----2\n", val);
    val = val & ~( 0x1F << 8*offset);
    val = val | AIR_GRP_PORT(port_index,offset); //port turnk group ptg_index; port port_index
    AIR_PRINT("PTC REG val:%x. port %d----3\n", val,port_index);

    aml_writeReg(unit, reg, val);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_lag_getMember
 * PURPOSE:
 *      Get LAG member count.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptg_index       --  Port trunk index
 *
 * OUTPUT:
 *      member      --  Member ports of  one port trunk
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_getMember(
    const UI32_T unit,
    const UI32_T ptg_index,
    AIR_LAG_PTGINFO_T * member)
{
    UI32_T val0 = 0, val1 = 0, i = 0, offset = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK((ptg_index >= AIR_LAG_MAX_PTG_NUM), AIR_E_BAD_PARAMETER);

    /* Mistake proofing */
    AIR_CHECK_PTR(member);
    aml_readReg(unit, (UI32_T)PTG(ptg_index), &val0);

    for(i = 0; i < AIR_LAG_MAX_MEM_NUM; i++){
        member->csr_gp_enable[i] = (UI32_T)BITS_OFF_R(val0, 7 + 8*i, 1);
        member->csr_gp_port[i] = (UI32_T)BITS_OFF_R(val0, 8*i, 5);
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_lag_set_ptgc_state
 * PURPOSE:
 *     set port trunk group control state.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptgc_enable     --  enabble or disable port trunk function
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_set_ptgc_state(
    const UI32_T unit,
    const BOOL_T ptgc_enable)
{
    /* Mistake proofing */
    AIR_PARAM_CHK(((TRUE != ptgc_enable) && (FALSE != ptgc_enable)), AIR_E_BAD_PARAMETER);

    aml_writeReg(unit, PTGC, ptgc_enable);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_lag_get_ptgc_state
 * PURPOSE:
 *      Get port trunk group control state.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_state        --  port trunk fucntion is enable or disable
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_get_ptgc_state(
    const UI32_T unit,
    UI32_T *ptr_state)
{
    UI32_T u32dat = 0;
    
    AIR_CHECK_PTR(ptr_state);
    aml_readReg(unit, PTGC, &u32dat);
    (*ptr_state) = BITS_OFF_R(u32dat, 0, 1);

    return AIR_E_OK;
}


/* FUNCTION NAME: air_lag_setDstInfo
 * PURPOSE:
 *      Set information for the packet distribution.
 *
 * INPUT:
 *      unit            --  Device ID
 *      dstInfo         --  Infomation selection of packet distribution
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_setDstInfo(
    const UI32_T unit,
    const AIR_LAG_DISTINFO_T dstInfo)
{
    UI32_T val = 0;
    aml_readReg(unit, (UI32_T)PTC, &val);

    /* Set infomation control bit map */
    val = val & ~ BITS(0,6);
    if(dstInfo.sp)
    {
        val |= PTC_INFO_SEL_SP;
    }
    if(dstInfo.sa)
    {
        val |= PTC_INFO_SEL_SA;
    }
    if(dstInfo.da)
    {
        val |= PTC_INFO_SEL_DA;
    }
    if(dstInfo.sip)
    {
        val |= PTC_INFO_SEL_SIP;
    }
    if(dstInfo.dip)
    {
        val |= PTC_INFO_SEL_DIP;
    }
    if(dstInfo.sport)
    {
        val |= PTC_INFO_SEL_SPORT;
    }
    if(dstInfo.dport)
    {
        val |= PTC_INFO_SEL_DPORT;
    }

    /* Write register */
    aml_writeReg(unit, (UI32_T)PTC, val);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_lag_getDstInfo
 * PURPOSE:
 *      Set port trunk hashtype.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_dstInfo     --  Infomation selection of packet distribution
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_getDstInfo(
    const UI32_T unit,
    AIR_LAG_DISTINFO_T *ptr_dstInfo)
{
    UI32_T val = 0;

    /* Mistake proofing */
    AIR_CHECK_PTR(ptr_dstInfo);

    /* Get infomation control bit map */
    aml_readReg(unit, (UI32_T)PTC, &val);
    if(val & PTC_INFO_SEL_SP)
    {
        ptr_dstInfo ->sp = 1;
    }
    if(val & PTC_INFO_SEL_SA)
    {
        ptr_dstInfo ->sa = 1;
    }
    if(val & PTC_INFO_SEL_DA)
    {
        ptr_dstInfo ->da = 1;
    }
    if(val & PTC_INFO_SEL_SIP)
    {
        ptr_dstInfo ->sip = 1;
    }
    if(val & PTC_INFO_SEL_DIP)
    {
        ptr_dstInfo ->dip = 1;
    }
    if(val & PTC_INFO_SEL_SPORT)
    {
        ptr_dstInfo ->sport = 1;
    }
    if(val & PTC_INFO_SEL_DPORT)
    {
        ptr_dstInfo ->dport = 1;
    }

    return AIR_E_OK;
}


/* FUNCTION NAME: air_lag_setState
 * PURPOSE:
 *      Set the enable/disable for a specific LAG port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      hashtype        --  crc32msb/crc32lsb/crc16/xor4
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
air_lag_sethashtype(
    const UI32_T unit,
    const UI32_T hashtype)
{
    UI32_T val = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK((hashtype > 3), AIR_E_BAD_PARAMETER);

    /* Read data from register */
    aml_readReg(unit, (UI32_T)PTC, &val);
    
    val = val & ~ BITS(8,9);
    val |= hashtype << 8;
    
    aml_writeReg(unit, (UI32_T)PTC, val);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_lag_getState
 * PURPOSE:
 *      Get port trunk hashtype.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      hashtype        --  crc32msb/crc32lsb/crc16/xor4
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_gethashtype(
    const UI32_T unit,
    UI32_T *hashtype)
{
    UI32_T val = 0;

    /* Mistake proofing */
    AIR_CHECK_PTR(hashtype);

    /* Read data from register */
    aml_readReg(unit, (UI32_T)PTC, &val);
    (*hashtype) = BITS_OFF_R(val, 8, 9);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_lag_setSpSel
 * PURPOSE:
 *      Set the enable/disable for selection source port composition.
 *
 * INPUT:
 *      unit            --  Device ID
 *      enable          --  enable or disable source port compare
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
air_lag_setSpSel(
    const UI32_T unit,
    const BOOL_T spsel_enable)
{
    UI32_T val = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK(((TRUE != spsel_enable) && (FALSE != spsel_enable)), AIR_E_BAD_PARAMETER);

    /* Read data from register */
    aml_readReg(unit, (UI32_T)PTC, &val);
    val = val & ~ BIT(20);
    val |= spsel_enable << 20;
    aml_writeReg(unit, (UI32_T)PTC, val);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_lag_getSpSel
 * PURPOSE:
 *      Get selection source port composition.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_state       --  source port compare is enable or disable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_getSpSel(
    const UI32_T unit,
    UI32_T *ptr_state)
{
    UI32_T val = 0;

    /* Mistake proofing */
    AIR_CHECK_PTR(ptr_state);

    /* Read data from register */
    AIR_CHECK_PTR(ptr_state);
    aml_readReg(unit, PTC, &val);
    (*ptr_state) = val & BIT(20);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_lag_setPTSeed
 * PURPOSE:
 *      Set the enable/disable for a specific LAG port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptseed          --  port trunk rand seed
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
air_lag_setPTSeed(
    const UI32_T unit,
    const UI32_T ptseed)
{
    aml_writeReg(unit, (UI32_T)PTSEED, ptseed);
    return AIR_E_OK;
}

/* FUNCTION NAME: air_lag_getPTSeed
 * PURPOSE:
 *      Get port trunk hashtype.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptseed          --  port trunk rand seed
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_getPTSeed(
    const UI32_T unit,
    UI32_T *ptseed)
{
    UI32_T val = 0;

    /* Mistake proofing */
    AIR_CHECK_PTR(ptseed);

    /* Read data from register */
    aml_readReg(unit, (UI32_T)PTSEED, ptseed);

    return AIR_E_OK;
}


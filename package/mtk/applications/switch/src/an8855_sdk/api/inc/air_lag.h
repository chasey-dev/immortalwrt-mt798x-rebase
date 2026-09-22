/* FILE NAME:  air_lag.h
 * PURPOSE:
 *      Define the Link Agrregation function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_LAG_H
#define AIR_LAG_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/
#define AIR_LAG_MAX_MEM_NUM       (4)
#define AIR_LAG_MAX_PTG_NUM       (2)

#define AIR_LAG_MAX_SA_LRN_NUM     BITS(0,12)
#define AIR_GRP_PORT(p,n)         (((p) & BITS(0,4)) << (n * 8))
#define AIR_LAG_MAX_BUSY_TIME      (20)



/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef struct AIR_LAG_PTGINFO_S
{
    UI32_T csr_gp_enable[4];
    UI32_T csr_gp_port[4];
}AIR_LAG_PTGINFO_T;


typedef struct AIR_LAG_DISTINFO_S
{
    UI32_T sp:1;
    UI32_T sa:1;
    UI32_T da:1;
    UI32_T sip:1;
    UI32_T dip:1;
    UI32_T sport:1;
    UI32_T dport:1;
    UI32_T reverse:25;
}AIR_LAG_DISTINFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const UI32_T        port_index);


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
    AIR_LAG_PTGINFO_T * member);

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
    const BOOL_T ptgc_enable);

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
    UI32_T *ptr_state);


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
    const AIR_LAG_DISTINFO_T dstInfo);

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
    AIR_LAG_DISTINFO_T *ptr_dstInfo);


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
    const UI32_T hashtype);

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
    UI32_T *hashtype);

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
    const BOOL_T spsel_enable);

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
    UI32_T *ptr_state);

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
    const UI32_T ptseed);

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
    UI32_T *ptseed);


#endif /* End of AIR_LAG_H */

/* FILE NAME: air_qos.c
 * PURPOSE:
 *      Define the Quailty of Service function in AIR SDK.
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

/* FUNCTION NAME:   air_qos_setScheduleAlgo
 * PURPOSE:
 *      Set schedule mode of a port queue.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port id
 *      queue                -- Queue id
 *      sch_mode             -- AIR_QOS_SCH_MODE_T
 *      weight               -- weight for WRR/WFQ
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      Weight default value is 1, only for WRR/WFQ mode
 */
AIR_ERROR_NO_T
air_qos_setScheduleAlgo(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T                queue,
    const AIR_QOS_SCH_MODE_T    sch_mode,
    const UI32_T                weight)
{
    UI32_T rc = AIR_E_OK;
    UI32_T mac_port = 0;
    AIR_QOS_SHAPER_MIN_T min_v;
    AIR_QOS_SHAPER_MAX_T max_v;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((queue >= AIR_QOS_QUEUE_MAX_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((sch_mode >= AIR_QOS_SCH_MODE_LAST), AIR_E_BAD_PARAMETER);
    if (AIR_QOS_SHAPER_NOSETTING != weight)
    {
        AIR_PARAM_CHK(((weight >  AIR_QOS_SHAPER_RATE_MAX_WEIGHT) || 
            (weight <  AIR_QOS_SHAPER_RATE_MIN_WEIGHT)), AIR_E_BAD_PARAMETER);
    }
    mac_port = port;
    min_v.byte = 0;
    max_v.byte = 0;
     /*Read register data*/
    switch(queue)
    {
        case AIR_QOS_QUEUE_0:
            rc += aml_readReg(unit, MMSCR0_Q0(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q0(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_1:
            rc += aml_readReg(unit, MMSCR0_Q1(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q1(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_2:
            rc += aml_readReg(unit, MMSCR0_Q2(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q2(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_3:
            rc += aml_readReg(unit, MMSCR0_Q3(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q3(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_4:
            rc += aml_readReg(unit, MMSCR0_Q4(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q4(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_5:
            rc += aml_readReg(unit, MMSCR0_Q5(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q5(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_6:
            rc += aml_readReg(unit, MMSCR0_Q6(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q6(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_7:
            rc += aml_readReg(unit, MMSCR0_Q7(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q7(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            AIR_PRINT("Not Support this queue %d num, please check again\n", queue);
            return AIR_E_BAD_PARAMETER;
    }

    /*Get para*/
    switch(sch_mode)
    {
        case AIR_QOS_SCH_MODE_SP:
            min_v.raw.min_sp_wrr_q = 1;
            min_v.raw.min_rate_en = 0;
            break;

        case AIR_QOS_SCH_MODE_WRR:
            min_v.raw.min_sp_wrr_q = 0;
            min_v.raw.min_rate_en = 0;
            min_v.raw.min_weight = weight - 1;
            break;

        case AIR_QOS_SCH_MODE_WFQ:
            min_v.raw.min_sp_wrr_q = 1;
            min_v.raw.min_rate_en = 1;
            min_v.raw.min_rate_man = 0;
            min_v.raw.min_rate_exp = 0;

            max_v.raw.max_rate_en = 0;
            max_v.raw.max_sp_wfq_q = 0;
            max_v.raw.max_weight = weight - 1;
            break;
        default:
            AIR_PRINT("Not Support this mode %d num, please check again\n", sch_mode);
            return AIR_E_BAD_PARAMETER;
    }

    /*Send to driver*/
    switch(queue)
    {
        case AIR_QOS_QUEUE_0:
            rc += aml_writeReg(unit, MMSCR0_Q0(mac_port), min_v.byte);
            rc += aml_writeReg(unit, MMSCR1_Q0(mac_port), max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_1:
            rc += aml_writeReg(unit, MMSCR0_Q1(mac_port), min_v.byte);
            rc += aml_writeReg(unit, MMSCR1_Q1(mac_port), max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_2:
            rc += aml_writeReg(unit, MMSCR0_Q2(mac_port), min_v.byte);
            rc += aml_writeReg(unit, MMSCR1_Q2(mac_port), max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_3:
            rc += aml_writeReg(unit, MMSCR0_Q3(mac_port), min_v.byte);
            rc += aml_writeReg(unit, MMSCR1_Q3(mac_port), max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_4:
            rc += aml_writeReg(unit, MMSCR0_Q4(mac_port), min_v.byte);
            rc += aml_writeReg(unit, MMSCR1_Q4(mac_port), max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_5:
            rc += aml_writeReg(unit, MMSCR0_Q5(mac_port), min_v.byte);
            rc += aml_writeReg(unit, MMSCR1_Q5(mac_port), max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_6:
            rc += aml_writeReg(unit, MMSCR0_Q6(mac_port), min_v.byte);
            rc += aml_writeReg(unit, MMSCR1_Q6(mac_port), max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_7:
            rc += aml_writeReg(unit, MMSCR0_Q7(mac_port), min_v.byte);
            rc += aml_writeReg(unit, MMSCR1_Q7(mac_port), max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            AIR_PRINT("Not Support this queue %d num, please check again\n", queue);
            return AIR_E_BAD_PARAMETER;
    }
    AIR_PRINT("Set schedule mode success,port is %d, queue is %d, min hex is %x, max hex is %x\n", port, queue, min_v.byte, max_v.byte);

    return rc;
}

/* FUNCTION NAME: air_qos_getScheduleAlgo
 * PURPOSE:
 *      Get schedule mode of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 * OUTPUT:
 *      ptr_sch_mode    --  AIR_QOS_SCH_MODE_T
 *      ptr_weight      --  weight for WRR/WFQ
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *     None
 */
AIR_ERROR_NO_T
air_qos_getScheduleAlgo(
    const UI32_T          unit,
    const UI32_T          port,
    const UI32_T          queue,
    AIR_QOS_SCH_MODE_T    *ptr_sch_mode,
    UI32_T                *ptr_weight)
{
    UI32_T rc = AIR_E_OK;
    UI32_T mac_port = 0;
    AIR_QOS_SHAPER_MIN_T min_v;
    AIR_QOS_SHAPER_MAX_T max_v;

    /*Read register data*/
    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((queue >= AIR_QOS_QUEUE_MAX_NUM), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_sch_mode);
    AIR_CHECK_PTR(ptr_weight);
    
    mac_port = port;
    min_v.byte = 0;
    max_v.byte = 0;

    switch(queue)
    {
        case AIR_QOS_QUEUE_0:
            rc += aml_readReg(unit, MMSCR0_Q0(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q0(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_1:
            rc += aml_readReg(unit, MMSCR0_Q1(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q1(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_2:
            rc += aml_readReg(unit, MMSCR0_Q2(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q2(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_3:
            rc += aml_readReg(unit, MMSCR0_Q3(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q3(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_4:
            rc += aml_readReg(unit, MMSCR0_Q4(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q4(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_5:
            rc += aml_readReg(unit, MMSCR0_Q5(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q5(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_6:
            rc += aml_readReg(unit, MMSCR0_Q6(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q6(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case AIR_QOS_QUEUE_7:
            rc += aml_readReg(unit, MMSCR0_Q7(mac_port), &min_v.byte);
            rc += aml_readReg(unit, MMSCR1_Q7(mac_port), &max_v.byte);
            if(AIR_E_OK != rc)
            {
                AIR_PRINT("Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            AIR_PRINT("Not Support this queue %d num, please check again", queue);
            return AIR_E_BAD_PARAMETER;
    }

    /*Send para*/
    if ((min_v.raw.min_rate_en) && AIR_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_WFQ == max_v.raw.max_sp_wfq_q)
    {
        *ptr_sch_mode = AIR_QOS_SCH_MODE_WFQ;
        *ptr_weight = max_v.raw.max_weight + 1;
    }
    else
    {
        if(AIR_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_WRR == min_v.raw.min_sp_wrr_q)
        {
            *ptr_sch_mode = AIR_QOS_SCH_MODE_WRR;
            *ptr_weight = min_v.raw.min_weight + 1;
        }
        else if(AIR_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_SP == min_v.raw.min_sp_wrr_q)
        {
            *ptr_sch_mode = AIR_QOS_SCH_MODE_SP;
            *ptr_weight = AIR_QOS_SHAPER_NOSETTING;
        }
    }
    AIR_PRINT("Get schedule mode success,port is %d, queue is %d, min hex is %x, max hex is %x\n", port, queue, min_v.byte, max_v.byte);

    return rc;
}

/* FUNCTION NAME:   air_qos_setTrustMode
 * PURPOSE:
 *      Set qos trust mode value.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                  -.Select port number
 *      mode                 -- Qos support mode
 *                              AIR_QOS_TRUST_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */

AIR_ERROR_NO_T
air_qos_setTrustMode(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_QOS_TRUST_MODE_T      mode)

{
    UI32_T rc = AIR_E_OTHERS;
    AIR_QOS_QUEUE_UPW_T stat;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((mode >= AIR_QOS_TRUST_MODE_LAST), AIR_E_BAD_PARAMETER);

    stat.byte = 0;
    /*get register val*/
    rc = aml_readReg(unit, PUPW(port), &(stat.byte));
    AIR_PRINT("[Dbg]: get port %d rate trust weight success, UPW hex is %x\n", port, stat.byte);
    stat.byte = AIR_QOS_QUEUE_DEFAULT_VAL;
    if(AIR_E_OK == rc)
    {
        switch(mode)
        {
            case AIR_QOS_TRUST_MODE_PORT:
                stat.raw.csr_port_weight = AIR_QOS_QUEUE_TRUST_HIGH_WEIGHT;
                break;

            case AIR_QOS_TRUST_MODE_1P_PORT:
                stat.raw.csr_1p_weight = AIR_QOS_QUEUE_TRUST_HIGH_WEIGHT;
                stat.raw.csr_port_weight = AIR_QOS_QUEUE_TRUST_MID_WEIGHT;
                break;

            case AIR_QOS_TRUST_MODE_DSCP_PORT:
                stat.raw.csr_dscp_weight = AIR_QOS_QUEUE_TRUST_HIGH_WEIGHT;
                stat.raw.csr_port_weight = AIR_QOS_QUEUE_TRUST_MID_WEIGHT;
                break;

            case AIR_QOS_TRUST_MODE_DSCP_1P_PORT:
                stat.raw.csr_dscp_weight = AIR_QOS_QUEUE_TRUST_HIGH_WEIGHT;
                stat.raw.csr_1p_weight = AIR_QOS_QUEUE_TRUST_MID_WEIGHT;
                stat.raw.csr_port_weight = AIR_QOS_QUEUE_TRUST_LOW_WEIGHT;
                break;

            default:
                AIR_PRINT("Not Support this mode %d yet\n", mode);
                return AIR_E_BAD_PARAMETER;

        }
    }

    /*set register val*/
    rc = aml_writeReg(unit, PUPW(port), stat.byte);
    if(AIR_E_OK != rc)
    {
        AIR_PRINT("[Dbg]: set port %d rate trust mode failed  rc is %d\n", port, rc);
    }
    else
    {
        AIR_PRINT("[Dbg]: set port %d rate trust mode %d weight success, UPW hex is %x\n", port, mode, stat.byte);
    }
    return rc;
}

/* FUNCTION NAME: air_qos_getTrustMode
 * PURPOSE:
 *      Get qos trust mode value.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port              -.Select port number
 * OUTPUT:
 *      ptr_weight      --  All Qos weight value
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getTrustMode(
    const UI32_T unit,
    const UI32_T                    port,
    AIR_QOS_TRUST_MODE_T *const ptr_mode)

{
    UI32_T rc = AIR_E_OTHERS;
    AIR_QOS_QUEUE_UPW_T stat;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_mode);

    /*get register val*/
    stat.byte = 0;
    *ptr_mode = AIR_QOS_TRUST_MODE_1P_PORT;
    rc = aml_readReg(unit, PUPW(port), &(stat.byte));
    if(AIR_E_OK != rc)
    {
        AIR_PRINT("[Dbg]: get port %d rate trust mode failed  rc is %d\n",port, rc);
    }
    else
    {
        if (AIR_QOS_QUEUE_TRUST_HIGH_WEIGHT == stat.raw.csr_1p_weight)
        {
            *ptr_mode = AIR_QOS_TRUST_MODE_1P_PORT;
        }
        else if (AIR_QOS_QUEUE_TRUST_HIGH_WEIGHT == stat.raw.csr_dscp_weight)
        {
            if (AIR_QOS_QUEUE_TRUST_MID_WEIGHT == stat.raw.csr_1p_weight)
            {
                *ptr_mode = AIR_QOS_TRUST_MODE_DSCP_1P_PORT;
            }
            else if (AIR_QOS_QUEUE_TRUST_MID_WEIGHT == stat.raw.csr_port_weight)
            {
                *ptr_mode = AIR_QOS_TRUST_MODE_DSCP_PORT;
            }
        }
        else if (AIR_QOS_QUEUE_TRUST_HIGH_WEIGHT == stat.raw.csr_port_weight)
        {
            *ptr_mode = AIR_QOS_TRUST_MODE_PORT;
        }
        else
        {
            AIR_PRINT("[Dbg]: port %d Not support this trust mode, UPW hex is %x\n", port, stat.byte);
        }
    }
    AIR_PRINT("[Dbg]: port %d get trust mode success, UPW hex is %x\n", port, stat.byte);
    return rc;
}

/* FUNCTION NAME: air_qos_setPri2Queue
 * PURPOSE:
 *      Set per port priority to out queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      pri             --  Qos pri value
 *      queue           --  Qos Queue value
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
air_qos_setPri2Queue(
    const UI32_T unit,
    const UI32_T pri,
    const UI32_T queue)
{
    UI32_T rc = AIR_E_OTHERS;
    AIR_QOS_QUEUE_PEM_T stat;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((queue >= AIR_QOS_QUEUE_MAX_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((pri >= AIR_QOS_QUEUE_MAX_NUM), AIR_E_BAD_PARAMETER);

    stat.byte = 0;
    /*get register val*/
    switch(pri / 2)
    {
        case 0:
            rc = aml_readReg(unit, PEM1, &stat.byte);
            if(AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    stat.raw.csr_que_cpu_h = queue;
                }
                else
                {
                     stat.raw.csr_que_cpu_l = queue;
                }
            }
            rc = aml_writeReg(unit, PEM1, stat.byte);
            break;

        case 1:
            rc = aml_readReg(unit, PEM2, &stat.byte);
            if(AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    stat.raw.csr_que_cpu_h = queue;
                }
                else
                {
                    stat.raw.csr_que_cpu_l = queue;
                }
            }
            rc = aml_writeReg(unit, PEM2, stat.byte);
            break;

        case 2:
            rc = aml_readReg(unit, PEM3, &stat.byte);
            if(AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    stat.raw.csr_que_cpu_h = queue;
                }
                else
                {
                    stat.raw.csr_que_cpu_l = queue;
                }
            }
            rc = aml_writeReg(unit, PEM3, stat.byte);
            break;

        case 3:
            rc = aml_readReg(unit, PEM4, &stat.byte);
            if(AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    stat.raw.csr_que_cpu_h = queue;
                }
                else
                {
                    stat.raw.csr_que_cpu_l = queue;
                }
            }
            rc = aml_writeReg(unit, PEM4, stat.byte);
            break;

        default:
            AIR_PRINT("[Dbg]: Not Support this pri %d yet\n", pri);
            return AIR_E_BAD_PARAMETER;
    }
    AIR_PRINT("[Dbg]: set pri %d to queue %d success, PEM hex is %x\n"
        , pri, queue, stat.byte);
    return rc;
}

/* FUNCTION NAME: air_qos_getPri2Queue
 * PURPOSE:
 *      Get per port priority to out queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      pri             --  Qos pri value
 *
 * OUTPUT:
 *      ptr_queue       --  Select out queue (0..7)
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getPri2Queue(
    const UI32_T unit,
    const UI32_T pri,
    UI32_T *const ptr_queue)
{
    UI32_T rc = AIR_E_OTHERS;
    AIR_QOS_QUEUE_PEM_T stat;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((pri >= AIR_QOS_QUEUE_MAX_NUM), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_queue);

   /*get register val*/
    stat.byte = 0;
    switch(pri / 2)
    {
        case 0:
            rc = aml_readReg(unit, PEM1, &stat.byte);
            if(AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    *ptr_queue = stat.raw.csr_que_cpu_h;
                }
                else
                {
                    *ptr_queue = stat.raw.csr_que_cpu_l;
                }
            }
            break;

        case 1:
            rc = aml_readReg(unit, PEM2, &stat.byte);
            if(AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    *ptr_queue = stat.raw.csr_que_cpu_h;
                }
                else
                {
                    *ptr_queue = stat.raw.csr_que_cpu_l;
                }
            }
            break;

        case 2:
            rc = aml_readReg(unit, PEM3, &stat.byte);
            if(AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    *ptr_queue = stat.raw.csr_que_cpu_h;
                }
                else
                {
                    *ptr_queue = stat.raw.csr_que_cpu_l;
                }
            }
            break;

        case 3:
            rc = aml_readReg(unit, PEM4, &stat.byte);
            if(AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    *ptr_queue = stat.raw.csr_que_cpu_h;
                }
                else
                {
                    *ptr_queue = stat.raw.csr_que_cpu_l;
                }
            }
            break;

        default:
            AIR_PRINT("[Dbg]: Not Support this pri %d yet\n", pri);
            return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK != rc)
    {
        AIR_PRINT("[Dbg]: get pri to queue failed  rc is %d\n", rc);
    }

    AIR_PRINT("[Dbg]: get pri %d to queue %d mode success, PEM hex is %x\n"
        , pri, *ptr_queue, stat.byte);
    return rc;
}

/* FUNCTION NAME: air_qos_setDscp2Pri
 * PURPOSE:
 *      Set DSCP mapping to priority.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dscp            --  Select DSCP value (0..63)
 *      priority        --  Select priority (0..7)
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
air_qos_setDscp2Pri(
    const UI32_T unit,
    const UI32_T dscp,
    const UI32_T pri)
{
    UI32_T rc = AIR_E_OTHERS;
    UI32_T reg = 0;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((dscp >= AIR_QOS_QUEUE_DSCP_MAX_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((pri >= AIR_QOS_QUEUE_MAX_NUM), AIR_E_BAD_PARAMETER);

    /*get register val*/
    switch (dscp/10)
    {
        case 0:
            rc = aml_readReg(unit, PIM1, &reg);
            if(AIR_E_OK == rc)
            {
                reg &= ~(AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << 3 * (dscp % 10);
                rc = aml_writeReg(unit, PIM1, reg);
            }
            break;

        case 1:
            rc = aml_readReg(unit, PIM2, &reg);
            if(AIR_E_OK == rc)
            {
                reg &= ~(AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM2, reg);
            }
            break;

        case 2:
            rc = aml_readReg(unit, PIM3, &reg);
            if(AIR_E_OK == rc)
            {
                reg &= ~(AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM3, reg);
            }
            break;

        case 3:
            rc = aml_readReg(unit, PIM4, &reg);
            if(AIR_E_OK == rc)
            {
                reg &= ~(AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM4, reg);
            }
            break;

        case 4:
            rc = aml_readReg(unit, PIM5, &reg);
            if(AIR_E_OK == rc)
            {
                reg &= ~(AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM5, reg);
            }
            break;

        case 5:
            rc = aml_readReg(unit, PIM6, &reg);
            if(AIR_E_OK == rc)
            {
                reg &= ~(AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM6, reg);
            }
            break;

        case 6:
            rc = aml_readReg(unit, PIM7, &reg);
            if(AIR_E_OK == rc)
            {
                reg &= ~(AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM7, reg);
            }
            break;

        default:
            AIR_PRINT("Not Support this dscp %d to pri, rc is %d\n", dscp, rc);
            return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK != rc)
    {
        AIR_PRINT("set dscp to pri failed ,rc is %d\n", rc);
    }
    else
    {
        AIR_PRINT("set dscp  %u to pri %u success, PIM hex is %x\n", dscp, pri, reg);
    }
    return rc;
}

/* FUNCTION NAME: air_qos_getDscp2Pri
 * PURPOSE:
 *      Get DSCP mapping priority.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dscp            --  Select DSCP value (0..63)
 *
 * OUTPUT:
 *      ptr_pri         --  Priority value (0..7)
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getDscp2Pri(
    const UI32_T unit,
    const UI32_T dscp,
    UI32_T * const ptr_pri)
{
    UI32_T rc = AIR_E_OTHERS;
    UI32_T reg = 0;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((dscp >= AIR_QOS_QUEUE_DSCP_MAX_NUM), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_pri);

    /*get register val*/
    switch (dscp/10)
    {
        case 0:
            rc = aml_readReg(unit, PIM1, &reg);
            if(AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10)))
                    >> AIR_QOS_QUEUE_PIM_WIDTH  * (dscp % 10);
            }
            break;

        case 1:
            rc = aml_readReg(unit, PIM2, &reg);
            if(AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10)))
                    >> AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 2:
            rc = aml_readReg(unit, PIM3, &reg);
            if(AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10)))
                    >> AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 3:
            rc = aml_readReg(unit, PIM4, &reg);
            if(AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10)))
                    >> AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 4:
            rc = aml_readReg(unit, PIM5, &reg);
            if(AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10)))
                    >> AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 5:
            rc = aml_readReg(unit, PIM6, &reg);
            if(AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10)))
                    >> AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 6:
            rc = aml_readReg(unit, PIM7, &reg);
            if(AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (AIR_QOS_QUEUE_PIM_MASK << AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10)))
                    >> AIR_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        default:
            AIR_PRINT("Not Support this dscp %d to pri, rc is %d\n", dscp, rc);
            return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_OK != rc)
    {
        AIR_PRINT("[Dbg]: get dscp %d to pri failed, rc is %d\n", dscp, rc);
    }

    AIR_PRINT("[Dbg]: get dscp %u to pri %d success, PIM hex is %d \n", dscp, *ptr_pri, reg);
    return rc;
}

/* FUNCTION NAME: air_qos_setRateLimitEnable
 * PURPOSE:
 *      Enable or disable port rate limit.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number (0..6)
 *      dir             --  AIR_QOS_RATE_DIR_INGRESS
 *                          AIR_QOS_RATE_DIR_EGRESS
 *      rate_en         --  TRUE: eanble rate limit
 *                          FALSE: disable rate limit
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
air_qos_setRateLimitEnable(
    const UI32_T unit,
    const UI32_T port,
    const AIR_QOS_RATE_DIR_T dir,
    const BOOL_T enable)
{
    UI32_T u32dat = 0, reg = 0;
    UI32_T u32glo = 0, greg = 0;
    UI32_T mac_port = 0;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((dir >= AIR_QOS_RATE_DIR_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((enable != TRUE && enable != FALSE), AIR_E_BAD_PARAMETER);

    mac_port = port;
    if(AIR_QOS_RATE_DIR_EGRESS == dir)
    {
        reg = ERLCR(mac_port);
        greg = GERLCR;
    }
    else if (AIR_QOS_RATE_DIR_INGRESS == dir)
    {
        reg = IRLCR(mac_port);
        greg = GIRLCR;
    }
    else
    {
        AIR_PRINT("Not Support this dir %d yet\n", dir);
        return AIR_E_BAD_PARAMETER;
    }

    aml_readReg(unit, reg, &u32dat);
    if(TRUE == enable)
    {
        u32dat |= BIT(REG_RATE_EN_OFFT);
        /* Enable tobke bucket mode */
        u32dat |= BIT(REG_TB_EN_OFFT);
    }
    else
    {
        u32dat &= ~(BIT(REG_RATE_EN_OFFT));
        /* Disable tobke bucket mode */
        u32dat &= ~(BIT(REG_TB_EN_OFFT));
    }
    aml_writeReg(unit, reg, u32dat);

    /* Rate include preamble/IPG/CRC */
    aml_readReg(unit, greg, &u32glo);
    u32glo &= ~(BITS_RANGE(REG_IPG_BYTE_OFFT, REG_IPG_BYTE_LENG));
    u32glo |= AIR_QOS_L1_RATE_LIMIT;
    aml_writeReg(unit, greg, u32glo);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_qos_getRateLimitEnable
 * PURPOSE:
 *      Get port rate limit state.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number (0..6)
 *      dir              -- AIR_QOS_RATE_DIR_T
 * OUTPUT:
 *      ptr_enable        --  TRUE: eanble rate limit
 *                          FALSE: disable rate limit
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getRateLimitEnable(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_QOS_RATE_DIR_T    dir,
    BOOL_T                      *ptr_enable)
{
    UI32_T u32dat = 0, reg = 0, ret = 0;
    UI32_T mac_port = 0;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((dir >= AIR_QOS_RATE_DIR_LAST), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_enable);

    mac_port = port;
    /* Get ingress / egress register value */
    if(AIR_QOS_RATE_DIR_EGRESS == dir)
    {
        reg = ERLCR(mac_port);
    }
    else
    {
        reg = IRLCR(mac_port);
    }
    aml_readReg(unit, reg, &u32dat);

    ret = (u32dat & BIT(REG_RATE_EN_OFFT));
    if(!ret)
    {
        *ptr_enable = FALSE;
    }
    else
    {
        *ptr_enable = TRUE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_qos_setRateLimit
 * PURPOSE:
 *      Set per port rate limit.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      ptr_cfg         --  AIR_QOS_RATE_LIMIT_CFG_T
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
air_qos_setRateLimit(
    const UI32_T unit,
    const UI32_T port,
    AIR_QOS_RATE_LIMIT_CFG_T    *ptr_cfg)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_cfg);
    AIR_PARAM_CHK((ptr_cfg->egress_cbs >= AIR_QOS_MAX_TOKEN), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((ptr_cfg->ingress_cbs >= AIR_QOS_MAX_TOKEN), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((ptr_cfg->egress_cir >= AIR_QOS_MAX_CIR), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((ptr_cfg->ingress_cir >= AIR_QOS_MAX_CIR), AIR_E_BAD_PARAMETER);

    mac_port = port;
    /* For Egress rate setting */
    /* Set egress rate CIR */
    aml_readReg(unit, ERLCR(mac_port), &u32dat);
    u32dat &= ~ BITS_RANGE(REG_RATE_CIR_OFFT, REG_RATE_CIR_LENG);
    u32dat |= ptr_cfg->egress_cir;
    /* Set egress rate CBS */
    u32dat &= ~ BITS_RANGE(REG_RATE_CBS_OFFT, REG_RATE_CBS_LENG);
    u32dat |= BITS_OFF_L(ptr_cfg->egress_cbs, REG_RATE_CBS_OFFT, REG_RATE_CBS_LENG);
    /* Enable tobke bucket mode */
    u32dat |= BIT(REG_TB_EN_OFFT);
    /* Set token period to 4ms */
    u32dat &= ~ BITS_RANGE(REG_RATE_TB_OFFT, REG_RATE_TB_LENG);
    u32dat |= BITS_OFF_L(AIR_QOS_TOKEN_PERIOD_4MS, REG_RATE_TB_OFFT, REG_RATE_TB_LENG);
    if(ptr_cfg->flags & AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_EGRESS)
    {
        /* Enable ratelimit mode*/
        u32dat |= BIT(REG_RATE_EN_OFFT);
    }
    aml_writeReg(unit, ERLCR(mac_port), u32dat);


    /* For Ingress rate setting */
    /* Set ingress rate CIR */
    aml_readReg(unit, IRLCR(mac_port), &u32dat);
    u32dat &= ~ BITS_RANGE(REG_RATE_CIR_OFFT, REG_RATE_CIR_LENG);
    u32dat |= ptr_cfg->ingress_cir;
    /* Set egress rate CBS */
    u32dat &= ~ BITS_RANGE(REG_RATE_CBS_OFFT, REG_RATE_CBS_LENG);
    u32dat |= BITS_OFF_L(ptr_cfg->ingress_cbs, REG_RATE_CBS_OFFT, REG_RATE_CBS_LENG);
    /* Enable tobke bucket mode */
    u32dat |= BIT(REG_TB_EN_OFFT);
    /* Set token period to 4ms */
    u32dat &= ~ BITS_RANGE(REG_RATE_TB_OFFT, REG_RATE_TB_LENG);
    u32dat |= BITS_OFF_L(AIR_QOS_TOKEN_PERIOD_4MS, REG_RATE_TB_OFFT, REG_RATE_TB_LENG);
    if(ptr_cfg->flags & AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_INGRESS)
    {
        /* Enable ratelimit mode*/
        u32dat |= BIT(REG_RATE_EN_OFFT);
    }
    aml_writeReg(unit, IRLCR(mac_port), u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_qos_getRateLimit
 * PURPOSE:
 *      Get per port rate limit.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_cfg          --  AIR_QOS_RATE_LIMIT_CFG_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getRateLimit(
    const UI32_T unit,
    const UI32_T port,
    AIR_QOS_RATE_LIMIT_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_cfg);

    mac_port = port;
    /* For Egress rate info */
    aml_readReg(unit, ERLCR(mac_port), &u32dat);
    ptr_cfg->egress_cir = BITS_OFF_R(u32dat, REG_RATE_CIR_OFFT, REG_RATE_CIR_LENG);
    ptr_cfg->egress_cbs = BITS_OFF_R(u32dat, REG_RATE_CBS_OFFT, REG_RATE_CBS_LENG);

    /* For Ingress rate info */
    aml_readReg(unit, IRLCR(mac_port), &u32dat);
    ptr_cfg->ingress_cir = BITS_OFF_R(u32dat, REG_RATE_CIR_OFFT, REG_RATE_CIR_LENG);
    ptr_cfg->ingress_cbs = BITS_OFF_R(u32dat, REG_RATE_CBS_OFFT, REG_RATE_CBS_LENG);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_qos_setPortPriority
 * PURPOSE:
 *      Get poer port based priority.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      priority        --  Select port priority
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
air_qos_setPortPriority(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T priority)
{
    UI32_T regPCR = 0;
    UI32_T mac_port = 0;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((priority >= AIR_QOS_QUEUE_MAX_NUM), AIR_E_BAD_PARAMETER);
    mac_port = port;
    aml_readReg(unit, PCR(mac_port), &regPCR);
    regPCR &= ~PCR_PORT_PRI_MASK;
    regPCR |= (priority & PCR_PORT_PRI_RELMASK) << PCR_PORT_PRI_OFFT;
    aml_writeReg(unit, PCR(mac_port), regPCR);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_qos_getPortPriority
 * PURPOSE:
 *      Set per port based priority.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_pri         --  Get port based priority
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getPortPriority(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_pri)
{
    UI32_T regPCR = 0;
    UI32_T mac_port = 0;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_pri);
    mac_port = port;
    aml_readReg(unit, PCR(mac_port), &regPCR);
    *ptr_pri = (regPCR >> PCR_PORT_PRI_OFFT) & PCR_PORT_PRI_RELMASK;
    return AIR_E_OK;
}

/* FUNCTION NAME: air_qos_setRateLimitExMngFrm
 * PURPOSE:
 *      Set rate limit control exclude/include management frames.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dir             --  AIR_RATE_DIR_INGRESS
 *                          AIR_RATE_DIR_EGRESS
 *      exclude         --  TRUE: Exclude management frame
 *                          FALSE:Include management frame
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
air_qos_setRateLimitExMngFrm(
    const UI32_T unit,
    const AIR_QOS_RATE_DIR_T dir,
    const BOOL_T exclude)
{
    UI32_T u32dat = 0, reg = 0;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((dir != AIR_QOS_RATE_DIR_EGRESS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != exclude) && (FALSE != exclude)), AIR_E_BAD_PARAMETER);

    reg = GERLCR;
    /* Set to register */
    aml_readReg(unit, reg, &u32dat);
    if(TRUE == exclude)
    {
        u32dat |= BIT(REG_MFRM_EX_OFFT);
    }
    else
    {
        u32dat &= ~(BIT(REG_MFRM_EX_OFFT));
    }
    aml_writeReg(unit, reg, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_qos_getRateLimitExMngFrm
 * PURPOSE:
 *      Get rate limit control exclude/include management frames.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dir             --  AIR_RATE_DIR_INGRESS
 *                          AIR_RATE_DIR_EGRESS
 * OUTPUT:
 *      ptr_exclude     --  TRUE: Exclude management frame
 *                          FALSE:Include management frame
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getRateLimitExMngFrm(
    const UI32_T unit,
    const AIR_QOS_RATE_DIR_T dir,
    BOOL_T *ptr_exclude)
{
    UI32_T reg = 0, u32dat = 0;

    /* Check parameter */
    AIR_PARAM_CHK((unit >= AIR_MAX_NUM_OF_UNIT), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((dir >= AIR_QOS_RATE_DIR_LAST), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_exclude);
    

    if(AIR_QOS_RATE_DIR_EGRESS == dir)
    {
        reg = GERLCR;
    }
    else
    {
        reg = GIRLCR;
    }

    /* Set to register */
    aml_readReg(unit, reg, &u32dat);
    if(BITS_OFF_R(u32dat, REG_MFRM_EX_OFFT, REG_MFRM_EX_LENG))
    {
        *ptr_exclude = TRUE;
    }
    else
    {
        *ptr_exclude = FALSE;
    }

    return AIR_E_OK;
}


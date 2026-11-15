/* FILE NAME: air_qos.h
* PURPOSE:
 *      Define the Quailty of Service function in AIR SDK.
*
* NOTES:
*       None
*/

#ifndef AIR_QOS_H
#define AIR_QOS_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/
#define AIR_QOS_MAX_TOKEN               (128)
#define AIR_QOS_MAX_CIR                 (80001)
#define AIR_QOS_TOKEN_PERIOD_1_4MS      (5)
#define AIR_QOS_TOKEN_PERIOD_4MS        (9)
#define AIR_QOS_L1_RATE_LIMIT           (0x18)
#define AIR_QOS_L2_RATE_LIMIT           (0x04)
#define AIR_QOS_QUEUE_PIM_WIDTH         (3)
#define AIR_QOS_QUEUE_PIM_MASK          (7)
#define AIR_QOS_QUEUE_DEFAULT_VAL       (0x222227)
#define AIR_QOS_QUEUE_TRUST_HIGH_WEIGHT (6)
#define AIR_QOS_QUEUE_TRUST_MID_WEIGHT  (5)
#define AIR_QOS_QUEUE_TRUST_LOW_WEIGHT  (4)
#define AIR_QOS_SHAPER_RATE_MAX_EXP     (4)
#define AIR_QOS_SHAPER_RATE_MAX_MAN     (0x1ffff)
#define AIR_QOS_SHAPER_RATE_MIN_WEIGHT  (1)
#define AIR_QOS_SHAPER_RATE_MAX_WEIGHT  (128)
#define AIR_QOS_QUEUE_0                 (0)
#define AIR_QOS_QUEUE_1                 (1)
#define AIR_QOS_QUEUE_2                 (2)
#define AIR_QOS_QUEUE_3                 (3)
#define AIR_QOS_QUEUE_4                 (4)
#define AIR_QOS_QUEUE_5                 (5)
#define AIR_QOS_QUEUE_6                 (6)
#define AIR_QOS_QUEUE_7                 (7)
#define AIR_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_SP                (1)
#define AIR_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_WRR               (0)
#define AIR_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_SP                (1)
#define AIR_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_WFQ               (0)
#define AIR_QOS_MAX_EXCESS_SP                                    (1)
#define AIR_QOS_MAX_EXCESS_DROP                                  (0)
#define AIR_QOS_QUEUE_MAX_NUM           (8)
#define AIR_QOS_QUEUE_DSCP_MAX_NUM      (64)
#define AIR_MAX_NUM_OF_QUEUE            (8) /*need to be replaced by AIR_QOS_QUEUE_MAX_NUM*/

#define AIR_QOS_SHAPER_NOSETTING      (0xffffffff)
#define AIR_QOS_SHAPER_RATE_MIN_WEIGHT  (1)
#define AIR_QOS_SHAPER_RATE_MAX_WEIGHT  (128)

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef enum
{
    AIR_QOS_SCH_MODE_SP,
    AIR_QOS_SCH_MODE_WRR,
    AIR_QOS_SCH_MODE_WFQ,
    AIR_QOS_SCH_MODE_LAST,
} AIR_QOS_SCH_MODE_T;

typedef union AIR_QOS_SHAPER_MIN_S
{
    struct
    {
        UI32_T   min_rate_man      :17;
        UI32_T   min_reserve       :2;
        UI32_T   min_rate_en       :1;
        UI32_T   min_rate_exp      :4;
        UI32_T   min_weight        :7;
        UI32_T   min_sp_wrr_q      :1;
    }raw;
    UI32_T byte;
}AIR_QOS_SHAPER_MIN_T;

typedef union AIR_QOS_SHAPER_MAX_S
{
    struct
    {
        UI32_T   max_rate_man      :17;
        UI32_T   max_reserve       :1;
        UI32_T   max_excess_en     :1;
        UI32_T   max_rate_en       :1;
        UI32_T   max_rate_exp      :4;
        UI32_T   max_weight        :7;
        UI32_T   max_sp_wfq_q      :1;
    }raw;
    UI32_T byte;
}AIR_QOS_SHAPER_MAX_T;

typedef enum
{
    /* The packet priority is based on port's priority. */
    AIR_QOS_TRUST_MODE_PORT,

    /*
    * The precedence of packet priority is based on 802.1p,
    * then port's priority.
    */
    AIR_QOS_TRUST_MODE_1P_PORT,

    /*
    * The precedence of packet priority is based on DSCP,
    * then port's priority.
    */
    AIR_QOS_TRUST_MODE_DSCP_PORT,

    /*
    * The precedence of packet priority is based on DSCP, 802.1p,
    * then port's priority.
    */
    AIR_QOS_TRUST_MODE_DSCP_1P_PORT,
    AIR_QOS_TRUST_MODE_LAST
} AIR_QOS_TRUST_MODE_T;

typedef union AIR_QOS_QUEUE_UPW_S
{
    struct
    {
        UI32_T csr_acl_weight   :3;
        UI32_T                  :1;
        UI32_T csr_stag_weight  :3;/*Not use yet*/
        UI32_T                  :1;
        UI32_T csr_1p_weight    :3;
        UI32_T                  :1;
        UI32_T csr_dscp_weight  :3;
        UI32_T                  :1;
        UI32_T csr_port_weight  :3;
        UI32_T                  :1;
        UI32_T csr_arl_weight   :3;
        UI32_T                  :9;
    }raw;
    UI32_T byte;
}AIR_QOS_QUEUE_UPW_T;

typedef union AIR_QOS_QUEUE_PEM_S
{
    struct
    {
        UI32_T csr_dscp_pri_l     :6;/*Not use yet*/
        UI32_T csr_que_lan_l      :2;/*Not use yet*/
        UI32_T csr_que_cpu_l      :3;
        UI32_T csr_tag_pri_l      :3;/*Not use yet*/
        UI32_T                    :2;
        UI32_T csr_dscp_pri_h     :6;/*Not use yet*/
        UI32_T csr_que_lan_h      :2;/*Not use yet*/
        UI32_T csr_que_cpu_h      :3;
        UI32_T csr_tag_pri_h      :3;/*Not use yet*/
        UI32_T                    :2;
    }raw;
    UI32_T byte;
}AIR_QOS_QUEUE_PEM_T;

typedef enum
{
    AIR_QOS_RATE_DIR_INGRESS,
    AIR_QOS_RATE_DIR_EGRESS,
    AIR_QOS_RATE_DIR_LAST
} AIR_QOS_RATE_DIR_T;

typedef struct AIR_RATE_LIMIT_S
{
#define AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_INGRESS     (1U << 0)
#define AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_EGRESS      (1U << 1)
    UI32_T flags;

    /*
    * The limit cover up to 2.5G
    * Rate = CIR * 32kbps
    * Range = 0..80000
    */
    UI32_T ingress_cir;
    UI32_T egress_cir;

    /*
    * Bucket = Max{(CBS * 512), (CIR * 2500)} Bytes
    * Range: 0..127
    */
    UI32_T ingress_cbs;
    UI32_T egress_cbs;
} AIR_QOS_RATE_LIMIT_CFG_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const UI32_T                weight);


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
    UI32_T                *ptr_weight);

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
    const AIR_QOS_TRUST_MODE_T      mode);


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
    const UI32_T port,
    AIR_QOS_TRUST_MODE_T *const ptr_mode);

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
    const UI32_T queue);

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
    UI32_T *const ptr_queue);

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
    const UI32_T pri);

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
    UI32_T * const ptr_pri);

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
    const BOOL_T enable);

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
    BOOL_T                      *ptr_enable);

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
    AIR_QOS_RATE_LIMIT_CFG_T    *ptr_cfg);

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
    AIR_QOS_RATE_LIMIT_CFG_T *ptr_cfg);

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
    const UI32_T priority);

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
    UI32_T *ptr_pri);

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
    const BOOL_T exclude);

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
    BOOL_T *ptr_exclude);

#endif /* End of AIR_QOS_H */

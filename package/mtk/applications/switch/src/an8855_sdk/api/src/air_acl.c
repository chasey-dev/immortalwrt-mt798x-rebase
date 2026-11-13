/* FILE NAME: air_acl.c
 * PURPOSE:
 *      Define the ACL function in AIR SDK.
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
#define ACL_DERIVE_TBL_MULTIFIELDS(data_buffer, offset, width, dst)             \
({                                                                              \
    UI32_T value = 0;                                                           \
    _deriveTblMultiFields((data_buffer), (offset), (width), &value);            \
    dst = value;                                                                \
})

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
static AIR_ERROR_NO_T
_checkDone(
    const UI32_T unit,
    const AIR_ACL_CHECK_TYPE_T type)
{
    UI32_T check_bit = 0, i = 0, reg = 0, value = 0, offset = 0;

    switch(type)
    {
        case AIR_ACL_CHECK_ACL:
            check_bit = 1;
            reg = ACL_MEM_CFG;
            offset = ACL_MEM_CFG_DONE_OFFSET;
            break;
        case AIR_ACL_CHECK_UDF:
            check_bit = 0;
            reg = ACL_AUTC;
            offset = ACL_UDF_ACC_OFFSET;
            break;
        case AIR_ACL_CHECK_TRTCM:
            check_bit = 0;
            reg = ACL_TRTCMA;
            offset = ACL_TRTCM_BUSY_OFFSET;
            break;
        case AIR_ACL_CHECK_METER:
            check_bit = 0;
            reg = ACLRMC;
            offset = ACL_RATE_BUSY_OFFSET;
            break;
        default:
            return AIR_E_BAD_PARAMETER;
    }
    for(i=0; i < ACL_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, reg, &value);
        if (check_bit == (value >> offset))
        {
            break;
        }
        AIR_UDELAY(1);
    }
    if(i >= ACL_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }
    return AIR_E_OK;
}

static void
_convertToTCcell(
    const UI32_T *data0,
    const UI32_T *data1,
    UI32_T *arr0,
    UI32_T *arr1,
    UI32_T size)
{
    UI32_T i = 0;

    for(i = 0; i < size; i++)
    {
        arr0[i] = data0[i] | (~data1[i]);
        arr1[i] = (~data0[i]) | (~data1[i]);
    }
}

static void
_parseFromTCcell(
    const UI32_T *data0,
    const UI32_T *data1,
    UI32_T *arr0,
    UI32_T *arr1,
    UI32_T size)
{
    UI32_T i = 0;

    for(i = 0; i < size; i++)
    {
        arr1[i] = ~(data0[i] & data1[i]);
        arr0[i] = data0[i] | (~arr1[i]);
    }
}

static int
_fillTblMultiFields(
    UI32_T          *data_buffer,
    UI32_T          data_count,
    const UI32_T    offset,
    const UI32_T    width,
    const UI32_T    value)
{
    UI32_T data_index = 0, bit_index = 0;
    UI32_T extended_data[2] = {0};
    UI32_T extended_mask[2] = {0};
    UI32_T msk;
    UI32_T val;

    AIR_CHECK_PTR(data_buffer);

    if((0 == data_count) || (0 == width) || (width > 32) || (offset+width > data_count*32))
    {
        return 0;
    }

    msk = ((1U<<(width-1U))<<1U)-1U;
    val = value & msk;
    data_index = offset / 32;
    bit_index = offset % 32;

    extended_data[0] = val << bit_index;
    extended_data[1] = (val >> (31U-bit_index))>>1U;
    extended_mask[0] = msk << bit_index;
    extended_mask[1] = (msk >> (31U-bit_index))>>1U;

    data_buffer[data_index] = (data_buffer[data_index] & ~extended_mask[0]) | extended_data[0];
    if ((data_index+1)<data_count)
    {
        data_buffer[data_index+1] = (data_buffer[data_index+1] & ~extended_mask[1]) | extended_data[1];
    }

    return 0;
}

static int
_deriveTblMultiFields(
    UI32_T          *data_buffer,
    const UI32_T    offset,
    const UI32_T    width,
    UI32_T          *ptr_value)
{
    UI32_T data_index = 0, bit_index = 0;
    UI32_T extended_data[2] = {0};
    UI32_T extended_mask[2] = {0};
    UI32_T msk = 0;

    AIR_CHECK_PTR(data_buffer);
    AIR_CHECK_PTR(ptr_value);

    if(width==0 || width>32)
    {
        return 0;
    }
    msk = ((1U<<(width-1U))<<1U)-1U;
    data_index = offset / 32;
    bit_index = offset % 32;

    extended_mask[0] = msk << bit_index;
    extended_mask[1] = (msk >> (31U-bit_index))>>1U;
    extended_data[0] = (data_buffer[data_index] & extended_mask[0]) >> bit_index;
    extended_data[1] = ((data_buffer[data_index+1] & extended_mask[1]) << (31U-bit_index))<<1U;

    *ptr_value = extended_data[0] | extended_data[1];
    return 0;
}

static void
_air_acl_setRuleTable(
    const AIR_ACL_RULE_TYPE_T type,
    const BOOL_T iskey,
    const AIR_ACL_FIELD_T *ptr_field,
    UI32_T *data)
{
    UI32_T n = 0;

    switch(type)
    {
        case AIR_ACL_RULE_TYPE_0:
            if(TRUE == iskey)
            {
                _fillTblMultiFields(data, 12, RULE_TYPE0_OFFSET, RULE_TYPE0_WIDTH, 0);
            }
            else
            {
                _fillTblMultiFields(data, 12, RULE_TYPE0_OFFSET, RULE_TYPE0_WIDTH, 1);
            }
            for(n=0; n<6; n++)
            {
                _fillTblMultiFields(data, 12, DMAC_OFFSET + DMAC_WIDTH*(5-n), DMAC_WIDTH, ptr_field->dmac[n]);
            }
            for(n=0; n<6; n++)
            {
                _fillTblMultiFields(data, 12, SMAC_OFFSET + SMAC_WIDTH*(5-n), SMAC_WIDTH, ptr_field->smac[n]);
            }
            _fillTblMultiFields(data, 12, STAG_OFFSET, STAG_WIDTH, ptr_field->stag);
            _fillTblMultiFields(data, 12, CTAG_OFFSET, CTAG_WIDTH, ptr_field->ctag);
            _fillTblMultiFields(data, 12, ETYPE_OFFSET, ETYPE_WIDTH, ptr_field->etype);
            _fillTblMultiFields(data, 12, DIP_OFFSET, DIP_WIDTH, ptr_field->dip[0]);
            _fillTblMultiFields(data, 12, SIP_OFFSET, SIP_WIDTH, ptr_field->sip[0]);
            _fillTblMultiFields(data, 12, DSCP_OFFSET, DSCP_WIDTH, ptr_field->dscp);
            _fillTblMultiFields(data, 12, PROTOCOL_OFFSET, PROTOCOL_WIDTH, ptr_field->protocol);
            _fillTblMultiFields(data, 12, DPORT_OFFSET, DPORT_WIDTH, ptr_field->dport);
            _fillTblMultiFields(data, 12, SPORT_OFFSET, SPORT_WIDTH, ptr_field->sport);
            _fillTblMultiFields(data, 12, UDF_OFFSET, UDF_WIDTH, ptr_field->udf);
            _fillTblMultiFields(data, 12, FIELDMAP_OFFSET, FIELDMAP_WIDTH, ptr_field->fieldmap);
            _fillTblMultiFields(data, 12, IS_IPV6_OFFSET, IS_IPV6_WIDTH, ptr_field->isipv6);
            _fillTblMultiFields(data, 12, PORTMAP_OFFSET, PORTMAP_WIDTH, ptr_field->portmap);
            break;
         case AIR_ACL_RULE_TYPE_1:
            _fillTblMultiFields(data, 12, RULE_TYPE1_OFFSET, RULE_TYPE1_WIDTH, 1);
            for(n=1; n<4; n++)
            {
                _fillTblMultiFields(data, 12, DIP_IPV6_OFFSET + DIP_IPV6_WIDTH*(n-1), DIP_IPV6_WIDTH, ptr_field->dip[n]);
            }
            for(n=1; n<4; n++)
            {
                _fillTblMultiFields(data, 12, SIP_IPV6_OFFSET + SIP_IPV6_WIDTH*(n-1), SIP_IPV6_WIDTH, ptr_field->sip[n]);
            }
            _fillTblMultiFields(data, 12, FLOW_LABEL_OFFSET, FLOW_LABEL_WIDTH, ptr_field->flow_label);
            break;
        default:
            return;
    }
}

static void
_air_acl_getRuleTable(
    const AIR_ACL_RULE_TYPE_T type,
    UI32_T *data,
    AIR_ACL_FIELD_T *ptr_field)
{
    UI32_T n = 0;

    switch(type)
    {
        case AIR_ACL_RULE_TYPE_0:
            for(n=0; n<6; n++)
            {
                ACL_DERIVE_TBL_MULTIFIELDS(data, DMAC_OFFSET + DMAC_WIDTH*(5-n), DMAC_WIDTH, ptr_field->dmac[n]);
            }
            for(n=0; n<6; n++)
            {
                ACL_DERIVE_TBL_MULTIFIELDS(data, SMAC_OFFSET + SMAC_WIDTH*(5-n), SMAC_WIDTH, ptr_field->smac[n]);
            }
            ACL_DERIVE_TBL_MULTIFIELDS(data, STAG_OFFSET, STAG_WIDTH, ptr_field->stag);
            ACL_DERIVE_TBL_MULTIFIELDS(data, CTAG_OFFSET, CTAG_WIDTH, ptr_field->ctag);
            ACL_DERIVE_TBL_MULTIFIELDS(data, ETYPE_OFFSET, ETYPE_WIDTH, ptr_field->etype);
            ACL_DERIVE_TBL_MULTIFIELDS(data, DIP_OFFSET, DIP_WIDTH, ptr_field->dip[0]);
            ACL_DERIVE_TBL_MULTIFIELDS(data, SIP_OFFSET, SIP_WIDTH, ptr_field->sip[0]);
            ACL_DERIVE_TBL_MULTIFIELDS(data, DSCP_OFFSET, DSCP_WIDTH, ptr_field->dscp);
            ACL_DERIVE_TBL_MULTIFIELDS(data, PROTOCOL_OFFSET, PROTOCOL_WIDTH, ptr_field->protocol);
            ACL_DERIVE_TBL_MULTIFIELDS(data, DPORT_OFFSET, DPORT_WIDTH, ptr_field->dport);
            ACL_DERIVE_TBL_MULTIFIELDS(data, SPORT_OFFSET, SPORT_WIDTH, ptr_field->sport);
            ACL_DERIVE_TBL_MULTIFIELDS(data, UDF_OFFSET, UDF_WIDTH, ptr_field->udf);
            ACL_DERIVE_TBL_MULTIFIELDS(data, FIELDMAP_OFFSET, FIELDMAP_WIDTH, ptr_field->fieldmap);
            ACL_DERIVE_TBL_MULTIFIELDS(data, IS_IPV6_OFFSET, IS_IPV6_WIDTH, ptr_field->isipv6);
            ACL_DERIVE_TBL_MULTIFIELDS(data, PORTMAP_OFFSET, PORTMAP_WIDTH, ptr_field->portmap);
            break;
         case AIR_ACL_RULE_TYPE_1:
            for(n=1; n<4; n++)
            {
                ACL_DERIVE_TBL_MULTIFIELDS(data, DIP_IPV6_OFFSET + DIP_IPV6_WIDTH*(n-1), DIP_IPV6_WIDTH, ptr_field->dip[n]);
            }
            for(n=1; n<4; n++)
            {
                ACL_DERIVE_TBL_MULTIFIELDS(data, SIP_IPV6_OFFSET + SIP_IPV6_WIDTH*(n-1), SIP_IPV6_WIDTH, ptr_field->sip[n]);
            }
            ACL_DERIVE_TBL_MULTIFIELDS(data, FLOW_LABEL_OFFSET, FLOW_LABEL_WIDTH, ptr_field->flow_label);
            break;
        default:
            return;
    }
}

static void
_air_acl_setActionTable(
    const AIR_ACL_ACTION_T *ptr_action,
    UI32_T *data)
{
    int i = 0;

    _fillTblMultiFields(data, 4, PORT_FORCE_OFFSET, PORT_FORCE_WIDTH, ptr_action->port_en);
    _fillTblMultiFields(data, 4, VLAN_PORT_SWAP_OFFSET, VLAN_PORT_SWAP_WIDTH, ptr_action->vlan_port_sel);
    _fillTblMultiFields(data, 4, DST_PORT_SWAP_OFFSET, DST_PORT_SWAP_WIDTH, ptr_action->dest_port_sel);
    _fillTblMultiFields(data, 4, PORT_OFFSET, PORT_WIDTH, ptr_action->portmap);

    _fillTblMultiFields(data, 4, ACL_MIB_EN_OFFSET, ACL_MIB_EN_WIDTH, ptr_action->cnt_en);
    _fillTblMultiFields(data, 4, ACL_MIB_ID_OFFSET, ACL_MIB_ID_WIDTH, ptr_action->cnt_idx);

    _fillTblMultiFields(data, 4, ATTACK_RATE_EN_OFFSET, ATTACK_RATE_EN_WIDTH, ptr_action->attack_en);
    _fillTblMultiFields(data, 4, ATTACK_RATE_ID_OFFSET, ATTACK_RATE_ID_WIDTH, ptr_action->attack_idx);

    _fillTblMultiFields(data, 4, RATE_EN_OFFSET, RATE_EN_WIDTH, ptr_action->rate_en);
    _fillTblMultiFields(data, 4, RATE_INDEX_OFFSET, RATE_INDEX_WIDTH, ptr_action->rate_idx);

    _fillTblMultiFields(data, 4, PORT_FW_EN_OFFSET, PORT_FW_EN_WIDTH, ptr_action->fwd_en);
    _fillTblMultiFields(data, 4, FW_PORT_OFFSET, FW_PORT_WIDTH, ptr_action->fwd);

    _fillTblMultiFields(data, 4, MIRROR_OFFSET, MIRROR_WIDTH, ptr_action->mirrormap);

    _fillTblMultiFields(data, 4, PRI_USER_EN_OFFSET, PRI_USER_EN_WIDTH, ptr_action->pri_user_en);
    _fillTblMultiFields(data, 4, PRI_USER_OFFSET, PRI_USER_WIDTH, ptr_action->pri_user);

    _fillTblMultiFields(data, 4, EG_TAG_EN_OFFSET, EG_TAG_EN_WIDTH, ptr_action->egtag_en);
    _fillTblMultiFields(data, 4, EG_TAG_OFFSET, EG_TAG_WIDTH, ptr_action->egtag);

    _fillTblMultiFields(data, 4, LKY_VLAN_EN_OFFSET, LKY_VLAN_EN_WIDTH, ptr_action->lyvlan_en);
    _fillTblMultiFields(data, 4, LKY_VLAN_OFFSET, LKY_VLAN_WIDTH, ptr_action->lyvlan);

    _fillTblMultiFields(data, 4, BPDU_OFFSET, BPDU_WIDTH, ptr_action->bpdu);

    _fillTblMultiFields(data, 4, ACL_MANG_OFFSET, ACL_MANG_WIDTH, ptr_action->mang);

    _fillTblMultiFields(data, 4, TRTCM_EN_OFFSET, TRTCM_EN_WIDTH, ptr_action->trtcm_en);
    _fillTblMultiFields(data, 4, DROP_PCD_SEL_OFFSET, DROP_PCD_SEL_WIDTH, ptr_action->trtcm.drop_pcd_sel);
    _fillTblMultiFields(data, 4, ACL_DROP_PCD_R_OFFSET, ACL_DROP_PCD_R_WIDTH, ptr_action->trtcm.drop_pcd_r);
    _fillTblMultiFields(data, 4, ACL_DROP_PCD_Y_OFFSET, ACL_DROP_PCD_Y_WIDTH, ptr_action->trtcm.drop_pcd_y);
    _fillTblMultiFields(data, 4, ACL_DROP_PCD_G_OFFSET, ACL_DROP_PCD_G_WIDTH, ptr_action->trtcm.drop_pcd_g);
    _fillTblMultiFields(data, 4, CLASS_SLR_SEL_OFFSET, CLASS_SLR_SEL_WIDTH, ptr_action->trtcm.cls_slr_sel);
    _fillTblMultiFields(data, 4, CLASS_SLR_OFFSET, CLASS_SLR_WIDTH, ptr_action->trtcm.cls_slr);
    _fillTblMultiFields(data, 4, ACL_TCM_SEL_OFFSET, ACL_TCM_SEL_WIDTH, ptr_action->trtcm.tcm_sel);
    _fillTblMultiFields(data, 4, ACL_TCM_OFFSET, ACL_TCM_WIDTH, ptr_action->trtcm.usr_tcm);
    _fillTblMultiFields(data, 4, ACL_CLASS_IDX_OFFSET, ACL_CLASS_IDX_WIDTH, ptr_action->trtcm.tcm_idx);

    _fillTblMultiFields(data, 4, ACL_VLAN_HIT_OFFSET, ACL_VLAN_HIT_WIDTH, ptr_action->vlan_en);
    _fillTblMultiFields(data, 4, ACL_VLAN_VID_OFFSET, ACL_VLAN_VID_WIDTH, ptr_action->vlan_idx);
}

static void
_air_acl_getActionTable(
    UI32_T *data,
    AIR_ACL_ACTION_T *ptr_action)
{
    ACL_DERIVE_TBL_MULTIFIELDS(data, PORT_FORCE_OFFSET, PORT_FORCE_WIDTH, ptr_action->port_en);
    ACL_DERIVE_TBL_MULTIFIELDS(data, VLAN_PORT_SWAP_OFFSET, VLAN_PORT_SWAP_WIDTH, ptr_action->vlan_port_sel);
    ACL_DERIVE_TBL_MULTIFIELDS(data, DST_PORT_SWAP_OFFSET, DST_PORT_SWAP_WIDTH, ptr_action->dest_port_sel);
    ACL_DERIVE_TBL_MULTIFIELDS(data, PORT_OFFSET, PORT_WIDTH, ptr_action->portmap);

    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_MIB_EN_OFFSET, ACL_MIB_EN_WIDTH, ptr_action->cnt_en);
    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_MIB_ID_OFFSET, ACL_MIB_ID_WIDTH, ptr_action->cnt_idx);

    ACL_DERIVE_TBL_MULTIFIELDS(data, ATTACK_RATE_EN_OFFSET, ATTACK_RATE_EN_WIDTH, ptr_action->attack_en);
    ACL_DERIVE_TBL_MULTIFIELDS(data, ATTACK_RATE_ID_OFFSET, ATTACK_RATE_ID_WIDTH, ptr_action->attack_idx);

    ACL_DERIVE_TBL_MULTIFIELDS(data, RATE_EN_OFFSET, RATE_EN_WIDTH, ptr_action->rate_en);
    ACL_DERIVE_TBL_MULTIFIELDS(data, RATE_INDEX_OFFSET, RATE_INDEX_WIDTH, ptr_action->rate_idx);

    ACL_DERIVE_TBL_MULTIFIELDS(data, PORT_FW_EN_OFFSET, PORT_FW_EN_WIDTH, ptr_action->fwd_en);
    ACL_DERIVE_TBL_MULTIFIELDS(data, FW_PORT_OFFSET, FW_PORT_WIDTH, ptr_action->fwd);

    ACL_DERIVE_TBL_MULTIFIELDS(data, MIRROR_OFFSET, MIRROR_WIDTH, ptr_action->mirrormap);

    ACL_DERIVE_TBL_MULTIFIELDS(data, PRI_USER_EN_OFFSET, PRI_USER_EN_WIDTH, ptr_action->pri_user_en);
    ACL_DERIVE_TBL_MULTIFIELDS(data, PRI_USER_OFFSET, PRI_USER_WIDTH, ptr_action->pri_user);

    ACL_DERIVE_TBL_MULTIFIELDS(data, EG_TAG_EN_OFFSET, EG_TAG_EN_WIDTH, ptr_action->egtag_en);
    ACL_DERIVE_TBL_MULTIFIELDS(data, EG_TAG_OFFSET, EG_TAG_WIDTH, ptr_action->egtag);

    ACL_DERIVE_TBL_MULTIFIELDS(data, LKY_VLAN_EN_OFFSET, LKY_VLAN_EN_WIDTH, ptr_action->lyvlan_en);
    ACL_DERIVE_TBL_MULTIFIELDS(data, LKY_VLAN_OFFSET, LKY_VLAN_WIDTH, ptr_action->lyvlan);

    ACL_DERIVE_TBL_MULTIFIELDS(data, BPDU_OFFSET, BPDU_WIDTH, ptr_action->bpdu);

    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_MANG_OFFSET, ACL_MANG_WIDTH, ptr_action->mang);

    ACL_DERIVE_TBL_MULTIFIELDS(data, TRTCM_EN_OFFSET, TRTCM_EN_WIDTH, ptr_action->trtcm_en);
    ACL_DERIVE_TBL_MULTIFIELDS(data, DROP_PCD_SEL_OFFSET, DROP_PCD_SEL_WIDTH, ptr_action->trtcm.drop_pcd_sel);
    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_DROP_PCD_R_OFFSET, ACL_DROP_PCD_R_WIDTH, ptr_action->trtcm.drop_pcd_r);
    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_DROP_PCD_Y_OFFSET, ACL_DROP_PCD_Y_WIDTH, ptr_action->trtcm.drop_pcd_y);
    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_DROP_PCD_G_OFFSET, ACL_DROP_PCD_G_WIDTH, ptr_action->trtcm.drop_pcd_g);
    ACL_DERIVE_TBL_MULTIFIELDS(data, CLASS_SLR_SEL_OFFSET, CLASS_SLR_SEL_WIDTH, ptr_action->trtcm.cls_slr_sel);
    ACL_DERIVE_TBL_MULTIFIELDS(data, CLASS_SLR_OFFSET, CLASS_SLR_WIDTH, ptr_action->trtcm.cls_slr);
    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_TCM_SEL_OFFSET, ACL_TCM_SEL_WIDTH, ptr_action->trtcm.tcm_sel);
    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_TCM_OFFSET, ACL_TCM_WIDTH, ptr_action->trtcm.usr_tcm);
    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_CLASS_IDX_OFFSET, ACL_CLASS_IDX_WIDTH, ptr_action->trtcm.tcm_idx);

    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_VLAN_HIT_OFFSET, ACL_VLAN_HIT_WIDTH, ptr_action->vlan_en);
    ACL_DERIVE_TBL_MULTIFIELDS(data, ACL_VLAN_VID_OFFSET, ACL_VLAN_VID_WIDTH, ptr_action->vlan_idx);

}

static AIR_ERROR_NO_T
_air_acl_writeReg(
    const UI32_T unit,
    const UI32_T rule_idx,
    const UI32_T block_num,
    const AIR_ACL_RULE_TCAM_T type,
    const AIR_ACL_MEM_SEL_T sel,
    const AIR_ACL_MEM_FUNC_T func,
    const UI32_T *data)
{
    UI32_T bn = 0, value = 0;
    AIR_ERROR_NO_T ret = AIR_E_OK;

    for (bn = 0; bn < block_num; bn++)
    {
        if(AIR_E_TIMEOUT == _checkDone(unit, AIR_ACL_CHECK_ACL))
        {
            return AIR_E_TIMEOUT;
        }
        aml_writeReg(unit, ACL_MEM_CFG_WDATA0, data[bn*4]);
        aml_writeReg(unit, ACL_MEM_CFG_WDATA1, data[bn*4+1]);
        aml_writeReg(unit, ACL_MEM_CFG_WDATA2, data[bn*4+2]);
        aml_writeReg(unit, ACL_MEM_CFG_WDATA3, data[bn*4+3]);

        value = (rule_idx << ACL_MEM_CFG_RULE_ID_OFFSET) | (type << ACL_MEM_CFG_TCAM_CELL_OFFSET) |
            (bn << ACL_MEM_CFG_DATA_BN_OFFSET) | (sel << ACL_MEM_CFG_MEM_SEL_OFFSET) |
            (func << ACL_MEM_CFG_FUNC_SEL_OFFSET) | ACL_MEM_CFG_EN;
        if ((ret = aml_writeReg(unit, ACL_MEM_CFG, value)) != AIR_E_OK)
        {
            return ret;
        }
    }
    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_air_acl_readReg(
    const UI32_T unit,
    const UI32_T rule_idx,
    const UI32_T block_num,
    const AIR_ACL_RULE_TCAM_T type,
    const AIR_ACL_MEM_SEL_T sel,
    const AIR_ACL_MEM_FUNC_T func,
    UI32_T *data)
{
    UI32_T bn = 0, value = 0;
    AIR_ERROR_NO_T ret = AIR_E_OK;

    for (bn = 0; bn < block_num; bn++)
    {
        value = (rule_idx << ACL_MEM_CFG_RULE_ID_OFFSET) | (type << ACL_MEM_CFG_TCAM_CELL_OFFSET) |
            (bn << ACL_MEM_CFG_DATA_BN_OFFSET) | (sel << ACL_MEM_CFG_MEM_SEL_OFFSET) |
            (func << ACL_MEM_CFG_FUNC_SEL_OFFSET) | ACL_MEM_CFG_EN;
        if ((ret = aml_writeReg(unit, ACL_MEM_CFG, value)) != AIR_E_OK)
        {
            return ret;
        }
        if(AIR_E_TIMEOUT == _checkDone(unit, AIR_ACL_CHECK_ACL))
        {
            return AIR_E_TIMEOUT;
        }
        aml_readReg(unit, ACL_MEM_CFG_RDATA0, data+bn*4);
        aml_readReg(unit, ACL_MEM_CFG_RDATA1, data+bn*4+1);
        aml_readReg(unit, ACL_MEM_CFG_RDATA2, data+bn*4+2);
        aml_readReg(unit, ACL_MEM_CFG_RDATA3, data+bn*4+3);
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_setRuleCtrl
 * PURPOSE:
 *      Set ACL rule control.
 *
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 *      ptr_rule        --  Structure of ACL rule control
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
air_acl_setRuleCtrl(
    const UI32_T unit,
    const UI32_T rule_idx,
    AIR_ACL_CTRL_T *ptr_ctrl)
{
    UI32_T data_en[4] = {0}, data_end[4] = {0}, data_rev[4] = {0};

    if(TRUE == ptr_ctrl->rule_en)
    {
        _air_acl_readReg(unit, AIR_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_READ, data_en);
        data_en[rule_idx/32] |= (1 << (rule_idx%32));
        _air_acl_writeReg(unit, AIR_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_WRITE, data_en);
    }
    else
    {
        _air_acl_readReg(unit, AIR_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_READ, data_en);
        data_en[rule_idx/32] &= ~(1 << (rule_idx%32));
        _air_acl_writeReg(unit, AIR_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_WRITE, data_en);
    }

    if(TRUE == ptr_ctrl->end)
    {
        _air_acl_readReg(unit, AIR_ACL_RULE_CONFIG_END, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_READ, data_end);
        data_end[rule_idx/32] |= (1 << (rule_idx%32));
        _air_acl_writeReg(unit, AIR_ACL_RULE_CONFIG_END, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_WRITE, data_end);
    }
    else
    {
        _air_acl_readReg(unit, AIR_ACL_RULE_CONFIG_END, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_READ, data_end);
        data_end[rule_idx/32] &= ~(1 << (rule_idx%32));
        _air_acl_writeReg(unit, AIR_ACL_RULE_CONFIG_END, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_WRITE, data_end);

    }

    if(TRUE == ptr_ctrl->reverse)
    {
        _air_acl_readReg(unit, AIR_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_READ, data_rev);
        data_rev[rule_idx/32] |= (1 << (rule_idx%32));
        _air_acl_writeReg(unit, AIR_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_WRITE, data_rev);
    }
    else
    {
        _air_acl_readReg(unit, AIR_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_READ, data_rev);
        data_rev[rule_idx/32] &= ~(1 << (rule_idx%32));
        _air_acl_writeReg(unit, AIR_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_WRITE, data_rev);
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_getRuleCtrl
 * PURPOSE:
 *      Get ACL rule control.
 *
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 *
 * OUTPUT:
 *      ptr_ctrl        --  Structure of ACL rule control
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
air_acl_getRuleCtrl(
    const UI32_T unit,
    const UI32_T rule_idx,
    AIR_ACL_CTRL_T *ptr_ctrl)
{
    UI32_T data_en[4] = {0}, data_end[4] = {0}, data_rev[4] = {0};

    _air_acl_readReg(unit, AIR_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_READ, data_en);
    if(data_en[rule_idx/32] & (1 << (rule_idx%32)))
    {
        ptr_ctrl->rule_en = TRUE;
    }
    else
    {
        ptr_ctrl->rule_en = FALSE;
    }

    _air_acl_readReg(unit, AIR_ACL_RULE_CONFIG_END, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_READ, data_end);
    if(data_end[rule_idx/32] & (1 << (rule_idx%32)))
    {
        ptr_ctrl->end = TRUE;
    }
    else
    {
        ptr_ctrl->end = FALSE;
    }

    _air_acl_readReg(unit, AIR_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_READ, data_rev);
    if(data_rev[rule_idx/32] & (1 << (rule_idx%32)))
    {
        ptr_ctrl->reverse = TRUE;
    }
    else
    {
        ptr_ctrl->reverse = FALSE;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_setRule
 * PURPOSE:
 *      Set ACL rule entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 *      rule            --  Structure of ACL rule entry
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
air_acl_setRule(
    const UI32_T            unit,
    const UI32_T            rule_idx,
    AIR_ACL_RULE_T          *rule)
{
    UI32_T  type0_key[12] = {0}, type0_mask[12] = {0};
    UI32_T  type1_key[12] = {0}, type1_mask[12] = {0};
    UI32_T  type0_t[12] = {0}, type0_c[12] = {0};
    UI32_T  type1_t[12] = {0}, type1_c[12] = {0};
    AIR_ACL_CTRL_T ctrl;

    /* Check parameter */
    AIR_PARAM_CHK((rule_idx >= ACL_MAX_RULE_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((rule->key.flow_label > BITS_RANGE(0, FLOW_LABEL_WIDTH)), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((rule->key.fieldmap > BITS_RANGE(0, AIR_ACL_FIELD_TYPE_LAST)), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != rule->key.isipv6) && (FALSE != rule->key.isipv6), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((rule->key.portmap & (~AIR_ALL_PORT_BITMAP)), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((TRUE != rule->ctrl.rule_en) && (FALSE != rule->ctrl.rule_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != rule->ctrl.reverse) && (FALSE != rule->ctrl.reverse), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != rule->ctrl.end) && (FALSE != rule->ctrl.end), AIR_E_BAD_PARAMETER);

    memset(type0_key, 0, sizeof(type0_key));
    memset(type0_mask, 0, sizeof(type0_mask));
    memset(type1_key, 0, sizeof(type1_key));
    memset(type1_mask, 0, sizeof(type1_mask));

    memset(type0_t, 0, sizeof(type0_t));
    memset(type0_c, 0, sizeof(type0_c));
    memset(type1_t, 0, sizeof(type1_t));
    memset(type1_c, 0, sizeof(type1_c));

    /* Fill rule type table */
    _air_acl_setRuleTable(AIR_ACL_RULE_TYPE_0, TRUE, &rule->key, type0_key);
    _air_acl_setRuleTable(AIR_ACL_RULE_TYPE_0, FALSE, &rule->mask, type0_mask);

    /* Calculate T/C cell */
    _convertToTCcell(type0_key, type0_mask, type0_t, type0_c, 12);

    /* Set T/C cell to reg */
    _air_acl_writeReg(unit, rule_idx, 3, AIR_ACL_RULE_T_CELL, AIR_ACL_MEM_SEL_RULE, AIR_ACL_MEM_FUNC_WRITE, type0_t);
    _air_acl_writeReg(unit, rule_idx, 3, AIR_ACL_RULE_C_CELL, AIR_ACL_MEM_SEL_RULE, AIR_ACL_MEM_FUNC_WRITE, type0_c);

    /* If match ipv6 flow lable or dip/dip, set rule type 1 */
    if ((1 == rule->key.isipv6) && (rule->key.fieldmap & ((1 << AIR_ACL_DIP) | (1 << AIR_ACL_SIP) | (1 << AIR_ACL_FLOW_LABEL))))
    {
         _air_acl_setRuleTable(AIR_ACL_RULE_TYPE_1, TRUE, &rule->key, type1_key);
         _air_acl_setRuleTable(AIR_ACL_RULE_TYPE_1, FALSE, &rule->mask, type1_mask);
         _convertToTCcell(type1_key, type1_mask, type1_t, type1_c, 12);
         _air_acl_writeReg(unit, rule_idx+1, 3, AIR_ACL_RULE_T_CELL, AIR_ACL_MEM_SEL_RULE, AIR_ACL_MEM_FUNC_WRITE, type1_t);
         _air_acl_writeReg(unit, rule_idx+1, 3, AIR_ACL_RULE_C_CELL, AIR_ACL_MEM_SEL_RULE, AIR_ACL_MEM_FUNC_WRITE, type1_c);
    }

    /* Config rule enable/end/rev */
    memcpy(&ctrl, &rule->ctrl, sizeof(AIR_ACL_CTRL_T));
    if ((1 == rule->key.isipv6) && (rule->key.fieldmap & ((1 << AIR_ACL_DIP) | (1 << AIR_ACL_SIP) | (1 << AIR_ACL_FLOW_LABEL))))
    {
        ctrl.end = 0;
        air_acl_setRuleCtrl(unit, rule_idx, &ctrl);
        air_acl_setRuleCtrl(unit, rule_idx+1, &rule->ctrl);
    }
    else
    {
        air_acl_setRuleCtrl(unit, rule_idx, &rule->ctrl);
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_delRule
 * PURPOSE:
 *      Delete an ACL rule entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
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
air_acl_delRule(
    const UI32_T unit,
    const UI32_T rule_idx)
{
    UI32_T  type0_t[12]={0}, type0_c[12]={0}, type1_t[12]={0}, type1_c[12]={0};
    AIR_ACL_CTRL_T ctrl={0};

    /* Mistake proofing */
    AIR_PARAM_CHK((rule_idx >= ACL_MAX_RULE_NUM), AIR_E_BAD_PARAMETER);

    /* Delete the entry from ACL rule table */
    _air_acl_writeReg(unit, rule_idx, 3, AIR_ACL_RULE_T_CELL, AIR_ACL_MEM_SEL_RULE, AIR_ACL_MEM_FUNC_WRITE, type0_t);
    _air_acl_writeReg(unit, rule_idx, 3, AIR_ACL_RULE_C_CELL, AIR_ACL_MEM_SEL_RULE, AIR_ACL_MEM_FUNC_WRITE, type0_c);
    air_acl_setRuleCtrl(unit, rule_idx, &ctrl);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_clearRule
 * PURPOSE:
 *      Clear all ACL rule entries.
 *
 * INPUT:
 *      unit            --  Device ID
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
air_acl_clearRule(
    const UI32_T   unit)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;
    UI32_T  data[4]={0};

    value = (AIR_ACL_MEM_FUNC_CLEAR << ACL_MEM_CFG_FUNC_SEL_OFFSET) | ACL_MEM_CFG_EN;
    if ((ret = aml_writeReg(unit, ACL_MEM_CFG, value)) != AIR_E_OK)
    {
        return ret;
    }

    _air_acl_writeReg(unit, AIR_ACL_RULE_CONFIG_ENABLE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_WRITE, data);
    _air_acl_writeReg(unit, AIR_ACL_RULE_CONFIG_END, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_WRITE, data);
    _air_acl_writeReg(unit, AIR_ACL_RULE_CONFIG_REVERSE, 1, 0, 0, AIR_ACL_MEM_FUNC_CONFIG_WRITE, data);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_clearAction
 * PURPOSE:
 *      Clear all ACL action entries.
 *
 * INPUT:
 *      unit            --  Device ID
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
air_acl_clearAction(
    const UI32_T   unit)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;
    UI32_T  data[4]={0};

    value = (AIR_ACL_MEM_SEL_ACTION << ACL_MEM_CFG_MEM_SEL_OFFSET) | (AIR_ACL_MEM_FUNC_CLEAR << ACL_MEM_CFG_FUNC_SEL_OFFSET) | ACL_MEM_CFG_EN;
    if ((ret = aml_writeReg(unit, ACL_MEM_CFG, value)) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_getRule
 * PURPOSE:
 *      Get ACL rule entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      rule_idx        --  Index of ACL rule entry
 *
 * OUTPUT:
 *      ptr_rule        --  Structure of ACL rule entry
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
air_acl_getRule(
    const UI32_T unit,
    const UI32_T rule_idx,
    AIR_ACL_RULE_T *ptr_rule)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  bn = 0;
    UI32_T  value = 0, n = 0, idx = 0;
    UI32_T  type0_key[12] = {0}, type0_mask[12] = {0};
    UI32_T  type1_key[12] = {0}, type1_mask[12] = {0};
    UI32_T  type0_t[12] = {0}, type0_c[12] = {0};
    UI32_T  type1_t[12] = {0}, type1_c[12] = {0};

    /* Mistake proofing */
    AIR_PARAM_CHK((rule_idx >= ACL_MAX_RULE_NUM), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_rule);

    _air_acl_readReg(unit, rule_idx, 3, AIR_ACL_RULE_T_CELL, AIR_ACL_MEM_SEL_RULE, AIR_ACL_MEM_FUNC_READ, type0_t);
    _air_acl_readReg(unit, rule_idx, 3, AIR_ACL_RULE_C_CELL, AIR_ACL_MEM_SEL_RULE, AIR_ACL_MEM_FUNC_READ, type0_c);
    /* rule type 1 */
    if(1 == (type0_t[0] & 0x1))
    {
        idx = rule_idx-1;
        AIR_PRINT("This is the part of ipv6 rule, please get rule(%d)\n", idx);
        return AIR_E_OK;
    }

    _parseFromTCcell(type0_t, type0_c, type0_key, type0_mask, 12);

    _air_acl_getRuleTable(AIR_ACL_RULE_TYPE_0, type0_key, &ptr_rule->key);
    _air_acl_getRuleTable(AIR_ACL_RULE_TYPE_0, type0_mask, &ptr_rule->mask);

    if ((TRUE == ptr_rule->key.isipv6) && (ptr_rule->mask.fieldmap & ((1 << AIR_ACL_DIP) | (1 << AIR_ACL_SIP) | (1 << AIR_ACL_FLOW_LABEL))))
    {
        _air_acl_readReg(unit, rule_idx+1, 3, AIR_ACL_RULE_T_CELL, AIR_ACL_MEM_SEL_RULE, AIR_ACL_MEM_FUNC_READ, type1_t);
        _air_acl_readReg(unit, rule_idx+1, 3, AIR_ACL_RULE_C_CELL, AIR_ACL_MEM_SEL_RULE, AIR_ACL_MEM_FUNC_READ, type1_c);
        _parseFromTCcell(type1_t, type1_c, type1_key, type1_mask, 12);

        _air_acl_getRuleTable(AIR_ACL_RULE_TYPE_1, type1_key, &ptr_rule->key);
        _air_acl_getRuleTable(AIR_ACL_RULE_TYPE_1, type1_mask, &ptr_rule->mask);
    }
    if ((TRUE == ptr_rule->key.isipv6) && (ptr_rule->mask.fieldmap & ((1 << AIR_ACL_DIP) | (1 << AIR_ACL_SIP) | (1 << AIR_ACL_FLOW_LABEL))))
    {
        air_acl_getRuleCtrl(unit, rule_idx+1, &ptr_rule->ctrl);
    }
    else
    {
        air_acl_getRuleCtrl(unit, rule_idx, &ptr_rule->ctrl);
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_setAction
 * PURPOSE:
 *      Set an ACL action entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      act_idx         --  Index of ACL action entry
 *      act             --  Structure of ACL action entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setAction(
    const UI32_T unit,
    const UI32_T act_idx,
    const AIR_ACL_ACTION_T act)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  data[4] = {0};

    /* Check parameter */
    AIR_PARAM_CHK((act_idx >= ACL_MAX_ACTION_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != act.port_en) && (FALSE != act.port_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != act.dest_port_sel) && (FALSE != act.dest_port_sel), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != act.vlan_port_sel) && (FALSE != act.vlan_port_sel), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.portmap & (~AIR_ALL_PORT_BITMAP)), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((TRUE != act.cnt_en) && (FALSE != act.cnt_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.cnt_idx >= ACL_MAX_MIB_NUM), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((TRUE != act.attack_en) && (FALSE != act.attack_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.attack_idx >= ACL_MAX_ATTACK_RATE_NUM), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((TRUE != act.rate_en) && (FALSE != act.rate_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.rate_idx >= ACL_MAX_METER_NUM), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((TRUE != act.vlan_en) && (FALSE != act.vlan_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.vlan_idx >= ACL_MAX_VLAN_NUM), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((act.mirrormap > BITS_RANGE(0, ACL_MAX_MIR_SESSION_NUM)), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((TRUE != act.pri_user_en) && (FALSE != act.pri_user_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.pri_user >= AIR_MAX_NUM_OF_QUEUE), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((TRUE != act.lyvlan_en) && (FALSE != act.lyvlan_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != act.lyvlan) && (FALSE != act.lyvlan), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != act.mang) && (FALSE != act.mang), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != act.bpdu) && (FALSE != act.bpdu), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((TRUE != act.fwd_en) && (FALSE != act.fwd_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.fwd >= AIR_ACL_ACT_FWD_LAST), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((TRUE != act.egtag_en) && (FALSE != act.egtag_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.egtag >= AIR_ACL_ACT_EGTAG_LAST), AIR_E_BAD_PARAMETER);

    AIR_PARAM_CHK((TRUE != act.trtcm_en) && (FALSE != act.trtcm_en), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != act.trtcm.cls_slr_sel) && (FALSE != act.trtcm.cls_slr), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.trtcm.cls_slr >= ACL_MAX_CLASS_SLR_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != act.trtcm.drop_pcd_sel) && (FALSE != act.trtcm.drop_pcd_sel), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.trtcm.drop_pcd_g >= ACL_MAX_DROP_PCD_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.trtcm.drop_pcd_y >= ACL_MAX_DROP_PCD_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.trtcm.drop_pcd_r >= ACL_MAX_DROP_PCD_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != act.trtcm.tcm_sel) && (FALSE != act.trtcm.tcm_sel), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.trtcm.usr_tcm >= AIR_ACL_ACT_USR_TCM_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((act.trtcm.tcm_idx >= ACL_MAX_TRTCM_NUM), AIR_E_BAD_PARAMETER);

    _air_acl_setActionTable(&act, data);
    _air_acl_writeReg(unit, act_idx, 1, 0, AIR_ACL_MEM_SEL_ACTION, AIR_ACL_MEM_FUNC_WRITE, data);

    return ret;
}

/* FUNCTION NAME: air_acl_getAction
 * PURPOSE:
 *      Get an ACL action entry with speficic index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      act_idx         --  Index of ACL action entry
 *
 * OUTPUT:
 *      ptr_act         --  Structure of ACL action entry
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getAction(
    const UI32_T unit,
    const UI32_T act_idx,
    AIR_ACL_ACTION_T *ptr_act)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T i = 0;
    UI32_T data[4] = {0};

    /* Mistake proofing */
    AIR_PARAM_CHK((act_idx >= ACL_MAX_ACTION_NUM), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_act);

    _air_acl_readReg(unit, act_idx, 1, 0, AIR_ACL_MEM_SEL_ACTION, AIR_ACL_MEM_FUNC_READ, data);
    _air_acl_getActionTable(data, ptr_act);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_delAction
 * PURPOSE:
 *      Delete an ACL action entry with specific index
 *
 * INPUT:
 *      unit            --  Device ID
 *      act_idx         --  Index of ACL action entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_ENTRY_NOT_FOUND
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_delAction(
    const UI32_T unit,
    const UI32_T act_idx)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;
    UI32_T  data[4] = {0};

    /* Check parameter */
    AIR_PARAM_CHK((act_idx >= ACL_MAX_ACTION_NUM), AIR_E_BAD_PARAMETER);

    _air_acl_writeReg(unit, act_idx, 1, 0, AIR_ACL_MEM_SEL_ACTION, AIR_ACL_MEM_FUNC_WRITE, data);
    return ret;
}

/* FUNCTION NAME: air_acl_setTrtcm
 * PURPOSE:
 *      Set a trTCM entry with the specific index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      tcm_idx         --  Index of trTCM entry
 *      tcm             --  Structure of trTCM entry
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
 *      Index 0 ~ 31 can be selected in tcm_idx.
 */
AIR_ERROR_NO_T
air_acl_setTrtcm(
    const UI32_T unit,
    const UI32_T tcm_idx,
    const AIR_ACL_TRTCM_T tcm)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK((tcm_idx >= ACL_MAX_TRTCM_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((tcm.cbs > ACL_MAX_CBS_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((tcm.cir > ACL_MAX_CIR_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((tcm.pbs > ACL_MAX_PBS_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((tcm.pir > ACL_MAX_PIR_NUM), AIR_E_BAD_PARAMETER);

    if(AIR_E_TIMEOUT == _checkDone(unit, AIR_ACL_CHECK_TRTCM))
    {
        return AIR_E_TIMEOUT;
    }

    aml_writeReg(unit, ACL_TRTCMW_CBS, tcm.cbs & ACL_TRTCM_CBS_MASK);
    aml_writeReg(unit, ACL_TRTCMW_EBS, tcm.pbs & ACL_TRTCM_EBS_MASK);
    aml_writeReg(unit, ACL_TRTCMW_CIR, tcm.cir & ACL_TRTCM_CIR_MASK);
    aml_writeReg(unit, ACL_TRTCMW_EIR, tcm.pir & ACL_TRTCM_EIR_MASK);

    value = (1 << ACL_TRTCM_BUSY_OFFSET) | ACL_TRTCM_WRITE | ((tcm_idx & ACL_TRTCM_ID_MASK) << ACL_TRTCM_ID_OFFSET);

    if ((ret = aml_writeReg(unit, ACL_TRTCMA, value)) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_delTrtcm
 * PURPOSE:
 *      Delete an ACL trTCM entry.
 *
 * INPUT:
 *      unit            --  Device ID
 *      trtcm_idx       --  Index of ACL trTCM entry
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
 *      Index 0 ~ 31 can be selected in tcm_idx.
 */
AIR_ERROR_NO_T
air_acl_delTrtcm(
    const UI32_T unit,
    const UI32_T tcm_idx)
{
    AIR_ACL_TRTCM_T tcm;

    /* Mistake proofing */
    AIR_PARAM_CHK((tcm_idx >= ACL_MAX_TRTCM_NUM), AIR_E_BAD_PARAMETER);

    /* Delete the entry from trTCM table */
    memset(&tcm, 0, sizeof(tcm));
    return air_acl_setTrtcm(unit, tcm_idx, tcm);
}

/* FUNCTION NAME: air_acl_clearTrtcm
 * PURPOSE:
 *      Clear all ACL trTCM entries.
 *
 * INPUT:
 *      unit            --  Device ID
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
air_acl_clearTrtcm(
    const UI32_T unit)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_ACL_TRTCM_T tcm;
    UI32_T i = 0;

    /* Delete all entries from trTCM table */
    memset(&tcm, 0, sizeof(tcm));
    for(i=0; i < ACL_MAX_TRTCM_NUM; i++)
    {
        ret = air_acl_setTrtcm(unit, i, tcm);
        if(AIR_E_OK != ret)
        {
            return ret;
        }
    }
    return ret;
}

/* FUNCTION NAME: air_acl_getTrtcm
 * PURPOSE:
 *      Get a trTCM entry with the specific index.
 *
 * INPUT:
 *      unit            --  Device ID
 *      tcm_idx         --  Index of trTCM entry
 *
 * OUTPUT:
 *      ptr_tcm         --  Structure of trTCM entry
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_TIMEOUT
 *
 * NOTES:
 *      Index 0 ~ 31 can be selected in tcm_idx.
 */
AIR_ERROR_NO_T
air_acl_getTrtcm(
    const UI32_T unit,
    const UI32_T tcm_idx,
    AIR_ACL_TRTCM_T *ptr_tcm)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0, trtcmr1 = 0, trtcmr2 = 0, trtcmr3 = 0, trtcmr4 = 0;

    /* Mistake proofing */
    AIR_PARAM_CHK((tcm_idx >= ACL_MAX_TRTCM_NUM), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_tcm);

    if(AIR_E_TIMEOUT == _checkDone(unit, AIR_ACL_CHECK_TRTCM))
    {
        return AIR_E_TIMEOUT;
    }

    value = (1 << ACL_TRTCM_BUSY_OFFSET) | ACL_TRTCM_READ | ((tcm_idx & ACL_TRTCM_ID_MASK) << ACL_TRTCM_ID_OFFSET);

    if ((ret = aml_writeReg(unit, ACL_TRTCMA, value)) != AIR_E_OK)
    {
        return ret;
    }

    aml_readReg(unit, ACL_TRTCMR_CBS, &trtcmr1);
    aml_readReg(unit, ACL_TRTCMR_EBS, &trtcmr2);
    aml_readReg(unit, ACL_TRTCMR_CIR, &trtcmr3);
    aml_readReg(unit, ACL_TRTCMR_EIR, &trtcmr4);

    ptr_tcm->cbs = trtcmr1 & ACL_TRTCM_CBS_MASK;
    ptr_tcm->pbs = trtcmr2 & ACL_TRTCM_EBS_MASK;
    ptr_tcm->cir = trtcmr3 & ACL_TRTCM_CIR_MASK;
    ptr_tcm->pir = trtcmr4 & ACL_TRTCM_EIR_MASK;

    return AIR_E_OK;

}

/* FUNCTION NAME: air_acl_setTrtcmEnable
 * PURPOSE:
 *      Set trTCM enable so the meter table will be updated based on PIR and CIR.
 *      The color marker will also be enabled when ACL is hit.
 *
 * INPUT:
 *      unit            --  Device ID
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
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
air_acl_setTrtcmEnable(
    const UI32_T unit,
    const BOOL_T state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK(((TRUE != state) && (FALSE != state)), AIR_E_BAD_PARAMETER);

    /* Read data from register */
    aml_readReg(unit, ACL_TRTCM, &u32dat);

    if (TRUE == state)
    {
        u32dat |= (BIT(ACL_TRTCM_EN_OFFSET));
    }
    else
    {
        u32dat &= ~(BIT(ACL_TRTCM_EN_OFFSET));
    }

    /* Write data to register */
    aml_writeReg(unit, ACL_TRTCM, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_getTrtcm
 * PURPOSE:
 *      Get a trTCM enable state.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_state       --  FALSE:Disable
 *                          TRUE: Enable
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getTrtcmEnable(
    const UI32_T unit,
    BOOL_T *const ptr_state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_CHECK_PTR(ptr_state);

    /* Read data from register */
    aml_readReg(unit, ACL_TRTCM, &u32dat);

    if (u32dat & BIT(ACL_TRTCM_EN_OFFSET))
    {
        (*ptr_state) = TRUE;
    }
    else
    {
        (*ptr_state) = FALSE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_setPortEnable
 * PURPOSE:
 *      Set ACL state for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
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
air_acl_setPortEnable(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state)
{
    UI32_T u32dat = 0, value = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != state) && (FALSE != state)), AIR_E_BAD_PARAMETER);

    value = state ? 1 : 0;
    aml_readReg(unit, ACL_PORT_EN, &u32dat);
    u32dat = (u32dat & ~(ACL_EN_MASK << port)) | (value << port);
    aml_writeReg(unit, ACL_PORT_EN, u32dat);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_getPortEnable
 * PURPOSE:
 *      Get ACL state for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_state       --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getPortEnable(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);

    /* Mistake proofing for state checking */
    AIR_CHECK_PTR(ptr_state);

    /* Read data from register */
    aml_readReg(unit, ACL_PORT_EN, &u32dat);

    (*ptr_state) = BITS_OFF_R(u32dat, port, ACL_EN_MASK);

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_setDropEnable
 * PURPOSE:
 *      Set ACL drop precedence state
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      state           --  FALSE:Disable
 *                          TRUE: Enable
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
air_acl_setDropEnable(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK(((TRUE != state) && (FALSE != state)), AIR_E_BAD_PARAMETER);

    u32dat = state ? 1 : 0;
    aml_writeReg(unit, DPCR_EN(port), u32dat);
    return AIR_E_OK;

}

/* FUNCTION NAME: air_acl_getDropEnable
 * PURPOSE:
 *      Get ACL drop precedence state
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_state       --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getDropEnable(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T *ptr_state)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_state);

    aml_readReg(unit, DPCR_EN(port), &u32dat);
    *ptr_state = u32dat ? TRUE : FALSE;

    return AIR_E_OK;
}

/* FUNCTION NAME: air_acl_setDropThreshold
 * PURPOSE:
 *      Set ACL drop threshold.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      color           --  AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *      high            --  High threshold
 *      low             --  Low threshold
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_setDropThreshold(
    const UI32_T unit,
    const UI32_T port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T queue,
    const UI32_T high,
    const UI32_T low)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((color >= AIR_ACL_DP_COLOR_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((queue >= AIR_MAX_NUM_OF_QUEUE), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((high > BITS_RANGE(0, DPCR_HIGH_THRSH_WIDTH)), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((low > BITS_RANGE(0, DPCR_LOW_THRSH_WIDTH)), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, DPCR(port, color, queue), &u32dat);
    u32dat = (u32dat & ~(DPCR_LOW_THRSH_MASK)) | low;
    u32dat = (u32dat & ~(DPCR_HIGH_THRSH_MASK << DPCR_HIGH_THRSH_OFFSET)) | (high << DPCR_HIGH_THRSH_OFFSET);
    aml_writeReg(unit, DPCR(port, color, queue), u32dat);
    return AIR_E_OK;

}

/* FUNCTION NAME: air_acl_getDropThreshold
 * PURPOSE:
 *      Get ACL drop threshold.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      color           --  AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *
 * OUTPUT:
 *      ptr_high        --  High threshold
 *      ptr_low         --  Low threshold
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_getDropThreshold(
    const UI32_T unit,
    const UI32_T port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T queue,
    UI32_T *ptr_high,
    UI32_T *ptr_low)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((color >= AIR_ACL_DP_COLOR_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((queue >= AIR_MAX_NUM_OF_QUEUE), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_high);
    AIR_CHECK_PTR(ptr_low);

    aml_readReg(unit, DPCR(port, color, queue), &u32dat);
    *ptr_low = u32dat & DPCR_LOW_THRSH_MASK;
    *ptr_high = (u32dat >> DPCR_HIGH_THRSH_OFFSET) & DPCR_HIGH_THRSH_MASK;
    return AIR_E_OK;

}

/* FUNCTION NAME: air_acl_setDropProbability
 * PURPOSE:
 *      Set ACL drop probability.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      color           --  AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *      probability     --  Drop probability (value:0 ~ 1023, unit:1/1023; eg: value-102 = 10% )
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_setDropProbability(
    const UI32_T unit,
    const UI32_T port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T queue,
    const UI32_T probability)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((color >= AIR_ACL_DP_COLOR_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((queue >= AIR_MAX_NUM_OF_QUEUE), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((probability > BITS_RANGE(0, DPCR_PBB_WIDTH)), AIR_E_BAD_PARAMETER);

    aml_readReg(unit, DPCR(port, color, queue), &u32dat);
    u32dat = (u32dat & ~(DPCR_PBB_MASK << DPCR_PBB_OFFSET)) | (probability << DPCR_PBB_OFFSET);
    aml_writeReg(unit, DPCR(port, color, queue), u32dat);
    return AIR_E_OK;

}

/* FUNCTION NAME: air_acl_getDropProbability
 * PURPOSE:
 *      Get ACL drop probability.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      color           --  AIR_ACL_DP_COLOR_YELLOW: Yellow
 *                          AIR_ACL_DP_COLOR_RED   : Red
 *      queue           --  Output queue number
 *
 * OUTPUT:
 *      ptr_probability --  Drop probability (value:0 ~ 1023, unit:1/1023; eg: value-102 = 10% )
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Key parameter include port, color, queue.
 */
AIR_ERROR_NO_T
air_acl_getDropProbability(
    const UI32_T unit,
    const UI32_T port,
    const AIR_ACL_DP_COLOR_T color,
    const UI8_T queue,
    UI32_T *ptr_probability)
{
    UI32_T u32dat = 0;

    /* Mistake proofing for port checking */
    AIR_PARAM_CHK((port >= AIR_MAX_NUM_OF_PORTS), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((color >= AIR_ACL_DP_COLOR_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((queue >= AIR_MAX_NUM_OF_QUEUE), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_probability);

    /* Read data from register */
    aml_readReg(unit, DPCR(port, color, queue), &u32dat);

    (*ptr_probability) = BITS_OFF_R(u32dat, DPCR_PBB_OFFSET, DPCR_PBB_WIDTH);

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      air_acl_setGlobalState
 * PURPOSE:
 *      Set the ACL global enable state.
 * INPUT:
 *      unit        -- Device ID
 *      state       -- Enable state of ACL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setGlobalState(
    const UI32_T        unit,
    const BOOL_T        state)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0, data = 0;

    AIR_PARAM_CHK((TRUE != state) && (FALSE != state), AIR_E_BAD_PARAMETER);

    value = state ? 1 : 0;
    if ((ret = aml_readReg(unit, ACL_GLOBAL_CFG, &data)) != AIR_E_OK)
    {
        return ret;
    }
    data = (data & ~ACL_EN_MASK) | value;
    if ((ret = aml_writeReg(unit, ACL_GLOBAL_CFG, data)) != AIR_E_OK)
    {
        return ret;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      air_acl_getGlobalState
 * PURPOSE:
 *      Get the ACL global enable state.
 * INPUT:
 *      unit             -- Device ID
 * OUTPUT:
 *      ptr_state        -- Enable state
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getGlobalState(
    const UI32_T         unit,
    BOOL_T               *ptr_state)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;

    AIR_CHECK_PTR(ptr_state);
    if ((ret = aml_readReg(unit, ACL_GLOBAL_CFG, &value)) != AIR_E_OK)
    {
        return ret;
    }

    value &= ACL_EN_MASK;
    *ptr_state = value ? TRUE : FALSE;
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      air_acl_setUdfRule
 * PURPOSE:
 *      Set ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- Device ID
 *      rule_idx         -- ACLUDF table entry index
 *      udf_rule         -- Structure of ACL UDF rule entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setUdfRule(
    const UI32_T                unit,
    const UI32_T                rule_idx,
    AIR_ACL_UDF_RULE_T          udf_rule)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;
    UI32_T  data[3] = {0};

    /* Check parameter */
    AIR_PARAM_CHK((rule_idx >= ACL_MAX_UDF_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((TRUE != udf_rule.valid) && (FALSE != udf_rule.valid), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((udf_rule.offset_format >= AIR_ACL_RULE_OFS_FMT_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((udf_rule.offset >= ACL_MAX_WORD_OFST_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((udf_rule.cmp_sel >= AIR_ACL_RULE_CMP_SEL_LAST), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((udf_rule.pattern > ACL_MAX_CMP_PAT_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((udf_rule.mask > ACL_MAX_CMP_BIT_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((udf_rule.low_threshold > ACL_MAX_CMP_PAT_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((udf_rule.high_threshold > ACL_MAX_CMP_BIT_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((udf_rule.portmap & (~AIR_ALL_PORT_BITMAP)), AIR_E_BAD_PARAMETER);

    _fillTblMultiFields(data, 3, PORT_BITMAP_OFFSET, PORT_BITMAP_WIDTH, udf_rule.portmap);
    _fillTblMultiFields(data, 3, UDF_RULE_EN_OFFSET, UDF_RULE_EN_WIDTH, udf_rule.valid);
    _fillTblMultiFields(data, 3, UDF_PKT_TYPE_OFFSET, UDF_PKT_TYPE_WIDTH, udf_rule.offset_format);
    _fillTblMultiFields(data, 3, WORD_OFST_OFFSET, WORD_OFST_WIDTH, udf_rule.offset);

    _fillTblMultiFields(data, 3, CMP_SEL_OFFSET, CMP_SEL_WIDTH, udf_rule.cmp_sel);
    if(AIR_ACL_RULE_CMP_SEL_PATTERN == udf_rule.cmp_sel)
    {
        _fillTblMultiFields(data, 3, CMP_PAT_OFFSET, CMP_PAT_WIDTH, udf_rule.pattern);
        _fillTblMultiFields(data, 3, CMP_MASK_OFFSET, CMP_MASK_WIDTH, udf_rule.mask);
    }
    else
    {
        _fillTblMultiFields(data, 3, CMP_PAT_OFFSET, CMP_PAT_WIDTH, udf_rule.low_threshold);
        _fillTblMultiFields(data, 3, CMP_MASK_OFFSET, CMP_MASK_WIDTH, udf_rule.high_threshold);
    }

    if(AIR_E_TIMEOUT == _checkDone(unit, AIR_ACL_CHECK_UDF))
    {
        return AIR_E_TIMEOUT;
    }
    aml_writeReg(unit, ACL_AUTW0, data[0]);
    aml_writeReg(unit, ACL_AUTW1, data[1]);
    aml_writeReg(unit, ACL_AUTW2, data[2]);
    value = (rule_idx & ACL_UDF_ADDR_MASK) | ACL_UDF_WRITE | (1U << ACL_UDF_ACC_OFFSET);
    if ((ret = aml_writeReg(unit, ACL_AUTC, value)) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      air_acl_getUdfRule
 * PURPOSE:
 *      Get ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- Device ID
 *      rule_idx         -- ACLUDF table entry index
 * OUTPUT:
 *      ptr_udf_rule     -- Structure of ACL UDF rule entry
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getUdfRule(
    const UI32_T                unit,
    const UI8_T                 rule_idx,
    AIR_ACL_UDF_RULE_T          *ptr_udf_rule)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;
    UI32_T  data[3] = {0};

    /* Check parameter */
    AIR_PARAM_CHK((rule_idx >= ACL_MAX_UDF_NUM), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_udf_rule);

    value = (rule_idx & ACL_UDF_ADDR_MASK) | ACL_UDF_READ | (1U << ACL_UDF_ACC_OFFSET);
    if ((ret = aml_writeReg(unit, ACL_AUTC, value)) != AIR_E_OK)
    {
        return ret;
    }
    if(AIR_E_TIMEOUT == _checkDone(unit, AIR_ACL_CHECK_UDF))
    {
        return AIR_E_TIMEOUT;
    }
    aml_readReg(unit, ACL_AUTR0, data);
    aml_readReg(unit, ACL_AUTR1, data+1);
    aml_readReg(unit, ACL_AUTR2, data+2);

    ACL_DERIVE_TBL_MULTIFIELDS(data, PORT_BITMAP_OFFSET, PORT_BITMAP_WIDTH, ptr_udf_rule->portmap);

    ACL_DERIVE_TBL_MULTIFIELDS(data, UDF_RULE_EN_OFFSET, UDF_RULE_EN_WIDTH, ptr_udf_rule->valid);
    ACL_DERIVE_TBL_MULTIFIELDS(data, UDF_PKT_TYPE_OFFSET, UDF_PKT_TYPE_WIDTH, ptr_udf_rule->offset_format);
    ACL_DERIVE_TBL_MULTIFIELDS(data, WORD_OFST_OFFSET, WORD_OFST_WIDTH, ptr_udf_rule->offset);
    ACL_DERIVE_TBL_MULTIFIELDS(data, CMP_SEL_OFFSET, CMP_SEL_WIDTH, ptr_udf_rule->cmp_sel);
    if(AIR_ACL_RULE_CMP_SEL_PATTERN == ptr_udf_rule->cmp_sel)
    {
        ACL_DERIVE_TBL_MULTIFIELDS(data, CMP_PAT_OFFSET, CMP_PAT_WIDTH, ptr_udf_rule->pattern);
        ACL_DERIVE_TBL_MULTIFIELDS(data, CMP_MASK_OFFSET, CMP_MASK_WIDTH, ptr_udf_rule->mask);
    }
    else
    {
        ACL_DERIVE_TBL_MULTIFIELDS(data, CMP_PAT_OFFSET, CMP_PAT_WIDTH, ptr_udf_rule->low_threshold);
        ACL_DERIVE_TBL_MULTIFIELDS(data, CMP_MASK_OFFSET, CMP_MASK_WIDTH, ptr_udf_rule->high_threshold);
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      air_acl_delUdfRule
 * PURPOSE:
 *      Delete ACL UDF rule of specified entry index.
 * INPUT:
 *      unit             -- Device ID
 *      rule_idx         -- ACLUDF table entry index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_delUdfRule(
    const UI32_T      unit,
    const UI8_T       rule_idx)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;
    UI32_T  data = 0;

    /* Check parameter */
    AIR_PARAM_CHK((rule_idx >= ACL_MAX_UDF_NUM), AIR_E_BAD_PARAMETER);

    aml_writeReg(unit, ACL_AUTW0, data);
    aml_writeReg(unit, ACL_AUTW1, data);
    aml_writeReg(unit, ACL_AUTW2, data);

    value = (rule_idx & ACL_UDF_ADDR_MASK) | ACL_UDF_WRITE | (1U << ACL_UDF_ACC_OFFSET);
    if ((ret = aml_writeReg(unit, ACL_AUTC, value)) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      air_acl_clearUdfRule
 * PURPOSE:
 *      Clear acl all udf rule.
 * INPUT:
 *      unit             -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_clearUdfRule(
    const UI32_T    unit)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;

    value = ACL_UDF_CLEAR | (1U << ACL_UDF_ACC_OFFSET);
    if ((ret = aml_writeReg(unit, ACL_AUTC, value)) != AIR_E_OK)
    {
        return ret;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      air_acl_setMeterTable
 * PURPOSE:
 *      Set flow ingress rate limit by meter table.
 * INPUT:
 *      unit                -- Device ID
 *      meter_id            -- Meter id
 *      enable              -- Meter enable state
 *      rate                -- Ratelimit(unit:64kbps)
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_setMeterTable(
    const UI32_T            unit,
    const UI32_T            meter_id,
    const BOOL_T            enable,
    const UI32_T            rate)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;

    /* Check parameter */
    AIR_PARAM_CHK((meter_id >= ACL_MAX_METER_NUM), AIR_E_BAD_PARAMETER);
    AIR_PARAM_CHK((rate > ACL_MAX_TOKEN_NUM), AIR_E_BAD_PARAMETER);

    if(TRUE == enable)
    {
        value = (1 << ACL_RATE_BUSY_OFFSET) | ACL_RATE_WRITE | ((meter_id & ACL_RATE_ID_MASK) << ACL_RATE_ID_OFFSET) | ACL_RATE_EN |
            (rate & ACL_RATE_TOKEN_MASK);
    }
    else if(FALSE == enable)
    {
        value = (1 << ACL_RATE_BUSY_OFFSET) | ACL_RATE_WRITE | ((meter_id & ACL_RATE_ID_MASK) << ACL_RATE_ID_OFFSET) | ACL_RATE_DIS;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if(AIR_E_TIMEOUT == _checkDone(unit, AIR_ACL_CHECK_METER))
    {
        return AIR_E_TIMEOUT;
    }
    if ((ret = aml_writeReg(unit, ACLRMC, value)) != AIR_E_OK)
    {
        return ret;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      air_acl_getMeterTable
 * PURPOSE:
 *      Get meter table configuration.
 * INPUT:
 *      unit                -- Device ID
 *      meter_id            -- Meter id
 * OUTPUT:
 *      ptr_enable          -- Meter enable state
 *      ptr_rate            -- Ratelimit(unit:64kbps)
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_acl_getMeterTable(
    const UI32_T            unit,
    const UI32_T            meter_id,
    BOOL_T                  *ptr_enable,
    UI32_T                  *ptr_rate)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T  value = 0;

    /* Check parameter */
    AIR_PARAM_CHK((meter_id >= ACL_MAX_METER_NUM), AIR_E_BAD_PARAMETER);
    AIR_CHECK_PTR(ptr_enable);
    AIR_CHECK_PTR(ptr_rate);

    if(AIR_E_TIMEOUT == _checkDone(unit, AIR_ACL_CHECK_METER))
    {
        return AIR_E_TIMEOUT;
    }
    value = (1 << ACL_RATE_BUSY_OFFSET) | ACL_RATE_READ | ((meter_id & ACL_RATE_ID_MASK) << ACL_RATE_ID_OFFSET) | ACL_RATE_EN;

    if ((ret = aml_writeReg(unit, ACLRMC, value)) != AIR_E_OK)
    {
        return ret;
    }
    aml_readReg(unit, ACLRMD1, &value);
    *ptr_enable = ((value >> ACL_RATE_EN_OFFSET) & 0x1) ? TRUE : FALSE;
    *ptr_rate = value & ACL_RATE_TOKEN_MASK;

    return AIR_E_OK;
}


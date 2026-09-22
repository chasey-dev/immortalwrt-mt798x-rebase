/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AIROHA SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AIROHA SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AIROHA'S OPTION, TO REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

/* FILE NAME: air_lpdet.h
* PURPOSE:
 *      Define the loop detect function in AIR SDK.
*
* NOTES:
*       None
*/

#ifndef AIR_LPDET_H
#define AIR_LPDET_H

/* INCLUDE FILE DECLARATIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* Definition of Loop Detection */
typedef enum{
    AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME = 0,
    AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM,
    AIR_SWC_LPDET_CTRL_TYPE_LAST
} AIR_SWC_LPDET_CTRL_TYPE_T;

/* FUNCTION NAME:   air_lpdet_setLdfSrcMac
 * PURPOSE:
 *      Set the loop detect frame source MAC address.
 * INPUT:
 *      unit                     -- Device ID
 *      mac                      -- Source MAC address
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      It's unique and specified for loop frame.
 */
AIR_ERROR_NO_T
air_lpdet_setLdfSrcMac(
    const UI32_T    unit,
    const AIR_MAC_T mac);

/* FUNCTION NAME:   air_lpdet_getLdfSrcMac
 * PURPOSE:
 *      Get loop detect frame source MAC address.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      mac                      -- Source MAC address
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      It's unique and specified for loop frame.
 */
AIR_ERROR_NO_T
air_lpdet_getLdfSrcMac(
    const UI32_T unit,
    AIR_MAC_T    mac);

/* FUNCTION NAME:   air_lpdet_setCtrl
 * PURPOSE:
 *      Set the loop detect control.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 *      enable                   -- FALSE: Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lpdet_setCtrl(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const BOOL_T                    enable);

/* FUNCTION NAME:   air_lpdet_getCtrl
 * PURPOSE:
 *      Get the loop detect control.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 * OUTPUT:
 *      ptr_enable               -- FALSE: Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lpdet_getCtrl(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    BOOL_T                          *ptr_enable);

/* FUNCTION NAME:   air_lpdet_clearStatus
 * PURPOSE:
 *      Clear the loop detect status.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 *      port_bitmap              -- Loop status port bitmap
 *                                  AIR_PORT_BITMAP_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lpdet_clearStatus(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const AIR_PORT_BITMAP_T         port_bitmap);

/* FUNCTION NAME:   air_lpdet_getStatus
 * PURPOSE:
 *      Get the loop detect status.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 * OUTPUT:
 *      port_bitmap              -- Loop status port bitmap
 *                                  AIR_PORT_BITMAP_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lpdet_getStatus(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    AIR_PORT_BITMAP_T               port_bitmap);

#endif /* End of AIR_LPDET_H */

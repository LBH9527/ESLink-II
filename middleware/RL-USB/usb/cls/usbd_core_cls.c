/**
 * @file    usbd_core_msc.c
 * @brief   Mass Storage Class driver
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "string.h"

#include "RTL.h"
#include "rl_usb.h"
#include "usb_for_lib.h"



/*
 *  USB Device Endpoint 0 Event Callback - CLS specific handling (Setup Request To Interface)
 *    Parameters:      none
 *    Return Value:    TRUE - Setup class request ok, FALSE - Setup class request not supported
 */


//BOOL USBD_EndPoint0_Setup_CLS_ReqToIF  (BOOL setup) {
//  /* Should handle any custom class interface requests */
//  /* Sample code which demonstrates how to use it */
//  if (USBD_SetupPacket.wIndexL == 0x01) {         /* Check interface number of custom class */
//    if (setup) {                                  /* Interface request is in SETUP stage (after setup packet has been received) */
//    /* USBD_SetupPacket contains information of received request: bRequest, wValue, wIndex, wLength 
//       which can be analyzed to determine what exactly needs to be done and to prepare response */
//      switch (USBD_SetupPacket.bRequest) {
//        case 0x01:                                /* Request: 0x01 - custom defined request */
//          if (USBD_SetupPacket.bmRequestType.Dir == REQUEST_HOST_TO_DEVICE) {
//            /* Request which expects data to be received from host */
//            /* Prepare buffer for data reception */
//            USBD_EP0Data.pData = data_recv;
//          } else {
//            /* Request which expects data to be sent to host */ 
//            /* Prepare data to send */
//            data_send[0] = 0xAB;
//            data_send[1] = 0xCD;
//            /* ... */
//            USBD_EP0Data.pData = data_send;
//            if (USBD_EP0Data.Count > NUMBER_OF_BYTES_TO_SEND) {
//              /* If more data is requested then we have to send, we have to correct Count. 
//                 Also if last packet is sized as maximum packet we have to generate
//                 Zero length packet to terminate the response. */
//              USBD_EP0Data.Count = NUMBER_OF_BYTES_TO_SEND;
//              if (!(USBD_EP0Data.Count & (USBD_MAX_PACKET0 - 1))) USBD_ZLP = 1;
//            }
//            USBD_DataInStage();
//          }
//          return (__TRUE);
//      }
//    } else {                                      /* Interface request is in OUT stage (after data has been received) */
//      switch (USBD_SetupPacket.bRequest) {
//        case 0x01:                                /* Request: 0x01 - custom defined request */
//          /* Received data available for processing
//               Data buffer: data_recv
//               Data length: USBD_SetupPacket.wLength
//           */
//          USBD_StatusInStage();                   /* send Acknowledge */
//          return (__TRUE);
//      }
//    }
//  }
//  return (__FALSE);                               /* If this was not a request for our custom class interface or 
//                                                     if it was a request that we do not handle */
//}



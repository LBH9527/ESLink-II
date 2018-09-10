/**
 * @file    usb_lib.h
 * @brief   USB library header
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

#ifndef __USB_LIB_H__
#define __USB_LIB_H__

/*------------------------------------------------------------------------------
 *           USB Device Configuration
 *----------------------------------------------------------------------------*/
extern U8 USBD_AltSetting[];
extern U8 USBD_EP0Buf[];
extern const U8 usbd_power;
extern const U8 usbd_hs_enable;
extern const U16 usbd_if_num;
extern const U8 usbd_ep_num;
extern const U8 usbd_max_packet0;


/*------------------------------------------------------------------------------
 *           USB Device Class Configuration
 *----------------------------------------------------------------------------*/
extern const U8 usbd_hid_enable;
extern const U8 usbd_hid_if_num;
extern const U8 usbd_hid_ep_intin;
extern const U8 usbd_hid_ep_intout;
extern const U16 usbd_hid_interval     [2];
extern const U16 usbd_hid_maxpacketsize[2];
extern const U8 usbd_hid_inreport_num;
extern const U8 usbd_hid_outreport_num;
extern const U16 usbd_hid_inreport_max_sz;
extern const U16 usbd_hid_outreport_max_sz;
extern const U16 usbd_hid_featreport_max_sz;
extern U16 USBD_HID_PollingCnt;
extern U16 USBD_HID_PollingReload[];
extern U8 USBD_HID_IdleCnt[];
extern U8 USBD_HID_IdleReload[];
extern U8 USBD_HID_IdleSet[];
extern U8 USBD_HID_InReport[];
extern U8 USBD_HID_OutReport[];
extern U8 USBD_HID_FeatReport[];

extern const U8 usbd_msc_enable;
extern const U8 usbd_msc_if_num;
extern const U8 usbd_msc_ep_bulkin;
extern const U8 usbd_msc_ep_bulkout;
extern const U16 usbd_msc_maxpacketsize[2];
extern const U8 *usbd_msc_inquiry_data;
extern const U16 USBD_MSC_BulkBufSize;
extern       U8 USBD_MSC_BulkBuf[];

extern const U8 usbd_adc_enable;
extern const U8 usbd_adc_cif_num;
extern const U8 usbd_adc_sif1_num;
extern const U8 usbd_adc_sif2_num;
extern const U8 usbd_adc_ep_isoout;
extern const U32 usbd_adc_cfg_datafreq;
extern const U32 usbd_adc_cfg_p_s;
extern const U32 usbd_adc_cfg_p_c;
extern const U32 usbd_adc_cfg_b_s;
extern       S16 USBD_ADC_DataBuf[];

extern const U8 usbd_cdc_acm_enable;
extern const U8 usbd_cdc_acm_cif_num;
extern const U8 usbd_cdc_acm_dif_num;
extern const U8 usbd_cdc_acm_bufsize;
extern const U8 usbd_cdc_acm_ep_intin;
extern const U8 usbd_cdc_acm_ep_bulkin;
extern const U8 usbd_cdc_acm_ep_bulkout;
extern const U16 usbd_cdc_acm_sendbuf_sz;
extern const U16 usbd_cdc_acm_receivebuf_sz;
extern const U16 usbd_cdc_acm_maxpacketsize[2];
extern const U16 usbd_cdc_acm_maxpacketsize1[2];
extern U8 USBD_CDC_ACM_SendBuf[];
extern U8 USBD_CDC_ACM_ReceiveBuf[];
extern U8 USBD_CDC_ACM_NotifyBuf[10];

//add by 9527 for usbd cls
//(down)
extern const U8 usbd_cls_ep_bulkin ;
extern const U8 usbd_cls_ep_bulkout ;
extern const U16 usbd_cls_buf_sz;


extern U8 USBD_CLS_BulkBuf[];
//(up)

extern void usbd_os_evt_set(U16 event_flags, U32 task);
extern U16 usbd_os_evt_get(void);
extern U32 usbd_os_evt_wait_or(U16 wait_flags, U16 timeout);

extern const BOOL __rtx;


/*------------------------------------------------------------------------------
 *      USB Device Descriptors
 *----------------------------------------------------------------------------*/
extern const U8 USBD_HID_ReportDescriptor[];
extern const U16 USBD_HID_ReportDescriptorSize;
extern const U16 USBD_HID_DescriptorOffset;
extern const U8 USBD_DeviceDescriptor[];
extern const U8 USBD_DeviceQualifier[];
extern const U8 USBD_DeviceQualifier_HS[];
extern const U8 USBD_ConfigDescriptor[];
extern const U8 USBD_ConfigDescriptor_HS[];
extern const U8 USBD_OtherSpeedConfigDescriptor[];
extern const U8 USBD_OtherSpeedConfigDescriptor_HS[];
extern const U8 USBD_OtherSpeedConfigDescriptor[];
extern const U8 USBD_OtherSpeedConfigDescriptor_HS[];
extern const U8 USBD_StringDescriptor[];


// added by 9527 for WinUSB

extern const U8 WINUSBD_StringDescriptor[];


//typedef struct
//{
//// Header
//uint32_t dwLength;
//uint16_t bcdVersion;
//uint16_t wIndex;
//uint8_t  bCount;
//uint8_t  bReserved1[7];
//// Function Section 1
//uint8_t  bFirstInterfaceNumber1;
//uint8_t  bReserved2;
//uint8_t  bCompatibleID1[8];
//uint8_t  bSubCompatibleID1[8];
//uint8_t  bReserved3[6];
//    // Function Section 2
////    uint8_t  bFirstInterfaceNumber2;
////uint8_t  bReserved4;
////uint8_t  bCompatibleID2[8];
////uint8_t  bSubCompatibleID2[8];
////uint8_t  bReserved5[6];
////    // Function Section 3
////    uint8_t  bFirstInterfaceNumber3;
////uint8_t  bReserved6;
////uint8_t  bCompatibleID3[8];
////uint8_t  bSubCompatibleID3[8];
////uint8_t  bReserved7[6];
//} USBD_CompatIDDescStruct;

extern const U8 USBD_CompatIDDesc;
//typedef __packed struct
//{
//// Header
//uint32_t dwLength;
//uint16_t  bcdVersion;
//uint16_t  wIndex;
//uint16_t  wCount;
//// Custom Property Section 1
//uint32_t dwSize;
//uint32_t dwPropertyDataType;
//uint16_t  wPropertyNameLength;
//uint16_t  bPropertyName[20];
//uint32_t dwPropertyDataLength;
//uint16_t  bPropertyData[39];
//} USBD_ExtPropertiesDescStruct;
//extern const USBD_ExtPropertiesDescStruct 
extern const U8 USBD_ExtPropertiesDesc;
#endif  /* __USB_LIB_H__ */

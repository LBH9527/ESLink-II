/**
 * @file    error.h
 * @brief   collection of known errors and accessor for the friendly string
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

#ifndef ERROR_H
#define ERROR_H

#ifdef __cplusplus
}
#endif

//ISP_ERR_BUSY,									/**< Device is busy */
//	ISP_ERR_GEN,									/**< General error */
//	ISP_ERR_NOTSUPPORTED,					/**< Capability not supported */
//	ISP_ERR_ALIGNERR,							/**< Attempted to do an operation on an unaligned section of the device */
//	ISP_ERR_LOCKED,								/**< Device was locked and a program/erase operation was attempted */
//	ISP_ERR_PROGERR,							/**< Error programming device (blocking mode only) */
//	ISP_ERR_ERASEERR,							/**< Erase error (blocking mode only) */
//	ISP_ERR_NOTBLANK,							/**< Program operation on block that is not blank */
//	ISP_ERR_PAGESIZE,							/**< PageProgram write size exceeds page size */
//	ISP_ERR_VAL,									/**< Program operation failed validation or readback compare */
//	ISP_ERR_RANGE,								/**< Range error, bad block number, address out of range, etc. */
//	ISP_ERR_MEMMODE,							/**< Library calls not allowed while in memory mode. */
// Keep in sync with the lists error_message and error_type
typedef enum {
    /* Shared errors */
    ERROR_SUCCESS = 0,
    ERROR_FAILURE,
    ERROR_INTERNAL,

    /*spi flash*/
    ERROR_SF_ERASE,
    ERROR_SF_WRITE,
    ERROR_SF_READ,
    ERROR_SF_ADDR_OUT_BOUND,
    
    //ERROR_APP            //更新时序    
    ERROR_APP1_INIT,
    
    /* Flash IAP interface */
    ERROR_IAP_INIT,
    ERROR_IAP_UNINIT,
    ERROR_IAP_WRITE,
    ERROR_IAP_ERASE_SECTOR,
    ERROR_IAP_ERASE_ALL,
    ERROR_IAP_OUT_OF_BOUNDS,
    ERROR_IAP_UPDT_NOT_SUPPORTED,
    ERROR_IAP_UPDT_INCOMPLETE,
    ERROR_IAP_NO_INTERCEPT,
    ERROR_BL_UPDT_BAD_CRC,
    //
    ERROR_INTF_ERROR,
    
    //配置字下载
    ERROR_USER_HEX_DL  ,
    ERROR_TIMING_INFO_DL  ,
    ERROR_TIMING_DL  ,
    ERROR_CONFIG_DL  ,
    
    
    
    
    /* Target flash errors */
    ERROR_RESET,
    ERROR_ALGO_DL,
    ERROR_ALGO_DATA_SEQ,
    ERROR_INIT,
    ERROR_SECURITY_BITS,
    ERROR_UNLOCK,
    ERROR_ERASE_SECTOR,
    ERROR_ERASE_ALL,
    ERROR_WRITE,

    //bootisp
    ERROR_BOOTISP_START,        //启动失败
    ERROR_BOOTISP_CHECK_EMPTY,  //擦空失败
    ERROR_BOOTISP_WRITE,
    /* FileSystem interface */
    ERROR_FS_INIT,
    ERROR_FS_CREAT,
    ERROR_OFL_PRJ_OPEN,         //脱机文件打开失败
    ERROR_OFL_PRJ_CREAT,        //脱机文件创建失败
    ERROR_OFL_PRJ_REPEAT,       //脱机文件已存在
    
    ERROR_OFL_PRJ_READ,
    
    
    FRAMEHEADERROR             = 0xA0,
    DEVICETYPEERROR            = 0xA1,
    FUNCTIONCODEERROR          = 0xA2,
    COMMANDPARITYERROR         = 0xA3,
    DEVIVEBUSY                 = 0xA4,
    USERHEX_DL_FAIL              = 0xA7,
    UNCLOCKFAIL                = 0xA8,
    ERASEFAIL                  = 0xA9,
    CHECHEMPTYFAIL             = 0xAA,
    HEXPRGFAIL                 = 0xAB,
    PRGCONFIGFAIL              = 0xAC,
    CONFIGVERIFYFAIL           = 0xAD,
    ENCRYPFAIL                 = 0xAE,
    ERROR_HEX_CHECKSUM_FAIL            = 0xAF,

    CONFIGCHECKSUMFAIL         = 0xB0,
    SWDOPERATIONFAIL           = 0xB2,
   
    //  CONFIGRDFAIL                 0xB4
    SERIALNUMCHECKSUMFAIL      = 0xB5,
    CODEVERIFYFAIL             = 0xB6,

    HOSTOVERTIME               = 0xC0,
    POWERSETFAIL               = 0xC1,
    POWERCONFLICT              = 0xC2,
    SPEEDSETFAIL               = 0xC3,
    TIMINGINFORDLFAIL          = 0xC4,
    UPDATAADDERROR             = 0xC5,
    UPDATAFAIL                 = 0xC6,
    UPDATAPARITYERROR          = 0xC7,
    SWITCHFAIL                 = 0xC8 ,  
    
    ISP_RUN_OK                 = 0xF4,
	ISP_ERR_LASTINDEX,

    
    
    
    // Add new values here

    ERROR_COUNT
} error_t;

const char *error_get_string(error_t error);

typedef unsigned char error_type_t;

#define ERROR_TYPE_INTERNAL 0x1
#define ERROR_TYPE_TRANSIENT 0x2
#define ERROR_TYPE_USER 0x4
#define ERROR_TYPE_TARGET 0x8
#define ERROR_TYPE_INTERFACE 0x10
// If you add another error type:
// 1. update error_type_names, used by read_file_fail_txt()
// 2. update ERROR_TYPE_MASK
// 3. make sure that error type bits still fit inside of error_type_t
#define ERROR_TYPE_MASK 0x1F

error_type_t error_get_type(error_t error);

#ifdef __cplusplus
}
#endif

#endif

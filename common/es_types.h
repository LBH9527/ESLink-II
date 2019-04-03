/**
  **************************************************************************************
  * @file    es_type.h
  * @brief   
  *
  * @version V0.01
  * @data    4/3/2018
  * @author  Eastsoft AE Team
  * @note
  *
  * Copyright (C) 2018 Shanghai Eastsoft Microelectronics Co., Ltd. ALL rights reserved.
  *
  **************************************************************************************
  */
/**
 * \file
 * \brief 通用数据类型相关定义头文件
 *
 * \internal
 * \par Modification history
 * - 1.00 14-11-01  9527, first implementation.
 * \endinternal
 */

#ifndef __ES_TYPES_H
#define __ES_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include <stdbool.h>        //for bool
//#include "am_errno.h"


/******************************************************************************/

/**
 * \name 通用常量定义
 * @{
 */
#define ES_OK               0         /**< \brief OK               */
#define ES_ERROR          (-1)        /**< \brief 一般错误         */

#define ES_NO_WAIT          0         /**< \brief 超时：不等待     */
#define ES_WAIT_FOREVER   (-1)        /**< \brief 超时：永远等待   */

#ifndef EOF
#define EOF               (-1)        /**< \brief 文件结束         */
#endif

#define NONE              (-1)        /**< \brief 每当为空时不执行 */
#define EOS               '\0'        /**< \brief C字符串结束      */

#ifndef NULL
#define NULL             ((void *)0)  /**< \brief 空指针           */
#endif

#define ES_LITTLE_ENDIAN  1234        /**< \brief 小端模式         */
#define ES_BIG_ENDIAN     3412        /**< \brief 大端模式         */

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

//typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;

/** @} */

/**
 * \addtogroup am_if_types
 * \copydoc am_types.h
 * @{
 */
//#ifdef __GNUC__

//typedef _Bool bool_t;
//#define TRUE   1
//#define FALSE  0
//#else

//#ifndef TRUE

/**
 * \brief 布尔类型定义
 */
//typedef enum {
//    FALSE = 0, 
//    TRUE  = !FALSE
//} bool_t;

#ifndef TRUE
  #define TRUE            1
#endif

#ifndef FALSE
  #define FALSE           0
#endif

#ifndef DISABLE
    #define DISABLE         0
#endif

#ifndef ENABLE
    #define ENABLE          1
#endif
//#endif /* TRUE */

//#endif

/**
 * \brief 空指针定义
 */
#ifndef NULL
#define NULL      ((void *)0)
#endif
 
#ifdef __cplusplus
  #define   __I     volatile             /**< Defines 只读     */
#else
  #define   __I     volatile const       /**< Defines 只读     */
#endif
#define     __O     volatile             /**< Defines 只写     */
#define     __IO    volatile             /**< Defines 可读可写 */


/**
 * @} 
 */
 
#ifdef __cplusplus
}
#endif
 
#endif /* __AM_TYPES_H */

/* end of file */

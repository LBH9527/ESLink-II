/**
 * @file    daplink_debug.h
 * @brief   optional trace messages useful in development
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

#ifndef ESLINK_DEBUG_H
#define ESLINK_DEBUG_H

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "RTL.h"
#include "rl_usb.h"
#include "main.h"

#define USB_DEBUG
//#define UART_DEBUG

#ifdef util_assert
  #undef util_assert
#endif
#define util_assert(EXPR)                                                           \
  if (!(EXPR))                                                                   \
  {                                                                              \
//    printf("(%s) has assert failed at %s.\n", #EXPR, __FUNCTION__);            \
while (1);

\
}


#if defined (USB_DEBUG)

static const char error_msg[] = "\r\n<OVERFLOW>\r\n";

static inline uint32_t daplink_debug(uint8_t *buf, uint32_t size)
{
  uint32_t total_free;
  uint32_t write_free;
  uint32_t error_len = strlen(error_msg);

  if (usb_state !=  USB_CONNECTED)
    return 0;

  total_free = USBD_CDC_ACM_DataFree();

  if (total_free < error_len)
  {
    // No space
    return 0;
  }

  // Size available for writing
  write_free = total_free - error_len;
  size = MIN(write_free, size);
  USBD_CDC_ACM_DataSend(buf, size);

  if (write_free == size)
  {
    USBD_CDC_ACM_DataSend((uint8_t *)error_msg, error_len);
  }

  return size;
}

static char daplink_debug_buf[128] = {0};
static inline uint32_t daplink_debug_print(const char *format, ...)
{
  uint32_t ret;
  int32_t r = 0;
  va_list arg;
  ret = 1;
  va_start(arg, format);
  r = vsnprintf(daplink_debug_buf, sizeof(daplink_debug_buf), format, arg);

  if (r >= sizeof(daplink_debug_buf))
  {
    r = snprintf(daplink_debug_buf, sizeof(daplink_debug_buf), "<Error - string length %i exceeds print buffer>\r\n", r);
    ret = 0;
  }

  va_end(arg);
  daplink_debug((uint8_t *)daplink_debug_buf, r);
  return ret;
}

//#elseif defined (USB_DEBUG)



#else

static inline uint32_t daplink_debug_print(const char *format, ...)
{
  return 1;
}

static inline uint32_t daplink_debug(uint8_t *data, uint32_t size)
{
  return 1;
}

#endif

#define debug_msg(fmt, args...) daplink_debug_print(fmt, ## args);
#define debug_data(buf, size) daplink_debug(buf, size);



#endif







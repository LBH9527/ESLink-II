/**
 * @file    main.h
 * @brief
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

#ifndef MAIN_H
#define MAIN_H

#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @enum main_usb_busy_t
 *  @brief Defines the filtered activity of the USB connection
 */
typedef enum {
    MAIN_USB_IDLE,   /*!< MAIN_USB_IDLE value means no bus activity */
    MAIN_USB_ACTIVE  /*!< MAIN_USB_ACTIVE value means MSC transfers are occuring */
} main_usb_busy_t;

/**
 *  @enum main_usb_connect_t
 *  @brief State the USB connection can be in
 */
typedef enum {
    USB_DISCONNECTED,
    USB_CONNECTING,
    USB_CONNECTED,
    USB_CHECK_CONNECTED,
    USB_CONFIGURED,
    USB_DISCONNECTING
} main_usb_connect_t;

extern main_usb_connect_t usb_state;
/**
 *  @enum main_led_state_t
 *  @brief Statest the USB connection can be in
 */
typedef enum {
    MAIN_LED_OFF = 0,
    MAIN_LED_FLASH,
    MAIN_LED_FLASH_PERMANENT
} main_led_state_t;

/**
 *  Allows other parts of the program to request the device to eject physical media
 *  @param  none
 *  @return none
 */
void main_msc_disconnect_event(void);

/**
 *  Delay the disconnect if there is one in progress
 *  @param  none
 *  @return none
 */
void main_msc_delay_disconnect_event(void);

/**
 *  Allows other parts of the program to request the device to immediately eject physical media
 *  @param  none
 *  @return none
 */
void main_force_msc_disconnect_event(void);

/**
 *  Allows other parts of the program to request a LED to toggle state
 *  @param  permanent decides if the led should remain on or flash
 *  @return none
 */
void main_blink_msc_led(main_led_state_t permanent);

void main_reset_target(uint8_t send_unique_id);
void main_usb_set_test_mode(bool enabled);
void main_usb_configure_event(void);
void main_usb_busy_event(void);
void main_powerdown_event(void);
void main_disable_debug_event(void);
void main_hid_send_event(void);
void main_cdc_send_event(void);
void main_msc_disconnect_event(void);
void main_msc_delay_disconnect_event(void);
void main_force_msc_disconnect_event(void);
void main_blink_hid_led(main_led_state_t permanent);
void main_blink_msc_led(main_led_state_t permanent);
void main_blink_cdc_led(main_led_state_t permanent);

void main_es_send_event(void);


#ifdef __cplusplus
}
#endif

#endif

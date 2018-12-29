#ifndef _EEPROM_PORT_H
#define _EEPROM_PORT_H   

#include "errno.h"

//EEPROM中的数据地址
#define EE_OFL_PRJ_NAME                 0x0000      //脱机工程名，用做退出脱机状态时，会写脱机序列号信息
#define EE_OFL_SERIAL_NUMBER_PART_ADDR  0x20        //方案分区信息
#define EE_SERIAL_NUMBER_ADDR           0x30        //序列号地址  

#define EE_RTC_SELF_CALI_VALUE_ADDR     0xA0        //rtc自校正值地址  

#define EE_ESLINK_SERIAL_NUMBER_ADDR    0xF0        //eslink ii 产品序列号


error_t set_offline_project_name(uint8_t *buf, uint8_t size);
error_t get_offline_project_name(uint8_t *buf, uint8_t size);
error_t set_offline_serial_number(uint8_t *buf, uint8_t size);
error_t get_offline_serial_number(uint8_t *buf, uint8_t size);
error_t set_offline_partition(uint8_t *buf, uint8_t size);
error_t get_offline_partition(uint8_t *buf, uint8_t size);
error_t get_rtc_self_calibrate(uint8_t *buf, uint8_t size);
error_t set_rtc_self_calibrate(uint8_t *buf, uint8_t size);
error_t set_eslinkii_serial_number(uint8_t *buf, uint8_t size);
error_t get_eslinkii_serial_number(uint8_t *buf, uint8_t size);

#endif

#include "es_common.h"

#include "eeprom.h"
#include "eeprom_port.h"


//读脱机文件名
error_t get_offline_project_name(uint8_t *buf, uint8_t size)
{
  uint8_t ret;
  fm24cxx_read(EE_OFL_PRJ_NAME, buf, size);

  if (ret != TRUE)
    return ERROR_EE_READ;

  return ERROR_SUCCESS;
}
//写脱机文件名
error_t set_offline_project_name(uint8_t *buf, uint8_t size)
{
  uint8_t ret;
  ret = fm24cxx_write(EE_OFL_PRJ_NAME, buf, size);

  if (ret != TRUE)
    return ERROR_EE_WRITE;

  return ERROR_SUCCESS;
}

//获取脱机序列号
error_t get_offline_serial_number(uint8_t *buf, uint8_t size)
{
  uint8_t ret;
  ret = fm24cxx_read(EE_SERIAL_NUMBER_ADDR, buf, size);

  if (ret != TRUE)
    return ERROR_EE_READ;

  return ERROR_SUCCESS;
}
error_t set_offline_serial_number(uint8_t *buf, uint8_t size)
{
  uint8_t ret;

  ret = fm24cxx_write(EE_SERIAL_NUMBER_ADDR, buf, size);

  if (ret != TRUE)
    return ERROR_EE_WRITE;

  return ERROR_SUCCESS;
}

//读脱机序列号的分区信息
error_t get_offline_partition(uint8_t *buf, uint8_t size)
{
  uint8_t ret;

  ret = fm24cxx_read(EE_OFL_SERIAL_NUMBER_PART_ADDR, buf, size);

  if (ret != TRUE)
    return ERROR_EE_READ;

  return ERROR_SUCCESS;
}

//写脱机序列号的分区信息
error_t set_offline_partition(uint8_t *buf, uint8_t size)
{
  uint8_t ret;
  ret = fm24cxx_write(EE_OFL_SERIAL_NUMBER_PART_ADDR, buf, size);

  if (ret != TRUE)
    return ERROR_EE_WRITE;

  return ERROR_SUCCESS;

}

//写RTC自校正值
error_t set_rtc_self_calibrate(uint8_t *buf, uint8_t size)
{
  uint8_t ret;

  ret = fm24cxx_write(EE_RTC_SELF_CALI_VALUE_ADDR, buf, size);

  if (ret != TRUE)
    return ERROR_EE_WRITE;

  return ERROR_SUCCESS;
}
//获取自校正值
error_t get_rtc_self_calibrate(uint8_t *buf, uint8_t size)
{
  uint8_t ret;
  ret = fm24cxx_read(EE_RTC_SELF_CALI_VALUE_ADDR, buf, size);

  if (ret != TRUE)
    return ERROR_EE_READ;

  return ERROR_SUCCESS;
}



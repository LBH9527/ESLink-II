//M939 info偏移地址为0x40000
#include "eslink.h"
#include "errno.h"
#include "sflash_port.h"
#include "program_port.h"
#include "uartboot_prog_intf.h"
#include "uartboot_host.h"
#include "uartboot_target_config.h"
#include "target_config.h"

#define FLASH_AREA      1
#define INFO_AREA       0
//一次编程支持的长度，根据RAM大小可以修改.长度需要为2^n
#define BOOTISP_PRG_SIZE  1024

static void uartboot_init(es_target_cfg *target);
static error_t uartboot_prog_init(void); //进模式
static error_t uartboot_prog_uninit(void); //退出模式

static error_t uartboot_prog_erase_chip(uint8_t para);
static error_t uartboot_prog_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t uartboot_prog_read_chipid(uint8_t *buf);
static error_t uartboot_prog_read_checksum(uint8_t *buf);
static error_t uartboot_prog_encrypt_chip(void);
static error_t uartboot_chipid_check(void);
static error_t uartboot_prog_program_config(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t uartboot_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size);
static error_t uartboot_prog_verify_config(uint32_t addr,  uint8_t *buf, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t uartboot_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t uartboot_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t uartboot_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t uartboot_target_program_config_all(uint32_t *failed_addr);
static error_t uartboot_target_program_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr);
static error_t uartboot_target_verify_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr, uint32_t *failed_data);

struct  es_prog_ops uartboot_prog_intf =
{
  uartboot_init,
  uartboot_prog_init,
  uartboot_prog_uninit,
  uartboot_prog_erase_chip,
  uartboot_prog_check_empty,
  uartboot_prog_read_chipid,
  uartboot_chipid_check,
  uartboot_prog_read_checksum,
  uartboot_prog_encrypt_chip,

  uartboot_prog_program_config,
  uartboot_prog_read_config,
  uartboot_prog_verify_config,
  uartboot_prog_program_flash,
  uartboot_prog_read_flash,
  uartboot_prog_verify_flash,
  uartboot_target_program_config_all,
  uartboot_target_program_all,
  uartboot_target_verify_all,
  0
};
static const es_target_cfg *target_dev;

/*******************************************************************************
* 函 数 名: uartboot_program_flash
* 功能说明:
* 形    参: area:flash空间
* 返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_program_flash(uint8_t area, uint32_t addr, uint8_t *buf, uint32_t size)
{
  if (area == INFO_AREA)
    addr += CHIP_INFO_FLASH_BASE;

  if (uartboot_write_memory(addr, buf, size) != TRUE)
    return  ERROR_UARTBOOT_WRITE;

  return ERROR_SUCCESS;
}
/*******************************************************************************
* 函 数 名: uartboot_read_flash
* 功能说明:
* 形    参: area:flash空间
* 返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_read_flash(uint8_t area, uint32_t addr, uint8_t *buf, uint32_t size)
{
  if (area == INFO_AREA)
    addr += CHIP_INFO_FLASH_BASE;

  if (uartboot_read_memory(addr, buf, size) != TRUE)
    return ERROR_UARTBOOT_READ;

  return ERROR_SUCCESS;
}
/*******************************************************************************
* 函 数 名: uartboot_set_target_reset
* 功能说明:
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_set_target_reset(void)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t addr;
  uint32_t data;

  addr = 0x40080000;
  data = 0x55AA6996;

  ret = uartboot_program_flash(FLASH_AREA, addr, (uint8_t *)&data, 4);

  if (ret != ERROR_SUCCESS)
    return ret;

  addr = 0x40080824;
  data = 0x00000001;

  ret = uartboot_program_flash(FLASH_AREA, addr, (uint8_t *)&data, 4);

  if (ret != ERROR_SUCCESS)
    return ret;

  return ERROR_SUCCESS;
}
/*******************************************************************************
* 函 数 名:
* 功能说明:
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static void uartboot_init(es_target_cfg *target)
{
  target_dev = target;
  uartboot_prog_intf.cb = online_file_read;
  uart_initialize();
}
/*******************************************************************************
* 函 数 名: uartboot_prog_init
* 功能说明: 进模式
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_init(void) //
{
  error_t ret = ERROR_SUCCESS;

  if (uartboot_start() != TRUE)
  {
    eslink_set_target_reset_run(5);

    if (uartboot_start() != TRUE)
      return ERROR_UARTBOOT_START;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
* 函 数 名:
* 功能说明: 退出模式
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_uninit(void)
{
  return ERROR_SUCCESS;
}

//擦除
#define FULL_ERASE_CMD      0xFFFF
#define FULL_ERASE_CMD_H    0xFF
#define FULL_ERASE_CMD_L    0xFF
/*******************************************************************************
*  函 数 名: uartboot_prog_erase_chip
*  功能说明: 擦除.芯片加密后
*  形    参:
*  返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_erase_chip(uint8_t para)
{
  uint8_t data[4];
  error_t ret = ERROR_SUCCESS;

  ret = uartboot_chipid_check();

  if ((ERROR_SUCCESS != ret) && (ERROR_LV2_ENCRYPT != ret))
    return ret;

  //flash 擦除（包括falsh区和Info2）
  data[0] = FULL_ERASE_CMD_H;
  data[1] = FULL_ERASE_CMD_L;

  if (uartboot_extended_erase(data, 2) != TRUE)
  {
    return ERROR_UARTBOOT_ERASE;
  }

  //info1页擦
  data[0] = 0x00;
  data[1] = 0x00;     //页数
  data[2] = (CHIP_INFO1_INDEX & 0xff00) >> 8;
  data[3] = CHIP_INFO1_INDEX & 0x00ff;

  if (uartboot_extended_erase(data, 4) != TRUE)
  {
    return ERROR_UARTBOOT_ERASE;
  }

  eslink_set_target_reset_run(5);

  if (uartboot_start() != TRUE)
    return ERROR_UARTBOOT_START;

  return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名: uartboot_prog_check_empty
*  功能说明: 查空
*  形    参: failed_addr：错误地址  failedData:错误hsuju
*  返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_check_empty(uint32_t *failed_addr, uint32_t *failed_data)
{
  error_t ret = ERROR_SUCCESS;

  ret = uartboot_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  if (uartboot_check_empty(target_dev->code_start, target_dev->code_size) != TRUE)
  {
    if (failed_addr)
      *failed_addr = 0xFFFFFFFF ;

    if (failed_data)
      *failed_data = 0xFFFFFFFF ;

    return ERROR_UARTBOOT_CHECK_EMPTY;
  }

  if (uartboot_check_empty(CHIP_INFO1_ADDR, CHIP_INFO_SIZE) != TRUE)
  {
    if (failed_addr)
      *failed_addr = 0xFFFFFFFF ;

    if (failed_data)
      *failed_data = 0xFFFFFFFF ;

    return ERROR_UARTBOOT_CHECK_EMPTY;
  }

  return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名: uartboot_prog_read_chipid
*  功能说明: 读chipid
*  形    参:
*  返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_read_chipid(uint8_t *buf)
{
  error_t ret = ERROR_SUCCESS;
  ret = uartboot_read_flash(INFO_AREA, target_dev->chipid_addr, buf, 4);
  return ret;
}
/*******************************************************************************
*  函 数 名: uartboot_chipid_check
*  功能说明: id检测
*  形    参:
*  返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_chipid_check(void)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t chipid = 0;
  uint32_t reg_data ;

  ret = uartboot_read_flash(INFO_AREA, target_dev->chipid_addr, (uint8_t *)&chipid, 4);

  if (ret != ERROR_SUCCESS)
    return ret;

  if (chipid != target_dev->chipid_value)
  {
    ret = uartboot_read_flash(INFO_AREA, CHIP_CFG_GBRDP_ADDR, (uint8_t *)&reg_data, 4);

    if (ret != ERROR_SUCCESS)
      return ret;

    if ((chipid == 0x00000000) && (reg_data == 0x00000000))
      return  ERROR_LV2_ENCRYPT;

    return  ERROR_CHIP_ID_NOT_MATCH;
  }

  return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名:  uartboot_prog_read_checksum
*  功能说明:  读校验和
*  形    参:
*  返 回 值:  错误类型
*******************************************************************************/
static error_t uartboot_prog_read_checksum(uint8_t *buf)
{
  error_t ret;
  uint8_t checksum_h[4], checksum_l[4];

  ret = uartboot_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  ret = uartboot_read_flash(INFO_AREA, CHIP_CHECKSUM_ADDR, checksum_l, 4);

  if (ret != ERROR_SUCCESS)
    return ret;

  ret = uartboot_read_flash(INFO_AREA, CHIP_CHECKSUMN_ADDR, checksum_h, 4);

  if (ret != ERROR_SUCCESS)
    return ret;

  //参考上位机校验和取数据方式来返回数据
  if (buf)
  {
    *buf++ = checksum_l[0];
    *buf++ = checksum_l[1];
    *buf++ = checksum_h[0];
    *buf++ = checksum_h[1];
  }

  return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名:  uartboot_prog_encrypt_chip
*  功能说明:  加密
*  形    参:
*  返 回 值:  错误类型
*******************************************************************************/
static error_t uartboot_prog_encrypt_chip(void)
{
  error_t ret = ERROR_SUCCESS;
  static uint8_t reg_data[8] = {0xff};

  ret = uartboot_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  ret = uartboot_prog_intf.cb(CFG_WORD, CHIP_CFG_GBRDP_OFFSET, reg_data, sizeof(reg_data) / sizeof(uint8_t));

  if (ret !=  ERROR_SUCCESS)
    return ret;

  ret = uartboot_program_flash(INFO_AREA, CHIP_CFG_GBRDP_ADDR, reg_data, sizeof(reg_data) / sizeof(uint8_t));

  if (ret != ERROR_SUCCESS)
    return ERROR_UARTBOOT_ENCRYPT;

  return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名: uartboot_prog_program_config
*  功能说明: 配置字编程
*  形    参:
*  返 回 值:
*******************************************************************************/
static error_t uartboot_prog_program_config(uint32_t addr, uint8_t *buf, uint32_t size, uint32_t *failed_addr)
{
  uint8_t ret = ERROR_SUCCESS ;
  uint32_t prog_addr;
  uint32_t prog_size;
  const struct info_part_map *part;
  uint32_t i;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  for (i = 0; i < ITEM_NUM(info_part_map); i++)
  {
    part = &info_part_map[i];
    prog_addr = part->addr;
    prog_size = part->size ;
    ret = uartboot_program_flash(INFO_AREA, prog_addr, buf, prog_size);

    if (ret != ERROR_SUCCESS)
    {
      if (failed_addr)
        *failed_addr = 0xFFFFFFFF ;

      return ERROR_UARTBOOT_PROG_CFG_WORD;
    }

    buf += prog_size;
  }

  return ERROR_SUCCESS;
}
/*******************************************************************************
* 函 数 名: uartboot_prog_read_config
* 功能说明: 读配置字
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t read_addr;
  uint32_t read_size;
  const struct info_part_map *part;
  uint32_t i;

  ret = uartboot_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  for (i = 0; i < ITEM_NUM(info_part_map); i++)
  {
    part = &info_part_map[i];
    read_addr = part->addr;;
    read_size = part->size ;

    ret = uartboot_read_flash(INFO_AREA, read_addr, buf, read_size);

    if (ret != ERROR_SUCCESS)
      return ret;

    buf += read_size;
  }

  return ERROR_SUCCESS;
}
/*******************************************************************************
* 函 数 名: uartboot_prog_verify_config
* 功能说明: 配置字校验
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_verify_config(uint32_t addr,  uint8_t *buf, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t i;
  uint8_t read_buf[BOOTISP_PRG_SIZE];
  uint32_t verify_size;

  uint32_t read_addr;
  uint32_t read_size;
  const struct info_part_map *part;
  uint32_t item_num;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  for (item_num = 0; item_num < ITEM_NUM(info_part_map); item_num++)
  {
    part = &info_part_map[item_num];
    read_addr = part->addr;
    read_size = part->size;

    while (read_size > 0)
    {
      verify_size = MIN(read_size, sizeof(read_buf));

      ret = uartboot_read_flash(INFO_AREA, read_addr, read_buf, verify_size);

      if (ret != ERROR_SUCCESS)
        return ret;

      for (i = 0; i < verify_size; i++)
      {
        if (*buf++ != read_buf[i])
        {
          if (failed_addr)
            *failed_addr = addr + ROUND_DOWN(i, 4)  ;

          if (failed_data)
          {
            *failed_data |= (read_buf[ROUND_DOWN(i, 4)] << 0) ;
            *failed_data |= (read_buf[ROUND_DOWN(i, 4) + 1] << 8) ;
            *failed_data |= (read_buf[ROUND_DOWN(i, 4) + 2] << 16) ;
            *failed_data |= (read_buf[ROUND_DOWN(i, 4) + 3] << 24) ;
          }

          return ERROR_UARTBOOT_VERIFY;
        }
      }

      read_addr += verify_size;
      read_size -= verify_size;
    }
  }

  return ERROR_SUCCESS;
}
/*******************************************************************************
* 函 数 名: uartboot_prog_program_flash
* 功能说明: flash编程
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr)
{
  error_t ret = ERROR_SUCCESS;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  ret = uartboot_program_flash(FLASH_AREA, addr, data, size) ;

  if (ret != ERROR_SUCCESS)
  {
    if (failed_addr)
      *failed_addr = 0xFFFFFFFF ;

    return ret;
  }

  return ERROR_SUCCESS;
}
/*******************************************************************************
* 函 数 名: uartboot_prog_read_flash
* 功能说明: 读flash
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size)
{
  error_t ret = ERROR_SUCCESS;

  ret = uartboot_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  ret = uartboot_read_flash(FLASH_AREA, addr, data, size);

  if (ret != ERROR_SUCCESS)
    return ret;


  return ERROR_SUCCESS;
}
/*******************************************************************************
* 函 数 名: uartboot_prog_verify_flash
* 功能说明: flash校验
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t i;
  uint8_t read_buf[BOOTISP_PRG_SIZE];
  uint32_t verify_size;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  while (size > 0)
  {
    verify_size = MIN(size, sizeof(read_buf));

    ret = uartboot_read_flash(FLASH_AREA, addr, read_buf, verify_size);

    if (ret != ERROR_SUCCESS)
      return ret;


    for (i = 0; i < verify_size; i++)
    {
      if (data[i] != read_buf[i])
      {
        if (failed_addr)
          *failed_addr = addr + ROUND_DOWN(i, 4)  ;

        if (failed_data)
        {
          *failed_data |= (read_buf[ROUND_DOWN(i, 4)] << 0) ;
          *failed_data |= (read_buf[ROUND_DOWN(i, 4) + 1] << 8) ;
          *failed_data |= (read_buf[ROUND_DOWN(i, 4) + 2] << 16) ;
          *failed_data |= (read_buf[ROUND_DOWN(i, 4) + 3] << 24) ;
        }

        return ERROR_UARTBOOT_VERIFY;
      }
    }

    addr += verify_size;
    size -= verify_size;
  }

  return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名:  uartboot_target_program_config_all
*  功能说明:  配置字编程
*  形    参:  failed_addr  ： 编程失败地址
*  返 回 值:
*******************************************************************************/
static error_t uartboot_target_program_config_all(uint32_t *failed_addr)
{
  error_t ret = ERROR_SUCCESS;

  uint32_t cfg_word_addr;
  uint32_t cfg_word_size;

  uint32_t copy_size;
  uint32_t read_addr;
  uint8_t read_buf[BOOTISP_PRG_SIZE] = {0x00};

  ret = uartboot_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  cfg_word_addr =  target_dev->config_word_start;
  cfg_word_size =  target_dev->config_word_size;
  read_addr =  0;

  while (true)
  {
    copy_size = MIN(cfg_word_size, sizeof(read_buf));

    ret = uartboot_prog_intf.cb(CFG_WORD, read_addr, read_buf, copy_size);

    if (ERROR_SUCCESS != ret)
      return ret;

    ret = uartboot_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr);

    if (ERROR_SUCCESS != ret)
      return ret;

    // Update variables
    cfg_word_addr  += copy_size;
    cfg_word_size  -= copy_size;
    read_addr += copy_size;

    // Check for end
    if (cfg_word_size <= 0)
      break;
  }

  return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名: uartboot_target_program_all
*  功能说明: 编程  编程256k时间需要3分钟
*  形    参:
*  返 回 值:
*******************************************************************************/
static error_t uartboot_target_program_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t i;

  uint32_t code_addr;
  uint32_t code_size;
  uint32_t cfg_word_addr;
  uint32_t cfg_word_size;

  uint32_t copy_size;
  uint32_t read_addr;
  uint8_t read_buf[BOOTISP_PRG_SIZE];

  ret = uartboot_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  code_addr =  target_dev->code_start;
  code_size =  target_dev->code_size;
  read_addr =  0;

  while (true)
  {
    copy_size = MIN(code_size, sizeof(read_buf));
    ret = uartboot_prog_intf.cb(USER_HEX, read_addr, read_buf, copy_size);

    if (ERROR_SUCCESS != ret)
      return ret;

    if (sn_enable == ENABLE)    //序列号代码使能
      serial_number_intercept_write(sn, code_addr, read_buf, copy_size);  //填入序列号

    for (i = 0; i < copy_size; i++)
    {
      if (read_buf[i] != 0xFF)
        break;
    }

    if (i < copy_size)     //数据段都为0xFF,不进行编程
    {
      ret = uartboot_prog_program_flash(code_addr, read_buf, copy_size, failed_addr);

      if (ret !=  ERROR_SUCCESS)   //编程失败，返回编程失败地址
        return ret;
    }

    // Update variables
    code_addr  += copy_size;
    code_size  -= copy_size;
    read_addr += copy_size;

    // Check for end
    if (code_size <= 0)
      break;
  }

  //BootPin拉低时，不握手会变成失败
  if (uartboot_start() != TRUE)
    return ERROR_UARTBOOT_START;

  cfg_word_addr =  target_dev->config_word_start;
  cfg_word_size =  target_dev->config_word_size;
  read_addr =  0;

  while (true)
  {
    copy_size = MIN(cfg_word_size, sizeof(read_buf));
    ret = uartboot_prog_intf.cb(CFG_WORD, read_addr, read_buf, copy_size);

    if (ERROR_SUCCESS != ret)
      return ret;

    ret = uartboot_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    // Update variables
    cfg_word_addr  += copy_size;
    cfg_word_size  -= copy_size;
    read_addr += copy_size;

    // Check for end
    if (cfg_word_size <= 0)
      break;
  }

  return ret;
}
/*******************************************************************************
*  函 数 名: uartboot_target_verify_all
*  功能说明: 芯片数据校验
*  形    参: sn_enable：序列号是否使能  sn：序列号  failed_addr：错误地址
*             failed_data：错误数据
*  返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_target_verify_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr, uint32_t *failed_data)
{
  error_t ret = ERROR_SUCCESS;

  uint32_t checksum = 0;
  uint32_t sf_checksum = 0;   //spi保存的校验和
  uint32_t code_addr;
  uint32_t code_size;
  uint32_t cfg_word_addr;
  uint32_t cfg_word_size;
  uint32_t verify_size;
  uint32_t sf_addr;
  uint8_t sf_buf[BOOTISP_PRG_SIZE];

  ret = uartboot_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  code_addr =  target_dev->code_start;
  code_size =  target_dev->code_size;
  sf_addr = 0;

  while (true)
  {
    verify_size = MIN(code_size, sizeof(sf_buf));

    ret = uartboot_prog_intf.cb(USER_HEX, sf_addr, sf_buf, verify_size);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和

    if (sn_enable == ENABLE)
      serial_number_intercept_write(sn, code_addr, sf_buf, verify_size);  //填入序列号

    ret = uartboot_prog_verify_flash(code_addr, sf_buf, verify_size, failed_addr, failed_data);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    // Update variables
    code_addr  += verify_size;
    code_size  -= verify_size;
    sf_addr += verify_size;

    // Check for end
    if (code_size <= 0)
      break;
  }

  uartboot_prog_intf.cb(HEX_CHECKSUM, 0, (uint8_t *)&sf_checksum, 4);

  if ((sf_checksum & 0x0000ffff) != (checksum & 0x0000ffff))
  {
    ret = ERROR_USER_HEX_CHECKSUM;
    return  ret;
  }

  cfg_word_addr =  target_dev->config_word_start;
  cfg_word_size =  target_dev->config_word_size;
  sf_addr =  0;
  checksum = 0;

  while (true)
  {
    verify_size = MIN(cfg_word_size, sizeof(sf_buf));
    ret = uartboot_prog_intf.cb(CFG_WORD, sf_addr, sf_buf, verify_size);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和

    ret = uartboot_prog_verify_config(cfg_word_addr, sf_buf, verify_size, failed_addr, failed_data);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    // Update variables
    cfg_word_addr  += verify_size;
    cfg_word_size  -= verify_size;
    sf_addr += verify_size;

    // Check for end
    if (cfg_word_size <= 0)
      break;
  }

  uartboot_prog_intf.cb(CFG_WORD_CHECKSUM, 0, (uint8_t *)&sf_checksum, 4);

  if (sf_checksum != (checksum & 0x0000ffff))
  {
    ret = ERROR_CFG_WORD_CHECKSUM;
    return  ret;
  }

  return  ret;
}



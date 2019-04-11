#include "eslink.h"
#include "es_common.h"
#include "sflash_port.h"
#include "program_port.h"
#include "swd_host.h"
#include "swd_target_config.h"
#include "swd_prog_intf.h"
#include "swd_target_reset.h"
#include "target_config.h"

#define FLASH_AREA      1
#define INFO_AREA       0
//一次编程支持的长度，根据RAM大小可以修改.长度需要为2^n
#define SWD_PRG_SIZE  1024

static void swd_prog_init(es_target_cfg *target);
static error_t es_swd_init(void); //进模式
static error_t es_swd_uninit(void); //退出模式

static error_t es_swd_erase_chip(uint8_t para);
static error_t es_swd_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t es_swd_read_chipid(uint8_t *buf);
static error_t es_swd_read_checksum(uint8_t *buf);
static error_t es_swd_encrypt_chip(void);
static error_t es_swd_chipid_check(void);
static error_t es_swd_program_config(uint32_t addr, uint8_t *buf, uint32_t size, uint32_t *failed_addr);
static error_t es_swd_read_config(uint32_t addr,  uint8_t *buf, uint32_t size);
static error_t es_swd_verify_config(uint32_t addr,  uint8_t *buf, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t es_swd_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t es_swd_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t es_swd_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t swd_target_program_config_all(uint32_t *failed_addr);
static error_t swd_target_program_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr);
static error_t swd_target_verify_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr, uint32_t *failed_data);

struct  es_prog_ops swd_prog_intf =
{
  swd_prog_init,
  es_swd_init,
  es_swd_uninit,
  es_swd_erase_chip,
  es_swd_check_empty,
  es_swd_read_chipid,
  es_swd_chipid_check,
  es_swd_read_checksum,
  es_swd_encrypt_chip,

  es_swd_program_config,
  es_swd_read_config,
  es_swd_verify_config,
  es_swd_program_flash,
  es_swd_read_flash,
  es_swd_verify_flash,
  swd_target_program_config_all,
  swd_target_program_all,
  swd_target_verify_all,
};

static const es_target_cfg *target_dev;

//swd编程初始化
void swd_prog_init(es_target_cfg *target)
{
  target_dev = target;
  swd_prog_intf.cb = online_file_read;
}

/*******************************************************************************
*  函 数 名: swd_program_flash
*  功能说明: swd编程
*  形    参: area:编程区域flash / info addr：地址 buf：数据 size：长度
*  返 回 值: 错误类型
*******************************************************************************/
static error_t swd_program_flash(uint8_t area, uint32_t addr, uint8_t *buf, uint32_t size)
{
  const program_target_t *const flash = swd_target_device.flash_algo;
  uint32_t entry;

  // check if security bits were set
  if (1 == security_bits_set(addr, (uint8_t *)buf, size))
  {
    return ERROR_SECURITY_BITS;
  }

  if (area == FLASH_AREA)
    entry = flash->program_page;
  else if (area == INFO_AREA)
    entry = flash->program_info_page;
  else
    entry = 0;

  while (size > 0)
  {
    uint32_t write_size = MIN(size, flash->program_buffer_size);

    // Write page to buffer
    if (!swd_write_memory(flash->program_buffer, (uint8_t *)buf, write_size))
    {
      return ERROR_ALGO_DATA_SEQ;
    }

    // Run flash programming
    if (!swd_flash_syscall_exec(&flash->sys_call_s,
                                entry,
                                addr,
                                write_size,
                                flash->program_buffer,
                                0))
    {
      return ERROR_WRITE;
    }

    addr += write_size;
    buf += write_size;
    size -= write_size;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
*  函 数 名: es_swd_init
*  功能说明: 编程初始化，进模式  算法文件写入目标芯片的RAM中
*  形    参:
*  返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_init(void)
{
  const program_target_t *const flash = swd_target_device.flash_algo;

  if (0 == target_set_state(RESET_PROGRAM))
  {
    return ERROR_RESET;
  }

  // Download flash programming algorithm to target and initialise.
  if (0 == swd_write_memory(flash->algo_start, (uint8_t *)flash->algo_blob, flash->algo_size))
  {
    return ERROR_ALGO_DL;
  }

  if (0 == swd_flash_syscall_exec(&flash->sys_call_s, flash->init, swd_target_device.flash_start, 0, 0, 0))
  {
    return ERROR_INIT;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
* 函 数 名:
* 功能说明: 退出编程模式
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_uninit(void)
{
//    if (config_get_auto_rst()) {
//        // Resume the target if configured to do so
//        target_set_state(RESET_RUN);
//    } else {
//        // Leave the target halted until a reset occurs
//        target_set_state(RESET_PROGRAM);
//    }
//    // Check to see if anything needs to be done after programming.
//    // This is usually a no-op for most targets.
//    target_set_state(POST_FLASH_RESET);
//    swd_off();

  return ERROR_SUCCESS;
}

/*******************************************************************************
* 函 数 名: es_swd_erase_chip
* 功能说明: 擦除
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_erase_chip(uint8_t para)
{
  error_t status = ERROR_SUCCESS;
  const program_target_t *const flash = swd_target_device.flash_algo;

  status = es_swd_chipid_check();


  if ((ERROR_SUCCESS != status) && (ERROR_LV2_ENCRYPT != status))
    return status;

  //擦除flash
  if (0 == swd_flash_syscall_exec(&flash->sys_call_s, flash->erase_chip, 0, 0, 0, 0))
  {
    return ERROR_SWD_ERASE;
  }

  //擦除info
  if (0 == swd_flash_syscall_exec(&flash->sys_call_s, flash->erase_info, 0, 0, 0, 0))
  {
    return ERROR_SWD_ERASE;
  }

  // Reset and re-initialize the target after the erase if required
  if (swd_target_device.erase_reset)
  {
    status = es_swd_init();
  }

  return status;
}

/*******************************************************************************
* 函 数 名: es_swd_read_chipid
* 功能说明: 读芯片chipid
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_read_chipid(uint8_t *buf)
{
  if (!swd_read_memory(CHIP_INFO_FLASH_OFFSET + target_dev->chipid_addr, buf, 4))
  {
    return ERROR_SWD_READ;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
* 函 数 名: es_swd_chipid_check
* 功能说明: 判断chipid是否正确
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_chipid_check(void)
{
  uint32_t chipid = 0;
  uint32_t reg_data ;

  if (!swd_read_memory(CHIP_INFO_FLASH_OFFSET + target_dev->chipid_addr, (uint8_t *)&chipid, 4))
  {
    return ERROR_SWD_READ;
  }

  if (chipid != target_dev->chipid_value)
  {
    if (!swd_read_memory(CHIP_INFO_FLASH_OFFSET + CHIP_CFG_GBRDP_ADDR, (uint8_t *)&reg_data, 4))
    {
      return ERROR_SWD_READ;
    }

    if ((chipid == 0x00000000) && (reg_data == 0x00000000))
      return  ERROR_LV2_ENCRYPT;

    return  ERROR_CHIP_ID_NOT_MATCH;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
* 函 数 名: es_swd_read_checksum
* 功能说明: 读芯片校验和
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_read_checksum(uint8_t *buf)
{
  error_t ret = ERROR_SUCCESS;
  uint8_t checksum_h[4], checksum_l[4];

  ret = es_swd_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  if (!swd_read_memory(CHIP_INFO_FLASH_OFFSET + CHIP_CHECKSUM_ADDR, checksum_l, 4))
  {
    return ERROR_SWD_READ;
  }

  if (!swd_read_memory(CHIP_INFO_FLASH_OFFSET + CHIP_CHECKSUMN_ADDR, checksum_h, 4))
  {
    return ERROR_SWD_READ;
  }

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
* 函 数 名: es_swd_program_flash
* 功能说明: flash编程
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_program_flash(uint32_t addr, uint8_t *buf, uint32_t size, uint32_t *failed_addr)
{
  error_t ret = ERROR_SUCCESS ;

  ret = swd_program_flash(FLASH_AREA, addr, buf, size);

  if (ret !=  ERROR_SUCCESS)
  {
    if (failed_addr)
      *failed_addr = 0xFFFFFFFF;
  }

  return ret;
}

/*******************************************************************************
* 函 数 名: es_swd_read_flash
* 功能说明: 读flash
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_read_flash(uint32_t addr, uint8_t *buf, uint32_t size)
{
  if (!swd_read_memory(addr, buf, size))
  {
    return ERROR_SWD_READ;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
* 函 数 名: es_swd_verify_flash
* 功能说明: flash校验
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
  uint32_t i;
  uint8_t read_buf[SWD_PRG_SIZE];
  uint32_t verify_size;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  while (size > 0)
  {
    verify_size = MIN(size, sizeof(read_buf));

    if (!swd_read_memory(addr, read_buf, verify_size))
    {
      return ERROR_SWD_READ;
    }

    for (i = 0; i < verify_size; i++)
    {
      if (data[i] != read_buf[i])
      {
        if (failed_addr)
          *failed_addr = addr + ROUND_DOWN(i, 4)  ;

        if (failed_data)
        {
          *failed_data |= (read_buf[*failed_addr] << 0) ;
          *failed_data |= (read_buf[*failed_addr + 1] << 8) ;
          *failed_data |= (read_buf[*failed_addr + 2] << 16) ;
          *failed_data |= (read_buf[*failed_addr + 3] << 24) ;
        }

        return ERROR_SWD_VERIFY;
      }
    }

    addr += verify_size;
    size -= verify_size;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
* 函 数 名: es_swd_program_config
* 功能说明: 配置字编程
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_program_config(uint32_t addr, uint8_t *buf, uint32_t size, uint32_t *failed_addr)
{
  error_t ret = ERROR_SUCCESS ;
  uint32_t prog_addr;
  uint32_t prog_size;
  const struct info_part_map *part;
  uint32_t i;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  ret = es_swd_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  for (i = 0; i < ITEM_NUM(info_part_map); i++)
  {
    part = &info_part_map[i];

    prog_addr = CHIP_INFO_FLASH_OFFSET + part->addr;
    prog_size = part->size ;
    ret = swd_program_flash(INFO_AREA, prog_addr, buf, prog_size);

    if (ret != ERROR_SUCCESS)
    {
      if (failed_addr)
        *failed_addr = 0xFFFFFFFF ;

      return ERROR_SWD_PROG_CFG_WORD;
    }

    buf += prog_size;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
* 函 数 名: es_swd_read_config
* 功能说明: 读配置字
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_read_config(uint32_t addr,  uint8_t *buf, uint32_t size)
{
  error_t ret;
  uint32_t read_addr;
  uint32_t read_size;
  const struct info_part_map *part;
  uint32_t i;

  ret = es_swd_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  for (i = 0; i < ITEM_NUM(info_part_map); i++)
  {
    part = &info_part_map[i];
    read_addr = CHIP_INFO_FLASH_OFFSET + part->addr;
    read_size = part->size ;

    if (!swd_read_memory(read_addr, buf, read_size))
      return ERROR_SWD_READ;

    buf += read_size;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
* 函 数 名: es_swd_verify_config
* 功能说明: 配置字校验
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_verify_config(uint32_t addr,  uint8_t *buf, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
  uint32_t i;
  uint8_t read_buf[SWD_PRG_SIZE];
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
    read_addr = CHIP_INFO_FLASH_OFFSET + part->addr;
    read_size = part->size;

    while (read_size > 0)
    {
      verify_size = MIN(read_size, sizeof(read_buf));

      if (!swd_read_memory(read_addr, read_buf, verify_size))
      {
        return ERROR_SWD_READ;
      }

      for (i = 0; i < verify_size; i++)
      {
        if (*buf++ != read_buf[i])
        {
          if (failed_addr)
            *failed_addr = addr + ROUND_DOWN(i, 4)  ;

          if (failed_data)
          {
            *failed_data |= (read_buf[*failed_addr] << 0) ;
            *failed_data |= (read_buf[*failed_addr + 1] << 8) ;
            *failed_data |= (read_buf[*failed_addr + 2] << 16) ;
            *failed_data |= (read_buf[*failed_addr + 3] << 24) ;
          }

          return ERROR_SWD_VERIFY;
        }
      }

      read_addr += verify_size;
      read_size -= verify_size;
    }
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
* 函 数 名:  es_swd_encrypt_chip
* 功能说明:  加密。从配置字中读取加密信息，写入到加密位.
*         : M939 LEV2加密后无法swd读取数据
* 形    参:
* 返 回 值: 错误类型
*******************************************************************************/
static error_t es_swd_encrypt_chip(void)
{
  error_t ret = ERROR_SUCCESS;

  static uint8_t reg_data[8] = {0xff};

  ret = es_swd_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  ret = swd_prog_intf.cb(CFG_WORD, CHIP_CFG_GBRDP_OFFSET, reg_data, sizeof(reg_data) / sizeof(uint8_t));

  if (ret !=  ERROR_SUCCESS)
    return ret;

  ret = swd_program_flash(INFO_AREA, CHIP_INFO_FLASH_OFFSET + CHIP_CFG_GBRDP_ADDR, reg_data,  sizeof(reg_data) / sizeof(uint8_t));

  if (ret != ERROR_SUCCESS)
  {
    return ERROR_SWD_ENCRYPT;
  }

  //复位，加载加密字
  target_set_state(RESET_RUN);

  return ERROR_SUCCESS;
}


/*******************************************************************************
*  函 数 名: es_swd_check_empty
*  功能说明: 查空
*  形    参: failed_addr：错误地址 failed_data：错误数据
*  返 回 值: 错误类型
*******************************************************************************/
error_t es_swd_check_empty(uint32_t *failed_addr, uint32_t *failed_data)
{
  error_t status = ERROR_SUCCESS;
  uint32_t i;
  uint32_t code_addr;
  uint32_t code_size;
  uint32_t cfg_word_addr;
  uint32_t cfg_word_size;

  uint8_t read_buf[SWD_PRG_SIZE];
  uint32_t copy_size;
  const struct info_part_map *part;
  uint32_t item_num;

  status = es_swd_chipid_check();

  if (ERROR_SUCCESS != status)
    return status;

  //flash查空
  code_addr =  target_dev->code_start ;
  code_size =  target_dev->code_size ;

  while (true)
  {
    copy_size = MIN(code_size, sizeof(read_buf));

    if (!swd_read_memory(code_addr, read_buf, copy_size))
    {
      return ERROR_SWD_READ;
    }

    for (i = 0; i < copy_size; i++)
    {
      if (read_buf[i] != 0xFF)
      {
        if (failed_addr)
          *failed_addr = code_addr + ROUND_DOWN(i, 4)  ;

        if (failed_data)
        {
          *failed_data |= (read_buf[*failed_addr] << 0) ;
          *failed_data |= (read_buf[*failed_addr + 1] << 8) ;
          *failed_data |= (read_buf[*failed_addr + 2] << 16) ;
          *failed_data |= (read_buf[*failed_addr + 3] << 24) ;
        }

        return ERROR_SWD_FLASH_CHECK_EMPTY;
      }
    }

    // Update variables
    code_addr  += copy_size;
    code_size  -= copy_size;

    // Check for end
    if (code_size <= 0)
    {
      break;
    }
  }

  //配置字查空
  for (item_num = 0; item_num < ITEM_NUM(info_part_map); item_num++)
  {
    part = &info_part_map[item_num];
    cfg_word_addr =  CHIP_INFO_FLASH_OFFSET + part->addr;
    cfg_word_size =  part->size ;     //字节长度

    while (true)
    {
      copy_size = MIN(cfg_word_size, sizeof(read_buf));

      if (!swd_read_memory(cfg_word_addr, read_buf, copy_size))
      {
        return ERROR_SWD_READ;
      }

      for (i = 0; i < copy_size; i++)
      {
        if (read_buf[i] != 0xFF)
        {
          if (failed_addr)
            *failed_addr = code_addr + ROUND_DOWN(i, 4)  ;

          if (failed_data)
          {
            *failed_data |= (read_buf[*failed_addr] << 0) ;
            *failed_data |= (read_buf[*failed_addr + 1] << 8) ;
            *failed_data |= (read_buf[*failed_addr + 2] << 16) ;
            *failed_data |= (read_buf[*failed_addr + 3] << 24) ;
          }

          return ERROR_SWD_CFG_WORD_CHECK_EMPTY;
        }
      }

      // Update variables
      cfg_word_addr  += copy_size;
      cfg_word_size  -= copy_size;

      // Check for end
      if (code_size <= 0)
      {
        break;
      }
    }
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
*  函 数 名: swd_target_program_config_all
*  功能说明: 芯片配置字编程。
*  形    参: failed_addr：错误地址
*  返 回 值: 错误类型
*******************************************************************************/
static error_t swd_target_program_config_all(uint32_t *failed_addr)
{
  error_t ret = ERROR_SUCCESS;

  uint32_t cfg_word_addr;
  uint32_t cfg_word_size;

  uint32_t copy_size;
  uint32_t read_addr;
  uint8_t read_buf[SWD_PRG_SIZE] = {0x00};

  ret = es_swd_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  cfg_word_addr =  target_dev->config_word_start;
  cfg_word_size =  target_dev->config_word_size;
  read_addr =  0;

  while (true)
  {
    copy_size = MIN(cfg_word_size, sizeof(read_buf));

    ret = swd_prog_intf.cb(CFG_WORD, read_addr, read_buf, copy_size);

    if (ERROR_SUCCESS != ret)
      return ret;

    ret = es_swd_program_config(cfg_word_addr, read_buf, copy_size, failed_addr);

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
*  函 数 名: swd_target_program_all
*  功能说明: 芯片编程。flash和配置字编程
*  形    参: sn_enable：是否已编程序列号 sn：序列号代码
*             failed_addr：错误地址   failed_data ：错误数据
*  返 回 值: 编程错误地址
*******************************************************************************/
static error_t swd_target_program_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t i;

  uint32_t code_addr;
  uint32_t code_size;
  uint32_t cfg_word_addr;
  uint32_t cfg_word_size;

  uint32_t copy_size;
  uint32_t read_addr;
  uint8_t read_buf[SWD_PRG_SIZE];

  ret = es_swd_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  code_addr =  target_dev->code_start;
  code_size =  target_dev->code_size;
  read_addr =  0;

  while (true)
  {
    copy_size = MIN(code_size, sizeof(read_buf));
    ret = swd_prog_intf.cb(USER_HEX, read_addr, read_buf, copy_size);

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
      ret = es_swd_program_flash(code_addr, read_buf, copy_size, failed_addr);

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

  cfg_word_addr =  target_dev->config_word_start;
  cfg_word_size =  target_dev->config_word_size;
  read_addr =  0;

  while (true)
  {
    copy_size = MIN(cfg_word_size, sizeof(read_buf));
    ret = swd_prog_intf.cb(CFG_WORD, read_addr, read_buf, copy_size);

    if (ERROR_SUCCESS != ret)
      return ret;

    ret = es_swd_program_config(cfg_word_addr, read_buf, copy_size, failed_addr);

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
*  函 数 名: isp_target_verify_all
*  功能说明: 芯片验证
*  形    参: sn_enable：是否已编程序列号 sn：序列号代码
*             failed_addr：错误地址
*  返 回 值: 编程错误地址
*******************************************************************************/
static error_t  swd_target_verify_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr, uint32_t *failed_data)
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
  uint8_t sf_buf[SWD_PRG_SIZE];

  ret = es_swd_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  code_addr =  target_dev->code_start;
  code_size =  target_dev->code_size;
  sf_addr = 0;

  while (true)
  {
    verify_size = MIN(code_size, sizeof(sf_buf));

    ret = swd_prog_intf.cb(USER_HEX, sf_addr, sf_buf, verify_size);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和

    if (sn_enable == ENABLE)
      serial_number_intercept_write(sn, code_addr, sf_buf, verify_size);  //填入序列号

    ret = es_swd_verify_flash(code_addr, sf_buf, verify_size, failed_addr, failed_data);

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

  swd_prog_intf.cb(HEX_CHECKSUM, 0, (uint8_t *)&sf_checksum, 4);

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
    ret = swd_prog_intf.cb(CFG_WORD, sf_addr, sf_buf, verify_size);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和

    ret = es_swd_verify_config(cfg_word_addr, sf_buf, verify_size, failed_addr, failed_data);

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

  swd_prog_intf.cb(CFG_WORD_CHECKSUM, 0, (uint8_t *)&sf_checksum, 4);

  if (sf_checksum != (checksum & 0x0000ffff))
  {
    ret = ERROR_CFG_WORD_CHECKSUM;
    return  ret;
  }

  return  ret;
}



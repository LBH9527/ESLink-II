#include "eslink.h"
#include "errno.h"
#include "isp_prog_intf.h"
#include "sflash_port.h"
#include "program_port.h"
#include "./ES_ISP.h"
#include "./target_info.h"
//一次编程支持的长度，根据RAM大小可以修改
#define ISP_PRG_MINI_SIZE  1024

static void isp_init(es_target_cfg *target);
static error_t isp_prog_init(void); //进模式
static error_t isp_prog_uninit(void); //退出模式    
static error_t isp_prog_erase_chip(uint8_t para);
static error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t isp_prog_read_chipid(uint8_t *buf);
static error_t isp_prog_read_checksum(uint8_t *buf);
static error_t isp_prog_encrypt_chip(void);
static error_t isp_chipid_check(void);
static error_t isp_prog_program_config(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size);
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t isp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t isp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_target_program_config_all(uint32_t *failed_addr);
static error_t isp_target_program_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr);
static error_t isp_target_verify_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr, uint32_t *failed_data);
#if ESLINK_RTC_ENABLE
  static error_t isp_target_program_rtc(uint8_t mode) ;
#endif
struct  es_prog_ops isp_prog_intf =
{
  isp_init,
  isp_prog_init,
  isp_prog_uninit,
  isp_prog_erase_chip,
  isp_prog_check_empty,
  isp_prog_read_chipid,
  isp_chipid_check,
  isp_prog_read_checksum,
  isp_prog_encrypt_chip,

  isp_prog_program_config,
  isp_prog_read_config,
  isp_prog_verify_config,
  isp_prog_program_flash,
  isp_prog_read_flash,
  isp_prog_verify_flash,
  isp_target_program_config_all,
  isp_target_program_all,
  isp_target_verify_all,

#if ESLINK_RTC_ENABLE
  isp_target_program_rtc,
#else
  0,
#endif
};
static const es_target_cfg *isp_target_dev;

void isp_init(es_target_cfg *target)
{
  isp_target_dev = target;
  isp_prog_intf.cb = online_file_read;
//     isp_prog_intf.user_data =  0x00;
}

//进入isp模式
static error_t isp_entry_mode(void)
{
//    //复位
//    isp_reset();
  //读取ID
  if (isp_id_check() != TRUE)
    return ERROR_IN_ISP_MODE;

  //解锁
  if (isp_unlock_check() != TRUE)
    return ERROR_ISP_UNLOCK;

  //进ISP模式
  if (isp_mode_set() != TRUE)
    return ERROR_IN_ISP_MODE;

  //加载加密字
  if (isp_encrypt_check() != TRUE)
    return ERROR_IN_ISP_MODE;

  return ERROR_SUCCESS;
}
//编程初始化，进模式
static error_t isp_prog_init(void)
{
  error_t status;

  if (isp_mode_check() != TRUE)   //判断是否在isp模式
  {
    PORT_ISP_SETUP();
    eslink_set_target_hold_reset(40);
    status = isp_entry_mode();

    if (ERROR_SUCCESS != status)
      return status;
  }

  return  ERROR_SUCCESS;
}

//退出isp模式
static error_t isp_out_mode(void)
{
//    eslink_set_target_reset_run(40);
//    eslink_set_target_power_reset(40);
  return ERROR_SUCCESS;
}
//isp退出编程模式
static error_t isp_prog_uninit(void)
{
//    eslink_set_target_power_reset(40);
  return  ERROR_SUCCESS;
}

//
/*******************************************************************************
*  函 数 名: isp_prog_read_chipid
*  功能说明: 读目标芯片ID
*  形    参: ID值
*  返 回 值: 错误类型
*******************************************************************************/
static error_t isp_prog_read_chipid(uint8_t *buf)
{
  if (isp_read_config(isp_target_dev->chipid_addr, (uint32_t *)buf, 1) != TRUE)
    return ERROR_ISP_READ_CFG_WORD;

  return ERROR_SUCCESS;
}

/*******************************************************************************
 *  函 数 名: isp_chipid_check
 *  功能说明: 判断芯片chipid是否正确
 *  形    参:
 *  返 回 值: 错误类型
 *******************************************************************************/
static error_t isp_chipid_check(void)
{
  uint32_t chipid = 0;
  uint32_t reg_data ;

  if (isp_read_config(isp_target_dev->chipid_addr, &chipid, 1) != TRUE)
    return ERROR_ISP_READ_CFG_WORD;

  if (chipid != isp_target_dev->chipid_value)
  {
    if (isp_read_config(CHIP_CFG_GBRDP_ADDR, &reg_data, 1) != TRUE)
      return ERROR_ISP_READ_CFG_WORD;

    if ((chipid == 0x00000000) && (reg_data == 0x00000000))
      return  ERROR_LV2_ENCRYPT;

    return  ERROR_CHIP_ID_NOT_MATCH;
  }

  return ERROR_SUCCESS;

}
/*******************************************************************************
*  函 数 名:  isp_prog_read_checksum
*  功能说明:  读芯片校验和
*  形    参:
*  返 回 值:  错误类型
*******************************************************************************/
static error_t isp_prog_read_checksum(uint8_t *buf)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t checksum_h, checksum_l;

  ret = isp_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  if (isp_read_config(CHIP_CHECKSUM_ADDR, &checksum_l, 1) != TRUE)
    return ERROR_ISP_READ_CFG_WORD;

  if (isp_read_config(CHIP_CHECKSUMN_ADDR, &checksum_h, 1) != TRUE)
    return ERROR_ISP_READ_CFG_WORD;    
  //参考上位机校验和取数据方式来返回数据
  *buf++ =  checksum_l & 0xff ;  
  *buf++ = (checksum_l >> 16) & 0xff ;
  *buf++ =  checksum_h & 0xff ;
  *buf++ = (checksum_h >> 16) & 0xff ;
  return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名:  isp_prog_program_flash
*  功能说明:  flash编程
*  形    参:
*  返 回 值:  错误类型
*******************************************************************************/
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr)
{
  uint8_t ret ;
  uint32_t size_in_words;
  uint32_t offset;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  size_in_words = size / 4;

  ret = isp_program_code(addr, (uint32_t *)data, size_in_words, &offset);

  if (ret != TRUE)
  {
    if (failed_addr)
      *failed_addr = addr + offset * 4 ;

    return ERROR_ISP_PROG;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
*  函 数 名:  isp_prog_read_flash
*  功能说明:  读flash
*  形    参:  addr：地址 data：数据  size：长度
*  返 回 值:  错误类型
*******************************************************************************/
static error_t isp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t size_in_words;

  ret = isp_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  size_in_words = size / 4;

  if (isp_read_code(addr, (uint32_t *)data, size_in_words) != TRUE)
    return ERROR_ISP_READ;

  return ERROR_SUCCESS;

}
/*******************************************************************************
*  函 数 名: isp_prog_verify_flash
*  功能说明: flash校验
*  形    参: addr：地址 data：数据  size：长度 failed_addr：错误地址 failed_data：错误数据
*  返 回 值: 错误类型
*******************************************************************************/
static error_t isp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
  uint8_t ret ;
  uint32_t i;
  uint32_t rd_buf[ISP_PRG_MINI_SIZE / 4];
  uint32_t verify_size;
  uint32_t size_in_words;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  size_in_words = size / 4;

  while (size_in_words > 0)
  {
    verify_size = MIN(size_in_words, sizeof(rd_buf) / sizeof(rd_buf[0]));
    ret = isp_read_code(addr, rd_buf, verify_size);

    if (ret != TRUE)
      return ERROR_ISP_READ;

    for (i = 0; i < verify_size; i++)
    {
      if ((data[i * 4]   != ((rd_buf[i] >> 0) & 0xFF))  ||
          (data[i * 4 + 1] != ((rd_buf[i] >> 8) & 0xFF))  ||
          (data[i * 4 + 2] != ((rd_buf[i] >> 16) & 0xFF)) ||
          (data[i * 4 + 3] != ((rd_buf[i] >> 24) & 0xFF)))
      {
        if (failed_addr)
          *failed_addr = addr + i * 4 ;

        if (failed_data)
          *failed_data = rd_buf[i];

        return  ERROR_ISP_VERIFY;
      }
    }

    addr += verify_size * 4;
    size_in_words -= verify_size;
  }

  return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名: isp_prog_program_config
*  功能说明: 配置字编程
*  形    参: addr：地址 data：数据  size：长度 failed_addr：错误地址
*  返 回 值: 错误类型
*******************************************************************************/
static error_t isp_prog_program_config(uint32_t addr, uint8_t *buf, uint32_t size, uint32_t *failed_addr)
{
  uint8_t ret ;
  uint32_t offset;
  const struct info_part_map *part;
  uint32_t i;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  for (i = 0; i < ITEM_NUM(info_part_map); i++)
  {
    part = &info_part_map[i];

    ret = isp_program_config(part->addr, (uint32_t *)buf, part->size / 4, &offset);

    if (ret != TRUE)
    {
      if (failed_addr)
        *failed_addr = part->addr + offset * 4 ;

      return ERROR_ISP_PROG_CFG_WORD;
    }

    buf += part->size;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
*  函 数 名: isp_prog_read_config
*  功能说明: 读配置字,RTC功能使能时，需要读取RTC Info区的配置字
*  形    参: addr：地址 data：数据  size：长度
*  返 回 值: 错误类型
*******************************************************************************/
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size)
{
  error_t ret = ERROR_SUCCESS;
  const struct info_part_map *part;
  uint32_t i;

  ret = isp_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  if (size & 0x03)
    return ERROR_OUT_OF_BOUNDS;

  for (i = 0; i < ITEM_NUM(info_part_map); i++)
  {
    part = &info_part_map[i];

    if (isp_read_config(part->addr, (uint32_t *)buf, part->size / 4) != TRUE)
      return ERROR_ISP_READ_CFG_WORD;

    buf += part->size;
  }

#if ESLINK_RTC_ENABLE

  if (isp_read_config(CHIP_RTC_INFO_ADDR, (uint32_t *)buf, CHIP_RTC_INFO_SIZE / 4) != TRUE)
    return ERROR_ISP_READ_CFG_WORD;

#endif
  return ERROR_SUCCESS;
}

/*******************************************************************************
*  函 数 名: isp_prog_verify_config
*  功能说明: 配置字校验
*  形    参: addr：地址 data：数据  size：长度 failed_addr：错误地址 failed_data：错误数据
*  返 回 值: 错误类型
*******************************************************************************/
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *buf, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
  uint8_t ret ;
  uint32_t i;
  uint32_t rd_buf[ISP_PRG_MINI_SIZE / 4];
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
    read_size = part->size / 4;

    while (read_size > 0)
    {
      verify_size = MIN(read_size, sizeof(rd_buf) / sizeof(rd_buf[0]));
      ret = isp_read_config(read_addr, rd_buf, verify_size);

      if (ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD;

      for (i = 0; i < verify_size; i++)
      {
        if ((buf[i * 4] != ((rd_buf[i] >> 0) & 0xFF))  ||
            (buf[i * 4 + 1] != ((rd_buf[i] >> 8) & 0xFF))  ||
            (buf[i * 4 + 2] != ((rd_buf[i] >> 16) & 0xFF)) ||
            (buf[i * 4 + 3] != ((rd_buf[i] >> 24) & 0xFF)))
        {
          if (failed_addr)
            *failed_addr = read_addr + i * 4 ;

          if (failed_data)
            *failed_data = rd_buf[i];

          return  ERROR_ISP_CFG_WORD_VERIFY;
        }
      }

      read_addr += verify_size * 4;
      read_size -= verify_size;
    }

    buf += part->size;
  }

  return ERROR_SUCCESS;
}

/*******************************************************************************
*  函 数 名: isp_prog_encrypt_chip
*  功能说明: 芯片加密
*  形    参:
*  返 回 值: 错误类型
*******************************************************************************/
static error_t isp_prog_encrypt_chip(void)
{
  error_t ret = ERROR_SUCCESS;
  uint8_t result;
  static uint8_t reg_data[4] ;

  ret = isp_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  ret = isp_prog_intf.cb(CFG_WORD, CHIP_CFG_GBRDP_OFFSET, reg_data, sizeof(reg_data));

  if (ret !=  ERROR_SUCCESS)
    return ret;

  result = isp_program_config(CHIP_CFG_GBRDP_ADDR, (uint32_t *)reg_data, 1, NULL);

  if (result != TRUE)
    return ERROR_ISP_ENCRYPT;

  //加载加密字
  if (isp_encrypt_check() != TRUE)
    return ERROR_ISP_ENCRYPT;

  return ERROR_SUCCESS;
}

/*******************************************************************************
*  函 数 名: isp_prog_erase_chip
*  功能说明: 擦除
*  形    参: 擦除方式
*  返 回 值: 错误类型
*******************************************************************************/
static error_t isp_prog_erase_chip(uint8_t  para)
{
  error_t ret = ERROR_SUCCESS;

  ret = isp_chipid_check();
  //加密时无法读出chipid，
  if ((ERROR_SUCCESS != ret) && (ERROR_LV2_ENCRYPT != ret))
    return ret;

  if (isp_erase_chip() != TRUE)
    return ERROR_ISP_ERASE;

  return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名: isp_prog_check_empty
*  功能说明: 查空
*  形    参: failed_addr：错误地址  failed_data：错误数据
*  返 回 值: 错误类型
*******************************************************************************/
error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failed_data)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t i;
  uint32_t code_addr;
  uint32_t code_size;
  uint32_t cfg_word_addr;
  uint32_t cfg_word_size;
  uint32_t read_buf[ISP_PRG_MINI_SIZE / 4];
  uint32_t copy_size;
  const struct info_part_map *part;
  uint32_t item_num; 

  ret = isp_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  //flash查空
  code_addr =  isp_target_dev->code_start;
  code_size =  isp_target_dev->code_size / 4; //字长度

  while (true)
  {
    copy_size = MIN(code_size, sizeof(read_buf) / sizeof(read_buf[0]));
    isp_read_code(code_addr, read_buf, copy_size);

    for (i = 0; i < copy_size; i++)
    {
      if (read_buf[i] != 0xFFFFFFFF)
      {
        if (failed_addr)
          *failed_addr = code_addr + i * 4  ;

        if (failed_data)
          *failed_data = read_buf[i] ;

        return ERROR_ISP_FLASH_CHECK_EMPTY;
      }
    }

    // Update variables
    code_addr  += copy_size * 4;
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
    cfg_word_addr = part->addr;
    cfg_word_size = part->size / 4;   //字长度

    while (true)
    {
      copy_size = MIN(cfg_word_size, sizeof(read_buf) / sizeof(read_buf[0]));
      isp_read_config(cfg_word_addr, read_buf, copy_size);

      for (i = 0; i < copy_size; i++)
      {
        if (read_buf[i] != 0xFFFFFFFF)
        {
          if (failed_addr)
            *failed_addr = code_addr + i * 4  ;

          if (failed_data)
            *failed_data = read_buf[i] ;

          return ERROR_ISP_CFG_WORD_CHECK_EMPTY;
        }
      }

      // Update variables
      cfg_word_addr  += copy_size * 4;
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
*  函 数 名: isp_target_program_config_all
*  功能说明: 芯片配置字编程。
*  形    参: failed_addr：错误地址
*  返 回 值: 编程错误地址
*******************************************************************************/
static error_t isp_target_program_config_all(uint32_t *failed_addr)
{
  error_t ret = ERROR_SUCCESS;

  uint32_t cfg_word_addr;
  uint32_t cfg_word_size;

  uint32_t copy_size;
  uint32_t read_addr;
  uint8_t read_buf[ISP_PRG_MINI_SIZE] = {0x00};

  ret = isp_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  cfg_word_addr =  isp_target_dev->config_word_start;
  cfg_word_size =  isp_target_dev->config_word_size;
  read_addr =  0;

  while (true)
  {
    copy_size = MIN(cfg_word_size, sizeof(read_buf));

    ret = isp_prog_intf.cb(CFG_WORD, read_addr, read_buf, copy_size);

    if (ERROR_SUCCESS != ret)
      return ret;

    ret = isp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr);

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
*  函 数 名: isp_prog_program_flash
*  功能说明: 芯片编程。flash和配置字编程
*  形    参: sn_enable：是否已编程序列号 sn：序列号代码
*             failed_addr：错误地址   failed_data ：错误数据
*  返 回 值: 错误类型
*******************************************************************************/
static error_t isp_target_program_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr)
{
  error_t ret = ERROR_SUCCESS;
  uint32_t i;

  uint32_t code_addr;
  uint32_t code_size;
  uint32_t cfg_word_addr;
  uint32_t cfg_word_size;

  uint32_t copy_size;
  uint32_t read_addr;
  uint8_t read_buf[ISP_PRG_MINI_SIZE];

  ret = isp_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  code_addr =  isp_target_dev->code_start;
  code_size =  isp_target_dev->code_size;
  read_addr =  0;

  while (true)
  {
    copy_size = MIN(code_size, sizeof(read_buf));
    ret = isp_prog_intf.cb(USER_HEX, read_addr, read_buf, copy_size);

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
      ret = isp_prog_program_flash(code_addr, read_buf, copy_size, failed_addr);

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

  cfg_word_addr =  isp_target_dev->config_word_start;
  cfg_word_size =  isp_target_dev->config_word_size;
  read_addr =  0;

  while (true)
  {
    copy_size = MIN(cfg_word_size, sizeof(read_buf));
    ret = isp_prog_intf.cb(CFG_WORD, read_addr, read_buf, copy_size);

    if (ERROR_SUCCESS != ret)
      return ret;

    ret = isp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr);

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
*  返 回 值: 错误类型
*******************************************************************************/
static error_t  isp_target_verify_all(uint8_t sn_enable, serial_number_t *sn, uint32_t *failed_addr, uint32_t *failed_data)
{
  error_t ret = ERROR_SUCCESS;

  uint32_t checksum = 0;
  uint32_t sf_checksum = 0;   //spi保存的校验和

  uint32_t code_addr;
  uint32_t code_size;
  uint32_t cfg_word_addr;
  uint32_t cfg_word_size;
//    uint8_t read_buf[FLASH_PRG_MIN_SIZE];

  uint32_t verify_size;
  uint32_t sf_addr;
  uint8_t sf_buf[ISP_PRG_MINI_SIZE];

  ret = isp_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  code_addr =  isp_target_dev->code_start;
  code_size =  isp_target_dev->code_size;
  sf_addr = 0;

  while (true)
  {
    verify_size = MIN(code_size, sizeof(sf_buf));
    ret = isp_prog_intf.cb(USER_HEX, sf_addr, sf_buf, verify_size);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和

    if (sn_enable == ENABLE)
    {
      serial_number_intercept_write(sn, code_addr, sf_buf, verify_size);  //填入序列号
    }

    ret = isp_prog_verify_flash(code_addr, sf_buf, verify_size, failed_addr, failed_data);

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

  isp_prog_intf.cb(HEX_CHECKSUM, 0, (uint8_t *)&sf_checksum, 4);

  if ((sf_checksum & 0x0000ffff) != (checksum & 0x0000ffff))
  {
    ret = ERROR_USER_HEX_CHECKSUM;
    return  ret;
  }

  cfg_word_addr =  isp_target_dev->config_word_start;
  cfg_word_size =  isp_target_dev->config_word_size;
  sf_addr =  0;
  checksum = 0;

  while (true)
  {
    verify_size = MIN(cfg_word_size, sizeof(sf_buf));
    ret = isp_prog_intf.cb(CFG_WORD, sf_addr, sf_buf, verify_size);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和

    ret = isp_prog_verify_config(cfg_word_addr, sf_buf, verify_size, failed_addr, failed_data);

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

  isp_prog_intf.cb(CFG_WORD_CHECKSUM, 0, (uint8_t *)&sf_checksum, 4);

  if (sf_checksum != (checksum & 0x0000ffff))
  {
    ret = ERROR_CFG_WORD_CHECKSUM;
    return  ret;
  }

  return  ret;
}

/*******************************************************************************
*  函 数 名: isp_prog_program_flash
*  功能说明: 芯片编程。flash和配置字编程
*  形    参: mode：0x00 联机模式  0x01 脱机模式
*  返 回 值: 错误类型
*******************************************************************************/
#if ESLINK_RTC_ENABLE
static error_t isp_target_program_rtc(uint8_t para)
{
  error_t ret = ERROR_SUCCESS;
  uint8_t result;
  uint32_t code_addr;
  uint32_t code_size;
  uint32_t cfg_word_addr;
  uint32_t size;
  uint32_t read_addr;
  uint8_t buf[ISP_PRG_MINI_SIZE];
  uint32_t rd_buf[CHIP_RTC_INFO_SIZE / 4];
  uint32_t checksum = 0;
  uint32_t sf_checksum = 0;   //spi保存的校验和
  uint32_t i;

  ret = isp_chipid_check();

  if (ERROR_SUCCESS != ret)
    return ret;

  //flash编程
  code_addr =  isp_target_dev->code_start;
  code_size =  isp_target_dev->code_size;
  read_addr =  0;

  while (true)
  {
    size = MIN(code_size, sizeof(buf));
    ret = isp_prog_intf.cb(RTC_HEX, read_addr, buf, size);

    if (ERROR_SUCCESS != ret)
      return ret;

    for (i = 0; i < size; i++)
    {
      if (buf[i] != 0xFF)
        break;
    }

    if (i < size)     //数据段都为0xFF,不进行编程
    {
      ret = isp_prog_program_flash(code_addr, buf, size, NULL);

      if (ret !=  ERROR_SUCCESS)   //编程失败，返回编程失败地址
        return ret;
    }

    // Update variables
    code_addr  += size;
    code_size  -= size;
    read_addr += size;

    // Check for end
    if (code_size <= 0)
      break;
  }

  //flash编程校验
  code_addr =  isp_target_dev->code_start;
  code_size =  isp_target_dev->code_size;
  read_addr = 0;

  while (true)
  {
    size = MIN(code_size, sizeof(buf));

    ret = isp_prog_intf.cb(RTC_HEX, read_addr, buf, size);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    checksum += check_sum(size, buf);     //计算原始数据校验和
    ret = isp_prog_verify_flash(code_addr, buf, size, NULL, NULL);

    if (ret !=  ERROR_SUCCESS)
      return ret;

    // Update variables
    code_addr  += size;
    code_size  -= size;
    read_addr += size;

    // Check for end
    if (code_size <= 0)
      break;
  }

  isp_prog_intf.cb(RTC_HEX_CHECKSUM, 0, (uint8_t *)&sf_checksum, 4);

  if ((sf_checksum & 0x0000ffff) != (checksum & 0x0000ffff))
  {
    ret = ERROR_USER_HEX_CHECKSUM;
    return  ret;
  }

  //默认配置字编程
  cfg_word_addr =   CHIP_INFO_PART1_ADDR;  //info1的偏移地址

  buf[0] = (M939_RTC_CONFIG_DEFAULT_L >> 0) & 0xff;
  buf[1] = (M939_RTC_CONFIG_DEFAULT_L >> 8) & 0xff;
  buf[2] = (M939_RTC_CONFIG_DEFAULT_L >> 16) & 0xff;
  buf[3] = (M939_RTC_CONFIG_DEFAULT_L >> 24) & 0xff;
  buf[4] = (M939_RTC_CONFIG_DEFAULT_H >> 0) & 0xff;
  buf[5] = (M939_RTC_CONFIG_DEFAULT_H >> 8) & 0xff;
  buf[6] = (M939_RTC_CONFIG_DEFAULT_H >> 16) & 0xff;
  buf[7] = (M939_RTC_CONFIG_DEFAULT_H >> 24) & 0xff;

  if (isp_program_config(cfg_word_addr, (uint32_t *)buf, 0x08, NULL) != TRUE)
    return ERROR_ISP_PROG_CFG_WORD;

  //配置字校验
  if (isp_read_config(cfg_word_addr, (uint32_t *)buf, 0x08) != TRUE)
    return ERROR_ISP_READ_CFG_WORD;

  if ((buf[0] != ((M939_RTC_CONFIG_DEFAULT_L >> 0) & 0xFF)) |
      (buf[1] != ((M939_RTC_CONFIG_DEFAULT_L >> 8) & 0xFF)) |
      (buf[2] != ((M939_RTC_CONFIG_DEFAULT_L >> 16) & 0xFF)) |
      (buf[3] != ((M939_RTC_CONFIG_DEFAULT_L >> 24) & 0xFF)) |
      (buf[4] != ((M939_RTC_CONFIG_DEFAULT_H >> 0) & 0xFF)) |
      (buf[5] != ((M939_RTC_CONFIG_DEFAULT_H >> 8) & 0xFF)) |
      (buf[6] != ((M939_RTC_CONFIG_DEFAULT_H >> 16) & 0xFF)) |
      (buf[7] != ((M939_RTC_CONFIG_DEFAULT_H >> 24) & 0xFF)))
  {
    return  ERROR_ISP_CFG_WORD_VERIFY;
  }

  //rtc info编程
  read_addr = CHIP_RTC_INFO_OFFSET ;
  size =  isp_target_dev->config_word_size;
  ret = isp_prog_intf.cb(CFG_WORD, read_addr, buf, size);

  if (ERROR_SUCCESS != ret)
    return ret;

  rtc_info_erase();

  result = isp_program_config(CHIP_RTC_INFO_ADDR, (uint32_t *)buf, CHIP_RTC_INFO_SIZE / 4, NULL);

  if (result != TRUE)
  {
    return ERROR_ISP_PROG_CFG_WORD;
  }

  //info校验
  result = isp_read_config(CHIP_RTC_INFO_ADDR, rd_buf, CHIP_RTC_INFO_SIZE / 4);

  if (result != TRUE)
    return ERROR_ISP_READ_CFG_WORD;

  for (i = 0; i < CHIP_RTC_INFO_SIZE / 4; i++)
  {
    if ((buf[i * 4] != ((rd_buf[i] >> 0) & 0xFF))  ||
        (buf[i * 4 + 1] != ((rd_buf[i] >> 8) & 0xFF))  ||
        (buf[i * 4 + 2] != ((rd_buf[i] >> 16) & 0xFF)) ||
        (buf[i * 4 + 3] != ((rd_buf[i] >> 24) & 0xFF)))
    {
      return  ERROR_ISP_CFG_WORD_VERIFY;
    }
  }

  return ret;
}
#endif





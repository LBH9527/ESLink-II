#include "eslink.h"
#include "es_common.h"
#include "offline_file.h"
#include "settings_rom.h"
#include "sflash_port.h"
#include "offline_app.h"
#include "eeprom_port.h"
#include "menu.h"
#include "main.h"
#include "beep.h"
#include "isp_prog_intf.h"
#if ESLINK_SWD_ENABLE
  #include "swd_prog_intf.h"
#endif
#if ESLINK_BOOTISP_ENABLE
  #include "uartboot_prog_intf.h"
#endif

#if ESLINK_RTC_ENABLE
  #include "rtc_calibrate.h"
#endif
#include "target_config.h"
enum
{
  DETECT_TARGET_CHIP, //检测到芯片
  DETECT_NO_CHIP,     //未检测到芯片
} target_state_t;

typedef uint8_t (* program_start)(void);
program_start  ofl_prog_start;

static struct es_prog_ops *ofl_prog_intf;   //脱机编程接口
static es_target_cfg ofl_target_device;     //目标芯片信息
static ofl_prj_info_t ofl_prj_info;         //脱机方案信息
static ofl_serial_number_t sn_info;         //序列号信息
//更新序列号
static error_t  update_serial_number_8bit(uint64_t sn_data, uint8_t *buf, uint8_t size);
static error_t  update_serial_number_32bit(uint64_t sn_data, uint8_t *buf, uint8_t size);
static error_t  get_serial_number_8bit(uint64_t *sn_data, uint8_t *buf, uint8_t size);
static error_t  get_serial_number_32bit(uint64_t *sn_data, uint8_t *buf, uint8_t size);
static uint8_t ofl_machine_start(void);
static uint8_t ofl_auto_check_start(void);
/*******************************************************************************
*  函 数 名: ofl_prog_init
*  功能说明: 脱机编程初始化
*  形    参: mode: mini 还是全功能
*  返 回 值: 无
*******************************************************************************/
ofl_error_t ofl_prog_init(uint8_t mode)
{
  ofl_error_t ret = OFL_SUCCESS;
  prog_intf_type_t type;

  PORT_ISP_SETUP();
  ISP_SETUP();

  if (mode == OFFLINE_PROG_PLUS_MODE)
  {
    //获取方案信息
    online_file_read(OFL_PROG_INFO, 0, (uint8_t *) &ofl_prj_info, sizeof(ofl_prj_info_t));
    //获取脱机序列号
    get_offline_serial_number((uint8_t *) &sn_info, sizeof(ofl_serial_number_t));
    type = (prog_intf_type_t)ofl_prj_info.intf;
  }
  else if (mode == OFFLINE_PROG_MINI_MODE)
  {
    type = OFFLINE_PROG_MINI_DEFAULT_INTF;
  }

//烧录接口
  if (PRG_INTF_ISP ==  type)
    ofl_prog_intf = &isp_prog_intf;

#if ESLINK_SWD_ENABLE
  else if (PRG_INTF_SWD ==  type)
    ofl_prog_intf = &swd_prog_intf;

#endif
#if ESLINK_BOOTISP_ENABLE
  else if (PRG_INTF_BOOTISP == type)
    ofl_prog_intf = &uartboot_prog_intf;

#endif
  else
  {
    //todo 接口类型错误
    return OFL_ERR_PROG_INTF;
  }

  //获取目标芯片信息
  get_target_info((uint8_t *)&ofl_target_device);
  //烧录接口初始化
  ofl_prog_intf->init(&ofl_target_device);

  //启动烧写的方式
  if (mode == OFFLINE_PROG_PLUS_MODE)
  {
    if (OFL_PROG_START_MACHINE_MODE == sn_info.start_mode)   //机台启动
      ofl_prog_start = ofl_machine_start;
    //    else if(OFL_PROG_START_KEY_MODE == sn_info.start_mode）
    //        ;
    else
      ofl_prog_start = ofl_auto_check_start;       //自动检测
  }
  else if (mode == OFFLINE_PROG_MINI_MODE)
  {
    ofl_prog_start = ofl_auto_check_start;
  }

  return ret;
}

//脱机启动信号 ----外部机台触发
uint8_t ofl_machine_start(void)
{
  if (gpio_start_in_low())
  {
    es_delay_ms(5);

    if (gpio_start_in_low())
    {
      return DETECT_TARGET_CHIP;
    }
  }

  return DETECT_NO_CHIP;

}
//脱机启动信号 ----自动检测
uint8_t ofl_auto_check_start(void)
{
  uint8_t i, check_time = 0;

  for (i = 0; i < PROG_MODE_CHECK_TIME; i++)
  {
    if (ofl_prog_intf->prog_init() != ERROR_SUCCESS)    //未检测到芯片
    {
      check_time ++;
    }

    es_delay_ms(20);
  }

  if (check_time == 0)
    return DETECT_TARGET_CHIP;

  return DETECT_NO_CHIP;
}

/*******************************************************************************
*  函 数 名: ofl_out_prog_mode
*  功能说明: 出编程模式判断
*  形    参: 无
*  返 回 值: DETECT_NO_CHIP 未检测到芯片 DETECT_TARGET_CHIP 检测到芯片
*******************************************************************************/
uint8_t ofl_out_prog_mode(void)
{
  uint8_t i, check_time = 0;

  for (i = 0; i < PROG_MODE_CHECK_TIME; i++)
  {
    if (ofl_prog_intf->prog_init() == ERROR_SUCCESS)    //检测到芯片
    {
      check_time ++;
    }

    es_delay_ms(20);
  }

  if (check_time == 0)
    return DETECT_NO_CHIP;

  return DETECT_TARGET_CHIP;
}

/*******************************************************************************
*  函 数 名: ofl_prog
*  功能说明: 根据脱机步骤进行脱机编程
*  形    参:
*  返 回 值: 错误类型
*******************************************************************************/
ofl_error_t ofl_prog(void)
{
  error_t ret;
  uint32_t i = 0;

//    if( (sn_info.state !=  OFL_SERIALNUM_DISABLE) && (sn_info.success_count >  sn_info.total_size))
//        return OFL_ERR_COUNT_FULL;
  if (sn_info.success_count >=  sn_info.total_size)
    return OFL_ERR_COUNT_FULL;

  for (i = 0; i < ofl_prj_info.step; i++)
  {
    switch (ofl_prj_info.item[i])
    {
      case OFL_STEP_ERASE:
        ret = ofl_prog_intf->prog_init();

        if (ret != ERROR_SUCCESS)
          return OFL_ERR_ENTRY_MODE;

        ret = ofl_prog_intf->erase_chip(0);    

        if (ret != ERROR_SUCCESS)
          return  OFL_ERR_ERASE;

        break;

      case OFL_STEP_CHECK_EMPTY :
        ret = ofl_prog_intf->prog_init();

        if (ret != ERROR_SUCCESS)
          return OFL_ERR_ENTRY_MODE;

        ret = ofl_prog_intf->check_empty(NULL, NULL);

        if (ret != ERROR_SUCCESS)
          return  OFL_ERR_CHECK_EMPTY;

        break;

      case OFL_STEP_PROG:
        ret = ofl_prog_intf->prog_init();

        if (ret != ERROR_SUCCESS)
          return OFL_ERR_ENTRY_MODE;

        if (sn_info.state !=  OFL_SERIALNUM_DISABLE)
          ret = ofl_prog_intf->program_all(ENABLE, &sn_info.sn, NULL);
        else
          ret = ofl_prog_intf->program_all(DISABLE, NULL, NULL);

        if (ret != ERROR_SUCCESS)
          return  OFL_ERR_PROG;

        break;

      case OFL_STEP_VERIFY:
        ret = ofl_prog_intf->prog_init();

        if (ret != ERROR_SUCCESS)
          return OFL_ERR_ENTRY_MODE;

        if (sn_info.state !=  OFL_SERIALNUM_DISABLE)
          ret = ofl_prog_intf->verify_all(ENABLE, &sn_info.sn, NULL, NULL);
        else
          ret = ofl_prog_intf->verify_all(DISABLE, NULL, NULL, NULL);

        if (ret != ERROR_SUCCESS)
          return  OFL_ERR_VERIFY;

        break;

      case OFL_STEP_ENCRYPT :
        ret = ofl_prog_intf->prog_init();

        if (ret != ERROR_SUCCESS)
          return OFL_ERR_ENTRY_MODE;

        ret = ofl_prog_intf->encrypt_chip();

        if (ret != ERROR_SUCCESS)
          return  OFL_ERR_ENCRYPT;

        break;
#if ESLINK_RTC_ENABLE

      case OFL_STEP_RTC_CALI :
        ret = rtc_calibration_handler();

        if (ret != ERROR_SUCCESS)
        {
          return  OFL_ERR_ENCRYPT;
        }

        break;

      case OFL_STEP_RTC_VERIFY :
        ret = rtc_calibration_verify(NULL);

        if (ret != ERROR_SUCCESS)
        {
          return  OFL_ERR_ENCRYPT;
        }

        break;
#endif

      default:
        break;
    }
  }

  ret = ofl_prog_intf->chipid_check();

  if (ret != ERROR_SUCCESS)
  {
    return  OFL_ERR_CHIPID_CHECK;
  }

  //编程成功
  sn_info.success_count ++;       //烧录成功+1

  if (sn_info.state !=  OFL_SERIALNUM_DISABLE)
  {
    uint64_t data = 0;

    if (sn_info.read_mode  == OFL_SERIALNUM_READ_USE_IAP)   //32位机 IAP方式
    {
      get_serial_number_32bit(&data, sn_info.sn.data, sn_info.sn.size);
      data += sn_info.sn_step;
      update_serial_number_32bit(data, sn_info.sn.data, sn_info.sn.size);
    }
    else    //8位机返回指令
    {
      get_serial_number_8bit(&data, sn_info.sn.data, sn_info.sn.size);
      data += sn_info.sn_step;
      update_serial_number_8bit(data, sn_info.sn.data, sn_info.sn.size);
    }
  }

  //更新序列号
  set_offline_serial_number((uint8_t *) &sn_info, sizeof(ofl_serial_number_t));
  return OFL_SUCCESS;
}

void ofl_prog_handle(void)
{
  uint8_t menu_msg = MSG_NULL;
  static  ofl_prog_state_t state = IN_MODE_CHECK;
  ofl_error_t  prog_error ;

  switch (state)
  {
    case IN_MODE_CHECK :
      LED_GREEN_PASS_OFF();
      LED_YELLOW_BUSY_OFF();

      if (ofl_prog_start() == DETECT_TARGET_CHIP)
      {
        LED_RED_ERROR_OFF();

        state = OFL_PROG_ING;
        menu_msg = MSG_PROG_ING;
      }
      else
      {
        LED_RED_ERROR_TOGGLE();
      }

      break;

    case OFL_PROG_ING:
      LED_YELLOW_BUSY_ON();
      prog_error = ofl_prog() ;
      LED_YELLOW_BUSY_OFF();

      if (prog_error != OFL_SUCCESS)      //编程失败
      {
        LED_RED_ERROR_ON();
        beep_prog_fail();
      }
      else
      {
        LED_GREEN_PASS_ON();
        beep_prog_success();
      }

      switch (prog_error)
      {
        case OFL_SUCCESS:
          menu_msg = MSG_PROG_OK;
          break;

        case OFL_ERR_CHIPID_CHECK:       //ID检测失败
          menu_msg = MSG_ERR_CHIPID_CHECK;
          break;

        case OFL_ERR_ERASE:              //擦除失败
          menu_msg = MSG_ERR_ERASE;
          break;

        case OFL_ERR_CHECK_EMPTY:        //查空
          menu_msg = MSG_ERR_CHECK_EMPTY;
          break;

        case OFL_ERR_PROG:               //编程失败
          menu_msg = MSG_ERR_PROG;
          break;

        case OFL_ERR_VERIFY:             //校验
          menu_msg = MSG_ERR_VERIFY;
          break;

        case OFL_ERR_ENCRYPT:            //加密
          menu_msg = MSG_ERR_ENCRYPT;
          break;

        case OFL_ERR_COUNT_FULL:         //烧录计数溢出'
          menu_msg = MSG_ERR_COUNT_FULL;
          break;

//                  case OFL_ERR_PROG_INTF:          //编程接口设置失败
//                        menu_msg = MSG_PROG_OK;
//                        break;
        case OFL_ERR_ENTRY_MODE:
          menu_msg = MSG_ERR_ENTRY_MODE;
          break;
#if ESLINK_RTC_ENABLE

        case OFL_ERR_RTC_CALI:
          menu_msg = MSG_ERR_RTC_CALI;
          break;

        case OFL_ERR_RTC_VERIFY:
          menu_msg = MSG_ERR_RTC_VERIFY;
          break;
#endif

        default:
          menu_msg = MSG_ERR;
          break;
      }

      state = OUT_MODE_CHECK;
      break;

    case OUT_MODE_CHECK:
      if (ofl_out_prog_mode() == DETECT_NO_CHIP)
      {
        state = IN_MODE_CHECK;
        menu_msg = MSG_PROG_MODE_CHECK;
      }

      break;

    default:
      break;
  }

  gui_msg_write_data(&menu_msg) ;
  gui_refresh();
}

void mini_ofl_prog_handle(void)
{
  static  ofl_prog_state_t state = IN_MODE_CHECK;
  ofl_error_t  prog_error ;

  switch (state)
  {
    case IN_MODE_CHECK :

      LED_GREEN_PASS_OFF();
      LED_YELLOW_BUSY_OFF();

      if (ofl_prog_start() == DETECT_TARGET_CHIP)
      {
        LED_RED_ERROR_OFF();
        state = OFL_PROG_ING;
      }
      else
      {
        LED_RED_ERROR_TOGGLE();
      }

      break;

    case OFL_PROG_ING:
      LED_YELLOW_BUSY_ON();
      prog_error = ofl_mini_prog() ;
      LED_YELLOW_BUSY_OFF();

      if (prog_error != OFL_SUCCESS)      //编程失败
      {
        LED_RED_ERROR_ON();
      }
      else
      {
        LED_GREEN_PASS_ON();
      }

      state = OUT_MODE_CHECK;
      break;

    case OUT_MODE_CHECK:
      if (ofl_out_prog_mode() == TRUE)
      {
        state = IN_MODE_CHECK;
      }

      break;

    default:
      break;
  }
}
/*******************************************************************************
*  函 数 名: ofl_mini_prog
*  功能说明: mini脱机编程，默认为：擦除、编程、校验、加密
*  形    参:
*  返 回 值: 错误类型
*******************************************************************************/
ofl_error_t ofl_mini_prog(void)
{
  error_t ret;
  uint8_t step = OFL_STEP_ERASE;

  if (OFL_STEP_ERASE == step)
  {
    ret = ofl_prog_intf->erase_chip(0);    //全擦

    if (ret != ERROR_SUCCESS)
      return  OFL_ERR_ERASE;

    step = OFL_STEP_PROG;
  }

  if (OFL_STEP_PROG == step)
  {
    ret = ofl_prog_intf->program_all(DISABLE, NULL, NULL);

    if (ret != ERROR_SUCCESS)
      return  OFL_ERR_PROG;

    step = OFL_STEP_VERIFY;
  }

  if (OFL_STEP_VERIFY == step)
  {
    ret = ofl_prog_intf->verify_all(DISABLE, NULL, NULL, NULL);

    if (ret != ERROR_SUCCESS)
      return  OFL_ERR_VERIFY;

    step = OFL_STEP_ENCRYPT;
  }

  if (OFL_STEP_ENCRYPT == step)
  {
    ret = ofl_prog_intf->encrypt_chip();

    if (ret != ERROR_SUCCESS)
      return  OFL_ERR_ENCRYPT;

    step = OFL_STEP_ERASE;
  }

  return  OFL_SUCCESS;
}


//获取8位芯片的序列号
static error_t  get_serial_number_8bit(uint64_t *sn_data, uint8_t *buf, uint8_t size)
{
  uint8_t i;

  if (size & 0x01)
    return  ERROR_OUT_OF_BOUNDS;

  size = size / 2;

  if ((size <= 0) || (size > 8))
    return  ERROR_OUT_OF_BOUNDS;

  for (i = 0; i < size; i++)
  {
    *sn_data |= ((uint64_t)buf[i * 2] & 0xff) << (8 * (size - 1 - i));
  }

  return  ERROR_SUCCESS;
}
//更新序列号
static error_t  update_serial_number_8bit(uint64_t sn_data, uint8_t *buf, uint8_t size)
{
  uint8_t i;

  if (size & 0x01)
    return  ERROR_OUT_OF_BOUNDS;

  size = size / 2;

  if ((size <= 0) || (size > 8))
    return  ERROR_OUT_OF_BOUNDS;

  for (i = 0; i < size; i++)
  {
    buf[i * 2] = (sn_data >> (8 * (size - 1 - i))) & 0XFF;
  }

  return  ERROR_SUCCESS;
}
//获取32位芯片的序列号
static error_t  get_serial_number_32bit(uint64_t *sn_data, uint8_t *buf, uint8_t size)
{
  uint8_t i;

  if ((size <= 0) || (size > 8))
    return  ERROR_OUT_OF_BOUNDS;

  if (size == 0x04)
  {
    for (i = 0; i < 4; i++)
      *sn_data |= ((uint64_t)buf[i] & 0xff) << (8 * i);
  }
  else if (size == 0x08)
  {
    for (i = 0; i < 4; i++)
      *sn_data |= ((uint64_t)buf[i] & 0xff) << (8 * (i + 4));

    for (i = 4; i < 8; i++)
      *sn_data |= ((uint64_t)buf[i] & 0xff) << (8 * (i - 4));
  }

  return  ERROR_SUCCESS;
}
//更新32位芯片序列号
static error_t  update_serial_number_32bit(uint64_t sn_data, uint8_t *buf, uint8_t size)
{
  uint8_t i;

  if ((size <= 0) || (size > 8))
    return  ERROR_OUT_OF_BOUNDS;

  if (size == 0x04)
  {
    for (i = 0; i < 4; i++)
      buf[i] = (sn_data >> (8 * i)) & 0XFF;
  }
  else if (size == 0x08)
  {
    for (i = 0; i < 4; i++)
      buf[i] = (sn_data >> (8 * (i + 4))) & 0XFF;

    for (i = 4; i < 8; i++)
      buf[i] = (sn_data >> (8 * (i - 4))) & 0XFF;
  }

  return  ERROR_SUCCESS;
}


//退出脱机模式时，更新脱机序列号信息
ofl_error_t update_ofl_serial_number(void)
{
//    error_t ret = ERROR_SUCCESS;
  char path[16 + 1] = {'\0'};
  partition_t part;

//    if(sn_info.state ==  OFL_SERIALNUM_DISABLE)
//        return  OFL_SUCCESS;

  //读文件名
  get_offline_project_name((uint8_t *)path, 16);
  //读脱机序列号分区信息
  get_offline_partition((uint8_t *)&part, sizeof(partition_t));
  //读脱机序列号
  get_offline_serial_number((uint8_t *) &sn_info, sizeof(ofl_serial_number_t));
  ofl_file_lseek_write(path, part.start, (uint8_t *) &sn_info, part.size);

  return OFL_SUCCESS ;

}



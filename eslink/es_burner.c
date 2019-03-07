#include "eslink.h"
#include "errno.h"
#include "es_burner.h"
#include "main.h"
#include "settings_rom.h"
#include "update.h"
#include "sflash_port.h"
#include "eeprom_port.h"
#include "offline_file.h"
#include "isp_prog_intf.h"  
#include "target_config.h"  
#if ESLINK_SWD_ENABLE
#include "swd_prog_intf.h"
#endif
#if ESLINK_BOOTISP_ENABLE
#include "uartboot_prog_intf.h"
#endif

#if ESLINK_RTC_ENABLE
#include "rtc_calibrate.h"  
#endif
 


static es_target_cfg es_target_device;      //目标芯片信息
static struct es_prog_ops *online_prog_intf;
typedef enum {
    STATE_DISABLE,	//失能
	STATE_DL,		//序列号下载完成
    STATE_WRITE,	//序列号写入完成  
} sn_state_t;
//static sn_state_t sn_state = STATE_DISABLE;
static serial_number_t prog_sn ;       //序列号
//编程步长
#define FLASH_PRG_MIN_SIZE      1024

//字转换为字节
//data:字数组 len：字数组长度 res：字节数组
static void int2array(uint32_t *data, uint8_t len, uint8_t *res ) {
    uint8_t i = 0;

    for (i = 0; i < len; i++) {
        *res++    = *data  & 0xff;
        *res++ = (*data >> 8 ) & 0xff;
        *res++ = (*data >> 16) & 0xff;
        *res++ = (*data >> 24) & 0xff;          
        data++;
    }
} 

/*******************************************************************************
*	函 数 名: es_prog_set_intf
*	功能说明: 编程接口设置
*	形    参: type 编程接口类型
*	返 回 值: error
*******************************************************************************/
error_t es_prog_set_intf(prog_intf_type_t type)
{
    if(PRG_INTF_ISP == type )
    {
         online_prog_intf =  &isp_prog_intf; 
         online_prog_intf->init(&es_target_device);   
    }      
#if ESLINK_SWD_ENABLE
    else if (PRG_INTF_SWD ==  type )
    {
         online_prog_intf =  &swd_prog_intf; 
         online_prog_intf->init(&es_target_device);
    }
#endif
#if ESLINK_BOOTISP_ENABLE
    else if ( PRG_INTF_BOOTISP == type)
    {
         online_prog_intf =  &uartboot_prog_intf; 
         online_prog_intf->init(&es_target_device);
    } 
#endif
    else
    {
        return ERROR_PROG_INTF;
    }
    
    return ERROR_SUCCESS;

}
static error_t es_prog_get_intf(uint8_t *data)
{
    prog_intf_type_t type;
    
    if(online_prog_intf ==  &isp_prog_intf)
    {
        type = PRG_INTF_ISP;
    }
#if ESLINK_SWD_ENABLE
    else if(online_prog_intf ==  &swd_prog_intf)
    {
        type = PRG_INTF_SWD;
    }
#endif
#if ESLINK_BOOTISP_ENABLE
    else if(online_prog_intf ==  &isp_prog_intf)
    {
        type = PRG_INTF_BOOTISP;      
    }
#endif
    else
    {
          return ERROR_PROG_INTF;
    }
    data[0] = ((uint32_t)type>>0) &0xFF  ;
    data[1] = ((uint32_t)type>>8) &0xFF  ;
    data[2] = ((uint32_t)type>>16) &0xFF  ;
    data[3] = ((uint32_t)type>>24) &0xFF  ;
    return ERROR_SUCCESS;
}

/*
 *  序列号下载
 */
static error_t serial_number_download(uint8_t *data)
{
	uint8_t i;
	uint8_t len ;
    len = sizeof(serial_number_t) ;
    
    //TODO:判断序列号合法性。
	for(i = 0; i< len; i++)
    {
		 *((uint8_t*)&prog_sn + i) =  *(data++);
	}
//	prog_sn.addr = prog_sn.addr * 4;	//上位机下发的地址是除以4后的地址。还原为实际地址
	return ERROR_SUCCESS;
}


/*
 *  读固件版本
 *  固件版本不对应，则更新固件
 */
static error_t read_offline_version(uint8_t *buf)
{   
    error_t result = ERROR_SUCCESS;     
    uint32_t version;
//    if(get_offline_info(buf) != sizeof(offline_info_t) )
//        result = ERROR_IAP_READ;
    version = get_offlink_app_version();
    int2array(&version, 1, buf);
    //
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0; 
    return result;  
}
/*
 *  读芯片时序信息
 */
static error_t read_timinginfo(uint8_t *buf)
{
    error_t result = ERROR_SUCCESS;
    uint8_t len ;
    len = sizeof(timing_info_t) + sizeof(es_target_cfg) ;
    
    if(get_timing_info(buf) != len )
        result = ERROR_IAP_READ;
//    if(get_target_info(buf) != sizeof(es_target_cfg) )
//        result = ERROR_IAP_READ_TIMING_INFO;    
    return result;  
}
/*
 *  读ESlink II序列号
 */
static error_t read_eslink_sn(uint8_t *buf)
{    
    error_t result = ERROR_SUCCESS;
    uint32_t eslink_sn;     //eslink 产品序列号
    eslink_sn = get_eslinkii_serial_number();
    int2array(&eslink_sn, 1, buf);
    return result ;
}
/*
 *  写ESlink II序列号
 */
static error_t write_eslink_sn(uint8_t *buf)
{     
    if(set_eslinkii_serial_number(buf) != TRUE)
        return ERROR_IAP_WRITE;
    return ERROR_SUCCESS ;
}

/********************************读写联机数据到编程器******************************/ 
/*
 *  配置字下载
 */
static error_t es_download_config_word(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    static uint32_t addr;
    uint32_t size;
    
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);
    if( (*(data+0) == 0x00) && (*(data+1) == 0x00) && (*(data+2) == 0x00) && (*(data+3) == 0x00))
    {
         ret = online_file_erase(CFG_WORD, es_target_device.config_word_size );
         if(ERROR_SUCCESS != ret)
            return ret;  
         addr = 0x00;
    }         
    ret = online_file_write(CFG_WORD, addr, (data+8), size) ;    
    addr += size;
    return ret;            
}
/*
 *  配置字下载完成
 */
static error_t es_download_config_word_end(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t checksum = 0;  
    uint32_t cfg_word_szie = es_target_device.config_word_size;
    uint32_t read_addr = 0;
    uint32_t read_size = 0;
    uint8_t read_buf[FLASH_PRG_MIN_SIZE] = {0};        
    
    while(cfg_word_szie > 0){      
        read_size = MIN(cfg_word_szie, sizeof(read_buf));
        ret = online_file_read(CFG_WORD, read_addr,  read_buf, read_size);
        if(ERROR_SUCCESS != ret)
            return ret; 
        checksum += check_sum(read_size, read_buf);
        
        read_addr += read_size;
        cfg_word_szie -= read_size;
    }
    if( (data[0] == ((checksum>>0)&0xFF)) && 
        (data[1] == ((checksum>>8)&0xFF)) &&
        (data[2] == ((checksum>>16)&0xFF)) &&
        (data[3] == ((checksum>>24)&0xFF)) )
    {
        ret = ERROR_SUCCESS;
        ret = online_file_erase(CFG_WORD_CHECKSUM, 4 );
        online_file_write(CFG_WORD_CHECKSUM, 0, data, 4);      //验证正确后在保存数据  
    }
    else
    {
         ret = ERROR_CFG_WORD_CHECKSUM;
    }          
    //TODO:
    //实现CRC校验
    return ret;   
}
volatile static uint32_t hex_type = USER_HEX ;
volatile static uint32_t checksum_type = HEX_CHECKSUM ;
/*
 * 用户程序下载开始
 * 下载开始时，擦除目标芯片code size长度的spi flash。
 */
static error_t es_download_hex_start( uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t type;

    type = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    if(type != 0)       //RTC时序
    {
         hex_type = RTC_HEX;
         checksum_type = RTC_HEX_CHECKSUM;  
    }
    else
    {
        hex_type = USER_HEX;
        checksum_type = HEX_CHECKSUM;    
    }  
    ret = online_file_erase(hex_type, es_target_device.code_size );

    return ret;  
}

/*
 * 用户程序下载
 */
static error_t es_download_hex(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
    uint32_t i;
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);  
            
    i = 0;
    while(i<size)
    {
        if(*(data+8+i) != 0xFF)
            break;
        i++;
    }
    if(i >= size)   //全为0xff 不需要写入
        ret =  ERROR_SUCCESS;
    else
        ret = online_file_write(hex_type, addr, (data+8), size) ;         
        
    if(ERROR_SUCCESS != ret)
            return ret;      
    return ret;        
}
/*
 *  用户程序下载结束
 *  保存上位机下发的校验和到spi flash中。读取spi-flash中的用户程序,计算累加和和CRC校验，
 *  与上位机下发的校验做比较'
 * 下发的数据为“用户程序校验和（4bytes）+用户程序CRC32（4bytes）”
 * 校验和的地位为“和校验”
 */
static error_t es_download_hex_end(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t checksum = 0;  
//    uint32_t sf_checksum = 0;
    uint32_t code_size = es_target_device.code_size;
    uint32_t read_addr = 0;
    uint32_t read_size = 0;
    uint8_t read_buf[FLASH_PRG_MIN_SIZE] = {0};         

    while(code_size > 0){      
        read_size = MIN(code_size, sizeof(read_buf));
        ret = online_file_read(hex_type, read_addr,  read_buf, read_size) ;
        if(ERROR_SUCCESS != ret)
            return ret;
        checksum += check_sum(read_size, read_buf);
        
        read_addr += read_size;
        code_size -= read_size;
    }
    if( (data[0] == ((checksum>>0)&0xFF)) && 
        (data[1] == ((checksum>>8)&0xFF)) &&
        (data[2] == ((checksum>>16)&0xFF)) &&
        (data[3] == ((checksum>>24)&0xFF)) )
    {
        ret = ERROR_SUCCESS;
        ret = online_file_erase(checksum_type, 4 );
        ret = online_file_write(checksum_type, 0, data, 4);      //验证正确后在保存数据             
    }
    else
    {
         ret = ERROR_USER_HEX_CHECKSUM;
    }          
    //TODO:
    //实现CRC校验
    return ret;        
}

/********************************联机操作***************************************/   
//设置烧录接口
static error_t es_download_prg_intf(const uint8_t *buf)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t prg_temp = 0;
    prg_temp = (*(buf+0)) | (*(buf+1) << 8) | (*(buf+2) << 16) | (*(buf+3) << 24);

    ret = es_prog_set_intf((prog_intf_type_t)prg_temp) ;
    return ret;
} 
/*******************************************************************************
*	函 数 名: es_erase_chip
*	功能说明: 芯片擦除
*	形    参: 擦除模式
*	返 回 值: 无
*******************************************************************************/
error_t es_erase_chip(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t erase_mode;    
    
    ret = online_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        goto fail;
    erase_mode = data[0];    
    ret = online_prog_intf->erase_chip(erase_mode);
    if(ERROR_SUCCESS != ret)
    {
        goto fail;
    
    }  
    ret = online_prog_intf->prog_uninit();
    return  ERROR_SUCCESS;
fail:
    online_prog_intf->prog_uninit();     
    return ret;
}
/*******************************************************************************
*	函 数 名: es_check_empty
*	功能说明: 芯片查空。 查空flash 和配置字
*	形    参: 查空失败的地址和数据
*	返 回 值: 无
*******************************************************************************/
error_t es_check_empty(uint8_t *data)
{
    uint32_t failed_addr;
    uint32_t failed_data;
    error_t ret = ERROR_SUCCESS;
    
    ret = online_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        goto fail;     
    ret = online_prog_intf->check_empty(&failed_addr, &failed_data);
    if(ERROR_SUCCESS != ret)
    {
        int2array(&failed_addr, 1, data);
        int2array(&failed_data, 1, data+4);
        goto fail;
    }  
    
    online_prog_intf->prog_uninit();
    return  ERROR_SUCCESS;
fail:
    online_prog_intf->prog_uninit();     
    return ret;
}  
/*******************************************************************************
*	函 数 名: es_program_flash
*	功能说明: 芯片编程。flash和配置字编程
*	形    参: 编程错误地址 
*	返 回 值: 无
*******************************************************************************/
error_t es_program_flash(uint8_t sn_enable, uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t failed_addr = 0x0;      
    
    ret = online_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        goto fail;
        
    ret = online_prog_intf->program_all(sn_enable, &prog_sn, &failed_addr);
    if(ERROR_SUCCESS != ret)
    {
        int2array(&failed_addr, 1, data);
        goto fail;
    }
        
    online_prog_intf->prog_uninit();
    return  ERROR_SUCCESS;
fail:
    online_prog_intf->prog_uninit();     
    return ret;
    
}
/*******************************************************************************
*	函 数 名: es_program_verify
*	功能说明: 芯片校验 ，读出芯片中的flash和配置字并计算累加和，与spi flash保存的累加和作比较
*	形    参: 校验错误地址 和数据
*	返 回 值: 无
*******************************************************************************/
error_t es_program_verify(uint8_t sn_enable,uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;

    uint32_t failed_addr;
    uint32_t failed_data;     
    
    ret = online_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        goto fail;
   
    ret = online_prog_intf->verify_all(sn_enable, &prog_sn, &failed_addr , &failed_data);
    if(ERROR_SUCCESS != ret)
    {
        int2array(&failed_addr, 1, data);
        int2array(&failed_data, 1, data+4);
        goto fail;     
    }
    
    online_prog_intf->prog_uninit();
    return  ERROR_SUCCESS;
fail:
    online_prog_intf->prog_uninit();     
    return ret;
}
/*******************************************************************************
*	函 数 名: es_program_encrypt
*	功能说明: 加密
*	形    参: NONE
*	返 回 值: error_t
*******************************************************************************/
error_t es_program_encrypt(void)
{
    error_t ret = ERROR_SUCCESS;
    
    ret = online_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        goto fail;
   
    ret = online_prog_intf->encrypt_chip(); 
    if(ERROR_SUCCESS != ret)
        goto fail;
    
    online_prog_intf->prog_uninit();     
    return ERROR_SUCCESS;
fail:
    online_prog_intf->prog_uninit();     
    return ret;
}
/*******************************************************************************
*	函 数 名: es_read_flash
*	功能说明: 读目标芯片flash数据
*	形    参: 读出的数据
*	返 回 值: error_t
*******************************************************************************/
static error_t es_read_flash(uint8_t *wrbuf, uint8_t *rdbuf, uint16_t *read_size)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
    
    addr = (*(wrbuf+0)  <<  0) |
            (*(wrbuf+1) <<  8) |
            (*(wrbuf+2) << 16) |
            (*(wrbuf+3) << 24);
    size = (*(wrbuf+4)  <<  0) |
            (*(wrbuf+5) <<  8) |
            (*(wrbuf+6) << 16) |
            (*(wrbuf+7) << 24); 
    if(addr == es_target_device.code_start)
    {
        ret = online_prog_intf->prog_init();
        if(ERROR_SUCCESS != ret)
            goto fail;        
    }     
        
    memcpy( rdbuf, wrbuf, 4);
    ret = online_prog_intf->read_flash(addr, rdbuf+4, size); 
    if(ERROR_SUCCESS != ret)
    {
        *read_size = 0 ;
        goto fail;
    }
    else
    {
        *read_size = size+4;             
    }  
    
    if(addr+size == es_target_device.code_size)
    {
        online_prog_intf->prog_uninit(); 
    }        
    return ERROR_SUCCESS;
    
fail:
    online_prog_intf->prog_uninit();     
    return ret;

}
/*******************************************************************************
*	函 数 名: es_program_config_word
*	功能说明: 配置字编程
*	形    参: data: 编程错误地址 
*	返 回 值: error_t
*******************************************************************************/
static error_t es_program_config_word(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t failed_addr;   
    
    ret = online_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        goto fail; 
        
    ret = online_prog_intf->program_info_all( &failed_addr);
    if(ERROR_SUCCESS != ret)
    {
        int2array(&failed_addr, 1, data);
        goto fail;
    }    
    
    online_prog_intf->prog_uninit();
    return  ERROR_SUCCESS;
fail:
    online_prog_intf->prog_uninit();     
    return ret; 
}
//读配置字
static error_t es_read_config_word(uint8_t *wrbuf, uint8_t *rdbuf, uint16_t *read_size)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
    
    addr = (*(wrbuf+0)  <<  0) |
            (*(wrbuf+1) <<  8) |
            (*(wrbuf+2) << 16) |
            (*(wrbuf+3) << 24);
    size = (*(wrbuf+4)  <<  0) |
            (*(wrbuf+5) <<  8) |
            (*(wrbuf+6) << 16) |
            (*(wrbuf+7) << 24); 
            
    ret = online_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        goto fail;   
       
    memcpy( rdbuf, wrbuf, 4);
    ret = online_prog_intf->read_info(addr, rdbuf+4, size); 
    if(ERROR_SUCCESS != ret)
    {
        *read_size = 0 ;
        goto fail; 
    }
    else
    {               
        *read_size = size+4;          
    }  
    
    online_prog_intf->prog_uninit();
    return  ERROR_SUCCESS;
fail:
    online_prog_intf->prog_uninit();     
    return ret;  

}

//读chipid
static error_t es_read_chipid(uint8_t *data)
{
//    uint32_t chipid;
    error_t ret = ERROR_SUCCESS;
    
    ret = online_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        goto fail;   
        
    ret = online_prog_intf->read_chipid(data);
    if(ERROR_SUCCESS != ret)
        goto fail;          
//    data[0] = (chipid & 0x000000FF) ;  
//    data[1] = (chipid & 0x0000FF00) >> 8; 
//    data[2] = (chipid & 0x00FF0000) >> 16;
//    data[3] = (chipid & 0xFF000000) >> 24;  
    
    online_prog_intf->prog_uninit();
    return  ERROR_SUCCESS;
fail:
    online_prog_intf->prog_uninit();     
    return ret;  
}   
//读芯片校验和
static error_t es_read_chip_chksum(uint8_t *data)  
{
    error_t ret = ERROR_SUCCESS;
    
    ret = online_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        goto fail; 
   
    ret = online_prog_intf->read_chip_chksum(data);
    if(ERROR_SUCCESS != ret)
        goto fail;  
    
    online_prog_intf->prog_uninit();
    return  ERROR_SUCCESS;
fail:
    online_prog_intf->prog_uninit();     
    return ret;     
    
}
/********************************脱机文件操作***************************************/
static partition_t  file_part;
static uint32_t ofl_target_cfg_word_size  ;     
static uint32_t ofl_target_image_size ;
//读方案数量
static error_t read_ofl_prj_num(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    ret = get_ofl_file_num(data);
    return ret; 
} 

/*
 *  读脱机方案信息
 *  num: 方案序号  data：返回信息数据
 *  
 */
static error_t read_ofl_prj_info(uint8_t *num, uint8_t *data)
{   
    error_t ret = ERROR_SUCCESS;
    uint32_t prj_num;
    
    prj_num = (*(num+0) <<  0) |
            (*(num+1) <<  8) |
            (*(num+2) << 16) |
            (*(num+3) << 24);
    ret =  get_ofl_file_info(prj_num, data) ;
        
    return ret;
}
/*
 *  删除脱机方案  
 */
static error_t delete_ofl_prj( uint8_t *data)
{
     error_t ret = ERROR_SUCCESS;
//     uint8_t filename[OFL_FILE_NAME_MAX_LEN] ;      
//    
//    memcpy(filename, data, sizeof(filename) ); 
    ret = ofl_file_delete(data[0] );
    if(ERROR_SUCCESS != ret)
        return ret;
    ret = get_all_ofl_file();
    return ret;
}
//脱机工程信息下载
static error_t download_ofl_prj_info(uint8_t len , uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t filename[OFL_FILE_NAME_MAX_LEN] ;      

    ret = ofl_file_get_free();
    if(ERROR_SUCCESS != ret)
        return ret;
    memcpy(filename, data, sizeof(filename) ); 
    //创建脱机方案
    ret = ofl_file_open((char *)filename) ;
    if(ERROR_SUCCESS != ret)
        return ret;
    ofl_file_init_partition();
    //保存分区信息

    file_part.type = OFL_INFO_PART;
    file_part.size = len;
    file_part.start = OFL_PRJ_INFO_ADDR;
    file_part.data = 0;
    ofl_file_add_partition(&file_part);
    
    //保存数据
    ret = ofl_file_write_start();    
    ret = ofl_file_write(data, len);
    if(ERROR_SUCCESS != ret)
        return ret;

    return ERROR_SUCCESS;
}
/*
 *  脱机方案时序下载
 *  
 */
static error_t download_ofl_prj_timing(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t size;
//    uint32_t addr;   
//    
//    addr = (*(data+0) <<  0) |
//            (*(data+1) <<  8) |
//            (*(data+2) << 16) |
//            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24); 
    ret = ofl_file_write( data+8 , size);
    return ret;
}
/*
 *  脱机方案时序下载完成
 *  
 */
static error_t download_ofl_prj_timing_end(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t checksum = 0;  
    uint32_t timing_size = 0;
    uint32_t read_addr = 0;
    uint32_t read_size = 0;     
    uint8_t read_buf[FLASH_PRG_MIN_SIZE] = {0};    

    ret = ofl_file_write_end();  
    timing_size =  ESLINK_ROM_LINK_SIZE;
    read_addr =  file_part.start + file_part.size;
    
    ret = ofl_file_read_start();       
    while(true)
    {        
        read_size = MIN(timing_size, sizeof(read_buf));
        ret = ofl_file_read(read_addr, read_buf, read_size);
        if(ERROR_SUCCESS != ret)
            return ret; 
        checksum += check_sum(read_size, read_buf);
        
        read_addr += read_size;
        timing_size -= read_size;
        if (timing_size <= 0) 
            break;  
    }
    ret = ofl_file_read_end();     
    if(ERROR_SUCCESS != ret)
        return ret;   
    //上位机下发的校验和低位两个字节
    if( (data[0] ==  (checksum & 0x000000FF) ) &&
        (data[1] ==  (checksum & 0x0000FF00) >> 8 ) ) 
    {
        ret = ERROR_SUCCESS;
        //保存分区信息
        file_part.type = OFL_TIMING_PART;       
        file_part.start += file_part.size;
        file_part.size = ESLINK_ROM_LINK_SIZE;
        file_part.data = checksum;
        ofl_file_add_partition(&file_part);     
        ret = ofl_file_write_start();         
    }
    else
    {
    //TODO：删除方案          
         ret = ERROR_OFL_TIMING_CHECKSUM;
        delete_current_ofl_file();
    }   
    return ret;    
  
}
/*
 *  脱机方案时序信息下载
 *  
 */
static error_t download_ofl_prj_timing_info(uint8_t len ,uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t i;
    uint32_t offset;
    uint32_t size;
    static es_target_cfg target_dev;   
   
    ret = ofl_file_write( data, len);    
    if(ERROR_SUCCESS != ret)
            return ret;
    //获取脱机方案的目标芯片信息               
    offset = sizeof(timing_info_t);      
    size = sizeof(es_target_cfg);
    for(i=0 ; i< size; i++)        
        *((uint8_t *)&target_dev + i ) = data[offset + i];  
    // 根据目标芯片分区值，跟新分区信息
    ofl_target_cfg_word_size = target_dev.config_word_size ;     
    ofl_target_image_size = target_dev.code_size ;
    
    //保存分区信息
    file_part.type = OFL_TIMING_INFO_PART;       
    file_part.start += file_part.size;
    file_part.size = sizeof(es_target_cfg)+sizeof(timing_info_t);
    file_part.data = 0;
    ofl_file_add_partition(&file_part); 
 
    return ret;
}
//脱机方案配置字下载
static error_t download_ofl_prj_config(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;     
    uint32_t size; 
//    uint32_t addr;    
//    
//    addr = (*(data+0) <<  0) |
//            (*(data+1) <<  8) |
//            (*(data+2) << 16) |
//            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);   
    ret = ofl_file_write(data+8 , size);
    return ret;
}
//脱机方案配置字下载完成
static error_t download_ofl_prj_config_end(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t checksum = 0;  
    uint32_t cfg_word_size = 0;
    uint32_t read_addr = 0;
    uint32_t read_size = 0;     
    uint8_t read_buf[FLASH_PRG_MIN_SIZE] = {0};   
    
    ret = ofl_file_write_end();      

    cfg_word_size =  ofl_target_cfg_word_size;
    read_addr =  file_part.start + file_part.size;
    
    ret = ofl_file_read_start();       
    while(true)
    {        
        read_size = MIN(cfg_word_size, sizeof(read_buf));
        ret = ofl_file_read(read_addr, read_buf, read_size);
        if(ERROR_SUCCESS != ret)
            return ret; 
        checksum += check_sum(read_size, read_buf);
        
        read_addr += read_size;
        cfg_word_size -= read_size;
        if (cfg_word_size <= 0) 
            break;  
    }
    ret = ofl_file_read_end();   
    if(ret != ERROR_SUCCESS)
        return ret;
     //上位机下发的校验和低位两个字节
    if( (data[0] ==  (checksum & 0x000000FF) ) &&
        (data[1] ==  (checksum & 0x0000FF00) >> 8 ) ) 
    {
        ret = ERROR_SUCCESS;
        //保存分区信息
        file_part.type = OFL_CONFIG_PART;       
        file_part.start += file_part.size;
        file_part.size = ofl_target_cfg_word_size;
        file_part.data = checksum;
        ofl_file_add_partition(&file_part);     
        ret = ofl_file_write_start();         
    }
    else
    {
    //TODO：删除方案
         ret = ERROR_OFL_CFG_WORD_CHECKSUM;
        delete_current_ofl_file();
    }   
    return ret;
}
//脱机序列号下载
static error_t download_ofl_serial_num(uint8_t *data, uint16_t size)
{
    error_t ret = ERROR_SUCCESS;  
    //保存分区信息
    file_part.type = OFL_SERIALNUM_PART;       
    file_part.start += file_part.size;
    file_part.size = size;
    file_part.data = 0;
    ofl_file_add_partition(&file_part); 
    
    ret = ofl_file_write(data , size);
    return ret;     
}

//脱机方案用户HEX下载
static error_t download_ofl_prj_hex(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t size;
//    uint32_t addr;           
//    
//    addr = (*(data+0) <<  0) |
//            (*(data+1) <<  8) |
//            (*(data+2) << 16) |
//            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);        
    ret = ofl_file_write( data+8 , size);

    return ret;       
}
/*
 *  脱机方案用户HEX下载完成
 *  
 */
static error_t download_ofl_prj_hex_end(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t checksum = 0;  
    uint32_t image_size = 0;
    uint32_t read_addr = 0;
    uint32_t read_size = 0;     
    uint8_t read_buf[FLASH_PRG_MIN_SIZE] = {0};   
    
    ret = ofl_file_write_end();      

    image_size =  ofl_target_image_size;
    read_addr =  file_part.start + file_part.size;
    
    ret = ofl_file_read_start();       
    while(true)
    {        
        read_size = MIN(image_size, sizeof(read_buf));
        ret = ofl_file_read(read_addr, read_buf, read_size);
        if(ERROR_SUCCESS != ret)
            return ret; 
        checksum += check_sum(read_size, read_buf);
        
        read_addr += read_size;
        image_size -= read_size;
        if (image_size <= 0) 
            break;  
    }
    ret = ofl_file_read_end(); 
    if(ret != ERROR_SUCCESS)
        return ret;
     //上位机下发的校验和低位两个字节
    if( (data[0] ==  (checksum & 0x000000FF) ) &&
        (data[1] ==  (checksum & 0x0000FF00) >> 8 ) ) 
    {
        ret = ERROR_SUCCESS;
        //保存分区信息
        if(data[8] == 0x00)
            file_part.type = OFL_HEX_PART; 
        else if(data[8] == 0x01)
        {
            file_part.type = OFL_RTC_HEX_PART; 
            ofl_file_set_type(OFL_TYPE_RTC);
        }
            
        file_part.start += file_part.size;
        file_part.size = ofl_target_image_size;
        file_part.data = checksum&0x0000ffff;
        ofl_file_add_partition(&file_part);     
        ret = ofl_file_write_start();         
    }
    else
    {
    //TODO：删除方案
         ret = ERROR_OFL_USER_HEX_CHECKSUM;
        delete_current_ofl_file();
    }   
    return ret;
}    
/*
 *  脱机方案下载完成
 *  
 */
static error_t download_ofl_prj_end(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    ret = ofl_file_write_end();    

    //保存分区信息
    ret = ofl_file_save_partition();
   //更新脱机方案
	ret =  get_all_ofl_file() ;
	return ret;
}

#if ESLINK_RTC_ENABLE
/*
 *  获取自校正值
 *  
 */
static error_t read_rtc_cali(uint8_t *buf)
{
    error_t ret = ERROR_SUCCESS;   
    
    ret = get_rtc_self_calibrate(buf, 4);
    
//    buf[0] = (data >> 0) &0xFF  ;
//    buf[1] = (data >> 8) &0xFF  ;
//    buf[2] = (data >> 16) &0xFF  ;
//    buf[3] = (data >> 24) &0xFF  ;
    
    return  ret;    
}

/*
 *  设置自校正值
 *  
 */
static error_t download_rtc_cali(uint8_t *buf)
{
    error_t ret = ERROR_SUCCESS; 
    
    ret = set_rtc_self_calibrate( buf, 4);
    return  ret;    
}

#endif

prog_comm_frame_t prog_data; 

uint32_t prog_process_command(uint8_t *request, uint8_t *response)
{
    static uint8_t offline_status = 0;
    uint8_t result = ERROR_SUCCESS;
    uint32_t ack_len = 512;         //isp通信采用变长协议，用来返回回复数据的长度
    
    prog_data.wrbuf = request;
    prog_data.rdbuf = response;     
   
    if( (*request != 0xBA) | (*(request+1) != 0xDC) | (*(request+2) != 0xcd) | (*(request+3) != 0xAB) )
    {
        result = ERROR_FRAME_HEAD;
        goto __ACK;
    }
    if(*(request+4) != ESLINKII_DEVICE_TYPE){
        result = ERROR_DEVICE_TYPE;
        goto __ACK;
    }
    LED_YELLOW_ON();
    prog_data.device_type   = *(request+4);
    prog_data.fun_code      = *(request+5);
    prog_data.data_length   = (*(request+6) << 0) | (*(request+7) << 8);
    
    memset(prog_data.rdbuf, 0, PROG_FRAME_MAX_SIZE);   
    
    switch(prog_data.fun_code){   
        case ID_HANDSHAKE:
            prog_data.data_length = 0;
            break;
        case ID_READ_SERIAL_NUMBER :
            result = read_eslink_sn( &prog_data.rdbuf[8]);
            prog_data.data_length = 4;
            break;
        case ID_DL_SERIAL_NUMBER :
            result = write_eslink_sn( &prog_data.wrbuf[8]);
            prog_data.data_length = 0;
            break;  
        //------------------------Boot判断--------------------------------
        case ID_READ_OFL_VERSION:                      //0xD7 读脱机工程版本 
            result = read_offline_version( &prog_data.rdbuf[8]);
            prog_data.data_length = sizeof(offline_info_t) + 4;
            //for test
//            prog_data.rdbuf[8] = 0x00;
//            prog_data.rdbuf[9] = 0x01;
//            prog_data.rdbuf[10] = 0x00;
//            prog_data.rdbuf[11] = 0x00;
            break;                     
        case ID_DL_OFL_START:                          //0xD8  脱机工程下载开始 
            //跳转到boot，开始更新时序
            main_reset();     
            prog_data.data_length = 0;    
            break;
        case ID_DL_OFL_HEX:                            //0xD9 下载脱机工程HEX  //1024
            ack_len = 1024;
            prog_data.data_length = 0;    
            break;
        case ID_DL_OFL_HEX_END:                        //0xDA 下载结束  
            prog_data.data_length = 0;                
            break;
        //------------------------时序判断--------------------------------
        case ID_READ_TIMING_INFO:                       //0x28 读芯片信息
            result =  read_timinginfo(&prog_data.rdbuf[8]);
            prog_data.data_length = sizeof(es_target_cfg)+sizeof(timing_info_t); 
            break; 
         case ID_DL_TIMING_INFO:                        //0x27
//            if(offline_status != 1)
//                result = ERROR_FUN_CODE;                //内部错误，在boot中更新时序
            download_ofl_prj_timing_info(prog_data.data_length ,&prog_data.wrbuf[PROG_DATA_OFFSET]);    
            offline_status = 0;                         //退出脱机状态    
            prog_data.data_length = 0; 
            break;        

        case ID_DL_TIMING_START:                        //0x31.下载时序开始 
            if(prog_data.wrbuf[8] != 0x01)
            {
                //跳转到boot，开始更新时序
                main_reset();                  
            }
            else
            {
                 offline_status = 1;                    //进入脱机文件下载状态               
            }               
            prog_data.data_length = 0; 
            break;           
        case ID_DL_TIMING_ING:                          //0x32                
            if(offline_status != 1)
                result = ERROR_FUN_CODE;                //内部错误，在boot中更新时序
            result = download_ofl_prj_timing(&prog_data.wrbuf[PROG_DATA_OFFSET]);    
            ack_len = 1024;
            prog_data.data_length = 0; 
            break;          
        case ID_DL_TIMING_END:                          //0x33.时序下载结束             
            if(offline_status != 1)
                result = ERROR_FUN_CODE;                //内部错误，在boot中更新时序
            if ( download_ofl_prj_timing_end(&prog_data.wrbuf[PROG_DATA_OFFSET]) != ERROR_SUCCESS)
                offline_status = 0;                     //退出脱机状态            
            prog_data.data_length = 0;
            break;
        //------------------------用户程序下载--------------------------------
        case ID_DL_CONFIG_WORD:                         //0x17 下载配置字   
            result = es_download_config_word(&prog_data.wrbuf[PROG_DATA_OFFSET]);
          
            prog_data.data_length = 0;
            break;
        case ID_DL_CONFIG_WORD_END:                     //0x3A  配置字下载完成
            result = es_download_config_word_end(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0;
            break;
        case ID_DL_USERHEX_START:
            result = es_download_hex_start(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0;
            break;            
        case ID_DL_USERHEX:                             //0x18 下载用户HEX(1024)
            result = es_download_hex(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0;
            ack_len = 1024;
            break;
        case ID_DL_USERHEX_END:                         //0x1D 用户程序下载结束
            result = es_download_hex_end(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0;
            break;         
        //-------------------------联机操作-------------------------------------
        case ID_DL_PRG_INTF:                            //下载烧录接口  0x37
            result = es_download_prg_intf(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0;
            break;
        case ID_READ_PRG_INTF:                          //读烧录接口  0x39 
            result = es_prog_get_intf(&prog_data.rdbuf[PROG_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 0;
            else
                prog_data.data_length = 4;
            break;
        case ID_FULL_ERASE:	                            //0x20
            result = es_erase_chip(&prog_data.wrbuf[PROG_DATA_OFFSET]);  
            prog_data.data_length = 0;
            break;  
        case ID_CHECK_EMPTY:                            //查空  0x21
            result = es_check_empty(&prog_data.rdbuf[PROG_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 8;
            else
                prog_data.data_length = 0;
            break;           
        case ID_PRG_HEX:                                //编程  0x22
            result = es_program_flash(prog_data.wrbuf[PROG_DATA_OFFSET],&prog_data.rdbuf[PROG_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 4;
            else
                prog_data.data_length = 0;
            break;
        case ID_VERIFY:                                 //校验  0x23  
            result = es_program_verify(prog_data.wrbuf[PROG_DATA_OFFSET],&prog_data.rdbuf[PROG_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 8;
            else
                prog_data.data_length = 0;
            break;
        case ID_ENCRYPT:                                //加密 0x24 
            result = es_program_encrypt();
            prog_data.data_length = 0;
            break;
         case ID_PRG_CONFIG_WORD:                       //配置字编程  0x26
            result = es_program_config_word (&prog_data.rdbuf[PROG_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 4;
            else
                prog_data.data_length = 0;
            break;
        case ID_READ_CONFIG_WORD :                      //读配置字  0x25
            result = es_read_config_word(&prog_data.wrbuf[PROG_DATA_OFFSET],&prog_data.rdbuf[PROG_DATA_OFFSET],&prog_data.data_length);
            
            break;
        case ID_READ_FLASH:                             //读flash 0x2A
            result = es_read_flash(&prog_data.wrbuf[PROG_DATA_OFFSET],&prog_data.rdbuf[PROG_DATA_OFFSET],&prog_data.data_length);
            
            ack_len = 1024;
//            result = es_read_flash();
            break;  
        case ID_READ_CHIP_CHKSUM:                      //读芯片校验和   0x2D
             result = es_read_chip_chksum(&prog_data.rdbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 4;
             break;
        case ID_READ_CHIPID:                            //读芯片ID
            result = es_read_chipid(&prog_data.rdbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 4;
            break;
        case ID_DL_SERIALNUM:                           //下载序列号代码
			result = serial_number_download (&prog_data.wrbuf[PROG_DATA_OFFSET]);           
            prog_data.data_length = 0;
            break;           
                   
        //-------------------------脱机操作-------------------------------------
        case ID_READ_OFFLINE_PRJ_NUM :                  //读方案数量  0x2F
            result = read_ofl_prj_num(&prog_data.rdbuf[PROG_DATA_OFFSET]);
             if(result != ERROR_SUCCESS)
                prog_data.data_length = 0;
            else
                prog_data.data_length = 0x4;   
            break;
        case ID_READ_OFFLINE_PRJ_INFO :                 //读方案信息
            result = read_ofl_prj_info(&prog_data.wrbuf[PROG_DATA_OFFSET],&prog_data.rdbuf[PROG_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 0;
            else
                prog_data.data_length = 0x80;      
            break;
        case ID_DELETE_OFFLINE_PRJ :                    //删除指定脱机方案
            result = delete_ofl_prj(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0;
            break;
        case ID_DL_OFFLINE_PRJ_INFO:                    //脱机方案信息下载   0x2E
            result = download_ofl_prj_info(prog_data.data_length,&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0; 
            break;  
        case ID_DL_OFFLINE_CONFIG_WORD :                //脱机方案配置字下载
            result = download_ofl_prj_config(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0;        
            break;
        case ID_DL_OFFLINE_CONFIG_WORD_END:             //脱机方案配置字下载完成
            result = download_ofl_prj_config_end(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0; 
            break;
        case ID_DL_OFFLINE_HEX:                         //脱机方案用户HEX下载
            result = download_ofl_prj_hex(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0;   
            ack_len = 1024;
            break;
        case ID_DL_OFFLINE_HEX_END:                     //脱机方案用户HEX下载完成
            result = download_ofl_prj_hex_end(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0;             
            break;
        case ID_DL_OFL_SERIALNUM :                      //脱机序列号
            result = download_ofl_serial_num(&prog_data.wrbuf[PROG_DATA_OFFSET],prog_data.data_length);
            prog_data.data_length = 0;
            break; 
        
		case ID_DL_OFFLINE_END:							//脱机方案下载完成
			result = download_ofl_prj_end(&prog_data.wrbuf[PROG_DATA_OFFSET]);
		    prog_data.data_length = 0; 
			break;
#if ESLINK_RTC_ENABLE
        case ID_RTC_START_SELF_CALIBRATE:               //启动RTC自校正
            rtc_out_mode(ENABLE);
            result =  ERROR_SUCCESS;
            prog_data.data_length = 0; 
            break;
        case ID_RTC_WRITE_CALIBRATE:                    //写RTC自校正值
            rtc_out_mode(DISABLE);
            result =  download_rtc_cali(&prog_data.wrbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 0;  
            break;
        case ID_RTC_READ_CALIBRATE:                     //读RTC自校正值  
            result =  read_rtc_cali(&prog_data.rdbuf[PROG_DATA_OFFSET]);
            prog_data.data_length = 4;         
            break;
        case ID_RTC_CALI :                              //RTC调校
            result = rtc_calibration_handler();
            prog_data.data_length = 0; 
            break;
        case ID_RTC_CALI_VERIFY:                        //RTC验证
            result = rtc_calibration_verify(&prog_data.rdbuf[PROG_DATA_OFFSET]);    
            prog_data.data_length = 4;
            break;
#endif
        default:
            result = ERROR_FUN_CODE;                    //内部错误，在boot中更新时序
            break;
    }  
//    memset(prog_data.data_buf, 0, ISP_PACKET_SIZE);      
    __ACK:   
    prog_data.rdbuf[0] = 0xBA;
    prog_data.rdbuf[1] = 0xDC;
    prog_data.rdbuf[2] = 0xCD;
    prog_data.rdbuf[3] = 0xAB;
    prog_data.rdbuf[4] = ESLINKII_DEVICE_TYPE;
    prog_data.rdbuf[5] = prog_data.fun_code;
    prog_data.state = ERROR_ESLINK_RUN_OK;
    LED_GREEN_ON();
    if(result != ERROR_SUCCESS)
    {
        LED_RED_ON();
        if(result == ERROR_FRAME_HEAD)
        {
            prog_data.rdbuf[5] = 0x00;
            prog_data.data_length = 0x00;    
        } 
        prog_data.state = result;
    }      
    prog_data.rdbuf[6] = (uint8_t)(prog_data.data_length&0xFF);    
    prog_data.rdbuf[7] = (uint8_t)(prog_data.data_length>>8); 
    prog_data.rdbuf[prog_data.data_length+8] = prog_data.state;    
    prog_data.checksum = check_sum(prog_data.data_length +9, prog_data.rdbuf);
    prog_data.rdbuf[prog_data.data_length + 9] = (uint8_t)(prog_data.checksum&0xFF);            
    prog_data.rdbuf[prog_data.data_length + 10] = (uint8_t)(prog_data.checksum>>8); 
    
    return ack_len;
}  

/*******************************************************************************
*	函 数 名: es_burner_init
*	功能说明: 
*	形    参: 无
*	返 回 值: 无
*******************************************************************************/
error_t es_burner_init(void)
{    
    error_t ret;
	get_target_info((uint8_t*)&es_target_device);       //获取目标芯片信息
	ret = es_prog_set_intf(ESLINK_ONLINE_DEFAULT_INTF); //上电后默认是ISP编程
	ISP_SETUP();
    return ret;       
}


 
                                 





































 
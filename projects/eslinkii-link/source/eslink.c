#include "main.h"
#include "eslink.h"
#include "config_rom_set.h"
#include "update.h"
#include "online_file.h"
#include "offline_file.h"
#include "isp_prog_intf.h"

static es_target_cfg es_target_device;  
static es_serial_number_t isp_sn ;       //序列号

// Delay for specified time
//    delay:  delay time in ms
void es_delay_ms(uint32_t delay) 
{
    delay *= ((SystemCoreClock/1000U) + (ES_DELAY_SLOW_CYCLES-1U)) / ES_DELAY_SLOW_CYCLES;
    ES_DELAY_SLOW(delay);
}
// Delay for specified time
//    delay:  delay time in us
void es_delay_us(uint32_t delay) 
{
    delay *= ((SystemCoreClock/1000000U) + (ES_DELAY_SLOW_CYCLES-1U)) / ES_DELAY_SLOW_CYCLES;
    ES_DELAY_SLOW(delay);
}
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
static struct es_prog_ops *es_prog_intf;
/*******************************************************************************
*	函 数 名: es_prog_set_intf
*	功能说明: 编程接口设置
*	形    参: type 编程接口类型
*	返 回 值: error
*******************************************************************************/
static error_t es_prog_set_intf(prog_intf_type_t type)
{
    if(PRG_INTF_ISP == type )
    {
         es_prog_intf =  &isp_prog_intf;    
    }
    else if (PRG_INTF_SWD ==  type )
    {
    
    }
    else if ( PRG_INTF_BOOTISP == type)
    {
    
    } 
    else
    {
        return ERROR_PROG_INTF;
    }
    return ERROR_SUCCESS;

}
static error_t es_prog_get_intf(uint8_t *data)
{
    prog_intf_type_t type;
    
    if(es_prog_intf ==  &isp_prog_intf)
    {
        type = PRG_INTF_ISP;
    }
//    else if(es_prog_intf ==  &isp_prog_intf)
//    {
//    
//    }
//    else if(es_prog_intf ==  &isp_prog_intf)
//    {
//    
//    
//    }
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
error_t serial_number_download(uint8_t *data, uint8_t size)
{
//      uint8_t i;
      
//      for(i = 
//      isp_sn.addr

}
/*
 * 更新序列号到用户HEX中
 */
void serial_number_intercept_write(uint32_t addr, uint32_t *data, uint32_t size)
{
    uint8_t sn_is_valid;    // 序列号写一次后失效              
    
    if(  isp_sn.enable_flag && (sn_is_valid == ENABLE) )
    {
        if((isp_sn.size+3)/4 == 1)
        {
            if( (isp_sn.addr >= addr) & (isp_sn.addr < addr+size) )
            {
                data[isp_sn.addr - addr] = isp_sn.data[1];	
                sn_is_valid = DISABLE;
            }
        }
        else if((isp_sn.size+3)/4 == 2)
        {
            if( (isp_sn.addr >= addr) & (isp_sn.addr < addr+size) )
                data[isp_sn.addr - addr] = isp_sn.data[0];	
            if( ( (isp_sn.addr+1) >= addr) & ( (isp_sn.addr+1) < addr+size))
            {
                data[(isp_sn.addr - addr)+1] = isp_sn.data[1];	
                sn_is_valid = DISABLE;
            }
        }
    }
}
/******************************************************************/
static uint32_t get_timing_size(void)
{
     return ESLINK_ROM_LINK_SIZE;
}

/*
 *  读固件版本
 *  固件版本不对应，则更新固件
 */
static error_t read_offline_version(uint8_t *buf)
{   
    error_t result = ERROR_SUCCESS;     
    if(get_offline_info(buf) != sizeof(offline_info_t) )
        result = ERROR_IAP_READ;
    
    return result;  
}
/*
 *  读芯片时序信息
 */
error_t read_timinginfo(uint8_t *buf)
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


/********************************读写联机数据到编程器******************************/ 
/*
 *  配置字下载
 */
static error_t es_download_config_word(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
    
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);
    if( !addr)
    {
         ret = online_file_erase(OL_CONFIG_PART, es_target_device.config_word_size );
         if(ERROR_SUCCESS != ret)
            return ret;         
    }         
    ret = online_file_write(OL_CONFIG_PART, addr, (data+8), size) ;    

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
        ret = online_file_read(OL_CONFIG_PART, read_addr,  read_buf, read_size);
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
//         ol_prj_write(SF_USER_HEX_CHECKOUT_ADDR, data, 4);      //验证正确后在保存数据   
    }
    else
    {
         ret = ERROR_CFG_WORD_CHECKSUM;
    }          
    //TODO:
    //实现CRC校验
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
    
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);  
    //下载开始时，擦除目标芯片配置字长度的spi flash。
    if( !addr)
    {
        ret = online_file_erase(OL_HEX_PART, es_target_device.code_size );
        if(ERROR_SUCCESS != ret)
            return ret;  
    }          
    ret = online_file_write(OL_HEX_PART, addr, (data+8), size) ;
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
    uint32_t code_size = es_target_device.code_size;
    uint32_t read_addr = 0;
    uint32_t read_size = 0;
    uint8_t read_buf[FLASH_PRG_MIN_SIZE] = {0};         
    
//    ol_file_write_end();

    while(code_size > 0){      
        read_size = MIN(code_size, sizeof(read_buf));
        ret = online_file_read(OL_HEX_PART, read_addr,  read_buf, read_size) ;
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
//         ol_prj_write(SF_USER_HEX_CHECKOUT_ADDR, data, 4);      //验证正确后在保存数据   
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
static error_t es_erase_chip(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t erase_mode; 
    
    erase_mode = data[0];
    ret = es_prog_intf->erase_chip(&erase_mode);
    return ret;
}
//查空
static error_t es_check_empty(uint8_t *data)
{
    uint32_t failedAddress;
    uint32_t failedData;
    error_t ret = ERROR_SUCCESS;
    ret = es_prog_intf->check_empty(&failedAddress, &failedData);
    if(ERROR_SUCCESS != ret)
    {
        int2array(&failedAddress, 1, data);
        int2array(&failedData, 1, data);
    
    }             
    return ret;
}  
/*******************************************************************************
*	函 数 名: es_program_flash
*	功能说明: 编程
*	形    参: 编程错误地址 和数据
*	返 回 值: 无
*******************************************************************************/
static error_t es_program_flash(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t i;
    uint32_t hex_size;    
    uint32_t prg_addr;  //编程地址
    uint32_t copy_size;
    uint32_t read_addr;
    uint8_t read_buf[FLASH_PRG_MIN_SIZE];
    
    hex_size =  es_target_device.code_size;
    prg_addr = 0;
    read_addr =  0;
    
    while(true)
    {
        copy_size = MIN(hex_size, sizeof(read_buf) );
        
        ret = online_file_read(OL_HEX_PART, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret;
        for(i=0; i<copy_size; i++)
        {
            if(read_buf[i] != 0xFF)
                break;
        }
        if(i < copy_size)      //数据段都为0xFF,不进行编程
        {
            ret = es_prog_intf->program_flash(prg_addr, read_buf, copy_size); 
            if( ret !=  ERROR_SUCCESS)
                return ret;          
        }           
        // Update variables
        prg_addr  += copy_size;
        hex_size  -= copy_size;
        read_addr += copy_size;
        // Check for end
        if (hex_size <= 0) 
            break;       
    }
    return  ERROR_SUCCESS;
}
/*******************************************************************************
*	函 数 名: es_read_flash
*	功能说明: 读目标芯片flash数据
*	形    参: 读出的数据
*	返 回 值: error_t
*******************************************************************************/
static error_t es_read_flash(uint8_t *wrbuf, uint8_t *rdbuf)
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
    
    return ret;

}
/*******************************************************************************
*	函 数 名: es_program_config_word
*	功能说明: 配置字编程
*	形    参: data: 编程错误地址 和 数据
*	返 回 值: error_t
*******************************************************************************/
static error_t es_program_config_word(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    
    return ret;

}
//读配置字
static error_t es_read_config_word(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    
    return ret;

}

//读chipid
static error_t es_read_chipid(uint8_t *data)
{
     error_t ret = ERROR_SUCCESS;
     
     return ret;

}   

/********************************脱机操作***************************************/
static struct  ofl_file_partition  part_info;

//脱机工程信息下载
static error_t download_ofl_prj_info(uint8_t len , uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t filename[OFL_FILE_NAME_MAX_LEN] ;      
//    uint8_t buf[128];       //for test
    memcpy(filename, data, sizeof(filename) ); 
    
    ret = ofl_file_open((char *)filename) ;
    if(ERROR_SUCCESS != ret)
        return ret;
        
    ret = ofl_file_write(OFL_INFO_PART, data, len);
    if(ERROR_SUCCESS != ret)
        return ret;
//    ofl_file_read(OFL_INFO_PART,0,buf,128);   //for test
    return ERROR_SUCCESS;
}
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
     uint8_t filename[OFL_FILE_NAME_MAX_LEN] ;      
    
    memcpy(filename, data, sizeof(filename) ); 
    ret = delete_ofl_file((char*) filename);
    if(ERROR_SUCCESS != ret)
        return ret;
    ret = get_all_ofl_file();
    return ret;
}
/*
 *  脱机方案时序下载
 *  
 */
static error_t download_ofl_prj_timing(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
    
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24); 
    ret = ofl_file_write(OFL_TIMING_PART, data+8 , size);
    return ret;
}
/*
 *  脱机方案时序下载完成
 *  
 */
static error_t download_ofl_prj_timing_end(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t checksum_temp[4];
    
	ret = get_ofl_file_checksum(OFL_TIMING_PART, checksum_temp) ;
    if(ERROR_SUCCESS != ret)
        return ret; 
    if( (data[0] ==  checksum_temp[0]) && (data[1] ==  checksum_temp[1]) &&
        (data[2] ==  checksum_temp[2]) && (data[3] ==  checksum_temp[3]) ) 
    {
//         ofl_file_write(data, 4);      //验证正确后在保存数据 
        ret = ERROR_SUCCESS;
    }
    else
    {
    //TODO：删除方案
         ret = ERROR_OFL_TIMING_CHECKSUM;
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
    ret = ofl_file_write( OFL_TIMING_INFO_PART, data, len);    
    return ret;
}
//脱机方案配置字下载
static error_t download_ofl_prj_config(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;      
    
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);  
    ret = ofl_file_write(OFL_CONFIG_PART, data+8 , size);
    return ret;
}
//脱机方案配置字下载完成
static error_t download_ofl_prj_config_end(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t checksum_temp[4];
    
    ret = get_ofl_file_checksum(OFL_CONFIG_PART, checksum_temp) ; 
    if(ret != ERROR_SUCCESS)
        return ret;
    if( (data[0] ==  checksum_temp[0]) && (data[1] ==  checksum_temp[1]) &&
        (data[2] ==  checksum_temp[2]) && (data[3] ==  checksum_temp[3]) ) 
    {
//         ofl_file_write(data, 4);      //验证正确后在保存数据 
         ret = ERROR_SUCCESS;
    }
    else
    {
    //TODO：删除方案
         ret = ERROR_OFL_CFG_WORD_CHECKSUM;
    }   
    return ret;
}
//脱机方案用户HEX下载
static error_t download_ofl_prj_hex(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;      
    
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);  
    ret = ofl_file_write(OFL_HEX_PART, data+8 , size);
    return ret;       
}
/*
 *  脱机方案用户HEX下载完成
 *  
 */
static error_t download_ofl_prj_hex_end(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t checksum_temp[4];
    
    ret = get_ofl_file_checksum(OFL_HEX_PART, checksum_temp) ; 
    if(ret != ERROR_SUCCESS)
        return ret;
    if( (data[0] ==  checksum_temp[0]) && (data[1] ==  checksum_temp[1]) &&
        (data[2] ==  checksum_temp[2]) && (data[3] ==  checksum_temp[3]) ) 
    {
//         ofl_file_write(data, 4);      //验证正确后在保存数据 
         ret = ERROR_SUCCESS;
    }
    else
    {
    //TODO：删除方案
         ret = ERROR_OFL_USER_HEX_CHECKSUM;
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
	 //更新脱机方案
	 ret =  get_all_ofl_file() ;
	 return ret;
}


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

    prog_data.device_type   = *(request+4);
    prog_data.fun_code      = *(request+5);
    prog_data.data_length   = (*(request+6) << 0) | (*(request+7) << 8);
    
    memset(prog_data.rdbuf, 0, PROG_FRAME_SIZE);   
    
    switch(prog_data.fun_code){   
        case ID_HANDSHAKE:
            prog_data.data_length = 0;
            break;
        //------------------------Boot判断--------------------------------
        case ID_READ_BOOT_VERSION:                      //0xD7 读脱机工程版本 
            result = read_offline_version( &prog_data.rdbuf[8]);
            prog_data.data_length = sizeof(offline_info_t);
            //for test
            prog_data.rdbuf[8] = 0x01;
            prog_data.rdbuf[9] = 0x01;
            prog_data.rdbuf[10] = 0x00;
            prog_data.rdbuf[11] = 0x00;
            break;                     
        case ID_DL_BOOT_START:                          //0xD8  脱机工程下载开始 
            prog_data.data_length = 0;    
            break;
        case ID_DL_BOOT_HEX:                            //0xD9 下载脱机工程HEX  //1024
            ack_len = 1024;
            prog_data.data_length = 0;    
            break;
        case ID_DL_BOOT_HEX_END:                        //0xDA 下载结束  
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
            download_ofl_prj_timing_info(prog_data.data_length ,&prog_data.wrbuf[ES_DATA_OFFSET]);    
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
                 offline_status = 1;                    //进入脱机状态               
            }               
            prog_data.data_length = 0; 
            break;           
        case ID_DL_TIMING_ING:                          //0x32                
            if(offline_status != 1)
                result = ERROR_FUN_CODE;                //内部错误，在boot中更新时序
            result = download_ofl_prj_timing(&prog_data.wrbuf[ES_DATA_OFFSET]);    
            ack_len = 1024;
            prog_data.data_length = 0; 
            break;          
        case ID_DL_TIMING_END:                          //0x33.时序下载结束             
            if(offline_status != 1)
                result = ERROR_FUN_CODE;                //内部错误，在boot中更新时序
            if ( download_ofl_prj_timing_end(&prog_data.wrbuf[ES_DATA_OFFSET]) != ERROR_SUCCESS)
                offline_status = 0;                     //退出脱机状态            
            prog_data.data_length = 0;
            break;
        //------------------------用户程序下载--------------------------------
        case ID_DL_CONFIG_WORD:                         //0x17 下载配置字   
            result = es_download_config_word(&prog_data.wrbuf[ES_DATA_OFFSET]);          
            result = ERROR_SUCCESS;
            prog_data.data_length = 0;
            break;
        case ID_DL_CONFIG_WORD_END:                     //0x3A  配置字下载完成
            result = es_download_config_word_end(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;
            break;
        case ID_DL_USERHEX:                             //0x18 下载用户HEX(1024)
            result = es_download_hex(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;
            ack_len = 1024;
            break;
        case ID_DL_USERHEX_END:                         //0x1D 用户程序下载结束
            result = es_download_hex_end(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;
            break;         
        //-------------------------联机操作-------------------------------------
        case ID_DL_PRG_INTF:                            //下载烧录接口  0x37
            result = es_download_prg_intf(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;
            break;
        case ID_READ_PRG_INTF:                          //读烧录接口  0x39 
            result = es_prog_get_intf(&prog_data.rdbuf[ES_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 0;
            else
                prog_data.data_length = 4;
            break;
        case ID_FULL_ERASE:	                            //0x20
            result = es_erase_chip(&prog_data.wrbuf[ES_DATA_OFFSET]);  
            prog_data.data_length = 0;
            break;  
        case ID_CHECK_EMPTY:                            //查空
            result = es_check_empty(&prog_data.rdbuf[ES_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 8;
            break;           
        case ID_PRG_HEX:                                //flash编程
            result = es_program_flash(&prog_data.rdbuf[ES_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 8;
            else
                prog_data.data_length = 0;
            break;
        case ID_READ_FLASH:                             //读flash 0x2A
            result = es_read_flash(&prog_data.wrbuf[ES_DATA_OFFSET],&prog_data.rdbuf[ES_DATA_OFFSET]);
            
            ack_len = 1024;
//            result = es_read_flash();
            break;
         case ID_PRG_CONFIG_WORD:                       //配置字编程
            result = es_program_config_word (&prog_data.rdbuf[ES_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 8;
            else
                prog_data.data_length = 0;
            break;
        case ID_READ_CONFIG_WORD :                      //读配置字
            result = es_read_config_word(&prog_data.rdbuf[ES_DATA_OFFSET]);
//            prog_data.data_length = 4;
            break;
        
        case ID_READ_CHIPID:                            //读芯片ID
            result = es_read_chipid(&prog_data.rdbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 4;
            break;
        case ID_DL_SERIALNUM:                           //下载序列号代码
            prog_data.data_length = 0;
            break;           
            
        //-------------------------脱机操作-------------------------------------
        case ID_READ_OFFLINE_PRJ_NUM :                  //读方案数量  0x2F
            result = read_ofl_prj_num(&prog_data.rdbuf[ES_DATA_OFFSET]);
             if(result != ERROR_SUCCESS)
                prog_data.data_length = 0;
            else
                prog_data.data_length = 0x4;   
            break;
        case ID_READ_OFFLINE_PRJ_INFO :                 //读方案信息
            result = read_ofl_prj_info(&prog_data.wrbuf[ES_DATA_OFFSET],&prog_data.rdbuf[ES_DATA_OFFSET]);
            if(result != ERROR_SUCCESS)
                prog_data.data_length = 0;
            else
                prog_data.data_length = 0x80;      
            break;
        case ID_DELETE_OFFLINE_PRJ :                    //删除指定脱机方案
            result = delete_ofl_prj(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;
            break;
        case ID_DL_OFFLINE_PRJ_INFO:                    //脱机方案信息下载   0x2E
            result = download_ofl_prj_info(prog_data.data_length,&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0; 
            break;  
        case ID_DL_OFFLINE_CONFIG_WORD :                //脱机方案配置字下载
            result = download_ofl_prj_config(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;        
            break;
        case ID_DL_OFFLINE_CONFIG_WORD_END:             //脱机方案配置字下载完成
            result = download_ofl_prj_config_end(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;     
        case ID_DL_OFFLINE_HEX:                         //脱机方案用户HEX下载
            result = download_ofl_prj_hex(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;   
            ack_len = 1024;
            break;
        case ID_DL_OFFLINE_HEX_END:                     //脱机方案用户HEX下载完成
            result = download_ofl_prj_hex_end(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;   
            
            break; 
		case ID_DL_OFFLINE_END:							//脱机方案下载完成
			 result = download_ofl_prj_end(&prog_data.wrbuf[ES_DATA_OFFSET]);
		    prog_data.data_length = 0; 
			break;
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
    if(result != ERROR_SUCCESS)
    {
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
*	函 数 名: eslink_init
*	功能说明: 
*	形    参: 无
*	返 回 值: 无
*******************************************************************************/
void eslink_init(void)
{        
     get_target_info((uint8_t*)&es_target_device);       //获取目标芯片信息
     es_prog_set_intf(PRG_INTF_ISP);                    //上电后默认是ISP编程
     es_prog_intf->init(&es_target_device);
     ISP_SETUP();
     PORT_ISP_SETUP();
     
}



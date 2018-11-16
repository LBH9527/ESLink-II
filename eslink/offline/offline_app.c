#include "eslink.h"
#include "es_common.h" 
#include "offline_file.h" 
#include "isp_prog_intf.h"  
#include "settings_rom.h"
#include "settings_spi_flash.h"
#include "offline_app.h"
#include "eeprom.h"
#include "menu.h"

static error_t ofl_program_flash(uint8_t sn_enable) ;
static error_t ofl_program_verify(uint8_t sn_enable);

static struct es_prog_ops *ofl_prog_intf;   //脱机编程接口
static es_target_cfg ofl_target_device;     //目标芯片信息    
static ofl_prj_info_t ofl_prj_info;         //脱机方案信息
static ofl_serial_number_t sn_info;                //序列号信息

error_t ofl_prog_init(void)
{
    error_t ret = ERROR_SUCCESS;
    
    PORT_ISP_SETUP();
    ISP_SETUP();
    //获取方案信息
    online_file_read(OFL_PROG_INFO, 0,(uint8_t*) &ofl_prj_info, sizeof(ofl_prj_info_t)); 
    //获取脱机序列号
    fm24cxx_read(EE_SERIAL_NUMBER_ADDR,(uint8_t*) &sn_info, sizeof(ofl_serial_number_t) );
    
    //脱机烧录接口
    if(PRG_INTF_ISP == ofl_prj_info.intf )
    {
        ofl_prog_intf = &isp_prog_intf;   
    }
    else if (PRG_INTF_SWD ==  ofl_prj_info.intf )
    {
    
    }
    else if ( PRG_INTF_BOOTISP == ofl_prj_info.intf)
    {
    
    } 
    else
    {
        //todo 接口类型错误
        while(1);
        return ERROR_PROG_INTF;
    } 
    //获取目标芯片信息
	get_target_info((uint8_t*)&ofl_target_device); 
    //烧录接口初始化
	ofl_prog_intf->init(&ofl_target_device);    
    
    return  ret;
}


//判断编程模式
//TRUE 进模式成功 FALSE 进模式失败
uint8_t ofl_in_prog_mode(void)
{
    static uint8_t check_time = 0;
    
    //外部触发
    if(OFL_PROG_START_MACHINE_MODE == sn_info.start_mode )    //自动检测
    {
        if( ofl_start_in_low() != 1)    //按键按下
        {
            check_time = 0;  
        }
        else
        {
            es_delay_ms(20);
            if(check_time ++ >= PROG_MODE_CHECK_TIME )
                return TRUE; 
        }
        
    } 
    else
    {      
        if(ofl_prog_intf->prog_init() != ERROR_SUCCESS)        //连续检测5次OK，间隔20ms ，表示进入检测到了芯片    
        {
            check_time = 0;          
        }
        else
        {
            es_delay_ms(20);
            if(check_time ++ >= PROG_MODE_CHECK_TIME )
                return TRUE;        
        }      
    }
    
    return FALSE;    
}
//出模式判断
uint8_t ofl_out_prog_mode(void)
{
    static uint8_t check_time = 0;
    
    //外部触发
    if(OFL_PROG_START_MACHINE_MODE == sn_info.start_mode )    //自动检测
    {
        if( ofl_start_in_low() != 0)    //按键弹起
        {
            check_time = 0;  
        }
        else
        {
            es_delay_ms(20);
            if(check_time ++ >= PROG_MODE_CHECK_TIME )
                return TRUE; 
        }           
    } 
    else
    {      
        if(ofl_prog_intf->prog_init() == ERROR_SUCCESS)        //连续检测5次OK，间隔20ms ，表示进入检测到了芯片    
        {
            check_time = 0;          
        }
        else
        {
            es_delay_ms(20);
            if(check_time ++ >= PROG_MODE_CHECK_TIME )
                return TRUE;        
        }      
    }
    
    return FALSE;   

}

//根据脱机步骤进行脱机编程
ofl_prog_error_t ofl_prog(void)
{
    error_t ret;
    uint32_t faildata,failaddr;
    uint32_t i = 0;
    if(sn_info.success_count >=  sn_info.total_size)
        return OFL_COUNT_FULL;
    for(i = 0; i<ofl_prj_info.step; i++)
    {
        switch(ofl_prj_info.item[i])
        {
            case OFL_STEP_ERASE:
                ret = ofl_prog_intf->erase_chip(0);    //全擦
                break;
            case OFL_STEP_CHECK_EMPTY :
                ret = ofl_prog_intf->check_empty(&failaddr, &faildata);
                break;
            case OFL_STEP_PROG:                   
                if(sn_info.state !=  OFL_SERIALNUM_DISABLE)
                {
                     ret = ofl_program_flash(1);
                }                    
                else
                    ret = ofl_program_flash(0);
                
                break;
            case OFL_STEP_VERIFY:
                if(sn_info.state !=  OFL_SERIALNUM_DISABLE)
                    ret = ofl_program_verify(1);
                else
                    ret = ofl_program_verify(0);
                break;
            case OFL_STEP_ENCRYPT :
                ret = ofl_prog_intf->encrypt_chip(); 
                break;
        }  
        //编程失败 返回
        if(ret != ERROR_SUCCESS)
            return OFL_PROG_FAIL;
    }
    //编程成功
    sn_info.success_count ++;       //烧录成功+1
    if(sn_info.state !=  OFL_SERIALNUM_DISABLE)
    {  
        uint64_t data;
        if(sn_info.read_mode  == OFL_SERIALNUM_READ_USE_IAP)    //32位机 IAP方式
        {               
            data =((uint64_t)sn_info.sn.data[0]<< 56) | ((uint64_t)sn_info.sn.data[1] << 48) | ((uint64_t)sn_info.sn.data[2] << 40) | ((uint64_t)sn_info.sn.data[3] << 32)  \
                  | ((uint64_t)sn_info.sn.data[4]<< 24) | (sn_info.sn.data[5] << 16) | (sn_info.sn.data[6] << 8) | (sn_info.sn.data[7] << 0);
            
            data += sn_info.sn_step; 
            memcpy( &sn_info.sn.data,(uint8_t*)&data, 8);       
        }
        else    //8位机返回指令
        {
            data =((uint64_t)sn_info.sn.data[1]<< 56) | ((uint64_t)sn_info.sn.data[3] << 48) | ((uint64_t)sn_info.sn.data[5] << 40) | ((uint64_t)sn_info.sn.data[7] << 32)  \
                  | ((uint64_t)sn_info.sn.data[9]<< 24) | (sn_info.sn.data[11] << 16) | (sn_info.sn.data[13] << 8) | (sn_info.sn.data[15] << 0);            
            data += sn_info.sn_step; 
            sn_info.sn.data[1] =  (data >> 56) & 0xff;
            sn_info.sn.data[3] =  (data >> 48) & 0xff;
            sn_info.sn.data[5] =  (data >> 40) & 0xff;
            sn_info.sn.data[7] =  (data >> 32) & 0xff;
            sn_info.sn.data[9] =  (data >> 24) & 0xff;
            sn_info.sn.data[11] =  (data >> 16) & 0xff;   
            sn_info.sn.data[13] =  (data >> 8) & 0xff;
            sn_info.sn.data[15] =  (data >> 0) & 0xff;               
        }  
  
    }  
    fm24cxx_write(EE_SERIAL_NUMBER_ADDR,(uint8_t*) &sn_info, sizeof(ofl_serial_number_t) );
    return OFL_SUCCESS;
}



static error_t ofl_program_flash(uint8_t sn_enable)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t i;
    
    uint32_t failaddr;
    uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	   
    
    uint32_t copy_size;      
    uint32_t read_addr;
    uint8_t read_buf[FLASH_PRG_MIN_SIZE];
    
    code_addr =  ofl_target_device.code_start;
	code_size =  ofl_target_device.code_size;
    read_addr =  0; 
    
    ret = ofl_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        return  ret;
        
    while(true)
    {
        copy_size = MIN(code_size, sizeof(read_buf) );    
        
        ret = online_file_read(USER_HEX, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret;
        if(sn_enable == 1)     //序列号代码使能
            serial_number_intercept_write(&sn_info.sn, code_addr, read_buf, copy_size);	//填入序列号
            
        for(i=0; i<copy_size; i++)
        {
            if(read_buf[i] != 0xFF)
                break;
        }
        if(i < copy_size)      //数据段都为0xFF,不进行编程
        {
            ret = ofl_prog_intf->program_flash(code_addr, read_buf, copy_size, &failaddr); 
            if( ret !=  ERROR_SUCCESS)   //编程失败，返回编程失败地址
            {
                 return ret;   
            }                          
        }           
        // Update variables
        code_addr  += copy_size;
        code_size  -= copy_size;
        read_addr += copy_size;
        // Check for end
        if (code_size <= 0) 
            break;       
    }
    cfg_word_addr =  ofl_target_device.config_word_start;
	cfg_word_size =  ofl_target_device.config_word_size;
    read_addr =  0;
    while(true)
    {
        copy_size = MIN(cfg_word_size, sizeof(read_buf) );          
        ret = online_file_read(CFG_WORD, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret; 		
        ret = ofl_prog_intf->program_config_word(cfg_word_addr, read_buf, copy_size, NULL); 
        if( ret !=  ERROR_SUCCESS)
        {
            return ret; 
        }                  
        // Update variables
        cfg_word_addr  += copy_size;
        cfg_word_size  -= copy_size;
        read_addr += copy_size;
        // Check for end
        if (cfg_word_size <= 0) 
            break;       
    }
    return  ERROR_SUCCESS;  
}

static error_t ofl_program_verify(uint8_t sn_enable)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t faildata,failaddr;
    uint32_t checksum = 0;  
    uint32_t sf_checksum = 0;   //spi保存的校验和
    
    uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	   
//    uint8_t read_buf[FLASH_PRG_MIN_SIZE];
    
    uint32_t verify_size; 
    uint32_t sf_addr;  
    uint8_t sf_buf[FLASH_PRG_MIN_SIZE];   
    
    ret = ofl_prog_intf->prog_init();
    if(ERROR_SUCCESS != ret)
        return  ret;
        
    code_addr =  ofl_target_device.code_start;
	code_size =  ofl_target_device.code_size;
    sf_addr = 0;
    while(true)
    {
        verify_size = MIN(code_size, sizeof(sf_buf) );
       
        ret = online_file_read(USER_HEX, sf_addr, sf_buf , verify_size);
        if(ERROR_SUCCESS != ret)
            return ret;
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        if( sn_enable == 1)
            serial_number_intercept_write(&sn_info.sn, code_addr, sf_buf, verify_size);	//填入序列号  
            
        ret = ofl_prog_intf->verify_flash(code_addr, sf_buf, verify_size, &failaddr, &faildata);                        
        if( ret !=  ERROR_SUCCESS)
        {    
            return  ret;                       
        }   
        // Update variables
        code_addr  += verify_size;
        code_size  -= verify_size;
        sf_addr += verify_size;
        // Check for end
        if (code_size <= 0) 
            break;       
    }  
    online_file_read(HEX_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if((sf_checksum&0x0000ffff) != (checksum&0x0000ffff))
        return ERROR_USER_HEX_CHECKSUM;
    
    cfg_word_addr =  ofl_target_device.config_word_start;
	cfg_word_size =  ofl_target_device.config_word_size;
    sf_addr =  0;
    checksum = 0;
    while(true)
    {
        verify_size = MIN(cfg_word_size, sizeof(sf_buf) );          
        ret = online_file_read(CFG_WORD, sf_addr, sf_buf , verify_size);
        if(ERROR_SUCCESS != ret)
            return ret; 
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        
        ret = ofl_prog_intf->verify_config_word(cfg_word_addr, sf_buf, verify_size, &failaddr, &faildata); 
        if( ret !=  ERROR_SUCCESS)
        {    
            return  ret;                       
        }   
        // Update variables
        cfg_word_addr  += verify_size;
        cfg_word_size  -= verify_size;
        sf_addr += verify_size;
        // Check for end
        if (cfg_word_size <= 0) 
            break;       
    }
    online_file_read(CFG_WORD_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if(sf_checksum != (checksum&0x0000ffff))
        return ERROR_CFG_WORD_CHECKSUM;
    
    return  ERROR_SUCCESS;
}


//退出脱机模式时，更新脱机序列号信息
error_t update_ofl_serial_number(void)
{  
    error_t ret = ERROR_SUCCESS;
    char path[16+1];
    partition_t part; 
    
    if(sn_info.state ==  OFL_SERIALNUM_DISABLE)
        return  ERROR_SUCCESS;
    
    //读文件名
    fm24cxx_read(EE_OFL_PRJ_NAME, (uint8_t*)path, 16 );   
    //读分区信息        
    fm24cxx_read(EE_OFL_SERIAL_NUMBER_PARTITION, (uint8_t*)&part, sizeof(partition_t) );      
    //    
    fm24cxx_read(EE_SERIAL_NUMBER_ADDR,(uint8_t*) &sn_info, sizeof(ofl_serial_number_t) );

    ret = ofl_file_lseek_write( path, part.start, (uint8_t*) &sn_info, part.size );
    
    return ret ;

}



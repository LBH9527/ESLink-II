#include "eslink.h"
#include "es_common.h" 
#include "offline_file.h" 
#include "isp_prog_intf.h"  
#include "settings_rom.h"
#include "settings_spi_flash.h"
#include "offline_app.h"
#include "eeprom.h"

//static struct es_prog_ops *ofl_prog_intf;   //脱机编程接口
//static es_target_cfg ofl_target_device;     //目标芯片信息    
static ofl_prj_info_t ofl_prj_info;         //脱机方案信息
static ofl_serial_number_t sn_info;                //序列号信息

void ofl_prog_init(void)
{
    error_t ret;
    //获取方案信息
    online_file_read(OFL_PROG_INFO, 0,(uint8_t*) &ofl_prj_info, sizeof(ofl_prj_info_t)); 
    //获取脱机序列号
    fm24cxx_read(EE_SERIAL_NUMBER_ADDR,(uint8_t*) &sn_info, sizeof(ofl_serial_number_t) );    

    //脱机接口

    ret = es_program_init(ofl_prj_info.intf);                    
    if(ret != ERROR_SUCCESS)
    {
        //todo 接口类型错误
        while(1);
    }     
}


//判断编程模式
uint8_t ofl_in_prog_mode(void)
{
    static uint8_t check_time = 0;
    
    //机台启动
    if(OFL_SERIALNUM_PROG_START_MODE == sn_info.start_mode )    //自动检测
    {
        
        
    } 
    else
    {
        if(es_entry_prog_mode() != ERROR_SUCCESS)        //连续检测5次OK，间隔20ms ，表示进入检测到了芯片    
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

//下载脱机序列号

//根据脱机步骤进行脱机编程
uint8_t ofl_prog(void)
{
    error_t ret;
    uint8_t data[8];
    uint32_t i = 0;
    for(i = 0; i<ofl_prj_info.step; i++)
    {
        switch(ofl_prj_info.item[i])
        {
            case OFL_STEP_ERASE:
                ret = es_erase_chip(0x00);  //全擦
                break;
            case OFL_STEP_CHECK_EMPTY :
                ret = es_check_empty(data);
                break;
            case OFL_STEP_PROG:                   
                if(sn_info.state !=  OFL_SERIALNUM_DISABLE)
                {
                     serial_number_download((uint8_t*)&sn_info.sn_addr);
                      ret = es_program_flash(1,data);
                }                    
                else
                    ret = es_program_flash(0,data);
                break;
            case OFL_STEP_VERIFY:
                if(sn_info.state !=  OFL_SERIALNUM_DISABLE)
                    ret = es_program_verify(1, data);
                else
                    ret = es_program_verify(0, data);
                break;
            case OFL_STEP_ENCRYPT :
                ret = es_program_encrypt();
                break;
        }  
        if(ret != ERROR_SUCCESS)
            return FALSE;
    }
    //编程成功
    sn_info.success_count ++;       //烧录成功+1
    if(sn_info.state !=  OFL_SERIALNUM_DISABLE)
    {
        if(sn_info.sn_size == 4)
        {
             sn_info.sn_data[0]  += sn_info.sn_step; 
        }          
        else if(sn_info.sn_size == 8) 
        {
             sn_info.sn_data[1]  += sn_info.sn_step; 
        }          
    }  
    fm24cxx_write(EE_SERIAL_NUMBER_ADDR,(uint8_t*) &sn_info, sizeof(ofl_serial_number_t) );
    return TRUE;
}



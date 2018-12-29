#include "eslink.h"
#include "es_common.h" 
#include "offline_file.h" 
#include "isp_prog_intf.h"  
#include "settings_rom.h"
#include "sflash_port.h"
#include "offline_app.h"
#include "eeprom_port.h"
#include "menu.h"

static struct es_prog_ops *ofl_prog_intf;   //脱机编程接口
static es_target_cfg ofl_target_device;     //目标芯片信息    
static ofl_prj_info_t ofl_prj_info;         //脱机方案信息
static ofl_serial_number_t sn_info;                //序列号信息
//更新序列号
static error_t  update_serial_number_8bit(uint64_t sn_data, uint8_t *buf, uint8_t size);
static error_t  update_serial_number_32bit(uint64_t sn_data, uint8_t *buf, uint8_t size);
static error_t  get_serial_number_8bit(uint64_t *sn_data, uint8_t *buf, uint8_t size);
static error_t  get_serial_number_32bit(uint64_t *sn_data, uint8_t *buf, uint8_t size);

/*******************************************************************************
*	函 数 名: ofl_prog_init
*	功能说明: 脱机编程初始化
*	形    参: 无
*	返 回 值: 无
*******************************************************************************/
ofl_error_t ofl_prog_init(void)
{
    error_t ret = ERROR_SUCCESS;
    
    PORT_ISP_SETUP();
    ISP_SETUP();
    //获取方案信息
    online_file_read(OFL_PROG_INFO, 0,(uint8_t*) &ofl_prj_info, sizeof(ofl_prj_info_t)); 
    //获取脱机序列号
    get_offline_serial_number((uint8_t*) &sn_info, sizeof(ofl_serial_number_t) );
    
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
        return OFL_ERR_PROG_INTF;
    } 
    //获取目标芯片信息
	get_target_info((uint8_t*)&ofl_target_device); 
    //烧录接口初始化
	ofl_prog_intf->init(&ofl_target_device);    
    
    return  OFL_SUCCESS;
}

/*******************************************************************************
*	函 数 名: ofl_in_prog_mode
*	功能说明: 判断编程模式
*	形    参: 无
*	返 回 值: TRUE 进模式成功 FALSE 进模式失败
*******************************************************************************/
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
/*******************************************************************************
*	函 数 名: ofl_out_prog_mode
*	功能说明: 出编程模式判断
*	形    参: 无
*	返 回 值: TRUE 进模式成功 FALSE 进模式失败
*******************************************************************************/
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
ofl_error_t ofl_prog(void)
{
    error_t ret;
    uint32_t faildata,failaddr;
    uint32_t i = 0;
    if(sn_info.success_count >=  sn_info.total_size)
        return OFL_ERR_COUNT_FULL;           
        
    for(i = 0; i<ofl_prj_info.step; i++)
    {
        switch(ofl_prj_info.item[i])
        {
            case OFL_STEP_ERASE:
                ret = ofl_prog_intf->erase_chip(0);    //全擦                 
                if(ret != ERROR_SUCCESS)
                {                  
                    return  OFL_ERR_ERASE;
                }
                break;
            case OFL_STEP_CHECK_EMPTY :
                ret = ofl_prog_intf->check_empty(&failaddr, &faildata);
                if(ret != ERROR_SUCCESS)
                {                  
                    return  OFL_ERR_CHECK_EMPTY;
                }
                break;
            case OFL_STEP_PROG:                   
                if(sn_info.state !=  OFL_SERIALNUM_DISABLE)
                    ret = ofl_prog_intf->program_all(ENABLE, &sn_info.sn, &failaddr);                   
                else
                    ret = ofl_prog_intf->program_all(DISABLE, NULL, &failaddr);
                if(ret != ERROR_SUCCESS)
                {                  
                    return  OFL_ERR_PROG;
                }                   
                break;
            case OFL_STEP_VERIFY:
                if(sn_info.state !=  OFL_SERIALNUM_DISABLE)
                    ret = ofl_prog_intf->verify_all(ENABLE, &sn_info.sn, &failaddr , &faildata);
                else
                    ret = ofl_prog_intf->verify_all(DISABLE, NULL, &failaddr, &faildata);
                if(ret != ERROR_SUCCESS)
                {                  
                    return  OFL_ERR_VERIFY;
                }     
                break;
            case OFL_STEP_ENCRYPT :
                ret = ofl_prog_intf->encrypt_chip(); 
                 if(ret != ERROR_SUCCESS)
                {                  
                    return  OFL_ERR_ENCRYPT;
                }  
                break;
#if ESLINK_RTC_ENABLE   
            case OFL_STEP_RTC_CALI :
//                rtc_calibration_handler();
                break;
            case OFL_STEP_RTC_VERIFY :
                break;
#endif
            default:
                break;
                
        }  
    }
    //编程成功
    sn_info.success_count ++;       //烧录成功+1
    if(sn_info.state !=  OFL_SERIALNUM_DISABLE)
    {  
        uint64_t data = 0;
        if(sn_info.read_mode  == OFL_SERIALNUM_READ_USE_IAP)    //32位机 IAP方式
        {                     
            get_serial_number_32bit( &data, sn_info.sn.data, sn_info.sn.size);              
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
//    fm24cxx_write(EE_SERIAL_NUMBER_ADDR, );
    //更新序列号
    set_offline_serial_number((uint8_t*) &sn_info, sizeof(ofl_serial_number_t));
    return OFL_SUCCESS;
}



//获取8位芯片的序列号
static error_t  get_serial_number_8bit(uint64_t *sn_data, uint8_t *buf, uint8_t size)
{
    uint8_t i;
    
    if(size & 0x01)
        return  ERROR_OUT_OF_BOUNDS;
    size = size /2;
    if((size <=0) || (size > 8) )
         return  ERROR_OUT_OF_BOUNDS;
    
    for(i=0; i<size; i++)
    {
        *sn_data |= ((uint64_t)buf[i*2] & 0xff) << (8 * (size-1-i));    
    }
    return  ERROR_SUCCESS;
}
//更新序列号
static error_t  update_serial_number_8bit(uint64_t sn_data, uint8_t *buf, uint8_t size)
{
    uint8_t i;
    
    if(size & 0x01)
        return  ERROR_OUT_OF_BOUNDS;
    size = size /2;
    if((size <=0) || (size > 8) )
         return  ERROR_OUT_OF_BOUNDS;
    
    for(i=0; i<size; i++)
    {
        buf[i*2] = (sn_data >> (8 * (size-1-i))) & 0XFF;    
    }  
   
    return  ERROR_SUCCESS;   
}
//获取32位芯片的序列号
static error_t  get_serial_number_32bit(uint64_t *sn_data, uint8_t *buf, uint8_t size)
{
    uint8_t i;
    
    if((size <=0) || (size > 8) )
         return  ERROR_OUT_OF_BOUNDS;      
    
    if(size == 0x04)
    {
        for(i=0; i<4; i++)
            *sn_data |= ((uint64_t)buf[i] & 0xff) << (8 * i);    
    }
    else if(size == 0x08)
    {
        for(i=0; i<4; i++)
            *sn_data |= ((uint64_t)buf[i] & 0xff) << (8 * (i+4));  
        for(i=4; i<8; i++)
            *sn_data |= ((uint64_t)buf[i] & 0xff) << (8 * (i-4)); 
    }
    return  ERROR_SUCCESS;
}
//更新32位芯片序列号
static error_t  update_serial_number_32bit(uint64_t sn_data, uint8_t *buf, uint8_t size)
{
    uint8_t i;
    
    if((size <=0) || (size > 8) )
         return  ERROR_OUT_OF_BOUNDS;     
    if(size == 0x04)
    {
        for(i=0; i<4; i++)
            buf[i] = (sn_data >> (8 * i)) & 0XFF;     
    }
    else if(size == 0x08)
    {   
        for(i=0; i<4; i++)
            buf[i] = (sn_data >> (8 * (i+4))) & 0XFF;
        for(i=4; i<8; i++)
            buf[i] = (sn_data >> (8 * (i-4))) & 0XFF; 
    }
   
    return  ERROR_SUCCESS;   
}


//退出脱机模式时，更新脱机序列号信息
ofl_error_t update_ofl_serial_number(void)
{  
    error_t ret = ERROR_SUCCESS;
    char path[16+1] = {'\0'};
    partition_t part; 
    
    if(sn_info.state ==  OFL_SERIALNUM_DISABLE)
        return  OFL_SUCCESS;
    
    //读文件名
    get_offline_proget_name((uint8_t*)path, 16);  
//    fm24cxx_read(EE_OFL_PRJ_NAME, (uint8_t*)path, 16 );   
    //读脱机序列号分区信息 
    get_offline_partition((uint8_t*)&part, sizeof(partition_t));    
//    fm24cxx_read(EE_OFL_SERIAL_NUMBER_PARTITION, (uint8_t*)&part, sizeof(partition_t) );      
    //读脱机序列号    
//    fm24cxx_read(EE_SERIAL_NUMBER_ADDR,(uint8_t*) &sn_info, sizeof(ofl_serial_number_t) );
    get_offline_serial_number((uint8_t*) &sn_info, sizeof(ofl_serial_number_t) );
    ret = ofl_file_lseek_write( path, part.start, (uint8_t*) &sn_info, part.size );
    
    return OFL_SUCCESS ;

}



#include "eslink.h"
#include "errno.h"
#include "isp_prog_intf.h"
#include "sflash_port.h"
#include "isp_port.h" 
#include "./ES_ISP.h"
#include "./target_info.h"

#define ISP_WRITE_MAX_SIZE  1024   
#define ISP_READ_MAX_SIZE   512  

static void isp_init(es_target_cfg *target);
static error_t isp_prog_init(void); //进模式
static error_t isp_prog_uninit(void); //退出模式

static error_t isp_prog_erase_chip (uint8_t para);
static error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t isp_prog_read_chipid(uint8_t *buf);
static error_t isp_prog_read_checksum(uint8_t *buf);
static error_t isp_prog_encrypt_chip(void);
static error_t isp_chipid_check(void);
static error_t isp_prog_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_addr );
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *data, uint32_t size);
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t isp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t isp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_target_program_config_all(uint32_t *failed_addr);
static error_t isp_target_program_all(  uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr);
static error_t isp_target_verify_all( uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data);

struct  es_prog_ops isp_prog_intf = {
    isp_init,
    isp_prog_init,
    isp_prog_uninit,
    isp_prog_erase_chip,
    isp_prog_check_empty,
    isp_prog_read_chipid,
//    isp_chipid_check,
    isp_prog_read_checksum,
    isp_prog_encrypt_chip,

    isp_prog_program_config,
    isp_prog_read_config,
    isp_prog_verify_config,
    isp_prog_program_flash,
    isp_prog_read_flash,
    isp_prog_verify_flash,
    isp_target_program_config_all ,
    isp_target_program_all,
    isp_target_verify_all,
    0,
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
    if(isp_unlock_check() != TRUE)
        return ERROR_IN_ISP_MODE;
    
    return ERROR_SUCCESS; 
}

//退出isp模式
static error_t isp_out_mode(void)
{      

    return ERROR_SUCCESS; 
}

////判断是否检测到芯片 
//error_t isp_chip_check(void)
//{

//        return ERROR_IN_ISP_MODE;
//    return ERROR_SUCCESS; 
//}
//编程初始化，进模式
static error_t isp_prog_init(void)
{
     error_t status;
     
    if(isp_mode_check() != TRUE)    //判断是否在isp模式
    {
        PORT_ISP_SETUP();
        eslink_set_target_hold_reset(20);  
        es_delay_ms(3);     //等待开机流程完毕
        status = isp_entry_mode();
        if(ERROR_SUCCESS != status)
            return status; 
    }
    return  ERROR_SUCCESS;
}
//isp退出编程模式
static error_t isp_prog_uninit(void)
{
    isp_out_mode();
    if(isp_mode_check() != TRUE)
        return  ERROR_SUCCESS;
//        isp_set_target_reset(0);
//        PORT_ISP_OFF();  
    return ERROR_OUT_ISP_MODE;
}   

//读目标芯片ID
static error_t isp_prog_read_chipid(uint8_t *buf)
{  
    if(isp_read_config(isp_target_dev->chipid_addr, (uint8_t*)buf, 4) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS; 
}
 
//判断芯片chipid是否正确  
static error_t isp_chipid_check(void)
{
    uint32_t chipid = 0;
   
    if(isp_read_config(isp_target_dev->chipid_addr, (uint8_t*)&chipid, 4) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    if(chipid != isp_target_dev->chipid_value)    
    {
          //测试模式，不判断ID
//         return  ERROR_CHIP_ID_NOT_MATCH;
    }
        
    return ERROR_SUCCESS; 

}
//读芯片校验和
static error_t isp_prog_read_checksum(uint8_t *buf)
{
    error_t ret = ERROR_SUCCESS; 
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
   
    if(isp_read_config(CHIP_CHECKSUM_ADDR, buf, 4) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS;     
}
//flash编程
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr)
{
    uint8_t ret ; 
    uint32_t offset;
        
    ret = isp_program_code( addr, data, size,&offset);
    if(ret != TRUE)
    {
        if(failed_addr)
            *failed_addr = addr + offset ; 
        return ERROR_ISP_PROG;
    }
    return ERROR_SUCCESS;     
}
//读flash
static error_t isp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size)
{
    error_t ret = ERROR_SUCCESS;  
    uint8_t rd_buf[ISP_READ_MAX_SIZE];
    uint32_t rd_size;
   
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
   
    while (size > 0) 
    {
        rd_size = MIN(size, sizeof(rd_buf));
        if(isp_read_code(addr, data, rd_size) != TRUE)
            return ERROR_ISP_READ;
        addr += rd_size;
        size -= rd_size;
        data += rd_size;
    }      
    return ERROR_SUCCESS;
}
//flash校验
static error_t isp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
    uint8_t ret ; 
    uint32_t i;
    uint8_t rd_buf[ISP_READ_MAX_SIZE];
    uint32_t verify_size;
    
    while (size > 0) 
    {          
        verify_size = MIN(size, sizeof(rd_buf));
        ret = isp_read_code(addr, rd_buf, verify_size); 
        if( ret != TRUE)
            return ERROR_ISP_READ_CFG_WORD;
        for(i=0; i< verify_size; i++)
        {
            if( data[i] != rd_buf[i]) 
            {
                if(failed_addr)
                    *failed_addr = addr + i ; 
                if(failed_data)
                    *failed_data = rd_buf[i];
                return  ERROR_ISP_VERIFY;  
            } 
        } 
        addr += verify_size;
        size -= verify_size;
        data += verify_size;
    }
    
    return ERROR_SUCCESS;      
}
/*
 * 配置字编程
 * 注意：配置字默认从地址0开始编程，且能一次编程结束
 */
static error_t isp_prog_program_config(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr )
{   
    uint8_t ret ; 
    uint32_t offset;    
    uint32_t prog_addr;
    uint32_t prog_size;  
//    uint8_t part;    
//    uint32_t info_addr[CHIP_INFO_PART_SIZE] = { CHIP_INFO_PART1_ADDR,CHIP_INFO_PART2_ADDR,CHIP_INFO_PART3_ADDR};                                                                                                                                   
//    uint32_t info_size[CHIP_INFO_PART_SIZE] = { CHIP_INFO_PART1_SIZE,CHIP_INFO_PART2_SIZE,CHIP_INFO_PART3_SIZE};
        
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS; 
    if(size != CHIP_INFO_SIZE)
        return ERROR_ISP_PROG_CFG_WORD;
    
//    prog_addr = CHIP_INFO_PART1_ADDR;     
    prog_size = CHIP_INFO_PART1_SIZE;     
//    ret = isp_program_config(prog_addr, data, prog_size, &offset);
//    if(ret != TRUE)
//    {
//        if(failed_addr)
//            *failed_addr = prog_addr + offset*4 ; 
//        return ERROR_ISP_PROG_CFG_WORD;
//    }
    data += prog_size;
    //lv1加密字
    data +=  CHIP_INFO_PART2_SIZE ;
    
    prog_addr = CHIP_INFO_PART3_ADDR;     
    prog_size = CHIP_INFO_PART3_SIZE;     
    ret = isp_program_config(prog_addr, data, prog_size, &offset);
    if(ret != TRUE)
    {
        if(failed_addr)
            *failed_addr = prog_addr + offset*4 ; 
        return ERROR_ISP_PROG_CFG_WORD;
    } 
    data += prog_size;
    
    prog_addr = CHIP_INFO_PART4_ADDR;     
    prog_size = CHIP_INFO_PART4_SIZE;     
    ret = isp_program_config(prog_addr, data, prog_size, &offset);
    if(ret != TRUE)
    {
        if(failed_addr)
            *failed_addr = prog_addr + offset*4 ; 
        return ERROR_ISP_PROG_CFG_WORD;
    } 
    data += prog_size;
    
    return ERROR_SUCCESS;          
}

//读配置字  
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *data, uint32_t size)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t part;
    uint32_t read_addr;
    uint32_t read_size;
    uint32_t info_addr[CHIP_INFO_PART_SIZE] = { CHIP_INFO_PART1_ADDR,CHIP_INFO_PART2_ADDR,
                                                CHIP_INFO_PART3_ADDR,CHIP_INFO_PART4_ADDR};                                                                                                                                   
    uint32_t info_size[CHIP_INFO_PART_SIZE] = { CHIP_INFO_PART1_SIZE,CHIP_INFO_PART2_SIZE,
                                                CHIP_INFO_PART3_SIZE,CHIP_INFO_PART4_SIZE};
     
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS; 
    if(size != CHIP_INFO_SIZE)
        return ERROR_ISP_PROG_CFG_WORD;
        
    for(part=0; part<CHIP_INFO_PART_SIZE; part++)
    {
        read_addr = info_addr[part];     
        read_size = info_size[part];     
        if(isp_read_config(read_addr, data, read_size) != TRUE)
            return ERROR_ISP_READ_CFG_WORD;      
        data += read_size;
    }
    return ERROR_SUCCESS;

     
}
//配置字校验  
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size,uint32_t *failed_addr, uint32_t *failed_data)
{
    uint8_t ret ; 
    uint32_t i;
    uint8_t rd_buf[ISP_READ_MAX_SIZE]; 
//    uint8_t part;
//    uint32_t info_addr[CHIP_INFO_PART_SIZE] = { CHIP_INFO_PART1_ADDR,CHIP_INFO_PART2_ADDR,CHIP_INFO_PART3_ADDR};                                                                                                                                   
//    uint32_t info_size[CHIP_INFO_PART_SIZE] = { CHIP_INFO_PART1_SIZE,CHIP_INFO_PART2_SIZE,CHIP_INFO_PART3_SIZE};
    uint32_t read_addr;
    uint32_t read_size;     
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS; 

//    read_addr = CHIP_INFO_PART1_ADDR;     
    read_size = CHIP_INFO_PART1_SIZE;  
//    ret = isp_read_config(read_addr, rd_buf, read_size); 
//    if( ret != TRUE)
//        return ERROR_ISP_READ_CFG_WORD;
//    for(i=0; i< read_size; i++)
//    {
//        if(data[i] != rd_buf[i])                          
//        {
//            if(failed_addr)
//                *failed_addr = read_addr + i ;  
//            if(failed_data)
//                *failed_data = rd_buf[i];
//            return  ERROR_ISP_CFG_WORD_VERIFY;  
//        } 
//    }  
    data += read_size;         

    data += CHIP_INFO_PART2_SIZE;
    
    read_addr = CHIP_INFO_PART3_ADDR;     
    read_size = CHIP_INFO_PART3_SIZE; 
    ret = isp_read_config(read_addr, rd_buf, read_size); 
    if( ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD;    
    for(i=0; i< read_size; i++)
    {
        if(data[i] != rd_buf[i])                          
        {
            if(failed_addr)
                *failed_addr = read_addr + i ;  
            if(failed_data)
                *failed_data = rd_buf[i];
            return  ERROR_ISP_CFG_WORD_VERIFY;  
        } 
    }  
    data += read_size;

    read_addr = CHIP_INFO_PART4_ADDR;     
    read_size = CHIP_INFO_PART4_SIZE;  
    ret = isp_read_config(read_addr, rd_buf, read_size); 
    if( ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    for(i=0; i< read_size; i++)
    {
        if(data[i] != rd_buf[i])                          
        {
            if(failed_addr)
                *failed_addr = read_addr + i ;  
            if(failed_data)
                *failed_data = rd_buf[i];
            return  ERROR_ISP_CFG_WORD_VERIFY;  
        } 
    }  
    
    return ERROR_SUCCESS;  
}

// 芯片加密 
static error_t isp_prog_encrypt_chip(void)
{ 
    error_t ret;
    uint8_t result;
    static uint8_t lv1_reg[8] = {0x00};     //保存lv1加密字
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != result)
        return ret; 
    ret = isp_prog_intf.cb(CFG_WORD, CHIP_INFO_PART1_SIZE, lv1_reg , sizeof(lv1_reg));
        if( ret !=  ERROR_SUCCESS)
            return ret; 
    result = isp_program_config(CHIP_LV1_ADDR, lv1_reg, sizeof(lv1_reg), NULL);
    if(result != TRUE)
        return ERROR_ISP_ENCRYPT;        
    return ERROR_SUCCESS;    
}

//擦除
static error_t isp_prog_erase_chip (uint8_t  para) 
{
    error_t ret = ERROR_SUCCESS;
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret;  
        
    if(isp_erase_code(isp_target_dev->code_start, isp_target_dev->code_size) != TRUE)
		return ERROR_ISP_ERASE;   

    //擦除info7
    if(isp_erase_info7() != TRUE)
		return ERROR_ISP_ERASE;
         
    return ERROR_SUCCESS;
}


/*******************************************************************************
*	函 数 名: isp_prog_check_empty
*	功能说明: 查空
*	形    参: failed_addr：错误地址  failed_data：错误数据
*	返 回 值: 错误类型
*******************************************************************************/
error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failed_data)           
{
    error_t ret = ERROR_SUCCESS;
    uint32_t i;
	uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	
    
    uint8_t read_buf[ISP_READ_MAX_SIZE]; 
	uint32_t copy_size; 
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
	code_addr =  isp_target_dev->code_start;
	code_size =  isp_target_dev->code_size ; //64K bytes
	while(true)
	{
		copy_size = MIN(code_size, sizeof(read_buf) );      
	    isp_read_code(code_addr, read_buf, copy_size);
		for(i = 0; i<copy_size; i++)
		{
			if(read_buf[i] != 0xFF)
			{    
                if(failed_addr)
                    *failed_addr = code_addr + i  ;
                if( failed_data)
                    *failed_data = read_buf[i] ; 
				return ERROR_ISP_FLASH_CHECK_EMPTY;
			} 				
		} 
        // Update variables
        code_addr  += copy_size*4;
        code_size  -= copy_size*4;
        
        // Check for end
        if (code_size <= 0) {
            break;
        } 
	}  

    //配置字查空
	cfg_word_addr = CHIP_INFO_PART2_ADDR;
	cfg_word_size = CHIP_INFO_PART2_SIZE + CHIP_INFO_PART3_SIZE;     //字长度

    isp_read_config(cfg_word_addr, read_buf, cfg_word_size);
    for(i = 0; i<cfg_word_size; i++)
    {
        if(read_buf[i] != 0xFF)
        {    
            if(failed_addr)
                *failed_addr = cfg_word_addr + i  ;
            if(failed_data)
                *failed_data = read_buf[i] ; 
            return ERROR_ISP_FLASH_CHECK_EMPTY;
        } 				
    } 

	cfg_word_addr = CHIP_INFO_PART4_ADDR;
	cfg_word_size = CHIP_INFO_PART4_SIZE ;     //字长度

    isp_read_config(cfg_word_addr, read_buf, cfg_word_size);
    for(i = 0; i<cfg_word_size; i++)
    {
        if(read_buf[i] != 0xFF)
        {    
            if(failed_addr)
                *failed_addr = cfg_word_addr + i  ;
            if(failed_data)
                *failed_data = read_buf[i] ; 
            return ERROR_ISP_FLASH_CHECK_EMPTY;
        } 				
    } 

    return ERROR_SUCCESS;     
}  

/*******************************************************************************
*	函 数 名: isp_target_program_config_all
*	功能说明: 芯片配置字编程。
*	形    参: failed_addr：错误地址  
*	返 回 值: 编程错误地址
*******************************************************************************/
static error_t isp_target_program_config_all(uint32_t *failed_addr)
{
    error_t ret = ERROR_SUCCESS;       
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	 
    
    uint32_t copy_size;
    uint32_t read_addr;
    uint8_t read_buf[ISP_WRITE_MAX_SIZE] = {0x00};
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    cfg_word_addr =  isp_target_dev->config_word_start;
	cfg_word_size =  isp_target_dev->config_word_size;
    read_addr =  0;

    while(true)
    {
        copy_size = MIN(cfg_word_size, sizeof(read_buf) );
        
        ret = isp_prog_intf.cb(CFG_WORD, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret;     
        ret = isp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
        if(ERROR_SUCCESS != ret)
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
*	函 数 名: isp_target_program_all
*	功能说明: 芯片编程。flash和配置字编程
*	形    参: sn_enable：是否已编程序列号 sn：序列号代码 
*             failed_addr：错误地址   failed_data ：错误数据
*	返 回 值: 错误类型
*******************************************************************************/
static error_t isp_target_program_all(uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr) 
{
    error_t ret = ERROR_SUCCESS;
    uint32_t i;
    
    uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	   
    
    uint32_t copy_size;      
    uint32_t read_addr;
    uint8_t read_buf[ISP_WRITE_MAX_SIZE];
       
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    code_addr =  isp_target_dev->code_start;
	code_size =  isp_target_dev->code_size;
    read_addr =  0; 
        
    while(true)
    {
        copy_size = MIN(code_size, sizeof(read_buf) ); 
        ret = isp_prog_intf.cb(USER_HEX, read_addr, read_buf , copy_size);
        
        if(ERROR_SUCCESS != ret)
            return ret;
        if(sn_enable == ENABLE)     //序列号代码使能
            serial_number_intercept_write(sn ,code_addr, read_buf, copy_size);	//填入序列号
        for(i=0; i<copy_size; i++)
        {
            if(read_buf[i] != 0xFF)
                break;
        }
        if(i < copy_size)      //数据段都为0xFF,不进行编程
        {
            ret = isp_prog_program_flash(code_addr, read_buf, copy_size, failed_addr); 
            if( ret !=  ERROR_SUCCESS)   //编程失败，返回编程失败地址
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
    while(true)
    {
        copy_size = MIN(cfg_word_size, sizeof(read_buf) );          
        ret = isp_prog_intf.cb(CFG_WORD, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret;     		
        ret = isp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
        if( ret !=  ERROR_SUCCESS)
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
*	函 数 名: isp_target_verify_all
*	功能说明: 芯片验证
*	形    参: sn_enable：是否已编程序列号 sn：序列号代码 
*             failed_addr：错误地址   
*	返 回 值: 错误类型
*******************************************************************************/
static error_t  isp_target_verify_all( uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data)
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
    uint8_t sf_buf[ISP_WRITE_MAX_SIZE];     
  
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    code_addr =  isp_target_dev->code_start;
	code_size =  isp_target_dev->code_size;
    sf_addr = 0;
    while(true)
    {
        verify_size = MIN(code_size, sizeof(sf_buf) );
        ret = isp_prog_intf.cb(USER_HEX, sf_addr, sf_buf , verify_size);   
        if( ret !=  ERROR_SUCCESS)
            return ret; 
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        if( sn_enable == ENABLE)
        {              
            serial_number_intercept_write(sn, code_addr, sf_buf, verify_size);	//填入序列号 
        }                       
        ret = isp_prog_verify_flash(code_addr, sf_buf, verify_size,failed_addr,failed_data);                        
        if( ret !=  ERROR_SUCCESS)
            return ret; 
        // Update variables
        code_addr  += verify_size;
        code_size  -= verify_size;
        sf_addr += verify_size;
        // Check for end
        if (code_size <= 0) 
            break;       
    }  
    isp_prog_intf.cb(HEX_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if((sf_checksum&0x0000ffff) != (checksum&0x0000ffff))
    {
        ret = ERROR_USER_HEX_CHECKSUM;
        return  ret;
    }         
    
    cfg_word_addr =  isp_target_dev->config_word_start;
	cfg_word_size =  isp_target_dev->config_word_size;
    sf_addr =  0;
    checksum = 0;
    while(true)
    {
        verify_size = MIN(cfg_word_size, sizeof(sf_buf) );          
        ret = isp_prog_intf.cb(CFG_WORD, sf_addr, sf_buf , verify_size);
        if( ret !=  ERROR_SUCCESS)
            return ret; 
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        
        ret = isp_prog_verify_config(cfg_word_addr, sf_buf, verify_size,failed_addr,failed_data); 
        if( ret !=  ERROR_SUCCESS)
            return ret; 
        // Update variables
        cfg_word_addr  += verify_size;
        cfg_word_size  -= verify_size;
        sf_addr += verify_size;
        // Check for end
        if (cfg_word_size <= 0) 
            break;       
    }
    isp_prog_intf.cb(CFG_WORD_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if(sf_checksum != (checksum&0x0000ffff))
    {
        ret = ERROR_CFG_WORD_CHECKSUM;
        return  ret; 
    }           
    return  ret; 
}







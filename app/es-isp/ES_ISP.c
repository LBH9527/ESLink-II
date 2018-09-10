#include "ES_ISP.h"    
#include "error.h"
#include "isp_m939.h"





isp_serial_number_t isp_serial_number ;

isp_process_error_t isp_process_err;

/* Dummy Weak Functions that need to be provided by user */
//__weak error_t isp_read_chipid(uint32_t addr, uint32_t *buf)
//{
//    return ERROR_SUCCESS;
//}
//__weak error_t isp_erase_chip (uint8_t eraseMode) 
//{
//    return ERROR_SUCCESS;
//}
//__weak error_t isp_check_empty(uint32_t *notempty_addr, uint32_t *notempty_data)
//{
//    return ERROR_SUCCESS;
//}
//__weak error_t isp_programe_config(uint32_t *buf)
//{
//    return ERROR_SUCCESS;
//}
//__weak error_t isp_read_config(uint32_t addr, uint32_t size, uint32_t *buf)
//{
//    return ERROR_SUCCESS;
//}

//todo： option_read 等函数都用指针函数实现，加芯片时，值需要在对应的时序文件中修改。
/*
*    读芯片ID。验证时addr可以根据IDE_Spac设固定值，方便自己测试
*/
error_t isp_read_chipid(uint32_t addr,uint32_t *buf)
{    
    error_t result = ERROR_SUCCESS;
    	
    option_read(addr, 1, buf);  

    return result;   
}

/*  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed 
 */

error_t isp_erase_chip (uint8_t mode) 
{
    uint8_t status = ERROR_SUCCESS;
       
    if(mode == 0)              //全擦
        status = erase_all(0x00);
    else                            //页擦除
        status = erase_all(0x01); 

    return (error_t)status;
}
/*
 * 查空
 */
error_t isp_check_empty(void)
{
    error_t result = ERROR_SUCCESS;    
      
    result = empty_check();
    
    return result;      
}
/*
 *配置字编程
 */
error_t isp_programe_config(uint32_t addr, const uint8_t *data, uint32_t size)
{
    union B32_B08 buf[256];
    error_t result = ERROR_SUCCESS;    
        
    //读SPI Flash中的配置字数据  

//    es_ext_flash_read(SF_CONFIG_INFO_ADDR, 1024, buf[0].B08);
	        
//    result = config_programe(&(buf[0].B32));
    
    return result;        
}

/*
 *读配置字
 */
error_t isp_read_config(uint32_t addr, uint32_t size, uint32_t *buf)
{
    error_t status = ERROR_SUCCESS;
        
    if(config_read_all(addr, size, buf) != TRUE)
        status = PRGCONFIGFAIL;                 //配置字编程失败
    
    return status;        
}
#define ISP_PRG_MINI_SIZE       1024        //1K
//isp编程
error_t isp_prg_program(uint32_t addr, const uint8_t *data, uint32_t size)
{
//    uint32_t addr = start_addr;
//    uint32_t size = image_size;
    uint32_t write_size = 0;
    union B32_B08 falsh_buf[ISP_PRG_MINI_SIZE/4];   //烧写数据长度 1K字节（自定义）
    error_t status = ERROR_SUCCESS;
    
    while(size > 0){     
        write_size = MIN(size, sizeof(falsh_buf));

        status = flash_full_programme( addr, &(falsh_buf[0].B32), write_size/4);
        if(status != ERROR_SUCCESS)
            return status;        
                   
        addr += write_size;
        size -= write_size; 
    }        

    return status;     
}

/*
 * 芯片验证
 *  比较spi flash中的用户hex和目标芯片中的数据
 */

error_t isp_verify(uint32_t start_addr, uint32_t image_size) 
{
    uint8_t status = ERROR_SUCCESS;
    uint32_t i;
    uint32_t size = image_size;
    uint32_t addr = start_addr;
    union B32_B08 sf_checksum;
    uint32_t checksum = 0;
    uint32_t write_size = 0;
    union B32_B08 flash_buf[256];           //数组长度 4的倍数
    
    //读目标芯片中的数据，与spi flash中的数据比较是否一致，不一致返回错误地址和错误值。
    //如果数据全相同，在比较校验和与上位机下发的校验和是否一致。
    
    while(size > 0){     
        write_size = MIN(size, sizeof(flash_buf));
//        if(es_ext_flash_read(addr, write_size, flash_buf[0].B08) != SF_SUCCESS)
            return UNCLOCKFAIL;
        status = code_area_verify( addr, &(flash_buf[0].B32), write_size/4 ) ;
        if(status != ERROR_SUCCESS)
            return status;
             
        for(i=0; i<write_size; i++){
            checksum += flash_buf[i].B08[0];
            checksum += flash_buf[i].B08[1];
            checksum += flash_buf[i].B08[2];
            checksum += flash_buf[i].B08[3];
        }
        addr += write_size;
        size -= write_size; 
    }   
    //读spi flash中的用户hex数据计算校验和，与保存在spi flash 中 SF_HEX_CHECKSUM_ADDR 
    //确认spi flash中的数据正确。
//    es_ext_flash_read(SF_HEX_CHECKSUM_ADDR, 4, sf_checksum.B08);	
    if((sf_checksum.B32 & 0x0000ffff) != (checksum & 0x0000ffff)){
		return ERROR_HEX_CHECKSUM_FAIL;			
	}
       	
    //读出spi flash中的芯片配置信息数据
//    es_ext_flash_read(SF_CONFIG_INFO_ADDR, 1024, flash_buf[0].B08);
    info_area_verify(&(flash_buf[0].B32));
    return status;      
}

/*
 * 芯片加密
 */
error_t isp_encrypt_chip(void)
{
    error_t status = ERROR_SUCCESS;
    union B32_B08 buf[32]; 
    
//    es_ext_flash_read((SF_CONFIG_INFO_ADDR+112), 32, buf[0].B08);
    status = encrypt_chip(&(buf[0].B32));
    
    return status;    
}







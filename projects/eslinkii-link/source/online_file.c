#include "es_common.h"
#include "errno.h"
#include "online_file.h"
#include "spi_flash.h"

/*******************************************************************************
*	函 数 名: online_file_erase
*	功能说明: 擦除
*	形    参: type：擦除数据类型   size：擦除长度
*	返 回 值: 无
*******************************************************************************/
error_t online_file_erase(uint8_t type, uint32_t size )
{
    uint32_t erase_addr;
    uint32_t erase_size;
    
    if(OL_HEX_PART == type)
        erase_addr = SF_USER_HEX_ADDR ;
    else if(OL_CONFIG_PART == type)  
        erase_addr = SF_CONFIG_WORD_ADDR ;
    erase_size = ES_ROUND_UP(size, 4096);
    if(sf_erase_sector(erase_addr , erase_size)  != 0)
        return ERROR_SPI_FLASH_ERASE; 
    return ERROR_SUCCESS;
}

error_t online_file_write(uint8_t type, uint32_t addr, const uint8_t *buf, uint32_t size )
{
    uint32_t write_addr;
    
    if(OL_HEX_PART == type)
        write_addr = SF_USER_HEX_ADDR + addr;
    else if(OL_CONFIG_PART == type)  
        write_addr = SF_CONFIG_WORD_ADDR + addr;

    if(spi_flash_write(write_addr, buf, size) != 0)
       return ERROR_SPI_FLASH_WRITE;
//    if(sf_CmpData(write_addr, buf, size) != 0)
//        return FALSE;
   return ERROR_SUCCESS;
}
error_t online_file_read(uint8_t type, uint32_t addr, uint8_t *buf, uint32_t size )
{
    uint32_t read_addr;
    
    if(OL_HEX_PART == type)
        read_addr = SF_USER_HEX_ADDR + addr;
    else if(OL_CONFIG_PART == type)  
        read_addr = SF_CONFIG_WORD_ADDR + addr;
    
    if(spi_flash_read(read_addr, buf, size) != 0)
        return ERROR_SPI_FLASH_READ;
    return ERROR_SUCCESS;  
}

void online_prmt_init(void)
{


}
/*******************************************************************************
*	函 数 名: online_save_prmt
*	功能说明: 保存参数
*	形    参: type：参数类型  data 参数值
*	返 回 值: 无
*******************************************************************************/
error_t online_save_prmt(oline_prmt_type_t type, uint32_t size)
{
//      if(type == CFG_WORD_CHECKSUM)


}



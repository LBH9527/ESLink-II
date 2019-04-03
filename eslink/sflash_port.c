#include "es_common.h"
#include "errno.h"
#include "sflash_port.h"
#include "spi_flash.h"

bool config_spi_flash_verify_program()
{
    return false;
}

static error_t get_sflash_addr(uint8_t type, uint32_t *addr)
{
    switch(type)
    {
        case USER_HEX:
            *addr = SF_USER_HEX_ADDR ;
        break;
        case CFG_WORD:
            *addr = SF_CONFIG_WORD_ADDR ;
        break;
        case HEX_CHECKSUM:
            *addr = SF_USER_HEX_CHECKSUM_ADDR ;
        break;
        case CFG_WORD_CHECKSUM:  
            *addr = SF_CONFIG_WORD_CHECKSUM_ADDR;
        break;
        case OFL_PROG_INFO:
            *addr = SF_OFFLINE_INFO_ADDR;
        break;
        case RTC_HEX:
            *addr = SF_RTC_HEX_ADDR ;
            break;
        case RTC_HEX_CHECKSUM:
            *addr = SF_RTC_HEX_CHECKSUM_ADDR;
            break;
        default:
            return ERROR_OUT_OF_BOUNDS;
//        break;   
    }
    return ERROR_SUCCESS;
}
/*******************************************************************************
*  函 数 名: online_file_erase
*  功能说明: 擦除
*  形    参: type：擦除数据类型   size：擦除长度
*  返 回 值: 无
*******************************************************************************/
error_t online_file_erase(uint8_t type, uint32_t size )
{
    error_t ret = ERROR_SUCCESS;
    uint32_t erase_addr;
    uint32_t erase_size;
    
    ret = get_sflash_addr( type, &erase_addr);
    if(ret != ERROR_SUCCESS)
        return ret;
        
    if(size < 0x4000)       //小于16K
    {
        erase_size = ROUND_UP(size, 0x1000);
        if(sf_erase_sector(erase_addr , erase_size)  != SF_SUCCESS)
            return ERROR_SPI_FLASH_ERASE;      
    }
    else
    {
        erase_size = ROUND_UP(size, 0x10000);
        if(sf_erase_block_64K(erase_addr , erase_size)  != SF_SUCCESS)
            return ERROR_SPI_FLASH_ERASE;     
    }    
    return ERROR_SUCCESS;
}

/*******************************************************************************
*  函 数 名: online_file_write
*  功能说明: spi 写
*  形    参: type：数据类型   addr：写入地址 buf：数据 size：长度
*  返 回 值: 无
*******************************************************************************/
error_t online_file_write(uint8_t type, uint32_t addr, const uint8_t *buf, uint32_t size )
{
    error_t ret = ERROR_SUCCESS;
    uint32_t addr_offset;
    
    ret = get_sflash_addr( type, &addr_offset);
    if(ret != ERROR_SUCCESS)
        return ret;
    addr +=  addr_offset;

    if(spi_flash_write(addr, buf, size) != SF_SUCCESS)
       return ERROR_SPI_FLASH_WRITE;
   if(config_spi_flash_verify_program())
   {
        if(sf_cmp_data(addr, buf, size) != SF_SUCCESS)    //比较写入的数据
            return ERROR_SPI_FLASH_WRITE;        
   }

   return ERROR_SUCCESS;
}
error_t online_file_read(uint8_t type, uint32_t addr, uint8_t *buf, uint32_t size )
{
     error_t ret = ERROR_SUCCESS;
    uint32_t addr_offset;
    
    ret = get_sflash_addr( type, &addr_offset);
    if(ret != ERROR_SUCCESS)
        return ret;
        
    addr +=  addr_offset;
    if(spi_flash_read(addr, buf, size) != SF_SUCCESS)
        return ERROR_SPI_FLASH_READ;
    return ERROR_SUCCESS;  
}





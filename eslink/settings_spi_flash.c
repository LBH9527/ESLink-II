#include "es_common.h"
#include "errno.h"
#include "settings_spi_flash.h"
#include "spi_flash.h"

bool config_spi_flash_verify_program()
{
    return false;
}

/*******************************************************************************
*	函 数 名: online_file_erase
*	功能说明: 擦除
*	形    参: type：擦除数据类型   size：擦除长度
*	返 回 值: 无
*******************************************************************************/
error_t online_file_erase(oline_data_type_t type, uint32_t size )
{
    uint32_t erase_addr;
    uint32_t erase_size;
    
    switch(type)
    {
        case USER_HEX:
            erase_addr = SF_USER_HEX_ADDR ;
        break;
        case CFG_WORD:
            erase_addr = SF_CONFIG_WORD_ADDR ;
        break;
        case HEX_CHECKSUM:
            erase_addr = SF_USER_HEX_CHECKSUM_ADDR ;
        break;
        case CFG_WORD_CHECKSUM:  
            erase_addr = SF_CONFIG_WORD_CHECKSUM_ADDR;
        break;
        case OFL_PROG_INFO:
            erase_addr = SF_OFFLINE_INFO;
        break;
        default:
            return ERROR_OUT_OF_BOUNDS;
//        break;   
    }
    if(size < 0x4000)       //小于16K
    {
        erase_size = ES_ROUND_UP(size, 0x1000);
        if(sf_erase_sector(erase_addr , erase_size)  != 0)
            return ERROR_SPI_FLASH_ERASE;      
    }
    else
    {
        erase_size = ES_ROUND_UP(size, 0x10000);
        if(sf_erase_block_64K(erase_addr , erase_size)  != 0)
            return ERROR_SPI_FLASH_ERASE;     
    }    
    return ERROR_SUCCESS;
}

error_t online_file_write(oline_data_type_t type, uint32_t addr, const uint8_t *buf, uint32_t size )
{
    uint32_t addr_offset;
    switch(type)
    {
        case USER_HEX:
            addr_offset = SF_USER_HEX_ADDR ;
        break;
        case CFG_WORD:
            addr_offset = SF_CONFIG_WORD_ADDR ;
        break;
        case HEX_CHECKSUM:
            addr_offset = SF_USER_HEX_CHECKSUM_ADDR ;
        break;
        case CFG_WORD_CHECKSUM:  
            addr_offset = SF_CONFIG_WORD_CHECKSUM_ADDR;
        break;
        case OFL_PROG_INFO:
            addr_offset = SF_OFFLINE_INFO;
        break;
        default:
            return ERROR_OUT_OF_BOUNDS;
//        break;   
    }
    addr +=  addr_offset;

    if(spi_flash_write(addr, buf, size) != 0)
       return ERROR_SPI_FLASH_WRITE;
   if(config_spi_flash_verify_program())
   {
        if(sf_cmp_data(addr, buf, size) != 0)    //比较写入的数据
            return ERROR_SPI_FLASH_WRITE;   
   
   }

   return ERROR_SUCCESS;
}
error_t online_file_read(oline_data_type_t type, uint32_t addr, uint8_t *buf, uint32_t size )
{
    uint32_t addr_offset;
    switch(type)
    {
        case USER_HEX:
            addr_offset = SF_USER_HEX_ADDR ;
        break;
        case CFG_WORD:
            addr_offset = SF_CONFIG_WORD_ADDR ;
        break;
        case HEX_CHECKSUM:
            addr_offset = SF_USER_HEX_CHECKSUM_ADDR ;
        break;
        case CFG_WORD_CHECKSUM:  
            addr_offset = SF_CONFIG_WORD_CHECKSUM_ADDR;
        break;
        case OFL_PROG_INFO:
            addr_offset = SF_OFFLINE_INFO;
        break;
        default:
            return ERROR_OUT_OF_BOUNDS;
//        break;   
    }
    addr +=  addr_offset;
    if(spi_flash_read(addr, buf, size) != 0)
        return ERROR_SPI_FLASH_READ;
    return ERROR_SUCCESS;  
}





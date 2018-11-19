#include "eslink.h"
#include "update.h"
#include "settings_rom.h"
#include "settings_spi_flash.h"
#include "offline_file.h" 
#include "eeprom.h" 

//ofl_prog_info_t ofl_prog_info;
 //时序基本信息解码
//脱机序列号
//脱机步骤
static error_t get_ofl_info(partition_t *part )
{
    error_t ret;
    uint8_t buf[OFL_PRJ_PARTITION_LEN];
//    uint32_t checksum = part->data;
    uint32_t size = part->size;
    uint32_t addr = part->start;       
//    uint32_t read_size = 0;
//    uint32_t checksum_temp = 0;
    
    if( part->type != OFL_INFO_PART)
        return ERROR_OFL_DATA_FORMAT; 
    if(size !=  OFL_PRJ_PARTITION_LEN)
        return ERROR_OFL_DATA_FORMAT; 
    
    ofl_file_read_start(); 
    ret = ofl_file_read(addr, buf, size);
    if(ERROR_SUCCESS != ret)
        return ret;   
    ofl_file_read_end();
   
	ret = online_file_erase(OFL_PROG_INFO, OFL_PRJ_PARTITION_LEN );	//擦除扇区
    ret = online_file_write(OFL_PROG_INFO, 0, buf, OFL_PRJ_PARTITION_LEN);     
    if(ERROR_SUCCESS != ret)
        return ret; 
    if(ERROR_SUCCESS != ret)
        return ret;
		
    return ERROR_SUCCESS;
}


//更新脱机时序
static error_t update_ofl_timing(partition_t *part )
{
    error_t ret;
    uint32_t prg_addr = 0;
    uint8_t buf[1024];
//    uint32_t flash_type = part->type;
    uint32_t checksum = part->data;
    uint32_t size = part->size;
    uint32_t addr = part->start;       
    uint32_t read_size = 0;
    uint32_t checksum_temp = 0;
    
    if( part->type != OFL_TIMING_PART)
         return ERROR_OFL_DATA_FORMAT; 
    
    ofl_file_read_start();  
    while(true)
    {        
        read_size = MIN(size, sizeof(buf));
        ret = ofl_file_read(addr, buf, read_size);
        if(ERROR_SUCCESS != ret)
            return ret;
        if(ofl_update_app_program(UPDATE_LINK_APP, prg_addr, buf, read_size) != TRUE)
            ret = ERROR_IAP_WRITE;
        if(ERROR_SUCCESS != ret)
            return ret; 
        checksum_temp += check_sum(read_size, buf); 
        addr += read_size;
        prg_addr += read_size;
        size -= read_size;
        if (size <= 0) 
            break;  
    }     
    ofl_file_read_end();
    
    if(checksum_temp != checksum)
        return ERROR_OFL_DATA_FORMAT;
    return ERROR_SUCCESS;
}
//更新目标芯片信息
static error_t update_ofl_target_info(partition_t *part )
{
    error_t ret;
//    uint32_t prg_addr = 0;
    uint8_t buf[1024];
//    uint32_t flash_type = part->type;
//    uint32_t checksum = part->data;
    uint32_t size = part->size;
    uint32_t addr = part->start;       
    uint32_t read_size = 0;
//    uint32_t checksum_temp = 0;
    
    if( part->type != OFL_TIMING_INFO_PART)
         return ERROR_OFL_DATA_FORMAT; 
    
    ofl_file_read_start();  
    while(true)
    {        
        read_size = MIN(size, sizeof(buf)); 
        ret = ofl_file_read(addr, buf, read_size);
        if(set_timing_info(buf) != TRUE )
            ret = ERROR_IAP_WRITE; 
        if(ERROR_SUCCESS != ret)
            return ret; 
//        checksum_temp += check_sum(read_size, buf); 
        addr += read_size;
//        prg_addr += read_size;
        size -= read_size;
        if (size <= 0) 
            break;  
    }     
    ofl_file_read_end();    
    return ERROR_SUCCESS;
}
//更新脱机配置字
static error_t update_ofl_config_word(partition_t *part )
{
    error_t ret;
    uint32_t prg_addr = 0;
    uint8_t buf[1024];

    uint32_t checksum = part->data;
    uint32_t size = part->size;
    uint32_t addr = part->start;       
    uint32_t read_size = 0;
    uint32_t checksum_temp = 0;
    
    if(part->type != OFL_CONFIG_PART)
         return ERROR_OFL_DATA_FORMAT;     
    
    ret = online_file_erase(CFG_WORD, size );
    if(ERROR_SUCCESS != ret)
        return ret; 
    ofl_file_read_start();  
    while(true)
    {        
        read_size = MIN(size, sizeof(buf));
        ret = ofl_file_read(addr, buf, read_size);
        if(ERROR_SUCCESS != ret)
            return ret; 
        ret = online_file_write(CFG_WORD, prg_addr, buf, read_size) ;   
        if(ERROR_SUCCESS != ret)
            return ret; 
        checksum_temp += check_sum(read_size, buf); 
        addr += read_size;
        prg_addr += read_size;
        size -= read_size;
        if (size <= 0) 
            break;  
    }     
    ofl_file_read_end();
    //比较脱机工程中读书的数据的校验和与脱机工程文件中保存的校验和
    if((checksum_temp&0x0000ffff) != checksum)  
        return ERROR_OFL_DATA_FORMAT;
           
    ret = online_file_erase(CFG_WORD_CHECKSUM, 4 );
    online_file_write(CFG_WORD_CHECKSUM, 0, (uint8_t*)&checksum_temp, 4);      //验证正确后在保存数据  
    return ERROR_SUCCESS;      
}
//更新脱机序列号
static error_t update_ofl_serialnum(partition_t *part )
{
    error_t ret;
    uint8_t buf[sizeof(ofl_serial_number_t)];
//    uint8_t buf_temp[sizeof(ofl_serial_number_t)];
//    uint32_t checksum = part->data;
    uint32_t size = part->size;
    uint32_t addr = part->start;       
//    uint32_t read_size = 0;
//    uint32_t checksum_temp = 0;
    
    if( part->type != OFL_SERIALNUM_PART)
        return ERROR_OFL_DATA_FORMAT; 
//    if(size !=  OFL_PRJ_PARTITION_LEN)
//        return ERROR_OFL_DATA_FORMAT; 
    
    ofl_file_read_start(); 
    ret = ofl_file_read(addr, buf, size);
    if(ERROR_SUCCESS != ret)
        return ret;   
    ofl_file_read_end();
    
    fm24cxx_write(EE_OFL_SERIAL_NUMBER_PARTITION, (uint8_t*)&(part->type), sizeof(partition_t));
//    fm24cxx_read(EE_OFL_SERIAL_NUMBER_PARTITION, buf, sizeof(partition_t));
    fm24cxx_write(EE_SERIAL_NUMBER_ADDR, buf, sizeof(buf));
    return ERROR_SUCCESS;   
}

//更新用户HEX
static error_t update_ofl_user_hex(partition_t *part )
{
    error_t ret;
    uint32_t prg_addr = 0;
    uint8_t buf[1024];

    uint32_t checksum = part->data;
    uint32_t size = part->size;
    uint32_t addr = part->start;       
    uint32_t read_size = 0;
    uint32_t checksum_temp = 0;
    
    if(part->type != OFL_HEX_PART)
         return ERROR_OFL_DATA_FORMAT; 
    ret = online_file_erase(USER_HEX, size );
    if(ERROR_SUCCESS != ret)
        return ret;
    ofl_file_read_start();      
    while(true)
    {        
        read_size = MIN(size, sizeof(buf));
        ret = ofl_file_read(addr, buf, read_size);
        if(ERROR_SUCCESS != ret)
            return ret; 
        ret = online_file_write(USER_HEX, prg_addr, buf, read_size) ;   
        if(ERROR_SUCCESS != ret)
            return ret; 
        checksum_temp += check_sum(read_size, buf); 
        addr += read_size;
        prg_addr += read_size;
        size -= read_size;
        if (size <= 0) 
            break;  
    }     
    ofl_file_read_end();
    //比较脱机工程中读书的数据的校验和与脱机工程文件中保存的校验和
    if((checksum_temp&0x0000ffff) != checksum)
        return ERROR_OFL_DATA_FORMAT;   
           
    ret = online_file_erase(HEX_CHECKSUM, 4 );
    ret = online_file_write(HEX_CHECKSUM, 0, (uint8_t*)&checksum, 4);      //验证正确后在保存数据  
    if(ERROR_SUCCESS != ret)
        return ret;
    return ERROR_SUCCESS;      
    
}

//更新脱机方案
error_t ofl_prj_update(char *path)
{
    error_t ret;
//    uint32_t i = 0;
    ofl_file_partition ofl_partition;
    
    uint32_t checksum = 0;
//    uint32_t size = 0;
//    uint32_t addr = 0;
    
//    uint32_t prg_addr = 0;
//    uint8_t buf[1024];
//    uint32_t read_size = 0;
//    char *name;
    //TODO:关闭中断
    ofl_file_get_partition(path ,&ofl_partition);
    //验证脱机文件的分区信息合法性
    if(ofl_partition.magic_word != OFL_PARTITION_MAGIC_WORD)
        return ERROR_OFL_DATA_FORMAT;
    if( (ofl_partition.size < OFL_PART_MIN) || (ofl_partition.size > OFL_PART_MAX) )          //
        return ERROR_OFL_DATA_FORMAT;
    checksum += check_sum(sizeof(ofl_partition)-4, ofl_partition.buf) ;
    if( checksum !=  ofl_partition.checksum)
        return ERROR_OFL_DATA_FORMAT;     
    
    ret = get_ofl_info( &ofl_partition.part[0]);
    if(ERROR_SUCCESS != ret)
        return ret; 
    ret = update_ofl_timing(&ofl_partition.part[1]);
    if(ERROR_SUCCESS != ret)
        return ret; 
    ret = update_ofl_target_info(&ofl_partition.part[2]);
    if(ERROR_SUCCESS != ret)
        return ret; 
    ret = update_ofl_config_word(&ofl_partition.part[3]);
    if(ERROR_SUCCESS != ret)
        return ret; 
    ret = update_ofl_serialnum(&ofl_partition.part[4]);
    if(ERROR_SUCCESS != ret)
        return ret;           
    ret = update_ofl_user_hex(&ofl_partition.part[5]);
    if(ERROR_SUCCESS != ret)
        return ret; 
 
    //更新成功，在EE中写入文件名，在脱出脱机状态时，会写序列号。
    fm24cxx_write(EE_OFL_SERIAL_NUMBER_PARTITION, (uint8_t*)path, OFL_FILE_NAME_MAX_LEN);
    return ERROR_SUCCESS;
}

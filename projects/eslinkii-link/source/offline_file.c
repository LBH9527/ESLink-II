/*******************************************************************************
*
*	模块名称 : 
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 :   任务设置：
*	修改记录 :
*	版本号    日期         作者        说明
*
*
*******************************************************************************/

#include "es_common.h"  
#include "eslink.h"
#include "spi_flash.h"
#include "ff.h"
#include "offline_file.h"
#include "offline_def.h"


static uint8_t file_find(void) ;

//typedef enum {
//    FILE_STATE_CLOSED,
//    FILE_STATE_FREE,
//    FILE_STATE_WRITE,
//    FILE_STATE_READ,
//} file_state_t;
//static file_state_t state = FILE_STATE_CLOSED;

//const char eslink_online_name[] = "S0:\\EslinkOnline";


FATFS FatFs;                    /* Work area (file system object) for logical drive */
static FIL file_handle;         /* File object */
static char file_name[OFL_FILE_NAME_MAX_LEN];
static struct ofl_file_tbl file_tbl;
static es_target_cfg target_dev;
static struct ofl_file_partition file_part;

static uint8_t file_mount(void)
{
    FATFS *fs;
    FRESULT res;        /* API result code */
    DWORD fre_clust, fre_sect, tot_sect;
    
    res = f_mount(&FatFs,"0:",1);
    if(res != FR_OK)        //挂载不成功，格式化SPI FLASH
    {
//        sf_erase_chip();
        res = f_mkfs("0:",0,0);
        if(res ==  FR_OK)
        {
            res = f_mount(0,"0:",0);	    //取消挂载				
            res = f_mount(&FatFs,"0:",1);    //挂载文件
        }
        else
        {
            return FALSE;
            //TODO: 挂载不成功处理
            while(1);
        }         
    }
     res = f_getfree("0:", &fre_clust, &fs);
     /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    
//    uint8_t buf[128] = {0x01, 0x02, 0x03};
//    uint8_t readbuf[128];
//    uint32_t bw;
//    res = f_open(&file_handle, "12345",   FA_CREATE_ALWAYS|FA_WRITE   );
//   res = f_write(&file_handle, buf , 128, &bw);
//   f_close(&file_handle);   
//    f_open(&file_handle, "12345",   FA_READ   );
//    res = f_read(&file_handle, readbuf, 128 , &bw);
//    f_close(&file_handle); 
     return TRUE;
}

static uint8_t file_read(uint32_t addr, uint8_t *buf, uint32_t size);
/*
 * 创建方案
 * path 方案名
 */
error_t ofl_file_open(char *path)
{
    FRESULT result;    
    //TODO:判断SPi FLASH 储存空间是否不足
    memcpy(file_name, path, sizeof(file_name) );      

    result  = f_open(&file_handle, file_name,  FA_CREATE_ALWAYS);  
    if (result != FR_OK)
    {          
        f_close(&file_handle); 
        return ERROR_FS_OPEN;
    }
    f_close(&file_handle);     
    
    return ERROR_SUCCESS;  
}

static uint8_t file_read(uint32_t addr, uint8_t *buf, uint32_t size)
{
    FRESULT result;
    uint32_t bw;
     
//    result = f_open(&file_handle, "12345",  FA_READ|FA_OPEN_EXISTING);
    result = f_open(&file_handle, file_name,  FA_READ|FA_OPEN_EXISTING);
    if (result != FR_OK)
    {          
        f_close(&file_handle); 
        return FALSE;
    }
    result = f_lseek(&file_handle,addr);  
    if (result != FR_OK)
    {          
        f_close(&file_handle); 
        return FALSE;
    }    
	result = f_read(&file_handle, buf, size , &bw);
	if(size != bw)
    {
        f_close(&file_handle); 
        return FALSE;              
    } 
    f_close(&file_handle);     
    return TRUE;     
}

/*
 * 写脱机工程文件
 * Flash_FS只支持顺序写入
 */
static uint8_t file_write( const uint8_t *buf, uint32_t size)
{
    FRESULT result;
    uint32_t length;   
    uint32_t bw;
     uint32_t write_size;   
//    if(state != FILE_STATE_WRITE)
//        return FALSE;
    result  = f_open(&file_handle, file_name,  FA_WRITE);
    if (result != FR_OK)
    {          
        f_close(&file_handle); 
        return FALSE;
    }   
 /* Move to end of the file to append data */
    result = f_lseek(&file_handle, f_size(&file_handle));    
    if (result != FR_OK)
    {          
        f_close(&file_handle); 
        return FALSE;
    }    
//    fpos = ftell (file_handle);    
    result = f_write(&file_handle, buf , size, &bw);
	if (result != FR_OK) 
    {
        f_close(&file_handle); 
        return FALSE;
    }
//    length = fwrite(buf, sizeof (uint8_t), size, file_handle);  
//    if(size == length)
//        break;
//    
//    if(n >=  retry)     //写入失败
//    {
//         fclose (fout);
//         return  -1 ;           
//    }
     /* 使用函数ferror检测是否发生过错误 */
    if (f_error(&file_handle) != NULL)  
    {
        f_close(&file_handle); 
        return FALSE; 
    }
//    if (f_sync (&file_handle))  
//    {
//       f_close(&file_handle); 
//        return FALSE;
//    } 

    if(size != bw)
    {
        f_close(&file_handle); 
        return FALSE;              
    } 
    f_close(&file_handle);     
//    while (write_size > 0) 
//   {
//        uint8_t rb_buf[16];
//        uint32_t verify_size = MIN(write_size, sizeof(rb_buf));
//        if (ofl_file_read(0, rb_buf, verify_size) != TRUE) {
//            return FALSE;
//        }
//        if (memcmp(buf, rb_buf, verify_size) != 0) {
//            return FALSE;
//        }
////        addr += verify_size;
//        buf += verify_size;
//        size -= verify_size;
//        write_size -= verify_size;    
//    }
    return TRUE;
}

static uint8_t file_find(void)
{
    DIR DirInf;
    FILINFO FileInf;
    FRESULT result;
    static char lfname[_MAX_LFN + 1];   /* Buffer to store the LFN */
//    uint8_t cnt = 0;
    char *fn ;
    FileInf.lfname = lfname;
	FileInf.lfsize = sizeof(lfname);      
    
	result = f_opendir(&DirInf, "/"); 
	if (result != FR_OK)
    {
        f_close(&file_handle); 
        return FALSE;  
    }
    for(;;)
    {
         result = f_readdir(&DirInf,&FileInf); 		    /* Read a directory item */
		if (result != FR_OK || FileInf.fname[0] == 0)   /* Break on error or end of dir */
		{
			break;
		}
        if (FileInf.fname[0] == '.')                    /* Ignore dot entry */
		{
			continue;
		}
        fn = *FileInf.lfname ? FileInf.lfname : FileInf.fname;

        if(! (FileInf.fattrib & AM_DIR) )
        {
             memcpy( file_tbl.path[file_tbl.count] , fn , OFL_FILE_NAME_MAX_LEN);
             file_tbl.count++;              
        }         
    }
    f_closedir(&DirInf);         
    return TRUE;  
}

/*
 *  获取所有脱机方案
 */
error_t get_all_ofl_file(void)
{
    uint8_t i;
    uint8_t data[128];      //for test
    
    file_tbl.count = 0;
    if(file_find() != TRUE)
        return ERROR_FS_FIND ;
    for( i=0; i<file_tbl.count; i++)
    {
        memcpy(file_name , file_tbl.path[i], sizeof( file_name));  
        ofl_file_read(OFL_INFO_PART,0 ,data, OFL_PRJ_INFO_LEN);
    } 
    return ERROR_SUCCESS;
}

/*
 *  读脱机方案数量
 *  data：返回信息数据
 */
error_t get_ofl_file_num( uint8_t *data)
{
    *data = file_tbl.count; 
    return ERROR_SUCCESS;
}
/*
 *  读脱机工程文件名称
 *  num: 方案序号  data：返回信息数据
 *  
 */
error_t get_ofl_file_name (uint8_t num, char *data)
{
     memcpy(data, file_tbl.path[num], OFL_FILE_NAME_MAX_LEN);
     return ERROR_SUCCESS; 
}
/*
 *  读脱机方案信息
 *  num: 方案序号  data：返回信息数据
 *  
 */
error_t get_ofl_file_info(uint8_t num, uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    memcpy(file_name , file_tbl.path[num], sizeof( file_name));
    ret = ofl_file_read(OFL_INFO_PART,0 ,data, OFL_PRJ_INFO_LEN);
    return ret ;
} 
/*
 *  删除脱机方案数据
 *  
 */
error_t delete_ofl_file(const char *name)
{
    f_unlink(name);    
    return ERROR_SUCCESS; 
}

/*
 * 写脱机工程文件
 * Flash_FS只支持顺序写入
 */
error_t ofl_file_write( uint8_t type, const uint8_t *buf, uint32_t size)
{
    uint8_t i ;
    uint8_t len ;
    
    if(OFL_TIMING_INFO_PART == type)
    {            
        i = sizeof(timing_info_t);      
        len = sizeof(es_target_cfg);
        for( ; i< len; i++) 
            *((uint8_t *)&target_dev) = *(buf++);   
    }
    if(file_write(buf, size) != TRUE)
         return ERROR_FS_WRITE ;
    return ERROR_SUCCESS ;
}

/*
 *  读脱机工程数据
 *  type: 读数据类型, addr:地址 data：数据 len：读数据长度
 *  
 */
error_t ofl_file_read(uint8_t type, uint32_t addr, uint8_t *data, uint32_t len)
{
    uint32_t addr_offset;
    uint32_t size;
    
     switch(type)
     {
        case OFL_INFO_PART:
            addr_offset = 0 ;    
            break;
        case OFL_TIMING_PART:
            addr_offset = OFL_PRJ_TIMING_ADDR; 
            break;
        case OFL_TIMING_INFO_PART:
            addr_offset = OFL_PRJ_CHIP_INFO_ADDR; 
            
            break;
        case OFL_HEX_PART:
            size = sizeof(es_target_cfg)+sizeof(timing_info_t) ;
            addr_offset = OFL_PRJ_CHIP_INFO_ADDR + size;
            break;
        case OFL_CONFIG_PART:
            size =  target_dev.code_size;
            addr_offset = file_part.cfg_word.start + size;
            break;
        default:
            break;        
     }  
     addr += addr_offset;   
     if(file_read( addr, data, len) != TRUE)
        return ERROR_FS_READ;
     return ERROR_SUCCESS;
}

/*
 *  读分区数据校验
 *  type: 读数据类型 data：校验和
 *  
 */
error_t get_ofl_file_checksum(uint8_t type, uint8_t *data)
{
    uint32_t size;
    uint32_t checksum = 0;  

    uint32_t read_addr = 0;
    uint32_t read_size = 0;
    uint8_t read_buf[1024] = {0};

    while(size > 0){      
        read_size = MIN(size, sizeof(read_buf));
        if(ofl_file_read(type, read_addr,  read_buf, read_size) != 0)
            return ERROR_FS_READ;
        checksum += check_sum(read_size, read_buf);
        
        read_addr += read_size;
        size -= read_size;
    }
    *data++ = (checksum>>0)&0xFF ;
    *data++ = (checksum>>8)&0xFF ;
    *data++ = (checksum>>16)&0xFF ;
    *data++ = (checksum>>24)&0xFF ; 
    //TODO:CRC校验
    *data++ = 0;
    *data++ = 0;
    *data++ = 0;
    *data++ = 0;
    return ERROR_SUCCESS;
}

/*
 *  脱机工程初始化，挂载文件系统，获取工程列表
 *  
 */
error_t ofl_init(void)
{
    error_t ret;
    
    
    spi_flash_init();  
    if(file_mount() != TRUE)
        return ERROR_FS_MOUNT ;
    
    ret = get_all_ofl_file();
    return ret;
}

/************************************脱机方案解码*******************************/
void get_ofl_file_partition(struct ofl_file_partition *part)
{


}

/*
 *  方案解析，
 *  id: 方案序号 
 *  
 */
uint8_t ofl_file_decode_init(uint8_t id)
{
    uint8_t len = 0;
    timing_info_t timing_info;
    es_target_cfg target_dev;
    uint32_t check_sum;
    uint8_t *data;
    uint32_t addr;
    uint32_t size;
    
    memcpy(file_name , file_tbl.path[id], sizeof( file_name));
    file_part.prj_info.start = 0;
    file_part.prj_info.size = OFL_PRJ_INFO_LEN;
    file_part.prj_info.data = 0;
    
    file_part.timing.start = OFL_PRJ_TIMING_ADDR;
    file_part.timing.size = OFL_PRJ_TIMING_LEN;
    file_part.timing.data = 0;
    
    addr = OFL_PRJ_TIMING_ADDR + OFL_PRJ_TIMING_LEN;
    size = sizeof(es_target_cfg)+sizeof(timing_info_t) ;
    file_part.chip_info.start = addr;
    file_part.chip_info.size =  size;
    file_part.chip_info.data = 0;
    
//    ofl_file_read(addr, data ,size);      
    len = sizeof(timing_info_t);
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&timing_info + i) = *(data++);
    
    len = sizeof(es_target_cfg);
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&target_dev + i) = *(data++);
//    len = 4;
//    for(uint8_t i = 0; i< len; i++) 
//        *((uint8_t *)&check_sum + i) = *(data++);
    
    addr = file_part.chip_info.start + file_part.chip_info.size;
    size = target_dev.config_word_size; 
    file_part.image.start = addr;
    file_part.image.size = size;

//    ofl_file_read(file_part.chip_info.start, (uint8_t*)checksum ,file_part.chip_info.size);
    file_part.image.data = check_sum;
    
    addr = file_part.image.start + file_part.image.size;
    size = target_dev.code_size; 
    file_part.cfg_word.start = addr;
    file_part.cfg_word.size = size;
    file_part.cfg_word.data = 0;
    
}


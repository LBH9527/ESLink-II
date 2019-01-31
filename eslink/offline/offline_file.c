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

static uint8_t file_find(void) ;


typedef enum {
    FILE_STATE_CLOSED,
    FILE_STATE_WRITE,
    FILE_STATE_READ,
} file_state_t;
static file_state_t state = FILE_STATE_CLOSED;

//const char eslink_online_name[] = "S0:\\EslinkOnline";


FATFS FatFs;                    /* Work area (file system object) for logical drive */
static FIL file_handle;         /* File object */
static char file_name[OFL_FILE_NAME_MAX_LEN];
static struct ofl_file_tbl file_tbl;
//static uint8_t file_partition[OFL_PRJ_PARTITION_LEN] ;       //保存分区信息
static  ofl_file_partition file_partition;

static uint8_t file_mount(void)
{    
    FRESULT res;        

    res = f_mount(&FatFs,"0:",1);        
    if(res != FR_OK)        //挂载不成功，格式化SPI FLASH
    {
        sf_erase_chip();
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
   
     return TRUE;
}

static uint8_t file_read(uint32_t addr, uint8_t *buf, uint32_t size)
{
    FRESULT result;
    uint32_t bw;

//    result = f_open(&file_handle, file_name,  FA_READ|FA_OPEN_EXISTING);
//    if (result != FR_OK)
//    {          
//        f_close(&file_handle); 
//        return FALSE;
//    }
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
//    f_close(&file_handle);     
    return TRUE;     
}

/*
 * 写脱机工程文件
 * Flash_FS只支持顺序写入
 */
static uint8_t file_write( const uint8_t *buf, uint32_t size)
{
    FRESULT result;
//    uint32_t length;   
    uint32_t bw;
//    uint32_t write_size;   

//    result  = f_open(&file_handle, file_name,  FA_WRITE);
//    if (result != FR_OK)
//    {          
//        f_close(&file_handle); 
//        return FALSE;
//    }   
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
//    f_close(&file_handle);     
    
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
 * 开始写入数据
 * Flash_FS只支持顺序写入
 */
error_t ofl_file_write_start(void)
{
    FRESULT result;
    result  = f_open(&file_handle, file_name,  FA_WRITE);
    if (result != FR_OK)
    {          
        f_close(&file_handle); 
        return ERROR_FS_OPEN;
    }  
    if(state != FILE_STATE_CLOSED)
        return ERROR_FS_WRITE;   
    state = FILE_STATE_WRITE;
    return ERROR_SUCCESS ;
}
/*
 * 写脱机工程文件
 * Flash_FS只支持顺序写入
 */
error_t ofl_file_write(const uint8_t *buf, uint32_t size)
{      
    if(state != FILE_STATE_WRITE)
        return ERROR_FS_WRITE;
    if(file_write(buf, size) != TRUE)
         return ERROR_FS_WRITE ;
    return ERROR_SUCCESS ;
}
error_t ofl_file_write_end(void)
{
    state = FILE_STATE_CLOSED;
    f_close(&file_handle);     
    return ERROR_SUCCESS ;
}  

error_t ofl_file_read_start(void)
{
    FRESULT result;
    
    if(state != FILE_STATE_CLOSED)
        return ERROR_FS_READ;
    state =  FILE_STATE_READ;
    
    result = f_open(&file_handle, file_name,  FA_READ|FA_OPEN_EXISTING);
    if (result != FR_OK)
    {          
        f_close(&file_handle); 
        return ERROR_FS_READ;
    } 
    return ERROR_SUCCESS;
}
/*
 *  读脱机工程数据
 *  type: 读数据类型, addr:地址 data：数据 len：读数据长度
 *  
 */
error_t ofl_file_read( uint32_t addr, uint8_t *data, uint32_t len)
{
    if(state != FILE_STATE_READ)
        return ERROR_FS_READ;  
     if(file_read( addr, data, len) != TRUE)
        return ERROR_FS_READ;
     return ERROR_SUCCESS;
}
//读文件结束
error_t ofl_file_read_end(void)
{
    state = FILE_STATE_CLOSED;
    f_close(&file_handle);     
    return ERROR_SUCCESS ;
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
    ofl_file_read_start();
    for( i=0; i<file_tbl.count; i++)
    {
        memcpy(file_name , file_tbl.path[i], sizeof( file_name));  
        ofl_file_read(OFL_PRJ_INFO_ADDR ,data, OFL_PRJ_INFO_LEN);
    } 
    ofl_file_read_end();
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
    ret = ofl_file_read_start();
    ret = ofl_file_read(OFL_PRJ_INFO_ADDR ,data, OFL_PRJ_INFO_LEN);
    ret = ofl_file_read_end();
    return ret ;
} 
//删除当前方案
error_t delete_current_ofl_file(void)
{
    f_unlink(file_name);    
    return ERROR_SUCCESS; 
}
/*
 *  删除脱机方案数据
 *  删除的方案编号
 *  
 */
error_t ofl_file_delete(uint8_t num)
{
    char path[OFL_FILE_NAME_MAX_LEN + 1];
    FRESULT result;    
    
    memcpy(path, file_tbl.path[num], OFL_FILE_NAME_MAX_LEN);
    result = f_unlink(path);  
    if (result != FR_OK)
    {          
        return ERROR_OFL_DELETE;
    }
    return ERROR_SUCCESS; 
}
/*
 * 创建方案
 * path 方案名
 */
error_t ofl_file_open(char *path)
{
    FRESULT result;    
    //TODO:判断SPi FLASH 储存空间是否不足
    memcpy(file_name, path, sizeof(file_name) );      
    if( file_tbl.count >= MAX_PAJ_NUM)
        return   ERROR_OFL_NUM_OVERSTEP;
        
    result  = f_open(&file_handle, file_name,  FA_CREATE_NEW);  
    
    if (result == FR_EXIST)     //文件已经存在
    {
        f_close(&file_handle); 
        return ERROR_OFL_EXIT;
        
    }
    if (result != FR_OK)
    {          
        f_close(&file_handle); 
        return ERROR_FS_OPEN;
    }
    f_close(&file_handle);  
    return ERROR_SUCCESS;  
}
/*
 *  脱机工程初始化，挂载文件系统，获取工程列表
 *  
 */
error_t ofl_file_init(void)
{
    error_t ret;     
    
    if(file_mount() != TRUE)
        return ERROR_FS_MOUNT ;
    
    ret = get_all_ofl_file();

    return ret;
}


/************************************脱机方案解码*******************************/
void ofl_file_init_partition( void)
{
  //在前128个字节中填入0xFF，以便后期写入分区值
    memset(file_partition.buf, 0xff , OFL_PRJ_PARTITION_LEN);
    
    file_partition.magic_word = OFL_PARTITION_MAGIC_WORD;
    file_partition.size = 0;
    file_partition.checksum = 0;
    file_partition.type = 0;
    ofl_file_write_start();
    ofl_file_write(file_partition.buf, OFL_PRJ_PARTITION_LEN);
    ofl_file_write_end();    
    
}
void ofl_file_add_partition( partition_t * part)
{
//    memcpy(&file_partition.part[file_partition.size], part, sizeof(partition_t)); 
    file_partition.part[file_partition.size].type =  part->type;
    file_partition.part[file_partition.size].size =  part->size;
    file_partition.part[file_partition.size].start =  part->start;
    file_partition.part[file_partition.size].data =  part->data;
    file_partition.size++;
}
//方案类型 
//1、普通：0x00 2、包括RTC时序的方案类型：0x01
void ofl_file_set_type(uint32_t type)
{
    file_partition.type = type;
}
//保存分区信息
error_t ofl_file_save_partition(void)
{
    FRESULT result;
//    uint32_t length;   
    uint32_t bw;
//    uint32_t write_size; 
    file_partition.checksum = check_sum(sizeof(file_partition) - 4, file_partition.buf);    //
    
    result  = f_open(&file_handle, file_name,  FA_WRITE);
    if (result != FR_OK)
    {          
        f_close(&file_handle); 
        return ERROR_FS_WRITE;
    }  
 /* Move to end of the file to append data */
    result = f_lseek(&file_handle, OFL_PRJ_PARTITION_ADDR);    
    if (result != FR_OK)
    {          
        f_close(&file_handle); 
        return ERROR_FS_WRITE;
    }     
    result = f_write(&file_handle, (uint8_t*)&file_partition , OFL_PRJ_PARTITION_LEN, &bw);
	if (result != FR_OK) 
    {
        f_close(&file_handle); 
        return ERROR_FS_WRITE;
    }
     /* 使用函数ferror检测是否发生过错误 */
    if (f_error(&file_handle) != NULL)  
    {
        f_close(&file_handle); 
        return ERROR_FS_WRITE; 
    }  
    if(OFL_PRJ_PARTITION_LEN != bw)
    {
        f_close(&file_handle); 
        return ERROR_FS_WRITE;              
    } 
    f_close(&file_handle);     
    return ERROR_SUCCESS ;
    
}
//获取脱机文件的分区信息
//path 文件名指针
error_t ofl_file_get_partition(char *path, ofl_file_partition * partition)
{     
    error_t ret;
    
    ret = ofl_file_open(path);
    ret = ofl_file_read_start();
    ret = ofl_file_read(OFL_PRJ_PARTITION_ADDR, partition->buf,OFL_PRJ_PARTITION_LEN);
    ret = ofl_file_read_end(); 
    
    return ret;
}
//向指定文件写入数据
error_t ofl_file_lseek_write(char *path, uint32_t addr, uint8_t *buf, uint32_t size)
{
    static FIL fd; 
    uint32_t bw;
    FRESULT result;
    
    result  = f_open(&fd, path,  FA_WRITE);
    if (result != FR_OK)
    {          
        f_close(&fd); 
        return ERROR_FS_WRITE;
    }  
 /* Move to end of the file to append data */
    result = f_lseek(&fd, addr);    
    if (result != FR_OK)
    {          
        f_close(&fd); 
        return ERROR_FS_WRITE;
    }     
    result = f_write(&fd, buf , size, &bw);
	if (result != FR_OK) 
    {
        f_close(&fd); 
        return ERROR_FS_WRITE;
    }
     /* 使用函数ferror检测是否发生过错误 */
    if (f_error(&fd) != NULL)  
    {
        f_close(&fd); 
        return ERROR_FS_WRITE; 
    }  
    if(size != bw)
    {
        f_close(&fd); 
        return ERROR_FS_WRITE;              
    } 
    f_close(&fd);     
    return ERROR_SUCCESS ;
}

//计算脱机工程空间 ，小于400K报错
error_t ofl_file_get_free(void)
{
    FATFS *fs;
    FRESULT res;        
    DWORD fre_clust, fre_sect, tot_sect;  
         res = f_getfree("0:", &fre_clust, &fs);
     /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    //TODO:容量不足需要提示
    if( fre_sect < 100)      //小于400K
    {
        return ERROR_OFL_SPACE_NOT_ENOUGH ;
    }
    return ERROR_SUCCESS ;
}


#include "stdio.h"
#include "RTL.h"
#include "es_common.h"   
#include "spi_flash_fs.h"  
#define ESLINK_ONLINE_FILENAME      "S0:\\EslinkOnline"

static FILE *fout;

int mount_filesystem(void)
{
    uint8_t result;
//        fformat ("S0:");
    /* 加载SPI Flash */
	result = finit("S0:");	

    if(result != 0)     
    {          
//        return result;
        if (fformat ("S0:") != 0)    //格式化
        {
             funinit("S0:");        //卸载
            //TODO:格式化失败，判断是否是spi flash故障。
              return 0; //挂载失败
        }
    }
    /* 创建/打开 联机工程文件*/
	fout = fopen (ESLINK_ONLINE_FILENAME,"w" ); 
    if (fout == NULL) 
    {
        fclose (fout);
        return -1;
    }
    /* 关闭文件 */
    fclose(fout);
    return 0 ;    
    
}

static bool file_write_init = false;
void ol_file_write_start(void)
{
//    fout = fopen (ESLINK_ONLINE_FILENAME, "w");     
}
////写数据到联机工程
int ol_file_write(uint32_t addr, const uint8_t *buf, size_t size)
{
////    FILE *fout;
//    U32  fpos;
//    uint32_t length;   

//    fpos = ftell (fout);                             
//    fseek (fout, addr, SEEK_SET);      
//    length = fwrite(buf, sizeof (uint8_t), size, fout);  
////    if(size == length)
////        break;
////    
////    if(n >=  retry)     //写入失败
////    {
////         fclose (fout);
////         return  -1 ;           
////    }
//     /* 使用函数ferror检测是否发生过错误 */
//    if (ferror(fout) != NULL)  
//    {
//        fclose (fout);
//        file_write_init = false;
//         return  -1 ; 
//    }
//    //    if (fflush (fout))  
////    {
////        fclose (fout);
////        return  -1 ; 
////    } 

//    if(size != length)
//    {
//        fclose (fout);
//        file_write_init = false;
//        return -1;          
//    }        
//  
//    return 0;
}

void ol_file_write_end(void)
{
//    file_write_init = false;
//    /* 关闭文件 */
//    fclose(fout);     
}

   

//int ol_file_read_start()
//{
//     fout = fopen (ESLINK_ONLINE_FILENAME, "r");     
//}
int ol_file_read(uint32_t addr, uint8_t *buf, size_t size)
{
//    uint32_t length; 
//    U32  fpos;
////    FILE * file; 
//    static bool file_initialized = false;

//    if(file_initialized != true)
//    {
//         fout = fopen (ESLINK_ONLINE_FILENAME, "r");          
//    }      
//    fseek (fout, addr, SEEK_SET);
////    fpos = ftell (fout);   
//    /* 读数据 */     
//	length = fread(buf, sizeof(uint8_t), size, fout);
//    if(length !=  size)
//    {
//         fclose (fout);
//         return  -1 ;           
//    }
//    /* 使用函数ferror检测是否发生过错误 */
//    if (ferror(fout) != NULL)  
//    {
//        fclose (fout);
//         return  -1 ; 
////        printf("写入内容失败\r\n");
//    }
//    if(size != length)
//        return -1; 
//    return 0;  
}

int fs_cmp_data(uint32_t addr, const uint8_t *buf, uint32_t size)
{
//    uint8_t rb_buf[32];
//    FILE *fout;
//    uint32_t length; 
//    
//    fout = fopen (ESLINK_ONLINE_FILENAME, "r"); 
//    
//    
//    while (size)
//	{
//        uint32_t verify_size = MIN(size, sizeof(rb_buf));
//        fseek (fout, addr, SEEK_SET);  
//		/* 读一个字节 */
//        length = fread(rb_buf, sizeof(uint8_t), verify_size, fout);
//        if (memcmp(buf, rb_buf, verify_size) != 0) 
//        {
//            return -1;
//        } 
//        size -=  verify_size;
//        addr += verify_size;
//        buf += verify_size;
//        
//	}  
// /* 使用函数ferror检测是否发生过错误 */
//    if (ferror(fout) != NULL)  
//    {
//        fclose (fout);
//         return  -1 ; 
//    }
//    /* 关闭文件 */
//    fclose(fout);
//    return 0;
    
}
//查找文件
int fs_file_find(void)
{
//    FINFO info;
//    
//    info.fileID = 0;                             /* info.fileID must be set to 0 */
//    
//    while(ffind ("S0:*.*", &info) == 0)          /* find whatever is in drive "S0:" */ 
//    {

//    }   
//    if (info.fileID == 0)  
//	{
//        //没有脱机方案
//        return -1;
//	}   
//    return 0;    
    
}


//读数据
int fs_file_read(FILE * fin, uint32_t read_offset, uint8_t *buf, size_t size)
{
//    uint32_t length; 
//    FILE * file; 
//    
//    file = fin;
//    
//    fseek (file, read_offset, SEEK_SET);
//    /* 读数据 */
//    
//	length = fread(buf, sizeof(uint8_t), size, file);
//    if(length !=  size)
//    {
//         fclose (file);
//         return  -1 ;           
//    }
//    /* 使用函数ferror检测是否发生过错误 */
//    if (ferror(file) != NULL)  
//    {
//        fclose (file);
//         return  -1 ; 
////        printf("写入内容失败\r\n");
//    }
   
    return size;       
}
//#include "es_common.h"  
//#include "spi_flash.h"
//#include "ff.h"

//#define ESLINK_ONLINE_FILENAME      "eslink_online.txt"
//FATFS fs;
//FIL file;
//FRESULT res_flash;
//int mount_filesystem(void)
//{
//    FRESULT res;        /* API result code */

//    res = f_mount(&fs,"0:",1);
//    if(res != FR_OK)        //挂载不成功，格式化SPI FLASH
//    {
//        sf_erase_chip();
//        res = f_mkfs("0:",0,0,0,0);
//        if(res ==  FR_OK)
//        {
//            res = f_mount(0,"0:",0);	    //取消挂载				
//            res = f_mount(&fs,"0:",1);    //挂载文件
//        }
//        else
//        {
//            //TODO: 挂载不成功处理
//            while(1);
//        }         
//    }
//    res = f_open(&file, ESLINK_ONLINE_FILENAME, FA_CREATE_NEW | FA_WRITE | FA_READ  );
//     if (res != FR_OK) {
//        /* Seek to end of the file to append data */
//        if (res != FR_OK)
//            f_close(&file);
//    }
//     return 0;
//}

//int ol_file_read(uint32_t addr, uint8_t *buf, size_t size)
//{
//    
//    
//}
//int ol_file_write(uint32_t addr, const uint8_t *buf, size_t size)
//{
//    
//}
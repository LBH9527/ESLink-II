//脱机方案管理
//1、管理脱机方案
//2、管理脱机方案的分区信息
//Note:采用指针方式方便以后更换文件驱动。

#include "RTL.h"
#include "es_common.h"
#include "prj_file_driver.h"

static void init(void);
static int open( uint32_t size, uint8_t *buf);
static int write(const uint8_t *buf, size_t size);
static int read(uint32_t read_offset,  uint8_t *buf, size_t size);
static int delete();
static int close(void);




static FILE * fout;   
#define PRJ_NAME_SIZE           32

static U8 *prj_name[PRJ_NAME_SIZE];
//声明一个内存池，共cnt块，每块大小32字节

static U8 *prj_malloc(uint8_t cnt)
{
    //声明一个内存池，每块大小32字节，内存块个数：cnt，
    _declare_box(blob_pool, PRJ_NAME_SIZE, cnt);
    
    //内存池初始化     
    _init_box (blob_pool, sizeof(blob_pool), PRJ_NAME_SIZE);
    //从内存池申请一个内存块 ,并清零
    return(_calloc_box (blob_pool));      
    
}      

static uint8_t get_prj_num(void)
{
    uint8_t  prj_num = 0;
    FINFO info;
    info.fileID = 0;                             /* info.fileID must be set to 0 */
    
    while(ffind ("S0:*.*", &info) == 0)          /* find whatever is in drive "S0:" */    
    {
        prj_num ++;     
    }
    if (info.fileID == 0)  
	{
//		printf ("SPI Flash中没有存放文件\r\n");
        return -1;
	}
    return prj_num;
}

//初始化文件系统，挂载不通过，先格式化，在挂载
//static void init(void)
//{
//    uint8_t result;
//    //挂载spi flash 
//	result = finit ("S0:"); 		//正式使用时在boot中  加载（是否是在上电过程中自检，判断文件系统是否挂载成功）
//    if(result != 0)     //格式化
//    {
//        if (fformat ("S0:") != 0)    //格式化失败
//        {
//            result = funinit("S0:");        //卸载
//            //TODO:格式化失败，判断是否是spi flash故障。
//            
//        }
//    }
//    //prj_module = 0            初始化
//}


int get_prj_table(struct ofl_prj_tbl *table)
{
    FINFO info;
    info.fileID = 0;                             /* info.fileID must be set to 0 */
    uint8_t  prj_num = 0;
    
    while(ffind ("S0:*.*", &info) == 0)          /* find whatever is in drive "S0:" */    
    {
        memcpy( table->name_tbl[prj_num] , info.name , PRJ_NAME_SIZE); 
        prj_num ++;  
    }
    if (info.fileID == 0)  
	{
//		printf ("SPI Flash中没有存放文件\r\n");
        return -1;
	}        
    table->prj_num =  prj_num;   
//    *prj_name =  prj_malloc( get_prj_num());
//    
//    while(ffind ("S0:*.*", &info) == 0)          /* find whatever is in drive "S0:" */    
//    {
//        memcpy(*prj_name , info.name , PRJ_NAME_SIZE);
//        *prj_name +=  PRJ_NAME_SIZE;
//    }
//    if (info.fileID == 0)  
//	{
////		printf ("SPI Flash中没有存放文件\r\n");
//        return -1;
//	}     
    return 0;
}

//创建/打开 文件
static int open( uint32_t size, uint8_t *buf)
{
    uint8_t n = 3;
    char file_name[32];
    uint32_t bw; 
  
    
    for(n=0; n<32; n++)
    {
         file_name[n] =  *(buf+n);
    }     
    //todo: 是否需要在文件名前加上此文件的id，是否会更方便查找此文件。
	fout = fopen (file_name, "w");      //创建方案名的文件。
	if (fout == NULL) 
    {
        fclose (fout);
        return -1;
    }    
    return 0;  
}
//指定工程中写数据
//只支持顺序写入数据
//return >= 0: successful read data size
//         -1: error
//static int write(uint32_t offset, const uint8_t *buf, size_t size)
static int write(const uint8_t *buf, size_t size)
{
    uint8_t n,retry = 3;
    uint32_t length; 
    
    for(n=0; n< retry; n++)
    {
         length = fwrite(buf, sizeof (uint8_t), size, fout);  
         if(size == length)
             break;
    }
    if(n >=  retry)     //写入失败
    {
         fclose (fout);
         return  -1 ;           
    }
     /* 使用函数ferror检测是否发生过错误 */
    if (ferror(fout) != NULL)  
    {
        fclose (fout);
         return  -1 ; 
//        printf("写入内容失败\r\n");
    }
    return size;   
        
}

//read
//the actual read data bytes or 0 on end of file or failed.
static int read(uint32_t read_offset,  uint8_t *buf, size_t size)
{
    uint32_t length; 
    
    fseek (fout, read_offset, SEEK_SET);
      /* 读数据 */
    
	length = fread(buf, sizeof(uint8_t), size, fout);
    if(length !=  size)
    {
         fclose (fout);
         return  -1 ;           
    }
    /* 使用函数ferror检测是否发生过错误 */
    if (ferror(fout) != NULL)  
    {
        fclose (fout);
         return  -1 ; 
//        printf("写入内容失败\r\n");
    }
   
    return size;     
} 

static int delete(void)
{
    
    
}

static int find(const char *name)
{
    FINFO info;
    
    info.fileID = 0;                             /* info.fileID must be set to 0 */
    
    while(ffind ("S0:*.*", &info) == 0)          /* find whatever is in drive "S0:" */    
    {
          if(info.name  ==  name)
              break;        
    }
    if (info.fileID == 0)  
	{
//		printf ("SPI Flash中没有存放文件\r\n");
        return -1;
	}
    return 0;
    
}
static int close(void)
{
    fclose (fout);
    return 0;
    
}

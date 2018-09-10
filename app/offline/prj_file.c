//脱机方案处理和分区处理
#include "RTL.h"
#include "es_common.h"    
#include "prj_file.h"

// Set to 1 to enable debugging
#define PRJ_FILE_DEBUG     0
#ifdef PRJ_FILE_DEBUG
#include "eslink_debug.h"
#define printf    debug_msg
#else
#define printf(...)
#endif


//脱机   
//正式使用时在boot中  加载（是否是在上电过程中自检，判断文件系统是否挂载成功）
//上电后挂载文件系统
int prj_file_init(void)
{
    uint8_t result;
    /* 加载SPI Flash */
	result = finit("S0:");	
//    fformat ("S0:");
    if(result != 0)     
    {
        
        //挂载失败
        return result;
//        if (fformat ("S0:") != 0)    //格式化失败
//        {
//             funinit("S0:");        //卸载
//            //TODO:格式化失败，判断是否是spi flash故障。
//              return 0; //挂载失败
//        }
    }
    return ERROR_SUCCESS ;
    
}

error_t ofl_prj_uninit(void)
{
    
    return ERROR_SUCCESS ;
}

int prj_file_get_table(struct prj_tbl *table)
{
    FINFO info;
    info.fileID = 0;                             /* info.fileID must be set to 0 */
    uint8_t  prj_num = 0;      
    char (*name)[PRJ_NAME_LEN];
    
    name = table->name_tbl;
    
    while(ffind ("S0:*.*", &info) == 0)          /* find whatever is in drive "S0:" */    
    {
        memcpy( name++ , info.name , PRJ_NAME_LEN);
             
        prj_num ++;  
        printf( "name: %-20s, ID: %04d  ", info.name, prj_num );
    }
    if (info.fileID == 0)  
	{
//		printf ("SPI Flash中没有存放文件\r\n");
        return -1;
	}        
    table->prj_cnt =  prj_num;   
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


/*******************************************************************************
*函数名：ofl_prj_find
* 描述 : 查找脱机方案信息
* 输入 ：
* 输出 ：0：存在指定名字的脱机方案
*******************************************************************************/
int prj_file_find(const char *name)
{
    FINFO info;
    
    info.fileID = 0;                             /* info.fileID must be set to 0 */
    
    while(ffind ("S0:*.*", &info) == 0)          /* find whatever is in drive "S0:" */    
    {
        if (!strcmp(name, (char*)info.name))
              return 0;        
    }
    if (info.fileID == 0)  
	{
        //没有脱机方案
        return -1;
	}
    return -2;      //没找到此名称的脱机方案
    
}    

//offline prj create
//创建/打开 文件
int prj_file_open(struct prj_ctl_block *prj)
{   
    FILE * fout;   
    char file_name[PRJ_NAME_LEN] = {0};
      
//    file_name =  prj->name;       
   
    memcpy(file_name, prj->name, PRJ_NAME_LEN );   
    //todo: 是否需要在文件名前加上此文件的id，是否会更方便查找此文件。
	//fout = fopen (file_name, "w");      //创建方案名的文件。
    fout = fopen (file_name, "w");    
	if (fout == NULL) 
    {
        fclose (fout);
        return -1;
    }
    prj->pFile = fout;
    return 0;  
}  

//int prj_creat( struct ofl_prj *prj)
//{
//    if(prj_file_open(prj) != 0)
//        return ERROR_FS_CREAT;
//    get_prj_table(

//}
//offlin e prj write
//写数据
int prj_file_write(const struct prj_ctl_block *prj, uint32_t addr, const uint8_t *buf, size_t size)
{
    uint8_t n,retry = 3;
    uint32_t length; 
    FILE * fout; 
    
    fout = prj->pFile;
    
    fseek (fout, addr, SEEK_SET);
    
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

//读数据
int prj_file_read(struct  prj_ctl_block *prj, uint32_t read_offset, uint8_t *buf, size_t size)
{
    uint32_t length; 
    FILE * fout; 
    
    fout = prj->pFile;
    
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


//方案删除
// =0 删除成功
//不为0 删除失败
int prj_file_del(struct prj_ctl_block *prj)
{

    return (   fdelete (prj->name) );
//    if (fdelete (prj->name) != 0) 
//    {
//         re
//        //printf("%s文件不存在（返回值：%d）\r\n", FileName, result);
//    }
//    else
//    {
//        
//        //printf("删除%s文件成功\r\n", FileName);
//    }	    
    
}


//uint8_t prj_manager(offline_state_t state,uint32_t ops,  uint32_t size, uint8_t *buf)
//{
//    FILE *fout;   
//    uint8_t result;
//  
///*    switch(offline_wr_state)
//    {
//        case STATE_WR_INFO:
//            result = prj_file_open(fout, size, buf);
//            prj_module.part[OFFLINE_INFIO].type = OFFLINE_INFIO;
//            prj_module.part[OFFLINE_INFIO].offset = 0;
//            prj_module.part[OFFLINE_INFIO].size = size;
//            break;
//        case STATE_WR_TIMING_START:
//            result = offline_prj_write(fout, size, buf) ;
//            prj_module.part[OFFLINE_TIMING].type = OFFLINE_TIMING;
//            prj_module.part[OFFLINE_TIMING].offset = 0;
//            prj_module.part[OFFLINE_INFIO].size += size;
//            break;
//        case STATE_WR_TIMING:
//            result = offline_prj_write(fout, size, buf) ;
//            prj_module.part[OFFLINE_INFIO].size += size;
//            
//            break;
//        case STATE_WR_TIMING_END:
//            //返回校验
////            offline_prj_read(fout,prj_module.part[OFFLINE_INFIO].size, );
//            break;
//        case STATE_WR_CONFIG:
//            
//            break;
//        case STATE_WR_IMAGE:
//            break;
//        
//        
//        
//        
//        
//    }
// 	
//*/	
//}






//int olf_prj_erase(const struct olf_prj *prj, uint32_t addr, size_t size)
//{


//}    

//int olf_prj_erase_all(const struct olf_prj *part)
//{
//    return fal_partition_erase(part, 0, part->len);
//}



////static struct prj_item_list prj_item_head;

////int offline_prj_item_init(void) 
////{

////	memset(&prj_item_head, 0x0, sizeof(struct prj_item_list_node));
////	list_init(&prj_item_head.list);
////	prj_item_head.obj.name = "NO INFO";
////	return 0;
////}

////prj_item_list_t offline_prj_item_create(prj_item_t item)
////{
//////    list_t *p_list;
////    struct prj_item_list *new_node;
//////	struct prj_item_list_node *info_node;
//////	struct prj_item_list_node *_info_node;
//////	p_list = &prj_item_head.list;
////    
////    new_node = (struct prj_item_list *) malloc(
////			sizeof(struct prj_item_list));
////    
////    new_node->obj = item;
////	
////	list_insert_after(&prj_item_head.list, &new_node->list);
////	return new_node;
////}
////    

////int offline_prj_item_delete(prj_item_list_t item)
////{
////    list_remove(&(item->list));

////}	

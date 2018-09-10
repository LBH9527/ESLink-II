#include "eslink.h"
#include "offline.h"     

//static uint8_t flash_buf[512];
//static uint8_t flash_buf[FLASH_DECODER_MIN_SIZE];
//static prj_state_t state = PRJ_STATE_CLOSED;
//static timing_type_t timing_type;
//static uint32_t initial_addr;
//static uint32_t current_addr;
//static bool flash_initialized;
//static bool initial_addr_set;


//方案信息表
static struct prj_tbl  ofl_prj_table;
//当前方案
static struct  prj_ctl_block ofl_pcb ;

 /*******************************************************************************
*函数名：ofl_get_prj_table
* 描述 ：初始化脱机文件 的操作接口（方便在不同flash上的移植），获取脱机工程表，
* 输入 ：
* 输出 ：
*******************************************************************************/
//error_t ofl_get_prj_table(struct prj_tbl *tbl)
error_t ofl_get_prj_table(void)
{
     error_t result;
     result = prj_file_get_table(&ofl_prj_table);        //获取脱机工程表
    
     return ERROR_SUCCESS;
}
/*******************************************************************************
*函数名：ofl_get_prj_num
* 描述 ：读脱机方案数
* 输入 ：
* 输出 ：
*******************************************************************************/
uint8_t ofl_get_prj_num(void)
{
     return( ofl_prj_table.prj_cnt)  ;    
}
/*******************************************************************************
*函数名：ofl_prj_create
* 描述 ：创建指定名称的脱机文件
* 输入 ：
* 输出 ：
*******************************************************************************/
error_t ofl_prj_create(const char *name)
{
    int result;
    
    result = prj_file_find(name);
    if((result) == 0)   //已近存在此方案
        return ERROR_OFL_PRJ_REPEAT;
    memcpy(  ofl_pcb.name , name  ,strlen(name));
    if(prj_file_open(&ofl_pcb) <0)
        return ERROR_OFL_PRJ_CREAT; 
    ofl_pcb.pos = PRJ_START_ADDR;
    ofl_pcb.part_addr = 0;
    
    ofl_pcb.part.type = 0;
    ofl_pcb.part.start = 0;
    ofl_pcb.part.size = 0;
    ofl_pcb.part.data = 0;
    return ERROR_SUCCESS;
    
}
/*******************************************************************************
*函数名：ofl_prj_open
* 描述 ：打开指定编号的脱机文件
* 输入 ：
* 输出 ：
*******************************************************************************/
error_t ofl_prj_open(uint8_t id )
{
    int result;
    
    result = prj_file_find(ofl_prj_table.name_tbl[id]);
        
    if((result) != 0)     // 未找到此方案
        return result;
    memcpy(  ofl_pcb.name , ofl_prj_table.name_tbl[id]  ,PRJ_NAME_LEN );
    prj_file_open(&ofl_pcb);
    ofl_pcb.pos = PRJ_START_ADDR;
    ofl_pcb.part_addr = 0;
    ofl_pcb.part.type = 0;
    ofl_pcb.part.start = 0;
    ofl_pcb.part.size = 0;
    ofl_pcb.part.data = 0;
    
    return ERROR_SUCCESS;
}

error_t ofl_prj_del(uint8_t id )
{
    
      return ERROR_SUCCESS;
}

error_t ofl_prj_del_all(void)
{
    
     return ERROR_SUCCESS;
}

//获取分区大小和属性
//void ofl_prj_get_sector(struct ofl_prj_target *ofl_prj,uint32_t read_offset, uint8_t *buf,struct prj_partition_sector )
//{   
//    struct prj_target *psInst;  

//    psInst = &(ofl_prj->target);    
//    
//    prj_file_read(psInst, read_offset,   ,8);
//}

static int ofl_prj_read( uint32_t addr, uint8_t *buf, size_t size)
{
     return ( prj_file_read(&ofl_pcb ,addr,buf, size ))  ; 
}

static int ofl_prj_write( uint32_t addr, const uint8_t *buf, size_t size)
{
     return (prj_file_write(&ofl_pcb, addr, buf, size)) ;
    
}



/*******************************************************************************
*函数名：
* 描述 ：读方案信息
* 输入 ：
* 输出 ：
*******************************************************************************/
error_t ofl_prj_read_info(uint8_t *data)
//error_t ofl_prj_read_info(void)
{
    uint32_t n;
    uint8_t sector_buf[ PART_SECTOR_LEN];  
    uint8_t flash_buf[512] = {0};
    
    ofl_prj_read(ofl_pcb.part_addr, sector_buf, PART_SECTOR_LEN);
    for (n = 0; n < PART_SECTOR_LEN; n++) 
    {
        *((uint8_t *)&ofl_pcb.part + n) = sector_buf[n];
    }
    
    if(ofl_pcb.part.type != INFIO_PART)
        return ERROR_INTERNAL;
    
    ofl_prj_read(ofl_pcb.pos, flash_buf, ofl_pcb.part.size);
    
//    ofl_pcb.mode.port =  (flash_buf[PRJ_NAME_LEN    ]    <<24)| 
//                            (flash_buf[PRJ_NAME_LEN + 1]    <<16)| 
//                            (flash_buf[PRJ_NAME_LEN + 2]    <<8) | 
//                            (flash_buf[PRJ_NAME_LEN + 3]) ;
//    ofl_pcb.mode.step =  (flash_buf[PRJ_NAME_LEN + 4]    <<24)| 
//                            (flash_buf[PRJ_NAME_LEN + 5]    <<16)| 
//                            (flash_buf[PRJ_NAME_LEN + 6]    <<8) | 
//                            (flash_buf[PRJ_NAME_LEN + 7]) ;
//                                
//    for (n = 0; n < ofl_pcb.mode.step; n++) 
//    {
//        ofl_pcb.mode.item[n] =   (flash_buf[PRJ_NAME_LEN + 4*n + 8]  <<24) | 
//                                    (flash_buf[PRJ_NAME_LEN + 4*n + 9]  <<16) | 
//                                    (flash_buf[PRJ_NAME_LEN + 4*n + 10]  <<8) | 
//                                    (flash_buf[PRJ_NAME_LEN + 4*n + 11]) ;
//    }
//    
   ofl_pcb.mode.step =  (data[PRJ_NAME_LEN    ]    <<24)| 
                            (data[PRJ_NAME_LEN + 1]    <<16)| 
                            (data[PRJ_NAME_LEN + 2]    <<8) | 
                            (data[PRJ_NAME_LEN + 3]) ;
   
                                
    for (n = 0; n < ofl_pcb.mode.step; n++) 
    {
        ofl_pcb.mode.item[n] =   (data[PRJ_NAME_LEN + 4*n + 4]  <<24) | 
                                    (data[PRJ_NAME_LEN + 4*n + 5]  <<16) | 
                                    (data[PRJ_NAME_LEN + 4*n + 6]  <<8) | 
                                    (data[PRJ_NAME_LEN + 4*n + 7]) ;
    }
    
    ofl_pcb.mode.port =  (data[PRJ_NAME_LEN + 4*n + 8]    <<24)| 
                            (data[PRJ_NAME_LEN + 4*n + 9]    <<16)| 
                            (data[PRJ_NAME_LEN + 4*n + 10]    <<8) | 
                            (data[PRJ_NAME_LEN + 4*n + 11]) ;   
    return ERROR_SUCCESS;
}



/*******************************************************************************
*函数名：ofl_prj_read_timing_info
* 描述 ：读时序信息.(默认“时序信息”在一帧数据内）
* 输入 ：
* 输出 ：
*******************************************************************************/
error_t ofl_prj_read_timing_info(void *target)
{
    uint32_t n;
    uint8_t sector_buf[ PART_SECTOR_LEN]; 
    uint8_t data_buf[PRJ_READ_MIN_SIZE];

    
    ofl_prj_read(ofl_pcb.part_addr, sector_buf, PART_SECTOR_LEN);
    for (n = 0; n < PART_SECTOR_LEN; n++) 
    {
        *((uint8_t *)&ofl_pcb.part + n) = sector_buf[n];
    }  
    if(ofl_pcb.part.type != TIMING_INFO_PART)       //判断分区信息与数据属性是否一致
        return  ERROR_OFL_PRJ_READ;
    
    ofl_pcb.size = ofl_pcb.part.size;               //时序总长度
    ofl_pcb.pos = ofl_pcb.part.start;              //时序起始地址
    ofl_prj_read(ofl_pcb.pos, data_buf , ofl_pcb.size); 

        
    switch(ofl_pcb.mode.port )
    {
        case 0:
            break;
        case 1:
            ofl_set_swd_target_info(data_buf,(target_cfg_t*)target);
            break;  
        case 3:
            break;
        case 4:
            break;
        default:
            break;
        
    }
    
}



/*******************************************************************************
*函数名：
* 描述 ：读时序
* 输入 ：
* 输出 ：
*******************************************************************************/
error_t ofl_prj_read_timing(void *target)
{
    uint32_t n;
    uint8_t sector_buf[ PART_SECTOR_LEN];  
    uint32_t  read_size;
    block_date_t block;
    
    ofl_prj_read(ofl_pcb.part_addr, sector_buf, PART_SECTOR_LEN);
    for (n = 0; n < PART_SECTOR_LEN; n++) 
    {
        *((uint8_t *)&ofl_pcb.part + n) = sector_buf[n];
    }
    
    if(ofl_pcb.part.type != TIMING_PART)       //判断分区信息与数据属性是否一致
        return  ERROR_OFL_PRJ_READ;
    
    ofl_pcb.size = ofl_pcb.part.size;               //时序总长度
    ofl_pcb.pos = ofl_pcb.part.start;      //时序起始地址
    while( ofl_pcb.size > 0)
    {
        //上位机按块下发时序数据，每包数据情况：起始地址+数据长度+数据。  
        read_size = MIN(ofl_pcb.size, BLOCK_DATE_SIZE );
        ofl_prj_read(ofl_pcb.pos, (uint8_t *)&block.satrt_addr, read_size);
        
        switch( ofl_pcb.mode.port )
        {
            case OFL_TYPE_UNKNOWN:
                break;
            case OFL_TYPE_ISP:
                
                break;
            case OFL_TYPE_SWD:    
                ofl_set_swd_timing((target_cfg_t*) target, block.satrt_addr, block.buf, block.size);
                break;
            case OFL_TYPE_BOOTISP:
                break;
            
        }  
        ofl_pcb.size -= read_size;
        ofl_pcb.pos += read_size;
    }
    return  ERROR_SUCCESS;
} 

//用户程序
error_t ofl_read_image(void)
{
    uint32_t n;
    uint8_t sector_buf[ PART_SECTOR_LEN];  
    uint32_t  read_size;
    block_date_t block;
    
    ofl_prj_read(ofl_pcb.part_addr, sector_buf, PART_SECTOR_LEN);
    for (n = 0; n < PART_SECTOR_LEN; n++) 
    {
        *((uint8_t *)&ofl_pcb.part + n) = sector_buf[n];
    }
    
    if(ofl_pcb.part.type != HEX_PART)       //判断分区信息与数据属性是否一致
        return  ERROR_OFL_PRJ_READ;
    
    ofl_pcb.size = ofl_pcb.part.size;           //时序总长度
    ofl_pcb.pos = ofl_pcb.part.start;          //时序起始地址  

    while( ofl_pcb.size > 0)
    {
         //上位机按块下发时序数据，每包数据情况：起始地址+数据长度+数据。  
        read_size = MIN(ofl_pcb.size, BLOCK_DATE_SIZE );
        ofl_prj_read(ofl_pcb.pos, (uint8_t *)&block.satrt_addr, read_size);
        switch( ofl_pcb.mode.port )
        {
            case OFL_TYPE_UNKNOWN:
                break;
            case OFL_TYPE_ISP:
                
                break;
            case OFL_TYPE_SWD:    
              
                break;
            case OFL_TYPE_BOOTISP:
                break;
            
        }  
        ofl_pcb.size -= read_size;
        ofl_pcb.pos += read_size;
    }

    
}
//写分区信息
error_t ofl_prj_write_part(struct prj_partition *part)
{
    uint8_t n;
    uint8_t sector_buf[ PART_SECTOR_LEN];   
    
    for(n = 0; n < PART_SECTOR_LEN; n++)
    {
        sector_buf[n] = *((uint8_t *)part + n) ;
    }
    
//    sector_buf[0] = part_type          & 0xFF;
//    sector_buf[1] = (part_type >> 8)   & 0xFF;
//    sector_buf[2] = (part_type >> 16)  & 0xFF;
//    sector_buf[3] = (part_type >> 24)  & 0xFF;
//    sector_buf[4] = addr          & 0xFF;
//    sector_buf[5] = (addr >> 8)   & 0xFF;
//    sector_buf[6] = (addr >> 16)  & 0xFF;
//    sector_buf[7] = (part_type >> 24)  & 0xFF;
//    sector_buf[8] = size  & 0xFF;
//    sector_buf[9] = (size >> 8)  & 0xFF;
//    sector_buf[10] = size >> 16) & 0xFF;
//    sector_buf[11] = (size >> 24) & 0xFF;       
    
    ofl_prj_write(ofl_pcb.part_addr, sector_buf, sizeof(sector_buf));
    ofl_pcb.part_addr += sizeof(sector_buf);
    
    //清零part
    part->type = 0;
    part->start = 0;
    part->size = 0;
    part->data = 0; 
    return ERROR_SUCCESS; 
}

///* file system partition table */
//struct dfs_partition
//{
//    uint32_t type;        /* file system type */
//    uint32_t  offset;       /* partition start offset */
//    uint32_t size;         /* partition size */
//    uint32_t type_data;  
//};

//if (!(header[0] == 'S' &&
//          header[1] == 'F' &&
//          header[2] == 'D' &&
//          header[3] == 'P')) 
//int dfs_filesystem_get_partition(struct dfs_partition *part,
//                                      uint8_t         *buf,
//                                      uint32_t        pindex)
//{
//#define DPT_ADDRESS     0x1be       /* device partition offset in Boot Sector */
//#define DPT_ITEM_SIZE   16          /* partition item size */

//    uint32_t *dpt;
//    uint32_t type;


//    dpt = buf + DPT_ADDRESS + pindex * DPT_ITEM_SIZE;

//    /* check if it is a valid partition table */
//    if ((*dpt != 0x80) && (*dpt != 0x00))
//        return -EIO;

//    /* get partition type */
//    type = *(dpt+4);
//    if (type == 0)
//        return -EIO;

//    /* set partition information
//     *    size is the number of 512-Byte */
//    part->type = type;
//    part->offset = *(dpt+8) | *(dpt+9)<<8 | *(dpt+10)<<16 | *(dpt+11)<<24;
//    part->size = *(dpt+12) | *(dpt+13)<<8 | *(dpt+14)<<16 | *(dpt+15)<<24;

//    return RT_EOK;
//}


/*******************************************************************************
*函数名：ofl_prj_write_info
* 描述 ：写方案信息.以方案信息中的前64个字节创建文件。
* 输入 ：
* 输出 ：
*******************************************************************************/
error_t ofl_prj_write_info( const uint8_t *data,uint32_t size)
{
    uint8_t n;
    char name[PRJ_NAME_LEN] ;
    uint8_t ret;
    
    for(n = 0; n < PRJ_NAME_LEN; n++)
    {
        *(name+n) = *(data+n);
    }  
    ret = ofl_prj_create( name);
    if(ret != 0)
        return ret;     
    
    ofl_prj_write(ofl_pcb.pos, data, size);
    ofl_pcb.pos += size;
    return ERROR_SUCCESS;      
}

/*******************************************************************************
*函数名：ofl_prj_write_data
* 描述 ：写数据
* 输入 ：
* 输出 ：
*******************************************************************************/
void ofl_prj_write_data( const uint8_t *data, size_t size)
{
    uint32_t  copy_size;
    uint8_t flash_buf[512];
    //写数据
    while(size >0)
    {
        copy_size = MIN(size, sizeof(flash_buf) );
        ofl_prj_write(ofl_pcb.pos, data, copy_size);
        size -= copy_size;         
        data += copy_size;
        ofl_pcb.pos += copy_size;    
    } 
}








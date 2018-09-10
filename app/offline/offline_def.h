#ifndef _OFFLINE_DEF_H_
#define _OFFLINE_DEF_H_

#include "stdio.h" 
#include "stdint.h" 
#include "error.h"

/*          flie 数据分区信息
 * |----------------------------|   Storage Size
 * |      partition 表          |   
 * |----------------------------|
 * |      INFO                  |  
 * |----------------------------|
 * |      TIMING_IFNO           |
 * |----------------------------|
 * |      TIMING                |
 * |----------------------------|
 * |      CONFIG                |
 * |----------------------------|
 * |      HEX                   |
 * |----------------------------|
 * 
*/



//分区信息长度
#define PART_AREA_SIZE         64     
#define PRJ_START_ADDR         64      //保存文件的起始地址
//每个分区信息数据的长度
#define PART_SECTOR_LEN      16          


//支持的方案最大数
#define MAX_PAJ_NUM         10
//方案名长度
#define PRJ_NAME_LEN        60
//每次读方案数据长度
#define PRJ_READ_MIN_SIZE      0x200



//总方案表
 struct prj_tbl{
    uint8_t prj_cnt;            //方案总数
    uint8_t cur_id;             //当前方案编号
    char name_tbl[10][PRJ_NAME_LEN];    //方案名表  
}; 

//脱机最大步骤数
#define MAX_OFL_STEP        5             
struct ofl_step_mode{       
    uint32_t step;                  //脱机步骤
    uint32_t item[MAX_OFL_STEP];    //脱机项目
    uint32_t port;                  //脱机接口
};       //脱机模式

//脱机接口类型  
#define OFL_TYPE_UNKNOWN    0x00
#define OFL_TYPE_ISP        0x01
#define OFL_TYPE_SWD        0x02
#define OFL_TYPE_BOOTISP    0x03

//方案分区信息
#define UNKNOWN_PART             0x0000
#define INFIO_PART               0x0100           //基本信息
#define TIMING_INFO_PART         0x0200           //时序信息
//#define ISP_TIMING_INFO          0x0200           
//#define SWD_TIMING_INFO          0x0201
//#define BOOTISP_TIMING_INFO      0x0202
#define TIMING_PART              0x0300           //时序
#define CONFIG_PART              0x0400           //配置字
#define HEX_PART                 0x0500           //用户Hex    
/* file system partition  */
//分区信息索引
struct  prj_partition
{
    uint32_t type;                  /* partition type */
    uint32_t start;                 /* partition start offset */
    uint32_t size;                  /* partition size */
    uint32_t data;                  //各种类型的私有数据。（校验和等数据）
};
//extern struct prj_partition prj_part;

//方案控制块
struct prj_ctl_block{      
    char name[PRJ_NAME_LEN];
    FILE * pFile; 
    uint8_t  part_addr;     //文件索引分区的读写地址
                            
    
    uint32_t pos;           /* Current file position */
    uint32_t size;           /* Size in bytes */
     struct ofl_step_mode mode;     // 脱机操作
    struct  prj_partition  part;    //文件分区信息
    
};  
typedef struct prj_ctl_block  *ofl_prj_data_t  ;
//extern  struct prj_ctl_block ofl_pcb ;











//脱机方案中的数据类型
//typedef enum{
//    
//    INFIO_PART          = 0x0100,           //基本信息
//    TIMING_INFO_PART    = 0x2000,
////    ISP_TIMING_INFO     = 0x0200,           //时序信息
////    SWD_TIMING_INFO     = 0x0201,
////    BOOTISP_TIMING_INFO = 0x0202,
//    TIMING_PART         = 0x0300,           //时序
//    CONFIG_PART         = 0x0400,           //配置字
//    IMAGE_PART          = 0x0500,           //用户Hex    
//}partition_type;

// partition





////timing 状态
// typedef enum {
//    TIMING_STATE_CLOSED,
//    TIMING_STATE_OPEN,
//    TIMING_STATE_DONE,
//    TIMING_STATE_ERROR 
//} timing_state_t;
 
// struct prj_partition{ 
//    
//    char prj_name[PRJ_NAME_LEN];            //方案名，用于查找方案
////    struct prj_partition_sector sector;
//    uint32_t info_start;            //基本信息   
//    uint32_t info_end;  
//    uint32_t timing_info_start;
//    uint32_t timing_info_size;
//    uint32_t timing_start;          //时序
//    uint32_t timing_size;  
//    uint32_t image_start;           //用户程序
//    uint32_t image_end;  
//    uint32_t config_start;          //配置字
//    uint32_t config_end;     
//};


#endif


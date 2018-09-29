#ifndef _OFFLINE_DEF_H_
#define _OFFLINE_DEF_H_

#include "stdio.h" 
#include "stdint.h" 
#include "errno.h"
#include "eslink_addr.h"

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


//支持的方案最大数
#define MAX_PAJ_NUM                         10
//方案名最大长度
#define OFL_FILE_NAME_MAX_LEN               16
//分区信息长度   
#define OFL_PRJ_INFO_ADDR                  	0x00                //保存文件的起始地址
#define OFL_PRJ_INFO_LEN                    0x80                //规定文件信息定长  
#define OFL_PRJ_TIMING_ADDR             	(OFL_PRJ_INFO_ADDR + OFL_PRJ_INFO_LEN)      //时序起始地址
#define OFL_PRJ_TIMING_LEN              	(ESLINK_ROM_LINK_SIZE)                        //时序长度 
#define OFL_PRJ_CHIP_INFO_ADDR              (OFL_PRJ_TIMING_ADDR + OFL_PRJ_TIMING_LEN)  //芯片信息地址


//支持的方案最大数
#define MAX_PAJ_NUM                         10 
//方案名最大长度
#define OFL_FILE_NAME_MAX_LEN               16
 
//方案分区信息
#define UNKNOWN_PART                    0x00
#define OFL_INFO_PART                   0x01           //基本信息
#define OFL_TIMING_PART                 0x02           //时序信息
#define OFL_TIMING_INFO_PART              0x03           //芯片信息
#define OFL_HEX_PART                    0x04           //用户Hex 
#define OFL_CONFIG_PART                 0x05           //配置字
 
//分区信息索引
struct  partition
{
//    uint32_t type;                  /* partition type */
    uint32_t start;                 /* partition start offset */
    uint32_t size;                  /* partition size */
    uint32_t data;                  //各种类型的私有数据。（校验和等数据）
};

struct ofl_file_partition{      
//    char prj_name[PRJ_NAME_LEN];            //方案名，用于查找方案
    struct partition prj_info;				//方案信息
	struct partition timing;				//时序信息
	struct partition chip_info;            //芯片信息
	struct partition image;				//用户HEX
	struct partition cfg_word;				//用户配置字
};


//总方案表
 struct ofl_file_tbl{
    uint8_t count;                  //方案总数
    uint8_t cur_id;             //当前方案编号
    char path[MAX_PAJ_NUM][OFL_FILE_NAME_MAX_LEN];    //方案名表  
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


////方案控制块
//struct prj_ctl_block{      
//    char name[OFL_FILE_NAME_MAX_LEN];
//    FILE * pFile; 
//    uint8_t  part_addr;     //文件索引分区的读写地址
//                            
//    
//    uint32_t pos;           /* Current file position */
//    uint32_t size;           /* Size in bytes */
//     struct ofl_step_mode mode;     // 脱机操作
////    struct  ofl_file_partition  part;    //文件分区信息
//    
//};  
//typedef struct prj_ctl_block  *ofl_prj_data_t  ;



#endif


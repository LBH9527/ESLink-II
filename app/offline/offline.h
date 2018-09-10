#ifndef __OFFLINE_H__
#define __OFFLINE_H__

#include "stdint.h" 
#include "string.h"
#include "error.h"  
#include "es_common.h"   
#include "prj_file.h"
#include "offline_def.h" 
#include "swd_prj_decoder.h"







////方案基本信息
//struct {
//    char prj_name[32];
//    uint8_t chip_name[16];
//    uint8_t version[4];
//    uint8_t date[8];        
////    offline_step_mode_t offline_step;
//    uint32_t offline_step;
//    uint32_t *offline_items;
//    uint8_t  interface_type[4];        //脱机烧录接口  
//    
//    //待添加
//    //isp_serial_number_t 
//    //target_cfg_t target_device
//}prj_info;



//脱机方案基本信息：在方案中的地址和长度。
//struct  {
// 
//    uint8_t program_name[32];
//    uint8_t chip_name[16];
//    uint8_t version[4];
//    uint8_t date[8];        
////    offline_step_mode_t offline_step;
//    uint32_t offline_step;
//    uint32_t *offline_items;
//    uint8_t  interface_type[4];        //脱机烧录接口  
//    
//    uint32_t info_start;            //基本信息   
//    uint32_t info_end;              
//    uint32_t timing_start;          //时序
//    uint32_t timing_end;  
//    uint32_t image_start;           //用户程序
//    uint32_t image_end;  
//    uint32_t config_start;          //配置字
//    uint32_t config_end;   
//    
//} project_info;
//project_cfg_t offline_prj_info;
error_t ofl_get_prj_table(void) ;
uint8_t ofl_get_prj_num(void);
error_t ofl_prj_write_info( const uint8_t *data,uint32_t size);

error_t ofl_prj_write_part( struct prj_partition *part)  ;
void ofl_prj_write_data( const uint8_t *data, size_t size);


error_t ofl_prj_open(uint8_t id );
//error_t ofl_prj_read_info(uint8_t *data);

error_t ofl_prj_read_info(uint8_t *data);
error_t ofl_prj_read_timing_info(void *target);
error_t ofl_prj_read_timing(void *target);

#endif

#ifndef _OFFLINE_DEF_H_
#define _OFFLINE_DEF_H_

#include "eslink.h"
#include "eslink_addr.h"
#include "errno.h"



/*          flie 数据分区信息  
 * |----------------------------|
   |    partition               |
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
#define MAX_PAJ_NUM                     10
//方案名最大长度
#define OFL_FILE_NAME_MAX_LEN           16
//分区信息长度  
#define OFL_PRJ_PARTITION_ADDR          0x00                //文件分区信息在文件中的起始地址
#define OFL_PRJ_PARTITION_LEN           0x80                //文件分区信息在文件中的长度
#define OFL_PRJ_INFO_ADDR               0x80                //保存文件的起始地址
#define OFL_PRJ_INFO_LEN                0x80                //规定文件信息定长  

//支持的方案最大数
#define MAX_PAJ_NUM                         10 
//方案名最大长度
#define OFL_FILE_NAME_MAX_LEN               16
 
#define OFL_PARTITION_MAGIC_WORD  0x4F464c00            //用户分区信息 
//方案分区信息类型
typedef enum{
    OFL_INFO_PART           = 0x4F464c01 ,              //基本信息
    OFL_TIMING_PART         = 0x4F464c02 ,              //时序信息
    OFL_TIMING_INFO_PART    = 0x4F464c03,               //芯片信息
    OFL_CONFIG_PART         = 0x4F464c04 ,              //配置字
    OFL_SERIALNUM_PART      = 0x4F464c05 ,              //脱机序列号
    OFL_HEX_PART            = 0x4F464c06 ,              //用户Hex     
} ofl_part_type_t ;
         
//分区信息索引
typedef struct 
{
    uint32_t type;                  /* partition type */
    uint32_t start;                 /* partition start offset */
    uint32_t size;                  /* partition size */
    uint32_t data;                  //各种类型的私有数据。（校验和等数据）
}partition_t;

#define OFL_PART_MIN    6       //最小分区数
#define OFL_PART_MAX    7       //最大分区数
union __attribute__((packed)) ofl_file_partition 
{
    uint8_t buf[OFL_PRJ_PARTITION_LEN];
    struct __attribute__((packed)) {
        uint32_t magic_word;            
        uint32_t size;                  //分区 part个数
        partition_t part[OFL_PART_MAX]; //分区信息
        uint32_t record_type;           //方案类型
        uint32_t checksum;              //方案校验和（ OFL_PRJ_PARTITION_LEN - 4）
    };
};
typedef union ofl_file_partition ofl_file_partition;

//总方案表
 struct ofl_file_tbl{
    uint8_t count;                  //方案总数
    uint8_t cur_id;                 //当前方案编号
    char path[MAX_PAJ_NUM][OFL_FILE_NAME_MAX_LEN];    //方案名表  
}; 

//脱机序列号

//脱机序列号编程使能
#define OFL_SERIALNUM_IN_FLASH     0x12340013       //脱机序列号编程使能，序列号填写到目标芯片的FALSH中    0x12340013       
#define OFL_SERIALNUM_IN_EEPROM    0x12340014       //脱机序列号编程使能，序列号填写到目标芯片的在EEPROM中   0x12340014
#define OFL_SERIALNUM_DISABLE      0x12340000       //脱机序列号编程不使能  0x12340000   
// 启动烧写方式 （自动检测  或 机台的start 信号）                               
#define OFL_PROG_START_AUTO_CHECK_MODE  0x522C0001      //自动检测
#define OFL_PROG_START_MACHINE_MODE     0x522C0002      //机台启动
#define OFL_PROG_START_KEY_MODE         0x522C0003      //按键启动
//序列号数据读取方式  (用户读取序列号的方式，32位机用IAP方式读取，8位机用返回指令的方式）  
#define OFL_SERIALNUM_READ_USE_IAP      0x00001111          
typedef struct __attribute__((packed)) 
{      
    uint32_t state;              //脱机序列号编程使能  OFL_SERIALNUM_IN_FLASH / OFL_SERIALNUM_IN_EEPROM/OFL_SERIALNUM_DISABLE 
//    uint32_t sn_addr;               //序列号起始地址
//    uint32_t sn_size;               //数据长度       
//    uint8_t sn_data[16];            //序列号代码
    serial_number_t sn;
    uint32_t sn_data_checksum;      //序列码校验和  
    uint32_t sn_step;                  //步长    
    uint32_t total_size;            //烧写芯片个数       
    uint32_t success_count;         //已成功编程芯片个数  
    uint32_t start_mode;            //启动烧写方式:OFL_SERIALNUM_PROG_START_MODE
    uint32_t read_mode;             //序列号数据读取方式:OFL_SERIALNUM_READ_USE_IAP 
} ofl_serial_number_t;

//脱机最大步骤数 
#define MAX_OFL_STEP            5   
#define OFL_STEP_ERASE          0x00000020      //擦除
#define OFL_STEP_CHECK_EMPTY    0x00000021      //查空
#define OFL_STEP_PROG           0x00000022      //烧录
#define OFL_STEP_VERIFY         0x00000023      //校验
#define OFL_STEP_ENCRYPT        0x00000024      //加密

typedef union __attribute__((packed)) 
{
    uint8_t buf[OFL_PRJ_PARTITION_LEN];
    struct __attribute__((packed)) {
        uint32_t prj_name[4];                   //方案名称
        uint32_t chip_name[4];                  //芯片名称
        uint32_t date[2];                       //日期
        uint32_t checksum;                      //累加校验和
        uint32_t crc;                           //CRC校验和
        uint32_t intf;                          //脱机烧录接口   
        uint32_t step;                          //脱机步骤数
        uint32_t item[20];                      //脱机步骤项目              
    };
}ofl_prj_info_t ;




#endif


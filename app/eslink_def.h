#ifndef __ESLINK_DEF_H__
#define __ESLINK_DEF_H__


//eslink 参数信息
typedef struct {
    uint32_t build_key;
    uint32_t hic_id;
    uint32_t version;           //固件版本
    uint16_t power_state;       //power状态
} eslink_info_t;

typedef struct {  
    uint32_t version;        /*时序版本*/
} offline_info_t;

//时序信息
typedef struct {
    uint32_t id;             /*时序号*/   
    uint32_t version;        /*时序版本*/
    uint32_t checksum;       //时序校验和
} timing_info_t;  

//目标芯片信息
typedef struct  {       
    uint32_t code_start;            //主程序区起始地址
    uint32_t code_size;             //主程序区容量
    uint32_t config_word_start;     //配置字信息区起始地址
    uint32_t config_word_size;      //配置字信息区容量
    uint32_t encrypt_addr;           //加密字地址
    uint32_t encrypt_value;          //加密字值
    uint32_t chipid_addr;           //芯片ID地址
    uint32_t chipid_value;          //芯片ID值
    
} es_target_cfg;  

/*
 *  序列号代码
 */
typedef struct {
    uint8_t enable_flag;     //序列号是否有效.
    uint32_t addr;           //序列号起始地址
    uint32_t size;           //序列号数据长度
    uint32_t data[2];       //序列号代码值
}es_serial_number_t;

//编程操作接口
struct es_prog_ops {
    void    (*init) (es_target_cfg *target) ;
    error_t (*erase_chip) ( uint8_t *para);
    error_t (*check_empty) (uint32_t *addr, uint32_t *data);    
    error_t (*program_flash) (uint32_t adr, uint8_t *buf, uint32_t size);
    error_t (*read_flash) (uint32_t adr, uint32_t *buf, uint32_t size);
    error_t (*read_chipid) (void);
    error_t (*program_config_word) (uint32_t adr, uint32_t *buf, uint32_t size);
    error_t (*read_config_word) (uint32_t adr, uint32_t *buf, uint32_t size);                
};
//编程协议帧接口
typedef struct{
    uint32_t  frame_head;
    uint8_t   device_type;
	uint8_t   fun_code ;
	uint16_t  data_length;
    uint8_t   state;
    uint16_t  checksum;
    uint8_t *wrbuf;
    uint8_t *rdbuf;
}prog_comm_frame_t;    
//调试协议帧接口
typedef struct{
    uint32_t  frame_head;           //帧头
	uint16_t  data_length;          //数据长度（接收、发送）
    uint8_t   device_addr;          //设备地址
	uint8_t   fun_code ;            //功能码                           
    
    uint8_t   device_state;         //设备状态
    uint16_t  checksum;             //校验和 
    uint8_t   *wrbuf;            //指向接收数组
    uint8_t   *rdbuf;            //指向发送数组
    
}debug_comm_frame_t;  

//编程接口定义
typedef enum 
{
    PRG_INTF_ISP = 0x52000000 ,         //isp烧录
    PRG_INTF_SWD = 0x52000001 ,         //swd烧录
    PRG_INTF_BOOTISP = 0x52000002,          //bootisp烧录
} prog_intf_type_t;   

//脱机步骤
#define OFL_STEP_ERASE           0x20
#define OFL_STEP_BLANK_CHECK     0x21  
#define OFL_STEP_PRORAM          0x22
#define OFL_STEP_VERIFY           0x23
#define OFL_STEP_ENCRYPT           0x24
#define OFL_SETP_CFG_WORD           0x26

//编程步长
#define FLASH_PRG_MIN_SIZE      1024

//芯片擦除模式
#define FULL_ERASE      0         //全擦
#define PAGE_EARSE      1         //页擦
                                  //二次开发擦除
//通信协议中的数据长度，每帧数据长度位0x03e8 = 1000
#define  BLOCK_DATE_SIZE   1000
typedef struct {    
    uint32_t satrt_addr;        //起始地址
    uint32_t size;              //数据长度
    uint32_t buf[BLOCK_DATE_SIZE-8];  //数据
} block_date_t;

/*******************************************************************************
    通信协议相关宏定义
*******************************************************************************/
//通信协议常量定义
#define ESLINKII_DEVICE_TYPE                    0x52
#define ES_COMM_FRAME_MAX_LEN                   0x500               //数据包长度

#define PROG_FRAME_HEAD                         0xBADCCDAB  
#define PROG_FRAME_SIZE                         0x400               //帧长度 1024 
#define DEBUG_FRAME_HEAD                        0xABCDDCBA     




#define OFL_FRAME_HEAD                  0xABCDDCBA          //帧头
#define OFL_DEV_ADDR                    0x33                //设备地址

#define ES_DATA_OFFSET					0x08                //数据区在buffer中的偏移

#define OFL_DATA_LEN					0x09                //数据区长度
#define OFL_CODE_OFFSET					0x11                //代码区在buffer中的偏移
#define CMD_HEADER_LEN                  8           //协议头长度


// ESLINKII Command IDs
#define ID_HANDSHAKE                        0x01

#define ID_READ_BOOT_VERSION                0xD7    //读boot版本
#define ID_DL_BOOT_START                    0xD8    //脱机工程下载开始   
#define ID_DL_BOOT_HEX                      0xD9    //下载脱机工程HEX'  （1024）
#define ID_DL_BOOT_HEX_END                  0xDA    //bootloader下载结束 

//#define ID_DL_TIMING                        0x19    //时序下载(1024)
//#define ID_DL_TIMING_START                  0x1B    //时序下载开始    
//#define ID_DL_TIMING_END                    0x1C	//时序下载结束
#define ID_READ_TIMING_INFO                 0x28    //读芯片信息
#define ID_DL_TIMING_INFO                   0x27    //下载芯片信息


#define ID_DL_PRG_INTF                      0x37    //下载烧录接口
#define ID_DL_CONFIG_WORD                   0x17    //下载配置字
#define ID_DL_CONFIG_WORD_END                   0x3A    //配置字下载完成
#define ID_DL_USERHEX                       0x18    //下载用户HEX(1024)
#define ID_DL_USERHEX_END                   0x1D    //用户HEX下载结束
#define ID_DL_SERIALNUM                     0x2B    //下载序列号代码

#define ID_READ_PRG_INTF                    0x39    //读烧录接口
#define ID_FULL_ERASE                       0x20    //擦除
#define ID_CHECK_EMPTY                      0x21    //查空
#define ID_PRG_HEX                          0x22    //编程
#define ID_VERIFY                           0x23    //校验     
#define ID_ENCRYPT                          0x24    //加密
#define ID_READ_CONFIG_WORD                 0x25    //读配置字
#define ID_PRG_CONFIG_WORD                  0x26    //配置字编程
#define ID_READ_CHIPID                      0x29    //读芯片ID
#define ID_READ_FLASH                       0x2A    //读flash  （帧长度 1024）

#define ID_DL_OFFLINE_PRJ_INFO              0x2E    //脱机方案下载
#define ID_READ_OFFLINE_PRJ_NUM             0x2F    //读方案数量
#define ID_READ_OFFLINE_PRJ_INFO            0x30    //读方案信息
#define ID_DELETE_OFFLINE_PRJ               0x38    //删除指定脱机方案
#define ID_DL_TIMING_START                  0x31    //时序下载开始
#define ID_DL_TIMING_ING                    0x32    //时序下载（1024）
#define ID_DL_TIMING_END                    0x33    //时序结束
#define ID_DL_OFFLINE_CONFIG_WORD           0x34    //脱机方案配置字下载
#define ID_DL_OFFLINE_CONFIG_WORD_END       0x3B    //脱机方案配置字下载完成
#define ID_DL_OFFLINE_HEX               0x35    //脱机方案用户HEX  （1024）
#define ID_DL_OFFLINE_HEX_END           0x36    //脱机方案用户HEX下载完成
#define ID_DL_OFFLINE_END           0x3C    //脱机方案下载完成


#endif

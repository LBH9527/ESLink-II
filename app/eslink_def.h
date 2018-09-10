#ifndef __ESLINK_DEF_H__
#define __ESLINK_DEF_H__


//eslink 参数信息
typedef struct {
    uint32_t build_key;
    uint32_t hic_id;
    uint32_t version;           //固件版本
    uint16_t power_state;       //power状态
} eslink_info_t;

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
    uint32_t config_start;          //配置字信息区起始地址
    uint32_t config_size;           //配置字信息区容量
    uint32_t encrypt_addr;           //加密字地址
    uint32_t encrypt_value;          //加密字值
    uint32_t chipid_addr;           //芯片ID地址
    uint32_t chipid_value;          //芯片ID值
    
} es_target_cfg;
extern es_target_cfg es_target_device;
#define TARGET_TIMING_INFO_LEN          12          //The Length of chip  Information ,Unit Byte功能码 0x28  
#define TARGET_IC_INFO_LEN              32          //target_isp_cfg_t结构体从code_start到chipid_value长度(byte)


//es_link数据结构    
    
union B32_B08
{
    uint8_t     B08[4];
	uint32_t    B32;
};
union B16_B08
{
    uint8_t     B08[2];
	uint16_t    B16;
};  
#define ISP_FRAME_HEAD              0xABCDDCBA     
#pragma  pack(1)
typedef union{
    
    uint32_t B32[256];
    uint8_t B08[1024];
}uint32_t_uint8_t;
#pragma pack()
//#pragma  pack(1)
typedef struct{
    uint32_t  frame_head;
    uint8_t   device_type;
	uint8_t   fun_code ;
	uint16_t  data_length;
    uint8_t   state;
    uint16_t  checksum;
    uint32_t_uint8_t *wrbuf;
    uint32_t_uint8_t *rdbuf;
}ISP_DATA_t;



 #define ISP_PACKET_SIZE             0x400       //帧长度 1024 
//编程接口定义
#define PRG_INTF_ISP            0x51310000          //isp烧录
#define PRG_INTF_SWD            0x51310001          //swd烧录
#define PRG_INTF_BOOTISP        0x51310002          //bootisp烧录

//时序信息最大长度
#define PRG_INTF_ISP            0x51310000          //isp烧录
//通信协议中的数据长度，每帧数据长度位0x03e8 = 1000
#define  BLOCK_DATE_SIZE   1000
typedef struct {    
    uint32_t satrt_addr;        //起始地址
    uint32_t size;              //数据长度
    uint32_t buf[BLOCK_DATE_SIZE-8];  //数据
} block_date_t;

//芯片擦除模式
#define FULL_ERASE      0         //全擦
#define PAGE_EARSE      1         //页擦
                                  //二次开发擦除
/*******************************************************************************
    通信协议相关宏定义
*******************************************************************************/
//通信协议常量定义
#define ESLINKII_DEVICE_TYPE            0x52
#define OFL_FRAME_HEAD                  0xABCDDCBA          //帧头
#define OFL_DEV_ADDR                    0x33                //设备地址

#define ES_DATA_OFFSET					0x08                //数据区在buffer中的偏移

#define OFL_DATA_LEN					0x09                //数据区长度
#define OFL_CODE_OFFSET					0x11                //代码区在buffer中的偏移



// ISP Command IDs
#define ID_HANDSHAKE                    0x01
#define RESETCD                      0x02 
#define STEPCD                       0x03
#define READSTATUS                   0x04
#define RUNCD                        0x05
#define STOPCD                       0x06
#define READCOREREGISTER             0x09
#define WRITECOREREGISTER            0x10
#define READMEMORY                   0x11
#define WRITEMEMORY                  0x12
#define READAP                       0x13
#define WRITEAP                      0x14
#define READDP                       0x15
#define WRITEDP                      0x16

#define ID_DL_PRG_INTF                      0x37            //下载烧录接口
#define ID_DL_CONFIG                        0x17            //帧长度
#define ID_DL_USERHEX                       0x18            //1024
#define ID_DL_USERHEX_END                   0x1D        //用户程序下载结束

#define RESETSEQUENDECD                     0x1A

#define ID_DL_TIMING                        0x19            //1024 时序下载
#define ID_DL_TIMING_START                  0x1B            //时序下载开始    
#define ID_DL_TIMING_END                    0x1C	        //时序下载结束
#define ID_READ_TIMING_INFO                 0x28            //读芯片信息
#define ID_DL_TIMING_INFO                   0x27


//#define DEBCONFIGDL                  0x1E
//#define DEBHEXDOWNLOAD               0x1F

#define ID_FULL_ERASE                   0x20       //擦除
#define ID_CHECK_EMPTY                  0x21       //查空
#define ID_PRG_HEX                      0x22       //编程
#define ID_VERIFY                       0x23       //校验     
#define ID_ENCRYPT                      0x24       //加密
#define ID_READ_CONFIG                  0x25       //读配置字
#define ID_PRG_CONFIG                    0x26


#define ID_READ_CHIPID                  0x29       //读芯片ID
#define ID_READ_FLASH                      0x2A        //读flash  帧长度 1024
#define ID_DL_SERIALNUM              0x2B        //下载序列号代码

#define READCHIPCHK                  0x2D

#define SETSPEED                     0xD1
//#define READSPEED                    0xD2
#define READDEVICESTATUS             0xD2
//#define READVOLTAGE                  0xD4
#define SETPOWER                     0xD6
#define ID_READ_BOOT_VERSION          0xD7     //读boot版本
#define ID_DL_BOOT_START                    0xD8 //boot下载开始
  
#define ID_DL_BOOT_HEX                     0xD9    //发送新boot'      //1024
#define ID_DL_BOOT_HEXEND                 0xDA  //bootloader下载结束       
#define SWITCHCLASS                  0xDB
#define DEBUGEN                      0xDC
#define READSTDEVICEID               0xDD

#define ID_DL_OFFLINE_PRJ_INFO                          0x2E    //脱机方案下载
#define ID_READ_OFFLINE_PRJ_NUM                         0x2F    //读方案数量
#define ID_READ_OFFLINE_PRJ_INFO                        0x30    //读方案信息
#define ID_DL_OFFLINE_PRJ_TIMING_START                  0x31    //脱机方案时序下载开始
#define ID_DL_OFFLINE_PRJ_TIMING_ING                    0x32    //脱机方案时序下载  1024
#define ID_DL_OFFLINE_PRJ_TIMING_END                    0x33    //脱机方案时序结束
#define ID_DL_OFFLINE_PRJ_CONFIG                        0x34    //脱机方案配置字下载
#define ID_DL_OFFLINE_PRJ_HEX                           0x35    //脱机方案用户HEX   1024
#define ID_DL_OFFLINE_PRJ_HEX_END                       0x36    //脱机方案用户HEX下载完成



#endif

#ifndef __BURNER_DEF_H__
#define __BURNER_DEF_H__  


//编程接口定义
typedef enum 
{
    PRG_INTF_ISP = 0x52000000 ,         //isp烧录
    PRG_INTF_SWD = 0x52000001 ,         //swd烧录
    PRG_INTF_BOOTISP = 0x52000002,          //bootisp烧录
} prog_intf_type_t;   


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
#define PROG_FRAME_HEAD                         0xBADCCDAB  
#define PROG_FRAME_MAX_SIZE                     0x400               //帧长度 1024 
#define PROG_DATA_OFFSET              0x08                //数据区在buffer中的偏移
#define FRAME_HEADER_LEN                        8                   //协议头长度 

// ESLINKII Command IDs
#define ID_HANDSHAKE                        0x01

#define ID_READ_OFL_VERSION                0xD7    //读固件版本
#define ID_DL_OFL_START                    0xD8    //脱机工程下载开始   
#define ID_DL_OFL_HEX                      0xD9    //下载脱机工程HEX'  （1024）
#define ID_DL_OFL_HEX_END                  0xDA    //脱机下载结束 

#define ID_READ_SERIAL_NUMBER               0xDE    //读产品序列号
#define ID_DL_SERIAL_NUMBER                 0xDF    //下载产品序列号

//#define ID_DL_TIMING                        0x19    //时序下载(1024)
//#define ID_DL_TIMING_START                  0x1B    //时序下载开始    
//#define ID_DL_TIMING_END                    0x1C  //时序下载结束
#define ID_READ_TIMING_INFO                 0x28    //读芯片信息
#define ID_DL_TIMING_INFO                   0x27    //下载芯片信息
#define ID_READ_CHIP_CHKSUM                 0x2D    //读芯片校验和

#define ID_DL_PRG_INTF                      0x37    //下载烧录接口
#define ID_DL_CONFIG_WORD                   0x17    //下载配置字
#define ID_DL_CONFIG_WORD_END               0x3A    //配置字下载完成
#define ID_DL_USERHEX                       0x18    //下载用户HEX(1024)
#define ID_DL_USERHEX_END                   0x1D    //用户HEX下载结束
#define ID_DL_SERIALNUM                     0x2B    //下载序列号代码
#define ID_DL_OFL_SERIALNUM                 0x2C    //下载脱机序列号

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
#define ID_DL_OFFLINE_HEX                   0x35    //脱机方案用户HEX  （1024）
#define ID_DL_OFFLINE_HEX_END               0x36    //脱机方案用户HEX下载完成
#define ID_DL_OFFLINE_END                   0x3C    //脱机方案下载完成///////////////////////////////////////////////
#define ID_RTC_START_SELF_CALIBRATE         0x3D
#define ID_RTC_WRITE_CALIBRATE              0x3E      //写RTC自校正值
#define ID_RTC_READ_CALIBRATE               0x3F      //读RTC自校正值  
#define ID_RTC_CALI                         0x40     //RTC调校
#define ID_RTC_CALI_VERIFY                  0x41     //RTC验证
#define ID_DL_USERHEX_START                 0x42     //下载用户程序开始
#endif

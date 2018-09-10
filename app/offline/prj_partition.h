#ifndef OFFLINE_PRJ_MANAGER_H_
#define OFFLINE_PRJ_MANAGER_H_




/* 
    脱机工程烧录状态
*/
typedef enum 
{    
    STATE_WR_INFO,  
    STATE_WR_TIMING_START,
    STATE_WR_TIMING,
    STATE_WR_TIMING_END,
    STATE_WR_CONFIG,    
    STATE_WR_IMAGE,
}offline_state_t;

#define OFFLINE_EOK            0x00
#define OFFLINE_ERR_OPEN_FILE       0x01        //打开文件错误
#define OFFLINE_ERR_WRITE_FILE       0x02      //写错误
#define OFFLINE_ERR_READ_FILE       0x03      //读错误
#endif

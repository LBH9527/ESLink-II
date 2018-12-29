#ifndef __M939_ISP_H
#define __M939_ISP_H

#define FC_READ_CMD          0x00    //读取fc_cmd数据
#define FC_READ_PA           0x01
#define FC_READ_PLD          0x02
#define FC_READ_PHD          0x03
#define FC_READ_STA          0X05
#define FC_READ_RA           0X06
#define FC_READ_RD           0X07

#define FC_WRITE_CMD            0X40        //写入指令
#define FC_WRITE_PA             0X41        //写入位置
#define FC_WRITE_PLD            0X42        //编程数据
#define FC_WRITE_PHD            0X43        //编程数据
#define FC_WRITE_RA             0X46   

#define ISP_UNLOCK              0x10        //解锁        
#define ISP_LOCK                0x11        //上锁
#define ISP_CHECK_UNLOCK        0x12        //检查状态
#define ISP_PASS                0x7F        //切换成swd模式，


#endif

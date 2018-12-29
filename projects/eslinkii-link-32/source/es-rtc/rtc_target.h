#ifndef __RTC_TARGET_H__
#define __RTC_TARGET_H__  



//调校前温度寄存器地址
#define RTC_TEMP_BEFORE_CALI_ADDR       0x20000
//调校后温度寄存器地址
#define RTC_TEMP_AFTER_CALI_ADDR        0x20400
//调校前或调校后标志
#define RTC_TEMP_CALI_FLAG_ADDR         0x20800
#define RTC_TEMP_CALI_FLAG              0x12345678

#define RTC_INFO_BASC_ADDR              0X1000
#define RTC_INFO_SIZE                   32
#define RTC_TEMP_TBDR_ADDR              0x100C
//硬件补偿温度边界值寄存器
#define RTC_TEMPBDR_OFFSET              0x02
//温度边界值寄存器
#define RTC_TEMP_TBDR_OFFSET            3
//硬件补偿低温四次项系数地址
#define RTC_LTCAR_OFFSET                4

//硬件补偿低温二次项系数地址
#define RTC_LTCCR_OFFSET                6
//硬件补偿高温四次项系数地址  
#define RTC_HTCAR_OFFSET                8 
//硬件补偿高温二次项系数地址  
#define RTC_HTCCR_OFFSET                10
//温感标定点温度提取值寄存器
#define RTC_TEMP_TCALBDR_OFFSET         22 
#define RTC_TEMP_TCALBDR_INV_OFFSET     23 




#define INFO_TEMPT_ADDR                     0x0348
#define RTC_TEMP_TCALBDR_ADDR               0x1058


#endif

        
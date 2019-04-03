/**
  **************************************************************************************
  * @file    rtc_calibrate.c
  * @brief   rtc
  *
  * @version V0.01
  * @data    4/3/2018
  * @author  Eastsoft AE Team
  * @note   采用定时器中断的方式，在中断中用GPIO输出秒冲。16位的TIMER无法输出频率为1s的PWM
  *
  * Copyright (C) 2018 Shanghai Eastsoft Microelectronics Co., Ltd. ALL rights reserved.
  *
  **************************************************************************************
  */
#include "eslink.h"
#include "errno.h"
#include "es_common.h" 
#include "eslink_gpio.h"   
#include "fsl_ftm.h"   
#include "eeprom_port.h"
#include "program_port.h"
#include "isp_prog_intf.h"
#include "rtc_target.h" 
#include "rtc_calibrate.h"
#include "target_config.h" 

#if ESLINK_RTC_ENABLE 
//ES_RTC_IN  目标芯片RTC 输入，采用PWM输入捕获，测量时间
//RTC_OUT   pwm输出，接校准台
//CLK_IN    timer 时钟输入

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The Flextimer base address/channel used for board */
#define FTM_BASEADDR FTM1
#define FTM_IRQ_NUM FTM1_IRQn
//输入捕获
/* FTM channel used for input capture */
#define FTM_INPUT_CAPTURE_CHANNEL kFTM_Chnl_1
/* Interrupt to enable and flag to read */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl1InterruptEnable

//RTC调校状态
typedef enum  {
    STATE_CALI_START = 0,   //调校前
    STATE_CALI_ING,
    STATE_CALI_END,
}state_t;
static state_t state = STATE_CALI_START;

static uint32_t rtc_self_cali_freq;     //自校正频率值
static uint8_t rtc_self_cali_flag;      //自校正标志 
static float target_freq_ppm;        //目标芯片频率偏差值
static uint32_t time_cnt;               // 校正计数 
static uint32_t capture_cnt0;           //目标芯片RTC触发脉冲 捕获值
static uint32_t capture_cnt1;           //目标芯片RTC触发脉冲 捕获值
static uint32_t time_over_cnt;          //溢出计数
static uint8_t trig_count;              //捕获触发计数
static uint32_t capture_cnt[9];         //目标芯片RTC 1Hz时钟脉冲计数 数组

static void Init_FTM(void)
{
    ftm_config_t ftmConfigStruct;

    FTM_GetDefaultConfig(&ftmConfigStruct);
//    ftmConfigStruct.prescale = kFTM_Prescale_Divide_64;
    FTM_Init(FTM_BASEADDR, &ftmConfigStruct); 
    FTM_SetupInputCapture(FTM_BASEADDR, FTM_INPUT_CAPTURE_CHANNEL, kFTM_FallingEdge, 0); 
    FTM_SetTimerPeriod(FTM_BASEADDR, 0xffff);  
//    //定时器溢出中断
//    FTM_EnableInterrupts(FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable); 
//    EnableIRQ(FTM_IRQ_NUM);  
//    FTM_StartTimer(FTM_BASEADDR, kFTM_ExternalClock); 
////     FTM_StartTimer(FTM_BASEADDR, kFTM_SystemClock); 

            
    NVIC_DisableIRQ(FTM_IRQ_NUM);
    FTM_StopTimer(FTM_BASEADDR);    
}
static void rtc_gpio_init(void)
{
    PIN_RTC_OUT_GPIO->PSOR = 1 << PIN_RTC_OUT_BIT;       //out  high
    PIN_RTC_OUT_GPIO->PDDR |= (1U << PIN_RTC_OUT_BIT); /*!< Enable target LED_RED */ 
}
static void rtc_gpio_uninit(void)
{
    PIN_RTC_OUT_GPIO->PDDR &= ~(1 << PIN_RTC_OUT_BIT);            /* Input */
}
#define RTC_OUT_TOGGLE() \
    PIN_RTC_OUT_GPIO->PTOR = 1 << PIN_RTC_OUT_BIT               //翻转
//开始自校正脉冲   
static void start_self_cali(void)
{
     rtc_self_cali_flag = 0x55;     
}
//停止自校正脉冲
static void stop_self_cali(void)
{
     rtc_self_cali_flag = 0;        
}


//rtc 初始化
void rtc_Init(void)
{       
    Init_FTM(); 
    rtc_gpio_uninit();
    stop_self_cali();     
//    rtc_self_cali_freq =  9999853;
}
//rtc输出
void rtc_pwm_out(uint8_t mode)
{
    if(mode == ENABLE)
    {
        rtc_gpio_init();
        start_self_cali();
        time_cnt = 0;
        FTM_SetTimerPeriod(FTM_BASEADDR, 0xC34F);   //5ms
        //定时器溢出中断
        FTM_EnableInterrupts(FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable); 
        //关闭捕获中断
        FTM_DisableInterrupts(FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE); 
        
        //开启定时器中断
        EnableIRQ(FTM_IRQ_NUM);  

        FTM_StartTimer(FTM_BASEADDR, kFTM_ExternalClock);          
    }          
    else
    {
        rtc_self_cali_flag = 0;
        rtc_gpio_uninit();
        //关闭中断
        DisableIRQ(FTM_IRQ_NUM);        
        FTM_StopTimer(FTM_BASEADDR);    
    }             
}

static uint32_t abs(uint32_t x1, uint32_t x2)
{
    if (x1 >= x2)
  {
    return(x1 - x2);
  }
  else
  {
    return(x2 - x1);
  }  
}

//rtc调校开始
//1、烧录RTC调校程序。
//2、启动定时器，测量目标芯片的RTC精度。
static error_t rtc_calibration_start(void)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t failaddr;
    uint32_t faildata;
    uint8_t result;  
    uint32_t reg_temp[6] = {0x00};    //寄存器临时变量
    uint32_t rtc_info_reg[RTC_INFO_SIZE];          //rtc info寄存器值
//    uint32_t temp1, temp2;
    
    //ISP编程
    ret = isp_prog_intf.prog_init();      
    if(ERROR_SUCCESS != ret)
        return ret;
    ret = isp_prog_intf.erase_chip(0);    
    if(ERROR_SUCCESS != ret)
        return ret;
    ret = isp_prog_intf.check_empty(&failaddr, &faildata);
    if(ERROR_SUCCESS != ret)
        return ret;
    ret = isp_prog_intf.program_private(NULL);
    if(ERROR_SUCCESS != ret)
        return ret;
    result = isp_read_config(INFO_TEMPT_ADDR, reg_temp, 6);
    if(result != TRUE)
        return ERROR_RTC_CALI_START;
    //判断温感标定温度值是否正确
#if 0
    temp1 = (reg_temp[0] >> 16) & 0x0000ffff;
    temp2 = reg_temp[0]  & 0x0000ffff;
    if( (temp1 + temp2) != 0xffff )
        return ERROR_RTC_DATA_FORMAT;
    if( (reg_temp[2] + reg_temp[4]) != 0xFFFFFFFF)
        return ERROR_RTC_DATA_FORMAT;
#endif
    //调教前写入数据
    //读info寄存器
    result = isp_read_config(RTC_INFO_BASC_ADDR, rtc_info_reg, RTC_INFO_SIZE);
    rtc_info_reg[RTC_TEMP_TBDR_OFFSET] = reg_temp[0];
    rtc_info_reg[RTC_TEMP_TCALBDR_OFFSET] = reg_temp[2];
    rtc_info_reg[RTC_TEMP_TCALBDR_INV_OFFSET] = reg_temp[4];
    result = rtc_info_erase();
    result = isp_program_config(RTC_INFO_BASC_ADDR, rtc_info_reg, 24,  &failaddr);
    if(result != TRUE)
        return ERROR_RTC_CALI_START;
    
    es_set_trget_power(TRGET_POWER_DISABLE);
    es_delay_ms(40);
    es_set_trget_power(TRGET_POWER_ENABLE); 
    
    eslink_set_target_reset_run(10);  
    
    return ret;
}

//rtc调校
//测量目标芯片RTC精度
//方法：从第3个脉冲开始采样，如果连续3个脉冲之间相差在0.2ppm，则取算术平均值当做结果，
//      测试脉冲如果超过8个，还没有符合要求，则返回调校失败。
static error_t rtc_calibration(void)
{
    uint32_t cnt_ppm;
    uint32_t target_avg_cnt;        //目标芯片脉冲数平均值
    while(trig_count <= 8)
    {            
        if( trig_count >= 0x05 )            //连续3个脉冲互差都小于0.2ppm
        {
            //脉冲触发数比 捕获脉冲数组 多1
            cnt_ppm =  abs(capture_cnt[trig_count - 3], capture_cnt[trig_count -2]) ;
            if(cnt_ppm  > 2)
                continue;
            cnt_ppm =  abs(capture_cnt[trig_count - 3], capture_cnt[trig_count - 1 ]) ;
            if(cnt_ppm  > 2)
                continue;
            cnt_ppm =  abs(capture_cnt[trig_count - 1], capture_cnt[trig_count - 2]) ; 
            if(cnt_ppm  > 2)
                continue;            
            target_avg_cnt = (capture_cnt[trig_count -2] + capture_cnt[trig_count -1] + \
                                capture_cnt[trig_count - 3] ) / 3  ;
            target_freq_ppm =  (1 - (float)target_avg_cnt/(float)rtc_self_cali_freq) * 1000000 ;                   
            
            return ERROR_SUCCESS;
        }         
    } 
    //调校时间大于8s，返回错误
    trig_count = 0;
    FTM_StopTimer(FTM_BASEADDR);   
    return  ERROR_RTC_CALI_TIMEOUT;                                         
}

/*
*   y0 - y = a(T - T1)^2    
*   y :当前频率偏差   y0:顶点频率偏差 a：抛物线偏差   T:当前温度  T1：顶点温度
*   y =  target_freq_ppm    a:LTCCR HTCCR
*
*/
static error_t rtc_calibration_end(void)
{
    uint8_t result;  
    error_t ret = ERROR_SUCCESS;  
    uint32_t rtc_info_reg[RTC_INFO_SIZE];          //rtc info寄存器值
    uint32_t current_temp;              //当前温度     
    float parabolic_vertex_ppm;         //顶点频率偏差
    int32_t parabolic_vertex_ppm_reg;
    float rtc_temp_bdr;                 //硬件补偿温度边界值
    uint32_t parabolic_open_reg;        //抛物线开口寄存器值
    float parabolic_open;               //抛物线开口       
    uint32_t parabolic_vertex_temp_reg; //抛物线顶点温度寄存器
    float parabolic_vertex_temp;        //抛物线顶点温度    
    uint32_t reg_temp[2] = {0xffffffff, 0xffffffff};    //寄存器临时变量
    float temp;
//    uint32_t fail;
    
    //ISP编程
    ret = isp_prog_intf.prog_init();      
    if(ERROR_SUCCESS != ret)
        return ret;
 
    //当前温度
    isp_read_code(RTC_TEMP_BEFORE_CALI_ADDR, reg_temp, 2);
//    if((reg_temp[0] + reg_temp[1]) != 0xFFFFFFFF)
//         return ERROR_RTC_DATA_FORMAT;
    current_temp =  reg_temp[0];
    
    //读info寄存器
    isp_read_config(RTC_INFO_BASC_ADDR, rtc_info_reg, RTC_INFO_SIZE);
    rtc_temp_bdr = (float)(rtc_info_reg[RTC_TEMPBDR_OFFSET] & 0x0000ffff) ;
    
    if( current_temp >=  rtc_temp_bdr)
    {
        parabolic_open_reg =  rtc_info_reg[RTC_HTCCR_OFFSET] & 0x0000ffff;
        parabolic_vertex_temp_reg = rtc_info_reg[RTC_HTCAR_OFFSET] & 0x0000ffff;
    }
    else
    {
        parabolic_open_reg =  rtc_info_reg[RTC_LTCCR_OFFSET] & 0x0000ffff;
        parabolic_vertex_temp_reg = rtc_info_reg[RTC_LTCAR_OFFSET] & 0x0000ffff;
    }
    parabolic_open =  (float)parabolic_open_reg / 524288;      //2^19
    parabolic_vertex_temp =  (float)parabolic_vertex_temp_reg / 256; //2^8
    
    temp =  (float)current_temp/256 - parabolic_vertex_temp; //T-T1
    temp *= temp;   //(T-T1)^2
    temp *= parabolic_open;  //a(T - T1)^2    
    parabolic_vertex_ppm =  target_freq_ppm +  temp ; //y0 = y + a(T - T1)^2 ;   
    
    //保存计算结果到RTC info
    parabolic_vertex_ppm *= 256  ;
    parabolic_vertex_ppm_reg = (int32_t) parabolic_vertex_ppm;
    rtc_info_reg[16] =  parabolic_vertex_ppm_reg  & 0x0000ffff;
    rtc_info_reg[17] = parabolic_vertex_ppm_reg  & 0x0000ffff;
    rtc_info_reg[18] = 5888;
    rtc_info_reg[19] = parabolic_vertex_ppm_reg  & 0x0000ffff;
    rtc_info_reg[20] = current_temp & 0x0000ffff;
    rtc_info_reg[21] = 0xffffffff;      //调校后温度
//    rtc_info_reg[22] = 0xFFFEE0B6 ;               //温感标定点温度提取值寄存器
//    rtc_info_reg[23] = 0x00011F49 ;               //温感标定点温度提取值寄存器反码
    
    result = rtc_info_erase();
    if(result != TRUE)
        return ERROR_RTC_CALI_PROG ;
    result = isp_program_config(RTC_INFO_BASC_ADDR, rtc_info_reg, RTC_INFO_SIZE,  NULL);
    if(result != TRUE)
        return ERROR_RTC_CALI_PROG ;
        
    //写调校后标志    
    reg_temp[0] =  RTC_TEMP_CALI_FLAG; 
    reg_temp[1] = 0xffffffff;    
    result = isp_program_code(RTC_TEMP_CALI_FLAG_ADDR, reg_temp, 2, NULL);    
    if(result != TRUE)
        return ERROR_RTC_CALI_PROG ;       
    
    eslink_set_target_reset_run(20); 


    return ret;
}


//RTC调校处理

error_t rtc_calibration_handler(void)
{
    error_t ret = ERROR_SUCCESS;
    
    get_rtc_self_calibrate((uint8_t*)&rtc_self_cali_freq, 4); 
    if( (rtc_self_cali_freq >= 10000200) || (rtc_self_cali_freq <= 9999800) )
        return ERROR_RTC_SELF_CAIL;
    state = STATE_CALI_START;
    if(STATE_CALI_START ==  state)
    {
        ret = rtc_calibration_start();
        if(ret != ERROR_SUCCESS)
            return ret;
        FTM_SetTimerPeriod(FTM_BASEADDR, 0xffff);  

        //捕获中断
        FTM_EnableInterrupts(FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE); 
        //定时器溢出中断
        FTM_EnableInterrupts(FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable); 
        EnableIRQ(FTM_IRQ_NUM); 
        FTM_StartTimer(FTM_BASEADDR, kFTM_ExternalClock); 
        
        trig_count = 0;
        time_over_cnt = 0;
        capture_cnt0 = 0;
        capture_cnt1 = 0;
        
        state = STATE_CALI_ING;      
    }
    if( STATE_CALI_ING ==  state)
    {
        ret = rtc_calibration();
        if(ret ==  ERROR_SUCCESS)
            state = STATE_CALI_END;                   
    }
    if( STATE_CALI_END ==  state)
    {
        ret = rtc_calibration_end();  
            
    }    
    //捕获中断
    FTM_DisableInterrupts(FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE); 
    //定时器溢出中断
    FTM_DisableInterrupts(FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);     
    DisableIRQ(FTM_IRQ_NUM); 
    FTM_StopTimer(FTM_BASEADDR); 
    return ret; 
}

//调校验证
#define RTC_TARGET_FREQ_PPM_VALUE   1   //调校后的ppm 范围
error_t rtc_calibration_verify(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t result;  
    uint32_t rtc_info_reg[RTC_INFO_SIZE];          //rtc info寄存器值
    uint32_t reg_temp[2] = {0xffffffff, 0xffffffff};    //寄存器临时变量
//    uint32_t fail_temp;
    
    get_rtc_self_calibrate((uint8_t*)&rtc_self_cali_freq, 4); 
    if( (rtc_self_cali_freq >= 10000200) || (rtc_self_cali_freq <= 9999800) )
        return ERROR_RTC_SELF_CAIL;
        
    eslink_set_target_power_reset(40);
    es_delay_ms(5000);
    FTM_SetTimerPeriod(FTM_BASEADDR, 0xffff); 
    //捕获中断
    FTM_EnableInterrupts(FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE); 
    //定时器溢出中断
    FTM_EnableInterrupts(FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable); 
    EnableIRQ(FTM_IRQ_NUM); 
    FTM_StartTimer(FTM_BASEADDR, kFTM_ExternalClock); 
    trig_count = 0;
    time_over_cnt = 0;
    capture_cnt0 = 0;
    capture_cnt1 = 0;
    
    //计算调校后偏差    
    rtc_calibration();
    
    //捕获中断
    FTM_DisableInterrupts(FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE); 
    //定时器溢出中断
    FTM_DisableInterrupts(FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);     
    DisableIRQ(FTM_IRQ_NUM); 
    FTM_StopTimer(FTM_BASEADDR); 
    
    ret = isp_prog_intf.prog_init();      
    if(ERROR_SUCCESS != ret)
        return ret;
    //写调校后温度
    isp_read_code(RTC_TEMP_AFTER_CALI_ADDR, reg_temp, 2);  
//    if((reg_temp[0] + reg_temp[1]) != 0xffffffff)
//        return ERROR_RTC_DATA_FORMAT;
    //读info寄存器
    isp_read_config(RTC_INFO_BASC_ADDR, rtc_info_reg, RTC_INFO_SIZE);
    rtc_info_reg[21] =  reg_temp[0];
    result = rtc_info_erase();
    if(result != TRUE)
        return ERROR_RTC_CALI_PROG ;
    result = isp_program_config(RTC_INFO_BASC_ADDR, rtc_info_reg, RTC_INFO_SIZE,  NULL);
    if(result != TRUE)
        return ERROR_RTC_CALI_PROG ;     
     //频率偏差是否在范围内都要写调校后温度
    if( (target_freq_ppm > 0)  && (target_freq_ppm > RTC_TARGET_FREQ_PPM_VALUE) )
    {
        if(data)
        {
            *data     =((uint8_t *)&target_freq_ppm)[0];
            *(data+1) =((uint8_t *)&target_freq_ppm)[1];
            *(data+2) =((uint8_t *)&target_freq_ppm)[2];
            *(data+3) =((uint8_t *)&target_freq_ppm)[3];            
        }    
        return ERROR_RTC_CALI_VERIFY;
    }
    else if( (target_freq_ppm < 0) && ((0-target_freq_ppm) > RTC_TARGET_FREQ_PPM_VALUE))
    {
        if(data)
        {
            *data     =((uint8_t *)&target_freq_ppm)[0];
            *(data+1) =((uint8_t *)&target_freq_ppm)[1];
            *(data+2) =((uint8_t *)&target_freq_ppm)[2];
            *(data+3) =((uint8_t *)&target_freq_ppm)[3];            
        }     
        return ERROR_RTC_CALI_VERIFY; 
    }              
    eslink_set_target_power_reset(40);
    return ERROR_SUCCESS;
}

void FTM1_IRQHandler(void)
{
    //捕获中断
    if ((FTM_GetStatusFlags(FTM_BASEADDR) & kFTM_Chnl1Flag) == kFTM_Chnl1Flag)
    {
        /* Clear interrupt flag.*/
        FTM_ClearStatusFlags(FTM_BASEADDR, kFTM_Chnl1Flag);
        trig_count++;
        if(trig_count%2 == 0)
        {
            capture_cnt0 = FTM_BASEADDR->CONTROLS[FTM_INPUT_CAPTURE_CHANNEL].CnV;
         //   capture_cnt[trig_count-1] =  0x10000 - capture_cnt1 + capture_cnt0 + ((time_over_cnt-1) << 16) ;    
            capture_cnt[trig_count-1] = (time_over_cnt << 16) - capture_cnt1 + capture_cnt0  ;  
        }
            
        else
        {
            capture_cnt1 = FTM_BASEADDR->CONTROLS[FTM_INPUT_CAPTURE_CHANNEL].CnV;
          //  capture_cnt[trig_count-1] =  (time_over_cnt << 16) - capture_cnt0 + capture_cnt1 + ((time_over_cnt-1) << 16) ;    
            capture_cnt[trig_count-1] = (time_over_cnt << 16) - capture_cnt0 + capture_cnt1 ;
        }   
        time_over_cnt = 0;
    }
    //溢出中断
    if(FTM_BASEADDR->SC & FTM_SC_TOF_MASK)
    {
        FTM_BASEADDR->SC &= ~FTM_SC_TOF_MASK;
        
        if(  0x55 == rtc_self_cali_flag ) 
        {
            time_cnt++ ;
            if( time_cnt % 100 == 0)
            {
                RTC_OUT_TOGGLE();
                time_cnt = 0; 
            }                 
        }
        else
        {
            time_over_cnt ++;
        }
    }
}

#endif



#include "es_common.h"
#include "eslink_gpio.h"
#include "fsl_ftm.h"
#include "rtc.h"

//ES_RTC_IN  目标芯片RTC 输入，采用PWM输入捕获，测量时间
//RTC_OUT   pwm输出，接校准台
//CLK_IN    timer 时钟输入
#define RTC_OUT_HZ      10       // 100ms
#define RTC_PULSE_WIDTH 50U;        //占空比
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The Flextimer base address/channel used for board */
#define FTM_BASEADDR FTM1
/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk)/1)
//#define FTM_SOURCE_CLOCK    20000000

/* Interrupt number and interrupt handler for the FTM base address used */
#define FTM_INTERRUPT_NUMBER FTM0_IRQn
#define FTM_LED_HANDLER FTM0_IRQHandler

/* Interrupt to enable and flag to read */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl6InterruptEnable
#define FTM_CHANNEL_FLAG kFTM_Chnl6Flag





static void Init_FTM(void)
{
    ftm_config_t ftmConfigStruct;
    ftm_chnl_pwm_signal_param_t ftmParam;
    ftm_pwm_level_select_t pwmLevel = kFTM_LowTrue;
    /*
     * ftmConfigStruct.prescale = kFTM_Prescale_Divide_1;
     * ftmConfigStruct.bdmMode = kFTM_BdmMode_0;
     * ftmConfigStruct.pwmSyncMode = kFTM_SoftwareTrigger;
     * ftmConfigStruct.reloadPoints = 0;
     * ftmConfigStruct.faultMode = kFTM_Fault_Disable;
     * ftmConfigStruct.faultFilterValue = 0;
     * ftmConfigStruct.deadTimePrescale = kFTM_Deadtime_Prescale_1;
     * ftmConfigStruct.deadTimeValue = 0;
     * ftmConfigStruct.extTriggers = 0;
     * ftmConfigStruct.chnlInitState = 0;
     * ftmConfigStruct.chnlPolarity = 0;
     * ftmConfigStruct.useGlobalTimeBase = false;
     */
    FTM_GetDefaultConfig(&ftmConfigStruct);
    ftmConfigStruct.prescale = kFTM_Prescale_Divide_128;
    FTM_Init(FTM_BASEADDR, &ftmConfigStruct);

    /* FTM config */
    ftmParam.chnlNumber = kFTM_Chnl_0;
    ftmParam.level = pwmLevel;
    ftmParam.dutyCyclePercent = RTC_PULSE_WIDTH;
    ftmParam.firstEdgeDelayPercent = 0U;                                                            
    
    FTM_SetupPwm(FTM_BASEADDR, &ftmParam, 1U, kFTM_EdgeAlignedPwm, RTC_OUT_HZ, FTM_SOURCE_CLOCK);

   FTM_StartTimer(FTM_BASEADDR, kFTM_SystemClock);
// FTM_StartTimer(FTM_BASEADDR, kFTM_ExternalClock);
    
}


void rtc_Init(void)
{
    Init_FTM();

}

//rtc 校准
//pwm输出 ，频率为1s
void rtc_calibration(void)
{
    //rtc out
     FTM_StartTimer(FTM_BASEADDR, kFTM_SystemClock);

}

//目标芯片RTC输入，得到频率误差
void  rtc_in(void)
{
     

}
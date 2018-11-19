
#include "es_common.h"
#include "eslink_gpio.h"
#include "fsl_adc16.h" 

/* DAC base address */
#define DAC_BASEADDR DAC0

//电压检测
void voltage_detect()
{


}

//
void adc_init(void)
{
    adc16_config_t adc16_condig;
    
    adc16_condig.referenceVoltageSource =  kADC16_ReferenceVoltageSourceVref;
//    adc16_condig.clockSource = 
//    ADC16_Init( DAC_BASEADDR,   );
}
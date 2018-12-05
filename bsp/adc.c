#include "es_common.h"
#include "eslink_gpio.h"
#include "fsl_adc16.h" 
#include "fsl_pdb.h"

#define ADC_BASEADDR ADC0
#define ADC_USER_CHANNEL 14U
#define ADC_CHANNEL_GROUP 0U
#define ADC_CLOCK_SOURCE kADC16_ClockSourceAlt0
#define ADC_CLOCK_DIVIDER kADC16_ClockDivider4


#define PDB_BASE PDB0
#define PDB_TRIGGER_CHANNEL0 0U

#define PDB_PRETRIGGER_CHANNEL0_MASK 1U



#define PDB_MODUL_VALUE 2000U
#define PDB_INT_VALUE 1500U
#define PRETRIGGER_DELAY_VALUE 500U

//电压检测
void voltage_detect()
{
    ADC16_GetChannelConversionValue(ADC_BASEADDR, ADC_CHANNEL_GROUP);

}

void get_adc_average(void)
{


}
//
void adc_init(void)
{
    adc16_config_t adc16_config;
    adc16_channel_config_t adc16_channel_config;
    
    /*
     * adc16_config.referenceVoltageSource = kADC16_ReferenceVoltageSourceVref;
     * adc16_config.clockSource = kADC16_ClockSourceAsynchronousClock;
     * adc16_config.enableAsynchronousClock = true;
     * adc16_config.clockDivider = kADC16_ClockDivider8;
     * adc16_config.resolution = kADC16_ResolutionSE12Bit;
     * adc16_config.longSampleMode = kADC16_LongSampleDisabled;
     * adc16_config.enableHighSpeed = false;
     * adc16_config.enableLowPower = false;
     * adc16_config.enableContinuousConversion = false;
     */
    adc16_config.referenceVoltageSource =  kADC16_ReferenceVoltageSourceVref;
    /* Config ADC */
    adc16_config.clockSource = ADC_CLOCK_SOURCE;
    adc16_config.clockDivider = ADC_CLOCK_DIVIDER;
    adc16_config.enableAsynchronousClock = false;
    adc16_config.enableHighSpeed = true;

    /* Init ADC */
    ADC16_Init(ADC_BASEADDR, &adc16_config);
    
    adc16_channel_config.enableInterruptOnConversionCompleted = false;
    adc16_channel_config.channelNumber = ADC_USER_CHANNEL;
    ADC16_SetChannelConfig(ADC_BASEADDR, ADC_CHANNEL_GROUP, &adc16_channel_config);
}


static void Init_PDB(void)
{
    pdb_config_t pdbConfigStruct;
    pdb_adc_pretrigger_config_t pdbAdcPreTriggerConfigStruct;

    /*
     * pdbConfigStruct.loadValueMode = kPDB_LoadValueImmediately;
     * pdbConfigStruct.prescalerDivider = kPDB_PrescalerDivider1;
     * pdbConfigStruct.dividerMultiplicationFactor = kPDB_DividerMultiplicationFactor1;
     * pdbConfigStruct.triggerInputSource = kPDB_TriggerSoftware;
     * pdbConfigStruct.enableContinuousMode = false;
     */
    PDB_GetDefaultConfig(&pdbConfigStruct);

    /* Config PDB */
    pdbConfigStruct.triggerInputSource = kPDB_TriggerInput8;

    PDB_Init(PDB_BASE, &pdbConfigStruct);
    PDB_EnableDMA(PDB_BASE, false);
    PDB_DisableInterrupts(PDB_BASE, kPDB_SequenceErrorInterruptEnable);

    /* Configure the delay interrupt. */
    PDB_SetModulusValue(PDB_BASE, PDB_MODUL_VALUE);
    PDB_SetCounterDelayValue(PDB_BASE, PDB_INT_VALUE);

    /* Configure the ADC Pre-Trigger. */
    PDB_SetADCPreTriggerDelayValue(PDB_BASE, PDB_TRIGGER_CHANNEL0, PDB_PRETRIGGER_CHANNEL0_MASK,
                                   PRETRIGGER_DELAY_VALUE); 
   
    pdbAdcPreTriggerConfigStruct.enablePreTriggerMask =   PDB_PRETRIGGER_CHANNEL0_MASK;
    pdbAdcPreTriggerConfigStruct.enableOutputMask = PDB_PRETRIGGER_CHANNEL0_MASK;
    PDB_SetADCPreTriggerConfig(PDB_BASE, PDB_TRIGGER_CHANNEL0, &pdbAdcPreTriggerConfigStruct);
    pdbAdcPreTriggerConfigStruct.enableBackToBackOperationMask = PDB_PRETRIGGER_CHANNEL0_MASK; 

    /* Load PDB registe */
    PDB_DoLoadValues(PDB_BASE);
}



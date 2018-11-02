//#include "ES_ICD.h"
#include "eslink.h"
#include "debug_def.h"
#include "hr7p169b_icd.h"

//uint16_t breakpoint_addr_buffer[BreakPointNum] = {0};       //端点地址数组
//uint16_t NoStopLab[256]; //110禁止停留区，用于c调试时停到库函数中找不到对应的源程序行问题
//uint8_t  Lab_Num;	//库函数个数。
//uint16_t Main_addr; //c程序中main的入口地址20111206
//uint8_t bReceiveStop = 0; 		//   receive stop command during c step or c stepover
ICD_TARGET_STATE target_state;
#define PIN_DELAY(n)    ES_DELAY_SLOW(n)
static void  nop_delay()//400ns
{
	__nop();
/*	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();   //800ns*/
}

static void icd_start(void)
{
    PIN_ISPSDA_SET();       
    PIN_DELAY(1);                       
    PIN_ISPCLK_SET(); 
    PIN_DELAY(1);                       
    PIN_ISPSDA_CLR();    
    PIN_DELAY(1);                       
    PIN_ISPCLK_CLR();    
    PIN_DELAY(1);       
}

static void icd_stop(void)
{
    PIN_ISPCLK_CLR();                   
    PIN_DELAY(1);                       
    PIN_ISPSDA_CLR();                   
    PIN_DELAY(1);                       
    PIN_ISPCLK_SET();                   
    PIN_DELAY(1);                       
    PIN_ISPSDA_SET();      
}
#define ICD_TIMEOUT     ((uint32_t)0x100)
static uint8_t icd_timeout_cb(void)
{
    icd_stop();  
    return FALSE;
}
static uint8_t icd_wait_ack(void)
{
    uint32_t timeout = ICD_TIMEOUT;  	
    
    PIN_ISPSDA_SET();
    nop_delay();//_nop_();    
    PIN_ISPCLK_SET(); 
    nop_delay();//_nop_(); 
    PIN_ISPSDA_OUT_DISABLE();
    while(PIN_ISPSDA_IN())   //等待应答
    {
        if((timeout--) == 0)
        {                  
             return icd_timeout_cb();
        }       
    }      
    PIN_ISPSDA_OUT_ENABLE();
	PIN_ISPCLK_CLR();
    return TRUE;
}
void icd_write_inst(uint8_t data)
{
    uint8_t i;
    for(i = 0; i < 8; i++)
    {		
    	PIN_ISPCLK_CLR();
	
		if(data & 0x80)
		{
			PIN_ISPSDA_SET();
		}
		else 
		{
			PIN_ISPSDA_CLR();
		}
		nop_delay();//_nop_();

   		PIN_ISPCLK_SET();
        nop_delay();//_nop_();
		data <<= 1;
    } 
}
uint8_t icd_write_data_and_parity(uint16_t data)
{
    uint8_t i;
    uint8_t count = 0;          //奇偶校验计算
    for(i = 0; i < 16; i++)
    {		
    	PIN_ISPCLK_CLR();
	
		if(data & 0x80)
		{
			PIN_ISPSDA_SET();
            count++;
		}
		else 
		{
			PIN_ISPSDA_CLR();
		}
		nop_delay();//_nop_();

   		PIN_ISPCLK_SET();
        nop_delay();//_nop_();
		data <<= 1;
    } 
    PIN_ISPCLK_CLR();
	nop_delay();
    if(count & 0x01)            //
        PIN_ISPSDA_SET();
    else
        PIN_ISPSDA_CLR();
//	nop_delay();
	PIN_ISPCLK_SET();
    
    return TRUE;
}
uint8_t icd_read_data_and_parity(uint16_t *read_data)
{
    uint8_t i;
	uint16_t data = 0;
	uint8_t count = 0;      //计算奇偶校验

	PIN_ISPSDA_OUT_DISABLE();//SDA 设置为输入
	for (i=0; i<16; i++)
	{            
        data <<= 1;         
		PIN_ISPCLK_SET();
		nop_delay();
		if(PIN_ISPSDA_IN()) 
		{ 
			data |= 0x01;
            count++;
		}
        PIN_ISPCLK_CLR();
		nop_delay();          
	}  
    	PIN_ISPCLK_CLR();
	nop_delay();
	
	PIN_ISPCLK_SET();
	nop_delay();

	for(i=0;i<3;i++)
	{
         if(PIN_ISPSDA_IN() == (count&0x01))
            break;
	}
	PIN_ISPCLK_CLR();
    nop_delay();
	PIN_ISPSDA_CLR();
	PIN_ISPSDA_OUT_ENABLE();
    
    if(i >= 3)
        return FALSE;
    *read_data = data;
    return TRUE; 


}

/***************************************ICD 命令*******************************/
uint8_t icd_control(uint8_t cmd_data)
{
    uint8_t check_ack;
    
    icd_start();
    icd_write_inst(cmd_data);
    check_ack = icd_wait_ack(); 
    if(check_ack != TRUE)
    {
        icd_stop();
        return FALSE;
    }
    icd_stop();     
    return TRUE;       
}

//ICD命令：写命令
// cmd_data : 指令码  write_data:要写入的数据
// 返回：TRUE 写入成功 FALSE：写入失败

uint8_t icd_send(uint8_t cmd_data, uint16_t write_data)	//icd_write
{  
    icd_start();
    icd_write_inst(cmd_data) ;
    if(icd_wait_ack() !=  TRUE)
        goto fail;        
    icd_write_data_and_parity(write_data); 
    if(icd_wait_ack() !=  TRUE)
        goto fail;    
    icd_stop();
    return TRUE;
fail:   
	icd_stop();
	return FALSE;
}


/////////////////////////////////////////////////////////////?????????,???????
//cmd_data：指令码     read_data：读到的数据
// 返回：TRUE 写入成功 FALSE：写入失败
uint8_t icd_recv(uint8_t cmd_data, uint16_t *read_data)
{
    icd_start();
    icd_write_inst(cmd_data) ;
    if(icd_wait_ack() !=  TRUE)
        goto fail;         
 
    if(icd_read_data_and_parity(read_data) != TRUE)
        goto fail;  
    icd_stop();
    return TRUE;
fail:   
	icd_stop();
	return FALSE;
 }
 
/*******************************************************************************
*   函 数 名: icd_halt_check
*   功能说明: halt查询
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
#define MAX_ICD_RETRY   50
uint8_t icd_halt_check(void)
{	
    uint16_t halt_data;
    uint8_t i;
    
	for(i=0; i<MAX_ICD_RETRY; i++)
	{
        if(icd_recv(HALT_CHECK_CMD, &halt_data) != TRUE) 
            return FALSE;  
        if(halt_data == 0x5aa5) 
            break;
        es_delay_ms(1);
	}
    if(i >= MAX_ICD_RETRY)	
		return FALSE;
    return TRUE	;
}
/*******************************************************************************
*   函 数 名: icd_halt_check
*   功能说明: 单步   2.4.1
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t icd_step(void)				//
{	
    if(target_state != ICD_HALT)
        return FALSE;    
    
	if(icd_control(ICD_STEP_CMD) != TRUE)
        return FALSE;
    
    target_state = ICD_RUN;            //目标芯片状态更新
    
    if(icd_halt_check() != TRUE)
        return FALSE;

    target_state = ICD_HALT;            //目标芯片状态更新
    return TRUE;	
}

/*******************************************************************************
*   函 数 名: icd_halt_check
*   功能说明: 步出	2.4.2
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t icd_step_out(void)
{
    if(target_state != ICD_HALT)
        return FALSE;
    
	if(icd_control(STEP_OUT_CMD) != TRUE)	
        return FALSE;
    
    target_state = ICD_RUN;            //目标芯片状态更新
    
    if(icd_halt_check() != TRUE)
        return FALSE;
    
    target_state = ICD_HALT;            //目标芯片状态更新
    return TRUE;	
}

/*******************************************************************************
*   函 数 名: icd_step_over
*   功能说明: 步越	2.4.3	  
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t icd_step_over(void)
{  
    if(target_state != ICD_HALT)
        return FALSE;
    
	if(icd_control(STEP_OVER_CMD) != TRUE)	
        return FALSE;
    
    target_state = ICD_RUN;            //目标芯片状态更新
    
    if(icd_halt_check() != TRUE)
        return FALSE;
    
    target_state = ICD_HALT;            //目标芯片状态更新
    return TRUE;	
 }
/*******************************************************************************
*   函 数 名: icd_run
*   功能说明: 运行	2.4.4	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t icd_run(void)
{
    if(target_state != ICD_HALT)
        return FALSE;
    
	if(icd_control(RUN_FLAG_CMD) != TRUE) 
        return FALSE;
    
    target_state = ICD_RUN;            //目标芯片状态更新
    return TRUE;	    
    
}

/*******************************************************************************
*   函 数 名: icd_halt
*   功能说明: 停止	2.4.5		
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t icd_halt(void)				
{     
//    if(target_state != ICD_RUN)
//        return FALSE;
    
    if(icd_control(ICD_HALT_CMD) != TRUE) 
        return FALSE;  
    
    if(icd_halt_check() != TRUE)
        return FALSE;
    
    target_state = ICD_HALT;            //目标芯片状态更新
    return TRUE;	 
}

/*******************************************************************************
*   函 数 名: icd_chip_reset
*   功能说明: 芯片复位	2.4.7		
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t icd_chip_reset(void)
{
    if(target_state != ICD_HALT)
        return FALSE; 
    
    if(icd_control(ICD_RESET_CMD) != TRUE) 
        return FALSE;
    
    target_state = ICD_HALT;            //目标芯片状态更新
    return TRUE;	
}

/***************************************ICD 读写数据*******************************/
//批量数据写操作 
//address:地址 size：16位数据长度 write_data:写入数据buf的地址
//uint8_t write_block_operation(uint16_t address,uint32_t size, uint16_t *write_buf)
uint8_t icd_write_str(uint16_t address,uint16_t *write_buf , uint32_t size )
{
    uint8_t i;
    uint8_t retry;
    
    retry = 30;
    for(i=0; i<retry; i++)
    {
        if(icd_send(SET_ADDR,address) != FALSE)
            break;
    }
    if(i >= retry)
        return FALSE;
	
    while(size--)
    {
        for(i=0; i<retry; i++)
        {
            if(icd_send(WR_DATA, *write_buf) != FALSE)
                break;
        }
        if(i >= retry)
            return FALSE; 
        write_buf++;
    }
    return TRUE;
}
//数据读操作.size：16位数据长度
//uint8_t read_block_operation(uint16_t address,uint32_t size, uint16_t *read_buf)
uint8_t icd_read_str(uint16_t address, uint16_t *read_buf, uint32_t size)
{
    uint8_t i;
    uint8_t retry;
    
    retry = 30;
    for(i=0; i<retry; i++)
    {
        if(icd_send(SET_ADDR,address) != FALSE)
            break;
    }
    if(i >= retry)
        return FALSE;
    
    while(size--)
    {
        for(i=0; i<retry; i++)
        {
            if(icd_send(RD_DATA, *read_buf) != FALSE)
                break;
        }
        if(i >= retry)
            return FALSE; 
        read_buf++;
    }
    return TRUE;    
}

////写一个数据操作
////address:地址  write_data:写入数据的地址
//uint8_t icd_write_data_operation(uint16_t address, uint16_t write_data)
//{
//    uint8_t error;
//    
//    error = icd_send(SET_ADDR,address);            //set_addr
//	if(error != TRUE)
//       return error;
//    error = icd_send(WR_DATA, write_data);	            //wr data
//    if(error != TRUE)
//       return error;  
//  
//    return TRUE;
//}
//uint8_t icd_read_data_operation(uint16_t address, uint16_t *read_data)
//{
//    uint8_t error;
//    
//    error = icd_send(SET_ADDR,address);            //set_addr
//	if(error != TRUE)
//       return error;
//    
//    error = icd_read(RD_DATA, read_data);
//    if(error != TRUE)
//       return error;  
//    
//    return TRUE;    
//}
/*******************************************************************************/





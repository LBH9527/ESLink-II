
#include "eslink.h"
#include "hr7p169b_icd.h"
#define ICD_NACK    1
#define ICD_ACK     0
//uint16_t breakpoint_addr_buffer[BreakPointNum] = {0};       //端点地址数组
//uint16_t NoStopLab[256]; //110禁止停留区，用于c调试时停到库函数中找不到对应的源程序行问题
//uint8_t  Lab_Num;	//库函数个数。
//uint16_t Main_addr; //c程序中main的入口地址20111206
//uint8_t bReceiveStop = 0; 		//   receive stop command during c step or c stepover


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
    
    PIN_ISPSDA_OUT_DISABLE();
    PIN_ISPCLK_CLR();
    PIN_DELAY(1);   
    PIN_ISPCLK_SET();
    PIN_DELAY(1);   
    while(PIN_ISPSDA_IN() != ICD_ACK)   //等待应答
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
		PIN_DELAY(1);//_nop_();

   		PIN_ISPCLK_SET();
        PIN_DELAY(1);//_nop_();
		data <<= 1;
    } 
    PIN_ISPCLK_CLR(); 
	PIN_DELAY(1); 
    PIN_ISPSDA_CLR();
}
uint8_t icd_write_data_and_parity(uint16_t data)
{
    uint8_t i;
    uint8_t count = 0;          //奇偶校验计算
    for(i = 0; i < 16; i++)
    {		
    	PIN_ISPCLK_CLR();
	
		if(data & 0x8000)
		{
			PIN_ISPSDA_SET();
            count++;
		}
		else 
		{
			PIN_ISPSDA_CLR();
		}
		PIN_DELAY(1);//_nop_();

   		PIN_ISPCLK_SET();
        PIN_DELAY(1);//_nop_();
		data <<= 1;
    } 
    PIN_ISPCLK_CLR();
	PIN_DELAY(1);
    if(count & 0x01)            //
        PIN_ISPSDA_SET();
    else
        PIN_ISPSDA_CLR();
//	PIN_DELAY();
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
        PIN_ISPCLK_CLR();
        PIN_DELAY(1);
        data <<= 1;
        PIN_ISPCLK_SET();
        if(PIN_ISPSDA_IN())
        {
            data |= 0x01;
            count++;
        }              
	}  
    PIN_ISPCLK_CLR();
	PIN_DELAY(1);
	
	PIN_ISPCLK_SET();
//	PIN_DELAY(1);

	for(i=0;i<3;i++)
	{
         if(PIN_ISPSDA_IN() == (count&0x01))
            break;
	}
	PIN_ISPCLK_CLR();
    PIN_DELAY(1);
	PIN_ISPSDA_CLR();
	PIN_ISPSDA_OUT_ENABLE();      
    
    *read_data = data;
    if(i >= 3)
        return FALSE;
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
        icd_recv(HALT_CHECK_CMD, &halt_data);
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
    uint8_t i;
    uint8_t retry;        
   
    retry = 30;     
	for(i=0; i<retry; i++)
	{
        if(icd_control(ICD_STEP_CMD) != FALSE)	
            break; 
	}
    if(i >= retry)	
		return FALSE;
    return TRUE	;  
}

/*******************************************************************************
*   函 数 名: icd_halt_check
*   功能说明: 步出	2.4.2
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t icd_step_out(void)
{     
    uint8_t i;
    uint8_t retry;        
   
    retry = 30;     
	for(i=0; i<retry; i++)
	{
        if(icd_control(STEP_OUT_CMD) != FALSE)	
            break; 
	}
    if(i >= retry)	
		return FALSE;
    return TRUE	;  
}

/*******************************************************************************
*   函 数 名: icd_step_over
*   功能说明: 步越	2.4.3	  
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t icd_step_over(void)
{  
    uint8_t i;
    uint8_t retry;        
   
    retry = 30;     
	for(i=0; i<retry; i++)
	{
        if(icd_control(STEP_OVER_CMD) != FALSE)	
            break; 
	}
    if(i >= retry)	
		return FALSE;
    return TRUE	;    
 }
/*******************************************************************************
*   函 数 名: icd_run
*   功能说明: 运行	2.4.4	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t icd_run(void)
{
    uint8_t i;
    uint8_t retry;        
   
    retry = 30;
    for(i=0; i<retry; i++)
    {
        if(icd_control(RUN_FLAG_CMD) != FALSE) 
            break;    
    }
    if(i >= retry)
        return FALSE; 
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
    uint8_t i;
    uint8_t retry;
    
//    if(target_state != ICD_RUN)
//        return FALSE;
    
    retry = 30;
    for(i=0; i<retry; i++)
    {
        if(icd_control(ICD_HALT_CMD) != FALSE) 
            break;    
    }
    if(i >= retry)
        return FALSE;      
    
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
    uint8_t i;
    uint8_t retry;    
    
    retry = 30;
    for(i=0; i<retry; i++)
    {
        if(icd_control(ICD_RESET_CMD) != FALSE) 
            break;    
    }
    if(i >= retry)
        return FALSE;        
    
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
            else
            {
                //读失败，重新写入地址
                if(icd_send(SET_ADDR,address) != TRUE)
                    return FALSE;              
            }           
        }
        if(i >= retry)
            return FALSE; 
        write_buf++;
        address++;
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
            if(icd_recv(RD_DATA, read_buf) != FALSE)
                break;
            else
            {
                //读失败，重新写入地址
                if(icd_send(SET_ADDR,address) != TRUE)
                    return FALSE;              
            }                   
        }
        //强制认为读到的数据都正确
//        if(i >= retry)
//            return FALSE; 
        read_buf++;
        address++;
    }
    return TRUE;    
}




//#define PIN_DELAY(n)    ES_DELAY_SLOW(n)
//void  nop_delay()//400ns
//{
//	__nop();
///*	_nop_();
//	_nop_();
//	_nop_();
//	_nop_();
//	_nop_();
//	_nop_();
//	_nop_();   //800ns*/
//}

//static void icd_start(void)
//{
//    PIN_ISPSDA_SET();       
//    PIN_DELAY(1);                       
//    PIN_ISPCLK_SET(); 
//    PIN_DELAY(1);                       
//    PIN_ISPSDA_CLR();    
//    PIN_DELAY(1);                       
//    PIN_ISPCLK_CLR();    
//    PIN_DELAY(1);       
//}

//static void icd_end(void)
//{
//    PIN_ISPCLK_CLR();                   
//    PIN_DELAY(1);                       
//    PIN_ISPSDA_CLR();                   
//    PIN_DELAY(1);                       
//    PIN_ISPCLK_SET();                   
//    PIN_DELAY(1);                       
//    PIN_ISPSDA_SET();      
//}
///********************************ICD 通信协议 时序操作**************************/			
///***************************************/ 
// //写数据、写奇偶校验、读ack
//uint8_t  _write_data_parity_read_ack(uint16_t data)
//{
//	uint8_t  ii1,ii2;
//    uint8_t count;          //奇偶校验计算
//  
//	for(ii1 = 0; ii1 < 16; ii1++)	//196?16?,???????15??need modify
//	{
////		ttemp3 <<= 1;   // 16?,need modify
////		temp_data = data & 0x8000;
//		PIN_ISPCLK_CLR();
//		nop_delay();
//		if(data & 0x8000)
//		{
//			PIN_ISPSDA_SET();
//			count++;
//		}
//		else 
//		{
//			PIN_ISPSDA_CLR();
//		}		
//		nop_delay();
//		PIN_ISPCLK_SET();
//		data <<= 1;  // 16?,need modify 
//	} 
//	
//	PIN_ISPCLK_CLR();
//	nop_delay();
//    if(count & 0x01)            //
//        PIN_ISPSDA_SET();
//    else
//        PIN_ISPSDA_CLR();
////	nop_delay();
//	PIN_ISPCLK_SET();
//	
//    
//	nop_delay();
//	PIN_ISPCLK_CLR();
//	nop_delay();
//	PIN_ISPSDA_OUT_DISABLE();
//	nop_delay();
//	PIN_ISPCLK_SET();
//	nop_delay();
////	temp_flag = SDAIN;	// read ack bit

//	for(ii1=0;ii1<3;ii1++)
//	{
//		 if(PIN_ISPSDA_IN() == ICD_PL_READ_ACK)// read ack bit
//            break;	

//	}
//	PIN_ISPCLK_CLR();
////	SDAOUT=0;
//	PIN_ISPSDA_OUT_ENABLE();
//    PIN_ISPSDA_CLR();
//	if(ii2>=3)
//        return FALSE;
//    return TRUE;    
//}
////icd 读数据、判断奇偶校验
//// read_data 读到的数据指针
////返货 TRUE/FALSE
//uint8_t _read_data_parity(uint16_t *read_data)           		 	//read 8 bits data
// {
//	uint8_t jj1;
//	uint16_t data = 0;
//	uint8_t count = 0;      //计算奇偶校验

//    PIN_ISPSDA_OUT_DISABLE();
//	for(jj1 = 0; jj1 < 16; jj1++)//196? 16?,need modify
//	{ 
//		data <<= 1;
//		PIN_ISPCLK_CLR();
//		nop_delay();
//		PIN_ISPCLK_SET();
//		nop_delay();
//		if(PIN_ISPSDA_IN()) 
//		{ 
//			data |= 0x01;
//            count++;
//		}
//	}

//	PIN_ISPCLK_CLR();
//	nop_delay();
//	
//	PIN_ISPCLK_SET();
//	nop_delay();

//	for(jj1=0;jj1<3;jj1++)
//	{
//         if(PIN_ISPSDA_IN() == (count&&0x01))
//            break;
//	}
//	PIN_ISPCLK_CLR();
//    nop_delay();
//	PIN_ISPSDA_CLR();
//	PIN_ISPSDA_OUT_ENABLE();
//    
//    if(jj1 >= 3)
//        return FALSE;
//    *read_data = data;
//    return TRUE; 
//}
//// icd写指令码、读ack
//// return TRUE/FALSE
//uint8_t _write_cmd_read_ack(uint8_t cmd_data)			// write 8 bit
//{
//   uint8_t  i;
//    
//   for(i = 0; i < 8; i++)
//   {		
//    	PIN_ISPCLK_CLR();
//	
//		if(cmd_data & 0x80)
//		{
//			PIN_ISPSDA_SET();
//		}
//		else 
//		{
//			PIN_ISPSDA_CLR();
//		}
//		nop_delay();//_nop_();

//   		PIN_ISPCLK_SET();
//        nop_delay();//_nop_();
//		cmd_data <<= 1;
//    } 

//    PIN_ISPSDA_OUT_DISABLE();
////	nop_delay();//_nop_();
//	PIN_ISPCLK_CLR();
//    
////	SDAOUT_EN = 1;
//	nop_delay();//_nop_();
//	PIN_ISPCLK_SET();
//	nop_delay();//_nop_();

//	for(i=0; i<3; i++)
//	{
//        if(PIN_ISPSDA_IN() == ICD_PL_READ_ACK)// read ack bit
//            break;
//	}   
//	PIN_ISPCLK_CLR();
//    PIN_ISPSDA_OUT_ENABLE();
//	PIN_ISPSDA_CLR();
//    if(i >= 3)
//        return FALSE;
//    return TRUE;     
//}


///***************************************ICD 命令*******************************/
//uint8_t icd_control(uint8_t cmd_data)
//{
//    uint8_t i;
//    
//    for(i=0; i<30; i++)
//    {
//        icd_start();
//        if(_write_cmd_read_ack(cmd_data) == TRUE)
//            break;        
//    }
//    icd_end();    
//    if(i >= 30)      
//        return  CHIP_CONTROL_ERROR;
//    return ERROR_SUCCESS;       
//}

////ICD命令：写命令
//// cmd_data : 指令码  write_data:要写入的数据
//// 返回：ERROR_SUCCESS 写入成功 CHIP_CONTROL_ERROR：写入失败

//uint8_t icd_write(uint8_t cmd_data, uint16_t write_data)	//icd_write
//{  
//	uint8_t i;
//    
//    for(i=0; i<30; i++)
//    {
//        icd_start();
//        if(_write_cmd_read_ack(cmd_data) ==  TRUE)
//            break;        
//    }
//    if(i >= 30)
//    {
//        icd_end();
//        return  CHIP_CONTROL_ERROR;
//    }
//    
//    for(i=0; i<30; i++)
//    {
//        if(_write_data_parity_read_ack(write_data) == TRUE)       
//            break;
//    }
//    if(i >= 30)
//    {
//        icd_end();
//        return  CHIP_CONTROL_ERROR;
//    }
//    icd_end();
//    return ERROR_SUCCESS;
//}


///////////////////////////////////////////////////////////////?????????,???????
////cmd_data：指令码     read_data：读到的数据
//// 返回：ERROR_SUCCESS 写入成功 CHIP_CONTROL_ERROR：写入失败
//uint8_t icd_read(uint8_t cmd_data, uint16_t *read_data)
//{
//    uint8_t i;     
//    
//    for(i=0; i<30; i++)
//    {
//        icd_start();
//        if(_write_cmd_read_ack(cmd_data) == TRUE)
//            break;
//    }
//    if(i >= 30)
//    {
//        icd_end();
//        return  CHIP_CONTROL_ERROR;
//    }
//        
//    for(i=0; i<30; i++)
//    {
//        if(_read_data_parity(read_data) == TRUE)
//            break;        
//    }    
//    if(i >= 30)
//    {
//        icd_end();
//        return  CHIP_CONTROL_ERROR;
//    }
//    icd_end();

//	return ERROR_SUCCESS;
// }
///***************************************ICD 读写数据*******************************/
////批量数据写操作 
////address:地址 size：16位数据长度 write_data:写入数据buf的地址
//uint8_t write_block_operation(uint16_t address,uint32_t size, uint16_t *write_buf)
//{
//    uint8_t error;
//    
//    error = icd_write(SET_ADDR,address);            //set_addr
//	if(error != ERROR_SUCCESS)
//       return error;
//	
//    while(size--)
//    {
//        error = icd_write(WR_DATA, *write_buf);	            //wr data
//        if(error != ERROR_SUCCESS)
//           return error;  
//        write_buf++;
//    }
//    return ERROR_SUCCESS;
//}
////数据读操作.size：16位数据长度
//uint8_t read_block_operation(uint16_t address,uint32_t size, uint16_t *read_buf)
//{
//    uint8_t error;
//    
//    error = icd_write(SET_ADDR,address);            //set_addr
//	if(error != ERROR_SUCCESS)
//       return error;
//    
//    while(size--)
//    {
//        error = icd_read(RD_DATA, read_buf);
//        if(error != ERROR_SUCCESS)
//           return error;  
//        read_buf++;
//    }
//    return ERROR_SUCCESS;    
//}

////写一个数据操作
////address:地址  write_data:写入数据的地址
//uint8_t icd_write_data_operation(uint16_t address, uint16_t write_data)
//{
//    uint8_t error;
//    
//    error = icd_write(SET_ADDR,address);            //set_addr
//	if(error != ERROR_SUCCESS)
//       return error;
//    error = icd_write(WR_DATA, write_data);	            //wr data
//    if(error != ERROR_SUCCESS)
//       return error;  
//  
//    return ERROR_SUCCESS;
//}
//uint8_t icd_read_data_operation(uint16_t address, uint16_t *read_data)
//{
//    uint8_t error;
//    
//    error = icd_write(SET_ADDR,address);            //set_addr
//	if(error != ERROR_SUCCESS)
//       return error;
//    
//    error = icd_read(RD_DATA, read_data);
//    if(error != ERROR_SUCCESS)
//       return error;  
//    
//    return ERROR_SUCCESS;    
//}

///*******************************************************************************
//*   函 数 名: icd_halt_check
//*   功能说明: halt查询
//*   形    参: 无
//*   返 回 值: 无
//*******************************************************************************/
//#define MAX_ICD_RETRY   50
//uint8_t icd_halt_check(void)
//{	
//    uint16_t halt_data;
//    uint8_t i;
//    
//	for(i=0; i<MAX_ICD_RETRY; i++)
//	{
//        if(icd_read(HALT_CHECK_CMD, &halt_data) != TRUE) 
//            return FALSE;  
//        if(halt_data == 0x5aa5) 
//            break;
//        es_delay_ms(1);
//	}
//    if(i >= MAX_ICD_RETRY)	
//		return FALSE;
//    return TRUE	;
//}
///*******************************************************************************
//*   函 数 名: icd_halt_check
//*   功能说明: 单步   2.4.1
//*   形    参: 无
//*   返 回 值: 无
//*******************************************************************************/
//uint8_t icd_step(void)				//
//{	
//    uint8_t i;
//    uint8_t retry;        
//   
//    retry = 30;     
//	for(i=0; i<retry; i++)
//	{
//        if(icd_control(ICD_STEP_CMD) != TRUE)	
//        return FALSE;
//	}
//    if(i >= retry)	
//		return FALSE;
//    return TRUE	;  
//}

///*******************************************************************************
//*   函 数 名: icd_halt_check
//*   功能说明: 步出	2.4.2
//*   形    参: 无
//*   返 回 值: 无
//*******************************************************************************/
//uint8_t icd_step_out(void)
//{     
//    uint8_t i;
//    uint8_t retry;        
//   
//    retry = 30;     
//	for(i=0; i<retry; i++)
//	{
//        if(icd_control(STEP_OUT_CMD) != TRUE)	
//        return FALSE;
//	}
//    if(i >= retry)	
//		return FALSE;
//    return TRUE	;  
//}

///*******************************************************************************
//*   函 数 名: icd_step_over
//*   功能说明: 步越	2.4.3	  
//*   形    参: 无
//*   返 回 值: 无
//*******************************************************************************/
//uint8_t icd_step_over(void)
//{  
//    uint8_t i;
//    uint8_t retry;        
//   
//    retry = 30;     
//	for(i=0; i<retry; i++)
//	{
//        if(icd_control(STEP_OVER_CMD) != TRUE)	
//        return FALSE;
//	}
//    if(i >= retry)	
//		return FALSE;
//    return TRUE	;    
// }
///*******************************************************************************
//*   函 数 名: icd_run
//*   功能说明: 运行	2.4.4	
//*   形    参: 无
//*   返 回 值: 无
//*******************************************************************************/
//uint8_t icd_run(void)
//{
//    uint8_t i;
//    uint8_t retry;        
//   
//    retry = 30;
//    for(i=0; i<retry; i++)
//    {
//        if(icd_control(RUN_FLAG_CMD) != FALSE) 
//            break;    
//    }
//    if(i >= retry)
//        return FALSE; 
//    return TRUE;    
//}

///*******************************************************************************
//*   函 数 名: icd_halt
//*   功能说明: 停止	2.4.5		
//*   形    参: 无
//*   返 回 值: 无
//*******************************************************************************/
//uint8_t icd_halt(void)				
//{    
//    uint8_t i;
//    uint8_t retry;
//    
////    if(target_state != ICD_RUN)
////        return FALSE;
//    
//    retry = 30;
//    for(i=0; i<retry; i++)
//    {
//        if(icd_control(ICD_HALT_CMD) != FALSE) 
//            break;    
//    }
//    if(i >= retry)
//        return FALSE;      
//    
//    return TRUE;	 
//}

///*******************************************************************************
//*   函 数 名: icd_chip_reset
//*   功能说明: 芯片复位	2.4.7		
//*   形    参: 无
//*   返 回 值: 无
//*******************************************************************************/
//uint8_t icd_chip_reset(void)
//{  
//    uint8_t i;
//    uint8_t retry;    
//    
//    retry = 30;
//    for(i=0; i<retry; i++)
//    {
//        if(icd_control(ICD_RESET_CMD) != FALSE) 
//            break;    
//    }
//    if(i >= retry)
//        return FALSE;        
//    
//    return TRUE;	
//}

///***************************************ICD 读写数据*******************************/
////批量数据写操作 
////address:地址 size：16位数据长度 write_data:写入数据buf的地址
////uint8_t write_block_operation(uint16_t address,uint32_t size, uint16_t *write_buf)
//uint8_t icd_write_str(uint16_t address,uint16_t *write_buf , uint32_t size )
//{
//write_block_operation( address, size, write_buf); 
////    uint8_t i;
////    uint8_t retry;
////    
////    retry = 30;
////    for(i=0; i<retry; i++)
////    {
////        if(icd_send(SET_ADDR,address) != FALSE)
////            break;
////    }
////    if(i >= retry)
////        return FALSE;
////	
////    while(size--)
////    {
////        for(i=0; i<retry; i++)
////        {
////            if(icd_send(WR_DATA, *write_buf) != FALSE)
////                break;
////        }
////        if(i >= retry)
////            return FALSE; 
////        write_buf++;
////    }
////    return TRUE;

//}
////数据读操作.size：16位数据长度
////uint8_t read_block_operation(uint16_t address,uint32_t size, uint16_t *read_buf)
//uint8_t icd_read_str(uint16_t address, uint16_t *read_buf, uint32_t size)
//{
//    read_block_operation( address,  size,  read_buf);
////    uint8_t i;
////    uint8_t retry;
////    
////    retry = 30;
////    for(i=0; i<retry; i++)
////    {
////        if(icd_send(SET_ADDR,address) != FALSE)
////            break;
////    }
////    if(i >= retry)
////        return FALSE;
////    
////    while(size--)
////    {
////        for(i=0; i<retry; i++)
////        {
////            if(icd_recv(RD_DATA, read_buf) != FALSE)
////                break;
////        }
////        if(i >= retry)
////            return FALSE; 
////        read_buf++;
////    }
////    return TRUE;    
//}

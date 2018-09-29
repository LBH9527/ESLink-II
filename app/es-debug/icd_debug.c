#include "ES_ICD.h"
#include "ES_config.h"
#include "icd_debug.h"

//uint16_t breakpoint_addr_buffer[BreakPointNum] = {0};       //端点地址数组
//uint16_t NoStopLab[256]; //110禁止停留区，用于c调试时停到库函数中找不到对应的源程序行问题
//uint8_t  Lab_Num;	//库函数个数。
//uint16_t Main_addr; //c程序中main的入口地址20111206
//uint8_t bReceiveStop = 0; 		//   receive stop command during c step or c stepover

icd_breakpoint_t icd_breakpoint;

static uint8_t icd_halt_check(void);

#define PIN_DELAY(n)    ES_DELAY_SLOW(n)
void  nop_delay()//400ns
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

static void icd_end(void)
{
    PIN_ISPCLK_CLR();                   
    PIN_DELAY(1);                       
    PIN_ISPSDA_CLR();                   
    PIN_DELAY(1);                       
    PIN_ISPCLK_SET();                   
    PIN_DELAY(1);                       
    PIN_ISPSDA_SET();      
}
/********************************ICD 通信协议 时序操作**************************/			
/***************************************/ 
 //写数据、写奇偶校验、读ack
uint8_t  _write_data_parity_read_ack(uint16_t data)
{
	uint8_t  ii1,ii2;
    uint8_t count;          //奇偶校验计算
  
	for(ii1 = 0; ii1 < 16; ii1++)	//196?16?,???????15??need modify
	{
//		ttemp3 <<= 1;   // 16?,need modify
//		temp_data = data & 0x8000;
		PIN_ISPCLK_CLR();
		nop_delay();
		if(data & 0x8000)
		{
			PIN_ISPSDA_SET();
			count++;
		}
		else 
		{
			PIN_ISPSDA_CLR();
		}		
		nop_delay();
		PIN_ISPCLK_SET();
		data <<= 1;  // 16?,need modify 
	} 
	
	PIN_ISPCLK_CLR();
	nop_delay();
    if(count & 0x01)            //
        PIN_ISPSDA_SET();
    else
        PIN_ISPSDA_CLR();
//	nop_delay();
	PIN_ISPCLK_SET();
	
    
	nop_delay();
	PIN_ISPCLK_CLR();
	nop_delay();
	PIN_ISPSDA_OUT_DISABLE();
	nop_delay();
	PIN_ISPCLK_SET();
	nop_delay();
//	temp_flag = SDAIN;	// read ack bit

	for(ii1=0;ii1<3;ii1++)
	{
		 if(PIN_ISPSDA_IN() == ICD_PL_READ_ACK)// read ack bit
            break;	

	}
	PIN_ISPCLK_CLR();
//	SDAOUT=0;
	PIN_ISPSDA_OUT_ENABLE();
    PIN_ISPSDA_CLR();
	if(ii2>=3)
        return FALSE;
    return TRUE;    
}
//icd 读数据、判断奇偶校验
// read_data 读到的数据指针
//返货 TRUE/FALSE
uint8_t _read_data_parity(uint16_t *read_data)           		 	//read 8 bits data
 {
	uint8_t jj1;
	uint16_t data = 0;
	uint8_t count = 0;      //计算奇偶校验

    PIN_ISPSDA_OUT_DISABLE();
	for(jj1 = 0; jj1 < 16; jj1++)//196? 16?,need modify
	{ 
		data <<= 1;
		PIN_ISPCLK_CLR();
		nop_delay();
		PIN_ISPCLK_SET();
		nop_delay();
		if(PIN_ISPSDA_IN()) 
		{ 
			data |= 0x01;
            count++;
		}
	}

	PIN_ISPCLK_CLR();
	nop_delay();
	
	PIN_ISPCLK_SET();
	nop_delay();

	for(jj1=0;jj1<3;jj1++)
	{
         if(PIN_ISPSDA_IN() == (count&&0x01))
            break;
	}
	PIN_ISPCLK_CLR();
    nop_delay();
	PIN_ISPSDA_CLR();
	PIN_ISPSDA_OUT_ENABLE();
    
    if(jj1 >= 3)
        return FALSE;
    *read_data = data;
    return TRUE; 
}
// icd写指令码、读ack
// return TRUE/FALSE
uint8_t _write_cmd_read_ack(uint8_t cmd_data)			// write 8 bit
{
   uint8_t  i;
    
   for(i = 0; i < 8; i++)
   {		
    	PIN_ISPCLK_CLR();
	
		if(cmd_data & 0x80)
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
		cmd_data <<= 1;
    } 

    PIN_ISPSDA_OUT_DISABLE();
//	nop_delay();//_nop_();
	PIN_ISPCLK_CLR();
    
//	SDAOUT_EN = 1;
	nop_delay();//_nop_();
	PIN_ISPCLK_SET();
	nop_delay();//_nop_();

	for(i=0; i<3; i++)
	{
        if(PIN_ISPSDA_IN() == ICD_PL_READ_ACK)// read ack bit
            break;
	}   
	PIN_ISPCLK_CLR();
    PIN_ISPSDA_OUT_ENABLE();
	PIN_ISPSDA_CLR();
    if(i >= 3)
        return FALSE;
    return TRUE;     
}


/***************************************ICD 命令*******************************/
uint8_t icd_control(uint8_t cmd_data)
{
    uint8_t i;
    
    for(i=0; i<30; i++)
    {
        icd_start();
        if(_write_cmd_read_ack(cmd_data) == TRUE)
            break;        
    }
    icd_end();    
    if(i >= 30)      
        return  CHIP_CONTROL_ERROR;
    return ICD_ERR_NONE;       
}

//ICD命令：写命令
// cmd_data : 指令码  write_data:要写入的数据
// 返回：ICD_ERR_NONE 写入成功 CHIP_CONTROL_ERROR：写入失败

uint8_t icd_write(uint8_t cmd_data, uint16_t write_data)	//icd_write
{  
	uint8_t i;
    
    for(i=0; i<30; i++)
    {
        icd_start();
        if(_write_cmd_read_ack(cmd_data) ==  TRUE)
            break;        
    }
    if(i >= 30)
    {
        icd_end();
        return  CHIP_CONTROL_ERROR;
    }
    
    for(i=0; i<30; i++)
    {
        if(_write_data_parity_read_ack(write_data) == TRUE)       
            break;
    }
    if(i >= 30)
    {
        icd_end();
        return  CHIP_CONTROL_ERROR;
    }
    icd_end();
    return ICD_ERR_NONE;
}


/////////////////////////////////////////////////////////////?????????,???????
//cmd_data：指令码     read_data：读到的数据
// 返回：ICD_ERR_NONE 写入成功 CHIP_CONTROL_ERROR：写入失败
uint8_t icd_read(uint8_t cmd_data, uint16_t *read_data)
{
    uint8_t i;     
    
    for(i=0; i<30; i++)
    {
        icd_start();
        if(_write_cmd_read_ack(cmd_data) == TRUE)
            break;
    }
    if(i >= 30)
    {
        icd_end();
        return  CHIP_CONTROL_ERROR;
    }
        
    for(i=0; i<30; i++)
    {
        if(_read_data_parity(read_data) == TRUE)
            break;        
    }    
    if(i >= 30)
    {
        icd_end();
        return  CHIP_CONTROL_ERROR;
    }
    icd_end();

	return ICD_ERR_NONE;
 }
/***************************************ICD 读写数据*******************************/
//批量数据写操作 
//address:地址 size：16位数据长度 write_data:写入数据buf的地址
uint8_t write_block_operation(uint16_t address,uint32_t size, uint16_t *write_buf)
{
    uint8_t error;
    
    error = icd_write(SET_ADDR,address);            //set_addr
	if(error != ICD_ERR_NONE)
       return error;
	
    while(size--)
    {
        error = icd_write(WR_DATA, *write_buf);	            //wr data
        if(error != ICD_ERR_NONE)
           return error;  
        write_buf++;
    }
    return ICD_ERR_NONE;
}
//数据读操作.size：16位数据长度
uint8_t read_block_operation(uint16_t address,uint32_t size, uint16_t *read_buf)
{
    uint8_t error;
    
    error = icd_write(SET_ADDR,address);            //set_addr
	if(error != ICD_ERR_NONE)
       return error;
    
    while(size--)
    {
        error = icd_read(RD_DATA, read_buf);
        if(error != ICD_ERR_NONE)
           return error;  
        read_buf++;
    }
    return ICD_ERR_NONE;    
}

//写一个数据操作
//address:地址  write_data:写入数据的地址
uint8_t icd_write_data_operation(uint16_t address, uint16_t write_data)
{
    uint8_t error;
    
    error = icd_write(SET_ADDR,address);            //set_addr
	if(error != ICD_ERR_NONE)
       return error;
    error = icd_write(WR_DATA, write_data);	            //wr data
    if(error != ICD_ERR_NONE)
       return error;  
  
    return ICD_ERR_NONE;
}
uint8_t icd_read_data_operation(uint16_t address, uint16_t *read_data)
{
    uint8_t error;
    
    error = icd_write(SET_ADDR,address);            //set_addr
	if(error != ICD_ERR_NONE)
       return error;
    
    error = icd_read(RD_DATA, read_data);
    if(error != ICD_ERR_NONE)
       return error;  
    
    return ICD_ERR_NONE;    
}
/*******************************************************************************/

/********************************************************************/
/* 运行控制---->HALT查询	2.4.6				        	*/
/********************************************************************/
#define MAX_ICD_RETRY   50
uint8_t icd_halt_check(void)
{	
    uint16_t halt_data;
    uint8_t ii;
    
	for(ii=0; ii<MAX_ICD_RETRY; ii++)
	{
        if(icd_read(HALT_CHECK_CMD, &halt_data) != ICD_ERR_NONE) 
            return CHIP_CONTROL_ERROR;  
        if(halt_data == 0x5aa5) 
            break;
        es_delay_ms(1);
	}
    if(ii >= MAX_ICD_RETRY)	
		return CHIP_CONTROL_ERROR;
    return ICD_ERR_NONE	;
}
/********************************************************************/
/* 运行控制---->单步   2.4.1  				        	*/
/********************************************************************/
static uint8_t icd_step(void)				//
{	
    if(icd_target_state != ICD_HALT)
        return CHIP_CONTROL_ERROR;    
    
	if(icd_control(ICD_STEP_CMD) != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    
    icd_target_state = ICD_RUN;            //目标芯片状态更新
    
    if(icd_halt_check() != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;

    icd_target_state = ICD_HALT;            //目标芯片状态更新
    return ICD_ERR_NONE;	
}
/********************************************************************/
/* 运行控制---->步出	2.4.2				        	*/
/********************************************************************/
static uint8_t icd_step_out(void)
{
    if(icd_target_state != ICD_HALT)
        return CHIP_CONTROL_ERROR;
    
	if(icd_control(STEP_OUT_CMD) != ICD_ERR_NONE)	
        return CHIP_CONTROL_ERROR;
    
    icd_target_state = ICD_RUN;            //目标芯片状态更新
    
    if(icd_halt_check() != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    
    icd_target_state = ICD_HALT;            //目标芯片状态更新
    return ICD_ERR_NONE;	
}
/********************************************************************/
/* 运行控制---->步越	2.4.3	                		        	*/
/********************************************************************/
static uint8_t icd_step_over(void)
{  
    if(icd_target_state != ICD_HALT)
        return CHIP_CONTROL_ERROR;
    
	if(icd_control(STEP_OVER_CMD) != ICD_ERR_NONE)	
        return CHIP_CONTROL_ERROR;
    
    icd_target_state = ICD_RUN;            //目标芯片状态更新
    
    if(icd_halt_check() != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    
    icd_target_state = ICD_HALT;            //目标芯片状态更新
    return ICD_ERR_NONE;	
 }
/********************************************************************/
/* 运行控制---->运行	2.4.4	                		        	*/
/********************************************************************/
static uint8_t icd_run(void)
{
    if(icd_target_state != ICD_HALT)
        return CHIP_CONTROL_ERROR;
    
	if(icd_control(ICD_RUN_CMD) != ICD_ERR_NONE) 
        return CHIP_CONTROL_ERROR;
    
    icd_target_state = ICD_RUN;            //目标芯片状态更新
    return ICD_ERR_NONE;	    
    
}
/********************************************************************/
/* 运行控制---->停止	2.4.5	                		        	*/
/********************************************************************/
uint8_t icd_halt(void)				
{     
    if(icd_target_state != ICD_RUN)
        return CHIP_CONTROL_ERROR;
    
    if(icd_control(ICD_HALT_CMD) != ICD_ERR_NONE) 
        return CHIP_CONTROL_ERROR;  
    
    if(icd_halt_check() != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    
    icd_target_state = ICD_HALT;            //目标芯片状态更新
    return ICD_ERR_NONE;	 
}



/********************************************************************/
/* 运行控制---->芯片复位	2.4.7				        	*/ 									        	
/********************************************************************/
static uint8_t icd_chip_reset(void)
{
    if(icd_target_state != ICD_RUN)
        return CHIP_CONTROL_ERROR; 
    
    if(icd_control(ICD_RESET_CMD) != ICD_ERR_NONE) 
        return CHIP_CONTROL_ERROR;
    
    icd_target_state = ICD_HALT;            //目标芯片状态更新
    return ICD_ERR_NONE;	
}


//Reset emulator condition is : mrst output a negative pulse
uint8_t emu_reset(void)				//
{
	uint8_t   i;
	uint16_t  halt_data;

    
//	icd_control(ICD_RESET_CMD);
//	AddVpp(RunVpp);//mrst=0;		//need modify
//	delay_1ms_A(2);//	ndelay(3000);//2.1ms
//	AddVpp0v();//mrst=1;
//	delay_1ms_A(2);//ndelay(3000);//2.1ms
//	AddVpp(RunVpp);//mrst=0;
    
    if(icd_chip_reset() !=  ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
	es_delay_ms(2);//ndelay(3000);//2.1ms

    
//	for(i=0; i<50; i++)
//	{
//        if(icd_read(HALT_CHECK_CMD, &halt_data) != ICD_ERR_NONE) 
//            return CHIP_CONTROL_ERROR;  
//        if(halt_data == 0x5aa5) 
//            break;
//        es_delay_ms(1);
//	}
//    if(i >= 50)	
//		return CHIP_CONTROL_ERROR;
    
    if(icd_halt_check() != ICD_ERR_NONE)                        //停止查询
        return CHIP_CONTROL_ERROR;
    
    if(icd_write_data_operation(STPWHL,0)!= ICD_ERR_NONE)       //stopwatch 清零
        return CHIP_CONTROL_ERROR;


	for(i = 0; i < ICD_BREAKPOINT_NUM; i++)			            //断点清零
	{     
		if(icd_breakpoint.addr_buffer[i] != 0x00)
		{
			icd_breakpoint.empty_flag = FALSE;
            if(icd_write_data_operation(SPCBK,icd_breakpoint.addr_buffer[i])!= ICD_ERR_NONE)  
                return CHIP_CONTROL_ERROR;
		}		
    }

    return ICD_ERR_NONE	;	
}




//调试器处理run命令
uint8_t emu_run(void)				//
{
	if( icd_write_data_operation(STPWHL,0) != ICD_ERR_NONE) //stopwatch 清零
        return CHIP_CONTROL_ERROR;    

    if(icd_run() !=  ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    
    return ICD_ERR_NONE	;	
}
//调试器处理stop命令
uint8_t emu_stop(void)
{
    if(icd_halt() !=  ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    
    return ICD_ERR_NONE	;	    
}
//仿真器 汇编单步
uint8_t emu_asm_step(void)
{
    if(icd_write_data_operation(STPWHL,0)!= ICD_ERR_NONE)       //stopwatch 清零
        return CHIP_CONTROL_ERROR;
    
    if(icd_step()!= ICD_ERR_NONE)      
        return CHIP_CONTROL_ERROR;
    
    return ICD_ERR_NONE;	
}
//仿真器 汇编步出
uint8_t emu_asm_step_out(void)
{
    if(icd_write_data_operation(STPWHL,0)!= ICD_ERR_NONE)       //stopwatch 清零
        return CHIP_CONTROL_ERROR;
    
    if(icd_step_out()!= ICD_ERR_NONE)       
        return CHIP_CONTROL_ERROR;
    
    return ICD_ERR_NONE;	
}
//仿真器 汇编步跃
uint8_t emu_asm_step_over(void)
{
    if(icd_write_data_operation(STPWHL,0)!= ICD_ERR_NONE)       //stopwatch 清零
        return CHIP_CONTROL_ERROR;
    if(icd_step_over()!= ICD_ERR_NONE)       
        return CHIP_CONTROL_ERROR;
    return ICD_ERR_NONE;
    
}
//仿真器 C单步
uint8_t emu_C_step(uint8_t *data_buf)	
{
    uint8_t i;
    uint16_t pc_value,read_data;
    union B16_B08 start_addr, end_addr;
    
	if(icd_write_data_operation(STPWHL,0)!= ICD_ERR_NONE)       //stopwatch 清零
        return CHIP_CONTROL_ERROR;    
    
    if(icd_halt_check() != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    
    for(i=0;i<=2;i++)		//modify 20120223
    {
        if(icd_read_data_operation(PCR, &pc_value ) != ICD_ERR_NONE)    //复位后pc为0，c单步直接到main 20111206
            return CHIP_CONTROL_ERROR;
        if(pc_value!=0)
            break;     
    }
    if(pc_value == 0)
	{
        if(icd_write_data_operation(CDBK,icd_main.addr)!= ICD_ERR_NONE) //set breakpoint
            return CHIP_CONTROL_ERROR;

//        if(icd_control(ICD_RUN_CMD) != ICD_ERR_NONE) 
//            return CHIP_CONTROL_ERROR;
//        icd_target_state = ICD_RUN;
        if(icd_run() != ICD_ERR_NONE)
            return CHIP_CONTROL_ERROR;
		return ICD_ERR_NONE;
	}  
//需要汇编单步走过的起始地址和结束地址
    start_addr.B08[1] = data_buf[2];  //get  start address
	start_addr.B08[0] = data_buf[3];
	end_addr.B08[1] = data_buf[6];     //get  end address
	end_addr.B08[0] = data_buf[7];    
	
    if(icd_write(SET_ADDR,LSTPCR) != ICD_ERR_NONE) 
        return CHIP_CONTROL_ERROR;

	while(icd_debug_flag.stop != SET)
	{
        if(icd_step() != ICD_ERR_NONE) 
            return CHIP_CONTROL_ERROR;

//    	while( (icd_read(HALT_CHECK_CMD)!=0x5aa5 )&&(!bReceiveStop) );
//        if(icd_halt_check() != ICD_ERR_NONE)
//            return CHIP_CONTROL_ERROR;
        
        if(icd_read_data_operation(PSW, &read_data ) != ICD_ERR_NONE)    // set sram initial address		
            return CHIP_CONTROL_ERROR;
		
		if((read_data&0x0060) != 0 )break;

        for(i=0;i<=2;i++)		
        {
            if(icd_read_data_operation(PCR, &pc_value ) != ICD_ERR_NONE)    // set sram initial address		
                return CHIP_CONTROL_ERROR;
            if(pc_value!=0)
                break;     
        }

		if(pc_value == 0)	//从程序最后c单步回到main 20111206
		{
            if(icd_write_data_operation(CDBK,icd_main.addr)!= ICD_ERR_NONE) //set breakpoint
                return CHIP_CONTROL_ERROR;
            if(icd_run() != ICD_ERR_NONE)
                return CHIP_CONTROL_ERROR;
            return ICD_ERR_NONE;
		}
        if(icd_breakpoint.empty_flag != TRUE)
        {
		    for(i = 0x00; i < BreakPointNum; i++)
 			{				
				if( icd_breakpoint.addr_buffer[i] == pc_value)
   		     	{	
                    //c单步 遇到断点地址，则需要上传芯片运行停止信息
      			 	icd_debug_flag.monitor = SET;		
					return ICD_ERR_NONE;
      			}    
 			}            
            
        }

        
		if( (pc_value < start_addr.B16) || (pc_value > end_addr.B16)||( start_addr.B16 == end_addr.B16) )  
		{
            //c单步 步出下发的PC地址区间，则需要上传芯片运行停止信息
            icd_debug_flag.monitor = SET;	
            break;
             
/* 		库函数地址由上位机判断 （add by 9527 2018年6月11日） 	
            byStopFlag = 0;		
           
			for(ii=0;ii<(Lab_Num*2);ii=ii+2)
			{
				if( (pc_value >= NoStopLab[ii]) 
					&& (pc_value <= NoStopLab[ii+1]) )
				{
					byStopFlag = 1;
					break;
				}
			}
            
			if(!byStopFlag)
				StepFlag = 0;
			else
				StepFlag = 1;*/
		}

	}

    return ICD_ERR_NONE;    
}
/********************************************************************/
/* Set emulator stepover	during C compiler   		        	*/
/********************************************************************/
//C步越
uint8_t emu_C_stepover(uint8_t *data_buf)
{    
    uint8_t i;
    union B16_B08 start_addr, end_addr;
    uint16_t pc_value,read_data;
    
    if(icd_write_data_operation(STPWHL, 0x0000) != ICD_ERR_NONE)	//stopwatch 清零
        return CHIP_CONTROL_ERROR;
   
	start_addr.B08[1] = data_buf[2];  //get  start address
	start_addr.B08[0] = data_buf[3];
	end_addr.B08[1] = data_buf[6];     //get  end address
	end_addr.B08[0] = data_buf[7];
    
//	while( (icd_read(HALT_CHECK)!=0x5aa5)&&(bReceiveStop==0) );//????halt????stop??
    if(icd_halt_check() != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
        
	icd_write(SET_ADDR,LSTPCR);

	while(1)
	{
        if(icd_step_over() != ICD_ERR_NONE)    // set sram initial address		
            return CHIP_CONTROL_ERROR;
//		if(icd_control(STEP_OVER)!= ICD_ERR_NONE)
//            return CHIP_CONTROL_ERROR;

////		while( (icd_read(HALT_CHECK)!=0x5aa5)&&(bReceiveStop==0) );//????halt????stop??
//        if(icd_halt_check() != ICD_ERR_NONE)
//            return CHIP_CONTROL_ERROR;
        
		if(icd_read_data_operation(PSW, &read_data ) != ICD_ERR_NONE)    // set sram initial address		
            return CHIP_CONTROL_ERROR;
        if((read_data&0x0060) != 0 )break;
        
//		if(bReceiveStop)  		//若收到停止命令，则退出C步越循环
//		{	outofaddress = 0;
//			break;
//		}
        if(icd_read_data_operation(PCR, &pc_value ) != ICD_ERR_NONE)    
                return CHIP_CONTROL_ERROR;

        if(icd_breakpoint.empty_flag != TRUE)
        {
		    for(i = 0x00; i < BreakPointNum; i++)
 			{				
				if( icd_breakpoint.addr_buffer[i] == pc_value)
   		     	{			
                    icd_debug_flag.monitor = SET;	
					return ICD_ERR_NONE;    
      			}    
 			}            
            
        }
		if( (pc_value < start_addr.B16) || (pc_value > end_addr.B16)||( start_addr.B16 == end_addr.B16) )  
		{	
            //c单步 步出下发的PC地址区间，则需要上传芯片运行停止信息
            icd_debug_flag.monitor = SET;	
            break;
            /* 		库函数地址由上位机判断 （add by 9527 2018年6月11日） 
			byStopFlag = 0;				
			for(ii=0;ii<(Lab_Num*2);ii=ii+2)
			{
				if( (currentPC >= NoStopLab[ii]) 
					&& (currentPC <= NoStopLab[ii+1]) )
				{
					byStopFlag = 1;
					break;
				}
			}
			if(!byStopFlag)
				outofaddress = 0;
			else
				outofaddress = 1;*/  
		 }		                   
	}
//	if(!bReceiveStop)
//	{
//		sysSta.sysSta = Sys_Monitor;			
//  	 	RUN_LED = 1;
//	}
    return ICD_ERR_NONE;    
}

/********************************************************************/
/* To modify emulate MCU's PC value to control Mcu opertion        	*/
//仿真器 修改PC值
/********************************************************************/
uint8_t emu_modify_PC(uint8_t *data_buf)				
{    	
	union  B16_B08  pc1, pc2;
	uint8_t i = 0;
	//-------------------------------------------------------// read current PC
	pc1.B08[1] =  data_buf[i++];  //get  new PC
	pc1.B08[0] =  data_buf[i++];
	pc2.B08[1] =  data_buf[i++];  //get  new PC
	pc2.B08[0] =  data_buf[i++];
//------------------------------------------------------------ // write PC

    if(icd_write_data_operation(PCR,pc2.B16)  != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;

//	for(i = 0; i < 12; i++)
//	{
//		ndelay(800); //500us
//	}
    es_delay_ms(6);

}

/********************************************************************/
/*  read current and last pc 									*/
//*******************************************************************/
uint8_t emu_read_PC(uint8_t *buf)
{   
    uint8_t i = 0;
    union B16_B08 pc;
    
    if(icd_read_data_operation(PCR, &pc.B16 ) != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    buf[i++] = 0;
    buf[i++] = 0;
    buf[i++] = pc.B08[1];
    buf[i++] = pc.B08[0];
    
    if(icd_read_data_operation(LSTPCR, &pc.B16 ) != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    buf[i++] = 0;
    buf[i++] = 0;
    buf[i++] = pc.B08[1];
    buf[i++] = pc.B08[0];
     
    return ICD_ERR_NONE;
}
	
/********************************************************************/
/* To set interrupt point to emulate MCU				        	*/
//*******************************************************************/
//设置PC断点
uint8_t emu_set_breakpoint(uint8_t *buf)		//
{   
	uint8_t i;
	union B16_B08 bkpt_addr;

    bkpt_addr.B08[1] = buf[2];	// get address
    bkpt_addr.B08[0] = buf[3];	// get address

    for(i = 0; i < BreakPointNum; i++)
    {				
        if( icd_breakpoint.addr_buffer[i] == bkpt_addr.B16)
            return ICD_ERR_NONE;       //断点地址已经存在buffer中，不需要设置此端点
 	}        
        
	for(i = 0; i < BreakPointNum; i++)	// modify breakpoint buffer
	{     	    	
		if(icd_breakpoint.addr_buffer[i] == 0x00)           
            break;            
    }   
    if(i >= BreakPointNum)
        return CHIP_CONTROL_ERROR;    
    if(icd_write_data_operation(SPCBK, bkpt_addr.B16)  != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    
   icd_breakpoint.addr_buffer[i] = bkpt_addr.B16;          
    return ICD_ERR_NONE;     
}
/********************************************************************/
/* To move emulate MCU operation's setted interrupt adderrss       	*/
/********************************************************************/
// 清除断点
uint8_t emu_clr_breakpoint(uint8_t *buf)		
{	
	uint8_t i;
    union B16_B08 bkpt_addr;
	
    bkpt_addr.B08[1] = buf[2];		// get address
    bkpt_addr.B08[0] = buf[3];		// get address

	if(icd_write_data_operation(CPCBK, bkpt_addr.B16)!= ICD_ERR_NONE)  //清除断点
        return CHIP_CONTROL_ERROR;
	
	for(i = 0; i < BreakPointNum; i++)	// clear breakpointer buffer
	{    
 	 	if(icd_breakpoint.addr_buffer[i] == bkpt_addr.B16)
        {
         	icd_breakpoint.addr_buffer[i] = 0x00;
        	break;
        }
    }
    
	for(i = 0; i < BreakPointNum; i++)		// check breakpointer buffer is empty or no
	{       
		if(icd_breakpoint.addr_buffer[i] != 0) 
			break;	
  	}		
    if(i >= BreakPointNum)  
        icd_breakpoint.empty_flag = TRUE;
    return ICD_ERR_NONE	;	
}
//********************************************************************/
//清除所有断点
uint8_t emu_clr_all_breakpointer(void)  // clr  all breakpointer
{
	uint8_t  i;

	if(icd_write_data_operation(CPCBK, 0x0000) != ICD_ERR_NONE)	// clr  all breakpoint
        return CHIP_CONTROL_ERROR;

	for(i = 0; i < BreakPointNum; i++)
	{	
		if( icd_breakpoint.addr_buffer[i] !=0)
		{
			icd_breakpoint.addr_buffer[i] = 0x00;	
		}
	}
    icd_breakpoint.empty_flag = TRUE;
    return ICD_ERR_NONE	;
}

/********************************************************************/
/* To send back all sram 								        	*/
/********************************************************************/
uint8_t update_chip_status(uint8_t *data_buf, uint8_t *code_buf)				                 
{
    union B16_B08  read_data;
    uint8_t i=0;
    
    //a reg
    if(icd_read_data_operation(0xFF85, &read_data.B16) != ICD_ERR_NONE)    //9527
        return CHIP_CONTROL_ERROR;
    data_buf[0] = read_data.B08[0];
    //b reg
    data_buf[1] = 0;               //7P196 no b reg
    //current pc
    if(icd_read_data_operation(PCR, &read_data.B16) != ICD_ERR_NONE)    //9527
        return CHIP_CONTROL_ERROR;
    code_buf[i++] = 0;
    code_buf[i++] = 0;
    code_buf[i++] = read_data.B08[1];
    code_buf[i++] = read_data.B08[0];
    //last pc
    if(icd_read_data_operation(LSTPCR, &read_data.B16) != ICD_ERR_NONE)    //9527
        return CHIP_CONTROL_ERROR;
    code_buf[i++] = 0;
    code_buf[i++] = 0;
    code_buf[i++] = read_data.B08[1];
    code_buf[i++] = read_data.B08[0];
    //STOPWATCH
    if(icd_read_data_operation(STPWHH, &read_data.B16) != ICD_ERR_NONE)    //9527
        return CHIP_CONTROL_ERROR;
    code_buf[i++] = read_data.B08[1];
    code_buf[i++] = read_data.B08[0];
    if(icd_read_data_operation(STPWHL, &read_data.B16) != ICD_ERR_NONE)    //9527
        return CHIP_CONTROL_ERROR;
    code_buf[i++] = read_data.B08[1];
    code_buf[i++] = read_data.B08[0];  
    
    return ICD_ERR_NONE	;
}
//-----------------------------------------------------------------------------
//		???SRAM?SFR
//-----------------------------------------------------------------------------
/*void Read_AllRam(void)
{		
	uint8_t read_ch;
	uuint16 idata length,con_add,sram_address_back;
	uint16  sram_address,start_add,ii2;
    union B16_B08 address_tempcom;

	uint8_t Rambackpointer;
	read_ch = serDataTest[DATA1_N];
	start_add = serDataTest[DATA2_N];
	start_add = (start_add<<8)+serDataTest[DATA3_N];
	length = CODE_START_N;
	comsendBuf[DATA1_N] = serDataTest[DATA1_N];
	comsendBuf[DATA2_N] = 0;
	if(start_add==0)
	{
		if(read_ch & 0x02)  //read sfr	
		{
			sram_address = 0x0000;
			icd_write(SET_ADDR, 0xFF80);
			if(Device_break_flag)return;
			con_add=0;
	 	    sram_address_back=0;
	    
		    for(ii2 = 0; ii2 <= 127; ii2++)
		    {  
				Mythe16bit_data.the16Bit = rd_16data(RD_DATA,SET_ADDR,(0xFF80+sram_address)); 	//?16bit??,?8??? ?????1  
				if(Device_break_flag)return;	
				LocalTemp2 = Mythe16bit_data.BB.low8bit;
				Rambackpointer = RamDataBackB[sram_address];
				if(Rambackpointer != LocalTemp2)
				{ 
					RamDataBackB[sram_address]=LocalTemp2;
					if( (sram_address==(sram_address_back+1))&&(con_add<0xff)&&(con_add!=0) )
					{
						con_add++;
						comsendBuf[length++] = LocalTemp2;				
					}
					else
					{	
						if(con_add!=0)
						{
							comsendBuf[length-con_add-3]=con_add;
						}
						length++;
						con_add=1;					
						address_tempcom.the16Bit= sram_address+0xFF80;
						comsendBuf[length++] = address_tempcom.BB.high8bit;
						comsendBuf[length++] = address_tempcom.BB.low8bit;
						comsendBuf[length++] = LocalTemp2;
					}
					sram_address_back= sram_address;
				}
				sram_address++;
			} 
			if(	con_add!=0)	comsendBuf[length-con_add-3]=con_add;
		}
	}
//-------------------------------------------------------------------------------

	if(read_ch & 0x01)  //read sram
	{
		sram_address = start_add;
		icd_write(SET_ADDR,sram_address);  
		if(Device_break_flag)
		{
			return;
		}	

        con_add=0;
 	    sram_address_back=0;

	    for(ii2 = start_add; ii2 <= RAM_MAX; ii2++)
	    {  	
			Mythe16bit_data.the16Bit = rd_16data(RD_DATA,SET_ADDR,sram_address); 	//?16bit??,?8??? ?????1  
			if(Device_break_flag)return;	
			LocalTemp2 = Mythe16bit_data.BB.low8bit;
			Rambackpointer = RamDataBack[sram_address];
			if(Rambackpointer != LocalTemp2)
			{ 
				RamDataBack[sram_address]=LocalTemp2;
				if( (sram_address==(sram_address_back+1))&&(con_add<0xff)&&(con_add!=0) )
				{
					con_add++;
					comsendBuf[length++] = LocalTemp2;				
				}
				else
				{	
					if(con_add!=0)
					{
						comsendBuf[length-con_add-3]=con_add;
					}
					length++;
					con_add=1;					
					address_tempcom.the16Bit= sram_address;
					comsendBuf[length++] = address_tempcom.BB.high8bit;
					comsendBuf[length++] = address_tempcom.BB.low8bit;
					comsendBuf[length++] = LocalTemp2;
				}
				sram_address_back= sram_address;
			}
			sram_address++;
			if(length>=0x3100)
			{
				comsendBuf[DATA2_N] = 1;
				break;
			}
		} 
		if(	con_add!=0)	comsendBuf[length-con_add-3]=con_add;
	}
//-------------------------------------------------------------------------------

	acktemp_length =length+3;		// sram number
}	*/
/************************modify block sram************************************/
//写寄存器数据 0x40
//填充内存功能。
uint8_t emu_modify_ram(uint8_t *data_buf, uint8_t *code_buf )
{	
    uint16_t i;
	union B16_B08 len,start_addr;
    uint8_t data;
    union B16_B08 read_data;
	
    
    len.B08[1] = data_buf[0];  //get  start address
	len.B08[0] = data_buf[1];
	start_addr.B08[1] = data_buf[2];     //get  end address
	start_addr.B08[0] = data_buf[3];
    data = data_buf[4];

	for(i = 0; i < len.B16; i++)
	{	//---------------------------------------------------------
        if(icd_write_data_operation(start_addr.B16, data) != ICD_ERR_NONE)	
            return CHIP_CONTROL_ERROR;
		if(icd_read_data_operation(start_addr.B16, &read_data.B16) != ICD_ERR_NONE)    
            return CHIP_CONTROL_ERROR;
		start_addr.B16++;
      	code_buf[i] = read_data.B08[0];
    }
    data_buf[4] = 0;        //按照协议清零
    return ICD_ERR_NONE	;
}
/************************?????************************************/		
//导入寄存器数据.
//说明：写入寄存器数据，并读出该数据
uint8_t emu_import_ram(uint8_t *data_buf, uint8_t *code_buf)
{	
	union B16_B08  addr;
    uint16_t data ;
	uint16_t i;
	uint16_t len;
    
    len = data_buf[0] << 8 | data_buf[1];
    
	for(i = 0; i < len; i++)
	{
		addr.B08[1] = code_buf[i * 3 ];      //9527 数据大小端确认
        addr.B08[0] = code_buf[i * 3 + 1 ];
        data = code_buf[i * 3 + 2 ];
        
        if(icd_write_data_operation(addr.B16, data)!= ICD_ERR_NONE)
            return CHIP_CONTROL_ERROR;	
        //再从改地址读出数据，回复给上位机判断

        if(icd_read_data_operation(addr.B16, &data) != ICD_ERR_NONE)
            return CHIP_CONTROL_ERROR; 
        code_buf[i * 3 + 2 ] = data;
       
  	}
    return ICD_ERR_NONE	;
}
///************************read block sram************************************/
//读寄存器段
//addr读数据地址 len读数据长度， buf：接收数据缓存区
uint8_t emu_read_ram(uint8_t *data_buf, uint8_t *code_buf)
{
    uint16_t i;
    uint16_t len, addr;
    union B16_B08 data;
    
    len = (data_buf[0] << 8) | data_buf[1];
    addr = (data_buf[2] << 8) | data_buf[3];
    for(i=0; i<len; i++)
    {
        if(icd_read_data_operation(addr, &data.B16) != ICD_ERR_NONE)
            return CHIP_CONTROL_ERROR;        
        addr++;
        code_buf[i] = data.B08[0];
    }

    return ICD_ERR_NONE	;
}


//读已用堆栈内容
uint8_t emu_read_used_stack(uint8_t *data_buf, uint8_t *code_buf)					//display all levels stack //need modify
{
    uint8_t i, statck_number;
    union B16_B08 read_data;

    if(icd_read_data_operation(STKLV, &read_data.B16) != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;
    statck_number = read_data.B08[0];
    if( (statck_number & STACK_NUM) == STACK_NUM)
        statck_number = STACK_NUM;
    
    data_buf[0] = statck_number;
    
	for(i = 0; i <statck_number; i++)	//
	{
        if(icd_read_data_operation(STK0+i, &read_data.B16) != ICD_ERR_NONE)
            return CHIP_CONTROL_ERROR;
        code_buf[i*4] = 0;
        code_buf[i*4 + 1] = 0;    
        code_buf[i*4 + 2] = read_data.B08[1];    
        code_buf[i*4 + 3] = read_data.B08[0];   
	}
    return ICD_ERR_NONE	;
}			
//读全部堆栈内容
uint8_t emu_read_all_stack(uint8_t *data_buf, uint8_t *code_buf)			//display all levels stack //need modify
{
  	union B16_B08 stack_data;
	uint8_t i,n;

    data_buf[0] = STACK_NUM;
    for(i = 0; i <STACK_NUM; i++)	//8?????
    {
        if(icd_read_data_operation(STK0 + i, &stack_data.B16) != ICD_ERR_NONE)
            return CHIP_CONTROL_ERROR;
        code_buf[i*4] = 0;
        code_buf[i*4 + 1] = 0;    
        code_buf[i*4 + 2] = stack_data.B08[1];    
        code_buf[i*4 + 3] = stack_data.B08[0];        
    }   
    return ICD_ERR_NONE	;    
}	
		
//设置条件断点
uint8_t emu_set_condition_breakpoint(uint8_t *buf)
{
	union B16_B08  condition_bkpt;
    uint8_t i = 0;
    
	//-----------------------------------------------------------------------------// condition register	
	condition_bkpt.B08[1] = buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = buf[i++];  // condition register low8bit
    if(icd_write_data_operation(BKS, condition_bkpt.B16)  != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;	
    
	condition_bkpt.B08[1] = buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = buf[i++];  // condition register low8bit
    if(icd_write_data_operation(STBK, condition_bkpt.B16)  != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;	
	
    condition_bkpt.B08[1] = buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = buf[i++];  // condition register low8bit
    if(icd_write_data_operation(ABK, condition_bkpt.B16)  != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;	
  	
    condition_bkpt.B08[1] = buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = buf[i++];  // condition register low8bit
    if(icd_write_data_operation(RBKA, condition_bkpt.B16)  != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;	  

    condition_bkpt.B08[1] = buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = buf[i++];  // condition register low8bit
    if(icd_write_data_operation(RBKD, condition_bkpt.B16)  != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;	  
    
    condition_bkpt.B08[1] = buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = buf[i++];  // condition register low8bit
    if(icd_write_data_operation(PCBKT, condition_bkpt.B16)  != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;	    
    
    condition_bkpt.B08[1] = buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = buf[i++];  // condition register low8bit
    if(icd_write_data_operation(RBKT, condition_bkpt.B16)  != ICD_ERR_NONE)
        return CHIP_CONTROL_ERROR;	
    return ICD_ERR_NONE	;    
}


/********************************************************************
* 下载禁止STOP的库文件地址集合       
* 发送库函数PC地址段
********************************************************************/
uint8_t Ack_DownloadNoStopLab(void)	
{
//	uint8_t *  lab_addr;
//	uint8_t i;

//	Lab_Num = buf[0];	
//	lab_addr = (uint8_t *)(serDataTest) + CODE_START_N + 2;
//	for(i=0; i<(Lab_Num*2); i++)
//	{
//		NoStopLab[i] = ((*(lab_addr+i*4))<<8) + *(lab_addr+i*4+1);	
//	}	
    return ICD_ERR_NONE;    //add by 9527 .新版本库文件地址由上位机判断。只保留正确应答。
}

//---------------------------------------------
//判断是否在库函数中，是的话走单步走出库函数
//----------------------------------------------
//void Lab_judge(void)
//{
//    uint16_t read_data;
//	uint8_t ii;//,ii2;

//	StepFlag = 1;
//	while((StepFlag)&&(!bReceiveStop))
//	{
//        
//		while((icd_read(HALT_CHECK)!=0x5aa5)&&(!bReceiveStop));

//        if(icd_read_data_operation(PSW, &read_data ) != ICD_ERR_NONE)    // set sram initial address		
//            return CHIP_CONTROL_ERROR;
//		
//		if((read_data&0x0060)!=0 )
//            break;

//        if(icd_read_data_operation(PCR, &read_data ) != ICD_ERR_NONE)    // curretn pc 
//            return CHIP_CONTROL_ERROR;


//		byStopFlag = 0;				
//		for(ii=0; ii<(Lab_Num*2); ii=ii+2)
//		{
//			if( (read_data >= NoStopLab[ii]) && (read_data <= NoStopLab[ii+1]) )				
//			{
//				byStopFlag = 1;
//				break;
//			}
//		}
//		if(byStopFlag)
//		{	
//			StepFlag = 1;
//			asmStep();  
//			
//		}
//		else
//		{
//			StepFlag = 0;
//		}
//	}	
//}	

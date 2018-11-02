#include "RTL.h"
#include "ES_ICD.h"
#include "HR7P169B_icd_debug.h"

// Reference to our main task
OS_TID icd_task_id;

//// Event flags for icd process task
//芯片停止检查任务
#define FLAGS_HALT_CHECK        (1 << 0)
      
#define FLAGS_ICD_RUN           (1 << 1)
#define FLAGS_ICD_ASM_STEP_OVER (1 << 2)
#define FLAGS_ICD_C_STEP        (1 << 3)
#define FLAGS_ICD_C_STEP_OVER   (1 << 4)
#define FLAGS_ICD_STEP_OUT      (1 << 5)


icd_breakpoint_t icd_breakpoint;
ICD_COMM_DATA_t icd_data;
ICD_TARGET_STATE icd_target_state;
ICD_DEBUG_FLAG_t icd_debug_flag;
ICD_DEBUG_MAIN_t icd_main;

uint16_t config_data[3];
void icd_error_ack(uint8_t error)
{
    
    
}

/*******************************************************************************
*   函 数 名: ES_reset_target
*   功能说明: 芯片复位			
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t ES_reset_target
{
	uint8_t   i;
	uint16_t  halt_data;
//	icd_control(ICD_RESET_CMD);
//	AddVpp(RunVpp);//mrst=0;		//need modify
//	delay_1ms_A(2);//	ndelay(3000);//2.1ms
//	AddVpp0v();//mrst=1;
//	delay_1ms_A(2);//ndelay(3000);//2.1ms
//	AddVpp(RunVpp);//mrst=0;
    
    if(icd_chip_reset() !=  ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;
	es_delay_ms(2);//ndelay(3000);//2.1ms

    
//	for(i=0; i<50; i++)
//	{
//        if(icd_read(HALT_CHECK_CMD, &halt_data) != ERROR_SUCCESS) 
//            return CHIP_CONTROL_ERROR;  
//        if(halt_data == 0x5aa5) 
//            break;
//        es_delay_ms(1);
//	}
//    if(i >= 50)	
//		return CHIP_CONTROL_ERROR;
    
    if(icd_halt_check() != TRUE)                        //停止查询
        return CHIP_CONTROL_ERROR;
    
    if(icd_write_str(STPWHL_REG_ADDR, 0, 1)!= TRUE)       //stopwatch 清零
        return CHIP_CONTROL_ERROR;


	for(i = 0; i < ICD_BREAKPOINT_NUM; i++)			            //断点清零
	{     
		if(icd_breakpoint.addr_buffer[i] != 0x00)
		{
			icd_breakpoint.empty_flag = FALSE;
            if(icd_write_str(SPCBK_REG_ADDR,&icd_breakpoint.addr_buffer[i], 1)!= ERROR_SUCCESS)  
                return CHIP_CONTROL_ERROR;
		}		
    }

    return ERROR_SUCCESS	;	  
}
/*******************************************************************************
*   函 数 名: ES_run
*   功能说明: 芯片运行		
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t ES_run(void)				//
{
	if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE)  //stopwatch 清零
        return CHIP_CONTROL_ERROR;    

    if(icd_run() !=  ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;
    
    return ERROR_SUCCESS	;	
}
/*******************************************************************************
*   函 数 名: ES_stop
*   功能说明: 芯片停止		
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t ES_stop(void)
{
    if(icd_halt() !=  ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;
    
    return ERROR_SUCCESS	;	    
}
/*******************************************************************************
*   函 数 名: ES_asm_step
*   功能说明: 汇编单步	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t ES_asm_step(void)
{
    if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE)       //stopwatch 清零
        return CHIP_CONTROL_ERROR;
    
    if(icd_step()!= ERROR_SUCCESS)      
        return CHIP_CONTROL_ERROR;
    
    return ERROR_SUCCESS;	
}
/*******************************************************************************
*   函 数 名: ES_asm_step_out
*   功能说明: 汇编步出	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t ES_asm_step_out(void)
{
    if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE)        //stopwatch 清零
        return CHIP_CONTROL_ERROR;
    
    if(icd_step_out()!= ERROR_SUCCESS)       
        return CHIP_CONTROL_ERROR;
    
    return ERROR_SUCCESS;	
}
/*******************************************************************************
*   函 数 名: emu_asm_step_over
*   功能说明: 汇编步跃	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t ES_asm_step_over(void)
{
    if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE)        //stopwatch 清零
        return CHIP_CONTROL_ERROR;
    if(icd_step_over()!= ERROR_SUCCESS)       
        return CHIP_CONTROL_ERROR;
    return ERROR_SUCCESS;
    
}
/*******************************************************************************
*   函 数 名: ES_C_step
*   功能说明: C单步	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
uint8_t ES_C_step(uint8_t *wr_buf)	
{
    uint8_t i;
    uint16_t pc_value,read_data;
    union B16_B08 start_addr, end_addr;
    
	if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE)       //stopwatch 清零
        return CHIP_CONTROL_ERROR;    
    
    if(icd_halt_check() != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;
    
    for(i=0;i<=2;i++)		//modify 20120223
    {
        if(icd_read_data_operation(PCR_REG_ADDR, &pc_value ) != ERROR_SUCCESS)    //复位后pc为0，c单步直接到main 20111206
            return CHIP_CONTROL_ERROR;
        if(pc_value!=0)
            break;     
    }
    if(pc_value == 0)
	{
        if(icd_write_str(CDBK_REG_ADDR,icd_main.addr)!= ERROR_SUCCESS) //set breakpoint
            return CHIP_CONTROL_ERROR;

//        if(icd_control(ICD_RUN_CMD) != ERROR_SUCCESS) 
//            return CHIP_CONTROL_ERROR;
//        icd_target_state = ICD_RUN;
        if(icd_run() != ERROR_SUCCESS)
            return CHIP_CONTROL_ERROR;
//        icd_debug_flag.monitor = SET;		
		return ERROR_SUCCESS;
	}  
//需要汇编单步走过的起始地址和结束地址
    start_addr.B08[1] = wr_buf[ICD_DATA_OFFSET + 2];  //get  start address
	start_addr.B08[0] = wr_buf[ICD_DATA_OFFSET + 3];
	end_addr.B08[1] = wr_buf[ICD_DATA_OFFSET + 6];     //get  end address
	end_addr.B08[0] = wr_buf[ICD_DATA_OFFSET + 7];    
	
    if(icd_write(SET_ADDR,LSTPCR_REG_ADDR) != ERROR_SUCCESS) 
        return CHIP_CONTROL_ERROR;

	while(icd_debug_flag.stop != SET)
	{
        if(icd_step() != ERROR_SUCCESS) 
            return CHIP_CONTROL_ERROR;

//    	while( (icd_read(HALT_CHECK_CMD)!=0x5aa5 )&&(!bReceiveStop) );
//        if(icd_halt_check() != ERROR_SUCCESS)
//            return CHIP_CONTROL_ERROR;
        
        if(icd_read_data_operation(PSW, &read_data ) != ERROR_SUCCESS)    // set sram initial address		
            return CHIP_CONTROL_ERROR;
		
		if((read_data&0x0060) != 0 )break;

        for(i=0;i<=2;i++)		
        {
            if(icd_read_data_operation(PCR_REG_ADDR, &pc_value ) != ERROR_SUCCESS)    // set sram initial address		
                return CHIP_CONTROL_ERROR;
            if(pc_value!=0)
                break;     
        }

		if(pc_value == 0)	//从程序最后c单步回到main 20111206
		{
            if(icd_write_str(CDBK_REG_ADDR,icd_main.addr)!= ERROR_SUCCESS) //set breakpoint
                return CHIP_CONTROL_ERROR;
            if(icd_run() != ERROR_SUCCESS)
                return CHIP_CONTROL_ERROR;
//            icd_debug_flag.monitor = SET;		
            return ERROR_SUCCESS;
		}
        if(icd_breakpoint.empty_flag != TRUE)
        {
		    for(i = 0x00; i < BreakPointNum; i++)
 			{				
				if( icd_breakpoint.addr_buffer[i] == pc_value)
   		     	{	
                    //c单步 遇到断点地址，则需要上传芯片运行停止信息
      			 	icd_debug_flag.monitor = SET;		
					return ERROR_SUCCESS;
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

    return ERROR_SUCCESS;    
}

/*******************************************************************************
*   函 数 名: ES_C_stepover
*   功能说明: C步越	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/  
uint8_t ES_C_stepover(uint8_t *wr_buf)	
{    
    uint8_t i;
    union B16_B08 start_addr, end_addr;
    uint16_t pc_value,read_data;
    
    if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE) 	//stopwatch 清零
        return CHIP_CONTROL_ERROR;
   
	start_addr.B08[1] = wr_buf[ICD_DATA_OFFSET + 2];  //get  start address
	start_addr.B08[0] = wr_buf[ICD_DATA_OFFSET + 3];
	end_addr.B08[1] = wr_buf[ICD_DATA_OFFSET + 6];     //get  end address
	end_addr.B08[0] = wr_buf[ICD_DATA_OFFSET + 7];
    
//	while( (icd_read(HALT_CHECK)!=0x5aa5)&&(bReceiveStop==0) );//????halt????stop??
    if(icd_halt_check() != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;
        
	icd_write(SET_ADDR,LSTPCR_REG_ADDR);

	while(1)
	{
        if(icd_step_over() != ERROR_SUCCESS)    // set sram initial address		
            return CHIP_CONTROL_ERROR;
//		if(icd_control(STEP_OVER)!= ERROR_SUCCESS)
//            return CHIP_CONTROL_ERROR;

////		while( (icd_read(HALT_CHECK)!=0x5aa5)&&(bReceiveStop==0) );//????halt????stop??
//        if(icd_halt_check() != ERROR_SUCCESS)
//            return CHIP_CONTROL_ERROR;
        
		if(icd_read_data_operation(PSW, &read_data ) != ERROR_SUCCESS)    // set sram initial address		
            return CHIP_CONTROL_ERROR;
        if((read_data&0x0060) != 0 )break;
        
//		if(bReceiveStop)  		//若收到停止命令，则退出C步越循环
//		{	outofaddress = 0;
//			break;
//		}
        if(icd_read_data_operation(PCR_REG_ADDR, &pc_value ) != ERROR_SUCCESS)    
                return CHIP_CONTROL_ERROR;

        if(icd_breakpoint.empty_flag != TRUE)
        {
		    for(i = 0x00; i < BreakPointNum; i++)
 			{				
				if( icd_breakpoint.addr_buffer[i] == pc_value)
   		     	{			
                    icd_debug_flag.monitor = SET;	
					return ERROR_SUCCESS;    
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
    return ERROR_SUCCESS;    
}
/*******************************************************************************
*   函 数 名: ES_modify_PC
*   功能说明: 修改PC值	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/ 
uint8_t ES_modify_PC(uint8_t *data_buf)				
{    	
	union  B16_B08  pc1, pc2;
	uint8_t i = 0;
	//-------------------------------------------------------// read current PC
	pc1.B08[1] =  data_buf[i++];  //get  new PC
	pc1.B08[0] =  data_buf[i++];
	pc2.B08[1] =  data_buf[i++];  //get  new PC
	pc2.B08[0] =  data_buf[i++];
//------------------------------------------------------------ // write PC

    if(icd_write_str(PCR_REG_ADDR,pc2.B16)  != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;

//	for(i = 0; i < 12; i++)
//	{
//		ndelay(800); //500us
//	}
    es_delay_ms(6);
    
    return ERROR_SUCCESS;    
}

/*******************************************************************************
*   函 数 名: ES_read_PC
*   功能说明: 读PC值	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/ 
uint8_t ES_read_PC(uint8_t *buf)
{   
    uint8_t i = 0;
    union B16_B08 pc;
    
    if(icd_read_data_operation(PCR_REG_ADDR, &pc.B16 ) != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;
    buf[i++] = 0;
    buf[i++] = 0;
    buf[i++] = pc.B08[1];
    buf[i++] = pc.B08[0];
    
    if(icd_read_data_operation(LSTPCR_REG_ADDR, &pc.B16 ) != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;
    buf[i++] = 0;
    buf[i++] = 0;
    buf[i++] = pc.B08[1];
    buf[i++] = pc.B08[0];
     
    return ERROR_SUCCESS;
}

/*******************************************************************************
*   函 数 名: ES_set_breakpoint
*   功能说明: 设置PC断点	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/ 
uint8_t ES_set_breakpoint(uint8_t *wr_buf)		//
{   
	uint8_t i;
	union B16_B08 bkpt_addr;

    bkpt_addr.B08[1] = wr_buf[ICD_DATA_OFFSET + 2];	// get address
    bkpt_addr.B08[0] = wr_buf[ICD_DATA_OFFSET + 3];	// get address

    for(i = 0; i < BreakPointNum; i++)
    {				
        if( icd_breakpoint.addr_buffer[i] == bkpt_addr.B16)
            return ERROR_SUCCESS;       //断点地址已经存在buffer中，不需要设置此端点
 	}        
        
	for(i = 0; i < BreakPointNum; i++)	// modify breakpoint buffer
	{     	    	
		if(icd_breakpoint.addr_buffer[i] == 0x00)           
            break;            
    }   
    if(i >= BreakPointNum)
        return CHIP_CONTROL_ERROR;    
    if(icd_write_str(SPCBK_REG_ADDR, bkpt_addr.B16)  != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;
    
   icd_breakpoint.addr_buffer[i] = bkpt_addr.B16;          
    return ERROR_SUCCESS;     
}
/*******************************************************************************
*   函 数 名: ES_clr_breakpoint
*   功能说明: 清除断点	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/ 
uint8_t ES_clr_breakpoint(uint8_t *wr_buf)		
{	
	uint8_t i;
    union B16_B08 bkpt_addr;
	
    bkpt_addr.B08[1] = wr_buf[ICD_DATA_OFFSET + 2];		// get address
    bkpt_addr.B08[0] = wr_buf[ICD_DATA_OFFSET + 3];		// get address

	if(icd_write_str(CPCBK_REG_ADDR, bkpt_addr.B16)!= ERROR_SUCCESS)  //清除断点
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
    return ERROR_SUCCESS	;	
}
/*******************************************************************************
*   函 数 名: ES_clr_all_breakpointer
*   功能说明: 清除所有断点	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/ 
uint8_t ES_clr_all_breakpointer(void)  // clr  all breakpointer
{
	uint8_t  i;

	if(icd_write_str(CPCBK_REG_ADDR, 0x0000) != ERROR_SUCCESS)	// clr  all breakpoint
        return CHIP_CONTROL_ERROR;

	for(i = 0; i < BreakPointNum; i++)
	{	
		if( icd_breakpoint.addr_buffer[i] !=0)
		{
			icd_breakpoint.addr_buffer[i] = 0x00;	
		}
	}
    icd_breakpoint.empty_flag = TRUE;
    return ERROR_SUCCESS	;
}

/********************************************************************/
/* To send back all sram 								        	*/
/********************************************************************/
uint8_t update_chip_status(uint8_t *rd_buf, uint16_t *read_len)				                 
{
    union B16_B08  read_data;

    //a reg
//    if(icd_read_data_operation(AREG_REG_ADDR, &read_data.B16) != ERROR_SUCCESS) 
    if(icd_read_data_operation(0xFF85, &read_data.B16) != ERROR_SUCCESS)    //0xFF85 累加器A寄存器
        return CHIP_CONTROL_ERROR;
    rd_buf[ICD_DATA_OFFSET] = read_data.B08[0];
    //b reg
    rd_buf[ICD_DATA_OFFSET + 1] = 0;               //7P196 no b reg
    //current pc
    if(icd_read_data_operation(PCR_REG_ADDR, &read_data.B16) != ERROR_SUCCESS)    //9527
        return CHIP_CONTROL_ERROR;
    rd_buf[ICD_CODE_OFFSET] = 0;
    rd_buf[ICD_CODE_OFFSET + 1] = 0;
    rd_buf[ICD_CODE_OFFSET + 2] = read_data.B08[1];
    rd_buf[ICD_CODE_OFFSET + 3] = read_data.B08[0];
    //last pc
    if(icd_read_data_operation(LSTPCR_REG_ADDR, &read_data.B16, 1) != ERROR_SUCCESS)    //9527
        return CHIP_CONTROL_ERROR;
    rd_buf[ICD_CODE_OFFSET + 4] = 0;
    rd_buf[ICD_CODE_OFFSET + 5] = 0;
    rd_buf[ICD_CODE_OFFSET + 6] = read_data.B08[1];
    rd_buf[ICD_CODE_OFFSET + 7] = read_data.B08[0];
    //STOPWATCH
    if(icd_read_data_operation(STPWHH_REG_ADDR, &read_data.B16, 1) != ERROR_SUCCESS)    //9527
        return CHIP_CONTROL_ERROR;
    rd_buf[ICD_CODE_OFFSET + 8] = read_data.B08[1];
    rd_buf[ICD_CODE_OFFSET + 8] = read_data.B08[0];
    if(icd_read_data_operation(STPWHL_REG_ADDR, &read_data.B16 , 1) != ERROR_SUCCESS)    //9527
        return CHIP_CONTROL_ERROR;
    rd_buf[ICD_CODE_OFFSET + 10] = read_data.B08[1];
    rd_buf[ICD_CODE_OFFSET + 11] = read_data.B08[0];  
    
    *read_len = 12 + ICD_ACK_NORMAL_LENTH;
    return ERROR_SUCCESS	;
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
uint8_t ES_modify_ram(uint8_t *wr_buf, uint8_t *rd_buf, uint16_t *read_len)	
{	
    uint16_t i;
	union B16_B08 len,start_addr;
    uint8_t data;
    union B16_B08 read_data;
	
    uint8_t *data_buf;
    
    data_buf = wr_buf + ICD_DATA_OFFSET;
    
    len.B08[1] = data_buf[0];  //get  start address
	len.B08[0] = data_buf[1];
	start_addr.B08[1] = data_buf[2];     //get  end address
	start_addr.B08[0] = data_buf[3];
    data = data_buf[4];

	for(i = 0; i < len.B16; i++)
	{	//---------------------------------------------------------
        if(icd_write_str(start_addr.B16, data) != ERROR_SUCCESS)	
            return CHIP_CONTROL_ERROR;
		if(icd_read_data_operation(start_addr.B16, &read_data.B16) != ERROR_SUCCESS)    
            return CHIP_CONTROL_ERROR;
		start_addr.B16++;
      	rd_buf[ICD_CODE_OFFSET + i] = read_data.B08[0];
    }
    data_buf[4] = 0;        //按照协议清零
    *read_len = ICD_ACK_NORMAL_LENTH + len.B16;
    return ERROR_SUCCESS	;
}
/************************?????************************************/		
//导入寄存器数据.
//说明：写入寄存器数据，并读出该数据
uint8_t ES_import_ram(uint8_t *wr_buf, uint8_t *rd_buf, uint16_t *read_len)
{	
	union B16_B08  addr;
    uint16_t data ;
	uint16_t i;
	uint16_t len;
    
    len = wr_buf[ICD_DATA_OFFSET] << 8 | wr_buf[ICD_DATA_OFFSET + 1];
    for(i=0; i<ICD_DATA_LEN; i++)
    {
        rd_buf[i+ICD_DATA_OFFSET] = wr_buf[i+ICD_DATA_OFFSET];        
    }
    
	for(i = 0; i < len; i++)
	{
		addr.B08[1] = wr_buf[ICD_CODE_OFFSET + i * 3 ];      //9527 数据大小端确认
        addr.B08[0] = wr_buf[ICD_CODE_OFFSET + i * 3 + 1 ];
        data = wr_buf[ICD_CODE_OFFSET + i * 3 + 2 ];
        
        if(icd_write_str(addr.B16, data)!= ERROR_SUCCESS)
            return CHIP_CONTROL_ERROR;	
        //再从改地址读出数据，回复给上位机判断

        if(icd_read_data_operation(addr.B16, &data) != ERROR_SUCCESS)
            return CHIP_CONTROL_ERROR; 
        rd_buf[ICD_CODE_OFFSET + i * 3 + 2 ] = data;
       
  	}
    *read_len = ICD_ACK_NORMAL_LENTH + len;
    return ERROR_SUCCESS	;
}
///************************read block sram************************************/
//读寄存器段
//addr读数据地址 len读数据长度， buf：接收数据缓存区
uint8_t ES_read_ram(uint8_t *wr_buf, uint8_t *rd_buf, uint16_t *read_len)	
{
    uint16_t i;
    uint16_t len, addr;
    union B16_B08 data;    
    
    len = (wr_buf[ICD_DATA_OFFSET] << 8) | wr_buf[ICD_DATA_OFFSET + 1];
    addr = (wr_buf[ICD_DATA_OFFSET + 2] << 8) | wr_buf[ICD_DATA_OFFSET + 3];
    
    for(i=0; i<ICD_DATA_LEN; i++)
    {
        rd_buf[i+ICD_DATA_OFFSET] = wr_buf[i+ICD_DATA_OFFSET];        
    }
    
    for(i=0; i<len; i++)
    {
        if(icd_read_data_operation(addr, &data.B16) != ERROR_SUCCESS)
            return CHIP_CONTROL_ERROR;        
        addr++;
        rd_buf[ICD_CODE_OFFSET + i] = data.B08[0];
    }
    *read_len = ICD_ACK_NORMAL_LENTH + len;
    
    return ERROR_SUCCESS	;
}


//读已用堆栈内容
uint8_t ES_read_used_stack(uint8_t *rd_buf, uint16_t *read_len)					//display all levels stack //need modify
{
    uint8_t i, statck_number;
    union B16_B08 read_data;

    for(i=0; i<ICD_DATA_LEN; i++)
    {
        rd_buf[i+ICD_DATA_OFFSET] = 0;        
    }
            
    if(icd_read_data_operation(STKLV_REG_ADDR, &read_data.B16) != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;
    statck_number = read_data.B08[0];
    if( (statck_number & STACK_NUM) == STACK_NUM)
        statck_number = STACK_NUM;
    
    rd_buf[ICD_DATA_OFFSET] = statck_number;
    
	for(i = 0; i <statck_number; i++)	//
	{
        if(icd_read_data_operation(STK0_REG_ADDR+i, &read_data.B16) != ERROR_SUCCESS)
            return CHIP_CONTROL_ERROR;
        rd_buf[ICD_CODE_OFFSET + i*4] = 0;
        rd_buf[ICD_CODE_OFFSET + i*4 + 1] = 0;    
        rd_buf[ICD_CODE_OFFSET + i*4 + 2] = read_data.B08[1];    
        rd_buf[ICD_CODE_OFFSET + i*4 + 3] = read_data.B08[0];   
	}
    *read_len  = ICD_ACK_NORMAL_LENTH + statck_number*4;
    return ERROR_SUCCESS	;
}			
//读全部堆栈内容
uint8_t ES_read_all_stack(uint8_t *rd_buf, uint16_t *read_len)			//display all levels stack //need modify
{
  	union B16_B08 stack_data;
	uint8_t i;

    for(i=0; i<ICD_DATA_LEN; i++)
    {
        rd_buf[i+ICD_DATA_OFFSET] = 0;        
    }
    
    rd_buf[ICD_DATA_OFFSET] = STACK_NUM;
    for(i = 0; i <STACK_NUM; i++)	//8?????
    {
        if(icd_read_data_operation(STK0_REG_ADDR + i, &stack_data.B16) != ERROR_SUCCESS)
            return CHIP_CONTROL_ERROR;
        rd_buf[ICD_CODE_OFFSET + i*4] = 0;
        rd_buf[ICD_CODE_OFFSET + i*4 + 1] = 0;    
        rd_buf[ICD_CODE_OFFSET + i*4 + 2] = stack_data.B08[1];    
        rd_buf[ICD_CODE_OFFSET + i*4 + 3] = stack_data.B08[0];        
    }  
    *read_len  = ICD_ACK_NORMAL_LENTH + STACK_NUM*4;    
    return ERROR_SUCCESS	;    
}	
		
//设置条件断点
uint8_t ES_set_condition_breakpoint(uint8_t *wr_buf)
{
	union B16_B08  condition_bkpt;
    uint8_t i = ICD_DATA_OFFSET;
    
	//-----------------------------------------------------------------------------// condition register	
	condition_bkpt.B08[1] = wr_buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = wr_buf[i++];  // condition register low8bit
    if(icd_write_str(BKS_REG_ADDR, condition_bkpt.B16)  != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;	
    
	condition_bkpt.B08[1] = wr_buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = wr_buf[i++];  // condition register low8bit
    if(icd_write_str(STBK_REG_ADDR, condition_bkpt.B16)  != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;	
	
    condition_bkpt.B08[1] = wr_buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = wr_buf[i++];  // condition register low8bit
    if(icd_write_str(ABK_REG_ADDR, condition_bkpt.B16)  != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;	
  	
    condition_bkpt.B08[1] = wr_buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = wr_buf[i++];  // condition register low8bit
    if(icd_write_str(RBKA_REG_ADDR, condition_bkpt.B16)  != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;	  

    condition_bkpt.B08[1] = wr_buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = wr_buf[i++];  // condition register low8bit
    if(icd_write_str(RBKD_REG_ADDR, condition_bkpt.B16)  != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;	  
    
    condition_bkpt.B08[1] = wr_buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = wr_buf[i++];  // condition register low8bit
    if(icd_write_str(PCBKT_REG_ADDR, condition_bkpt.B16)  != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;	    
    
    condition_bkpt.B08[1] = wr_buf[i++];  // condition registerhigh8bit
	condition_bkpt.B08[0] = wr_buf[i++];  // condition register low8bit
    if(icd_write_str(RBKT_REG_ADDR, condition_bkpt.B16)  != ERROR_SUCCESS)
        return CHIP_CONTROL_ERROR;	
    return ERROR_SUCCESS	;    
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
    return ERROR_SUCCESS;    //add by 9527 .新版本库文件地址由上位机判断。只保留正确应答。
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

//        if(icd_read_data_operation(PSW, &read_data ) != ERROR_SUCCESS)    // set sram initial address		
//            return CHIP_CONTROL_ERROR;
//		
//		if((read_data&0x0060)!=0 )
//            break;

//        if(icd_read_data_operation(PCR, &read_data ) != ERROR_SUCCESS)    // curretn pc 
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


//与编程器相关的操作
uint8_t download_config(uint8_t *wr_buf)
{
    
    //buf[0] = AA;
    //buf[1] = 55;  配置字起始
    config_data[0] = wr_buf[2 + ICD_CODE_OFFSET] << 8 | wr_buf[3 + ICD_CODE_OFFSET];      //配置字
    
    config_data[1] = wr_buf[32 + ICD_CODE_OFFSET] << 8 | wr_buf[33 + ICD_CODE_OFFSET];    //userID1
    config_data[2] = wr_buf[34 + ICD_CODE_OFFSET] << 8 | wr_buf[35 + ICD_CODE_OFFSET];    //userID2    
    
    return ERROR_SUCCESS;
}
void HandleDownLoad()
{
//	if(!downPro_Cmd)	// first enter programme down load
//	{   
//		downPro_Cmd = 1;
////------------------------------
//    	VT_Select1 = 1;
//    	VT_Select2 = 1;
////---------------------------------
//		if(eraseflash()!=RUN_OK)
//		{	
////			AddVpp(RunVpp);
//			SubStatus_A = ERASE_ERROR;
//			downPro_Cmd = 0;
//	      	return ;
//	    }	

//		if(proProgrammData()!=RUN_OK)
//		{  
////			AddVpp(RunVpp);
//      		SubStatus_A = PGM_ERROR;
//			downPro_Cmd = 0;
//	  		return ;
//	  	}
//	}
//    else 			// download programme without flash erase 
//    {   		
//		if(proProgrammData()!=RUN_OK) 
//		{  
////			AddVpp(RunVpp);
//        	SubStatus_A = PGM_ERROR;
//			downPro_Cmd = 0;
//			return ;
//		}		
//	}
}
//icd　用户hex下载开始
//Note：通过ISP方式将接收到的数据下载到目标芯片的flash中。
uint8_t icd_download_user_hex_begin(uint8_t *wr_buf)
{
    uint16_t size;
    uint32_t addr;
    static bool first = true;
    uint8_t *data_buf ;
    
    data_buf = wr_buf + ICD_DATA_OFFSET;
    size = data_buf[0]<<8 | data_buf[1];
    addr = data_buf[2]<<24 | data_buf[3] << 16 | data_buf[4]<<8 | data_buf[5];
    
    if(first != true)
    {
        //擦除目标芯片flash
        
        //写接收到的数据到flash中
        
//        spi_flash_write(SF_USER_HEX_ADDR+addr, size, code_buf);
    }
    else
    {
        //
        first = false;
        
    }
    return ERROR_SUCCESS; 
    
}
//icd　用户hex下载完成
//Note: 配置字编程
uint8_t icd_download_user_hex_end()
{      
    //清空断点地址
    memset(icd_breakpoint.addr_buffer, 0 , sizeof(icd_breakpoint.addr_buffer));
    //设置断点标志
    icd_breakpoint.empty_flag = TRUE;   //没有断点
    icd_main.set_point = RESET;         //默认不设置端点在main函数。
    icd_target_state = ICD_HALT;
    PIN_RST_OUT(1);
    
    return ERROR_SUCCESS; 
    
}
void EndDownLoad()
{
//	uint8_t i,retval;

//	if(downPro_Cmd)			//  is download end 
//	{		
//		if(write_config()==0)
//		{       
////			AddVpp(RunVpp);
//	   		SubStatus_A = OPTION_PGM_ERROR;
// 	   	 	downPro_Cmd = 0;

//			retval = Chip_ID_Judge1();
//			if(retval!=RUN_OK)
//			{
//				SubStatus_A = CHIP_ID_NOT_MATCH;
//			}

// 	    	return ;
//	    }
//		retval = Chip_ID_Judge1();
//		if(retval!=RUN_OK)
//		{
//			SubStatus_A = CHIP_ID_NOT_MATCH;
//			downPro_Cmd = 0;
//			return;
//		}
////		AddVpp(RunVpp);
////		delay_1ms_A(100);
//  VppSet(0);
//  VPPselect = 0;           //控制VPP电压降到最低，三极管断开
//  VPP_GND = 0;             //VPP、VDD、GND接地控制全部断开
//  delay_1ms_A(10);
//        start(); 
//        delay_1ms_A(10);

////		AddVpp0v();
////		delay_1ms_A(10);

//		downPro_Cmd = 0;
//		Pointernum = 0;
//		for(i = 0; i < BreakPointNum; i++)
//			Pointerbuffer[i] = 0x0000;
//		breakpointerflag = 0xff;
//		Lab_Num = 0;
//		need_setpoint = 0; 
//		stopwatch_had = 0;
//		stopwatch.the32Bit = 0;
//	}
//	else   
//	{			// not during download ,ack can't surpport;
//		SubStatus_A = FUN_ERROR;
//	}
}
/*
 *  读芯片信息。时序号+时序版本+时序校验和保存K22内部falsh中。
* 修改为保存在外部flash 中
 *  后面的信息保存在外部SPI falsh中
 */
icd_error_t icd_read_chipinfo(uint8_t *buf)
{
//    union B32_B08 data_buf[ICD_CHIP_INFO_LEN/4];      //长度需要被4整除
    icd_error_t status = ERROR_SUCCESS;
    
    spi_flash_read(CHIP_INFO_Fth, ICD_CHIP_INFO_LEN, buf);
//    memcpy(buf, buf[0].B08, ICD_CHIP_INFO_LEN);
    
    return status;  
}
/*
 * 芯片信息下载
* 擦除芯片信息区，并保存新的数据
 */
icd_error_t icd_write_chipinfo(uint8_t *buf)
{
//    union B32_B08 data_buf[ICD_CHIP_INFO_LEN/4];      //长度需要被4整除
    
//    memcpy(data_buf[0].B08, buf, ICD_CHIP_INFO_LEN);
    spi_flash_write(CHIP_INFO_Fth,ICD_CHIP_INFO_LEN, buf);
    
    return ERROR_SUCCESS;   
}
/*
 *  时序下载    
 *  下载时序到K22 flash 中。
 *  
 */
uint8_t icd_download_timing(uint8_t *data_buf, uint8_t *code_buf)	
{
    uint16_t code_len;
    uint32_t code_addr;
    
    code_len = (data_buf[0] << 8) | data_buf[1];
    if(code_len > 0x400)
        return SEND_DATE_LENTH_ERROR;
    code_addr = data_buf[4] << 8 | data_buf[5];
    if((code_addr > 0xBC00)||(code_addr < 0x2800))      //0xBC00和0x2800 历史芯片遗留。对应历史芯片的code区地址
        return SEND_DATE_ADDR_ERROR;
            
//    union B32_B08 data_buf[ICD_CHIP_INFO_LEN/4];      //长度需要被4整除
//    icd_error_t status = ERROR_SUCCESS;
//    
//    addr -= 0x2800;     //历史遗留
//    memcpy(data_buf[0].B08, buf, ICD_CHIP_INFO_LEN);
 //   eslink_rom_write(CHIP_INFO_Fth+addr, size, &data_buf[0].B32);
    
    return ERROR_SUCCESS;       
    
}
//---------------------------------------------
//		设置main入口地址
//---------------------------------------------
uint8_t icd_set_main_addr(uint8_t *wr_buf)	
{
	icd_main.addr =((uint16_t)(wr_buf[ICD_DATA_OFFSET + 2] & 0xff) << 8) + wr_buf[ICD_DATA_OFFSET+3];  
    return ERROR_SUCCESS;	    
}


//8位芯片调试功能
// Execute DAP command (process request and prepare response)
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
uint32_t debug_process_command(uint8_t *request, uint8_t *response)     
{
    uint16_t i;
    uint8_t error ;
    uint16_t checksum;    
    
    icd_data.wrbuf = request;
    
    icd_data.frame_head = (request[0] << 24) | (request[1] << 16) | (request[2] << 8) | (request[3]);
    if(icd_data.frame_head != ICD_FRAME_HEAD){
        icd_error_ack(HEAD_ERROR);
    }
    
    icd_data.data_length = request[4]<<8 | request[5];
    
    if(request[6] != ICD_DEV_ADDR)
        icd_error_ack(DEV_ADD_ERROR);
    icd_data.device_addr = ICD_DEV_ADDR;
    if(check_sum( icd_data.data_length -2, request) != ((request[icd_data.data_length-2] << 8) | request[icd_data.data_length-1]))
        icd_error_ack(CHECKSUM_ERROR);
    
    icd_data.fun_code = request[7];
    
//    
//    memset(icd_data.rdbuf, 0, sizeof(response));
    icd_data.rdbuf = response;
    
    switch(icd_data.fun_code){
        case ID_ICD_HandShake:      //0x01
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;
            icd_data.rdbuf[ICD_DATA_OFFSET] = 1;//com_interface;// communication mode
			icd_data.rdbuf[ICD_DATA_OFFSET+1] = ICD_SOFT_VER1;//record software version	
			icd_data.rdbuf[ICD_DATA_OFFSET+2] = ICD_SOFT_VER2;//
			icd_data.rdbuf[ICD_DATA_OFFSET+3] = 0x01;// record hardware version	
			icd_data.rdbuf[ICD_DATA_OFFSET+4] = 0x00;//
			icd_data.rdbuf[ICD_DATA_OFFSET+5] = 0x00;// record FPGA version	
			icd_data.rdbuf[ICD_DATA_OFFSET+6] = 0x00;//
			icd_data.rdbuf[ICD_DATA_OFFSET+7] = 0x00;// record FPGA version	
			icd_data.rdbuf[ICD_DATA_OFFSET+8] = 0x00;
            
            PORT_ISP_SETUP();
        
            error = ERROR_SUCCESS;
            break;
        case READ_CHIP_INFO:        //0x02
            error = icd_read_chipinfo(&icd_data.rdbuf[ICD_CODE_OFFSET]);
            //add by 9527 for test
            uint8_t test[22] = {0x00, 0x01, 0xa2, 0x9c, 0x01, 0x00, 0xeb, 0x00, 0xff,0xff, 0xff,0xff, 0x00,0x00, 0x00,0x00, 0x00, 0x01, 0x00, 0x00, 0x40, 0x00 } ;            
            for(i=0; i<22; i++)
            {
                icd_data.rdbuf[ICD_CODE_OFFSET+i] = test[i];
            }
                                                  

            icd_data.data_length = ICD_ACK_NORMAL_LENTH + ICD_CHIP_INFO_LEN;
            break;
        case DL_CHIP_INFO: 
            error = icd_write_chipinfo(&icd_data.wrbuf[ICD_CODE_OFFSET]);
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;
            break;
        case DL_SCHEDULE_HEX:      //下载时序文件 

            error = icd_download_timing(&icd_data.wrbuf[ICD_DATA_OFFSET], &icd_data.wrbuf[ICD_CODE_OFFSET]);  
            break;
        case DL_SCHEDULE_HEX_END:
            break;
        
        case Voltage_set:           //0x06 电压设置
            break;
        case READ_VOLTAGE_SET_INFO:  //0x07 读电压    //读电压设置值
            break;
        
        case Chipset:               //0x10 芯片信号选择
            error = ERROR_SUCCESS;
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;
            
            break;
        
        case DownloadConfig: 		//0x11 配置字下载download config			  
            error = download_config(icd_data.wrbuf);           // set config    flash.c
       	    icd_data.data_length = ICD_ACK_NORMAL_LENTH;     
            break;

        case PCdownload:      // 0x12 用户程序下载handle download programme 
                               //通过ISP下载用户hex到调试的目标芯片中。
//			  RUN_LED = 0;	
//			  OK_LED = 1;
//			  FAIL_LED = 1; 
            error = icd_download_user_hex_begin(icd_data.wrbuf);  
//	          RUN_LED = 1;
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;     
            break;

		case PC_downloadend:	//0x13 用户程序下载完成 programme download end 
//			 RUN_LED = 0;
//			 EndDownLoad();
//           RUN_LED = 1;
            error = icd_download_user_hex_end();
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;  
            break;
        case Download_NoStopLab:		//0x14		//add by v2.03 for c debug
            //add by 9527 .新版本库文件地址由上位机判断。只保留正确应答。
            error = Ack_DownloadNoStopLab();
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;    
		    break;
        case ID_SET_MAIN_ADDR:  //0x15
            error =  icd_set_main_addr(icd_data.wrbuf);
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;  
			 break;
        case EMU_mrst:              //0x20  复位
            error = ES_reset();

            if(error == ERROR_SUCCESS)                   
                error = update_chip_status(icd_data.rdbuf, &icd_data.data_length);                 
                             
            
            break;
        case ID_EMU_RUN:            //0x21  全速运行
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;    
            error = ERROR_SUCCESS;
            os_evt_set(FLAGS_ICD_RUN, icd_task_id);
			break;
        case ID_EMU_ASM_STEP:           //0x22 汇编单步
            error = ES_asm_step();
            if(error == ERROR_SUCCESS)            
                error = update_chip_status(icd_data.rdbuf, &icd_data.data_length);                               
                 
            break;
        case ID_EMU_ASM_STEP_OVER:  //0x23  汇编步越
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;  
            error = ERROR_SUCCESS;
            os_evt_set(FLAGS_ICD_ASM_STEP_OVER, icd_task_id);
            break;
        
        case EMU_halt:          //停止 0x24
            
            error = ES_stop();   
            if(error == ERROR_SUCCESS)       
            {   //mcu halt
                icd_debug_flag.stop = SET;      
                error = update_chip_status(icd_data.rdbuf, &icd_data.data_length);         
            }   
            break;     
        case C_step:                    //C单步 0x25
            for(i=0; i<ICD_DATA_LEN; i++)
            {
                icd_data.rdbuf[i+ICD_DATA_OFFSET] = icd_data.wrbuf[i+ICD_DATA_OFFSET];        
            }
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;   
            error = ERROR_SUCCESS;
            os_evt_set(FLAGS_ICD_C_STEP, icd_task_id);
            break;
        case C_stepover:                //C步越 0x26
            for(i=0; i<ICD_DATA_LEN; i++)
            {
                icd_data.rdbuf[i+ICD_DATA_OFFSET] = icd_data.wrbuf[i+ICD_DATA_OFFSET];        
            }
            icd_data.data_length = ICD_ACK_NORMAL_LENTH; 
            error = ERROR_SUCCESS;
            os_evt_set(FLAGS_ICD_C_STEP_OVER, icd_task_id);
            break;
        case EMU_stepout:               //步出 0x27
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;  
            error = ERROR_SUCCESS;
            os_evt_set(FLAGS_ICD_STEP_OUT, icd_task_id);
            break;
        
        case SetintPointer:             //0x28  设置PC断点
            error = ES_set_breakpoint(icd_data.wrbuf);            
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;    
            break;
        case ClrintPointer:            //0x29 清除PC断点
            error = ES_clr_breakpoint(icd_data.wrbuf);	
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;    
            break;
        
        case Delallbreakpointer:       //0x2A清除所有PC断点
            error = ES_clr_all_breakpointer ();
            error = ERROR_SUCCESS;
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;    
            break;  
        case Set_bk_condition:          //0x2B  设置条件
            error = ES_set_condition_breakpoint(icd_data.wrbuf);
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;   
            break;
        case ReadRam:                   //0x32 读寄存器段
            error = ES_read_ram(icd_data.wrbuf, icd_data.rdbuf, &icd_data.data_length);
            break;
        case ReadPc:                //读PC值 0x35
            break;
        case ReadUsedStack:             //0x36 读已用堆栈内容

            error = ES_read_used_stack(icd_data.rdbuf, &icd_data.data_length);          	// read stack content		// 
            break;
        case ID_ICD_DisplayAllStack:          //0x37 读全部堆栈内容
            error = ES_read_all_stack(icd_data.rdbuf, &icd_data.data_length);
            break;
 	
		case Resend_ack:			    //0x3A 要求重发数据
//			  Ack_ReSend_Ack();
            error = ERROR_SUCCESS;
            break;

        case Writeram :                //0x40 写寄存器数据
       		  //填充内存块
            error = ES_modify_ram(icd_data.wrbuf, icd_data.rdbuf, &icd_data.data_length);   		// 
       		
        	break;

        case ModifycunrrentPc:         //0x41 修改PC值
            error = ES_modify_PC(&icd_data.wrbuf[ICD_DATA_OFFSET]);			// 
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;  
            break;

        case Import_Various_RAM:       //0x47 导入寄存器数据
		    error = ES_import_ram(icd_data.wrbuf, icd_data.rdbuf, &icd_data.data_length);  	
            
		      break;
        case UpdateChipStatus: //0x70  收到主控的应答
            if(icd_debug_flag.monitor == SET)
            {
                error = update_chip_status(icd_data.rdbuf, &icd_data.data_length);   
                icd_debug_flag.monitor = RESET ;          
            }
            else
            {
                error = 0xF3;       //没有需要上报的数据
                
            }
            break;

        default:
            break;
        
    }

    if(error != ERROR_SUCCESS)
    {
        icd_data.device_state = error;   
        icd_data.data_length = ICD_ACK_NORMAL_LENTH;
    }
    else
    {
        icd_data.device_state = ICD_ERR_RUN_OK;
    }
    icd_data.rdbuf[0] = 0xAB;
    icd_data.rdbuf[1] = 0xCD;
    icd_data.rdbuf[2] = 0xDC;
    icd_data.rdbuf[3] = 0xBA;  
    icd_data.rdbuf[4] = icd_data.data_length >> 8;
    icd_data.rdbuf[5] = icd_data.data_length & 0xFF;
    icd_data.rdbuf[6] = icd_data.device_addr;
    icd_data.rdbuf[7] = icd_data.fun_code;
    icd_data.rdbuf[icd_data.data_length - 3] = icd_data.device_state;
    checksum = check_sum(icd_data.data_length-2,icd_data.rdbuf);

    icd_data.rdbuf[icd_data.data_length - 2] = (uint8_t)(checksum >> 8);
    icd_data.rdbuf[icd_data.data_length - 1] = (uint8_t) checksum; 
    
    return icd_data.data_length;

    
 /* 
    // 先应答，在进行对目标芯片的操作    
    switch(icd_data.fun_code){        
        case ID_EMU_RUN:                //0x21  全速运行 IDE上位启动
            error = ES_run();	
            break;
        case ID_EMU_ASM_STEP_OVER:      //0x23  汇编步越
            error = ES_asm_step_over();
            break;
        case EMU_stepout:
            error = ES_asm_step_out();	//步出 0x27
            break;
//        case C_step:                    //C单步 0x25
//            error = ES_C_step(&icd_data.wrbuf[ICD_DATA_OFFSET]);		
//            break;
//        case C_stepover:
//            error = ES_C_stepover(&icd_data.wrbuf[ICD_DATA_OFFSET]);	
//            break;
        default:
            break;
        
    }
    */
}


// ICD task ；部分命令不放在中断中处理。（处理事件可能会较长，所以不放在中断中处理）
__task void icd_process_continue(void *argv)
{
    uint8_t error;
    uint16_t flags = 0;
    
    icd_task_id = os_tsk_self();
    os_evt_set(FLAGS_HALT_CHECK, icd_task_id);
    while(1)
    {
        os_evt_wait_or( FLAGS_HALT_CHECK
                      | FLAGS_ICD_RUN
                      | FLAGS_ICD_ASM_STEP_OVER
                      | FLAGS_ICD_STEP_OUT
                      | FLAGS_ICD_C_STEP
                      | FLAGS_ICD_C_STEP_OVER
                        ,0xFFFF);                      
        // Find out what event happened
        flags = os_evt_get();     
        
        if (flags & FLAGS_ICD_RUN)    //0x21  全速运行 IDE上位启动
            error = ES_run();
        
        if (flags & FLAGS_ICD_ASM_STEP_OVER)    //0x23  汇编步越
            error = ES_asm_step_over();  
        
        if (flags & FLAGS_ICD_STEP_OUT)
            error = ES_asm_step_out();	        //步出 0x27
        
        if (flags & FLAGS_ICD_C_STEP)
            error = ES_C_step(icd_data.wrbuf);		 //C单步 0x25
        
        if (flags & FLAGS_ICD_C_STEP_OVER)
            error = ES_C_stepover(icd_data.wrbuf);	
        
        if (flags & FLAGS_HALT_CHECK) 
        {
            if(icd_target_state == ICD_RUN)
            {
                if(icd_halt_check() == ERROR_SUCCESS)
                    icd_debug_flag.monitor = SET;
            }            
            
        }     

        if(icd_main.set_point == SET)
        {
            icd_write_str(unset_breakpoint_1640,icd_main.addr); //c程序中main的入口地址20111206);
            icd_write_str(set_breakpoint_1640,icd_main.pointer_back);
            icd_main.set_point = RESET;
        }
        os_evt_set(FLAGS_HALT_CHECK, icd_task_id);
        os_dly_wait(10);       
        
    }    
}



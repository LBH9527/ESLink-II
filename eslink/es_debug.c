#include "eslink.h" 
#include "main.h" 
#include "es_debug.h" 
#include "sflash_port.h"
#include "isp_prog_intf.h"   
#include "settings_rom.h"
#include "update.h" 
 
#include "hr7p169b_icd.h"
//配置字需要到IDESpace 中确定地址后，写入指定的地址
#define CONFIG_WORD_LEN         (0x14)      //长度可以根据config_word_size = 0x28,     
                                            //配置字信息区容量来确认 
uint16_t config[CONFIG_WORD_LEN];

//// Event flags for icd process task
//芯片停止检查任务
#define FLAGS_HALT_CHECK        (1 << 0)       
#define FLAGS_DBG_RUN           (1 << 1)
#define FLAGS_DBG_ASM_STEP_OVER (1 << 2)
#define FLAGS_DBG_C_STEP        (1 << 3)
#define FLAGS_DBG_C_STEP_OVER   (1 << 4)
#define FLAGS_DBG_STEP_OUT      (1 << 5)
static uint32_t  debug_event = 0;

//仿真目标芯片信息
static es_target_cfg dbg_target_device = {
    .code_start = 0,             //主程序区起始地址
    .code_size = 0x4000,              //主程序区容量           
    .config_word_start = 0,     //配置字信息区起始地址
    .config_word_size = 0x28,      //配置字信息区容量
    .encrypt_addr = 0,           //加密字地址
    .encrypt_value = 0,          //加密字值
    .chipid_addr = 0x80fc,           //芯片ID地址
    .chipid_value = 0x02850161,          //芯片ID值


};  
//仿真目标芯片状态
debug_target_state target_state;   

debug_breakpoint_t debug_breakpoint;        //断点  

debug_flag_t debug_flag;        //停止和上报标志
debug_main_t debug_main;        //main函数地址设置

static uint16_t start_addr;     //c单步， c步越 PC区间的首地址
static uint16_t end_addr;       //c单步， c步越 PC区间的末地址

static bool flash_initialized = false;

/*
 *  读芯片信息
 */
error_t read_hr_chipinfo(uint8_t *buf)
{
    error_t result = ERROR_SUCCESS;
    uint8_t len ;
    len = sizeof(hr_target_cfg)  ;
    
    if(get_hr_target_info(buf+FRAME_CODE_OFFSET) != len )
        result = ERROR_IAP_READ;  
    return result;  
}

/*
 *  时序下载    
 *  下载时序到K22 flash 中。
 *  
 */
static error_t download_timing(uint8_t *data_buf)	
{    
    
}
/*
 * 芯片信息下载
 * 擦除芯片信息区，并保存新的数据
 */
static error_t es_write_chipinfo(uint8_t *buf)
{
//    union B32_B08 data_buf[ICD_CHIP_INFO_LEN/4];      //长度需要被4整除
    
//    memcpy(data_buf[0].B08, buf, ICD_CHIP_INFO_LEN);
//    spi_flash_write(CHIP_INFO_Fth,ICD_CHIP_INFO_LEN, buf);
    
    return ERROR_SUCCESS;   
}
/*
 *  下载芯片信息
 */
static error_t download_hr_chipinfo(uint8_t *data)	
{
    error_t result = ERROR_SUCCESS;
    
    if(set_hr_timing_info(data) != TRUE )
        result = ERROR_IAP_WRITE; 
    
    
    if(set_app_update(UPDATE_LINK_APP) != TRUE )
        result = ERROR_IAP_WRITE; 
    return ERROR_SUCCESS;           
}    

/*
 *  配置字下载
 */
static error_t download_config_word(uint8_t *data)
{
    uint16_t i = 0;
    uint16_t size;
    
    size = data[4]<<8 | data[5] - 0x14;    //代码长度    
    
    //config0       //配置字
    for(i=0; i<size/2; i++)
    {
        config[i] = (data[0x11 + i*2] << 0x08) | data[0x11 + i*2 + 1];    
    }
    //下发数据没有对配置字取反，需要手动取反
    config[0x02] = ~config[0x01] ;       
    config[0x12] = config[0x10];
    config[0x13] = config[0x11];
    config[0x10] = 0xff;
    config[0x11] = 0xff;
    return ERROR_SUCCESS;
}

//debug　用户hex下载开始
//Note： 通过ISP方式将接收到的数据下载到目标芯片的flash中。
error_t download_userhex(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
 
    size = data[0]<<8 | data[1];    //代码长度
    addr = data[2]<<24 | data[3] << 16 | data[4]<<8 | data[5];  //起始地址
    
    if (!flash_initialized) 
    {
        uint32_t failaddr;
        isp_prog_intf.init(&dbg_target_device);
        ret = isp_prog_intf.prog_init();
        if(ERROR_SUCCESS != ret)
            return  ERR_UNLOCK;
        ret = isp_prog_intf.erase_chip(0);
        if( ERROR_SUCCESS != ret)
            return ERR_ERASE;
//        ret = isp_prog_intf.program_config_word( dbg_target_device.config_word_start, (uint8_t*)config, dbg_target_device.config_word_size, &failaddr);
        if( ERROR_SUCCESS != ret)
            return ERR_ERASE;
        flash_initialized = true;
    }
    if (flash_initialized) 
    {
        ret = isp_prog_intf.program_flash(addr, data+FRAME_CODE_OFFSET, size, NULL); 
        if( ERROR_SUCCESS != ret)
             return ERR_PGM;
        return ERROR_SUCCESS;  
    }  
    return ERROR_SUCCESS;  
}

//icd　用户hex下载完成
//方式 01：下载  02:编程
error_t download_userhex_end(void)
{    
    error_t ret = ERROR_SUCCESS;
//    ret = isp_prog_intf.chipid_check(); //判断chipid
    isp_prog_intf.prog_uninit();        //退出isp模式
    if(ERROR_SUCCESS != ret)
        return  ERR_CHIP_ID_NOT_MATCH;
    flash_initialized = false;
    
    //清空断点地址
    memset(debug_breakpoint.addr, 0 , sizeof(debug_breakpoint.addr));
    //设置断点标志
    debug_breakpoint.empty = TRUE;          //没有断点
    //main地址是否需要设置
    debug_main.set_point = RESET;           //默认不设置端点在main函数。    
    debug_flag.stop = RESET;
    target_state = ICD_HALT;   

    ISP_SETUP();    
    PORT_ISP_SETUP();       
    PIN_RST_OUT(1);
    
    return ERROR_SUCCESS; 
}



//设置main入口地址
error_t set_main_addr(uint8_t *data)	
{
    //Main函数首地址， 取低字节
	debug_main.addr =((uint16_t)(data[2] & 0xff) << 8) + data[3];  
    return ERROR_SUCCESS;	    
}


/*******************************************************************************
*   函 数 名: dbg_reset_target
*   功能说明: 芯片复位			
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
error_t dbg_reset_target(void)
{
	uint8_t   i;
	uint16_t  halt_data;
    
    if(target_state != ICD_HALT)
        return ERR_CHIP_INFO; 

    if(icd_chip_reset() !=  TRUE)
        return ERR_CHIP_INFO;
	es_delay_ms(2);//ndelay(3000);//2.1ms

    target_state = ICD_HALT;                                //目标芯片状态更新    
    if(icd_halt_check() != TRUE)                            //停止查询
        return ERR_CHIP_INFO;
    
    if(icd_write_str(STPWHL_REG_ADDR, 0, 1)!= TRUE)         //stopwatch 清零
        return ERR_CHIP_INFO;  
    //写断点
	for(i = 0; i < BREAKPOINT_NUM; i++)			            
	{     
		if(debug_breakpoint.addr[i] != 0x00)
		{
			debug_breakpoint.empty = FALSE;
            if(icd_write_str(SPCBK_REG_ADDR,&debug_breakpoint.addr[i], 1)!= TRUE)  
                return ERR_CHIP_INFO;
		}		
    } 
    //芯片停止标志复位   
    debug_flag.stop = RESET;  

    return ERROR_SUCCESS;	  
}
/*******************************************************************************
*   函 数 名: dbg_run
*   功能说明: 芯片运行		
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
error_t dbg_run(void)				//
{
	if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE)  //stopwatch 清零
        return ERR_CHIP_INFO;    

    if(target_state != ICD_HALT)
        return ERR_CHIP_INFO;  
        
    if(icd_run() !=  TRUE)
        return ERR_CHIP_INFO;
    
    target_state = ICD_RUN;            //目标芯片状态更新
    
    //等待芯片停止
    while(icd_halt_check() != TRUE) ;
    //上报芯片信息
    debug_flag.monitor = SET;	
    
    return ERROR_SUCCESS	;	
}
/*******************************************************************************
*   函 数 名: dbg_halt
*   功能说明: 芯片停止		
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
error_t dbg_halt(void)
{
    if(icd_halt() !=  TRUE)
        return ERR_CHIP_INFO;
    
    if(icd_halt_check() != TRUE)
        return ERR_CHIP_INFO;
    
    target_state = ICD_HALT;            //目标芯片状态更新
    
    return ERROR_SUCCESS	;	    
}
/*******************************************************************************
*   函 数 名: dbg_asm_step
*   功能说明: 汇编单步	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
error_t dbg_asm_step(void)
{   
    uint8_t i;
    
    if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE)       //stopwatch 清零
        return ERR_CHIP_INFO;    
    
    if(target_state != ICD_HALT)
        return FALSE;
        
    if(icd_step()!= TRUE)  
         return ERR_CHIP_INFO; 
         
    target_state = ICD_RUN;                     //目标芯片状态更新
    
    for(i=0; i<10; i++)
    {
        if(icd_halt_check() != FALSE)
            break;             
    }
    if(i >= 10)
        return ERR_CHIP_INFO; 

    target_state = ICD_HALT;                    //目标芯片状态更新      
    return ERROR_SUCCESS;	
}
/*******************************************************************************
*   函 数 名: dbg_asm_step_out
*   功能说明: 汇编步出	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
error_t dbg_asm_step_out(void)
{
    if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE)        //stopwatch 清零
        return ERR_CHIP_INFO;
    
    if(target_state != ICD_HALT)
        return ERR_CHIP_INFO;
        
    if(icd_step_out()!= TRUE)       
        return ERR_CHIP_INFO;
    
    target_state = ICD_RUN;            //目标芯片状态更新
    
    if(icd_halt_check() != TRUE)
        return ERR_CHIP_INFO;
    
    target_state = ICD_HALT;            //目标芯片状态更新
    
    return ERROR_SUCCESS;	
}
/*******************************************************************************
*   函 数 名: emu_asm_step_over
*   功能说明: 汇编步跃	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
error_t dbg_asm_step_over(void)
{
    if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE)        //stopwatch 清零
        return ERR_CHIP_INFO;
    
    if(target_state != ICD_HALT)
        return FALSE;
        
    if(icd_step_over()!= TRUE)       
        return ERR_CHIP_INFO;         
    target_state = ICD_RUN;            //目标芯片状态更新  
    
    if(icd_halt_check() != TRUE)
        return ERR_CHIP_INFO;        
    target_state = ICD_HALT;            //目标芯片状态更新  
    
    return ERROR_SUCCESS;
    
}
/*******************************************************************************
*   函 数 名: dbg_C_step
*   功能说明: C单步。
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/
error_t dbg_C_step(void)	
{
    uint8_t i;
    uint16_t pc_value,read_data;
    
    //stopwatch 清零
	if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE)       
        return ERR_CHIP_INFO;   
        
    if(icd_halt_check() != TRUE)
        return ERR_CHIP_INFO;  
        
    target_state = ICD_HALT;    
    //判断是否为复位状态
    for(i=0;i<=2;i++)		
    {
        if(icd_read_str(PCR_REG_ADDR, &pc_value , 1 ) != TRUE)    //复位后pc为0，c单步直接到main 20111206
            return ERR_CHIP_INFO;
        if(pc_value!=0)        
            break;     
    }      
    if(pc_value == 0)
	{
        //设置断点到main
        if(icd_write_str(CDBK_REG_ADDR,&debug_main.addr, 1)!= TRUE) //set breakpoint
            return ERR_CHIP_INFO;

        if(icd_run() != ERROR_SUCCESS)
            return ERR_CHIP_INFO;
            
        target_state = ICD_RUN;            //目标芯片状态更新
		return ERROR_SUCCESS ;
	}  
    
    //接收到上位机的停止命令
	while(1)
	{          
        if(icd_step() != TRUE) 
            return ERR_CHIP_INFO;
        //等待芯片停止
        for(i=0; i<10; i++)
        {
            if(icd_halt_check() != FALSE)
                break;             
        }
        if(i >= 10)
            return ERR_CHIP_INFO; 
        
        if(debug_flag.stop == SET)   //接收到停止命令
        {
             debug_flag.stop = RESET; 
             return ERROR_SUCCESS;    
        }
        
        //读状态寄存器，判断有没有溢出标志
        if(icd_read_str(PSW, &read_data, 1 ) != TRUE)    // set sram initial address		
            return ERR_CHIP_INFO;    		
		if((read_data&0x0060) != 0 )break;

        //读pc值，判断是否为复位值
        for(i=0;i<=2;i++)		
        {
            if(icd_read_str(PCR_REG_ADDR, &pc_value, 1 ) != TRUE)    // set sram initial address		
                return ERR_CHIP_INFO;
            if(pc_value!=0)
                break;     
        }   
		if(pc_value == 0)	//从程序最后c单步回到main 20111206
		{
            if(icd_write_str(CDBK_REG_ADDR, &debug_main.addr, 1)!= TRUE) //set breakpoint
                return ERR_CHIP_INFO;
            if(icd_run() != ERROR_SUCCESS)
                return ERR_CHIP_INFO;
//            debug_flag.monitor = SET;		
            return ERROR_SUCCESS;
		}
        //判断是否单步到断点的位置
        if(debug_breakpoint.empty != TRUE)
        {
		    for(i = 0x00; i < BREAKPOINT_NUM; i++)
 			{				
				if( debug_breakpoint.addr[i] == pc_value)
   		     	{	
                    //c单步 遇到断点地址，则需要上传芯片运行停止信息
      			 	debug_flag.monitor = SET;		
					return ERROR_SUCCESS;
      			}    
 			}             
        }
        //判断有没有单步，步出地址区间
		if( (pc_value < start_addr) || (pc_value > end_addr)||( start_addr == end_addr) )  
		{
            //c单步 步出下发的PC地址区间，则需要上传芯片运行停止信息
            debug_flag.monitor = SET;	
            return ERROR_SUCCESS;
             
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
*   函 数 名: dbg_C_stepover
*   功能说明: C步越	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/  
error_t dbg_C_stepover(void)	
{    
    uint8_t i;
    uint16_t pc_value,read_data;
    
    //stopwatch 清零
    if(icd_write_str(STPWHL_REG_ADDR,0, 1) != TRUE) 	
        return ERR_CHIP_INFO;

    if(icd_halt_check() != TRUE)
        return ERR_CHIP_INFO;
    target_state = ICD_HALT;
	while(1)
	{
        if(icd_step_over() != TRUE)    // set sram initial address		
            return ERR_CHIP_INFO;
        //等待芯片停止
        for(i=0; i<10; i++)
        {
            if(icd_halt_check() != FALSE)
                break;             
        }
        if(i >= 10)
            return ERR_CHIP_INFO; 
        
        if(debug_flag.stop == SET)   //接收到停止命令
        {
             debug_flag.stop = RESET; 
             return ERROR_SUCCESS;    
        }
        
        //判断程序状态寄存器是否有溢出标志
		if(icd_read_str(PSW, &read_data, 1 ) != TRUE)    // set sram initial address		
            return ERR_CHIP_INFO;
        if((read_data&0x0060) != 0 )break;
        
        //判断程序是否运行到断点位置
        if(icd_read_str(PCR_REG_ADDR, &pc_value , 1) != TRUE)    
                return ERR_CHIP_INFO; 
        if(debug_breakpoint.empty != TRUE)
        {
		    for(i = 0x00; i < BREAKPOINT_NUM; i++)
 			{				
				if( debug_breakpoint.addr[i] == pc_value)
   		     	{			
                    debug_flag.monitor = SET;	
					return ERROR_SUCCESS;    
      			}    
 			}            
            
        }
        //判断程序是否走出程序空间
		if( (pc_value < start_addr) || (pc_value > end_addr)||( start_addr == end_addr) )  
		{	
            //c单步 步出下发的PC地址区间，则需要上传芯片运行停止信息
            debug_flag.monitor = SET;	
            return ERROR_SUCCESS;
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

    return ERROR_SUCCESS;    
}
/*******************************************************************************
*   函 数 名: ES_modify_PC
*   功能说明: 修改PC值	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/ 
error_t dbg_modify_PC(uint8_t *data)				
{    	
	uint16_t   pc1, pc2;
	//-------------------------------------------------------// read current PC
	pc1 =  (data[0] << 8) | data[1] ; //get  new PC 	
	pc2 =  (data[2] << 8) | data[3];  //get  new PC
//------------------------------------------------------------ // write PC

    if(icd_write_str(PCR_REG_ADDR, &pc2, 1)  != TRUE)
        return ERR_CHIP_INFO;
    //写PCR操作完成后，需要等待500us以上
    es_delay_ms(1);
    
    return ERROR_SUCCESS;    
}

/*******************************************************************************
*   函 数 名: dbg_read_PC
*   功能说明: 读PC值	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/ 
error_t dbg_read_PC(uint8_t *buf)
{   
    uint8_t i = 0;
    uint16_t pc_data;
    
    if(icd_read_str(PCR_REG_ADDR, &pc_data, 1 ) != TRUE)
        return ERR_CHIP_INFO;
    buf[i++] = 0;
    buf[i++] = 0;
    buf[i++] = (pc_data >> 8) & 0xff;
    buf[i++] = pc_data & 0xff;
    
    if(icd_read_str(LSTPCR_REG_ADDR, &pc_data, 1 ) != TRUE)
        return ERR_CHIP_INFO;
    buf[i++] = 0;
    buf[i++] = 0;
    buf[i++] = (pc_data >> 8) & 0xff;
    buf[i++] = pc_data & 0xff;
     
    return ERROR_SUCCESS;
}

/*******************************************************************************
*   函 数 名: dbg_set_breakpoint
*   功能说明: 设置PC断点	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/ 
error_t dbg_set_breakpoint(uint8_t *buf)		//
{   
	uint8_t i;
	uint16_t  bkpt_addr;

    bkpt_addr = (buf[2] << 8) + buf[3];	// get address

    for(i = 0; i < BREAKPOINT_NUM; i++)
    {				
        if( debug_breakpoint.addr[i] == bkpt_addr)
            return ERROR_SUCCESS;       //断点地址已经存在buffer中，不需要设置此端点
 	}        
    //查询断点buf 是否有空。    
	for(i = 0; i < BREAKPOINT_NUM; i++)	// modify breakpoint buffer
	{     	    	
		if(debug_breakpoint.addr[i] == 0x00)           
            break;            
    }      
    if(i >= BREAKPOINT_NUM)     //断点以满
    {
         return ERR_CHIP_INFO;  
    }           
    else
    {
        if(icd_write_str(SPCBK_REG_ADDR, &bkpt_addr, 1)  != TRUE)
            return ERR_CHIP_INFO;         
        debug_breakpoint.addr[i] = bkpt_addr;     
    }   
    
    return ERROR_SUCCESS;     
}
/*******************************************************************************
*   函 数 名: dbg_clr_breakpoint
*   功能说明: 清除断点	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/ 
error_t dbg_clr_breakpoint(uint8_t *buf)		
{	
	uint8_t i;
    uint16_t bkpt_addr;
	
    bkpt_addr = (buf[2] << 8) + buf[3];	// get address

	if(icd_write_str(CPCBK_REG_ADDR, &bkpt_addr, 1)!= TRUE)  //清除断点
        return ERR_CHIP_INFO;
	
	for(i = 0; i < BREAKPOINT_NUM; i++)	// clear breakpointer buffer
	{    
 	 	if(debug_breakpoint.addr[i] == bkpt_addr)
        {
         	debug_breakpoint.addr[i] = 0x00;
        	break;
        }
    }  
    //检查断点是否为0
	for(i = 0; i < BREAKPOINT_NUM; i++)		// check breakpointer buffer is empty or no
	{       
		if(debug_breakpoint.addr[i] != 0x00) 
			break;	
  	}		
    if(i >= BREAKPOINT_NUM)  
        debug_breakpoint.empty = TRUE;
        
    return ERROR_SUCCESS	;	
}
/*******************************************************************************
*   函 数 名: ES_clr_all_breakpointer
*   功能说明: 清除所有断点	
*   形    参: 无
*   返 回 值: 无
*******************************************************************************/ 
error_t dbg_clr_all_breakpointer(void)  // clr  all breakpointer
{
	uint8_t  i;

	if(icd_write_str(CPCBK_REG_ADDR, 0x0000, 1) != TRUE)	// clr  all breakpoint
        return ERR_CHIP_INFO;

	for(i = 0; i < BREAKPOINT_NUM; i++)
	{	
		if( debug_breakpoint.addr[i] !=0)
		{
			debug_breakpoint.addr[i] = 0x00;	
		}
	}
    debug_breakpoint.empty = TRUE;
    return ERROR_SUCCESS	;
}

///********************************************************************/
///* To send back all sram 								        	*/
///********************************************************************/
error_t update_chip_status(uint8_t *rd_buf)				                 
{
    uint16_t  read_data;

    //a reg
    if(icd_read_str(AREG, &read_data, 1) != TRUE)    //0xFF85 累加器A寄存器
        return ERR_CHIP_INFO;
    rd_buf[0] = read_data & 0xff;
    //b reg
    rd_buf[1] = 0;               //7P196 no b reg
    
    //current pc
    if(icd_read_str(PCR_REG_ADDR, &read_data, 1) != TRUE)    //9527
        return ERR_CHIP_INFO;
    rd_buf[FRAME_CODE_OFFSET] = 0;
    rd_buf[FRAME_CODE_OFFSET + 1] = 0;
    rd_buf[FRAME_CODE_OFFSET + 2] = (read_data>>8) & 0xff;
    rd_buf[FRAME_CODE_OFFSET + 3] = read_data & 0xff;
    //last pc
    if(icd_read_str(LSTPCR_REG_ADDR, &read_data, 1) != TRUE)    //9527
        return ERR_CHIP_INFO;
    rd_buf[FRAME_CODE_OFFSET + 4] = 0;
    rd_buf[FRAME_CODE_OFFSET + 5] = 0;
    rd_buf[FRAME_CODE_OFFSET + 6] = (read_data>>8) & 0xff;
    rd_buf[FRAME_CODE_OFFSET + 7] = read_data & 0xff;
    //STOPWATCH
    if(icd_read_str(STPWHH_REG_ADDR, &read_data, 1) != TRUE)    //9527
        return ERR_CHIP_INFO;
    rd_buf[FRAME_CODE_OFFSET + 8] = (read_data>>8) & 0xff;
    rd_buf[FRAME_CODE_OFFSET + 9] = read_data & 0xff;
    if(icd_read_str(STPWHL_REG_ADDR, &read_data , 1) != TRUE)    //9527
        return ERR_CHIP_INFO;
    rd_buf[FRAME_CODE_OFFSET + 10] =(read_data>>8) & 0xff;
    rd_buf[FRAME_CODE_OFFSET + 11] = read_data & 0xff; 
    
    return ERROR_SUCCESS;
}
///************************modify block sram************************************/
////写寄存器数据 0x40
////填充内存功能。
error_t dbg_modify_ram(uint8_t *wr_buf, uint8_t *rd_buf)	
{	
    uint16_t i;
	uint16_t len;
    uint16_t addr;
    uint16_t data;
    uint16_t read_data;
    
    len = wr_buf[0] << 8 |  wr_buf[1];  	//待写数据量
	addr = wr_buf[2] << 8 |  wr_buf[3];  //get  end address      
    data = wr_buf[4];   //待写数据
    
    for(i=0; i<FRAME_DATA_LEN; i++)
    {
        rd_buf[i] = wr_buf[i];        
    }

	for(i = 0; i < len; i++)
	{	//---------------------------------------------------------
        if(icd_write_str(addr, &data , 1) != TRUE)	
            return ERR_CHIP_INFO;
		if(icd_read_str(addr, &read_data, 1) != TRUE)    
            return ERR_CHIP_INFO;
		addr++;
        
      	rd_buf[FRAME_CODE_OFFSET + i] = read_data & 0xff;
    }       
    return ERROR_SUCCESS	;
}
	
//导入寄存器数据.
//说明：写入寄存器数据，并读出该数据
error_t dbg_import_ram_data(uint8_t *wr_buf, uint8_t *rd_buf)
{	
	uint16_t  addr;
    uint16_t data ;
	uint16_t i;
	uint16_t len;
    
    len = wr_buf[0] << 8 | wr_buf[1];   //导入寄存器数量
    for(i=0; i<FRAME_DATA_LEN; i++)
    {
        rd_buf[i] = wr_buf[i];        
    }
    
	for(i = 0; i < len; i++)
	{
		addr = wr_buf[FRAME_CODE_OFFSET + i * 3 ] << 8 | wr_buf[FRAME_CODE_OFFSET + i * 3 + 1 ];   //9527 数据大小端确认
        data = wr_buf[FRAME_CODE_OFFSET + i * 3 + 2 ];
        
        if(icd_write_str(addr, &data, 1)!= TRUE)
            return ERR_CHIP_INFO;	
        //再从改地址读出数据，回复给上位机判断  
        if(icd_read_str(addr, &data, 1) != TRUE)
            return ERR_CHIP_INFO; 
        
        rd_buf[FRAME_CODE_OFFSET + i * 3] =  wr_buf[FRAME_CODE_OFFSET + i * 3 ];
        rd_buf[FRAME_CODE_OFFSET + i * 3 + 1] =  wr_buf[FRAME_CODE_OFFSET + i * 3 + 1];
        rd_buf[FRAME_CODE_OFFSET + i * 3 + 2 ] = data;          
  	}

    return ERROR_SUCCESS	;
}

//读寄存器段
//addr读数据地址 len读数据长度， buf：接收数据缓存区
static error_t dbg_read_ram(uint8_t *wr_buf, uint8_t *rd_buf )	
{
    uint16_t i;
    uint16_t len, addr;
    uint16_t data;    
    
    len = (wr_buf[0] << 8) | wr_buf[1];
    addr = (wr_buf[2] << 8) | wr_buf[3];
    
    for(i=0; i<FRAME_DATA_LEN; i++)
    {
        rd_buf[i] = wr_buf[i];        
    }
    
    for(i=0; i<len; i++)
    {
        if(icd_read_str(addr, &data, 1) != TRUE)
            return ERR_CHIP_INFO;        
        addr++;
        rd_buf[FRAME_CODE_OFFSET + i] = (data  & 0xff );
    }
    
    return ERROR_SUCCESS	;
}


//读已用堆栈内容
error_t dbg_read_used_stack(uint8_t *rd_buf)					//display all levels stack //need modify
{
    uint8_t i, statck_number;
    uint16_t read_data;
  
  //读堆栈
    if(icd_read_str(STKLV_REG_ADDR, &read_data, 1) != TRUE)
        return ERR_CHIP_INFO;
    statck_number = read_data & 0xff;
    if( (statck_number & STACK_NUM) == STACK_NUM)
        statck_number = STACK_NUM;
    
    rd_buf[0] = statck_number;
    
	for(i = 0; i <statck_number; i++)	//
	{
        if(icd_read_str(STK0_REG_ADDR+i, &read_data, 1) != TRUE)
            return ERR_CHIP_INFO;
        rd_buf[FRAME_CODE_OFFSET + i*4] = 0;
        rd_buf[FRAME_CODE_OFFSET + i*4 + 1] = 0;    
        rd_buf[FRAME_CODE_OFFSET + i*4 + 2] = (read_data >> 8) & 0xff;    
        rd_buf[FRAME_CODE_OFFSET + i*4 + 3] = read_data & 0xff;   
	}
    return ERROR_SUCCESS	;
}			
//读全部堆栈内容
error_t dbg_read_all_stack(uint8_t *rd_buf)			//display all levels stack //need modify
{
  	uint16_t stack_data;
	uint8_t i;

    for(i=0; i<FRAME_DATA_LEN; i++)
    {
        rd_buf[i+FRAME_DATA_OFFSET] = 0;        
    }
    
    rd_buf[FRAME_DATA_OFFSET] = STACK_NUM;
    for(i = 0; i <STACK_NUM; i++)	//8?????
    {
        if(icd_read_str(STK0_REG_ADDR + i, &stack_data, 1) != TRUE)
            return ERR_CHIP_INFO;
        rd_buf[FRAME_CODE_OFFSET + i*4] = 0;
        rd_buf[FRAME_CODE_OFFSET + i*4 + 1] = 0;    
        rd_buf[FRAME_CODE_OFFSET + i*4 + 2] = stack_data>>8;    
        rd_buf[FRAME_CODE_OFFSET + i*4 + 3] = stack_data&0xff;        
    }    
    return ERROR_SUCCESS	;    
}	
		
//设置条件断点
error_t dbg_set_condition_breakpoint(uint8_t *buf)
{
	uint16_t condition_bkpt;
    
	//-----------------------------------------------------------------------------// condition register	
	condition_bkpt = (buf[0] << 8) |  buf[1];  // condition registerhigh8bit
    if(icd_write_str(BKS_REG_ADDR, &condition_bkpt, 1)  != TRUE)
        return ERR_CHIP_INFO;	
    
	condition_bkpt = (buf[2] << 8) |  buf[3];
    if(icd_write_str(STBK_REG_ADDR, &condition_bkpt, 1)  != TRUE)
        return ERR_CHIP_INFO;	
	
    condition_bkpt =(buf[4] << 8) |  buf[5];
    if(icd_write_str(ABK_REG_ADDR, &condition_bkpt, 1)  != TRUE)
        return ERR_CHIP_INFO;	
  	
    condition_bkpt =(buf[6] << 8) |  buf[7];
    if(icd_write_str(RBKA_REG_ADDR, &condition_bkpt, 1)  != TRUE)
        return ERR_CHIP_INFO;	  

    condition_bkpt =(buf[8] << 8) |  buf[9];
    if(icd_write_str(RBKD_REG_ADDR, &condition_bkpt, 1)  != TRUE)
        return ERR_CHIP_INFO;	  
    
    condition_bkpt =(buf[10] << 8) |  buf[11];
    if(icd_write_str(PCBKT_REG_ADDR, &condition_bkpt, 1)  != TRUE)
        return ERR_CHIP_INFO;	    
    
    condition_bkpt =(buf[12] << 8) |  buf[13];
    if(icd_write_str(RBKT_REG_ADDR, &condition_bkpt, 1)  != TRUE)
        return ERR_CHIP_INFO;	
    return ERROR_SUCCESS	;    
}


///********************************************************************
//* 下载禁止STOP的库文件地址集合       
//* 发送库函数PC地址段
//********************************************************************/
error_t Ack_DownloadNoStopLab(void)	
{
////	uint8_t *  lab_addr;
////	uint8_t i;

////	Lab_Num = buf[0];	
////	lab_addr = (uint8_t *)(serDataTest) + CODE_START_N + 2;
////	for(i=0; i<(Lab_Num*2); i++)
////	{
////		NoStopLab[i] = ((*(lab_addr+i*4))<<8) + *(lab_addr+i*4+1);	
////	}	
    return ERROR_SUCCESS;    //add by 9527 .新版本库文件地址由上位机判断。只保留正确应答。
}


debug_comm_frame_t dbg_data;

uint32_t debug_process_command(uint8_t *request, uint8_t *response)     
{
    uint16_t i;
    error_t result ;
    
    dbg_data.wrbuf = request;
    dbg_data.rdbuf = response; 
    
    dbg_data.frame_head = (request[0] << 24) | (request[1] << 16) | (request[2] << 8) | (request[3]);
    if(dbg_data.frame_head != DEBUG_FRAME_HEAD)
    {
        result = ERR_FRAME_HEAD;
        goto __ACK;
    }      
    dbg_data.data_length = request[4]<<8 | request[5];     
    if(request[6] != DEBUG_DEV_ADDR)
    {
        result = ERR_DEVICE_TYPE;
        goto __ACK;
    }
    LED_YELLOW_ON();
    dbg_data.device_addr = DEBUG_DEV_ADDR;   

    dbg_data.fun_code = request[7];    
//    if(check_sum( dbg_data.data_length -2, request) !=    \
//        ((request[dbg_data.data_length-2] << 8) | request[dbg_data.data_length-1]))
//    {      
//        result = ERR_CHECKSUM;
//        goto __ACK;
//    }    
    memset(dbg_data.rdbuf, 0, FRAME_ACK_NORMAL_LEN);
    dbg_data.rdbuf = response;
    
    switch(dbg_data.fun_code)
    {
        case ID_HANDSHAKE:                  //0x01                
            dbg_data.rdbuf[FRAME_DATA_OFFSET] = 1;//com_interface;// communication mode
			dbg_data.rdbuf[FRAME_DATA_OFFSET+1] = (ESLINK_ICD_DEBUG_VERSION>>8)&0xff; //record software version	
			dbg_data.rdbuf[FRAME_DATA_OFFSET+2] = ESLINK_ICD_DEBUG_VERSION&0xff;//
			dbg_data.rdbuf[FRAME_DATA_OFFSET+3] = 0x01;// record hardware version	
			dbg_data.rdbuf[FRAME_DATA_OFFSET+4] = 0x00;//
			dbg_data.rdbuf[FRAME_DATA_OFFSET+5] = 0x00;// record FPGA version	
			dbg_data.rdbuf[FRAME_DATA_OFFSET+6] = 0x00;//
			dbg_data.rdbuf[FRAME_DATA_OFFSET+7] = 0x00;// record FPGA version	
			dbg_data.rdbuf[FRAME_DATA_OFFSET+8] = 0x00;  
            
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
            result = ERROR_SUCCESS;
            break;
        case ID_READ_CHIP_INFO:             //0x02   读芯片信息
            result = read_hr_chipinfo(&dbg_data.rdbuf[FRAME_DATA_OFFSET]);
             dbg_data.data_length = FRAME_ACK_NORMAL_LEN + 0x200;
            break;
        case ID_DL_CHIP_INFO:               //0x03 下载芯片信息
            result = download_hr_chipinfo(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
            break;
        case ID_DL_SCHEDULE_HEX_START:  //时序下载开始 0x60
            //跳转到boot，开始更新时序
            main_reset(); 
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
            break;
//        case ID_DL_SCHEDULE_HEX:          //0x04 下载时序文件          
//            break;
//        case DL_SCHEDULE_HEX_END:         //0x05 时序下载完毕
//            break;
        
        case ID_VOLTAGE_SET:                //0x06 电压设置
            result = ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
            
            break;
        case READ_VOLTAGE_SET_INFO:         //0x07 读电压    //读电压设置值
            result = ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
            break;          
        case Chipset:                       //0x10 芯片信号选择
            result = ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;               
            break;        
        case ID_CONFIG_WORD_DL: 		    //0x11	配置下载 
            result = download_config_word(dbg_data.wrbuf);           // set config  
       	    dbg_data.data_length = FRAME_ACK_NORMAL_LEN;     
            break;   
        case ID_USERHEX_DL:                 // 0x12 用户程序下载
            LED_YELLOW_ON();    
            result = download_userhex(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);  
//            LED_GREEN_ON();
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;     
            break; 
		case ID_USERHEX_DL_END:	            //0x13 用户程序下载完成 programme download end 
            result = download_userhex_end();
            LED_GREEN_ON();
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
            break;
        case Download_NoStopLab:		    //0x14		//add by v2.03 for c debug
            //add by 9527 .新版本库文件地址由上位机判断。只保留正确应答。
            result = Ack_DownloadNoStopLab();
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;    
		    break;
        case ID_SET_MAIN_ADDR:              //0x15
            result =  set_main_addr(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
			 break;
        case ID_RESET:                      //0x20  复位
            result = dbg_reset_target(); 
            if(result == ERROR_SUCCESS)   
            {
                result = update_chip_status(&dbg_data.rdbuf[FRAME_DATA_OFFSET]);
                dbg_data.data_length = 12 + FRAME_ACK_NORMAL_LEN;                
            } 
            else
            {
                dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
            }                      
            break;
        case ID_RUN:                        //0x21  全速运行
            result = ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;              
            flag_send(debug_event, FLAGS_DBG_RUN);
			break;
        case ID_ASM_STEP:                   //0x22 汇编单步
            result = dbg_asm_step();
            if(result == ERROR_SUCCESS)   
            {
                result = update_chip_status(&dbg_data.rdbuf[FRAME_DATA_OFFSET]);
                dbg_data.data_length = 12 + FRAME_ACK_NORMAL_LEN;                
            } 
            else
            {
                dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
            }   
            break;
        case ID_ASM_STEP_OVER:              //0x23  汇编步越
            result = ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
            flag_send(debug_event, FLAGS_DBG_ASM_STEP_OVER);
            break;            
        case ID_HALT:                       //停止 0x24              
            result = dbg_halt();   
            if(result == ERROR_SUCCESS)       
            {   //mcu halt
                debug_flag.stop = SET;      
                result = update_chip_status(&dbg_data.rdbuf[FRAME_DATA_OFFSET]);
                dbg_data.data_length = 12 + FRAME_ACK_NORMAL_LEN;                
            } 
            else
            {
                dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
            }
            break;     
        case ID_C_STEP:                     //C单步 0x25

            result = ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;             
            flag_send(debug_event, FLAGS_DBG_C_STEP);
            start_addr = (dbg_data.wrbuf[10] << 0x08) + dbg_data.wrbuf[11]; //get  start address
            end_addr = (dbg_data.wrbuf[14] << 0x08) + dbg_data.wrbuf[15];   //get  end address 
            break;
        case ID_C_STEP_OVER:                //C步越 0x26

            result = ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;              
            flag_send(debug_event, FLAGS_DBG_C_STEP_OVER);
            start_addr = (dbg_data.wrbuf[10] << 0x08) + dbg_data.wrbuf[11]; //get  start address
            end_addr = (dbg_data.wrbuf[14] << 0x08) + dbg_data.wrbuf[15];   //get  end address 
            break;
        case ID_STEP_OUT:                   //步出 0x27
            result = ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;            
            flag_send(debug_event, FLAGS_DBG_STEP_OUT);
            break;              
        case ID_INSERT_BKP:                 //0x28  设置PC断点
            result = dbg_set_breakpoint(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);            
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;    
            break;
        case ID_RM_BKP:                     //0x29 清除PC断点
            result = dbg_clr_breakpoint(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);    
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;    
            break;
        
        case ID_DISABLE_ALL_BKPS:           //0x2A清除所有PC断点
            result = dbg_clr_all_breakpointer ();
            result = ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;    
            break;  
        case Set_bk_condition:              //0x2B  设置条件
            result = dbg_set_condition_breakpoint(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);    
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;   
            break;
        case ID_RD_RAM:                     //0x32 读寄存器段
            result = dbg_read_ram(&dbg_data.wrbuf[FRAME_DATA_OFFSET], &dbg_data.rdbuf[FRAME_DATA_OFFSET] );
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN + (dbg_data.wrbuf[FRAME_DATA_OFFSET] << 0x08) | dbg_data.wrbuf[FRAME_DATA_OFFSET + 1];
            break;
        case ID_RD_PC:                      //读PC值 0x35
            break;
        case ID_RD_USED_STACK:             //0x36 读已用堆栈内容

            result = dbg_read_used_stack(&dbg_data.rdbuf[FRAME_DATA_OFFSET]);          	// read stack content		// 
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN + dbg_data.rdbuf[FRAME_DATA_OFFSET] * 4;  
            break;
        case ID_RD_ALL_STACK:               //0x37 读全部堆栈内容
            result = dbg_read_all_stack(&dbg_data.rdbuf[FRAME_DATA_OFFSET]);
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN + STACK_NUM * 4;  
            break;
 	
		case ID_RETRY:			            //0x3A 要求重发数据
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
            result = ERROR_SUCCESS;
            break;

        case ID_WR_RAM  :                //0x40 写寄存器数据
       		  //填充内存块
            result = dbg_modify_ram(&dbg_data.wrbuf[FRAME_DATA_OFFSET], &dbg_data.rdbuf[FRAME_DATA_OFFSET] );// 
       		dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
        	break;

        case ID_MODIFY_PC :         //0x41 修改PC值
            result = dbg_modify_PC(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);			// 
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
            break;

        case ID_IMPORT_RAM_DATA :       //0x47 导入寄存器数据
		    result = dbg_import_ram_data(&dbg_data.wrbuf[FRAME_DATA_OFFSET], &dbg_data.rdbuf[FRAME_DATA_OFFSET] );	
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
		    break;
        case ID_UPDATE_CHIP_STATUS : //0x70  上传芯片运行停止信息 收到主控的应答
            if(debug_flag.monitor == SET)
            {
                result = update_chip_status(&dbg_data.rdbuf[FRAME_DATA_OFFSET]);
                dbg_data.data_length = 12 + FRAME_ACK_NORMAL_LEN;             
                debug_flag.monitor = RESET ;          
            }
            else
            {
                dbg_data.data_length = FRAME_ACK_NORMAL_LEN;  
                result = NO_DATA_UPDATE;       //没有需要上报的数据                 
            }
            break;

        default:
            result = ERROR_FUN_CODE ;    
            break;
        
    }
    __ACK:   
    
    dbg_data.rdbuf[0] = 0xAB;
    dbg_data.rdbuf[1] = 0xCD;
    dbg_data.rdbuf[2] = 0xDC;
    dbg_data.rdbuf[3] = 0xBA;  
    dbg_data.rdbuf[4] = dbg_data.data_length >> 8;
    dbg_data.rdbuf[5] = dbg_data.data_length & 0xFF;
    dbg_data.rdbuf[6] = dbg_data.device_addr;
    dbg_data.rdbuf[7] = dbg_data.fun_code;
    
    if(result != ERROR_SUCCESS) 
    {
        if( result != NO_DATA_UPDATE) 
            LED_RED_ON();
        dbg_data.device_state = result;   
        dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
    }
    else
    {
        LED_GREEN_ON();
        dbg_data.device_state = ERROR_ESLINK_RUN_OK;
    }
    
    dbg_data.rdbuf[dbg_data.data_length - 3] = dbg_data.device_state;
    dbg_data.checksum = check_sum(dbg_data.data_length-2,dbg_data.rdbuf); 
    dbg_data.rdbuf[dbg_data.data_length - 2] = (uint8_t)(dbg_data.checksum >> 8);
    dbg_data.rdbuf[dbg_data.data_length - 1] = (uint8_t) dbg_data.checksum; 
    
    if(debug_event != 0)
        main_icd_debug_handle();
    return dbg_data.data_length;
}



// ICD task ；部分命令不放在中断中处理。（处理事件可能会较长，所以不放在中断中处理）
void debug_process_handle(void)
{
    error_t ret;
    if( flag_recv(debug_event, FLAGS_DBG_RUN)  )                //0x21  全速运行 IDE上位启动
    {
        flag_clr(debug_event, FLAGS_DBG_RUN);
        ret = dbg_run();
    } 
    else if( flag_recv(debug_event, FLAGS_DBG_ASM_STEP_OVER)  )     //0x23  汇编步越
    {
        flag_clr(debug_event, FLAGS_DBG_ASM_STEP_OVER);
        ret = dbg_asm_step_over();    
    }
    else if( flag_recv(debug_event, FLAGS_DBG_STEP_OUT)  )      //步出 0x27
    {
         flag_clr(debug_event, FLAGS_DBG_STEP_OUT);
        ret = dbg_asm_step_out();	                            
    }
    else if( flag_recv(debug_event, FLAGS_DBG_C_STEP)  )        //C单步 0x25
    {      
         flag_clr(debug_event, FLAGS_DBG_C_STEP);
        ret = dbg_C_step();	
    }
    else if(flag_recv(debug_event, FLAGS_DBG_C_STEP_OVER))      //步越
    {
         flag_clr(debug_event, FLAGS_DBG_C_STEP_OVER);
        ret = dbg_C_stepover();	
    }
//    else if flag_recv(debug_event, FLAGS_HALT_CHECK)
    {
//        if(target_state == ICD_RUN)
        {
            if(icd_halt_check() == TRUE)
            {
                 debug_flag.monitor = SET;
                 target_state = ICD_HALT;
            }
                
        }            
    }     

    if(debug_main.set_point == SET)
    {
//        icd_write_str(unset_breakpoint_1640,debug_main.addr); //c程序中main的入口地址20111206);
//        icd_write_str(set_breakpoint_1640,debug_main.pointer_back);
        debug_main.set_point = RESET;
    }  
}


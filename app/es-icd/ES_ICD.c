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

extern icd_breakpoint_t icd_breakpoint;
ICD_COMM_DATA_t icd_data;
ICD_TARGET_STATE icd_target_state;
ICD_DEBUG_FLAG_t icd_debug_flag;
ICD_DEBUG_MAIN_t icd_main;

uint16_t config_data[3];
void icd_error_ack(uint8_t error)
{
    
    
}


//与编程器相关的操作
uint8_t download_config(uint8_t *wr_buf)
{
    
    //buf[0] = AA;
    //buf[1] = 55;  配置字起始
    config_data[0] = wr_buf[2 + ICD_CODE_OFFSET] << 8 | wr_buf[3 + ICD_CODE_OFFSET];      //配置字
    
    config_data[1] = wr_buf[32 + ICD_CODE_OFFSET] << 8 | wr_buf[33 + ICD_CODE_OFFSET];    //userID1
    config_data[2] = wr_buf[34 + ICD_CODE_OFFSET] << 8 | wr_buf[35 + ICD_CODE_OFFSET];    //userID2    
    
    return ICD_ERR_NONE;
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
    return ICD_ERR_NONE; 
    
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
    
    return ICD_ERR_NONE; 
    
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
    icd_error_t status = ICD_ERR_NONE;
    
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
    
    return ICD_ERR_NONE;   
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
//    icd_error_t status = ICD_ERR_NONE;
//    
//    addr -= 0x2800;     //历史遗留
//    memcpy(data_buf[0].B08, buf, ICD_CHIP_INFO_LEN);
 //   eslink_rom_write(CHIP_INFO_Fth+addr, size, &data_buf[0].B32);
    
    return ICD_ERR_NONE;       
    
}
//---------------------------------------------
//		设置main入口地址
//---------------------------------------------
uint8_t icd_set_main_addr(uint8_t *wr_buf)	
{
	icd_main.addr =((uint16_t)(wr_buf[ICD_DATA_OFFSET + 2] & 0xff) << 8) + wr_buf[ICD_DATA_OFFSET+3];  
    return ICD_ERR_NONE;	    
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
        
            error = ICD_ERR_NONE;
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
            error = ICD_ERR_NONE;
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
            error = emu_reset();

            if(error == ICD_ERR_NONE)                   
                error = update_chip_status(icd_data.rdbuf, &icd_data.data_length);                 
                             
            
            break;
        case ID_EMU_RUN:            //0x21  全速运行
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;    
            error = ICD_ERR_NONE;
            os_evt_set(FLAGS_ICD_RUN, icd_task_id);
			break;
        case ID_EMU_ASM_STEP:           //0x22 汇编单步
            error = emu_asm_step();
            if(error == ICD_ERR_NONE)            
                error = update_chip_status(icd_data.rdbuf, &icd_data.data_length);                               
                 
            break;
        case ID_EMU_ASM_STEP_OVER:  //0x23  汇编步越
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;  
            error = ICD_ERR_NONE;
            os_evt_set(FLAGS_ICD_ASM_STEP_OVER, icd_task_id);
            break;
        
        case EMU_halt:          //停止 0x24
            
            error = emu_stop();   
            if(error == ICD_ERR_NONE)       
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
            error = ICD_ERR_NONE;
            os_evt_set(FLAGS_ICD_C_STEP, icd_task_id);
            break;
        case C_stepover:                //C步越 0x26
            for(i=0; i<ICD_DATA_LEN; i++)
            {
                icd_data.rdbuf[i+ICD_DATA_OFFSET] = icd_data.wrbuf[i+ICD_DATA_OFFSET];        
            }
            icd_data.data_length = ICD_ACK_NORMAL_LENTH; 
            error = ICD_ERR_NONE;
            os_evt_set(FLAGS_ICD_C_STEP_OVER, icd_task_id);
            break;
        case EMU_stepout:               //步出 0x27
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;  
            error = ICD_ERR_NONE;
            os_evt_set(FLAGS_ICD_STEP_OUT, icd_task_id);
            break;
        
        case SetintPointer:             //0x28  设置PC断点
            error = emu_set_breakpoint(icd_data.wrbuf);            
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;    
            break;
        case ClrintPointer:            //0x29 清除PC断点
            error = emu_clr_breakpoint(icd_data.wrbuf);	
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;    
            break;
        
        case Delallbreakpointer:       //0x2A清除所有PC断点
            error = emu_clr_all_breakpointer ();
            error = ICD_ERR_NONE;
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;    
            break;  
        case Set_bk_condition:          //0x2B  设置条件
            error = emu_set_condition_breakpoint(icd_data.wrbuf);
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;   
            break;
        case ReadRam:                   //0x32 读寄存器段
            error = emu_read_ram(icd_data.wrbuf, icd_data.rdbuf, &icd_data.data_length);
            break;
        case ReadPc:                //读PC值 0x35
            break;
        case ReadUsedStack:             //0x36 读已用堆栈内容

            error = emu_read_used_stack(icd_data.rdbuf, &icd_data.data_length);          	// read stack content		// 
            break;
        case ID_ICD_DisplayAllStack:          //0x37 读全部堆栈内容
            error = emu_read_all_stack(icd_data.rdbuf, &icd_data.data_length);
            break;
 	
		case Resend_ack:			    //0x3A 要求重发数据
//			  Ack_ReSend_Ack();
            error = ICD_ERR_NONE;
            break;

        case Writeram :                //0x40 写寄存器数据
       		  //填充内存块
            error = emu_modify_ram(icd_data.wrbuf, icd_data.rdbuf, &icd_data.data_length);   		// 
       		
        	break;

        case ModifycunrrentPc:         //0x41 修改PC值
            error = emu_modify_PC(&icd_data.wrbuf[ICD_DATA_OFFSET]);			// 
            icd_data.data_length = ICD_ACK_NORMAL_LENTH;  
            break;

        case Import_Various_RAM:       //0x47 导入寄存器数据
		    error = emu_import_ram(icd_data.wrbuf, icd_data.rdbuf, &icd_data.data_length);  	
            
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

    if(error != ICD_ERR_NONE)
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
            error = emu_run();	
            break;
        case ID_EMU_ASM_STEP_OVER:      //0x23  汇编步越
            error = emu_asm_step_over();
            break;
        case EMU_stepout:
            error = emu_asm_step_out();	//步出 0x27
            break;
//        case C_step:                    //C单步 0x25
//            error = emu_C_step(&icd_data.wrbuf[ICD_DATA_OFFSET]);		
//            break;
//        case C_stepover:
//            error = emu_C_stepover(&icd_data.wrbuf[ICD_DATA_OFFSET]);	
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
            error = emu_run();
        
        if (flags & FLAGS_ICD_ASM_STEP_OVER)    //0x23  汇编步越
            error = emu_asm_step_over();  
        
        if (flags & FLAGS_ICD_STEP_OUT)
            error = emu_asm_step_out();	        //步出 0x27
        
        if (flags & FLAGS_ICD_C_STEP)
            error = emu_C_step(icd_data.wrbuf);		 //C单步 0x25
        
        if (flags & FLAGS_ICD_C_STEP_OVER)
            error = emu_C_stepover(icd_data.wrbuf);	
        
        if (flags & FLAGS_HALT_CHECK) 
        {
            if(icd_target_state == ICD_RUN)
            {
                if(icd_halt_check() == ICD_ERR_NONE)
                    icd_debug_flag.monitor = SET;
            }            
            
        }     

        if(icd_main.set_point == SET)
        {
            icd_write_data_operation(unset_breakpoint_1640,icd_main.addr); //c程序中main的入口地址20111206);
            icd_write_data_operation(set_breakpoint_1640,icd_main.pointer_back);
            icd_main.set_point = RESET;
        }
        os_evt_set(FLAGS_HALT_CHECK, icd_task_id);
        os_dly_wait(10);       
        
    }    
}



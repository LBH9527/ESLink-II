
////void spi_flash_demo(void)
////{
////     uint64_t ullSdCapacity;
////    
////    if(finit("S0:") != NULL)
////    {
////        debug_msg("error \r\n");
////        return;
////    }
////    else
////    {
////        debug_msg("success\r\n");
////    }                    
////      if (fformat ("S0:") != 0) 
////    {                                
////        debug_msg (" geshihua fail\r\n");
////    }
////    else  
////    {
////        debug_msg (" geshihua success\r\n");
////    }


////    ullSdCapacity = ffree("S0:");

////    debug_msg(" rongliang = %x\r\n", ullSdCapacity);
////    
////    if(funinit("S0:") != NULL)
////    {
////        printf("xiezai fail\r\n");
////    }
////	else
////	{
////		printf("xiezai success\r\n");
////	}
////}

//__task void AppTaskLED(void)
//{
//	const uint16_t usFrequency = 100; /* 延迟周期 */
//    uint8_t temp[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
//    uint32_t read_id[12];
//    
//    PORT_ISP_SETUP();
//        CLT_INIT(0);
////    PIN_CTL_GPIO->PDDR &= ~PIN_CTL_BIT; 
//    
////    PIN_CTL_PORT->PCR[PIN_CTL_BIT]   = PORT_PCR_MUX(1)  |  /* GPIO */
////            PORT_PCR_PE_MASK |  /* Pull enable */
////            PORT_PCR_PS_MASK;   /* Pull-up */
////    PIN_CTL_GPIO->PDDR &= ~(1 << PIN_CTL_BIT);             /* Input */
//	/* 设置延迟周期 */
////	os_itv_set(usFrequency);
////	 PIN_SWDIO_TMS_CLR();
////   PIN_SWDIO_TMS_SET();
////   PIN_ISPSDA_OUT_DISABLE();
////    unclock_chip();
//    while(1)
//    {
////        isp_read_chipid(target_device.chipid_addr,  read_id);
////        empty_check();
////        sf_read_info();
//        LED_RED_TOGGLE();
//        CTL_SET();
////        LED_GREEN_TOGGLE();`
////        LED_YELLOW_TOGGLE();
//        PIN_SWCLK_TCK_SET();
//        PIN_SWDIO_TMS_SET();
////         PIN_SWCLK_GPIO->PCOR = 1 << PIN_SWCLK_BIT;
////		/* os_itv_wait是周期性延迟，os_dly_wait是相对延迟。*/
//		os_dly_wait(100);
////        es_delay_ms(1000);
////        es_delay_us(500000);
//        CTL_CLR();
//        PIN_SWCLK_TCK_CLR();
//        PIN_SWDIO_TMS_CLR();
////        PIN_SWCLK_GPIO->PSOR = 1 << PIN_SWCLK_BIT;
//        LED_RED_TOGGLE();
////        es_delay_us(1000000);
//        os_dly_wait(100);
////         uart_write_data(temp, 5);

//    }
//}


///*
//*********************************************************************************************************
//*	函 数 名: AppTaskCreate
//*	功能说明: 创建应用任务
//*	形    参: 无
//*	返 回 值: 无
//*********************************************************************************************************
//*/
//static void AppTaskCreate (void)
//{
////	HandleTaskUserIF = os_tsk_create_user(AppTaskUserIF,             /* 任务函数 */ 
////	                                      1,                         /* 任务优先级 */ 
////	                                      &AppTaskUserIFStk,         /* 任务栈 */
////	                                      sizeof(AppTaskUserIFStk)); /* 任务栈大小，单位字节数 */
//	
//	 os_tsk_create_user(AppTaskLED,              /* 任务函数 */ 
//	                                   2,                       /* 任务优先级 */ 
//	                                   &AppTaskLEDStk,          /* 任务栈 */
//	                                   sizeof(AppTaskLEDStk));  /* 任务栈大小，单位字节数 */
//	
////	HandleTaskMsgPro = os_tsk_create_user(AppTaskMsgPro,             /* 任务函数 */ 
////	                                      3,                         /* 任务优先级 */ 
////	                                      &AppTaskMsgProStk,         /* 任务栈 */
////	                                      sizeof(AppTaskMsgProStk)); /* 任务栈大小，单位字节数 */
//}



//OFL_DATA_t ofl_data; 

//uint32_t offline_process_command(uint8_t *request, uint8_t *response)
//{
//    uint32_t sum;
//    uint8_t result = ERROR_SUCCESS;
//    uint32_t ack_len = 512; //isp通信采用变长协议，用来返回回复数据的长度
//    
//    ofl_data.wrbuf = request;
//    ofl_data.rdbuf = response;
//    
////    if( ((*(request+0) << 0) | (*(request+1) << 8) | (*(request+2) << 16) | (*(request+3) << 24)) != ISP_FRAME_HEAD){
////        result = FRAMEHEADERROR;
////        goto __ACK;
////    }
//    ofl_data.frame_head = 0xABCDDCBA;
//  
////    if(*(request+4) != 0x51){
////        result = DEVICETYPEERROR;
////        goto __ACK;
////    }

//    ofl_data.device_type = *(request+4);
//    ofl_data.fun_code = *(request+5);
//    ofl_data.data_length = (*(request+6) << 0) | (*(request+7) << 8);
//    
//    memset(ofl_data.rdbuf, 0, 0x400);   
//    
//    switch(ofl_data.fun_code)
//    {       
//        case ID_DL_OFFLINE_PRJ_INFO : 
//            ofl_pcb.part.type =  INFIO_PART;  
//            ofl_pcb.part.start = 0;
//            ofl_pcb.part.size =  ofl_data.data_length;
//            ofl_prj_write_part(&prj_part);
//            ofl_prj_write_info(&ofl_data.wrbuf[ES_DATA_OFFSET], ofl_data.data_length);   
//            result = ERROR_SUCCESS;
//            break;
//        case ID_READ_OFFLINE_PRJ_NUM :          //读方案数量
//            ofl_data.data_length = 0x04;
//            ofl_data.rdbuf[ES_DATA_OFFSET] = ofl_prj_get_num() ;
//            ofl_data.rdbuf[ES_DATA_OFFSET+1] = 0;
//            ofl_data.rdbuf[ES_DATA_OFFSET+2] = 0;
//            ofl_data.rdbuf[ES_DATA_OFFSET+3] = 0;
//            result = ERROR_SUCCESS;
//            break;
//        case ID_READ_OFFLINE_PRJ_INFO :         //读方案信息
//            ofl_data.data_length = 0x04; 
//           
//            result = ERROR_SUCCESS;
//            break;
//        case ID_DL_OFFLINE_PRJ_TIMING_START :   //脱机方案时序下载开始
//            ofl_pcb.part.type =  TIMING_INFO_PART;  
//            ofl_pcb.part.size =  ofl_data.data_length;        
//            ofl_prj_write_part(&prj_part);
//        
//            ofl_prj_write_data(&ofl_data.wrbuf[ES_DATA_OFFSET], ofl_data.data_length); 
//            ofl_data.data_length = 0;        
//            result = ERROR_SUCCESS;
//            break;
//        case ID_DL_OFFLINE_PRJ_TIMING_ING :     //脱机方案时序下载              
//           
//            ofl_prj_write_data(&ofl_data.wrbuf[ES_DATA_OFFSET], ofl_data.data_length);
//        
//            ofl_pcb.part.data += check_sum( ofl_data.data_length-8 , &ofl_data.wrbuf[16] ) ;  
//            ofl_pcb.part.size +=  ofl_data.data_length;    

//            ofl_data.data_length = 0;        
//            result = ERROR_SUCCESS;
//            break;
//        case ID_DL_OFFLINE_PRJ_TIMING_END :     //脱机方案时序结束
//            ofl_pcb.part.type =  TIMING_PART;             
//            ofl_prj_write_part(&prj_part);

//            ofl_data.rdbuf[8] = (ofl_pcb.part.data & 0xff000000) >> 24;
//            ofl_data.rdbuf[9] = (ofl_pcb.part.data & 0x00ff0000) >> 16;
//            ofl_data.rdbuf[10] = (ofl_pcb.part.data & 0x0000ff00) >> 8;
//            ofl_data.rdbuf[11] = (ofl_pcb.part.data & 0x000000ff);
//            ofl_data.data_length = 4;        
//            result = ERROR_SUCCESS;
//            break;
//        case ID_DL_OFFLINE_PRJ_CONFIG :         //脱机方案配置字下载
//            ofl_pcb.part.type =  CONFIG_PART;  
//            ofl_pcb.part.size =  ofl_data.data_length;
//            ofl_prj_write_part(&prj_part);
//        
//            ofl_prj_write_data(&ofl_data.wrbuf[ES_DATA_OFFSET], ofl_data.data_length); 
//            ofl_data.data_length = 0;        
//            result = ERROR_SUCCESS;
//            break;
//        case ID_DL_OFFLINE_PRJ_HEX :            //脱机方案用户HEX
//            ofl_prj_write_data(&ofl_data.wrbuf[ES_DATA_OFFSET], ofl_data.data_length);
//        
//            ofl_pcb.part.data += check_sum( ofl_data.data_length-8 , &ofl_data.wrbuf[16] ) ;  
//            ofl_pcb.part.size +=  ofl_data.data_length;  
//        
//            ofl_data.data_length = 0x00;
//            result = ERROR_SUCCESS;
//            break;
//        case ID_DL_OFFLINE_PRJ_HEX_END :        //脱机方案用户HEX下载完成
//            sum = (ofl_data.wrbuf[ES_DATA_OFFSET]      << 24) |
//                    (ofl_data.wrbuf[ES_DATA_OFFSET+1] << 16) |
//                    (ofl_data.wrbuf[ES_DATA_OFFSET+2] << 8 ) |
//                    (ofl_data.wrbuf[ES_DATA_OFFSET+3]  << 0);
//            if(ofl_pcb.part.data !=  sum)
//            {
//                ofl_data.data_length = 0x04;
//                result = ERROR_HEX_CHECKSUM_FAIL;
//            }
//            else
//            {
//                ofl_pcb.part.type =  HEX_PART;             
//                ofl_prj_write_part(&prj_part);
//       
//                ofl_data.data_length = 0x00;
//                result = ERROR_SUCCESS;
//            }
//            
//            break;
//        
//        
//        
//    }
//    
//}

//void isp_offline_program()
//{
////    sfud_err result = SFUD_SUCCESS;
//    uint8_t result;

//    
//    uint8_t n;
//    uint32_t *items_idx;
//    
//  /*    for(n=0; n<program_info.offline_step; n++)
//    {
//        items_idx = program_info.offline_items;
//        switch(*items_idx)
//        {
//            case FULL_ERASE:
//                result = isp_erase_chip (0) ;
//                break;
//            case CHECK_EMPTY:
//                result = isp_check_empty();
//                break;
//            case HEX_PRG:
////                result = isp_program_hex();

//                break;
//            case ID_VERIFY:
//                break;
//            case ID_ENCRYPT:
//                break;
//            
//            
//        }
//        
//    }    
//    */
//}


//static void make_adress_byte_array(const sfud_flash *flash, uint32_t addr, uint8_t *array) {
//    uint8_t len, i;

//    SFUD_ASSERT(flash);
//    SFUD_ASSERT(array);

//    len = flash->addr_in_4_byte ? 4 : 3;

//    for (i = 0; i < len; i++) {
//        array[i] = (addr >> ((len - (i + 1)) * 8)) & 0xFF;
//    }
//}

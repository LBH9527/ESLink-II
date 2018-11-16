#include "ES_ISP.h"
#include "isp_m939.h"


 
const uint8_t UnclockCode[4] = {0x55,0x45,0x53,0xAA};

#define PIN_DELAY(n)    ES_DELAY_SLOW(n)
//PIN_DELAY(1)----> 200ns

/*
 *  Send ISP Start Bit
 *    Parameters:      None
 *    Return Value:    None
 */
static void isp_start_bit(void)
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

/*
 *  Send ISP End Bit
 *    Parameters:      None
 *    Return Value:    None
 */
static void isp_end_bit(void)
{
    PIN_ISPCLK_CLR();                   
    PIN_DELAY(1);                       
    PIN_ISPSDA_CLR();                   
    PIN_DELAY(1);                       
    PIN_ISPCLK_SET();                   
    PIN_DELAY(1);                       
    PIN_ISPSDA_SET();      
}
/*
 *  Send Data Which Bits are Define by Parameter Number 
 *    Parameters:      data: Data Need to Send
 *                     number:Data Bits Need to Send
 *    Return Value:    None
 */
static void isp_send(uint32_t data,uint32_t number)
{    
    uint32_t n;
    
	for (n=0; n<number; n++)
    {
		PIN_ISPCLK_CLR();
//		PIN_DELAY(1);
		if (data & (1<<(number-1))){     
			PIN_ISPSDA_SET();
		}
		else{
			PIN_ISPSDA_CLR();
		}       
		data<<=1;
		PIN_ISPCLK_SET();
		PIN_DELAY(1);
	}
	PIN_ISPCLK_CLR();
	PIN_DELAY(1);
	PIN_ISPSDA_CLR();       
}

/*
 *  Receive One Byte by ISP
 *    Parameters:      None
 *    Return Value:    Date Read by ISP 
 */
static uint8_t isp_recv_byte(void)
{
    uint8_t n,data = 0;
    
    PIN_ISPSDA_OUT_DISABLE();
    for (n=0; n<8; n++){
        PIN_ISPCLK_CLR();
        PIN_DELAY(1);
        data <<= 1;
        PIN_ISPCLK_SET();
        if(PIN_ISPSDA_IN())
            data |= 0x01;
        __nop();
        __nop();
    }
    PIN_ISPCLK_CLR();
    PIN_DELAY(1);
    PIN_ISPSDA_OUT_ENABLE();
    PIN_ISPSDA_CLR();
    
    return data;
}

/*
 *  Receive halfword by ISP
 *    Parameters:      None
 *    Return Value:    Date Read by ISP 
 */
static uint32_t isp_recv_halfword(void)
{
    uint8_t n;
    uint32_t data = 0;
    
    PIN_ISPSDA_OUT_DISABLE();
    for (n=0; n<16; n++){
        PIN_ISPCLK_CLR();
        PIN_DELAY(1);
        data<<=1;
        PIN_ISPCLK_SET();
        if(PIN_ISPSDA_IN())
            data |= 0x01;
    }
    PIN_ISPCLK_CLR();
    PIN_DELAY(1);
    PIN_ISPSDA_OUT_ENABLE();
    PIN_ISPSDA_CLR();
    
    return data;    
}
    
/*
 *  Receive three Byte by ISP
 *    Parameters:      None
 *    Return Value:    Date Read by ISP 
 */
static uint32_t isp_recv_3rdbyte(void)
{
    uint8_t n;
    uint32_t data = 0;
    
    PIN_ISPSDA_OUT_DISABLE();
    for (n=0; n<24; n++){
        PIN_ISPCLK_CLR();
        PIN_DELAY(1);
        data<<=1;
        PIN_ISPCLK_SET();
        if(PIN_ISPSDA_IN())
            data |= 0x01;
    }
    PIN_ISPCLK_CLR();
    PIN_DELAY(1);
    PIN_ISPSDA_OUT_ENABLE();
    PIN_ISPSDA_CLR();
    
    return data;    
}
/*
 *  Receive word by ISP
 *    Parameters:      None
 *    Return Value:    Date Read by ISP 
 */
static uint32_t isp_recv_word(void)
{
    uint8_t n;
    uint32_t data = 0;
    
    PIN_ISPSDA_OUT_DISABLE();
    for (n=0; n<32; n++){
        PIN_ISPCLK_CLR();
        PIN_DELAY(1);
        data<<=1;
        PIN_ISPCLK_SET();
        PIN_DELAY(1);
        if(PIN_ISPSDA_IN())
            data |= 0x01;
    }
    PIN_ISPCLK_CLR();
    PIN_DELAY(1);
    PIN_ISPSDA_OUT_ENABLE();
    PIN_ISPSDA_CLR();
    
    return data;    
}  


/*
 *  Unclock Chip and Get into ISP Mode
 *    Parameters:      None
 *    Return Value:    TRUE - Success, FALSE - Error
 */
uint8_t unclock_chip(void)              
{
    uint8_t i, n;
    uint32_t j,temp;
    
    for(i=0;i<3;i++)
    {                                               //Try 3 Times to Unclock Chip
//		GPIO_SetBits(GPIOC,SEL33V_PIN);
////		GPIO_ResetBits(GPIOC,SEL5V_PIN);	
//		GPIO_SetBits(GPIOA,MRST_PIN);
        V33_OFF();

        PIN_RST_OUT(0);
		PIN_ISPCLK_CLR();
		PIN_ISPSDA_CLR();	
		es_delay_ms(10);		
//		switch(CurrentPower){
//			case 0x00:
//				GPIO_ResetBits(GPIOC,SEL33V_PIN);
//				break;
//			case 0x01:
//				GPIO_SetBits(GPIOC,SEL5V_PIN);
//				break;
//			default :
//				break;
//		}
        V33_ON();
        PIN_ISPSDA_SET();
		PIN_ISPCLK_SET();
//		GPIO_SetBits(GPIOA,MRST_PIN);
        PIN_RST_OUT(0);
		es_delay_ms(10);

        //---------------------------------------------------------
        
//		ISPstartBit();
//		es_delay_ms(20);		
		
		for(j=0;j<300;j++)
        {			
			isp_start_bit();
			isp_send(ID_CHECK_CD, 32);		
			temp = isp_recv_word();
			isp_end_bit();	
			if(temp == ID_CODE_VAL)
            {                                       //ID Check
					break;
			}
		}
        
		if(temp!=ID_CODE_VAL){
			return FALSE;
		}	

        //---------------------------------------------------------		
		
		isp_start_bit();
		for(n=0;n<4;n++)
        {
			isp_send(UnclockCode[n],8);              //Unclock Chip
		}
		isp_end_bit();	
		
		isp_start_bit();
		isp_send(STATUS_CHECK_CD,8);
		temp = isp_recv_byte();
		isp_end_bit();	
        if((temp&0xF0) == UNLOCK_CHECK_VAL)           //Unclock Check
        {
			isp_start_bit();
			isp_send(ISP_MODE_CD,8);                 //Get into ISP Mode
			isp_end_bit();
			
            isp_start_bit();
			isp_send(STATUS_CHECK_CD,8);
			temp = isp_recv_byte();
			isp_end_bit();
            if(temp == MODE_CHECK_VAL)                //Mode Check
            {	
				//---------------------------------------------------------	
                
				isp_start_bit();
				isp_send(ENCRYPT_LOAD_CD,8);         //Load Encryption Words
				temp = isp_recv_3rdbyte();	
				isp_end_bit();
				if(temp == ENCRYPTION_VAL){                  
                    
					return TRUE;
				}
			}
		}
        
        //---------------------------------------------------------		
        
		/*Reset Pin Pull Down with Pulling Up First Again*/
//		GPIO_ResetBits(GPIOA,MRST_PIN); 
        PIN_RST_OUT(1);
		es_delay_ms(10);
//		GPIO_SetBits(GPIOA,MRST_PIN);
        PIN_RST_OUT(0);
		es_delay_ms(10);
	}

	return FALSE;		
}

/*
 *  Select Programe Area
 *    Parameters:      data:Area Select Value
 *    Return Value:    None
 */
void prog_area_set(uint8_t data)            
{
	isp_start_bit();
	isp_send(AREA_SET_CD, 8);
	isp_send(data,8);	
    isp_end_bit();
}

/*
 *    Set Programme Address
 *    Parameters:      address: Programme Address
 *    Return Value:    None
 */
void prog_addr_set(uint32_t address)    //prog_addr_set
{
	isp_start_bit();
	isp_send(SET_ADDR_CD, 8);
	isp_send(address,32);                    
    isp_end_bit();
}

uint32_t prog_addr_read(void)      //prog_addr_read
{
	uint32_t address;
    
	isp_start_bit();
	isp_send(READ_ADDR_CD, 8);
	address = isp_recv_word();                   
    isp_end_bit();
    
	return address;
}

/*
 *  Write Data to Buff
 *    Parameters:      data: Data to Be Write
 *    Return Value:    None
 */
void code_buffer_write(uint32_t data0, uint32_t data1)   
{   
    isp_start_bit();
	isp_send(WRITE_BUFF0_CD,8);
	isp_send(data0,32);
    isp_end_bit();
	
	isp_start_bit();
	isp_send(WRITE_BUFF1_CD,8);
	isp_send(data1,32);
    isp_end_bit();
}

/*
 *  Write Data to Info Buff
 *    Parameters:      data: Data to Be Write
 *    Return Value:    None
 */
void info_buffer_write(uint32_t data)
{
	isp_start_bit();
	isp_send(WRITE_INFO_BUFF_CD,8);
	isp_send(data,16);                    
    isp_end_bit();
}

/*
 *  Programme Operation
 *    Parameters:      data: Programme Command
 *    Return Value:    TRUE - Success, FALSE - Error
 */
uint8_t prog_operation(uint32_t data)
{
	uint32_t i,n,temp;
	isp_start_bit();
	isp_send(data,8);
	isp_send(0x50,8);                    
    isp_end_bit();
    
	for(i=0;i<3;i++){
		es_delay_us(25);
		for(n=0;n<10;n++){
			isp_start_bit();
			isp_send(PROG_CHECK_CD,8);
			temp=isp_recv_byte();                    
			isp_end_bit();
			if(temp==PROG_CHECK_VAL){
				return TRUE;
			}
			es_delay_us(5);
		}
//		isp_start_bit();
//		isp_send(WORDPRGCD,8);
//		isp_send(0x50,8);                    
//		isp_end_bit();
	}
	return FALSE;
}

/*
 *  Programme Config Word by One
 *    Parameters:      address: Programme Address
 *                     data:Programme Data
 *    Return Value:    TRUE(1) - Success, FALSE(0) - Fault
 */
uint8_t prog_config_word(uint32_t address,uint32_t data)
{
	uint32_t temp;
    
	prog_addr_set(address);
    isp_process_err.addr = prog_addr_read(); 
	info_buffer_write(data);    
	temp = prog_operation(WORD_PRG_CD);
    
	return temp;
}


uint8_t prog_config_packet(uint32_t data)
{
	uint8_t temp;
    
	info_buffer_write(data);
	temp = prog_operation(WORD_ADD_PLUS_PRG);
	return temp;
}


//uint8_t PrgUserConfigWord(uint32_t address,uint32_t data){
//	uint32_t temp;
//    
//	prog_addr_set(address);
//	WriteInfoAllBuff(data);    
//	temp=prog_operation(WORD_PRG_CD);
//	return temp;
//}

//uint8_t PrgUserConfigWordOneByOne(uint32_t data){
//	uint8_t temp;
//	WriteInfoAllBuff(data);
//	temp=prog_operation(WORD_ADD_PLUS_PRG);
//	return temp;
//}


uint8_t prog_flash_word(uint32_t address,uint32_t data0, uint32_t data1)
{
	uint8_t temp;
    
	prog_addr_set(address);
	code_buffer_write(data0, data1);
	temp = prog_operation(WORD_PRG_CD);
    
	return temp;
}

uint8_t prog_flash_packet(uint32_t data0, uint32_t data1)
{
	uint8_t temp;
    
	code_buffer_write(data0, data1);
	temp=prog_operation(WORD_ADD_PLUS_PRG);
    
	return temp;
}

/*
 *  Read Config Word by One
 *    Parameters:      address: Config Word Address at Which Data Will be Read
 *    Return Value:    Config Word Value
 */
uint32_t config_word_read(uint32_t address)
{
	uint32_t data;
    
	prog_addr_set(address);
	isp_start_bit();
	isp_send(READ_FLASH_CD,8);
	data = isp_recv_word();                    
	isp_end_bit();	
    
	return data;
}

/*
 *  Read Config Word in a Row
 *    Parameters:      None
 *    Return Value:    Config Word Value
 */
uint32_t config_read_block(void)
{
	uint32_t data;
    
	isp_start_bit();
	isp_send(READ_FLASH_PLUS_CD,8);
	data = isp_recv_word();                    
	isp_end_bit();	
    
	return data;
}

uint32_t config_read_word(void)
{
	uint32_t data;
    
	isp_start_bit();
	isp_send(READ_FLASH1_CD,8);
	data = isp_recv_word();                    
	isp_end_bit();	
    
	return data;
}

/*
 *  Set CRC Control
 *    Parameters:      mode:control mode
 *    Return Value:    TRUE(1) - Success, FALSE(0) - Fault
 */
uint8_t crc_enable()
{
	uint8_t i, temp;

	isp_start_bit();
	isp_send(CRC_CTRL_WR,8);
	isp_send(CRC_EN,8);
	isp_end_bit();

    for(i=0;i<3;i++)
    {
	    isp_start_bit();
	    isp_send(CRC_CTRL_RD,8);
		temp = isp_recv_byte();
		isp_end_bit();
		if(temp == CRC_EN)
        {
			return TRUE;
		}
		es_delay_ms(1);
	}
	return FALSE;
}

uint8_t crc_disable()
{
	uint8_t i, temp;
    
	isp_start_bit();
	isp_send(CRC_CTRL_WR,8);
	isp_send(CRC_DIS,8);
	isp_end_bit();
    for(i=0;i<3;i++){
	  isp_start_bit();
	  isp_send(CRC_CTRL_RD,8);
		temp = isp_recv_byte();
		isp_end_bit();
		if(temp == CRC_DIS){
			return TRUE;
		}
		es_delay_ms(1);
	}
    
	return FALSE;
}

/*
 *  Triger CRC
 *    Parameters:      mode:control mode
 *    Return Value:    None
 */
void crc_triger_set(uint8_t mode)
{
//	isp_start_bit_M();
//	isp_send_M(CRC_TRIGER,8);
//	isp_send_M(mode,8);
//	isp_end_bit_M();
}

/*
 *  Set CRC Start Address
 *    Parameters:      address: CRC Start Address
 *    Return Value:    TRUE(1) - Success, FALSE(0) - Fault
 */
uint8_t crc_addr_set(uint32_t address)
{
	uint32_t i,temp;	
    
	temp = 0x00;
	isp_start_bit();
	isp_send(SET_ADDR_CD,8);
	isp_send(address,32);
    isp_end_bit();
    
    for(i=0;i<3;i++)
    {
	    isp_start_bit();
	    isp_send(READ_ADDR_CD,8);
		temp = isp_recv_word();
		isp_end_bit();
		if(temp == address){
			return TRUE;
		}
		es_delay_ms(1);
	}
	return FALSE;	
}

/*
 *  Set CRC Size
 *    Parameters:      size: CRC Size
 *    Return Value:    TRUE(1) - Success, FALSE(0) - Fault
 */
uint8_t crc_size_set(uint32_t area, uint32_t size)
{
	uint32_t i,temp;	

	temp = 0x00;
//	if(area==CODE_AREA_SELECT)
//    {
//		size /= 2;
//	}
	isp_start_bit();
	isp_send(CRC_SIZE_WR,8);
	isp_send(size,32);
    isp_end_bit();
    
    for(i=0;i<3;i++)
    {
        isp_start_bit();
        isp_send(CRC_SIZE_RD,8);
		temp = isp_recv_word();
		isp_end_bit();
		if(temp == size){
			return TRUE;
		}
		es_delay_ms(1);
	}
    
	return FALSE;	
}

/*
 *  CRC state check
 *    Parameters:      None
 *    Return Value:    CRC State Value
 */
uint8_t crc_state_check(void)
{
	uint8_t data;
    
//	isp_start_bit_M();
//	isp_send_M(CRC_STATE_CHECK,8);
//	data = isp_recv_byte_M();                    
//	isp_end_bit_M();	
    
	return data;
}

/*
 *  Read CRC Value
 *    Parameters:      None
 *    Return Value:    CRC Result Value
 */
uint32_t crc_value_read(void)
{
	uint32_t data;
    
	isp_start_bit();
	isp_send(CRC_VALUE_READ,8);
	data = isp_recv_word();                    
	isp_end_bit();	
    
	return data;
}

/*
 *  Read CRC Current Address
 *    Parameters:      None
 *    Return Value:    CRC Current Address Value
 */
uint32_t crc_addr_read(void)
{
	uint32_t data;
    
	isp_start_bit();
	isp_send(CRC_ADDR_Read,8);
	data = isp_recv_word();                    
	isp_end_bit();	
    
	return data;
}

/*
 *  CRC32/WinRAR Initial 
 *    Parameters:      area, address, size
 *    Return Value:    TRUE(1) - Success, FALSE(0) - Fault
 */
uint8_t crc_init(uint32_t area, uint32_t address, uint32_t size)
{
	if(crc_enable() == TRUE)
    {  
		prog_area_set(area);
        
		if(crc_addr_set(address)&&crc_size_set(area, size))
        {	
			crc_triger_set(0x61);                             //Triger Reset to 1
			while(!(crc_state_check()&0x04));                 //Wait for Reset to 1 Finish
			return TRUE;
		}
	}
	return FALSE;	
}

/*
 *  uint8_t erase_check(uint32_t number)
 *    Parameters:      number:Delay 1ms Number
 *    Return Value:    TRUE(1) - Success, FALSE(0) - Fault
 */
uint8_t erase_check(uint32_t number)
{
	uint32_t i,temp;
    
	es_delay_ms(number);

    for(i=0;i<20;i++){
        isp_start_bit();
        isp_send(ERASE_CHECK_CD,8);
		temp = isp_recv_byte();
		isp_end_bit();
		if(temp == ERASE_OK_VAL){
			return TRUE;
		}
		es_delay_ms(1);
	}
	return FALSE;
}

/*
 *  Erase Chip
 *    Parameters:      erasemode
													 0:Erase User Code Area 
													 1:Erase NO ENCRYPTION User Code Area 
													 2:Erase Information Area 0 
													 3:Erase Information Area 1 
													 4:Erase User Code Area and Information Area 2
													 5:Erase User Code Area and Information Area 3
													 6:Erase NO ENCRYPTION User Code Area and Information Area 2

 *    Return Value:    TRUE - Success, FALSE - Error
 */
uint8_t erase_all(uint8_t eraseMode)
{
    uint32_t chipID;   
    #ifdef DEBUG_EN
    uint32_t temp;
    #endif

    if(unclock_chip() == FALSE){                   
		return UNCLOCKFAIL;
	}
	
    prog_area_set(INFOR_AREA_SELECT);
    chipID = config_word_read(es_target_device.chipid_addr);
    if(chipID != es_target_device.chipid_value){
        //return es_target_device.chipid_addr;
    }    
	
	if(eraseMode == 0)					//全擦
	{	
		//------------------------------------------------Erase User Code Area
		
		isp_start_bit();
		isp_send(ERASE_CODE,8);
		isp_send(USER_AREA_VAL,8);	
		isp_send(0,8);			
		isp_end_bit();
		
		es_delay_ms(100);
		if(erase_check(10) == FALSE){
			return ERASEFAIL;
		}
	}
	else								//分页擦除
	{
		
		isp_start_bit();
		isp_send(ERASE_CODE,8);
		isp_send(USER_AREA_VAL,8);	
		isp_send(0,8);			
		isp_end_bit();
		
		es_delay_ms(100);
		if(erase_check(10)==FALSE){
			return ERASEFAIL;
		}		
		
	}
    
    //------------------------------------------------Erase Information Area 1(用户配置页)
    
    isp_start_bit();
    isp_send(ERASE_INF1,8);
    isp_send(INF1R_AREA_VAL,8);	
    isp_send(0,8);					
    isp_end_bit();
    
    es_delay_ms(100);    
    if(erase_check(5)==FALSE){
        return ERASEFAIL;
    }

    //------------------------------------------------Erase Information Area 2(加密)
    
	if(eraseMode==0)
	{
		isp_start_bit();
		isp_send(ERASE_INF2,8);
		isp_send(INF2R_AREA_VAL,8);	
		isp_send(0,8);					
		isp_end_bit();
		
		es_delay_ms(100);    
		if(erase_check(5)==FALSE){
			return ERASEFAIL;
		}
	}		
    
    //------------------------------------------------Erase Information Area 0、3（信息页擦除，发布时屏蔽）
    
    #ifdef DEBUG_EN
    
    isp_start_bit();
    isp_send(ERASE_INF0_UNLOCK,16);
    isp_end_bit();		

    isp_start_bit();
    isp_send(ERASE_INF0,8);
    isp_send(INF0R_AREA_VAL,8);
    isp_send(0,8);
    isp_end_bit();
    
    es_delay_ms(100);    
    if(erase_check(5)==FALSE){
        return ERASEFAIL;
    }
    
    isp_start_bit();
    isp_send(ERASE_INF3,8);
    isp_send(INF3R_AREA_VAL,8);
    isp_send(0,8);
    isp_end_bit();
    
    es_delay_ms(100);    
    if(erase_check(5)==FALSE){
        return ERASEFAIL;
    } 

    isp_start_bit();
    isp_send(0xB196,16);
    isp_end_bit();
    
    isp_start_bit();
    isp_send(0xB0,8);
    temp = isp_recv_byte();
    isp_end_bit();
    if(temp != 0X96)return ERASEFAIL;       
    
    #endif        

    return ERROR_SUCCESS;
}

/*
 *  Check if Information Area is Empty
 *    Parameters:      None
 *    Return Value:    TRUE(1) - Success, UNCLOCKFAIL(0xA8) - Unclock Fault, CHECHEMPTYFAIL(0xAA) - Check Empty Fault
 */
 uint8_t empty_check(void)
{
	uint32_t i;//,temp;
    uint32_t check_temp;
    
    if(unclock_chip()==FALSE){                
		return UNCLOCKFAIL;
	}	
	prog_area_set(CODE_AREA_SELECT);
    prog_addr_set(0);
	for(i=0; i<es_target_device.code_size/8; i++){		
		if((i%0x40) == 0){
            isp_start_bit();
			isp_send(STATUS_CHECK_CD,8);
			check_temp = isp_recv_byte();
			isp_end_bit();
		}
		if((i%0x1000) == 0){}
		
		
		isp_process_err.data = config_read_block();//config_word_read(i*8);

		if(isp_process_err.data != 0xFFFFFFFF){
			isp_process_err.addr = prog_addr_read() - 8;
			return CHECHEMPTYFAIL;
		}	

		isp_process_err.data = config_read_word();

		if(isp_process_err.data!= 0xFFFFFFFF){
			isp_process_err.addr = prog_addr_read();
			return CHECHEMPTYFAIL;
		}
		
		if(check_temp != MODE_CHECK_VAL){
			return UNCLOCKFAIL;
		}
	}

	prog_area_set(INFOR_AREA_SELECT);
	prog_addr_set(USER_INFO_START_ADDR);
	for(i=0; i<=0x06; i++)
    {
		isp_process_err.data = config_read_block();
		if(isp_process_err.data != 0xFFFFFFFF){
			isp_process_err.addr = prog_addr_read() - 8;
			return CHECHEMPTYFAIL;
		}
        
		isp_process_err.data=config_read_word();
		if(isp_process_err.data!=0xFFFFFFFF){
			isp_process_err.addr = prog_addr_read();
			return CHECHEMPTYFAIL;
		}        
	}
    
    prog_addr_set(0x000007C0);
	for(i=0; i<=0x06; i++)
    {
		isp_process_err.data = config_read_block();
		if(isp_process_err.data != 0xFFFFFFFF){
			isp_process_err.addr = prog_addr_read() - 8;
			return CHECHEMPTYFAIL;
		}
        
		isp_process_err.data = config_read_word();
		if(isp_process_err.data != 0xFFFFFFFF){
			isp_process_err.addr = prog_addr_read();
			return CHECHEMPTYFAIL;
		}        
	}
    
    prog_addr_set(0x00000800);
	for(i=0; i<=0x3; i++)
    {
		isp_process_err.data = config_read_block();
		if(isp_process_err.data != 0xFFFFFFFF){
			isp_process_err.addr = prog_addr_read() - 8;
			return CHECHEMPTYFAIL;
		}
        
		isp_process_err.data = config_read_word();
		if(isp_process_err.data != 0xFFFFFFFF){
			isp_process_err.addr = prog_addr_read();
			return CHECHEMPTYFAIL;
		}        
	}    
	
	return ERROR_SUCCESS;
}

/*
 *  CRC Check if  Area is Empty
 *    Parameters:      area, address, size
 *    Return Value:    TRUE(1) - Success, UNCLOCKFAIL(0xA8) - Unclock Fault, CHECHEMPTYFAIL(0xAA) - Check Empty Fault
 */
uint32_t crc_empty_check(uint32_t area, uint32_t address, uint32_t size)
{
	uint32_t temp;

    if(unclock_chip() == FALSE){                
		return UNCLOCKFAIL;
	}	
	if(crc_init(area, address, size) == FALSE){
		crc_disable();
		return FALSE;
	}	
	crc_triger_set(0xA0); //Triger Check Empty
	while(crc_state_check()&0x10); //Wait for Finish
	temp = crc_state_check()&0x20; //Check result
	if(temp != 0){
		isp_process_err.addr = crc_addr_read();
		isp_process_err.data = config_word_read(isp_process_err.addr);			
		crc_disable();
		return CHECHEMPTYFAIL;
	}
	if(crc_disable() != TRUE){ 
		return FALSE;
	}
	return TRUE;
}

/*
 *  CRC Check if  All Area is Empty
 *    Parameters:      None
 *    Return Value:    TRUE(1) - Success, UNCLOCKFAIL(0xA8) - Unclock Fault, CHECHEMPTYFAIL(0xAA) - Check Empty Fault
 */
uint32_t crc_all_empty_check(void)
{
	uint32_t temp;
	temp = crc_empty_check(CODE_AREA_SELECT, es_target_device.code_start, (es_target_device.code_size/8-1));
	if(temp != TRUE){
		if(temp == UNCLOCKFAIL){
			return UNCLOCKFAIL;
		}
		return CHECHEMPTYFAIL;	
	}
	temp = crc_empty_check(INFOR_AREA_SELECT, es_target_device.config_start+0x200, 256/4);//(CFG_AREA_SIZE+3)/4);
	if(temp != TRUE){
		if(temp == UNCLOCKFAIL){
			return UNCLOCKFAIL;
		}
		return CHECHEMPTYFAIL;	
	}
//	temp = crc_empty_check(INFORAREASELECT, es_target_device.config_start+ENCRYPINFOADDR, (ENCRYPINFOSIZE+3)/4);
//	if(temp != TRUE){
//		if(temp == UNCLOCKFAIL){
//			return UNCLOCKFAIL;
//		}
//		return CHECHEMPTYFAIL;	
//	}
	return TRUE;	
}

/*
 *  Programme Config Word
 *    Parameters:      None
 *    Return Value:    TRUE(1) - Success, UNCLOCKFAIL(0xB4) - Unclock Fault, PRGCONFIGFAIL(0xAC) - Programme Fault
 */
error_t config_programe(uint32_t *buf)      
{

    uint8_t i;
//	uint32_t error_addr;
    #ifdef DEBUG_EN
    uint32_t temp;
    #endif    
    if(unclock_chip()==FALSE){                    
		return UNCLOCKFAIL;
	}
    
	prog_area_set(INFOR_AREA_SELECT);
    
    //-----------------------------------------------------------------------配置字编程（4个字）
            
	if(prog_config_word(USER_INFO_START_ADDR, buf[0]) == FALSE){
		isp_process_err.addr = prog_addr_read();
		return PRGCONFIGFAIL;
	}
	for(i=1;i<=6;i++){
		if(prog_config_packet(buf[i*2]) == FALSE){
			isp_process_err.addr = prog_addr_read();
			return PRGCONFIGFAIL;
		}
	}     
	isp_process_err.addr = prog_addr_read(); 
    
    //-----------------------------------------------------------------------校验和、UserID编程（6个字）    5+4=9
       
	if(prog_config_word(0x000007C0,buf[14]) == FALSE){
		isp_process_err.addr = prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=6;i++){
		if(prog_config_packet(buf[14+i*2])==FALSE){
			isp_process_err.addr =prog_addr_read();
			return PRGCONFIGFAIL;
		}
	} 

	isp_process_err.addr =prog_addr_read();    
    //-----------------------------------------------------------------------机台信息页编程（97个字）      9+6+2=17（两个为加密字）

    #ifdef DEBUG_EN
    
    prog_area_set(INFOR_AREA_SELECT);
    
    isp_start_bit();
    isp_send(ERASE_INF0_UNLOCK,16);
    isp_end_bit();
    
    isp_start_bit();
    isp_send(0xB0,8);
    temp=isp_recv_byte();
    isp_end_bit();
    if(temp!=0X69)return FALSE;    
    
    //-----------------------------------------------------------------------
    
	if(prog_config_word(0x00000000,buf[36])==FALSE){                   
		error_addr =prog_addr_read();
		return PRGCONFIGFAIL;
	}
    
    error_addr =prog_addr_read();

	if(prog_config_word(0x00000008,buf[38])==FALSE){                   
		error_addr =prog_addr_read();
		return PRGCONFIGFAIL;
	}
    
	error_addr = prog_addr_read();    

    //-----------------------------------------------------------------------
    
	if(prog_config_word(0x00000038,buf[40])== FALSE){                   
		error_addr = prog_addr_read();
		return PRGCONFIGFAIL;
	}
	
	for(i=1;i<=9;i++){
		if(prog_config_packet(buf[40+i*2])==FALSE){
			error_addr =prog_addr_read();
			return PRGCONFIGFAIL;
		}
	}
    
	error_addr =prog_addr_read();    
    
    //-----------------------------------------------------------------------
    
	if(prog_config_word(0x00000200,buf[60])==FALSE){           
		error_addr =prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=40;i++){
		if(prog_config_packet(buf[60+i*2])==FALSE){
			error_addr =prog_addr_read();
			return PRGCONFIGFAIL;
		}
	} 
    
	error_addr =prog_addr_read();    

    isp_start_bit();
    isp_send(0xB196,16);
    isp_end_bit();
    
    isp_start_bit();
    isp_send(0xB0,8);
    temp=isp_recv_byte();
    isp_end_bit();
    if(temp!=0X96)return PRGCONFIGFAIL;      
    
    #endif    

    //-----------------------------------------------------------------------
       
	//error_addr =prog_addr_read();
	return ERROR_SUCCESS;
}

/*
uint8_t ConfigProgramme(void)
{
	uint32_t i;
    uint8_t retval;
    
//	FastReadOffLineFlash(Finalsector, 4, USBReceiveBuff.BReceiveBuff+12);	
//	FastReadOffLineFlash((Finalsector+4), 4, USBReceiveBuff.BReceiveBuff+16);	
//	temp=USBReceiveBuff.DReceiveBuff[4];
//	FastReadOffLineFlash((Finalsector+8), temp, USBReceiveBuff.BReceiveBuff+20);	
    
    FastReadOffLineFlash((Finalsector+8), 512, USBReceiveBuff.BReceiveBuff+20);

    if(unclock_chip()==FALSE){                    
		return UNCLOCKFAIL;
	}
    
	prog_area_set(INFOR_AREA_SELECT);
    
    //-----------------------------------------------------------------------配置字编程（4个字）
    
	if(prog_config_word(USER_INFO_START_ADDR,USBReceiveBuff.DReceiveBuff[5])==FALSE){
		error.addr=prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=3;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[5+i])==FALSE){
			error.addr=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	}    
    
    //-----------------------------------------------------------------------校验和、UserID编程（6个字）    5+4=9
    
	if(prog_config_word(0x000003E8,USBReceiveBuff.DReceiveBuff[9])==FALSE){
		error.addr=prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=5;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[9+i])==FALSE){
			error.addr=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	} 

    //-----------------------------------------------------------------------机台信息页编程（97个字）      9+6+2=17（两个为加密字）

    #ifdef DEBUG_EN
    
    isp_start_bit();
    isp_send(ERASE_INF0_UNLOCK,16);
    isp_end_bit();
    
	if(prog_config_word(0x00000000,USBReceiveBuff.DReceiveBuff[17])==FALSE){                   
		error.addr=prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=32;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[17+i])==FALSE){
			error.addr=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	}
    
    //-----------------------------------//17+33=50(中间0x84~0xFF跳过)
    
	if(prog_config_word(0x00000100,USBReceiveBuff.DReceiveBuff[50])==FALSE){           
		error.addr=prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=32;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[50+i])==FALSE){
			error.addr=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	}    
    
    #endif    
	
	return TRUE;
}*/

//uint8_t ConfigProgramme_AUTO(uint32_t address,uint32_t data){
////	GPIO_SetBits(GPIOA,MRST_PIN);
////	es_delay_ms(10);
//  if(unclock_chip()==FALSE){
//		return UNCLOCKFAIL;
//	}
//	prog_area_set(INFORAREASELECT);	
//	SetProgrammeAdd(address);
//	info_buffer_write(data);	
//	if(ProgOperation(WORDPRGCD)==FALSE){
//		error.addr=prog_addr_read();
//		return PRGCONFIGFAIL;
//	}	
//	return TRUE;
//}

//uint8_t ConfigProgramme_Test(uint32_t address, uint32_t number, uint32_t *point){
//	uint32_t i;	
//  if(unclock_chip()==FALSE){                   
//		return UNCLOCKFAIL;
//	}
//	prog_area_set(INFORAREASELECT);
//	if(prog_config_word(address,*point)==FALSE){
//		error.addr=prog_addr_read();
//		return HEXPRGFAIL;
//	}
//	for(i=1;i<number;i++){
//		if(prog_config_packet(*(point+i))==FALSE){
//			error.addr=prog_addr_read();
//			return HEXPRGFAIL;
//		}
//	}	
//	return TRUE;
//}

/*
 *  config_read
 *    Parameters:      None
 *    Return Value:    TRUE(1) - Success, FALSE(0) - Fault
 */
uint8_t config_read(uint32_t address, uint32_t number, uint32_t *data)
{
	uint32_t i;
    
	prog_addr_set(address);	
	for (i=0; i<number; i++)
    {
		*(data+i*2) = config_read_block();
        *(data+i*2+1) = config_read_word();
	} 
    
	return TRUE;
} 

uint8_t option_read(uint32_t address, uint32_t number, uint32_t *buf)
{    
    if(unclock_chip()==FALSE)
    {                   
		return UNCLOCKFAIL;
	}	
    
	prog_area_set(INFOR_AREA_SELECT);
    
	config_read(address, number, buf);
    
	return TRUE;
}
 
uint8_t config_read_all(uint32_t address, uint32_t number, uint32_t *buf)
{
	uint32_t temp;
    
    if(unclock_chip()==FALSE){                   
		return UNCLOCKFAIL;
	}	
    
	prog_area_set(INFOR_AREA_SELECT);
    
    if(address == 0x00000000)
    {
    
        config_read(USER_INFO_START_ADDR, 7, buf);
        config_read(0x000007C0, 7, buf + 14);        
        config_read(0x00000800, 4, buf + 28); 

        if(number <= 144)return TRUE;
        
        //#ifdef DEBUG_EN   开放读

        isp_start_bit();
        isp_send(ERASE_INF0_UNLOCK,16);
        isp_end_bit();
        
        isp_start_bit();
        isp_send(0xB0,8);
        temp = isp_recv_byte();
        isp_end_bit();
        if(temp!=0X69)return FALSE;
        
        config_read(0x00000000, 2,  buf + 36);
        config_read(0x00000038, 10, buf + 40);
        config_read(0x00000200, 31, buf + 60); 
        
        isp_start_bit();
        isp_send(0xB196,16);
        isp_end_bit();

        isp_start_bit();
        isp_send(0xB0,8);
        temp = isp_recv_byte();
        isp_end_bit();
        if(temp != 0X96)return FALSE;  

        //#endif
    }
    else
    {
        isp_start_bit();
        isp_send(ERASE_INF0_UNLOCK,16);
        isp_end_bit();
        
        isp_start_bit();
        isp_send(0xB0,8);
        temp = isp_recv_byte();
        isp_end_bit();
        if(temp != 0X69)return FALSE;
        
        config_read(0x000002F8, 10, buf);

        isp_start_bit();
        isp_send(0xB196,16);
        isp_end_bit();
        
        isp_start_bit();
        isp_send(0xB0,8);
        temp = isp_recv_byte();
        isp_end_bit();
        if(temp!=0X96)return FALSE;          
    }
    
	return TRUE;
}

///*
// *  Programme Calibration Word
// *    Parameters:      None
// *    Return Value:    TRUE(1) - Success, UNCLOCKFAIL(0xB4) - Unclock Fault, PRGCONFIGFAIL(0xAC) - Programme Fault
// */
////uint8_t CalProgramme(void){
////	uint32_t i,temp;	
////	
////	FastReadOffLineFlash(Finalsector+CALINFOADDR, 4, USBReceiveBuff.BReceiveBuff+12);	
////	FastReadOffLineFlash((Finalsector+CALINFOADDR+4), 4, USBReceiveBuff.BReceiveBuff+16);	
////	temp=USBReceiveBuff.DReceiveBuff[4];
////	if(temp > 0x200){
////			error.addr = 0x100;
////			return PRGCONFIGFAIL;		
////	}
////	FastReadOffLineFlash((Finalsector+CALINFOADDR+8), temp, USBReceiveBuff.BReceiveBuff+20);		

////    if(unclock_chip()==FALSE){                    
////		return UNCLOCKFAIL;
////	}
////	prog_area_set(INFORAREASELECT);
////	if(prog_config_word(USBReceiveBuff.DReceiveBuff[3],USBReceiveBuff.DReceiveBuff[5])==FALSE){
////		error.addr=prog_addr_read();
////		return PRGCONFIGFAIL;
////	}	
////	for(i=1;i<(USBReceiveBuff.DReceiveBuff[4]+3)/4;i++){
////		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[5+i])==FALSE){
////			error.addr=prog_addr_read();
////			return PRGCONFIGFAIL;
////		}
////	}
////	
////	return TRUE;
////}

uint8_t flash_programme(uint32_t address, uint32_t number, uint32_t *point)
{
    uint32_t i;	
	
	if(number%2 != 0){
		return FALSE;
	}
		
//    if(unclock_chip()==FALSE){                   
//		return UNCLOCKFAIL;
//	}
	prog_area_set(CODE_AREA_SELECT);
	if(prog_flash_word(address,*point,*(point+1))==FALSE){
		isp_process_err.addr = prog_addr_read();
		return HEXPRGFAIL;
	}
	for(i=2; i<number; i+=2){
		if(prog_flash_packet(*(point+i),*(point+i+1)) == FALSE){
			isp_process_err.addr = prog_addr_read();
			return HEXPRGFAIL;
		}
	}	
	return TRUE;
}

static uint8_t flash_read(uint32_t address, uint32_t number, uint32_t *point)
{
	uint32_t i;
	
	if(number%2 != 0){
		return FALSE;
	}

	prog_addr_set(address);
    
	for(i=0;i<number;i+=2){
		*(point+i) = config_read_block();
		*(point+i+1) = config_read_word();
	}	
	
	return TRUE;
}

/*
 *  EncrypChip
 *    Parameters:      None
 *    Return Value:    TRUE(1) - Success, UNCLOCKFAIL(0xA8) - Unclock Fault, ENCRYPFAIL(0xAE) - Encryp Fault
 */
error_t encrypt_chip(uint32_t *buf)      
{
    uint32_t temp;

//    FastReadOffLineFlash((Finalsector+112), 32, USBReceiveBuff.BReceiveBuff+28);
    
    if(unclock_chip()==FALSE){                   
		return UNCLOCKFAIL;
	}
	
	prog_area_set(INFOR_AREA_SELECT);
    
    //-------------------------------------------------------------------

	if(prog_flash_word(0x0800,buf[0],buf[2])==FALSE){ 
		return ENCRYPFAIL;
	}

    if(prog_config_word(0x0810,buf[4])==FALSE){ 
		return ENCRYPFAIL;
	}  

    if(prog_config_word(0x0818,buf[6])==FALSE){ 
		return ENCRYPFAIL;
	}   

    //-------------------------------------------------------------------    
    
	if(config_word_read(0x0800) != buf[0]){
		return ENCRYPFAIL;
	}
    
	if(config_word_read(0x0810) != buf[4]){
		return ENCRYPFAIL;
	}    

	if(config_word_read(0x0818) != buf[6]){
		return ENCRYPFAIL;
	}    
    
    isp_start_bit();
    isp_send(ENCRYPT_LOAD_CD,8);         //Load Encryption Words
    temp = isp_recv_3rdbyte();	
    isp_end_bit();
    if(temp != ENCRYPTION_VAL){                  
        
        return ENCRYPFAIL;
    }
    
    return ERROR_SUCCESS;	

}

//uint8_t FULLFlashProgramme(uint8_t *buf)
//{    
//	uint32_t temp, pagenum, n, i;//,finalpage
//	uint32_t encrypword[2],encrypstart,encrypend;    
//    uint32_t chipID;

//    prog_area_set(INFOR_AREA_SELECT);
//    chipID = config_word_read(es_target_device.chipid_addr);
//    if(chipID != es_target_device.chipid_value){
//        //return CHIPIDERROR;
//    }  
//    
//	pagenum = es_target_device.code_size/1024;
//	
//	temp = config_read(CFG_ENCPAGE0_ADDR, 1, encrypword);
//	if(temp != TRUE){
//		return temp;  
//	}
//	encrypstart = encrypword[0] & 0x7F;
//	encrypend = (encrypword[0]>>8) & 0x7F;
//	if(((encrypword[0]>>7)&0x1) && ((encrypword[0]>>15)&0x1)){
//		encrypword[1] = 0;
//	}
//	else{
//		if(encrypstart <= encrypend){
//			encrypword[1] = 0x10;
//		}
//		else{
//			encrypword[1] = 0x11;
//		}
//	}
//	
//	prog_area_set(CODE_AREA_SELECT);

//	for(n=0; n<pagenum; n++){
//		if((encrypword[1]==0)||                                           // no page encryption
//			 ((encrypword[1]==0x10)&&((n<encrypstart)||(n>encrypend)))||    // middle page encryption and lay aside
//		   ((encrypword[1]==0x11)&&((encrypstart>n>encrypend))))          // side page encryption and lay centrally
//		{

//			temp = FastReadOffLineFlash(1024*n, 1024, buf);
//			if(temp!=TRUE){
//				return FALSE;
//			}
//			j=FALSE;
//			for(i=0; i<256; i++){
//				if(USBReceiveBuff.DReceiveBuff[5+i]!=0xFFFFFFFF){				
//					j=TRUE;
//					break;	
//				}							
//			}
//			if(j==TRUE){			
//				if(SerialNumFlag==TRUE){
//					if((SerialNumBuff[1]+3)/4==1){
//						if((serial_number->addr>=256*n)&(SerialNumBuff[0]<256*(n+1))){
//							USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*n)]=SerialNumBuff[2];	
//							SerialNumFlag=FALSE;
//						}
//					}
//					else if((SerialNumBuff[1]+3)/4==2){
//						if((SerialNumBuff[0]>=256*n)&(SerialNumBuff[0]<256*(n+1)))
//							USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*n)]=SerialNumBuff[2];	
//						if(((SerialNumBuff[0]+1)>=256*n)&((SerialNumBuff[0]+1)<256*(n+1))){
//							USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*n)+1]=SerialNumBuff[3];	
//							SerialNumFlag=FALSE;
//						}
//					}
//					else{
//						SerialNumFlag=FALSE;
//						return FALSE;
//					}
//				}

//				
//				temp=FlashProgramme(1024*n,256,USBReceiveBuff.DReceiveBuff+5);
//				if(temp!=TRUE){				
//					return FALSE;	
//				}	
//			}				
//		}
//	}
//	
//	return TRUE;	
//}

error_t flash_full_programme(uint32_t addr,  uint32_t *buf, uint32_t size)
{
    uint32_t i;
    uint32_t program_buffer_size = 256;
    static uint8_t sn_is_valid = ENABLE;
    
    if(unclock_chip() == FALSE){                   
        return UNCLOCKFAIL;
    }    
    prog_area_set(INFOR_AREA_SELECT);
    if (config_word_read(es_target_device.chipid_addr) != es_target_device.chipid_value){
        //return CHIPIDERROR;
    } 

    prog_area_set(CODE_AREA_SELECT);
    while (size > 0) {
        uint32_t write_size = MIN(size, program_buffer_size);
        for(i=0; i<write_size; i++){
            if(buf[i] != 0xFFFFFFFF)				
                break;	            							
        }
        
        if(isp_serial_number.enable_flag != ENABLE){      //序列号不使能
            if(i >= write_size)                 //写入的数据都为FF，与擦除后的数据一样，不需要烧录
                return ERROR_SUCCESS;
        }
        else{                                   // 序列号使能                
            if(sn_is_valid == ENABLE){              // 序列号写一次后失效                  
                if((isp_serial_number.size+3)/4 == 1){
                    if( (isp_serial_number.addr >= addr) & (isp_serial_number.addr < addr+write_size) ){
                        buf[isp_serial_number.addr - addr] = isp_serial_number.data[1];	
                        sn_is_valid = DISABLE;
                    }
                }
                else if((isp_serial_number.size+3)/4 == 2){
                    if((isp_serial_number.addr >= addr) & (isp_serial_number.addr < addr+write_size))
                        buf[isp_serial_number.addr - addr] = isp_serial_number.data[0];	
                    if( ((isp_serial_number.addr+1) >= addr) & ((isp_serial_number.addr+1) < addr+write_size)){
                        buf[(isp_serial_number.addr - addr)+1] = isp_serial_number.data[1];	
                        sn_is_valid = DISABLE;
                    }
                }
                else{
                    sn_is_valid = DISABLE;
                    return SERIALNUMCHECKSUMFAIL;
                }  
            }

            //此区域都为FF，支持序列号功能且序列号地址不在此区域，不需要烧录
            if( (i >= write_size) && (sn_is_valid == ENABLE) )      
                return ERROR_SUCCESS;        
        }     
        
//        if(unclock_chip() == FALSE){                   
//            return UNCLOCKFAIL;
//        }   
        
        if(flash_programme(addr, write_size, buf)  != TRUE)
            return HEXPRGFAIL;	        
        addr += write_size;
        buf += write_size;
        size -= write_size;
    }
    return ERROR_SUCCESS;      
    
}
/*
error_t code_area_verify(isp_serial_number_t *serial_number)
{
	uint32_t i,j;
    uint32_t read_data[2];
    union B32_B08 temp;
    uint32_t pagenum,CheckSum,SerialNumBuff_temp[2],CheckSumFlag;//,finalpage
//	uint32_t encryPageWord[2],encryPageStart,encryPageEnd;
    uint32_t_uint8_t code_buf,sf_buf;
    
    if(unclock_chip()==FALSE){                   
		return UNCLOCKFAIL;
	}
    
    prog_area_set(INFOR_AREA_SELECT);
    if(config_word_read(es_target_device.chipid_addr) != es_target_device.chipid_value){
        //return FALSE;
    }    
    pagenum = es_target_device.code_size/1024;                                                       //Code area verify
	CheckSumFlag = FALSE;
	CheckSum=0;   
    
	prog_area_set(CODE_AREA_SELECT);
	for(i=0; i<pagenum; i++){
        if (spi_flash_read(i*1024, 1024, sf_buf.B08) != SF_SUCCESS)
            return CODEVERIFYFAIL;
        if (flash_read(i*1024, 256, code_buf.B32)!= TRUE)
            return CODEVERIFYFAIL;
        if(serial_number->flag == TRUE){
            if((serial_number->size + 3)/4 == 1){
                if((serial_number->addr >= 256*i) & (serial_number->addr < 256*(i+1))){                    
                    SerialNumBuff_temp[0] = sf_buf.B32[serial_number->addr - 256*i];	
                    sf_buf.B32[serial_number->addr - 256*i] = serial_number->data[0];
                    serial_number->flag = DISABLE;      //序列号写一次失效
                    CheckSumFlag = TRUE;
                }
            }
            else if((serial_number->size + 3)/4 == 2){
                if((serial_number->addr >= 256*i) & (serial_number->addr < 256*(i+1))){
                    SerialNumBuff_temp[0] = sf_buf.B32[serial_number->addr - 256*i];	
                    sf_buf.B32[serial_number->addr-256*i] = serial_number->data[0];
                }
                if(((serial_number->addr+1) >= 256*i)&( (serial_number->addr+1 )< 256*(i+1))){
                    SerialNumBuff_temp[1] = sf_buf.B32[serial_number->addr - 256*i + 1];						
                    sf_buf.B32[serial_number->addr - 256*i + 1] = serial_number->data[1];	
                    serial_number->flag = DISABLE;
                    CheckSumFlag = TRUE;
                }
            }
            else{ 
                 serial_number->flag = DISABLE;
                return FALSE;
            }
        }				
        for(j=0; j<1024; j++){
            if(sf_buf.B08[j] != code_buf.B08[j]){
                error.addr = i*1024+j;
                flash_read(error.addr, 2, read_data);
                error.data = read_data[0];
                return FALSE;											
            }
            CheckSum += code_buf.B08[j];
        }
        if(CheckSumFlag == TRUE){
            if((serial_number->size+3)/4 == 1){
                CheckSum -= (serial_number->data[0] >> 24) & 0xFF;
                CheckSum -= (serial_number->data[0] >> 16) & 0xFF;
                CheckSum -= (serial_number->data[0] >> 8)  & 0xFF;
                CheckSum -= (serial_number->data[0] >> 0)  & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 24) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 16) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 8)  & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 0)  & 0xFF;
               
                CheckSumFlag=FALSE;
            }
            else if((serial_number->size+3)/4==2){
                CheckSum -= (serial_number->data[0] >> 24) & 0xFF;
                CheckSum -= (serial_number->data[0] >> 16) & 0xFF;
                CheckSum -= (serial_number->data[0] >> 8)  & 0xFF;
                CheckSum -= (serial_number->data[0] >> 0)  & 0xFF;
                CheckSum -= (serial_number->data[1] >> 24) & 0xFF;
                CheckSum -= (serial_number->data[1] >> 16) & 0xFF;
                CheckSum -= (serial_number->data[1] >> 8)  & 0xFF;
                CheckSum -= (serial_number->data[1] >> 0)  & 0xFF;
                
                CheckSum -= (SerialNumBuff_temp[0] >> 24) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 16) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 8)  & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 0)  & 0xFF;
                CheckSum -= (SerialNumBuff_temp[1] >> 24) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[1] >> 16) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[1] >> 8)  & 0xFF;
                CheckSum -= (SerialNumBuff_temp[1] >> 0)  & 0xFF;
                CheckSumFlag = FALSE;
            }	
            else{
                return FALSE;
            }			
        }		
	}

	spi_flash_read(ThirdFinalsector, 4, temp.B08);	
	CheckSum &= 0x0000FFFF;
	temp.B32 &= 0x0000FFFF;
	if(CheckSum != temp.B32){
		return HEXCHECKSUMFAIL;			
	}
}
*/
/*
 * 验证芯片内部数据与spi flash中的数据+序列号数据是否一致。
 */
uint8_t code_area_verify( uint32_t addr,  uint32_t *buf, uint32_t size)
{
    uint32_t n;
    uint32_t program_buffer_size = 256;        //每次读code 数据长度
    union B32_B08 code_buf[256]; 
    static uint8_t sn_is_valid = ENABLE;
    
     if(unclock_chip()==FALSE){                   
		return UNCLOCKFAIL;
	}
    
    prog_area_set(INFOR_AREA_SELECT);
    if(config_word_read(es_target_device.chipid_addr) != es_target_device.chipid_value){
        //return FALSE;
    }    
   
//	CheckSumFlag = FALSE;
//	CheckSum=0;   
    
	prog_area_set(CODE_AREA_SELECT);
    
    while (size > 0) {
         uint32_t write_size = MIN(size, program_buffer_size);        
         if (flash_read(addr, write_size, &code_buf[0].B32)!= TRUE)
            return UNCLOCKFAIL;
         if( (isp_serial_number.enable_flag == ENABLE) && (sn_is_valid == ENABLE) ){
            if((isp_serial_number.size + 3)/4 == 1){
                if((isp_serial_number.addr >= addr) & (isp_serial_number.addr < addr+write_size)){                    
//                    data_temp[0] = buf[serial_number->addr - addr];	            //SPI flash此位置的原始值
                    buf[isp_serial_number.addr - addr] = isp_serial_number.data[0];   //填入序列号值，
                    sn_is_valid = DISABLE;      

                }
            }
            else if((isp_serial_number.size + 3)/4 == 2){
                if((isp_serial_number.addr >= addr) & (isp_serial_number.addr < addr+write_size)){
//                    data_temp[0] = buf[serial_number->addr - addr];	
                    buf[isp_serial_number.addr - addr] = isp_serial_number.data[0];
                }
                if( ((isp_serial_number.addr+1) >= addr)&( (isp_serial_number.addr+1 )< addr + write_size) ){
//                    data_temp[1] = buf[serial_number->addr - addr + 1];						
                    buf[isp_serial_number.addr - addr + 1] = isp_serial_number.data[1];	
                    sn_is_valid = DISABLE;

                }
            }
            else{ 
                 sn_is_valid = DISABLE;
                 return SERIALNUMCHECKSUMFAIL;
            }
        }
        for(n=0; n<write_size; n++){
            if(buf[n] != code_buf[n].B32){
                isp_process_err.addr = addr + n;
//                flash_read(error.addr, 2, read_temp);
//                error.data = read_temp[0];
                isp_process_err.data = code_buf[n].B32;
                return CODEVERIFYFAIL;											
            }                
        }         
    }
    return ERROR_SUCCESS;     
/*	uint32_t i,j;
    uint32_t read_data[2];
    union B32_B08 temp;
    uint32_t pagenum,CheckSum,SerialNumBuff_temp[2],CheckSumFlag;//,finalpage
//	uint32_t encryPageWord[2],encryPageStart,encryPageEnd;
    uint32_t_uint8_t code_buf,sf_buf;
    
    if(unclock_chip()==FALSE){                   
		return UNCLOCKFAIL;
	}
    
    prog_area_set(INFOR_AREA_SELECT);
    if(config_word_read(es_target_device.chipid_addr) != es_target_device.chipid_value){
        //return FALSE;
    }    
    pagenum = es_target_device.code_size/1024;                                                       //Code area verify
	CheckSumFlag = FALSE;
	CheckSum=0;   
    
	prog_area_set(CODE_AREA_SELECT);
	for(i=0; i<pagenum; i++){
        if (spi_flash_read(i*1024, 1024, sf_buf.B08) != SF_SUCCESS)
            return CODEVERIFYFAIL;
        if (flash_read(i*1024, 256, code_buf.B32)!= TRUE)
            return CODEVERIFYFAIL;
        if(serial_number->flag == TRUE){
            if((serial_number->size + 3)/4 == 1){
                if((serial_number->addr >= 256*i) & (serial_number->addr < 256*(i+1))){                    
                    SerialNumBuff_temp[0] = sf_buf.B32[serial_number->addr - 256*i];	
                    sf_buf.B32[serial_number->addr - 256*i] = serial_number->data[0];
                    serial_number->flag = DISABLE;      //序列号写一次失效
                    CheckSumFlag = TRUE;
                }
            }
            else if((serial_number->size + 3)/4 == 2){
                if((serial_number->addr >= 256*i) & (serial_number->addr < 256*(i+1))){
                    SerialNumBuff_temp[0] = sf_buf.B32[serial_number->addr - 256*i];	
                    sf_buf.B32[serial_number->addr-256*i] = serial_number->data[0];
                }
                if(((serial_number->addr+1) >= 256*i)&( (serial_number->addr+1 )< 256*(i+1))){
                    SerialNumBuff_temp[1] = sf_buf.B32[serial_number->addr - 256*i + 1];						
                    sf_buf.B32[serial_number->addr - 256*i + 1] = serial_number->data[1];	
                    serial_number->flag = DISABLE;
                    CheckSumFlag = TRUE;
                }
            }
            else{ 
                 serial_number->flag = DISABLE;
                return FALSE;
            }
        }				
        for(j=0; j<1024; j++){
            if(sf_buf.B08[j] != code_buf.B08[j]){
                error.addr = i*1024+j;
                flash_read(error.addr, 2, read_data);
                error.data = read_data[0];
                return FALSE;											
            }
            CheckSum += code_buf.B08[j];
        }
        if(CheckSumFlag == TRUE){
            if((serial_number->size+3)/4 == 1){
                CheckSum -= (serial_number->data[0] >> 24) & 0xFF;
                CheckSum -= (serial_number->data[0] >> 16) & 0xFF;
                CheckSum -= (serial_number->data[0] >> 8)  & 0xFF;
                CheckSum -= (serial_number->data[0] >> 0)  & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 24) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 16) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 8)  & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 0)  & 0xFF;
               
                CheckSumFlag=FALSE;
            }
            else if((serial_number->size+3)/4==2){
                CheckSum -= (serial_number->data[0] >> 24) & 0xFF;
                CheckSum -= (serial_number->data[0] >> 16) & 0xFF;
                CheckSum -= (serial_number->data[0] >> 8)  & 0xFF;
                CheckSum -= (serial_number->data[0] >> 0)  & 0xFF;
                CheckSum -= (serial_number->data[1] >> 24) & 0xFF;
                CheckSum -= (serial_number->data[1] >> 16) & 0xFF;
                CheckSum -= (serial_number->data[1] >> 8)  & 0xFF;
                CheckSum -= (serial_number->data[1] >> 0)  & 0xFF;
                
                CheckSum -= (SerialNumBuff_temp[0] >> 24) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 16) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 8)  & 0xFF;
                CheckSum -= (SerialNumBuff_temp[0] >> 0)  & 0xFF;
                CheckSum -= (SerialNumBuff_temp[1] >> 24) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[1] >> 16) & 0xFF;
                CheckSum -= (SerialNumBuff_temp[1] >> 8)  & 0xFF;
                CheckSum -= (SerialNumBuff_temp[1] >> 0)  & 0xFF;
                CheckSumFlag = FALSE;
            }	
            else{
                return FALSE;
            }			
        }		
	}

	spi_flash_read(ThirdFinalsector, 4, temp.B08);	
	CheckSum &= 0x0000FFFF;
	temp.B32 &= 0x0000FFFF;
	if(CheckSum != temp.B32){
		return HEXCHECKSUMFAIL;			
	}
    */
}
uint8_t info_area_verify(uint32_t *buf)
{
    uint32_t info_buf[28];
	uint32_t temp,CheckSum_1,CheckSum_2,j;
	
    CheckSum_1=0;
	CheckSum_2=0;

    

    //-----------------------------------------------------------------------配置字读
    
    if(unclock_chip()==FALSE){                   
		return UNCLOCKFAIL;
	}	
    
	prog_area_set(INFOR_AREA_SELECT);    
    
    temp = config_read(USER_INFO_START_ADDR, 7, info_buf);
    if(temp!=TRUE){
        return FALSE;
    }

    //-----------------------------------------------------------------------校验和、UserID读   
    
    temp=config_read(0x000007C0, 7, info_buf+14);
    if(temp!=TRUE){
        return FALSE;
    }
    
    //-----------------------------------------------------------------------和下载的配置做比较    
    
    for(j=0;j<7;j++)
    {
        if(buf[j*2] != info_buf[j*2])
        {
            return FALSE;										
        }
        CheckSum_1 += info_buf[j*8];
        CheckSum_1 += info_buf[j*8+1];        
        CheckSum_1 += info_buf[j*8+2];
        CheckSum_1 += info_buf[j*8+3];
        
        CheckSum_2+=buf[j*8];
        CheckSum_2+=buf[j*8+1];    
        CheckSum_2+=buf[j*8+2];
        CheckSum_2+=buf[j*8+3];       
    }
	
    for(j=9;j<14;j++)
    {
        if(buf[j*2] != info_buf[j*2])
        {
            return FALSE;										
        }
        CheckSum_1+=info_buf[j*8];
        CheckSum_1+=info_buf[j*8+1];        
        CheckSum_1+=info_buf[j*8+2];
        CheckSum_1+=info_buf[j*8+3];
        
        CheckSum_2+=buf[j*8+0];
        CheckSum_2+=buf[j*8+1];    
        CheckSum_2+=buf[j*8+2];
        CheckSum_2+=buf[j*8+3];       
    }	

    if(CheckSum_1!=CheckSum_2)
    {
        return FALSE;				
    }	
    
    //-----------------------------------------------------------------------机台信息页校验     11+6+2=19
    
    #ifdef DEBUG_EN      
    
    CheckSum_1=0;
	CheckSum_2=0;
    
    isp_start_bit();
    isp_send(ERASE_INF0_UNLOCK,16);
    isp_end_bit();
    
    isp_start_bit();
    isp_send(0xB0,8);
    temp = isp_recv_byte();
    isp_end_bit();
    if(temp!=0X69)return FALSE;   
    
    temp=config_read(0x00000000, 2, USBReceiveBuff.DReceiveBuff+43+256);
    if(temp!=TRUE)
    {
        return FALSE;
    }
    
    for(j=0;j<=1;j++)
    {
        if((USBReceiveBuff.DReceiveBuff[j*2+43]) != (USBReceiveBuff.DReceiveBuff[j*2+43+256]))
        {
            return FALSE;										
        }
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+172];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+173];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+174];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+175];
        
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+172+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+173+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+174+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+175+1024];        
    }  

    if(CheckSum_1!=CheckSum_2)
    {
        return FALSE;				
    }  

    //-----------------------------------------------------------------------
    
    CheckSum_1=0;
	CheckSum_2=0;
    
    temp=config_read(0x0000038, 10, USBReceiveBuff.DReceiveBuff+47+256);
    if(temp!=TRUE)
    {
        return FALSE;
    }
    
    for(j=0;j<=9;j++)
    {
        if((USBReceiveBuff.DReceiveBuff[j*2+47]) != (USBReceiveBuff.DReceiveBuff[j*2+47+256]))
        {
            return FALSE;										
        }
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+188];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+189]; 
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+190];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+191];
        
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+188+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+189+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+190+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+191+1024];        
    }  

    if(CheckSum_1!=CheckSum_2)
    {
        return FALSE;				
    } 

    //-----------------------------------------------------------------------
    
    CheckSum_1=0;
	CheckSum_2=0;
    
    temp=config_read(0x0000200, 41, USBReceiveBuff.DReceiveBuff+67+256);
    if(temp!=TRUE)
    {
        return FALSE;
    }
    
    for(j=0;j<=40;j++)
    {
        if((USBReceiveBuff.DReceiveBuff[j*2+67]) != (USBReceiveBuff.DReceiveBuff[j*2+67+256]))
        {
            return FALSE;										
        }
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+268];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+269]; 
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+270];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+271];
        
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+268+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+269+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+270+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+271+1024];        
    }  

    if(CheckSum_1!=CheckSum_2){
        return FALSE;				
    } 

    isp_start_bit();
    isp_send(0xB196,16);
    isp_end_bit();

    isp_start_bit();
    isp_send(0xB0,8);
    temp = isp_recv_byte();
    isp_end_bit();
    if(temp!=0X96)return FALSE;      
    
	#endif	

    //-----------------------------------------------------------------------  
   	
	return TRUE;
}

//uint8_t SerialNumVerify(void){
//	if(serial_number->size/4==1){
//		flash_read(SerialNumBuff[0]*4, 1, USBReceiveBuff.DReceiveBuff+5);	
//		if(USBReceiveBuff.DReceiveBuff[5]!=SerialNumBuff[2])
//			return FALSE;										
//	}
//	else if(SerialNumBuff[1]/4==2){
//		flash_read(SerialNumBuff[0]*4, 2, USBReceiveBuff.DReceiveBuff+5);	
//		if((USBReceiveBuff.DReceiveBuff[5]!=SerialNumBuff[2])|(USBReceiveBuff.DReceiveBuff[6]!=SerialNumBuff[3]))
//			return FALSE;									
//	}
//	else{
//		return FALSE;
//	}
//	return TRUE;	
//}

///*
// *  CRC Verify
// *    Parameters:      area, address, size
// *    Return Value:    UNCLOCKFAIL(0xA8) - Unclock Fault, FALSE - Error, crcvalue - CRC Result Value
// */
//uint8_t CRCVerify(uint32_t area, uint32_t address, uint32_t size)
//{
//    if(unclock_chip()==FALSE)
//    {                
//		return UNCLOCKFAIL;
//	}
//	
//	if(crc_init(area, address, size)==FALSE)
//    {
//		crc_disable();
//		return FALSE;
//	}
//	
//	crc_triger_set(0xCC);                 //Triger CRC Verify

//	while(crc_state_check()&0x01);        //Wait for Finish

//	CRCResult = crc_value_read();         //Read CRC Result Value
//	
//	if(crc_disable()!=TRUE)
//    { 
//		return FALSE;
//	}
//	
//	return TRUE;
//}









/*
 *  读芯片数据
 */
error_t isp_read_rom(uint32_t addr, uint32_t szie, uint32_t *buf)
{
    error_t status = ERROR_SUCCESS;
    
    if(addr == 0x00000000){
        if(unclock_chip() == FALSE){                   
            return UNCLOCKFAIL;
        }
        prog_area_set(CODE_AREA_SELECT);
    }
    if(flash_read(addr, (szie+3)/4 ,buf) != TRUE)
        status = UNCLOCKFAIL;

    return status;   

}

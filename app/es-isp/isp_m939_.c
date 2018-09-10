#include "bsp.h"
#include "eslink.h"
#include "isp_config.h"



#define STATUS_CHECK_CD    0xF0                                                //Unlock Check Command
#define ISP_MODE_CD        0xF1                                                //ISP Mode Select Command
#define ENCRYPT_LOAD_CD    0xFF                                                //Encrytion Command Download

#define ID_CHECK_CD        0x699655FA                                          //ID Check Command
#define ID_CODE_VAL        0x4D93914F                                          //ID Code//0x4D62012E//

#define AREA_SET_CD        0xE1//0                                                //Area Select Command
#define AREA_CHECK_CD      0xE0//1                                                //Area Check Command
#define SET_ADDR_CD        0xE3//2                                                //Address Set Command
#define READ_ADDR_CD       0xE2//3
#define WRITE_BUFF0_CD     0xE5//4                                                //Write Buff Command
#define READ_BUFF0_CD      0xE4//5                                                //Read Buff Command
#define WRITE_BUFF1_CD     0xE7//6                                                //Write Buff Command
#define READ_BUFF1_CD      0xE6//7                                                //Read Buff Command

#define READ_FLASH_CD      0xE8                                                //Read Data Command
#define READ_FLASH_PLUS_CD 0xEA//9                                                //Read Data Command
#define READ_FLASH1_CD     0xEC//A                                                //Read Data1 Command
#define WRITE_INFO_BUFF_CD 0xEF//B                                                //Write Buff Command

#define ERASE_CHECK_CD     0xC0                                                //Erase Check Command
#define ERASE_CODE         0xC1                                                //Erase User Code 
#define ERASE_INF0_UNLOCK  0xB169  
#define ERASE_INF0         0xC2                                                //Erase Information Area 0 Command
#define ERASE_INF1         0xC3                                                //Erase Information Area 1 Command
#define ERASE_INF2         0xC4                                                //Erase Information Area 2 Command
#define ERASE_INF3         0xC5                                                //Erase Information Area 3 Command
#define PROG_CHECK_CD      0xC8                                                //Programme Check Command
#define WORD_PRG_CD        0xC9                                                //Word Programme Command
#define WORD_ADD_PLUS_PRG  0xCA                                                 //先加后编
#define WORD_PRG_ADD_PLUS  0xCB                                                 //先编后加

#define UNLOCK_CHECK_VAL   0xA0                                                //Unlock Success Value
#define MODE_CHECK_VAL     0xA1                                                //ISP Mode Check Value

#define CODE_AREA_SELECT   0x4B 
#define INFOR_AREA_SELECT  0xBB                                                //Information Area Select Value for Reading and Programming

#define USER_AREA_VAL      0xF0                                                //User Area Select Value with Erasing
#define INF0R_AREA_VAL     0xE1                                                //Information Area 0 Select Value with Erasing
#define INF1R_AREA_VAL     0xD2                                                //Information Area 1 Select Value with Erasing
#define INF2R_AREA_VAL     0xC3                                                //Information Area 2 Select Value with Erasing
#define INF3R_AREA_VAL     0xB4                                                //Information Area 3 Select Value with Erasing

#define ENCRYPTION_VAL     0xA5A5A5                                            //Encrytion Code

#define ERASE_OK_VAL       0x69                                                //Erase-completely Value                                            
#define PROG_CHECK_VAL     0x87                                                //Programme Check Value

#define CRC_CTRL_WR        0xD1//0                                             //
#define CRC_CTRL_RD        0xD0//1                                             //
#define CRC_EN             0xA5                                                //
#define CRC_DIS            0x5A                                                //

#define CRC_STATE_CHECK    0xD2
#define CRC_TRIGER         0xD3//2                                             //

#define CRC_SIZE_WR        0xD5                                                //
#define CRC_SIZE_RD        0xD4//6                                             //

#define CRC_VALUE_READ     0xD6//8                                             //
#define CRC_ADDR_Read      0xD8//9                                             //

#define USER_INFO_START_ADDR	0x00000400
#define CFG_ENCPAGE0_ADDR		0x00000810
#define CFG_ENCPAGE1_ADDR		0x00000818	
   
#define PIN_DELAY(n) 


uint32_t checkemp_temp;
/*
 *  Send ISP Start Bit
 *    Parameters:      None
 *    Return Value:    None
 */
void isp_start_bit(void)
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
void isp_end_bit(void)
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
void isp_send(uint32_t data,uint32_t number)
{    
    uint32_t n;
    
	for (n=0; n<number; n++)
    {
		PIN_ISPCLK_CLR();
		PIN_DELAY(1);
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
uint8_t isp_recv_byte(void)
{
    uint8_t n,data = 0;
    
    PIN_ISPSDA_OUT_DISABLE();
    for (n=0; n<8; n++){
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
 *  Receive halfword by ISP
 *    Parameters:      None
 *    Return Value:    Date Read by ISP 
 */
uint32_t isp_recv_halfword(void)
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
uint32_t isp_recv_3rdbyte(void)
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
uint32_t isp_recv_word(void)
{
    uint8_t n;
    uint32_t data = 0;
    
    PIN_ISPSDA_OUT_DISABLE();
    for (n=0; n<32; n++){
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
 *  Unclock Chip and Get into ISP Mode
 *    Parameters:      None
 *    Return Value:    TRUE - Success, FALSE - Error
 */
uint8_t unclock_chip(void)              
{
    return 0;
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
 *  Set Programme Address
 *    Parameters:      address: Programme Address
 *    Return Value:    None
 */
void prog_addr_set(uint32_t address)    //prog_addr_set
{
	isp_start_bit();
	isp_send(SET_ADDR_CD, 8);
	isp_send(address,16);                    
    isp_end_bit();
}

uint32_t prog_addr_read(void)      //prog_addr_read
{
	uint32_t address;
    
	isp_start_bit();
	isp_send(READ_ADDR_CD, 8);
	address=isp_recv_halfword();                   
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
		delay_us(25);
		for(n=0;n<10;n++){
			isp_start_bit();
			isp_send(PROG_CHECK_CD,8);
			temp=isp_recv_byte();                    
			isp_end_bit();
			if(temp==PROG_CHECK_VAL){
				return TRUE;
			}
			delay_us(5);
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
    ErrorAddress = prog_addr_read(); 
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
uint32_t config_word_packet_read(void)
{
	uint32_t data;
    
	isp_start_bit();
	isp_send(READ_FLASH_PLUS_CD,8);
	data = isp_recv_word();                    
	isp_end_bit();	
    
	return data;
}

uint32_t ReadConfig1Word(void)
{
	uint32_t data;
	isp_start_bit();
	isp_send(READ_FLASH1_CD,8);
	data=isp_recv_word();                    
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
		delay_ms(1);
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
		temp=isp_recv_byte();
		isp_end_bit();
		if(temp==CRC_DIS){
			return TRUE;
		}
		delay_ms(1);
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
	isp_start_bit_M();
	isp_send_M(CRC_TRIGER,8);
	isp_send_M(mode,8);
	isp_end_bit_M();
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
		delay_ms(1);
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
		delay_ms(1);
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
    
	isp_start_bit_M();
	isp_send_M(CRC_STATE_CHECK,8);
	data = isp_recv_byte_M();                    
	isp_end_bit_M();	
    
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
	delay_ms(number);

    for(i=0;i<20;i++){
        isp_start_bit();
        isp_send(ERASE_CHECK_CD,8);
		temp = isp_recv_byte();
		isp_end_bit();
		if(temp == ERASE_OK_VAL){
			return TRUE;
		}
		delay_ms(1);
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
uint8_t erase_all(uint32_t eraseMode)
{
    uint32_t temp,chipID;
    
//	GPIO_ResetBits(GPIOB,YEL_LED_PIN);

    if(unclock_chip()==FALSE){                   
		return FALSE;
	}
	
    prog_area_set(INFOR_AREA_SELECT);
    chipID = config_word_read(CHIPID_ADD);
    if(chipID!=CHIPID_VALUE){
        //return CHIPIDERROR;
    }    
	
	if(eraseMode==0)					//全擦
	{	
		//------------------------------------------------Erase User Code Area
		
		isp_start_bit();
		isp_send(ERASE_CODE,8);
		isp_send(USER_AREA_VAL,8);	
		isp_send(0,8);			
		isp_end_bit();
		
		delay_ms(100);
		if(erase_check(10)==FALSE){
			return FALSE;
		}
	}
	else								//分页擦除
	{
		
		isp_start_bit();
		isp_send(ERASE_CODE,8);
		isp_send(USER_AREA_VAL,8);	
		isp_send(0,8);			
		isp_end_bit();
		
		delay_ms(100);
		if(erase_check(10)==FALSE){
			return FALSE;
		}		
		
	}
    
    //------------------------------------------------Erase Information Area 1(用户配置页)
    
    isp_start_bit();
    isp_send(ERASE_INF1,8);
    isp_send(INF1R_AREA_VAL,8);	
    isp_send(0,8);					
    isp_end_bit();
    
    delay_ms(100);    
    if(erase_check(5)==FALSE){
        return FALSE;
    }

    //------------------------------------------------Erase Information Area 2(加密)
    
	if(eraseMode==0)
	{
		isp_start_bit();
		isp_send(ERASE_INF2,8);
		isp_send(INF2R_AREA_VAL,8);	
		isp_send(0,8);					
		isp_end_bit();
		
		delay_ms(100);    
		if(erase_check(5)==FALSE){
			return FALSE;
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
    
    delay_ms(100);    
    if(erase_check(5)==FALSE){
        return FALSE;
    }
    
    isp_start_bit();
    isp_send(ERASE_INF3,8);
    isp_send(INF3R_AREA_VAL,8);
    isp_send(0,8);
    isp_end_bit();
    
    delay_ms(100);    
    if(erase_check(5)==FALSE){
        return FALSE;
    } 

    isp_start_bit();
    isp_send(0xB196,16);
    isp_end_bit();
    
    isp_start_bit();
    isp_send(0xB0,8);
    temp=isp_recv_byte();
    isp_end_bit();
    if(temp != 0X96)return FALSE;       
    
    #endif        

    return TRUE;
}

/*
 *  Check if Information Area is Empty
 *    Parameters:      None
 *    Return Value:    TRUE(1) - Success, UNCLOCKFAIL(0xA8) - Unclock Fault, CHECHEMPTYFAIL(0xAA) - Check Empty Fault
 */
uint32_t empty_check(void)
{
	uint32_t i;//,temp;

    if(unclock_chip()==FALSE){                
		return UNCLOCKFAIL;
	}	
    
	prog_area_set(CODE_AREA_SELECT);
    prog_addr_set(0);
	for(i=0;i<CODE_AREA_SIZE/8;i++){
		
		if((i%0x40) == 0){
            isp_start_bit();
			isp_send(STATUS_CHECK_CD,8);
			checkemp_temp=isp_recv_byte();
			isp_end_bit();
		}
		if((i%0x1000) == 0){}
		
		
		ErrorData=config_word_packet_read();//config_word_read(i*8);

		if(ErrorData!=0xFFFFFFFF){
			ErrorAddress = prog_addr_read() - 8;
			return CHECHEMPTYFAIL;
		}	

		ErrorData=ReadConfig1Word();

		if(ErrorData!=0xFFFFFFFF){
			ErrorAddress=prog_addr_read();
			return CHECHEMPTYFAIL;
		}
		
		if(checkemp_temp!=MODE_CHECK_VAL){
			return UNCLOCKFAIL;
		}
	}

	prog_area_set(INFOR_AREA_SELECT);
	prog_addr_set(USER_INFO_START_ADDR);
	for(i=0;i<=0x06;i++)
    {
		ErrorData = config_word_packet_read();
		if(ErrorData!=0xFFFFFFFF){
			ErrorAddress = prog_addr_read() - 8;
			return CHECHEMPTYFAIL;
		}
        
		ErrorData=ReadConfig1Word();
		if(ErrorData!=0xFFFFFFFF){
			ErrorAddress=prog_addr_read();
			return CHECHEMPTYFAIL;
		}        
	}
    
    prog_addr_set(0x000007C0);
	for(i=0;i<=0x06;i++)
    {
		ErrorData=ReadConfigWordOneByOne();
		if(ErrorData!=0xFFFFFFFF){
			ErrorAddress = prog_addr_read() - 8;
			return CHECHEMPTYFAIL;
		}
        
		ErrorData=ReadConfig1Word();
		if(ErrorData!=0xFFFFFFFF){
			ErrorAddress=prog_addr_read();
			return CHECHEMPTYFAIL;
		}        
	}
    
    prog_addr_set(0x00000800);
	for(i=0;i<=0x3;i++)
    {
		ErrorData=ReadConfigWordOneByOne();
		if(ErrorData!=0xFFFFFFFF){
			ErrorAddress = prog_addr_read() - 8;
			return CHECHEMPTYFAIL;
		}
        
		ErrorData=ReadConfig1Word();
		if(ErrorData!=0xFFFFFFFF){
			ErrorAddress=prog_addr_read();
			return CHECHEMPTYFAIL;
		}        
	}    
	
	return TRUE;
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
		ErrorAddress = ReadCRCAdd();
		ErrorData = config_word_read(ErrorAddress);			
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
	temp = crc_empty_check(CODE_AREA_SELECT, CODE_AREA_START, (CODE_AREA_SIZE/8-1));
	if(temp != TRUE){
		if(temp == UNCLOCKFAIL){
			return UNCLOCKFAIL;
		}
		return CHECHEMPTYFAIL;	
	}
	temp = crc_empty_check(INFOR_AREA_SELECT, CFG_AREA_START+0x200, 256/4);//(CFG_AREA_SIZE+3)/4);
	if(temp != TRUE){
		if(temp == UNCLOCKFAIL){
			return UNCLOCKFAIL;
		}
		return CHECHEMPTYFAIL;	
	}
//	temp = crc_empty_check(INFORAREASELECT, CFG_AREA_START+ENCRYPINFOADDR, (ENCRYPINFOSIZE+3)/4);
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
/*
 *  Programme Config Word
 *    Parameters:      None
 *    Return Value:    TRUE(1) - Success, UNCLOCKFAIL(0xB4) - Unclock Fault, PRGCONFIGFAIL(0xAC) - Programme Fault
 */
uint8_t ConfigProgramme(void)
{
	uint32_t i,temp;
    uint8_t retval;
    
//	FastReadOffLineFlash(Finalsector, 4, USBReceiveBuff.BReceiveBuff+12);	
//	FastReadOffLineFlash((Finalsector+4), 4, USBReceiveBuff.BReceiveBuff+16);	
//	temp=USBReceiveBuff.DReceiveBuff[4];
//	FastReadOffLineFlash((Finalsector+8), temp, USBReceiveBuff.BReceiveBuff+20);	
    
    //FastReadOffLineFlash((Finalsector+8), 1024, USBReceiveBuff.BReceiveBuff+20);
    FastReadOffLineFlash((Finalsector), 1024, USBReceiveBuff.BReceiveBuff+20);

    if(unclock_chip()==FALSE){                    
		return UNCLOCKFAIL;
	}
    
	prog_area_set(INFOR_AREA_SELECT);
    
    //-----------------------------------------------------------------------配置字编程（4个字）
            
	if(prog_config_word(USER_INFO_START_ADDR,USBReceiveBuff.DReceiveBuff[5])==FALSE){
		ErrorAddress=prog_addr_read();
		return PRGCONFIGFAIL;
	}

	for(i=1;i<=6;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[5+i*2])==FALSE){
			ErrorAddress=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	}   
    
	 ErrorAddress=prog_addr_read(); 
    
    //-----------------------------------------------------------------------校验和、UserID编程（6个字）    5+4=9
       
	if(prog_config_word(0x000007C0,USBReceiveBuff.DReceiveBuff[19])==FALSE){
		ErrorAddress=prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=6;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[19+i*2])==FALSE){
			ErrorAddress=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	} 

	ErrorAddress=prog_addr_read();    
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
    
	if(prog_config_word(0x00000000,USBReceiveBuff.DReceiveBuff[41])==FALSE){                   
		ErrorAddress=prog_addr_read();
		return PRGCONFIGFAIL;
	}
    
    ErrorAddress=prog_addr_read();

	if(prog_config_word(0x00000008,USBReceiveBuff.DReceiveBuff[43])==FALSE){                   
		ErrorAddress=prog_addr_read();
		return PRGCONFIGFAIL;
	}
    
	ErrorAddress=prog_addr_read();    

    //-----------------------------------------------------------------------
    
	if(prog_config_word(0x00000038,USBReceiveBuff.DReceiveBuff[45])==FALSE){                   
		ErrorAddress=prog_addr_read();
		return PRGCONFIGFAIL;
	}
	
	for(i=1;i<=9;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[45+i*2])==FALSE){
			ErrorAddress=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	}
    
	ErrorAddress=prog_addr_read();    
    
    //-----------------------------------------------------------------------
    
	if(prog_config_word(0x00000200,USBReceiveBuff.DReceiveBuff[65])==FALSE){           
		ErrorAddress=prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=40;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[65+i*2])==FALSE){
			ErrorAddress=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	} 
    
	ErrorAddress=prog_addr_read();    

    isp_start_bit();
    isp_send(0xB196,16);
    isp_end_bit();
    
    isp_start_bit();
    isp_send(0xB0,8);
    temp=isp_recv_byte();
    isp_end_bit();
    if(temp!=0X96)return FALSE;      
    
    #endif    

    //-----------------------------------------------------------------------
       
	//ErrorAddress=prog_addr_read();
	return TRUE;
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
		ErrorAddress=prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=3;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[5+i])==FALSE){
			ErrorAddress=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	}    
    
    //-----------------------------------------------------------------------校验和、UserID编程（6个字）    5+4=9
    
	if(prog_config_word(0x000003E8,USBReceiveBuff.DReceiveBuff[9])==FALSE){
		ErrorAddress=prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=5;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[9+i])==FALSE){
			ErrorAddress=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	} 

    //-----------------------------------------------------------------------机台信息页编程（97个字）      9+6+2=17（两个为加密字）

    #ifdef DEBUG_EN
    
    isp_start_bit();
    isp_send(ERASE_INF0_UNLOCK,16);
    isp_end_bit();
    
	if(prog_config_word(0x00000000,USBReceiveBuff.DReceiveBuff[17])==FALSE){                   
		ErrorAddress=prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=32;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[17+i])==FALSE){
			ErrorAddress=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	}
    
    //-----------------------------------//17+33=50(中间0x84~0xFF跳过)
    
	if(prog_config_word(0x00000100,USBReceiveBuff.DReceiveBuff[50])==FALSE){           
		ErrorAddress=prog_addr_read();
		return PRGCONFIGFAIL;
	}	
	for(i=1;i<=32;i++){
		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[50+i])==FALSE){
			ErrorAddress=prog_addr_read();
			return PRGCONFIGFAIL;
		}
	}    
    
    #endif    
	
	return TRUE;
}*/

//uint8_t ConfigProgramme_AUTO(uint32_t address,uint32_t data){
////	GPIO_SetBits(GPIOA,MRST_PIN);
////	delay_ms(10);
//  if(unclock_chip()==FALSE){
//		return UNCLOCKFAIL;
//	}
//	prog_area_set(INFORAREASELECT);	
//	SetProgrammeAdd(address);
//	info_buffer_write(data);	
//	if(ProgOperation(WORDPRGCD)==FALSE){
//		ErrorAddress=prog_addr_read();
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
//		ErrorAddress=prog_addr_read();
//		return HEXPRGFAIL;
//	}
//	for(i=1;i<number;i++){
//		if(prog_config_packet(*(point+i))==FALSE){
//			ErrorAddress=prog_addr_read();
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
uint8_t config_read(uint32_t address, uint32_t number, uint32_t *point)
{
	uint32_t i;
    
	prog_addr_set(address);	
	for (i=0;i<number;i++)
    {
		*(point+i*2) = config_word_packet_read();
        *(point+i*2+1) = ReadConfig1Word();
	} 
    
	return TRUE;
} 

uint8_t option_read(uint32_t address, uint32_t number, uint32_t *point)
{
    
    if(unclock_chip()==FALSE)
    {                   
		return UNCLOCKFAIL;
	}	
    
	prog_area_set(INFOR_AREA_SELECT);
    
	config_read(address, number, point);
    
	return TRUE;
}
 
uint8_t config_all_read(uint32_t address, uint32_t number, uint32_t *point)
{
	uint32_t temp;
    
    if(unclock_chip()==FALSE){                   
		return UNCLOCKFAIL;
	}	
    
	prog_area_set(INFOR_AREA_SELECT);
    
    if(address== 0x00000000)
    {
    
        config_read(USER_INFO_START_ADDR, 7, point);
        config_read(0x000007C0, 7, point + 14);        
        config_read(0x00000800, 4, point + 28); 

        if(number <= 144)return TRUE;
        
        //#ifdef DEBUG_EN   开放读

        isp_start_bit();
        isp_send(ERASE_INF0_UNLOCK,16);
        isp_end_bit();
        
        isp_start_bit();
        isp_send(0xB0,8);
        temp=isp_recv_byte();
        isp_end_bit();
        if(temp!=0X69)return FALSE;
        
        config_read(0x00000000, 2,  point + 36);
        config_read(0x00000038, 10, point + 40);
        config_read(0x00000200, 31, point + 60); 
        
        isp_start_bit();
        isp_send(0xB196,16);
        isp_end_bit();

        isp_start_bit();
        isp_send(0xB0,8);
        temp=isp_recv_byte();
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
        
        config_read(0x000002F8, 10, point);

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

/*
 *  Programme Calibration Word
 *    Parameters:      None
 *    Return Value:    TRUE(1) - Success, UNCLOCKFAIL(0xB4) - Unclock Fault, PRGCONFIGFAIL(0xAC) - Programme Fault
 */
//uint8_t CalProgramme(void){
//	uint32_t i,temp;	
//	
//	FastReadOffLineFlash(Finalsector+CALINFOADDR, 4, USBReceiveBuff.BReceiveBuff+12);	
//	FastReadOffLineFlash((Finalsector+CALINFOADDR+4), 4, USBReceiveBuff.BReceiveBuff+16);	
//	temp=USBReceiveBuff.DReceiveBuff[4];
//	if(temp > 0x200){
//			ErrorAddress = 0x100;
//			return PRGCONFIGFAIL;		
//	}
//	FastReadOffLineFlash((Finalsector+CALINFOADDR+8), temp, USBReceiveBuff.BReceiveBuff+20);		

//    if(unclock_chip()==FALSE){                    
//		return UNCLOCKFAIL;
//	}
//	prog_area_set(INFORAREASELECT);
//	if(prog_config_word(USBReceiveBuff.DReceiveBuff[3],USBReceiveBuff.DReceiveBuff[5])==FALSE){
//		ErrorAddress=prog_addr_read();
//		return PRGCONFIGFAIL;
//	}	
//	for(i=1;i<(USBReceiveBuff.DReceiveBuff[4]+3)/4;i++){
//		if(prog_config_packet(USBReceiveBuff.DReceiveBuff[5+i])==FALSE){
//			ErrorAddress=prog_addr_read();
//			return PRGCONFIGFAIL;
//		}
//	}
//	
//	return TRUE;
//}

uint8_t flash_programme(uint32_t address, uint32_t number, uint32_t *point)
{
  uint32_t i;	

	
	if(number%2 != 0){
		return FALSE;
	}
		
    if(unclock_chip()==FALSE){                   
		return UNCLOCKFAIL;
	}
	prog_area_set(CODE_AREA_SELECT);
	if(prog_flash_word(address,*point,*(point+1))==FALSE){
		ErrorAddress=prog_addr_read();
		return HEXPRGFAIL;
	}
	for(i=2;i<number;i+=2){
		if(PrgFlashWordOneByOne(*(point+i),*(point+i+1))==FALSE){
			ErrorAddress=prog_addr_read();
			return HEXPRGFAIL;
		}
	}	
	return TRUE;
}

uint8_t flash_read(uint32_t address, uint32_t number, uint32_t *point)
{
	uint32_t i;
//	GPIO_SetBits(GPIOA,MRST_PIN);
//	delay_ms(10);
	
	if(number%2 != 0){
		return FALSE;
	}


	prog_addr_set(address);
    
	for(i=0;i<number;i+=2){
		*(point+i)=ReadConfigWordOneByOne();
		*(point+i+1)=ReadConfig1Word();
	}	
	
	return TRUE;
}

/*
 *  Encryp Chip
 *    Parameters:      None
 *    Return Value:    TRUE(1) - Success, UNCLOCKFAIL(0xA8) - Unclock Fault, ENCRYPFAIL(0xAE) - Encryp Fault
 */
uint8_t EncrypChip(void)
{
    uint32_t temp;

    FastReadOffLineFlash((Finalsector+112), 32, USBReceiveBuff.BReceiveBuff+28);
    
    if(unclock_chip()==FALSE){                   
		return UNCLOCKFAIL;
	}
	
	prog_area_set(INFOR_AREA_SELECT);
    
    //-------------------------------------------------------------------

	if(prog_flash_word(0x0800,USBReceiveBuff.DReceiveBuff[7],USBReceiveBuff.DReceiveBuff[8])==FALSE){ 
		return ENCRYPFAIL;
	}

    if(prog_config_word(0x0810,USBReceiveBuff.DReceiveBuff[11])==FALSE){ 
		return ENCRYPFAIL;
	}  

    if(prog_config_word(0x0818,USBReceiveBuff.DReceiveBuff[13])==FALSE){ 
		return ENCRYPFAIL;
	}   

    //-------------------------------------------------------------------    
    
	if(config_word_read(0x0800)!=USBReceiveBuff.DReceiveBuff[7]){
		return ENCRYPFAIL;
	}
    
	if(config_word_read(0x0810)!=USBReceiveBuff.DReceiveBuff[11]){
		return ENCRYPFAIL;
	}    

	if(config_word_read(0x0818)!=USBReceiveBuff.DReceiveBuff[13]){
		return ENCRYPFAIL;
	}    
    
    isp_start_bit();
    isp_send(ENCRYPT_LOAD_CD,8);         //Load Encryption Words
    temp = ISPreceive24();	
    isp_end_bit();
    if(temp != ENCRYPTION_VAL){                  
        
        return ENCRYPFAIL;
    }
    
    return TRUE;	

}

uint8_t FULLFlashProgramme(void)
{    
	uint32_t temp,pagenum,n,i,j;//,finalpage
	uint32_t encrypword[2],encrypstart,encrypend,chipID;

    prog_area_set(INFOR_AREA_SELECT);
    chipID = config_word_read(CHIPID_ADD);
    if(chipID != CHIPID_VALUE){
        //return CHIPIDERROR;
    }  
    
	pagenum=CODE_AREA_SIZE/1024;
	
	temp=config_read(CFG_ENCPAGE0_ADDR, 1, encrypword);
	if(temp != TRUE){
		return temp;  
	}
	encrypstart = encrypword[0] & 0x7F;
	encrypend = (encrypword[0]>>8) & 0x7F;
	if(((encrypword[0]>>7)&0x1)&&((encrypword[0]>>15)&0x1)){
		encrypword[1] = 0;
	}
	else{
		if(encrypstart <= encrypend){
			encrypword[1] = 0x10;
		}
		else{
			encrypword[1] = 0x11;
		}
	}
	
	prog_area_set(CODE_AREA_SELECT);

	for(n=0; n<pagenum; n++){
		if((encrypword[1]==0)||                                           // no page encryption
			 ((encrypword[1]==0x10)&&((n<encrypstart)||(n>encrypend)))||    // middle page encryption and lay aside
		   ((encrypword[1]==0x11)&&((encrypstart>n>encrypend))))          // side page encryption and lay centrally
		{

			temp=FastReadOffLineFlash(1024*n, 1024, USBReceiveBuff.BReceiveBuff+20);
			if(temp!=TRUE){
				return FALSE;
			}
			j=FALSE;
			for(i=0; i<256; i++){
				if(USBReceiveBuff.DReceiveBuff[5+i]!=0xFFFFFFFF){				
					j=TRUE;
					break;	
				}							
			}
			if(j==TRUE){			
				if(SerialNumFlag==TRUE){
					if((SerialNumBuff[1]+3)/4==1){
						if((SerialNumBuff[0]>=256*n)&(SerialNumBuff[0]<256*(n+1))){
							USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*n)]=SerialNumBuff[2];	
							SerialNumFlag=FALSE;
						}
					}
					else if((SerialNumBuff[1]+3)/4==2){
						if((SerialNumBuff[0]>=256*n)&(SerialNumBuff[0]<256*(n+1)))
							USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*n)]=SerialNumBuff[2];	
						if(((SerialNumBuff[0]+1)>=256*n)&((SerialNumBuff[0]+1)<256*(n+1))){
							USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*n)+1]=SerialNumBuff[3];	
							SerialNumFlag=FALSE;
						}
					}
					else{
						SerialNumFlag=FALSE;
						return FALSE;
					}
				}

				
				temp=FlashProgramme(1024*n,256,USBReceiveBuff.DReceiveBuff+5);
				if(temp!=TRUE){				
					return FALSE;	
				}	
			}				
		}
	}
	
	return TRUE;	
}

uint8_t CodeAreaVerify(void){
	uint32_t pagenum,CheckSum,i,j,temp,SerialNumBuff_temp[2],CheckSumFlag;//,finalpage
	
    uint32_t chipID;  
	uint32_t encryPageWord[2],encryPageStart,encryPageEnd;

    if(unclock_chip()==FALSE){                   
		return FALSE;
	}
    
    prog_area_set(INFOR_AREA_SELECT);
    chipID = config_word_read(CHIPID_ADD);
    if(chipID!=CHIPID_VALUE){
        //return FALSE;
    }      
    
    pagenum=CODE_AREA_SIZE/1024;                                                       //Code area verify
	CheckSumFlag=FALSE;
	CheckSum=0;    
    
	temp=config_read(CFG_ENCPAGE0_ADDR, 1, encryPageWord);
	if(temp != TRUE){
		return temp;  
	}
	
	encryPageStart = encryPageWord[0] & 0x7F;
	encryPageEnd = (encryPageWord[0]>>8) & 0x7F;
	if(((encryPageWord[0]>>7)&0x1)&&((encryPageWord[0]>>15)&0x1)){
		encryPageWord[1] = 0;
	}
	else{
		if(encryPageStart <= encryPageEnd){
			encryPageWord[1] = 0x10;
		}
		else{
			encryPageWord[1] = 0x11;
		}
	}
	
	prog_area_set(CODE_AREA_SELECT);
	for(i=0; i<pagenum; i++){
		if((encryPageWord[1]==0)||                                           // no page encryption
			 ((encryPageWord[1]==0x10)&&((i<encryPageStart)||(i>encryPageEnd)))||    // middle page encryption and lay aside
		   ((encryPageWord[1]==0x11)&&((encryPageStart>i>encryPageEnd))))          // side page encryption and lay centrally
		{	

	//		GPIO_ToggleBits(GPIOC,YEL_LED_PIN);		
	//		FastReadOffLineFlash(i*1024, 1024, USBReceiveBuff.BReceiveBuff+20);
	//		flash_read(i*1024,256, USBReceiveBuff.DReceiveBuff+5+256);
			temp=FastReadOffLineFlash(i*1024, 1024, USBReceiveBuff.BReceiveBuff+20) & flash_read(i*1024,256, USBReceiveBuff.DReceiveBuff+5+256);
			if(temp!=TRUE){
				return FALSE;
			}			
			if(SerialNumFlag==TRUE){
				if((SerialNumBuff[1]+3)/4==1){
					if((SerialNumBuff[0]>=256*i)&(SerialNumBuff[0]<256*(i+1))){
						SerialNumBuff_temp[0]=USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*i)];	
						USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*i)]=SerialNumBuff[2];	
	//					USBReceiveBuff.DReceiveBuff[5+256+(SerialNumBuff[0]-256*i)]=SerialNumBuff[2];
						SerialNumFlag=FALSE;
						CheckSumFlag=TRUE;
					}
				}
				else if((SerialNumBuff[1]+3)/4==2){
					if((SerialNumBuff[0]>=256*i)&(SerialNumBuff[0]<256*(i+1))){
						SerialNumBuff_temp[0]=USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*i)];	
						USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*i)]=SerialNumBuff[2];	
	//					USBReceiveBuff.DReceiveBuff[5+256+(SerialNumBuff[0]-256*i)]=SerialNumBuff[2];
					}
					if(((SerialNumBuff[0]+1)>=256*i)&((SerialNumBuff[0]+1)<256*(i+1))){
						SerialNumBuff_temp[1]=USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*i)+1];						
						USBReceiveBuff.DReceiveBuff[5+(SerialNumBuff[0]-256*i)+1]=SerialNumBuff[3];
	//					USBReceiveBuff.DReceiveBuff[5+256+(SerialNumBuff[0]-256*i)+1]=SerialNumBuff[3];	
						SerialNumFlag=FALSE;
						CheckSumFlag=TRUE;
					}
				}
				else{
					SerialNumFlag=FALSE;
					return FALSE;
				}
			}				
			for(j=0;j<1024;j++){
				if((USBReceiveBuff.BReceiveBuff[j+20]) != (USBReceiveBuff.BReceiveBuff[j+20+1024])){
					ErrorAddress=i*1024+j;
					flash_read(i*1024+j, 2, USBReceiveBuff.DReceiveBuff+256);
					ErrorData=USBReceiveBuff.DReceiveBuff[256];
					return FALSE;											
				}
				CheckSum+=USBReceiveBuff.BReceiveBuff[j+20+1024];
			}
			if(CheckSumFlag==TRUE){
				if((SerialNumBuff[1]+3)/4==1){
					USBReceiveBuff.DReceiveBuff[5]=SerialNumBuff[2];
					USBReceiveBuff.DReceiveBuff[6]=SerialNumBuff_temp[0];		
					CheckSum-=(USBReceiveBuff.BReceiveBuff[20]+USBReceiveBuff.BReceiveBuff[21]+USBReceiveBuff.BReceiveBuff[22]+USBReceiveBuff.BReceiveBuff[23]
											-USBReceiveBuff.BReceiveBuff[24]-USBReceiveBuff.BReceiveBuff[25]-USBReceiveBuff.BReceiveBuff[26]-USBReceiveBuff.BReceiveBuff[27]);	
					CheckSumFlag=FALSE;
				}
				else if((SerialNumBuff[1]+3)/4==2){
					USBReceiveBuff.DReceiveBuff[5]=SerialNumBuff[2];
					USBReceiveBuff.DReceiveBuff[6]=SerialNumBuff[3];
					USBReceiveBuff.DReceiveBuff[7]=SerialNumBuff_temp[0];		
					USBReceiveBuff.DReceiveBuff[8]=SerialNumBuff_temp[1];		
					CheckSum-=(USBReceiveBuff.BReceiveBuff[20]+USBReceiveBuff.BReceiveBuff[21]+USBReceiveBuff.BReceiveBuff[22]+USBReceiveBuff.BReceiveBuff[23]
											+USBReceiveBuff.BReceiveBuff[24]+USBReceiveBuff.BReceiveBuff[25]+USBReceiveBuff.BReceiveBuff[26]+USBReceiveBuff.BReceiveBuff[27]
											-USBReceiveBuff.BReceiveBuff[28]-USBReceiveBuff.BReceiveBuff[29]-USBReceiveBuff.BReceiveBuff[30]-USBReceiveBuff.BReceiveBuff[31]
											-USBReceiveBuff.BReceiveBuff[32]-USBReceiveBuff.BReceiveBuff[33]-USBReceiveBuff.BReceiveBuff[34]-USBReceiveBuff.BReceiveBuff[35]);	
					CheckSumFlag=FALSE;
				}	
				else{
					return FALSE;
				}			
			}		
		}
	}

	FastReadOffLineFlash(ThirdFinalsector, 4, (USBReceiveBuff.BReceiveBuff+20));	
	CheckSum&=0x0000FFFF;
	USBReceiveBuff.DReceiveBuff[5]&=0x0000FFFF;
	if(CheckSum!=USBReceiveBuff.DReceiveBuff[5]){
		return HEXCHECKSUMFAIL;			
	}
	return TRUE;	
}

uint8_t InformationAreaVerify(void)
{
	uint32_t temp,CheckSum_1,CheckSum_2,j;
	
    CheckSum_1=0;
	CheckSum_2=0;

    FastReadOffLineFlash(Finalsector, 1024, USBReceiveBuff.BReceiveBuff+28);

    //-----------------------------------------------------------------------配置字读
    
    if(unclock_chip()==FALSE){                   
		return UNCLOCKFAIL;
	}	
    
	prog_area_set(INFOR_AREA_SELECT);    
    
    temp=config_read(USER_INFO_START_ADDR, 7, USBReceiveBuff.DReceiveBuff+7+256);
    if(temp!=TRUE){
        return FALSE;
    }

    //-----------------------------------------------------------------------校验和、UserID读   
    
    temp=config_read(0x000007C0, 7, USBReceiveBuff.DReceiveBuff+21+256);
    if(temp!=TRUE){
        return FALSE;
    }
    
    //-----------------------------------------------------------------------和下载的配置做比较    
    
    for(j=0;j<7;j++)
    {
        if((USBReceiveBuff.DReceiveBuff[j*2+7]) != (USBReceiveBuff.DReceiveBuff[j*2+7+256]))
        {
            return FALSE;										
        }
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+28];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+29];        
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+30];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+31];
        
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+28+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+29+1024];    
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+30+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+31+1024];       
    }
	
    for(j=9;j<14;j++)
    {
        if((USBReceiveBuff.DReceiveBuff[j*2+7]) != (USBReceiveBuff.DReceiveBuff[j*2+7+256]))
        {
            return FALSE;										
        }
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+28];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+29];        
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+30];
        CheckSum_1+=USBReceiveBuff.BReceiveBuff[j*8+31];
        
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+28+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+29+1024];    
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+30+1024];
        CheckSum_2+=USBReceiveBuff.BReceiveBuff[j*8+31+1024];       
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

uint8_t SerialNumVerify(void){
	if(SerialNumBuff[1]/4==1){
		flash_read(SerialNumBuff[0]*4, 1, USBReceiveBuff.DReceiveBuff+5);	
		if(USBReceiveBuff.DReceiveBuff[5]!=SerialNumBuff[2])
			return FALSE;										
	}
	else if(SerialNumBuff[1]/4==2){
		flash_read(SerialNumBuff[0]*4, 2, USBReceiveBuff.DReceiveBuff+5);	
		if((USBReceiveBuff.DReceiveBuff[5]!=SerialNumBuff[2])|(USBReceiveBuff.DReceiveBuff[6]!=SerialNumBuff[3]))
			return FALSE;									
	}
	else{
		return FALSE;
	}
	return TRUE;	
}

/*
 *  CRC Verify
 *    Parameters:      area, address, size
 *    Return Value:    UNCLOCKFAIL(0xA8) - Unclock Fault, FALSE - Error, crcvalue - CRC Result Value
 */
uint8_t CRCVerify(uint32_t area, uint32_t address, uint32_t size)
{
    if(unclock_chip()==FALSE)
    {                
		return UNCLOCKFAIL;
	}
	
	if(crc_init(area, address, size)==FALSE)
    {
		crc_disable();
		return FALSE;
	}
	
	crc_triger_set(0xCC);                 //Triger CRC Verify

	while(crc_state_check()&0x01);        //Wait for Finish

	CRCResult = crc_value_read();         //Read CRC Result Value
	
	if(crc_disable()!=TRUE)
    { 
		return FALSE;
	}
	
	return TRUE;
}

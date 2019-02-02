#include "eslink.h"
#include "M610_ISP.h"

#define PIN_DELAY(n)    ES_DELAY_SLOW(n)

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
static void isp_writeb(uint8_t data)
{
    uint32_t n;
    
	for (n=0; n<8; n++)
    {
        PIN_ISPCLK_CLR(); 
		if (data & 0x80)     
			PIN_ISPSDA_SET();
		else
			PIN_ISPSDA_CLR(); 
		PIN_DELAY(1);
		PIN_ISPCLK_SET();
		PIN_DELAY(1);
        
        data <<= 1;
	}
	PIN_ISPCLK_CLR(); 
	PIN_DELAY(1); 
    PIN_ISPSDA_CLR();    
}
 static uint8_t isp_readb(void)
{
    uint8_t n,data = 0;
    
    PIN_ISPSDA_OUT_DISABLE();
    for (n=0; n<8; n++)
    {
//        data <<= 1;
//        PIN_ISPCLK_SET();         
//        if(PIN_ISPSDA_IN())
//            data |= 0x01;
//        PIN_ISPCLK_CLR();
//        PIN_DELAY(1);
        PIN_ISPCLK_CLR();
        PIN_DELAY(1);
        data <<= 1;
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

void isp_send_bytes(uint8_t cmd, uint8_t *buf,  uint8_t size)
{
    uint8_t i;

    isp_start_bit();
    //写命令
    isp_writeb( cmd);
    //写数据
    for(i = 0; i < size; i++)
    {
        isp_writeb(*buf);
        buf++ ;
    }
    isp_end_bit();

}
void isp_rcv_bytes(uint8_t cmd, uint8_t *buf, uint32_t size)
{
    uint8_t i;

    isp_start_bit();
    //写命令
    isp_writeb( cmd);
    //读数据
    for(i = 0; i < size; i++)
    {
        *buf = isp_readb();
        buf++ ;
    }
    isp_end_bit();

}
/******************************************************************/
//模式选择
void mode_set(uint8_t mode)
{
    isp_start_bit();
    //写模式
    isp_writeb( mode);  
    isp_end_bit();     
}
//获取芯片状态   0xF0
uint8_t get_chip_status(void)
{
    uint8_t data;
    isp_rcv_bytes(STATUS_CHECK_CMD,&data, 1) ;
    return data;  
}
//area_set
//CODE_AREA/INFO_AREA
static void area_set(uint8_t area)
{
     isp_send_bytes(AREA_SET_CMD, &area, 1);    
}
//0xE3 设置缓冲器地址
static void addr_set(uint32_t addr)
{
    uint8_t byte[2];
    byte[0] = (addr & 0x0000FF00) >> 8;
    byte[1] = (addr & 0x000000FF) ;
    isp_send_bytes(ADDR_SET_CMD, byte, 2);   
}
//0xE4   设置数据缓冲器
static void data_write_set(uint32_t data)
{
    uint8_t byte[4];
    byte[0] = (data & 0xFF000000) >> 24;
    byte[1] = (data & 0x00FF0000) >> 16;
    byte[2] = (data & 0x0000FF00) >> 8;
    byte[3] = (data & 0x000000FF) ;
    isp_send_bytes(DATA_WRITE_CMD, byte, 4); 
    
} 
//获取编程状态   0xC5
uint8_t get_prog_status(void)
{
    uint8_t data;
    isp_rcv_bytes(PROG_CHECK_CMD,&data, 1) ;
    return data; 
}
//编程，
//CHIP_PROG_CMD : 编程后地址不变       0xC6
//PLUS_PROG_CMD : 地址+4 再编程        0xC7
//PROG_PLUS_CMD : 先编程，在地址+4     0xC8  
void prog_mode_set(uint8_t prg_cmd)
{
    uint8_t data = 0x50;
    isp_send_bytes(prg_cmd,&data, 1) ;      
} 
//芯片解锁
void unlock_chip(void)
{
    uint8_t i;
    uint8_t unlock_code[19] = {0x55, 0xAA, 0x49, 0x63, 0x68, 0x61, 0x69, 0x65, \
                            0x72, 0x20,0x43,0x6F,0x2C,0x4C, 0x74, 0x64, 0x2E,0xAA ,0x55} ;
    uint8_t code_len = sizeof(unlock_code);
    isp_start_bit();

    //写数据
    for(i = 0; i < code_len; i++)
    {
        isp_writeb(unlock_code[i]);
    }
    isp_end_bit();    
}
//read id
uint8_t id_check(void)
{
    uint8_t i;
    uint8_t byte[4] = {0x69, 0x96, 0x55, 0xFA} ;    

//    byte[3] = (ID_CHECK_CMD & 0xFF000000) >> 24;
//    byte[2] = (ID_CHECK_CMD & 0x00FF0000) >> 16;
//    byte[1] = (ID_CHECK_CMD & 0x0000FF00) >> 8;
//    byte[0] = (ID_CHECK_CMD & 0x000000FF) ;
    isp_start_bit();
    //写命令
    for(i = 0; i < 4; i++)
    {
        isp_writeb(byte[i]);
    }  
    //读数据
    for(i = 0; i < 4; i++)
    {
        byte[i] = isp_readb();
    }       
    isp_end_bit(); 
    for(i = 0; i < 4; i++)
    {
        if(byte[i] != ((ID_CHECK_VAL >> i*8) & 0xff ) )
             return FALSE;
    }         
    return TRUE;
}

//------------------------------------------------------------------------------
static uint8_t erase_and_check(uint8_t cmd ,uint8_t area,uint8_t delayms)
{
    uint8_t check_val ;
    uint32_t i;
    uint32_t wait;
    
     isp_send_bytes(cmd, &area, 1); 
     //延时，等待擦除完成
     es_delay_ms(delayms);
    wait = 1000;
    for( i = 0; i < wait; i++)
    {
        isp_rcv_bytes(ERASE_CHECK_CMD,&check_val, 1);
        if(check_val == ERASE_OK)
            break;
        es_delay_ms(1);
    }  
    if(i >= wait)
        return FALSE;
    return TRUE; 

}

/*******************************************************************************
*函数名：program_and_check
* 描述 ：字节编程。flash编程时间约为30us
* 输入 ：mode:编程模式 CHIP_PROG_CMD/PLUS_PROG_CMD 。 addr：地址。data：数据。
* 输出 ：
*******************************************************************************/
static uint8_t program_and_check(uint8_t mode)
{
    uint8_t i ;
    uint8_t wait ;
    uint8_t check_val = 0;
    
    //编程模式
    prog_mode_set( mode);
    //编程完成判断
    wait = 200; 
    es_delay_us(30);
    for(i=0; i < wait ; i++)
    {
         check_val = get_prog_status();
        if(PROG_OK == check_val)
            break;
        else if(PROG_FAIL==check_val)
            break; 
        es_delay_us(1);
    }
    if (PROG_OK != check_val) 
        return FALSE;  
    return TRUE;  

}
/*******************************************************************************
*函数名：program_word
* 描述 ：字节编程。flash编程时间约为30us
* 输入 ：mode:编程模式 CHIP_PROG_CMD/PLUS_PROG_CMD 。 addr：地址。data：数据。
* 输出 ：
*******************************************************************************/
static uint8_t program_data(uint32_t addr, uint32_t *data, uint32_t size, uint32_t *failed_offset) 
{
    uint32_t i;
    
    //设置地址缓冲器
     addr_set(addr);          
    //设置数据缓冲器
     data_write_set(*data);
     //编程并判断
    if(program_and_check(CHIP_PROG_CMD) != TRUE)    
       return FALSE;   

    data++;
//    size--;
//    while(size)
//    {
//         //设置数据缓冲器
//         data_write_set(*data);
//         //编程并判断
//        if(program_and_check(PLUS_PROG_CMD) != TRUE)    
//           return FALSE;     
//        data++;
//        size--;       
//    } 
    for(i=1; i<size; i++)
    {
         //设置数据缓冲器
         data_write_set(*data);
         //编程并判断
        if(program_and_check(PLUS_PROG_CMD) != TRUE)    
        {
             if(failed_offset)
                *failed_offset = addr + i*4;
            return FALSE; 
        }
               
        data++;        
    }
    return TRUE;  
}
/*******************************************************************************
*函数名：read_data
* 描述 ：读数据
* 输入 ：
* 输出 ：
*******************************************************************************/
static uint8_t read_data(uint32_t addr, uint32_t *data, uint32_t size) 
{
     uint32_t i;
     uint8_t tmp_data[4];

    //设置地址缓冲器
    addr_set(addr);  
    for( i=0; i<size; i++)
    {
         isp_rcv_bytes(FLASH_READ_PLUS_CMD, tmp_data, 4);   
         *data = (  (tmp_data[0] << 24) |
                    (tmp_data[1] << 16) | 
                    (tmp_data[2] << 8)  |
                    (tmp_data[3] )  )  ;
         data++;               
    } 
    return TRUE;
}

/*******************************************************************************
*函数名：isp_target_program_code
* 描述 ：isp编程。flash编程时间约为30us
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：
*******************************************************************************/
uint8_t isp_program_code(uint32_t addr, uint32_t *data, uint32_t size, uint32_t *failed_offset) 
{
    uint8_t i;
    uint8_t retry;
    
    area_set(CODE_AREA);
    retry = 10;    
    for(i=0; i<retry; i++)
    {
        if(program_data(addr, data, size, failed_offset) != FALSE)
            break;
    }
    if(i >= retry)
        return FALSE;
        
    return TRUE;        
}
/*******************************************************************************
*函数名：
* 描述 ：
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：
*******************************************************************************/
uint8_t isp_read_code(uint32_t addr, uint32_t *data, uint32_t size) 
{
     area_set(CODE_AREA);
     read_data(addr, data, size);
     return TRUE; 
}
/*******************************************************************************
*函数名：
* 描述 ：
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：
*******************************************************************************/
uint8_t isp_program_config(uint32_t addr, uint32_t *data, uint32_t size,uint32_t *failed_offset) 
{
    uint8_t i;
    uint8_t retry;
    
    area_set(INFO_AREA);
    retry = 10;    
    for(i=0; i<retry; i++)
    {
        if(program_data(addr, data, size, failed_offset) != FALSE)
            break;
    }
    if(i >= retry)
        return FALSE;
    return TRUE; 

}  
uint8_t isp_read_config(uint32_t addr, uint32_t *data, uint32_t size) 
{
     area_set(INFO_AREA);
     read_data(addr, data, size);
     return TRUE; 
}

/*******************************************************************************
*函数名：
* 描述 ：擦除程序区和配置字第0页 
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：
*******************************************************************************/
uint8_t isp_erase_chip(void)
{
    uint8_t result;  
	
    result = erase_and_check(ERASE_M0,FLASH_MAIN_AREA,20);    //C1 + A0
    if(result != TRUE)
        return FALSE;
    //erase option2 time about 5ms
    result = erase_and_check(ERASE_M0,FLASH_INTO0_AREA,10);    //C1 + A1
    if(result != TRUE)
        return FALSE;    
    //更新加密字状态
    get_chip_status();
    return TRUE;
}

uint8_t isp_erase_info1(void)
{
	uint8_t result;  
	
    result = erase_and_check(ERASE_M1,FLASH_INTO1_AREA,20);  
    if(result != TRUE)
        return FALSE;
  
    return TRUE;
}

//ISP复位
void isp_reset(void)
{
    PIN_ISPCLK_CLR();                                      
    PIN_ISPSDA_CLR(); 
    es_delay_ms(10);
    isp_start_bit();
    es_delay_ms(20);
}

//读芯片ID
uint8_t isp_id_check(void)
{
    uint8_t ret;
    uint8_t i;
    
    //读取ID
    for(i=0; i<100; i++)
    {
        ret = id_check();
        if(ret == TRUE)
            break;
    }
    if(ret != TRUE)
        return FALSE;
    return TRUE;
}
//解锁
uint8_t isp_unlock_check(void)
{
//    uint8_t ret;
    uint8_t check_val;
    uint32_t i;     
    
    //解锁   
    for(i=0; i<10; i++)
    {
        isp_start_bit();
        es_delay_ms(30);
        
        unlock_chip();
        check_val = get_chip_status() & 0xF0;
        if(UNLOCK_CHECK_VAL == check_val)
            break;
    }
    if(UNLOCK_CHECK_VAL != check_val)
        return FALSE;   
    return TRUE;
}

//isp 模式设置
 uint8_t isp_mode_set(void)
{
//    uint8_t ret;
    uint8_t check_val;
    uint32_t i;

    //isp模式设置
    for(i=0; i<3; i++)
    {
        mode_set(ISP_MODE_CMD);
        check_val = get_chip_status();
        if(ISP_MODE_CHECK_VAL ==  check_val)  
            break;    
    }
    if(ISP_MODE_CHECK_VAL !=  check_val) 
        return FALSE;
    return TRUE;
}
//isp模式检测
uint8_t isp_mode_check(void)
{
//    uint8_t ret;
    uint8_t check_val;
    uint32_t i;  
    
    for(i=0; i<3; i++)
    {     
        isp_rcv_bytes(STATUS_CHECK_CMD,&check_val, 1) ;      
        check_val = get_chip_status();
        if(ISP_MODE_CHECK_VAL ==  check_val)  
            break;    
    }
    if(ISP_MODE_CHECK_VAL !=  check_val) 
        return FALSE;
    return TRUE;

}


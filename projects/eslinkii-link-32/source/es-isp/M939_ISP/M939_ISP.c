#include "eslink.h"
#include "M939_ISP.h"


#define PIN_DELAY(n)    ES_DELAY_SLOW(n)
//#define PIN_DELAY(n)    ES_DELAY_FAST(n)
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
/***********************************编程解锁命令********************************/
//芯片解锁
void unlock_chip(void)
{
    uint8_t i;
    uint8_t unlock_code[4] = {0x55,0x45,0x53,0xAA};     //0x55+"ES"+AA     
    isp_start_bit();

    //写数据
    for(i = 0; i < 4; i++)
    {
        isp_writeb(unlock_code[i]);
    }
    isp_end_bit();    
}

//获取芯片状态   0xF0
uint8_t get_chip_status(void)
{
    uint8_t data;
    isp_rcv_bytes(STATUS_CHECK_CMD,&data, 1) ;
    return data;  
}
//read id
#define ID_CHECK_CMD                0x699655FA 
#define ID_CHECK_VAL                0x4D93914F 

uint8_t id_check(void)
{
    uint8_t i;
    uint8_t byte[4] = {0x69, 0x96, 0x55, 0xFA} ; 
    uint8_t id_data[4] = {0x4D, 0x93, 0x91, 0x4F};

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
        if(byte[i] !=  id_data[i])
             return FALSE;
    }         
    return TRUE;
}
//模式选择
//mode；ISP_MODE_CMD
void mode_set(uint8_t mode)
{
    isp_start_bit();
    //写模式
    isp_writeb( mode);  
    isp_end_bit();     
}


/***********************************接口访问命令********************************/
//area_set
//CODE_AREA/INFO_AREA
static void area_set(uint8_t area)
{
     isp_send_bytes(AREA_SET_CMD, &area, 1);    
}
//0xE3 设置缓冲器地址
static void addr_set(uint32_t addr)
{
    uint8_t byte[4];
    byte[0] = 0x00;
    byte[1] = (addr & 0x00FF0000) >> 16;
    byte[2] = (addr & 0x0000FF00) >> 8;
    byte[3] = (addr & 0x000000FF) ;
    isp_send_bytes(ADDR_SET_CMD, byte, 4);   
}
//0xE4   设置数据缓冲器
//data0：flash 低32位  data1：flash 高2位
static void data_write_set(uint32_t data0, uint32_t data1)
{
    uint8_t byte[4];
    
    byte[0] = (data0 & 0xFF000000) >> 24;
    byte[1] = (data0 & 0x00FF0000) >> 16;
    byte[2] = (data0 & 0x0000FF00) >> 8;
    byte[3] = (data0 & 0x000000FF) ;
    isp_send_bytes(DATA0_WRITE_CMD, byte, 4); 
    
    byte[0] = (data1 & 0xFF000000) >> 24;
    byte[1] = (data1 & 0x00FF0000) >> 16;
    byte[2] = (data1 & 0x0000FF00) >> 8;
    byte[3] = (data1 & 0x000000FF) ;
    isp_send_bytes(DATA1_WRITE_CMD, byte, 4);          
} 
 
//获取编程状态   0xC5
uint8_t get_prog_status(void)
{
    uint8_t data;
    isp_rcv_bytes(PROG_CHECK_CMD,&data, 1) ;
    return data; 
}
//编程命令 
void prog_mode_set(uint8_t prg_cmd)
{
    uint8_t data = 0x50;
    isp_send_bytes(prg_cmd,&data, 1) ;      
}   

/***********************************ISP模式命令********************************/
//info0解锁
static void info0_unlock_set(void)
{
    uint8_t data = 0x69;
    isp_send_bytes(UNLOCK_SET_CMD,&data, 1) ;    

}
//擦除检测命令
static uint8_t erase_and_check(uint8_t cmd ,uint8_t area,uint8_t delayms)
{
    uint8_t check_val ;
    uint8_t data[2] = {0x00, 0x00};
    uint32_t i;
    uint32_t wait;
    
    data[0] =  area;
    isp_send_bytes(cmd, data, 2); 
    //延时，等待擦除完成
    es_delay_ms(delayms);
    wait = 50;
    for( i = 0; i < wait; i++)
    {
        isp_rcv_bytes(ERASE_CHECK_CMD,&check_val, 1);
        if(check_val == ERASE_OK_VAL)
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
    wait = 30; 
    es_delay_us(30);
    for(i=0; i < wait ; i++)
    {
        check_val = get_prog_status();
        if(PROG_OK_VAL == check_val)
            break;
        else if(PROG_FAIL_VAL == check_val)
            break; 
        es_delay_us(1);
    }
    if (PROG_OK_VAL != check_val) 
        return FALSE;  
    return TRUE;  
}
 

/***********************************ISP操作流程********************************/
 
/*******************************************************************************
*函数名：code_program
* 描述 ：
* 输入 ：mode:编程模式 CHIP_PROG_CMD/PLUS_PROG_CMD 。 addr：地址。data：数据。
* 输出 ：
*******************************************************************************/
static uint8_t code_program(uint32_t addr, uint32_t *data, uint32_t size, uint32_t *failed_offset) 
{
    uint32_t i;
        
    //设置地址缓冲器
     addr_set(addr);      
    //设置数据缓冲器
     data_write_set(*data, *(data+1));
     
     //编程并判断
    if(program_and_check(PROG_CMD) != TRUE)    
       return FALSE;         

    for(i=2; i<size; i+=2)
    {
        data += 2;
         //设置数据缓冲器
         data_write_set(*data, *(data+1));
         //编程并判断
        if(program_and_check(PLUS_PROG_CMD) != TRUE)    
        {
             *failed_offset = i;
            return FALSE; 
        } 
    }
    return TRUE;  
}

//info区编程
//仅支持从0x10对齐的地址开始编程
static uint8_t info_program(uint32_t addr, uint32_t *data, uint32_t size, uint32_t *failed_offset) 
{
    uint32_t i;
    uint32_t read_size;
    uint32_t remain_size;   
    uint32_t remain_data;
    
    if(addr & 0x08)         //需要从0x10地址对齐的地方开始编程
          return FALSE;
    //设置地址缓冲器
    addr_set(addr); 
    
    remain_size = size % 2;
    read_size = size - remain_size;     
    
    //设置数据缓冲器
    data_write_set(*data, *(data+1));
     
    //编程并判断
    if(program_and_check(PROG_CMD) != TRUE)    
       return FALSE;   

    for(i=2; i<read_size; i+=2)
    {
        data += 2;
        //设置数据缓冲器
        data_write_set(*data, *(data+1));
         //编程并判断
        if(program_and_check(PLUS_PROG_CMD) != TRUE)    
        {
             *failed_offset = i;
            return FALSE; 
        }  
    }
    
    data += 2;
    if( remain_size)
    {
        remain_data = 0xffffffff;       //填充0xff
        //设置数据缓冲器
        data_write_set(*data, remain_data);
         //编程并判断
        if(program_and_check(PLUS_PROG_CMD) != TRUE)    
        {
             *failed_offset = i;
            return FALSE; 
        }      
    
    }
    return TRUE;  
}

/*******************************************************************************
*函数名：read_data
* 描述 ：flash/info数据读取
* 输入 ：
* 输出 ：
*******************************************************************************/
static uint8_t code_read(uint32_t addr, uint32_t *data, uint32_t size) 
{
     uint32_t i;
     uint8_t tmp_data[4];

    //设置地址缓冲器
    addr_set(addr);  
    
    for( i=0; i<size; i+=2)
    {
         isp_rcv_bytes(FLASH_READ_PLUS_CMD, tmp_data, 4);   
         *data = (  (tmp_data[0] << 24) |
                    (tmp_data[1] << 16) | 
                    (tmp_data[2] << 8)  |
                    (tmp_data[3] )  )  ;
         data++;    
         isp_rcv_bytes(FLASH_READ1_CMD, tmp_data, 4);   
         *data = (  (tmp_data[0] << 24) |
                    (tmp_data[1] << 16) | 
                    (tmp_data[2] << 8)  |
                    (tmp_data[3] )  )  ;
         data++;    
    } 
    return TRUE;
}
//读info区
//注意：只支持从0地址开始读的情况
static uint8_t info_read(uint32_t addr, uint32_t *data, uint32_t size) 
{
    uint32_t i;
    uint8_t tmp_data[4];
    uint32_t read_size;
    uint32_t remain_size;    //剩下的    
    
    //设置地址缓冲器
    addr_set(addr);  
    remain_size = size % 2;
    read_size = size - remain_size;      
    if(read_size)
    {    
        if(addr & 0x08)         //读双字节需要从0x10地址对齐的地方开始读
            return FALSE;
        for( i=0; i<read_size; i += 2)
        {
             isp_rcv_bytes(FLASH_READ_PLUS_CMD, tmp_data, 4);   
             *data = (  (tmp_data[0] << 24) |
                        (tmp_data[1] << 16) | 
                        (tmp_data[2] << 8)  |
                        (tmp_data[3] )  )  ;
             data++;    
             isp_rcv_bytes(FLASH_READ1_CMD, tmp_data, 4);   
             *data = (  (tmp_data[0] << 24) |
                        (tmp_data[1] << 16) | 
                        (tmp_data[2] << 8)  |
                        (tmp_data[3] )  )  ;
             data++;    
        }   
    }       
    remain_size = size % 2;
    if( remain_size)
    {
        isp_rcv_bytes(FLASH_READ_PLUS_CMD, tmp_data, 4);   
        *data = (  (tmp_data[0] << 24) |
                    (tmp_data[1] << 16) | 
                    (tmp_data[2] << 8)  |
                    (tmp_data[3] )  )  ;     
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
    
    //双字节编程
    if(size &0x01)
        return FALSE;
    area_set(CODE_AREA_VAL);
    retry = 10;    
    for(i=0; i<retry; i++)
    {
        if(code_program(addr, data, size, failed_offset) != FALSE)
            break;
    }
    if(i >= retry)
        return FALSE;
        
    return TRUE;        
}
/*******************************************************************************
*函数名：isp_read_code
* 描述 ：读取code区数据
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：TRUE/FALSE
*******************************************************************************/
uint8_t isp_read_code(uint32_t addr, uint32_t *data, uint32_t size) 
{
        //双字节编程
    if(size &0x01)
        return FALSE;
    area_set(CODE_AREA_VAL);
    code_read(addr, data, size);
    return TRUE; 
}
/*******************************************************************************
*函数名：isp_program_config 
* 描述 ：配置区编程
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：TRUE/FALSE
*******************************************************************************/
uint8_t isp_program_config(uint32_t addr, uint32_t *data, uint32_t size,uint32_t *failed_offset) 
{
    uint8_t i;
    uint8_t retry;
    
    area_set(INFO_AREA_VAL);
    retry = 10;    
    for(i=0; i<retry; i++)
    {
        if(info_program(addr, data, size, failed_offset) != FALSE)
            break;
    }
    if(i >= retry)
        return FALSE;
    return TRUE; 

}  
/*******************************************************************************
*函数名：isp_read_config
* 描述 ：读取配置区数据
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：TRUE/FALSE
*******************************************************************************/
uint8_t isp_read_config(uint32_t addr, uint32_t *data, uint32_t size) 
{    
    area_set(INFO_AREA_VAL);
    info_read(addr, data, size);
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
    //擦除code区
    result = erase_and_check(ERASE_M0_CMD, FLASH_MAIN_AREA, 10);
    if(result != TRUE)
        return FALSE;

    //擦除info1 
    result = erase_and_check(ERASE_M2_CMD, INFO_1_AREA, 2);
    if(result != TRUE)
        return FALSE; 

    //擦除info2    
    result = erase_and_check(ERASE_M3_CMD, INFO_2_AREA, 2);
    if(result != TRUE)
        return FALSE;         
#if 0
    //擦除info0 
    info0_unlock_set();
    result = erase_and_check(ERASE_M1_CMD, INFO_0_AREA, 2);
    if(result != TRUE)
        return FALSE;  
 
    //擦除info3
    result = erase_and_check(ERASE_M4_CMD, INFO_3_AREA, 2);
    if(result != TRUE)
        return FALSE; 
#endif        
    return TRUE;
}

//isp复位
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
//解锁并判断是否解锁成功
uint8_t isp_unlock_check(void)
{
//    uint8_t ret;
    uint8_t check_val;
    uint32_t i;     
    
    //解锁   
    for(i=0; i<10; i++)
    {
        unlock_chip();
        check_val = get_chip_status() & 0xF0;
        if(UNLOCK_CHECK_VAL == check_val)
            break;
    }
    if(UNLOCK_CHECK_VAL != check_val)
        return FALSE;   
    return TRUE;
}
//isp 加密字加载并判断
uint8_t isp_encrypt_check(void)
{
    uint8_t i;
    uint8_t data[3] = {0x00};
    
    isp_rcv_bytes(ENCRYPT_CHECK_CMD,data, 3) ;
    
    for(i=0; i<3; i++)
    {
        if(data[i] != 0xA5)
           return FALSE;
    }         
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




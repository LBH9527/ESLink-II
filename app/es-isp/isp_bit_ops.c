#include "eslink.h"
#include "isp_def.h"

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
	PIN_ISPSDA_CLR(); 
	PIN_DELAY(1);  
}
 static uint8_t isp_readb(void)
{
    uint8_t n,data = 0;
    
    PIN_ISPSDA_OUT_DISABLE();
    for (n=0; n<8; n++)
    {
        data <<= 1;
        PIN_ISPCLK_SET();         
        if(PIN_ISPSDA_IN())
            data |= 0x01;
        PIN_ISPCLK_CLR();
        PIN_DELAY(1);
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
void isp_rcv_bytes(uint8_t cmd, uint8_t *buf, uint8_t size)
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
/**********************************编程解锁命令********************************/
//unlock 芯片解锁
void isp_unlock(void)
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
//status check
int isp_status_check(void)
{
    uint8_t status_data;
    isp_rcv_bytes( STATUS_CHECK_CMD , &status_data, 1);
    if(status_data != 0xA1)
        return -1 ;
    return 0; 
}
//id check
int isp_id_check(void)
{
    uint8_t i;
    uint8_t cmd[4] = {0x69, 0x96, 0x55, 0xFA};
    uint8_t id_data[4] = {0x4D,0x93,0x91,0x4F};
    uint8_t data;
    
    isp_start_bit();
    //写命令
    for(i = 0; i < 4; i++)
    {
        isp_writeb(cmd[i]);
    }  
    //读数据
    for(i = 0; i < 4; i++)
    {
        data = isp_readb();
        if(data != id_data[i])
             break;
    }       
    isp_end_bit(); 
    if(i < 4)
        return -1;
    return 0;
}
//模式选择
void isp_mode_set(uint8_t mode)
{
    isp_start_bit();
    //写模式
    isp_writeb( mode);  
    isp_end_bit();     
}
//-----------------------------接口访问-----------------------------------------
//area_set
void isp_area_set(uint8_t area)
{
     isp_send_bytes(AREA_SET_CMD, &area, 1);    
}
//地址缓存区读取，判断地址是否合法
uint32_t isp_addr_check(void)
{    
    uint32_t addr;
    uint8_t byte[4];  
    
    isp_rcv_bytes(ADDR_CHECK_CMD, byte, 4);
    addr =  (byte[3] << 24) | (byte[2] << 16) | (byte[1] << 8) | byte[0]  ;
    return addr;    
}
//0xE3 设置地址
void isp_addr_set(uint32_t addr)
{
    uint8_t byte[4];
    byte[0] = 0 ;
    byte[1] = (addr & 0x00FF0000) >> 16;
    byte[2] = (addr & 0x0000FF00) >> 8;
    byte[3] = (addr & 0x000000FF) ;
    isp_send_bytes(ADDR_SET_CMD, byte, 4);   
}
//0xE5 0xE7   设置数据缓冲器
void isp_data_set(uint32_t data0,uint32_t data1)
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
//flash数据读取bit0-31,读完地址保持不变
uint32_t isp_flash_read(void)
{
    uint32_t data;
    uint8_t byte[4];
    isp_rcv_bytes(FLASH_READ_CMD, byte, 4);
    data =  (byte[3] << 24) | (byte[2] << 16) | (byte[1] << 8) | byte[0]  ;
    return data;     
}
//flash数据读取bit0-31,读完地址+8
uint32_t isp_flash_read_plus(void)
{
    uint32_t data;
    uint8_t byte[4];
    isp_rcv_bytes(FLASH_READ_PLUS_CMD, byte, 4);
    data =  (byte[3] << 24) | (byte[2] << 16) | (byte[1] << 8) | byte[0]  ;
    return data; 
}
//flash数据读取bit32-63
uint32_t isp_flash_read1(void)
{
    uint32_t data;
    uint8_t byte[4];
    isp_rcv_bytes(FLASH_READ1_CMD, byte, 4);
    data =  (byte[3] << 24) | (byte[2] << 16) | (byte[1] << 8) | byte[0]  ;
    return data;     
}  
//获取编程状态   0xC8
uint8_t isp_prog_check(void)
{
    uint8_t data;
    isp_rcv_bytes(PROG_CHECK_CMD,&data, 1) ;
    return data; 
}
//编程，
//PRG_CMD : 值为地址缓存区中的值   0xC9
//PLUS_PRG_CMD : 地址+8 再编程0xCA
//PRG_PLUS_CMD : 先编程，在地址+8 0xCB  
void isp_prog(uint8_t prg_cmd)
{
    uint8_t data = 0x50;
    isp_send_bytes(PRG_CMD,&data, 1) ;      
}      


//code区擦除
void erase_code_area(void)
{
    uint8_t data[2] = {0xF0, 0x00};
    isp_send_bytes(ERASE_CODE_CMD, data, sizeof(data));   
}
//info区解锁
void unlock_info_area(void)
{
    uint8_t data = 0x69;
    isp_send_bytes(ERASE_CODE_CMD, &data, 1);   
}
//锁定info区
void lock_info_area(void)
{
    uint8_t data = 0x96;
    isp_send_bytes(ERASE_CODE_CMD, &data, 1);   
}
// void lock_info_area_check(void)
//{
//    isp_send_bytes
//}
//擦除info0区
void erase_info_area(void)
{
    uint8_t data[2] = {0xe1, 0x00};
    isp_send_bytes(ERASE_INF0_CMD, data, sizeof(data));  
}


//void empty_check()
//{

//     isp_send_bytes(AREA_SET_CD,CODE_AREA_SELECT,

//}
////擦除完成检测
//int erase_finish_check(uint32_t delay)
//{
//    uint8_t data ;
//    uint8_t i;

//    for( i = 0; i < 20; i++)
//    {
//        isp_rcv_bytes(ERASE_CHECK_CD,data, 1)
//        if(data == ERASE_OK_VAL)
//            return 0;
//        isp_delay_ms(1);
//    }
//    return -1;

//}
//芯片解锁
void unlock_chip(void)
{
    
    
}






//------------------------------------------------------------------------------
//area: CODE_AREA 或  INFO_AREA
void isp_read_word(uint8_t area, uint32_t addr,  uint32_t *buf, uint32_t size)
{
//    uint8_t i;
//    
//    isp_area_set(CODE_AREA);  //   
//    isp_addr_set(addr);
//    
//	for (i=0; i<size; )
//    {
//		*(buf+i) = isp_flash_read_plus();
//        *(buf+i+1) = isp_flash_read1();
//        i += 2;
//	}       
    
}
//isp 编程流程
void isp_prog_word(uint32_t addr, uint32_t *buf, uint32_t size) 
{
//    uint8_t check_val = 0;
//    uint8_t retry = 30;
//    
//    if(size%2 != 0)
//		return FALSE;
//	
//    //设置地址缓冲器
//     isp_addr_set(addr);
//    
//    //设置数据缓冲器
//     isp_data_set(buf);
//    //编程
//    isp_prog( PRG_CMD);
//    //编程完成判断
//    retry = 30; 
//    do{
//        es_delay_us(30);
//        check_val = isp_prog_check();
//    } while ((check_val != PROG_CHECK_VAL) && retry-- );
//    if (check_val != PROG_CHECK_VAL)   
}
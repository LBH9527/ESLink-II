#include <stdbool.h>        //for bool 
#include "eslink.h"
#include "bootisp_host.h"
#include "bootisp_target_config.h"
#include "uart.h"

#define ERR_BOOTISP_OK      0
#define ERR_BOOTISP_FAIL    1


static void bootisp_delayus(uint32_t n)
{
    es_delay_us(n);
}

//异或校验
static uint8_t check_xor(uint8_t *data, uint8_t size)
{
	uint8_t Xor = 0;
    uint8_t i;
    
	if (size == 1)
	{
		Xor = data[0] ^ 0xff;
		return  Xor;
	}
	for ( i = 0; i < size; i++)
	{
		Xor ^= data[i];
	}
    
	return Xor;
}

//bootisp 写数据 
static bool write_data(uint8_t *wr_data, uint8_t wr_size)
{
    uint32_t timeout = 0;
    uint8_t len_data;
    
    timeout = wr_size*0xffff;
    while(wr_size > 0){          
        len_data = uart_write_data(wr_data, wr_size);
        if(wr_size >= len_data)
            wr_size -= len_data;     
        timeout--;
        bootisp_delayus(10);
        if(timeout == 0)
            return false;
    }
    return true;

} 

//读数据
static bool read_data(uint8_t *rd_data, uint8_t rd_size)
{
    uint32_t timeout = 0;
    uint8_t len_data;
    
    timeout = rd_size*0xffff;
    while(rd_size > 0){
        len_data = uart_read_data(rd_data, rd_size);
        rd_size -= len_data;
        timeout--;
        bootisp_delayus(10);
        if(timeout == 0)
            return false;
    } 
    return true;      
}  
//写数据和校验码    
static bool write_data_oxr(uint8_t *wr_data, uint8_t wr_size)
{
    uint8_t len_data;
    uint32_t  timeout = 0;
    uint8_t xor_temp;
    
    timeout = wr_size*0xffff;     
    while(wr_size > 0){
        len_data = (uint8_t)uart_write_free();
        if (len_data > wr_size) {
            len_data = wr_size;
        }           
        uart_write_data(wr_data, wr_size);
        wr_size -= len_data;     
        timeout--;
        if(timeout == 0)
            return false;
    } 
    xor_temp =  check_xor(wr_data, wr_size);
    timeout = 0xffff;    
    while(uart_write_free() == 0) 
    {
       if(timeout-- == 0)
            return false; 
    }   
    uart_write_data(&xor_temp, 1);    
     
    return true;
}
//写cmd并判断校验码
static bool write_cmd(uint8_t *cmd, uint8_t size)
{
    uint8_t ack ;
    
    if( write_data(cmd, size) != true)     
        return false;
    if( read_data(&ack, 1) != true) 
        return false;
    if(ack !=  BOOTISP_ACK)
        return false; 
    return true;
}
int bootisp_start(void)
{
    uint8_t start =  CMD_START;                  
    UART_Configuration UART_Config;
    
    UART_Config.Baudrate    = 115200;
    UART_Config.DataBits    = UART_DATA_BITS_8;
    UART_Config.Parity      = UART_PARITY_NONE;
    UART_Config.StopBits    = UART_STOP_BITS_1;
    UART_Config.FlowControl = UART_FLOW_CONTROL_NONE;
    uart_set_configuration(&UART_Config);
    
    if( write_cmd(&start, 1) != true)     
        return -ERR_BOOTISP_FAIL;     
    
    return ERR_BOOTISP_OK;
}

//data 为擦除的页数。0xffff为全擦
// page_size 擦除的页数（page_size)
// flash_index；flash页面代码
int extended_erase(uint8_t *data, uint8_t size)
{   
    uint16_t page_size;   
    uint8_t ack;
    uint8_t cmd_buf[2] = {CMD_ERASE, 0xFF - CMD_ERASE};
    
    //send cmd
    if( write_cmd(cmd_buf, sizeof(cmd_buf)) != true)     
        return -ERR_BOOTISP_FAIL; 
   
    //send data 
    page_size = (uint16_t) (data[0] << 8) |  data[1]; 
    if (page_size >= 0xFFFC)        //协议规定
    {
        if(write_data_oxr(data, 2) != true)
            return -ERR_BOOTISP_FAIL;
        
    }
    else
    {
        if(write_data_oxr(data, size) != true)
            return -ERR_BOOTISP_FAIL;        
    }         
    //check ack
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL; 
    return ERR_BOOTISP_OK;   
}
int check_empty(uint32_t addr, uint32_t size)
{
    uint8_t cmd_buf[2] = {CMD_CHECK_EMPTY, 0xFF - CMD_CHECK_EMPTY}; 
    uint8_t ack;
    uint8_t wr_buf[4];
    
    if((addr%4) != 0)               //4字节对齐
        return -ERR_BOOTISP_FAIL;
    if(((size+1)%16) != 0)               //flash长度位16的倍数
        return -ERR_BOOTISP_FAIL;
    //send cmd
    if( write_data(cmd_buf, sizeof(cmd_buf)) != true)     
        return -ERR_BOOTISP_FAIL;         
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL; 
    //send addr    
    wr_buf[0] = (addr & 0xff000000) >> 24;
    wr_buf[1] = (addr & 0x00ff0000) >> 16;
    wr_buf[2] = (addr & 0x0000ff00) >> 8;
	wr_buf[3] = (addr & 0x000000ff);
    if(write_data_oxr(wr_buf, 4) != true)
        return -ERR_BOOTISP_FAIL;  
    //check ack
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL; 
    //send size
    wr_buf[0] = (size & 0xff000000) >> 24;
    wr_buf[1] = (size & 0x00ff0000) >> 16;
    wr_buf[2] = (size & 0x0000ff00) >> 8;
	wr_buf[3] = (size & 0x000000ff);
    if(write_data_oxr(wr_buf, 4) != true)
        return -ERR_BOOTISP_FAIL;  
    //check ack
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL; 
    //empty check
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL;
    return ERR_BOOTISP_OK;    
}

int read_memory(uint32_t addr, uint8_t size, uint8_t *data)
{
    uint8_t ack;
    uint8_t cmd_buf[2] = {CMD_RD_MEMORY, 0xFF - CMD_RD_MEMORY};
    uint8_t wr_buf[4];
    
    if(write_cmd(cmd_buf, 2) != true)
        return -ERR_BOOTISP_FAIL;     
    
    wr_buf[0] = (addr & 0xff000000) >> 24;
    wr_buf[1] = (addr & 0x00ff0000) >> 16;
    wr_buf[2] = (addr & 0x0000ff00) >> 8;
	wr_buf[3] = (addr & 0x000000ff);
    if(write_data_oxr(wr_buf, 4) != true)
        return -ERR_BOOTISP_FAIL;   
    //check ack
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL;
    //read size
    wr_buf[0] = size - 1; //发送的数据为实际接收数据长度 -1  	    
    if(write_data_oxr(wr_buf, 1) != true)//需要接收数据长度  第一个字节为ACK后面紧接着为数据
        return -ERR_BOOTISP_FAIL;  
    //check ack
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL;
    //rcv data
    if( read_data(data,size+1) != true) 
        return -ERR_BOOTISP_FAIL;
   
    return ERR_BOOTISP_OK;
}

int go_cmd(uint32_t addr)
{
    uint8_t cmd_buf[2] = {CMD_GO, 0xFF - CMD_GO};
    uint8_t wr_buf[4]; 
    uint8_t ack;   
    
    if(write_cmd(cmd_buf, 2) != true)
        return -ERR_BOOTISP_FAIL;
    
    wr_buf[0] = (addr & 0xff000000) >> 24;
    wr_buf[1] = (addr & 0x00ff0000) >> 16;
    wr_buf[2] = (addr & 0x0000ff00) >> 8;
	wr_buf[3] = (addr & 0x000000ff);
    
    if(write_data_oxr(wr_buf, 4) != true)
        return -ERR_BOOTISP_FAIL;  
     //check ack
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL;
    //check addr
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL;
    
    return ERR_BOOTISP_OK;    
}

//写指定地址数据  长度最大为256   长度必须为4的倍数

int write_memory(uint32_t addr,  uint8_t *data, uint8_t size)
{
    uint8_t cmd_buf[2] = {CMD_WR_MEMORY, 0xFF - CMD_WR_MEMORY};
    uint8_t ack;
    uint8_t addr_buf[4];
    uint8_t wr_buf[BOOTSIP_DATA_MAX_LEN+1];
    
    if(size > 255)
        return -ERR_BOOTISP_FAIL;
    if((size%4) != 0)
        return -ERR_BOOTISP_FAIL;
    //send cmd and chack ack
     if( write_data(cmd_buf, 2) != true)     
        return -ERR_BOOTISP_FAIL;         
   
    //write addr

    addr_buf[0] = (addr & 0xff000000) >> 24;
    addr_buf[1] = (addr & 0x00ff0000) >> 16;
    addr_buf[2] = (addr & 0x0000ff00) >> 8;
	addr_buf[3] = (addr & 0x000000ff);  
    if(write_data_oxr(addr_buf, 4) != true)
        return -ERR_BOOTISP_FAIL;  
     //check ack
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL;
    //write data size and data    
    wr_buf[0] = size;             //发送的数据为实际需要些的数据长度 -1
    memcpy(&wr_buf[1], data, size); //拷贝需要发送的数据     
    if(write_data_oxr(wr_buf, size+1) != true)
        return -ERR_BOOTISP_FAIL;  
    //check ack
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL; 
    return ERR_BOOTISP_OK;    
}  

static int get_crc32_cmd(uint32_t addr, uint32_t size, uint32_t *crc_value)
{
    uint8_t cmd_buf[2] = {CMD_GET_CRC, 0xFF - CMD_GET_CRC};
    uint8_t ack;
    uint8_t wr_buf[4];
    uint8_t rd_buf[5]; 
    
    if((addr%4) != 0)               //4字节对齐
        return -ERR_BOOTISP_FAIL;
    if((size%16) != 0)               //flash长度位16的倍数
        return -ERR_BOOTISP_FAIL;
     //send cmd and chack ack
    if( write_data(cmd_buf, 2) != true)     
        return -ERR_BOOTISP_FAIL;   
    //write addr
    wr_buf[0] = (addr & 0xff000000) >> 24;
    wr_buf[1] = (addr & 0x00ff0000) >> 16;
    wr_buf[2] = (addr & 0x0000ff00) >> 8;
	wr_buf[3] = (addr & 0x000000ff);
    if(write_data_oxr(wr_buf, 4) != true)
        return -ERR_BOOTISP_FAIL; 
    //check ack
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL;
    //write size 
    size = size -1 ;
    wr_buf[0] = (size & 0xff000000) >> 24;
    wr_buf[1] = (size & 0x00ff0000) >> 16;
    wr_buf[2] = (size & 0x0000ff00) >> 8;
	wr_buf[3] = (size & 0x000000ff);

    if(write_data_oxr(wr_buf, 4) != true)
        return -ERR_BOOTISP_FAIL; 
    //check ack
    if( read_data(&ack, 1) != true) 
        return -ERR_BOOTISP_FAIL;
    if(ack !=  BOOTISP_ACK)
        return -ERR_BOOTISP_FAIL;    
    //rcv data
    if( read_data(rd_buf, 4) != true) 
        return -ERR_BOOTISP_FAIL;  
    *crc_value = (rd_buf[0] << 24) |
                (rd_buf[1] << 16) |
                (rd_buf[2] << 8)  |
                (rd_buf[3] << 0);   
    
    return ERR_BOOTISP_OK;    
}



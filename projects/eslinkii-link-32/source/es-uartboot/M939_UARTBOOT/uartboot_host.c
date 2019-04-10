#include "eslink.h"
#include "uartboot_host.h"
#include "uartboot_target_config.h"

 
static void uartboot_delay(uint32_t n)
{
    es_delay_ms(1);
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

 
//读数据
static uint8_t read_data(uint8_t *rd_data, uint8_t rd_size, uint8_t delay_ms)
{
    uint32_t timeout = 100 + delay_ms;
    uint8_t len_data;
                         
    while(rd_size > 0)
    {
        len_data = uart_read_data(rd_data, rd_size);
        rd_size -= len_data;
        rd_data += len_data;
        timeout--;
        uartboot_delay(1);
        if(timeout == 0)
            return FALSE;
    } 
    return TRUE;      
} 
//读数据并判断应答
static uint8_t read_cmd_and_check(uint8_t ack,  uint8_t delay_ms)
{
    uint32_t i = 0, timeout = 100 + delay_ms;
    uint8_t rd_data; 
    
    for(i=0; i<timeout; i++)
    {
        if( (uart_read_data(&rd_data, 1)) && (rd_data == ack))
           break;      
        uartboot_delay(1);    
    }
    if(i >= timeout)
        return  FALSE;
    return TRUE;      
}
//bootisp 写数据 
static uint8_t write_data(uint8_t *wr_data, uint8_t wr_size)
{
    uint32_t len_data;
    
    while(wr_size > 0)
    {      
        len_data = uart_write_free();
        if(len_data > wr_size)
            len_data = wr_size;
        uart_write_data(wr_data, len_data);
        wr_data += len_data;
        wr_size -= len_data;  
    }
    return TRUE;

} 
//写数据和校验码    
static uint8_t write_data_oxr(uint8_t *data, uint8_t size)
{
    uint8_t xor_temp;
    uint32_t timeout = 0;

    xor_temp =  check_xor(data, size); 
    
    if(write_data(data, size) != TRUE)
        return FALSE;
    
    timeout = 0xffff;    
    while(uart_write_free() == 0) 
    {
       if(timeout-- == 0)
            return false; 
    }   
    uart_write_data(&xor_temp, 1);  
  
    return TRUE;
}
//写cmd并判断校验码
static uint8_t write_cmd_and_check(uint8_t *cmd, uint8_t size)
{        
    if(write_data(cmd, size) != TRUE)
        return FALSE;
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE; 
   
    return TRUE;
}

uint8_t uartboot_start(void)
{
    uint8_t start =  CMD_START;                  
    UART_Configuration UART_Config;
    
    UART_Config.Baudrate    = BOOTISP_DEFAULT_BAUDRATE;
    UART_Config.DataBits    = UART_DATA_BITS_8;
    UART_Config.Parity      = UART_PARITY_EVEN;
    UART_Config.StopBits    = UART_STOP_BITS_1;
    UART_Config.FlowControl = UART_FLOW_CONTROL_NONE;
    uart_set_configuration(&UART_Config);
    
    if( write_cmd_and_check(&start, 1) != TRUE)     
        return FALSE;     
    
    return TRUE;
}

//data 为擦除的页数。0xffff为全擦
// page_size 擦除的页数（page_size)
// flash_index；flash页面代码
uint8_t uartboot_extended_erase(uint8_t *data, uint8_t size)
{   
    uint16_t page_size;   
    uint8_t cmd_buf[2] = {CMD_ERASE, 0xFF - CMD_ERASE};
    
    //send cmd
    if( write_cmd_and_check(cmd_buf, 2) != TRUE)     
        return FALSE; 
   
    //send data 
    page_size = (uint16_t) (data[0] << 8) |  data[1]; 
    if (page_size > 0xFFFC)        //协议规定
    {
        if(write_data_oxr(data, 2) != TRUE)
            return FALSE;         
    }
    else
    {
        if(write_data_oxr(data, size) != TRUE)
            return FALSE;        
    }         
    //check ack
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE; 
        
    return TRUE;   
}
uint8_t uartboot_check_empty(uint32_t addr, uint32_t size)
{
    uint8_t cmd_buf[2] = {CMD_CHECK_EMPTY, 0xFF - CMD_CHECK_EMPTY}; 
    uint8_t wr_buf[4];
    
    if(addr & 0x03)               //4字节对齐
        return FALSE;
    if(size & 0x0F )          //flash长度位16的倍数
        return FALSE;
    //send cmd
    write_cmd_and_check( cmd_buf, 2);
        
    //send addr    
    wr_buf[0] = (addr & 0xff000000) >> 24;
    wr_buf[1] = (addr & 0x00ff0000) >> 16;
    wr_buf[2] = (addr & 0x0000ff00) >> 8;
  wr_buf[3] = (addr & 0x000000ff);
    if(write_data_oxr(wr_buf, 4) != TRUE)
        return FALSE;  
    //check ack
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE; 
        
    //send size
    wr_buf[0] = ((size-1) & 0xff000000) >> 24;
    wr_buf[1] = ((size-1) & 0x00ff0000) >> 16;
    wr_buf[2] = ((size-1) & 0x0000ff00) >> 8;
  wr_buf[3] = ((size-1) & 0x000000ff);
    if(write_data_oxr(wr_buf, 4) != TRUE)
        return FALSE;  
    //check ack
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE; 
    //empty check
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE; 
    return TRUE;    
}

static uint8_t uartboot_read_block(uint32_t addr, uint8_t *data, uint32_t size)
{
    uint8_t cmd_buf[2] = {CMD_RD_MEMORY, 0xFF - CMD_RD_MEMORY};
    uint8_t wr_buf[4];
    
    if(size == 0)
        return TRUE;
    if(size > BOOTSIP_DATA_SIZE)
        return FALSE;
    if(write_cmd_and_check(cmd_buf, 2) != TRUE)
        return FALSE;     
    
    wr_buf[0] = (addr & 0xff000000) >> 24;
    wr_buf[1] = (addr & 0x00ff0000) >> 16;
    wr_buf[2] = (addr & 0x0000ff00) >> 8;
  wr_buf[3] = (addr & 0x000000ff);
    if(write_data_oxr(wr_buf, 4) != TRUE)
        return FALSE;   
    //check ack
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE; 
    //read size
    wr_buf[0] = size - 1; //发送的数据为实际接收数据长度 -1        
    if(write_data_oxr(wr_buf, 1) != TRUE)//需要接收数据长度  第一个字节为ACK后面紧接着为数据
        return FALSE;  
    //check ack
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE; 
    //rcv data
    if( read_data(data,size, 200) != TRUE) 
        return FALSE;
   
    return TRUE;
}
static uint8_t uartboot_read_block_retry(uint32_t addr, uint8_t *data, uint32_t size)
{
  uint8_t i;
  
  for (i = 0; i < 3; i++) 
  {
    if ( uartboot_read_block(addr, data, size) != FALSE) 
      break;
  }
    if(i >= 3)
    return FALSE;
  return TRUE;
}
uint8_t uartboot_read_memory(uint32_t addr, uint8_t *data, uint32_t size)
{
    uint32_t page;
    uint32_t single;
    
    page =  size / BOOTSIP_DATA_SIZE;
    single = size % BOOTSIP_DATA_SIZE;
    while (page) 
    {
        if (uartboot_read_block_retry(addr, data, BOOTSIP_DATA_SIZE) != TRUE)
            return FALSE;
        addr += BOOTSIP_DATA_SIZE;
        data += BOOTSIP_DATA_SIZE;
//        size -= BOOTSIP_DATA_SIZE;
        page --;
    }
    
    if (uartboot_read_block(addr, data, single) != TRUE)
        return FALSE;
    return TRUE;
}  

//写指定地址数据  
static uint8_t uartboot_write_block(uint32_t addr,  uint8_t *data, uint32_t size)  
{
    uint8_t cmd_buf[2] = {CMD_WR_MEMORY, 0xFF - CMD_WR_MEMORY};
    uint8_t wr_buf[BOOTSIP_DATA_SIZE+1];
    uint8_t addr_buf[4];
    
    if(size == 0)
        return TRUE;
    if(size > BOOTSIP_DATA_SIZE)
        return FALSE;
    if(size & 0x03)
        return FALSE;
    //send cmd and chack ack
     if( write_cmd_and_check(cmd_buf, 2) != TRUE)     
        return FALSE;         
   
    //write addr  
    addr_buf[0] = (addr & 0xff000000) >> 24;
    addr_buf[1] = (addr & 0x00ff0000) >> 16;
    addr_buf[2] = (addr & 0x0000ff00) >> 8;
  addr_buf[3] = (addr & 0x000000ff);  
    if(write_data_oxr(addr_buf, 4) != TRUE)
        return FALSE;  
     //check ack
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE; 
        
    //write data size and data    
    wr_buf[0] = size - 1;             //发送的数据为实际需要些的数据长度 -1
    memcpy(&wr_buf[1], data, size); //拷贝需要发送的数据     
    if(write_data_oxr(wr_buf, size+1) != TRUE)
        return FALSE;  
    //check ack
   if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_WRITE_PAGE_TIME) != TRUE)  
        return FALSE;  
    return TRUE;    
}  

uint8_t uartboot_write_block_retry(uint32_t addr,  uint8_t *data, uint32_t size)  
{
  uint8_t i;
  
  for (i = 0; i < 3; i++) 
  {
    if ( uartboot_write_block(addr, data, size) != FALSE) 
      break;
  }
    if(i >= 3)
    return FALSE;
  return TRUE;

}
uint8_t uartboot_write_memory(uint32_t addr,  uint8_t *data, uint32_t size)
{
    uint32_t page;
    uint32_t single;
    
    page =  size / BOOTSIP_DATA_SIZE;
    single = size % BOOTSIP_DATA_SIZE;
    while (page) 
    {
        if (uartboot_write_block_retry(addr, data, BOOTSIP_DATA_SIZE) != TRUE)
            return FALSE;
        addr += BOOTSIP_DATA_SIZE;
        data += BOOTSIP_DATA_SIZE;
        page --;
    }
    
    if (uartboot_write_block(addr, data, single) != TRUE)
        return FALSE;
    return TRUE;
} 

static uint8_t uartboot_get_crc32_cmd(uint32_t addr, uint32_t size, uint32_t *crc_value)
{
    uint8_t cmd_buf[2] = {CMD_GET_CRC, 0xFF - CMD_GET_CRC};
    uint8_t wr_buf[4];
    
    if((addr & 0x03) != 0)               //4字节对齐
        return FALSE;
    if((size & 0x0f) != 0)               //flash长度位16的倍数
        return FALSE;
     //send cmd and chack ack
     if( write_cmd_and_check(cmd_buf, 2) != TRUE)     
        return FALSE;  
    //write addr
    wr_buf[0] = (addr & 0xff000000) >> 24;
    wr_buf[1] = (addr & 0x00ff0000) >> 16;
    wr_buf[2] = (addr & 0x0000ff00) >> 8;
  wr_buf[3] = (addr & 0x000000ff);
    if(write_data_oxr(wr_buf, 4) != TRUE)
        return FALSE; 
    //check ack
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE;
    //write size 
    size = size -1 ;
    wr_buf[0] = (size & 0xff000000) >> 24;
    wr_buf[1] = (size & 0x00ff0000) >> 16;
    wr_buf[2] = (size & 0x0000ff00) >> 8;
  wr_buf[3] = (size & 0x000000ff);

    if(write_data_oxr(wr_buf, 4) != TRUE)
        return FALSE; 
    //check ack
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE;  
    //rcv data
    if( read_data((uint8_t*)crc_value ,4, BOOTISP_DEFAULT_TIME) != TRUE) 
        return FALSE;
    
    return TRUE;    
}

uint8_t uartboot_go_cmd(uint32_t addr)
{
    uint8_t cmd_buf[2] = {CMD_GO, 0xFF - CMD_GO};
    uint8_t wr_buf[4];  
    
    if(write_cmd_and_check(cmd_buf, 2) != TRUE)
        return FALSE;
    
    wr_buf[0] = (addr & 0xff000000) >> 24;
    wr_buf[1] = (addr & 0x00ff0000) >> 16;
    wr_buf[2] = (addr & 0x0000ff00) >> 8;
  wr_buf[3] = (addr & 0x000000ff);
    
    if(write_data_oxr(wr_buf, 4) != TRUE)
        return FALSE;  
     //check ack
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE;
    //check addr
    if(read_cmd_and_check(BOOTISP_ACK, BOOTISP_DEFAULT_TIME) != TRUE)  
        return FALSE;
    
    return TRUE;    
}





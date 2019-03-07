#include "eslink.h"
#include "./target_info.h"

#define PIN_DELAY(n)    ES_DELAY_SLOW(4*n)
//#define PIN_DELAY(n)    ES_DELAY_FAST(n)

#define ISP_CLOCK_CYCLE()                \
  PIN_ISPCLK_CLR();                      \
  PIN_DELAY(1);                          \
  PIN_ISPCLK_SET();                      \
  PIN_DELAY(1)

#define ISP_WRITE_BIT(bit)               \
  PIN_ISPSDA_OUT(bit);                   \
  PIN_ISPCLK_CLR();                      \
  PIN_DELAY(1);                          \
  PIN_ISPCLK_SET();                      \
  PIN_DELAY(1)

#define ISP_READ_BIT(bit)                \
  PIN_ISPCLK_CLR();                      \
  PIN_DELAY(1);                          \
  bit = PIN_ISPSDA_IN();                 \
  PIN_ISPCLK_SET();                      \
  PIN_DELAY(1)          

#define FC_READ_CMD             0x00    //读取fc_cmd数据
#define FC_READ_PA              0x01
#define FC_READ_PLD             0x02
#define FC_READ_PHD             0x03
#define FC_READ_STA             0X05
#define FC_READ_RA              0X06
#define FC_READ_RD              0X07

#define FC_WRITE_CMD            0X40        //写入指令
#define FC_WRITE_PA             0X41        //写入位置
#define FC_WRITE_PLD            0X42        //编程数据
#define FC_WRITE_PHD            0X43        //编程数据
#define FC_WRITE_RA             0X46   

#define ISP_UNLOCK              0x10        //解锁        
#define ISP_LOCK                0x11        //上锁
#define ISP_CHECK_UNLOCK        0x12        //检查状态
#define ISP_PASS                0x7F        //切换成swd模式，
#define UNLOCK_CHECK_VAL        0x45535450
// ISP Transfer Response
#define ISP_TRANSFER_OK         (1U<<0)
#define ISP_TRANSFER_WAIT       (1U<<1)
#define ISP_TRANSFER_FAULT      (1U<<2)
#define ISP_TRANSFER_ERROR      (1U<<3)

#define MAX_ISP_RETRY 100//10

#define CODE_AREA               (1U<<0)
#define INFO_AREA               (1U<<1)

#define ISP_WRITE               (1U<<0)
#define ISP_READ                (1U<<1)

static uint8_t isp_read_write(uint8_t op_code, uint32_t *data, uint8_t mode)
{
    uint32_t ack;                                                                 
    uint32_t bit;                                                                 
    uint32_t val;                                                                 
    uint32_t parity;   
    
    uint32_t n;      
    parity = 0U;   

    PIN_ISPSDA_OUT_ENABLE();
    /* Start Bit */     
    PIN_ISPSDA_SET();                   
    PIN_ISPCLK_CLR();                      
    PIN_DELAY(1);                          
    PIN_ISPCLK_SET();                      
    PIN_DELAY(1);      
          
    /* op code */          
	for (n = 7U; n; n--) 
    {             
        ISP_WRITE_BIT(op_code);
        op_code >>= 1;
	}     
   
    /* Turnaround */ 
    PIN_ISPSDA_OUT_DISABLE();
    PIN_ISPCLK_CLR();                      
    PIN_DELAY(1);                          
    PIN_ISPCLK_SET();                      
    PIN_DELAY(1);
    
    /* Acknowledge response */ 
    ISP_READ_BIT(bit);                                                             
    ack  = bit << 0;                                                              
    ISP_READ_BIT(bit);                                                             
    ack |= bit << 1;                                                              
    ISP_READ_BIT(bit);                                                             
    ack |= bit << 2;  
        
    if (ack == ISP_TRANSFER_OK)      /* OK response */   
    {    
        if(mode == ISP_WRITE)
        {
            /* Turnaround */                                                          
            PIN_ISPSDA_OUT_DISABLE();
            ISP_CLOCK_CYCLE(); 
            /* Write data */   
            PIN_ISPSDA_OUT_ENABLE();                                                   
                                                           
            val = *data;                                                              
            parity = 0U;                                                              
            for (n = 32U; n; n--)             /* Write WDATA[0:31] */    
            {   
                ISP_WRITE_BIT(val);       
                
                parity += val;                                                          
                val >>= 1;                                                              
            }                                                                         
            ISP_WRITE_BIT(parity);               /* Write Parity Bit */ 
            PIN_ISPSDA_CLR(); 
            ISP_CLOCK_CYCLE();    
        }
        else                             
        {
            /* Read data */                                                        
            val = 0U;                                                                 
            parity = 0U;                                                              
            for (n = 32U; n; n--)        /* Read RDATA[0:31] */ 
            {    
                ISP_READ_BIT(bit);                 
                parity += bit;                                                          
                val >>= 1;                                                              
                val  |= bit << 31;                                                                     
            }   
            
            ISP_READ_BIT(bit);                /* Read Parity */ 
            if ((parity ^ bit) & 1U) 
            {                                                
                ack = ISP_TRANSFER_ERROR;                                              
            }                                                                        
            if (data) 
            { 
                *data = val; 
            }    
            PIN_ISPSDA_OUT_ENABLE();                                                                
            PIN_ISPSDA_CLR();                                                        
                                                         
            ISP_CLOCK_CYCLE();                                                       
            ISP_CLOCK_CYCLE();                                      
        }
    PIN_ISPCLK_CLR();  
    PIN_ISPSDA_SET();    
    return ((uint8_t)ack);    
    }
                                                          
    PIN_ISPSDA_OUT_ENABLE(); 
    ISP_CLOCK_CYCLE(); 
    ISP_CLOCK_CYCLE(); 
    PIN_ISPCLK_CLR();  
    PIN_ISPSDA_SET(); 
    return ((uint8_t)ack);    
}


static uint8_t isp_read(uint8_t op_code, uint32_t *data)
{
    uint8_t i, ack;

    for (i = 0; i < MAX_ISP_RETRY; i++) 
    {
        ack = isp_read_write(op_code, data, ISP_READ); 
        // if ack != WAIT
        if (ack != ISP_TRANSFER_WAIT) 
            break;
    }  
    if( ack != ISP_TRANSFER_OK)
        return FALSE;
    return TRUE;
}  
static uint8_t isp_write(uint8_t op_code, uint32_t data)
{
    uint8_t i, ack;
    uint32_t temp;
    
    temp = data;

    for (i = 0; i < MAX_ISP_RETRY; i++) 
    {
        ack = isp_read_write(op_code, &temp, ISP_WRITE);
        // if ack != WAIT
        if (ack != ISP_TRANSFER_WAIT) 
            break;
    }  
    if( ack != ISP_TRANSFER_OK)
        return FALSE;
    return TRUE;
}  
   
/*******************************************************************************
*函数名：page_erase_and_check
* 描述 ：页擦
* 输入 ：area：页擦区域 addr：地址。delayms：
* 输出 ：TRUE/FALSE
*******************************************************************************/
static uint8_t page_erase_and_check(uint8_t area, uint32_t addr)
{
    uint32_t temp;
    uint32_t i;
    uint32_t wait;
    
    if(area  == CODE_AREA)
        temp =  (addr & 0x00ffffff) & 0xFEFFFFFF ;
    else
        temp =  (addr & 0x00ffffff) | (1 << 24) ;     
    if(isp_write(FC_WRITE_PA, temp) != TRUE)   
        return FALSE;
    if(isp_write(FC_WRITE_CMD, 0x000000F1) != TRUE)
        return FALSE;
    //延时，等待擦除完成
    wait = 100;
    es_delay_ms(2);
    for( i = 0; i < wait; i++)
    {
        if( isp_read(FC_READ_CMD, &temp) != TRUE)
            return FALSE;
        if(temp == 0x00)
            break;
        es_delay_ms(1);
    }  
    if(i >= wait)
        return FALSE;
    return TRUE; 

} 

/*******************************************************************************
*函数名：program_word
* 描述 ：字节编程。flash编程时间约为30us
* 输入 ：area：编程区域
*        addr：地址。data：数据。failed_offset:编程失败偏移地址
* 输出 ：TRUE/FALSE
*******************************************************************************/
static uint8_t program_data(uint8_t area, uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_offset) 
{
    uint32_t size_in_words;
    uint32_t i,n;
    uint32_t temp;
    uint32_t wait ;
    
    if (size == 0) 
        return TRUE;  
    if (size >1024)
        return FALSE;  
    //双字节编程
    if(size &0x07)
        return FALSE; 
        
    size_in_words = size / 8;  
    if(area  == CODE_AREA)
        temp =  ((addr & 0x00ffffff) | (( size_in_words - 1) << 25)) & 0xFEFFFFFF ;
    else
        temp =  (addr & 0x00ffffff) | (( size_in_words - 1) << 25) | (1 << 24) ;
    
    isp_write(FC_WRITE_PA, temp);  
    
    for(i=0; i<size_in_words; i++)
    {     
        //设置数据缓冲器
        temp =  *(data +0) | (*(data +1)<< 8) | (*(data +2)<< 16) | (*(data +3) << 24);
        if(isp_write(FC_WRITE_PLD, temp) != TRUE)
            return FALSE;
        temp =  *(data +4) | (*(data +5)<< 8) | (*(data +6)<< 16) | (*(data +7) << 24);
        if(isp_write(FC_WRITE_PHD, temp) != TRUE)  
            return FALSE;
        //编程并判断
        if(isp_write(FC_WRITE_CMD, 0x000000F0) != TRUE)
            return FALSE;
        wait = 100; 
        es_delay_us(20);
        for(n=0; n < wait ; n++)
        {
            if(isp_read(FC_READ_CMD, &temp) != TRUE)
                return FALSE;
            if(0x00 == temp)
                break;
            es_delay_us(1);
        }
        if (n >= wait) 
        {
            if(failed_offset)
                *failed_offset = n;
            return FALSE;           
        }
        
        data += 8;    
    }
    return TRUE;  
}
/*******************************************************************************
*函数名：read_data
* 描述 ：读数据
* 输入 ：
* 输出 ：TRUE/FALSE
*******************************************************************************/
static uint8_t read_data(uint8_t area, uint32_t addr, uint8_t *data, uint32_t size)
{
    uint32_t size_in_words;
    uint32_t temp;
    uint32_t i;
     
    if (size == 0) 
        return TRUE;  
    if (size >512)
        return FALSE;  
    if (size & 0x03)
        return FALSE;  
    size_in_words = size / 4; 
    
    if(area  == CODE_AREA)
        temp =  ((addr & 0x00ffffff) | ((( size_in_words - 1) << 25) & 0xFE000000)) & 0xFEFFFFFF ;
    else 
        temp =  (addr & 0x00ffffff) | ((( size_in_words - 1) << 25) & 0xFE000000) | (1 << 24) ;
    if(isp_write(FC_WRITE_RA, temp) != TRUE)
        return FALSE;
    
    for( i=0; i<size_in_words; i++)
    {
        if(isp_read(FC_READ_RD, &temp) != TRUE)
            return FALSE;
        *data =  temp & 0xff;
        *(data+1) =  (temp >> 8) & 0xff;
        *(data+2) =  (temp >> 16) & 0xff;
        *(data+3) =  (temp >> 24) & 0xff; 
        data += 4;
    }         

    return TRUE;
}  
/*******************************************************************************
*函数名：isp_target_program_code
* 描述 ：isp编程。flash编程时间约为30us
* 输入 ：addr：地址。data：数据。size：大小 failed_offset:编程失败偏移地址
* 输出 ：
*******************************************************************************/
uint8_t isp_program_code(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_offset) 
{    
    uint8_t i;
    uint8_t retry;
   
    retry = 10;    
    for(i=0; i<retry; i++)
    {
        if(program_data(CODE_AREA, addr, data, size, failed_offset) != FALSE)
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
uint8_t isp_read_code(uint32_t addr, uint8_t *data, uint32_t size) 
{
     if(read_data(CODE_AREA, addr, data, size) != TRUE)
        return FALSE;
     return TRUE; 
}
/*******************************************************************************
*函数名：isp_program_config
* 描述 ：配置字编程
* 输入 ：addr：地址。data：数据。size：大小  failed_offset:编程失败偏移地址
* 输出 ：
*******************************************************************************/
uint8_t isp_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_offset) 
{
    uint8_t i;
    uint8_t retry;
   
    retry = 10;    
    for(i=0; i<retry; i++)
    {
        if(program_data(INFO_AREA, addr, data, size, failed_offset) != FALSE)
            break;
    }
    if(i >= retry)
        return FALSE;
        
    return TRUE; 
}  
uint8_t isp_read_config(uint32_t addr, uint8_t *data, uint32_t size) 
{
     if(read_data(INFO_AREA, addr, data, size) != TRUE)
        return FALSE;
     return TRUE; 
}

/*******************************************************************************
*函数名：isp_erase_code
* 描述 ：code区擦除
* 输入 ：addr：地址。size：大小
* 输出 ：
*******************************************************************************/
uint8_t isp_erase_code(uint32_t addr, uint32_t size)
{
    uint32_t n;
    uint32_t page_num;
    
    page_num = size/1024;     
    for(n=0; n<page_num; n++)
    {
        if(page_erase_and_check(CODE_AREA, addr) != TRUE)
            return FALSE;
        addr += 0x400;
    }

    return TRUE;
} 
/*******************************************************************************
*函数名：isp_erase_info7
* 描述 ：info区擦除
* 输入 ：
* 输出 ：
*******************************************************************************/
uint8_t isp_erase_info7(void)
{
    if( page_erase_and_check(INFO_AREA, CHIP_INFO_PART2_ADDR) != TRUE)
        return FALSE;

    return TRUE;
}
uint8_t isp_erase_info6(void)
{
    if( page_erase_and_check(INFO_AREA, CHIP_INFO_PART1_ADDR) != TRUE)
            return FALSE;
    return TRUE;
}

/*******************************************************************************
*函数名：isp_unlock_check
* 描述 ：解锁并判断 
* 输入 ：
* 输出 ：
*******************************************************************************/
uint8_t isp_unlock_check(void)
{
    uint32_t temp;
    uint32_t i;     
    
    //解锁   
    for(i=0; i<10; i++)
    {
        if(isp_write(ISP_UNLOCK, 0xFFFFFFFF) != ISP_TRANSFER_OK)
            return FALSE;                    
        if(isp_read(ISP_CHECK_UNLOCK, &temp) != ISP_TRANSFER_OK)
            return FALSE;  
        if(UNLOCK_CHECK_VAL == temp)
            break;
    }
    if(UNLOCK_CHECK_VAL != temp)
        return FALSE;   
    return TRUE;
}

uint8_t isp_mode_check(void)
{
    uint32_t check_val;
    uint32_t i;  
    
    for(i=0; i<3; i++)
    {     
       if(isp_read(ISP_CHECK_UNLOCK, &check_val) != ISP_TRANSFER_OK)
            return FALSE;       

        if(UNLOCK_CHECK_VAL ==  check_val)  
            break;    
    }
    if(UNLOCK_CHECK_VAL !=  check_val) 
        return FALSE;
    return TRUE;   
}

////isp 模式设置
// uint8_t isp_mode_set(void)
//{
////    uint8_t ret;
//    uint8_t check_val;
//    uint32_t i;

//    //isp模式设置
//    for(i=0; i<3; i++)
//    {
//        mode_set(ISP_MODE_CMD);
//        check_val = get_chip_status();
//        if(ISP_MODE_CHECK_VAL ==  check_val)  
//            break;    
//    }
//    if(ISP_MODE_CHECK_VAL !=  check_val) 
//        return FALSE;
//    return TRUE;
//}



//}
////isp 加密字加载并判断
//uint8_t encrypt_check(void)
//{
//    uint16_t data;
//    isp_rcv_bytes(ENCRYPT_CHECK_CMD,(uint8_t*)&data, 2) ;
//    if(data != 0xA5A5)
//        return FALSE;
//    return TRUE;  
//}
////进入isp模式
//uint8_t isp_entry_mode(void)
//{
//    isp_reset();
//    if(isp_id_check() != TRUE)
//        return ERROR_IN_ISP_MODE;
//    if(isp_unlock() != TRUE)
//        return ERROR_ISP_UNLOCK;
//    if( isp_mode_set() != TRUE)
//        return ERROR_IN_ISP_MODE;
//    if( encrypt_check() != TRUE)
//        return ERROR_IN_ISP_MODE;
//    
//    return ERROR_SUCCESS; 
//}

////退出isp模式
//uint8_t isp_out_mode(void)
//{      
//    isp_reset(); 
//    return ERROR_SUCCESS; 
//}




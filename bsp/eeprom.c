#include "eslink.h" 
#include "eslink_gpio.h" 
#include "eeprom.h"

#define I2C_WR  0    /* 写控制bit */
#define I2C_RD  1    /* 读控制bit */     

static void soft_i2c_init(void)
{
//开漏输出，上下拉不使能
    port_pin_config_t config = 
    {
        kPORT_PullDisable,
        kPORT_FastSlewRate,
        kPORT_PassiveFilterDisable,
        kPORT_OpenDrainEnable,
        kPORT_LowDriveStrength,
        kPORT_MuxAsGpio,
    };
//    PORT_SetPinMux(PIN_IIC_SCL_PORT, PIN_IIC_SCL_BIT, kPORT_MuxAsGpio);
//    PORT_SetPinMux(PIN_IIC_SDA_PORT, PIN_IIC_SDA_BIT, kPORT_MuxAsGpio);
    PORT_SetPinConfig( PIN_IIC_SCL_PORT, PIN_IIC_SCL_BIT, &config);
  PORT_SetPinConfig( PIN_IIC_SDA_PORT, PIN_IIC_SDA_BIT, &config);

    IIC_SCL_INIT(1); 
  IIC_SDA_INIT(1);
}      

static void i2c_delay_us(uint32_t delay) 
{   
    delay *= ((SystemCoreClock/1000000U) + (4-1U)) / 4;
    while (--delay);
}     

static void i2c_start(void)
{    
  IIC_SDA_SET();
    i2c_delay_us(1);
    IIC_SCL_SET(); 
  i2c_delay_us(4);
  IIC_SDA_CLR();  
  i2c_delay_us(4);
  IIC_SCL_CLR();  
 }

static void i2c_stop(void)
{      
  IIC_SCL_CLR();
    i2c_delay_us(1);
  IIC_SDA_CLR();//STOP:when CLK is high DATA change form low to high
   i2c_delay_us(4);
  IIC_SCL_SET();    
  IIC_SDA_SET();//发送I2C总线结束信号
     i2c_delay_us(4);
  
}   

#define IIC_TIMEOUT     ((uint32_t)0x1000)
static uint8_t i2c_timeout_cb(void)
{
    i2c_stop();  
    return FALSE;
}
static uint8_t i2c_waitack(void)
{
  uint32_t timeout = IIC_TIMEOUT;    
  
  IIC_SDA_SET();
  i2c_delay_us(1);
  IIC_SCL_SET();
  i2c_delay_us(1);
    IIC_SDA_IN();//SDA 设置为输入    
  while( IIC_SDA_READ() )      //等待应答
  {
        if((timeout--) == 0)
        {                   
             return i2c_timeout_cb();
        }        
  }  
    IIC_SDA_OUT();  //SDA线输出
  IIC_SCL_CLR();  //再拉低SCL完成应答位，并保持住总线
    i2c_delay_us(1);
  return TRUE; 
}

static uint8_t i2c_read(void)
{
  uint8_t i;
  uint8_t receive = 0;

  IIC_SDA_IN();//SDA 设置为输入
  for (i=0; i<8; i++)
  {
        receive <<= 1;  //从高位到低位依次进行
    IIC_SCL_SET();
        i2c_delay_us(1);
        if(IIC_SDA_READ())
        {
             receive++;
        }         
        IIC_SCL_CLR();     
    i2c_delay_us(1);
        
  }    
  IIC_SDA_OUT();  //SDA线输出
  return receive; 
}  

/*******************************************************************************
*   函 数 名: i2c_read_noack
*   功能说明: I2C 读数据.不发送应答位
*   形    参:  无
*   返 回 值: 无
*******************************************************************************/
static uint8_t i2c_read_noack(void)
{
  uint8_t rcv;
  rcv = i2c_read();
  IIC_SDA_SET();    //8位数据发送完后，拉高SDA，发送非应答信号
  i2c_delay_us(1);
  IIC_SCL_SET();
  i2c_delay_us(1);
  IIC_SCL_CLR();
  return rcv;
}

/*******************************************************************************
*  函 数 名: i2c_Ack
*  功能说明: I2C 读数据.发送应答位
*  形    参:  无
*  返 回 值: 无
*******************************************************************************/
static uint8_t i2c_read_ack(void)
{
  uint8_t rcv;
  rcv = i2c_read();
  IIC_SDA_CLR();        //8位数据发送完后，拉低SDA，发送应答信号
  i2c_delay_us(1);
  IIC_SCL_SET();
  i2c_delay_us(1);
  IIC_SCL_CLR();
    IIC_SDA_SET();
  return rcv;
}

/*******************************************************************************
*  函 数 名: i2c_write
*  功能说明: CPU向I2C总线设备发送8bit数据
*  形    参:  _ucByte ： 等待发送的字节
*  返 回 值: 
*******************************************************************************/
static void i2c_write(uint8_t data)
{
  uint8_t i;
  
  for (i=0; i<8; i++)
  {
        if( (data&0x80) >> 7)
            IIC_SDA_SET();
        else
            IIC_SDA_CLR();
    
    i2c_delay_us(1);
    IIC_SCL_SET();
    i2c_delay_us(1);
    IIC_SCL_CLR();
    i2c_delay_us(1);
        data <<= 1;
  }
}

/*******************************************************************************
*  函 数 名: i2c_CheckDevice
*  功能说明: 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
*  形    参:  _Address：设备的I2C总线地址
*  返 回 值: TRUE/FALSE
*******************************************************************************/
static uint8_t i2c_check_device(uint8_t address)
{
  uint8_t Ack;
  
  i2c_start();    /* 发送启动信号 */
  /* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
  i2c_write(address | I2C_WR);
    Ack =  i2c_waitack();
  i2c_stop();      /* 发送停止信号 */
  return Ack;
}  

/*******************************************************************************
*  函 数 名: ee_CheckOk
*  功能说明: 判断串行EERPOM是否正常
*  形    参:  无
*  返 回 值: TRUE/FALSE
*******************************************************************************/
uint8_t fm24cxx_check(void)
{
  if (i2c_check_device(EE_DEV_ADDR) == TRUE)
  {
    return TRUE;
  }
  else
  {
    /* 失败后，切记发送I2C总线停止信号 */
    i2c_stop();
    return FALSE;
  }
}

/*******************************************************************************
*  函 数 名: fm24cxx_init
*  功能说明: 向串行EEPROM指定地址写入若干数据
*  形    参:  
*  返 回 值:
*******************************************************************************/
uint8_t fm24cxx_init(void)
{
//    uint8_t buf[16]= {0x55, 0xAA,0x33,};
//    uint8_t read_buf[16]= {00};
  soft_i2c_init();
    //自检
  if (fm24cxx_check() != TRUE)
  {
        return FALSE;
  }
    
//    fm24cxx_write(0,buf,16);
//    fm24cxx_read(0,read_buf,16);
//    memset(buf, 0xff, 16);
//    fm24cxx_write(0,buf,16);
//    fm24cxx_read(0,read_buf,16);
    return TRUE;
}

/*******************************************************************************
*  函 数 名: fm24cxx_write
*  功能说明: 向串行EEPROM指定地址写入若干数据
*  形    参:  addr : 起始地址
*       len : 数据长度，单位为字节
*       str : 存放写入数据的缓冲区指针
*  返 回 值:
*******************************************************************************/
uint8_t fm24cxx_write(uint16_t addr, uint8_t *str, uint16_t len)
{
    uint16_t i; 
            
    i2c_start();   
    #if EE_ADDR_A8 == 1
    i2c_write(EE_DEV_ADDR | I2C_WR | ((addr >> 7) & 0x0E));  /* 此处是写指令 */
    #else
    i2c_write(EE_DEV_ADDR | I2C_WR);  /* 此处是写指令 */
    #endif     
    if (i2c_waitack() != TRUE)
        goto fail;  /* EEPROM器件无应答 */         

    /* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
    if (EE_ADDR_BYTES == 1)
    {
        i2c_write((uint8_t)addr);
        if (i2c_waitack() != TRUE)
            goto fail;  /* EEPROM器件无应答 */
    }
    else
    {
        i2c_write(addr >> 8);
        if (i2c_waitack() != TRUE)
            goto fail;  /* EEPROM器件无应答 */
        i2c_write(addr);
        if (i2c_waitack() != TRUE)
            goto fail;  /* EEPROM器件无应答 */
    } 
    for(i=0; i<len; i++)
    {
        i2c_write(str[i]);
        if (i2c_waitack() != TRUE)
            goto fail;  /* EEPROM器件无应答 */
        addr++;     
    }
       
 
    i2c_stop();  
    //检查写入完成
    for (i = 0; i < 10; i++)
  {
    i2c_start();

    #if EE_ADDR_A8 == 1
        i2c_write(EE_DEV_ADDR | I2C_WR | ((addr >> 7) & 0x0E));  /* 此处是写指令 */
        #else
        i2c_write(EE_DEV_ADDR | I2C_WR);  /* 此处是写指令 */
        #endif     

    /* 第3步：发送一个时钟，判断器件是否正确应答 */
    if (i2c_waitack() == TRUE)
    {
      break;
    }
        i2c_delay_us(500);     //等待写入完成
  }
  if (i  >= 10)
  {
    goto fail;  /* EEPROM器件写超时 */
  }
    i2c_stop();
    return TRUE;
fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
  /* 发送I2C总线停止信号 */
  i2c_stop();
  return FALSE;
}
/*******************************************************************************
*  函 数 名: AT24CXX_ReadStr
*  功能说明: 从串行EEPROM指定地址处开始读取若干数据
*  形    参:  addr : 起始地址
*       len : 数据长度，单位为字节
*       str : 存放读到的数据的缓冲区指针
*  返 回 值: TRUE/FALSE
*******************************************************************************/
uint8_t fm24cxx_read(uint16_t addr, uint8_t *str, uint16_t len)
{
  i2c_start();

  #if EE_ADDR_A8 == 1
    i2c_write(EE_DEV_ADDR | I2C_WR | ((addr >> 7) & 0x0E));  /* 此处是写指令 */
  #else
    i2c_write(EE_DEV_ADDR | I2C_WR);  /* 此处是写指令 */
  #endif
    
    if (i2c_waitack() != TRUE)
  {
    goto fail;  /* EEPROM器件无应答 */
  }
    
  /* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
  if (EE_ADDR_BYTES == 1)
  {
    i2c_write((uint8_t)addr);
    if (i2c_waitack() != TRUE)
        {
            goto fail;  /* EEPROM器件无应答 */
        }
  }
  else
  {
    i2c_write(addr >> 8);
    if (i2c_waitack() != TRUE)
        {
            goto fail;  /* EEPROM器件无应答 */
        }

    i2c_write(addr);
    if (i2c_waitack() != TRUE)
        {
            goto fail;  /* EEPROM器件无应答 */
        }
  } 
    
  i2c_start();
    
    #if EE_ADDR_A8 == 1
    i2c_write(EE_DEV_ADDR | I2C_RD | ((addr >> 7) & 0x0E));  /* 此处是写指令 */
  #else
    i2c_write(EE_DEV_ADDR | I2C_RD);  /* 此处是写指令 */
  #endif
    if (i2c_waitack() != TRUE)
  {
    goto fail;  /* EEPROM器件无应答 */
  }
    
  while (--len)
  {
    *str = i2c_read_ack();
    str++;
  }
  *str = i2c_read_noack();
    i2c_stop();
    return TRUE;
fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
  /* 发送I2C总线停止信号 */
  i2c_stop();
  return FALSE;
}


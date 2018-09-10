#include "eslink.h"
#include "spi_flash.h"
#include "spi_flash_fs.h"
#include "ES_ISP.h"
#include "offline.h" 
#include "bootisp_prg_intf.h"
#include "swd_pgr_intf.h"



//es_target_cfg es_target_device;
//timing_info_t es_timing_info;

//for test
es_target_cfg es_target_device = {

    .code_start         = 0x00000000,       //CODE_AREA_START 
        .code_size          = 0x00040000,       //CODE_AREA_SIZE
//    .code_size          = 0x00200000,       //CODE_AREA_SIZE
    .config_start       = 0x00000000,       //CFG_AREA_START
    .config_size        = 0x00000260,       //CFG_AREA_SIZE
    .encrypt_addr       = 0x0007fffc,
    .encrypt_value      = 0x55aa55aa,
    .chipid_addr        = 0x000001fc,
    .chipid_value       = 0x0221067f,
};

timing_info_t es_timing_info = {
    .id          = 0x000000e7,
    .version     = 0x00000001,
//    .checksum    = 0xa5c05a3f,    
    .checksum    = 0x0000a70a,    
};

// Delay for specified time
//    delay:  delay time in ms
void es_delay_ms(uint32_t delay) 
{
    delay *= ((SystemCoreClock/1000U) + (ES_DELAY_SLOW_CYCLES-1U)) / ES_DELAY_SLOW_CYCLES;
    ES_DELAY_SLOW(delay);
}
// Delay for specified time
//    delay:  delay time in us
void es_delay_us(uint32_t delay) 
{
    delay *= ((SystemCoreClock/1000000U) + (ES_DELAY_SLOW_CYCLES-1U)) / ES_DELAY_SLOW_CYCLES;
    ES_DELAY_SLOW(delay);
}



uint8_t eslink_init(void)
{
    //TODO - DAP_Setup puts GPIO pins in a hi-z state which can
    //       cause problems on re-init.  This needs to be investigated
    //       and fixed.
    ISP_SETUP();
    PORT_ISP_SETUP();
    return 1;
    
}

uint8_t eslink_off(void)
{
    PORT_ISP_OFF();
    return 1;
    
}
//设置目标芯片电压
void eslink_set_trget_power(trget_power_t power)
{
    if(power == TRGET_POWER_DISABLE){
        V33_OFF();     
        V5_OFF();        
    }else if(power == TRGET_POWER_3V3){
        V33_ON();       
        V5_OFF();                 
    }else if(power == TRGET_POWER_5V){
        V33_OFF();       
        V5_ON();      
    }        
}

trget_power_t eslink_get_trget_power()
{
    
    
}


//TODO 把内部flash 和外部flash  抽象成结构体 （参考 keil flm文件）
//优点：更改芯片和外部flash 时只需要修改此结构体的函数指针即可
//eslink 芯片内部flash 数据存取
//写入数据前需要擦除的动作
void es_flash_write(uint32_t addr, uint32_t size, uint32_t *buf)
{
    
    
}
//读取K22芯片内部flash 数据
void es_flash_read(uint32_t addr, uint32_t size, uint8_t *buf)
{
    
    
}


//获取用户hex长度
static uint32_t get_hex_size(void)
{
   // return es_target_device.code_size ;
    return 0x40000;
}

#define FLASH_READ_SIZE    1024
typedef struct {
    uint8_t buf[FLASH_READ_SIZE];
//    uint8_t pos;            //position
    uint32_t addr;
} flash_state_t;


//默认数据保存起始地址都为扇区起始地址，暂时不考虑写入地址为非扇区首地址的情况
error_t ol_prj_write(uint32_t addr, const uint8_t *buf,uint32_t size )
{
    uint32_t erase_size = 0;
    //只考虑size不为扇区整数倍的情况
    if(addr == SF_USER_HEX_ADDR)
    {
        erase_size =  get_hex_size();
        if (erase_size % 0x10000)
        {
            erase_size = (erase_size+0xffff) / 0x10000  * 0x10000;
        }        
        //TODO:写入地址不为扇区首地址
        if(sf_erase_block( SF_USER_HEX_ADDR,  erase_size)  != 0)
            return ERROR_SF_ERASE;
    }  
    else if( addr == SF_USER_HEX_CHECKOUT_ADDR)
    {
        erase_size = 0x1000;        //TODO:修改默认长度
        if(sf_erase_sector( SF_USER_HEX_CHECKOUT_ADDR,  erase_size)  != 0)
            return ERROR_SF_ERASE;          
    }
        
    if(spi_flash_write(addr, buf,size) != 0)
        return ERROR_SF_WRITE;  
    if(sf_CmpData(addr, buf, size) != 0)
        return ERROR_SF_WRITE;

    return ERROR_SUCCESS;      
}
//返回读到的数据长度
error_t ol_prj_read(uint32_t addr,  uint8_t *buf, uint32_t size)
{
    
    spi_flash_read(addr, buf, size );
    return ERROR_SUCCESS;
    
}  

//void flash_set_target_config(flash_t *obj)
/*
 *  读固件版本
 *  固件版本信息保存在K22内部flash中。
 *  固件版本不对应，则更新固件
 */
error_t es_read_firmware_version(uint8_t *buf)
{
    uint8_t *version;
    uint8_t i;    
    error_t result = ERROR_SUCCESS;    
    
    version = (uint8_t*)FIRMWARE_VERSION_ADDR;
    for(i=0; i<FIRMWARE_VERSION_LEN; i++){
        buf[i] =  *(version + i);    
    }
    
    return result;  
}

    
/*
 *  读芯片时序信息。时序号+时序版本+时序校验和保存K22内部falsh中。
 *  后面的信息保存在外部SPI falsh中
 */
error_t isp_read_timinginfo(uint8_t *buf)
{
    uint8_t i;
    uint8_t *info;
    error_t result = ERROR_SUCCESS;
    
    info = (uint8_t*)CHIP_TIMING_INFO_ADDR;
    for(i=0; i<TARGET_TIMING_INFO_LEN; i++){
        buf[i] =  *(info + i);    
    }				
    ol_prj_read(SF_CHIP_INFO_ADDR, &buf[i],TARGET_IC_INFO_LEN);	
    
    return result;  
}

static uint32_t es_prg_intf = PRG_INTF_ISP;
//配置烧录接口
error_t es_download_prg_intf(const uint8_t *buf)
{
    uint32_t prg_temp = 0;
    prg_temp = (*(buf+0)) | (*(buf+1) << 8) | (*(buf+2) << 16) | (*(buf+3) << 24);
    if( (prg_temp ==  PRG_INTF_ISP) | (prg_temp ==  PRG_INTF_SWD) | (prg_temp ==  PRG_INTF_BOOTISP) )
    {
        es_prg_intf = prg_temp;
        return  ERROR_SUCCESS;
    }
    else
    {
         return ERROR_INTF_ERROR;
    }    
}
/*
 *  时序信息下载    
 *  下载时序到spi flash 中。
 *  读spi-flash中的数据，与上位机选择的目标芯片信息做比较，如果不一致则更新时序。并保存时序到SPI Flash 中
 */
error_t es_download_timinginfo(uint8_t *data)
{
//    error_t result = ERROR_SUCCESS;
//    uint8_t info[TARGET_IC_INFO_LEN] = {0};
//    
//    ol_prj_read(SF_CHIP_INFO_ADDR, info,TARGET_IC_INFO_LEN);
//    //不相等
//    if(memcmp(info, buf, TARGET_IC_INFO_LEN) != 0){
//        if(ol_prj_write(SF_CHIP_INFO_ADDR,  buf,TARGET_IC_INFO_LEN)!= SF_SUCCESS)
//            result = UPDATAFAIL;
//    }   
//    return result;  
    error_t result = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
    
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);
    if(ol_prj_write(SF_TIMING_INFO_ADDR + addr,  (data+8), size) != 0)
        result = USERHEX_DL_FAIL; 
    return result;  
}
error_t es_download_timing(const uint8_t *data)
{
    error_t result = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
    
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);
    if(ol_prj_write(SF_TIMING_ADDR + addr, (data+8), size) != 0)
        result = ERROR_CONFIG_DL; 
    return result;            
}
error_t es_download_timing_end(const uint8_t *data)
{
    
    
}



/********************************数据下载***************************************/
/*
 *  配置字下载
 *  下载配置字到spi-flash中
 */
error_t es_download_config(const uint8_t *data)
{
    error_t result = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
    
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);
    if(ol_prj_write(SF_CONFIG_INFO_ADDR + addr, (data+8), size) != 0)
        result = ERROR_CONFIG_DL; 
    return result;            
}

/*
 * 用户程序下载
 * 下载用户程序到spi-flash中
* 上位机从地址0x00开始下载，下载数据长度位目标芯片的空间。
* 上位机每帧下载长度0x0308=地址+长度+ 数据,数据长度位0x0300
 */
error_t es_download_hex(uint8_t *data)
{
    error_t result = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
    
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);
//    if(ol_prj_write(SF_USER_HEX_ADDR+addr,  (data+8), size) != 0)
//        result = USERHEX_DL_FAIL;
    if(ol_file_write(addr,(data+8), size) != 0)
        result = USERHEX_DL_FAIL;
//    if(fs_cmp_data( addr,(data+8), size) != 0)
//        result = USERHEX_DL_FAIL;
    return result;        
}
/*
 *  用户程序下载结束
 *  保存上位机下发的校验和到spi flash中。读取spi-flash中的用户程序,计算累加和和CRC校验，
 *  与上位机下发的校验做比较'
 * 下发的数据为“用户程序校验和（4bytes）+用户程序CRC32（4bytes）”
 * 校验和的地位为“和校验”
 */
error_t es_download_hex_end(uint8_t *data)
{
//    uint8_t dl_checksum[8] = {0};   //上位机下发的校验
    uint32_t checksum = 0;  
    uint32_t code_szie = get_hex_size();
    uint32_t read_addr = 0;
    uint32_t read_size = 0;
    uint8_t read_buf[1024] = {0};
    error_t result = ERROR_SUCCESS;         
    
    while(code_szie > 0){      
        read_size = MIN(code_szie, sizeof(read_buf));
//        ol_prj_read(SF_USER_HEX_ADDR + read_addr,  read_buf, read_size);
        if(ol_file_read(read_addr,  read_buf, read_size) != 0)
            return USERHEX_DL_FAIL;
        checksum += check_sum(read_size, read_buf);
        
        read_addr += read_size;
        code_szie -= read_size;
    }
    if( (data[0] == ((checksum>>0)&0xFF)) && 
        (data[1] == ((checksum>>8)&0xFF)) &&
        (data[2] == ((checksum>>16)&0xFF)) &&
        (data[3] == ((checksum>>24)&0xFF)) )
    {
         ol_prj_write(SF_USER_HEX_CHECKOUT_ADDR, data, 4);      //验证正确后在保存数据   
    }
    else
    {
         result = USERHEX_DL_FAIL;
    }          
    //TODO:
    //实现CRC校验
    return result;        
}





/********************************联机操作***************************************/
//擦除
error_t es_erase_chip(uint8_t *data)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t erase_mode;
    
    erase_mode = data[0];
//    ret = isp_erase_chip(erase_mode);
    ret = bootisp_erase_chip(erase_mode);
//    ret = swd_erase_chip();
    return ret;
}
//查空
error_t es_check_empty(void)
{
    error_t ret = ERROR_SUCCESS;
    ret = bootisp_check_empty();
//  ret = isp_check_empty();
    
    return ret;
}


//编程   
error_t es_program(void)
{
    uint8_t ret = ERROR_SUCCESS;
    flash_state_t read_flash;
    uint32_t hex_size;    
    uint32_t prg_addr;  //编程地址
    uint32_t copy_size;
    
    hex_size =  get_hex_size();
    prg_addr = 0;
    read_flash.addr =  SF_USER_HEX_ADDR;
    
    while(true)
    {
        copy_size = MIN(hex_size, FLASH_READ_SIZE );
        
        ret = ol_prj_read( read_flash.addr, read_flash.buf , copy_size);
        if(ret !=  ERROR_SUCCESS)
            return ret;
//        bootisp_program( prg_addr, read_flash.buf,  copy_size);
        ret = swd_prg_program(prg_addr, read_flash.buf,  copy_size);
        if( ret !=  ERROR_SUCCESS)
            return ret;
        // Update variables
        prg_addr += copy_size;
        hex_size -= copy_size;
        read_flash.addr += copy_size;
        // Check for end
        if (hex_size <= 0) {
            break;
        }         
    }
    return  ERROR_SUCCESS;
}


//获取配置字长度
static uint32_t get_config_size(void)
{      
    return SF_CONFIG_INFO_ADDR;
}
//获取配置字地址
static uint32_t get_config_addr(void)
{        
    return SF_CONFIG_INFO_SIZE;
}

/*
 *  配置字编程
 *  配置字长度默认为1024
 */
error_t es_program_config(void)
{
    flash_state_t read_flash;
    uint32_t cfg_size;    
    uint32_t copy_size;
    uint32_t prg_addr;  //编程地址
    
    cfg_size =  get_config_size();
    prg_addr = 0;
    read_flash.addr =  get_config_addr();    
    
    ol_prj_read( read_flash.addr, read_flash.buf , copy_size);
    
}
error_t es_read_config(uint8_t *data)
{
    uint32_t addr;
    uint32_t size;
//     isp_read_config();
}
//校验

static  struct prj_partition prj_part ;
/*变量定义*/
ISP_DATA_t isp_data; 

uint32_t es_process_command(uint8_t *request, uint8_t *response)
{
    uint8_t temp;
    uint8_t result = ERROR_SUCCESS;
    uint32_t ack_len = 512; //isp通信采用变长协议，用来返回回复数据的长度
    uint32_t sum;
    
    isp_data.wrbuf = (uint32_t_uint8_t*)request;
    isp_data.rdbuf = (uint32_t_uint8_t*)response;
    
    if( ((*(request+0) << 0) | (*(request+1) << 8) | (*(request+2) << 16) | (*(request+3) << 24)) != ISP_FRAME_HEAD){
        result = FRAMEHEADERROR;
        goto __ACK;
    }
    isp_data.frame_head = ISP_FRAME_HEAD;
  
    if(*(request+4) != ESLINKII_DEVICE_TYPE){
        result = DEVICETYPEERROR;
        goto __ACK;
    }

    isp_data.device_type = *(request+4);
    isp_data.fun_code = *(request+5);
    isp_data.data_length = (*(request+6) << 0) | (*(request+7) << 8);
    
    memset(isp_data.rdbuf->B08, 0, ISP_PACKET_SIZE);   
    
    switch(isp_data.fun_code){   
        case ID_HANDSHAKE:
            isp_data.data_length = 0;
            break;
        //------------------------Boot判断--------------------------------
        case ID_READ_BOOT_VERSION:         //0xD7  
//            error_info.result = es_read_firmware_version(&isp_data.data_buf->B08[8]);
//            isp_data.data_length = FIRMWARE_VERSION_LEN;

            //add by 9527 for test: 设置固件版本为固定值
            isp_data.data_length = FIRMWARE_VERSION_LEN;
            isp_data.rdbuf->B08[8] = 0x01;
            isp_data.rdbuf->B08[9] = 0x01;
            isp_data.rdbuf->B08[10] = 0x00;
            isp_data.rdbuf->B08[11] = 0x00;
            break;                     

        //------------------------时序判断--------------------------------
        case ID_READ_TIMING_INFO:       //0x28 读芯片信息
//            error_info.result =  isp_read_timinginfo(&isp_data.data_buf->B08[8]);
//            isp_data.data_length = TARGET_IC_INFO_LEN + TARGET_TIMING_INFO_LEN; 
//            isp_data.data_length = sizeof(es_target_device);        
            //add by 9527 for test
            isp_data.data_length = sizeof(es_target_device)+sizeof(es_timing_info);
            isp_data.rdbuf->B32[2] = es_timing_info.id;
            isp_data.rdbuf->B32[3] = es_timing_info.version;
            isp_data.rdbuf->B32[4] = es_timing_info.checksum;
            isp_data.rdbuf->B32[5] = es_target_device.code_start;
            isp_data.rdbuf->B32[6] = es_target_device.code_size;
            isp_data.rdbuf->B32[7] = es_target_device.config_start;
            isp_data.rdbuf->B32[8] = es_target_device.config_size;
            isp_data.rdbuf->B32[9] = es_target_device.encrypt_addr;
            isp_data.rdbuf->B32[10] = es_target_device.encrypt_value;
            isp_data.rdbuf->B32[11] = es_target_device.chipid_addr;
            isp_data.rdbuf->B32[12] = es_target_device.chipid_value;
            break; 
         case ID_DL_TIMING_INFO:             //0x27
            //读spi-flash中的数据，与上位机选择的目标芯片信息做比较，如果不一致则更新spi flash中的时序信息。
//            error_info.result = isp_download_timinginfo(&isp_data.data_buf->B08[20]);
            isp_data.data_length = 0;
            //add by 9527 for test
            if(result == ERROR_SUCCESS){
                es_target_device.code_start   = isp_data.wrbuf->B32[5];
                es_target_device.code_size    = isp_data.wrbuf->B32[6];
                es_target_device.config_start  = isp_data.wrbuf->B32[7];
                es_target_device.config_size   = isp_data.wrbuf->B32[8];
                es_target_device.encrypt_addr   = isp_data.wrbuf->B32[9];
                es_target_device.encrypt_value  = isp_data.wrbuf->B32[10];
                es_target_device.chipid_addr   = isp_data.wrbuf->B32[11];
                es_target_device.chipid_value  = isp_data.wrbuf->B32[12];                
            }
//            timing_info_init();
            break;
            
        //上位机判断时序需要更新，下发新的时序，在APP中擦除时序信息，跳转到BOOT中
        case ID_DL_TIMING_START:            //0x1B.下载时序开始
            
            //擦除芯片信息
            break;
        
        case ID_DL_TIMING:                  //0x19
            //es_download_timing();
            ack_len = 1024;
            break;
        
        case ID_DL_TIMING_END:             //0x1C.时序下载结束
            //时序下载完成后，更新app等
            break;
        //----------------------------------------------------------------------
        
        case ID_DL_PRG_INTF:         //下载烧录接口  0x31
            result = es_download_prg_intf(&isp_data.wrbuf->B08[ES_DATA_OFFSET]);
            isp_data.data_length = 0;
            break;
        case ID_DL_CONFIG:              //配置字下载//0x17
//            result = es_download_config(&isp_data.wrbuf->B08[8]);
        
            result = ERROR_SUCCESS;
            isp_data.data_length = 0;
            break;
        case ID_DL_USERHEX:                     //0x18
            result = es_download_hex(&isp_data.wrbuf->B08[ES_DATA_OFFSET]);
            isp_data.data_length = 0;
            ack_len = 1024;
            break;
        case ID_DL_USERHEX_END:                //0x1D
            result = es_download_hex_end(&isp_data.wrbuf->B08[ES_DATA_OFFSET]);
            isp_data.data_length = 0;
        
            break;

        //-----------------联机操作---------------------------------------------
        case ID_FULL_ERASE:	        //0x20
            result = es_erase_chip(&isp_data.wrbuf->B08[ES_DATA_OFFSET]);  
            isp_data.data_length = 0;
            break;        
        case ID_CHECK_EMPTY:            //0x21
            //	GPIO_ResetBits(GPIOB,YEL_LED_PIN);
            result = es_check_empty();
            isp_data.data_length = 0;
            break;
        case ID_PRG_HEX:                //0x22
//            if(isp_data.wrbuf->B08[12] == 0)
//                isp_serial_number.enable_flag = DISABLE;
//            else if(isp_data.wrbuf->B08[12] == 1)
//                isp_serial_number.enable_flag = ENABLE;
//            result = isp_program_hex(SF_USER_HEX_ADDR, es_target_device.code_size);
            result = es_program();
            isp_data.data_length = 0;
            break;
        case ID_VERIFY:                //0x23
//            result = isp_verify( SF_USER_HEX_ADDR, es_target_device.code_size);
            isp_data.data_length = 0;
            break;
        case ID_ENCRYPT:               //0x24
//            result = isp_encrypt_chip();        
            isp_data.data_length = 0;
            break;
        case ID_READ_CONFIG:           //0x25     
                       
            result = es_read_config(isp_data.wrbuf->B08[ES_DATA_OFFSET]);
            isp_data.data_length = (uint16_t)(isp_data.wrbuf->B32[3] + 4);  
            break;

        case ID_PRG_CONFIG:             //0x26
            //result = isp_programe_config();
        result = ERROR_SUCCESS;
            isp_data.data_length = 0;
            break;

        case ID_READ_CHIPID:	   //0x29
//            result = isp_read_chipid(&isp_data.rdbuf->B32[2] );
            isp_data.data_length = 8;  
            break;        
        case ID_READ_FLASH:         //0x2A 读目标芯片flash
//            isp_data.data_length = (uint16_t)(isp_data.wrbuf->B32[3] + 4);      
//            result = isp_read_rom(isp_data.wrbuf->B32[2],isp_data.wrbuf->B32[3],&isp_data.rdbuf->B32[3]);   
        
            result = ERROR_SUCCESS;
            isp_data.data_length = 0;
            ack_len = 1024;
            break;
        case ID_DL_SERIALNUM:	                //0x2B 下载序列号代码
//            isp_serial_number.enable_flag = ENABLE;
//            isp_serial_number.addr  = isp_data.wrbuf->B32[2];
//            isp_serial_number.size  = isp_data.wrbuf->B32[3];
//            isp_serial_number.data[0] = isp_data.wrbuf->B32[4];
//            isp_serial_number.data[1] = isp_data.wrbuf->B32[5];
            isp_data.data_length = 0;  
            break;

       
        case ID_DL_OFFLINE_PRJ_INFO :             
//            ofl_prj_write_info(&isp_data.wrbuf->B08[8], isp_data.data_length);   
//            prj_part.type =  INFIO_PART;  
//            prj_part.start = 0;
//            prj_part.size =  isp_data.data_length;
//            ofl_prj_write_part(&prj_part);
        
            isp_data.data_length = 0x00;
            result = ERROR_SUCCESS;
            break;
        case ID_READ_OFFLINE_PRJ_NUM :          //读方案数量
            isp_data.data_length = 0x04;
//            isp_data.rdbuf[ES_DATA_OFFSET] = ofl_prj_get_num() ;
//            isp_data.rdbuf[ES_DATA_OFFSET+1] = 0;
//            isp_data.rdbuf[ES_DATA_OFFSET+2] = 0;
//            isp_data.rdbuf[ES_DATA_OFFSET+3] = 0;
//            isp_data.rdbuf->B08[8] = ofl_get_prj_num() ;
//            isp_data.rdbuf->B08[9] = 0;
//            isp_data.rdbuf->B08[10] = 0;
//            isp_data.rdbuf->B08[11] = 0;
//            result = ERROR_SUCCESS;
            break;
        case ID_READ_OFFLINE_PRJ_INFO :         //读方案信息
            isp_data.data_length = 0x04; 
           
            result = ERROR_SUCCESS;
            break;
        case ID_DL_OFFLINE_PRJ_TIMING_START :   //脱机方案时序下载开始    0x31
//            prj_part.type =  TIMING_INFO_PART;  
//            prj_part.size =  isp_data.data_length;        
//            ofl_prj_write_part(&prj_part);
//        
//            ofl_prj_write_data(&isp_data.wrbuf->B08[ES_DATA_OFFSET], isp_data.data_length); 
//            result = es_download_timinginfo(&isp_data.wrbuf->B08[ES_DATA_OFFSET]) ;
            isp_data.data_length = 0;        
            result = ERROR_SUCCESS;
            
            break;
        case ID_DL_OFFLINE_PRJ_TIMING_ING :     //脱机方案时序下载              
           
//            ofl_prj_write_data(&isp_data.wrbuf->B08[ES_DATA_OFFSET], isp_data.data_length);
        
//            prj_part.data += check_sum( isp_data.data_length-8 , &isp_data.wrbuf->B08[16] ) ;  
//            prj_part.size +=  isp_data.data_length;    
            result = es_download_timing(&isp_data.wrbuf->B08[ES_DATA_OFFSET]) ;
            isp_data.data_length = 0;        
            result = ERROR_SUCCESS;
            ack_len = 1024;
            break;
        case ID_DL_OFFLINE_PRJ_TIMING_END :     //脱机方案时序结束
            prj_part.type =  TIMING_PART;             
//            ofl_prj_write_part(&prj_part);

////            isp_data.rdbuf[8] = (ofl_pcb.part.data & 0xff000000) >> 24;
////            isp_data.rdbuf[9] = (ofl_pcb.part.data & 0x00ff0000) >> 16;
////            isp_data.rdbuf[10] = (ofl_pcb.part.data & 0x0000ff00) >> 8;
////            isp_data.rdbuf[11] = (ofl_pcb.part.data & 0x000000ff);
//            isp_data.rdbuf->B08[8] = (prj_part.data & 0xff000000) >> 24;
//            isp_data.rdbuf->B08[9] = (prj_part.data & 0x00ff0000) >> 16;
//            isp_data.rdbuf->B08[10] = (prj_part.data & 0x0000ff00) >> 8;
//            isp_data.rdbuf->B08[11] = (prj_part.data & 0x000000ff);
            isp_data.data_length = 4;        
            result = ERROR_SUCCESS;
            break;
        case ID_DL_OFFLINE_PRJ_CONFIG :         //脱机方案配置字下载
//            prj_part.type =  CONFIG_PART;  
//            prj_part.size =  isp_data.data_length;
//            ofl_prj_write_part(&prj_part);
//        
//            ofl_prj_write_data(&isp_data.wrbuf->B08[ES_DATA_OFFSET], isp_data.data_length); 
            isp_data.data_length = 0;        
            result = ERROR_SUCCESS;
            ack_len = 1024;
            break;
        case ID_DL_OFFLINE_PRJ_HEX :            //脱机方案用户HEX
//            ofl_prj_write_data(&isp_data.wrbuf->B08[ES_DATA_OFFSET], isp_data.data_length);
//        
//            prj_part.data += check_sum( isp_data.data_length-8 , &isp_data.wrbuf->B08[ES_DATA_OFFSET] ) ;  
//            prj_part.size +=  isp_data.data_length;  
        
            isp_data.data_length = 0x00;
            result = ERROR_SUCCESS;
            ack_len = 1024;
            break;
        case ID_DL_OFFLINE_PRJ_HEX_END :        //脱机方案用户HEX下载完成
//            sum = (isp_data.wrbuf[ES_DATA_OFFSET]      << 24) |
//                    (isp_data.wrbuf[ES_DATA_OFFSET+1] << 16) |
//                    (isp_data.wrbuf[ES_DATA_OFFSET+2] << 8 ) |
//                    (isp_data.wrbuf[ES_DATA_OFFSET+3]  << 0);
        
        
//            if(prj_part.data !=  isp_data.wrbuf->B32[2])
//            {
//                isp_data.data_length = 0x04;
//                result = ERROR_HEX_CHECKSUM_FAIL;
//            }
//            else
            {
//                prj_part.type =  HEX_PART;             
//                ofl_prj_write_part(&prj_part);
       
                isp_data.data_length = 0x00;
                result = ERROR_SUCCESS;
            }
            
            break;
        
        default:
            break;
    }

//    memset(isp_data.data_buf, 0, ISP_PACKET_SIZE);   

    
    __ACK:
    isp_data.rdbuf->B32[0] = ISP_FRAME_HEAD;
    isp_data.rdbuf->B08[4] = ESLINKII_DEVICE_TYPE;
    isp_data.rdbuf->B08[5] = isp_data.fun_code;
	isp_data.state = ISP_RUN_OK; 
    if(result != ERROR_SUCCESS)
    {
        if(result == FRAMEHEADERROR)
        {
            isp_data.rdbuf->B08[5] = 0x00;
            isp_data.data_length = 0x00;    
        } 
        else if(result == CHECHEMPTYFAIL)
        {
            isp_data.data_length = 0x08;           
//            isp_data.rdbuf->B32[2] = isp_process_err.addr ;
//            isp_data.rdbuf->B32[3] = isp_process_err.data;
        }
        else if(result == HEXPRGFAIL)
        {            
            isp_data.data_length = 0x04;    
//            isp_data.rdbuf->B32[2] = isp_process_err.addr ;                                  
                    	
        }
        else if(result == CODEVERIFYFAIL)
        {
            isp_data.data_length = 0x08;                                          
//            isp_data.rdbuf->B32[2] = isp_process_err.addr ;
//            isp_data.rdbuf->B32[3] = isp_process_err.data;
        }
        isp_data.state = result;
    }        
    
    isp_data.rdbuf->B08[6] = (uint8_t)(isp_data.data_length&0xFF);    
    isp_data.rdbuf->B08[7] = (uint8_t)(isp_data.data_length>>8); 
    isp_data.rdbuf->B08[isp_data.data_length+8] = isp_data.state;    
    isp_data.checksum = check_sum(isp_data.data_length +9, isp_data.rdbuf->B08);
    isp_data.rdbuf->B08[isp_data.data_length + 9] = (uint8_t)(isp_data.checksum&0xFF);            
    isp_data.rdbuf->B08[isp_data.data_length + 10] = (uint8_t)(isp_data.checksum>>8); 
    
    return ack_len;
}


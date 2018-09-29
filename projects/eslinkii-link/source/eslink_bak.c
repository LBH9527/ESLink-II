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
//编程校验
error_t es_program_verify()
{
    error_t ret = ERROR_SUCCESS;
//  ret = isp_program_verify();
    
    return ret;

}

static  struct prj_partition prj_part ;
/*变量定义*/
prog_comm_frame_t prog_data; 

uint32_t es_process_command(uint8_t *request, uint8_t *response)
{
    uint8_t temp;
    uint8_t result = ERROR_SUCCESS;
    uint32_t ack_len = 512;         //isp通信采用变长协议，用来返回回复数据的长度
    uint32_t sum;
    
    prog_data.wrbuf = request;
    prog_data.rdbuf = response;     

    if( (*request != 0xBA) | (*(request+1) != 0xDC) | (*(request+2) != 0xcd) | (*(request+3) != 0xAB) )
    {
        result = FRAMEHEADERROR;
        goto __ACK;
    }
    if(*(request+4) != ESLINKII_DEVICE_TYPE){
        result = DEVICETYPEERROR;
        goto __ACK;
    }

    prog_data.device_type   = *(request+4);
    prog_data.fun_code      = *(request+5);
    prog_data.data_length   = (*(request+6) << 0) | (*(request+7) << 8);
    
    memset(prog_data.rdbuf, 0, PROG_FRAME_SIZE);    
    
    switch(prog_data.fun_code){   
        case ID_HANDSHAKE:
            prog_data.data_length = 0;
            break;
        //------------------------Boot判断--------------------------------
        case ID_READ_OFL_PRJ_VERSION:           //0xD7 读脱机工程版本 
            result = read_offline_version(&prog_data.rdbuf[8]);
            prog_data.data_length = sizeof(offline_info_t);
            break;                     
        case ID_DL_OFL_PRJ_START:               //0xD8  脱机工程下载开始  
            prog_data.data_length = 0;    
            break;
        case ID_DL_OFL_PRJ_HEX:                 //0xD9 下载脱机工程HEX  //1024
            ack_len = 1024;
            prog_data.data_length = 0;    
            break;
        case ID_DL_OFL_PRJ_HEXEND:              //0xDA 下载结束  
            prog_data.data_length = 0;                
            break;            

        //------------------------时序判断--------------------------------
        case ID_READ_TIMING_INFO:       //0x28 读芯片信息 
            break; 
         case ID_DL_TIMING_INFO:             //0x27 
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
            result = es_download_prg_intf(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;
            break;
        case ID_DL_CONFIG:              //配置字下载//0x17
            result = es_download_config(&prog_data.wrbuf->B08[8]);
        
            result = ERROR_SUCCESS;
            prog_data.data_length = 0;
            break;
       case ID_READ_CONFIG:           //0x25    
                       
            result = es_read_config(prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = (uint16_t)(prog_data.wrbuf->B32[3] + 4);  
            break;

        case ID_PRG_CONFIG:             //0x26
            //result = isp_programe_config();
            result = ERROR_SUCCESS;
            prog_data.data_length = 0;
            break;
        case ID_DL_USERHEX:                     //0x18
            result = es_download_hex(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;
            ack_len = 1024;
            break;
        case ID_DL_USERHEX_END:                //0x1D
            result = es_download_hex_end(&prog_data.wrbuf[ES_DATA_OFFSET]);
            prog_data.data_length = 0;
        
            break;

        //-----------------联机操作---------------------------------------------
        case ID_FULL_ERASE:	        //0x20
            result = es_erase_chip(&prog_data.wrbuf[ES_DATA_OFFSET]);  
            prog_data.data_length = 0;
            break;        
        case ID_CHECK_EMPTY:            //0x21
            //	GPIO_ResetBits(GPIOB,YEL_LED_PIN);
            result = es_check_empty();
            prog_data.data_length = 0;
            break;
        case ID_PRG_HEX:                //0x22
//            if(prog_data.wrbuf->B08[12] == 0)
//                isp_serial_number.enable_flag = DISABLE;
//            else if(prog_data.wrbuf->B08[12] == 1)
//                isp_serial_number.enable_flag = ENABLE;
//            result = isp_program_hex(SF_USER_HEX_ADDR, es_target_device.code_size);
            result = es_program();
            prog_data.data_length = 0;
            break;
        case ID_VERIFY:                //0x23
//            result = isp_verify( SF_USER_HEX_ADDR, es_target_device.code_size);
            prog_data.data_length = 0;
            break;
        case ID_ENCRYPT:               //0x24
//            result = isp_encrypt_chip();        
            prog_data.data_length = 0;
            break;


        case ID_READ_CHIPID:	   //0x29
//            result = isp_read_chipid(&prog_data.rdbuf->B32[2] );
            prog_data.data_length = 8;  
            break;        
        case ID_READ_FLASH:         //0x2A 读目标芯片flash
//            prog_data.data_length = (uint16_t)(prog_data.wrbuf->B32[3] + 4);      
//            result = isp_read_rom(prog_data.wrbuf->B32[2],prog_data.wrbuf->B32[3],&prog_data.rdbuf->B32[3]);   
        
            result = ERROR_SUCCESS;
            prog_data.data_length = 0;
            ack_len = 1024;
            break;
        case ID_DL_SERIALNUM:	                //0x2B 下载序列号代码
//            isp_serial_number.enable_flag = ENABLE;
//            isp_serial_number.addr  = prog_data.wrbuf->B32[2];
//            isp_serial_number.size  = prog_data.wrbuf->B32[3];
//            isp_serial_number.data[0] = prog_data.wrbuf->B32[4];
//            isp_serial_number.data[1] = prog_data.wrbuf->B32[5];
            prog_data.data_length = 0;  
            break;

       
        case ID_DL_OFFLINE_PRJ_INFO :             
//            ofl_prj_write_info(&prog_data.wrbuf->B08[8], prog_data.data_length);   
//            prj_part.type =  INFIO_PART;  
//            prj_part.start = 0;
//            prj_part.size =  prog_data.data_length;
//            ofl_prj_write_part(&prj_part);
        
            prog_data.data_length = 0x00;
            result = ERROR_SUCCESS;
            break;
        case ID_READ_OFFLINE_PRJ_NUM :          //读方案数量
            prog_data.data_length = 0x04;
//            prog_data.rdbuf[ES_DATA_OFFSET] = ofl_prj_get_num() ;
//            prog_data.rdbuf[ES_DATA_OFFSET+1] = 0;
//            prog_data.rdbuf[ES_DATA_OFFSET+2] = 0;
//            prog_data.rdbuf[ES_DATA_OFFSET+3] = 0;
//            prog_data.rdbuf->B08[8] = ofl_get_prj_num() ;
//            prog_data.rdbuf->B08[9] = 0;
//            prog_data.rdbuf->B08[10] = 0;
//            prog_data.rdbuf->B08[11] = 0;
//            result = ERROR_SUCCESS;
            break;
        case ID_READ_OFFLINE_PRJ_INFO :         //读方案信息
            prog_data.data_length = 0x04; 
           
            result = ERROR_SUCCESS;
            break;
        case ID_DL_OFFLINE_PRJ_TIMING_START :   //脱机方案时序下载开始    0x31
//            prj_part.type =  TIMING_INFO_PART;  
//            prj_part.size =  prog_data.data_length;        
//            ofl_prj_write_part(&prj_part);
//        
//            ofl_prj_write_data(&prog_data.wrbuf->B08[ES_DATA_OFFSET], prog_data.data_length); 
//            result = es_download_timinginfo(&prog_data.wrbuf->B08[ES_DATA_OFFSET]) ;
            prog_data.data_length = 0;        
            result = ERROR_SUCCESS;
            
            break;
        case ID_DL_OFFLINE_PRJ_TIMING_ING :     //脱机方案时序下载              
           
//            ofl_prj_write_data(&prog_data.wrbuf->B08[ES_DATA_OFFSET], prog_data.data_length);
        
//            prj_part.data += check_sum( prog_data.data_length-8 , &prog_data.wrbuf->B08[16] ) ;  
//            prj_part.size +=  prog_data.data_length;    
            result = es_download_timing(&prog_data.wrbuf[ES_DATA_OFFSET]) ;
            prog_data.data_length = 0;        
            result = ERROR_SUCCESS;
            ack_len = 1024;
            break;
        case ID_DL_OFFLINE_PRJ_TIMING_END :     //脱机方案时序结束
            prj_part.type =  TIMING_PART;             
//            ofl_prj_write_part(&prj_part);

////            prog_data.rdbuf[8] = (ofl_pcb.part.data & 0xff000000) >> 24;
////            prog_data.rdbuf[9] = (ofl_pcb.part.data & 0x00ff0000) >> 16;
////            prog_data.rdbuf[10] = (ofl_pcb.part.data & 0x0000ff00) >> 8;
////            prog_data.rdbuf[11] = (ofl_pcb.part.data & 0x000000ff);
//            prog_data.rdbuf->B08[8] = (prj_part.data & 0xff000000) >> 24;
//            prog_data.rdbuf->B08[9] = (prj_part.data & 0x00ff0000) >> 16;
//            prog_data.rdbuf->B08[10] = (prj_part.data & 0x0000ff00) >> 8;
//            prog_data.rdbuf->B08[11] = (prj_part.data & 0x000000ff);
            prog_data.data_length = 4;        
            result = ERROR_SUCCESS;
            break;
        case ID_DL_OFFLINE_PRJ_CONFIG :         //脱机方案配置字下载
//            prj_part.type =  CONFIG_PART;  
//            prj_part.size =  prog_data.data_length;
//            ofl_prj_write_part(&prj_part);
//        
//            ofl_prj_write_data(&prog_data.wrbuf->B08[ES_DATA_OFFSET], prog_data.data_length); 
            prog_data.data_length = 0;        
            result = ERROR_SUCCESS;
            ack_len = 1024;
            break;
        case ID_DL_OFFLINE_PRJ_HEX :            //脱机方案用户HEX
//            ofl_prj_write_data(&prog_data.wrbuf->B08[ES_DATA_OFFSET], prog_data.data_length);
//        
//            prj_part.data += check_sum( prog_data.data_length-8 , &prog_data.wrbuf->B08[ES_DATA_OFFSET] ) ;  
//            prj_part.size +=  prog_data.data_length;  
        
            prog_data.data_length = 0x00;
            result = ERROR_SUCCESS;
            ack_len = 1024;
            break;
        case ID_DL_OFFLINE_PRJ_HEX_END :        //脱机方案用户HEX下载完成
//            sum = (prog_data.wrbuf[ES_DATA_OFFSET]      << 24) |
//                    (prog_data.wrbuf[ES_DATA_OFFSET+1] << 16) |
//                    (prog_data.wrbuf[ES_DATA_OFFSET+2] << 8 ) |
//                    (prog_data.wrbuf[ES_DATA_OFFSET+3]  << 0);
        
        
//            if(prj_part.data !=  prog_data.wrbuf->B32[2])
//            {
//                prog_data.data_length = 0x04;
//                result = ERROR_HEX_CHECKSUM_FAIL;
//            }
//            else
            {
//                prj_part.type =  HEX_PART;             
//                ofl_prj_write_part(&prj_part);
       
                prog_data.data_length = 0x00;
                result = ERROR_SUCCESS;
            }
            
            break;
        
        default:
            break;
    }

//    memset(prog_data.data_buf, 0, ISP_PACKET_SIZE);   

    
    __ACK:
    prog_data.rdbuf[0] = 0xBA;
    prog_data.rdbuf[1] = 0xDC;
    prog_data.rdbuf[2] = 0xCD;
    prog_data.rdbuf[3] = 0xAB;
    prog_data.rdbuf[4] = ESLINKII_DEVICE_TYPE;
    prog_data.rdbuf[5] = prog_data.fun_code;
    prog_data.state = ISP_RUN_OK; 
    
    if(result != ERROR_SUCCESS)
    {
        if(result == ERROR_FRAME_HEAD)
        {
            prog_data.rdbuf[5] = 0x00;
            prog_data.data_length = 0x00;    
        } 
        else if(result == ERROR_PROG_CHECK_EMPTY)
        {
            prog_data.data_length = 0x08;           
//            prog_data.rdbuf->B32[2] = isp_process_err.addr ;
//            prog_data.rdbuf->B32[3] = isp_process_err.data;
        }
        else if(result == ERROR_PROG_PROG)
        {            
            prog_data.data_length = 0x04;    
//            prog_data.rdbuf->B32[2] = isp_process_err.addr ;                                  
                    	
        }
        else if(result == ERROR_PROG_VERIFY)
        {
            prog_data.data_length = 0x08;                                          
//            prog_data.rdbuf->B32[2] = isp_process_err.addr ;
//            prog_data.rdbuf->B32[3] = isp_process_err.data;
        }
        prog_data.state = result;
    }        
    
    prog_data.rdbuf[6] = (uint8_t)(prog_data.data_length&0xFF);    
    prog_data.rdbuf[7] = (uint8_t)(prog_data.data_length>>8); 
    prog_data.rdbuf[prog_data.data_length+8] = prog_data.state;    
    prog_data.checksum = check_sum(prog_data.data_length +9, prog_data.rdbuf);
    prog_data.rdbuf[prog_data.data_length + 9] = (uint8_t)(prog_data.checksum&0xFF);            
    prog_data.rdbuf[prog_data.data_length + 10] = (uint8_t)(prog_data.checksum>>8);
    
    return ack_len;
}


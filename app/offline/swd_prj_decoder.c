
#include "RTL.h"  
#include "offline_def.h"

#include "swd_prj_decoder.h"

//需要在上位机下发的flash blob 添加部分信息，才能在目标芯片的RAM中运行
#define HEADER_SIZE 0x20
static uint32_t blob_heard[HEADER_SIZE/4] = {0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040,
                                            0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2} ;
#define RAM_ENTRY         0x20000000    //目标芯片的ram入口地址，默认为0x2000 0000

//static const uint32_t ES32F0XXX_flash_prog_blob[] = {
//    0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2,
//    0x48624963, 0x4a636008, 0x60102001, 0x49636048, 0x600a4a61, 0x60d04a62, 0x60081e80, 0x49614862,
//    0x49626001, 0x69c16001, 0x43112202, 0x200061c1, 0x485d4770, 0x6001495b, 0x6001495c, 0x220269c1,
//    0x61c14311, 0x49544855, 0x4a556001, 0x60d12101, 0x60011e89, 0x47702000, 0x494f4850, 0x49506001,
//    0x60ca2201, 0x60011e91, 0x494e484f, 0x494f6001, 0x68016001, 0xd1172900, 0x61c12111, 0x07c96a01,
//    0x6a01d012, 0xd4fc0789, 0x61814949, 0x07896a01, 0x6a01d5fc, 0xd4fc0789, 0x22106a01, 0x290006c9,
//    0xdb0369c1, 0x61c14391, 0x47702001, 0x61c14391, 0x47702000, 0x0349211f, 0xd2014288, 0x47702000,
//    0x4a354936, 0x4b36600a, 0x60da2201, 0x600a1e92, 0x4a344935, 0x4a35600a, 0x2211600a, 0x6a0a61ca,
//    0x48346088, 0x6a086188, 0xd5fc0780, 0x07806a08, 0x6a08d4fc, 0x20100682, 0x69ca2a00, 0x4382db03,
//    0x200161ca, 0x43824770, 0xe7d761ca, 0x4b23b530, 0x601c4c21, 0x24014d22, 0x1ea460ec, 0x2301601c,
//    0x4298049b, 0x4b20d227, 0x29006098, 0x074cd023, 0x2c0008c8, 0x1c40d000, 0x6019491a, 0x6019491b,
//    0x61d92111, 0x07896a19, 0x4d1ad4fc, 0x3dff2410, 0xd0122800, 0x68116a19, 0x68516119, 0x619d6159,
//    0x6a193208, 0xd4fc0789, 0x1e406a19, 0xd4ef0649, 0x43a069d8, 0x200161d8, 0x6a18bd30, 0xd4fc0780,
//    0x43a069d8, 0x200061d8, 0x0000bd30, 0x55aa6996, 0x40080000, 0x40080400, 0x1acce551, 0x40046500,
//    0x40046400, 0x8ace0246, 0x40081000, 0x9bdf1357, 0x000051ae, 0x00005ea1, 0x00000000
//};

//// Start address of flash
//static const uint32_t flash_start = 0x00000000;
//// Size of flash
//static const uint32_t flash_size = 0x00040000;

///**
//* List of start and size for each size of flash sector - even indexes are start, odd are size
//* The size will apply to all sectors between the listed address and the next address
//* in the list.
//* The last pair in the list will have sectors starting at that address and ending
//* at address flash_start + flash_size.
//*/
//static const uint32_t sectors_info[] = {
//    0x00000000, 0x00000400,
//}; 

//target_cfg_t target_device_cfg = {
//    .sector_size    = 0,
//    .sector_cnt     = 0,
//    .flash_start    = 0,
//    .flash_end      = 0,
//    .ram_start      = 0,
//    .ram_end        = 0,
//    .flash_algo     = 0,
//    .erase_reset    = 0,
//}; 
#define SWD_TIMING_INFO_SZ     40      //swd 时序数据长度。（由通信协议确定）

//timing 状态
typedef enum {
    SWD_STATE_CLOSED,
    SWD_STATE_OPEN,
    SWD_STATE_WRITE,
    SWD_STATE_DONE,
    SWD_STATE_ERROR 
} SWD_STATE_t;

//target_cfg_t swd_target_davice ;

void ofl_set_swd_target_info(const uint8_t *data, target_cfg_t* target_flash)
{
    uint32_t n = 0;
    uint32_t temp[SWD_TIMING_INFO_SZ/4] = {0};
    
//    for (n = 0; n < SWD_TIMING_INFO_SZ/4; n++) 
//    {
//        temp[n] = (data[4*n+3] << 24) | (data[4*n+2] << 16) | (data[4*n+1]<< 8) | data[4*n];
//    } 
//    n = 0;
//    target_flash->flash_start    = temp[n++];
//    target_flash->flash_end      = temp[n++];
//    target_flash->sector_size    = 1024;
//    target_flash->sector_cnt     = (0x40000 / 1024);
//    target_flash->ram_start      = 0x20000000;
//    target_flash->ram_end        = 0x20007fff;
//    
//    target_flash->flash_algo->init       = temp[n++];
//    target_flash->flash_algo->uninit     = temp[n++];  
//    target_flash->flash_algo->erase_chip = temp[n++];      
//    target_flash->flash_algo->erase_sector = temp[n++];         
//    target_flash->flash_algo->program_page = temp[n++];
//    
//    target_flash->flash_start    = 0x00000000;
//    target_flash->flash_end      = 0x00040000;
//    target_flash->sector_size    = 1024;
//    target_flash->sector_cnt     = (0x40000 / 1024);
//    // BKPT : start of blob + 1
//    // RSB  : blob start + header + rw data offset
//    // RSP  : stack pointer
//    target_flash->flash_algo->sys_call_s.breakpoint = 0x20000001;
//    target_flash->flash_algo->sys_call_s.static_base = 0x200001d8;
//    target_flash->flash_algo->sys_call_s.stack_pointer = 0x20000800;
//    target_flash->flash_algo->program_buffer = 0x20000000 + 0x00000A00;  // mem buffer location
//    target_flash->flash_algo->algo_start = 0x20000000;                   // location to write prog_blob in target RAM
//    target_flash->flash_algo->algo_size = 0;             // prog_blob size
//    target_flash->flash_algo->algo_blob = 0;             // address of prog_blob
//    target_flash->flash_algo->program_buffer_size = 0x00000400;   // ram_to_flash_bytes_to_be_written
//    
//    target_flash->flash_algo->algo_size = sizeof(ES32F0XXX_flash_prog_blob);             // prog_blob size
//    target_flash->flash_algo->algo_blob = ES32F0XXX_flash_prog_blob;             // address of prog_blob
}






//#define SWD_TIMING_INFO_LEN     40


// 
//static SWD_STATE_t swd_state = SWD_STATE_CLOSED;

//脱机swd初始化，分配内存来保存flm文件的算法
//void ofl_swd_init(uint32_t size)
//{
//    uint32_t algo_size ;
//    
//    algo_size  =  size - 4 - SWD_TIMING_INFO_LEN;
//    swd_flash_algo_init(size);

//    swd_state =  SWD_STATE_CLOSED;
//}

//swd编程算法
//解析swd编程算法并保存
void ofl_set_swd_timing( target_cfg_t* target_flash,uint32_t addr, const uint32_t *data, uint32_t size)
{
     swd_prg_init(target_flash,0,0,0);
    
   
}
void swd_program_image(uint32_t adr, const uint8_t *buf, uint32_t size)
{
    
//    write_memory_cmd();
    uint8_t addr;
    uint32_t copy_size;
//    flash_manager_init(flash_intf_target);
    
//    while(addr < isp_target_device.code_size ){
//        memset(flash_buf, 0xFF, sizeof(flash_buf));
//        spi_flash_read(SF_USER_HEX_ADDR + addr ,1024 , flash_buf);
//        flash_manager_data(0,flash_buf,1024);   
//        addr +=  1024;          
//    }
}













// static uint32_t *timing_malloc(uint32_t size)
//{
//      //algo_size = timing_start;
//      //offline_prj_read(, 
////声明一个内存池，每块大小4字节，内存块个数：cnt，
//    _declare_box(blob_pool, 4, size/4);
//    
////内存池初始化     
//    _init_box (blob_pool, sizeof(blob_pool), 4);
//    return(_alloc_box (blob_pool));      
//    
//}

////分配内存保存flash algo
//void swd_flash_algo_init(uint32_t size)
//{
//    flash_prog_blob =   timing_malloc( size);     
//}


//error_t swd_timing_info_init(







//}

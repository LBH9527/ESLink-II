#include "es_common.h"
#include "eslink_app_addr.h"
#include "info.h"
#include "settings_rom.h"
#include "iap_flash_intf.h"


//#define INFO_OFFSET         0x10

//#define CFG_KEY             0x6b766c64  
//typedef struct {
//    uint32_t key;
////    uint32_t hic_id;
//    uint32_t version;
//} offline_info_t;

//const uint32_t key __attribute__((section(".ARM.__at_0x0000BF60"))) = 0x6b766c64;
//const   uint32_t key __attribute__((at(0x0000BFF0))) =   

//{
//   .key = CFG_KEY,
//   .version = OFL_VERSION,

//}; 
// Constant variables

//static volatile const offline_info_t info __attribute__(at(ESLINK_ROM_OFFLINE_START + ESLINK_ROM_OFFLINE_SIZE - INFO_OFFSET));
//const volatile  offline_info_t info __attribute__((at(ESLINK_ROM_OFFLINE_START + ESLINK_ROM_OFFLINE_SIZE - INFO_OFFSET))) = 
//{
//   .key = CFG_KEY,
//   .version = OFL_VERSION,

//}; 
//static offline_info_t info_copy = 
//{   
//    .key = 0,
//    .version = 0,

//};

//void set_offline_version(void)   
//{
//    if ( (CFG_KEY != info.key) && (0xffffffff == info.key) )
//    {
//        info_copy.key =  CFG_KEY;
//        info_copy.version =  OFL_VERSION;  
//        iap_Init();
//        //编程前，已经对flash进行擦除
//        info_copy.key =  UPDATE_OFFLINE_APP;
//        info_copy.version = OFL_VERSION;  
//        iap_flash_program(ESLINK_ROM_OFFLINE_START + ESLINK_ROM_OFFLINE_SIZE - INFO_OFFSET, (uint8_t*)&info_copy, sizeof(info_copy));        
//    } 
//      *(uint32_t *)(ESLINK_ROM_OFFLINE_START + ESLINK_ROM_OFFLINE_SIZE - INFO_OFFSET)  = CFG_KEY;
//      *(uint32_t *)(ESLINK_ROM_OFFLINE_START + ESLINK_ROM_OFFLINE_SIZE - INFO_OFFSET + 4)  = OFL_VERSION;
//    const uint32_t 
//}


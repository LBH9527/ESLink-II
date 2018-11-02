#include "es_common.h"
#include "eslink_addr.h"
#include "info.h"
#include "settings_rom.h"
#include "iap_flash_intf.h"

#define OFFLINK_INFO_OFFSET         0x10
#define CFG_KEY             0x6b766c64  
typedef struct {
    uint32_t key;
//    uint32_t hic_id;
    uint32_t version;
} offline_info_t;


// Constant variables
//static volatile const offline_info_t info_ofl __attribute__((at(ESLINK_ROM_OFFLINE_START + ESLINK_ROM_OFFLINE_SIZE - OFFLINK_INFO_OFFSET)));


//判断固件中的信息，与上位机下发的信息做比较，不一致则更新
uint8_t offlink_app_info_check(void)
{
    uint32_t version; 
    uint32_t ofl_version = *((uint32_t *)(ESLINK_ROM_OFFLINE_START + OFFLINK_INFO_OFFSET)) ;

    get_offline_info((uint8_t*) version);       //上位机下发脱机固件版本
    if(ofl_version != version)            //版本不一致，擦除固件信息，更新固件
    {       
         return FALSE;
    }
    else
    {
        return TRUE;      
    }
}

uint32_t get_offlink_app_version(void)
{
//    if(info_ofl.key != CFG_KEY)
//        return 0x00;
//    return  info_ofl.version ;
    uint32_t ofl_version = *((uint32_t *)(ESLINK_ROM_OFFLINE_START + OFFLINK_INFO_OFFSET)) ;
    if(ofl_version == 0xffffffff)
         ofl_version = 0;
    return ofl_version;
}  

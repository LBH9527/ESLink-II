#include "bootisp_target_config.h"


//static const sector_info_t sectors_info[] = {
//    {0x8000000, 0x4000},
//    {0x8010000, 0x10000},
//    {0x8020000, 0x20000},
//    {0x8100000, 0x4000},
//    {0x8110000, 0x10000},
//    {0x8120000, 0x20000},
//};
static const chip_info_t chip_info[] = {
    {0x00040400, 0x400, 0x0101, 0},
    {0x00040800, 0x400, 0x0102, 0},
};
bootisp_target_cfg_t bootisp_target_dev = {
    .flash_start = 0x00000000,
    .flash_size  = 0x00040000,
    
    .packet_size = 0x80,
    .ack = 0x79,
    .start = 0xFF,
    .checksum = 0x00,
    .chip_info = chip_info,
    .chip_info_length = sizeof(chip_info) / sizeof(chip_info_t)
    
};

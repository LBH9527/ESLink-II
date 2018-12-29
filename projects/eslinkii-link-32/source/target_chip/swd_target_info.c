#include "swd_flash_blob.h"
#include "swd_target_config.h"
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

// Start address of flash
//static const uint32_t flash_start = 0x00000000;
// Size of flash
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

//static const program_target_t flash = {
//    0x20000021, // Init
//    0x20000053, // UnInit
//    0x20000079, // EraseChip
//    0x200000d5, // EraseSector
//    0x2000012d, // ProgramPage

//    // BKPT : start of blob + 1
//    // RSB  : blob start + header + rw data offset
//    // RSP  : stack pointer
//    {
//        0x20000001,
//        0x200001d8,
//        0x20000800
//    },

//    0x20000000 + 0x00000A00,  // mem buffer location
//    0x20000000,               // location to write prog_blob in target RAM
//    sizeof(ES32F0XXX_flash_prog_blob),   // prog_blob size
//    ES32F0XXX_flash_prog_blob,           // address of prog_blob
//    0x00000400       // ram_to_flash_bytes_to_be_written
//};

// target information                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   vvvvvvvvvv                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
//target_cfg_t swd_target_device = {
//    .sector_size    = 1024,
//    .sector_cnt     = (0x40000 / 1024),
//    .flash_start    = 0,
//    .flash_end      = 0x00040000,
//    .ram_start      = 0x20000000,
//    .ram_end        = 0x20008000,
//    .flash_algo     = (program_target_t *) &flash,
//};


static const uint32_t ES32F065x_flash_prog_blob[] = {
//    0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2,
//    0x48624963, 0x48636008, 0x60012101, 0x4a624863, 0x4a636002, 0x1e8960d1, 0x48636001, 0x60014961,
//    0x60014962, 0x220269c1, 0x61c14311, 0x47702000, 0x495c485d, 0x495d6001, 0x69c16001, 0x43112202,
//    0x485661c1, 0x60014954, 0x21014a55, 0x1e8960d1, 0x20006001, 0x48514770, 0x6001494f, 0x22014950,
//    0x1e9160ca, 0x48506001, 0x6001494e, 0x6001494f, 0x29006801, 0x2111d117, 0x6a0161c1, 0xd01207c9,
//    0x07896a01, 0x494ad4fc, 0x6a016181, 0xd5fc0789, 0x07896a01, 0x6a01d4fc, 0x06c92210, 0x69c12900,
//    0x4391db03, 0x200161c1, 0x43914770, 0x200061c1, 0x211f4770, 0x42880349, 0x2000d901, 0x49374770,
//    0x600a4a35, 0x22014b36, 0x1e9260da, 0x4936600a, 0x600a4a34, 0x600a4a35, 0x61ca2211, 0x60886a0a,
//    0x61884834, 0x07806a08, 0x6a08d5fc, 0xd4fc0780, 0x06826a08, 0x2a002010, 0xdb0369ca, 0x61ca4382,
//    0x47702001, 0x61ca4382, 0xb530e7d7, 0x4c224b23, 0x4d23601c, 0x60ec2401, 0x601c1ea4, 0x049b2301,
//    0xd2294298, 0x60984b20, 0xd0252900, 0x08c8074c, 0xd0002c00, 0x491b1c40, 0x491c6019, 0x21116019,
//    0x6a1961d9, 0xd4fc0789, 0x21104c1a, 0x28003cff, 0x6a1dd014, 0xd4fc07ad, 0x611d6815, 0x615d6855,
//    0x3208619c, 0x07ad6a1d, 0x6a1dd4fc, 0x066d1e40, 0x69d8d4ed, 0x61d84388, 0xbd302001, 0x07806a18,
//    0x69d8d4fc, 0x61d84388, 0xbd302000, 0x55aa6996, 0x40080000, 0x40080400, 0x1acce551, 0x40046500,
//    0x40046400, 0x8ace0246, 0x40081000, 0x9bdf1357, 0x000051ae, 0x00005ea1, 0x00000000
};

// Start address of flash
static const uint32_t flash_start = 0x00000000;
// Size of flash
static const uint32_t flash_size = 0x00040000;

/**
* List of start and size for each size of flash sector - even indexes are start, odd are size
* The size will apply to all sectors between the listed address and the next address
* in the list.
* The last pair in the list will have sectors starting at that address and ending
* at address flash_start + flash_size.
*/
static const uint32_t sectors_info[] = {
    0x00000000, 0x00000400,
};

static const program_target_t flash = {
    0x20000021, // Init
    0x20000051, // UnInit
    0x20000077, // EraseChip
    0x200000d3, // EraseSector
    0x2000012b, // ProgramPage

    // BKPT : start of blob + 1
    // RSB  : blob start + header + rw data offset
    // RSP  : stack pointer
    {
        0x20000001,
        0x200001d8,
        0x20000800
    },

    0x20000000 + 0x00000A00,  // mem buffer location
    0x20000000,               // location to write prog_blob in target RAM
    sizeof(ES32F065x_flash_prog_blob),   // prog_blob size
    ES32F065x_flash_prog_blob,           // address of prog_blob
    0x00000400       // ram_to_flash_bytes_to_be_written
};

// target information                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   vvvvvvvvvv                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
target_cfg_t swd_target_device = {
    .sector_size    = 1024,
    .sector_cnt     = (0x00040000 / 1024),
    .flash_start    = 0,
    .flash_end      = 0x00040000,
    .ram_start      = 0x20000000,
    .ram_end        = 0x20008000,
    .flash_algo     = (program_target_t *) &flash,
};

#include "swd_flash_blob.h"
#include "swd_target_config.h"

static const uint32_t ES32F065x_flash_prog_blob[] = {
    0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2,
    0x48a549a6, 0x48a66008, 0x60012101, 0x4aa548a6, 0x4aa66002, 0x1e8960d1, 0x48a66001, 0x600149a4,
    0x600149a5, 0x220269c1, 0x61c14311, 0x47702000, 0x499f48a0, 0x49a06001, 0x69c16001, 0x43112202,
    0x489961c1, 0x60014997, 0x21014a98, 0x1e8960d1, 0x20006001, 0x48944770, 0x60014992, 0x22014993,
    0x1e9160ca, 0x48936001, 0x60014991, 0x60014992, 0x29006801, 0x2111d119, 0x6a0161c1, 0xd01407c9,
    0x60812100, 0x07896a01, 0x498cd4fc, 0x6a016181, 0xd5fc0789, 0x07896a01, 0x6a01d4fc, 0x211006ca,
    0x69c22a00, 0x438adb03, 0x200161c2, 0x438a4770, 0x200061c2, 0x211f4770, 0x42880349, 0x2000d901,
    0x49794770, 0x600a4a77, 0x22014b78, 0x1e9260da, 0x4978600a, 0x600a4a76, 0x600a4a77, 0x61ca2211,
    0x60886a0a, 0x61884876, 0x07806a08, 0x6a08d5fc, 0xd4fc0780, 0x06826a08, 0x2a002010, 0xdb0369ca,
    0x61ca4382, 0x47702001, 0x61ca4382, 0xb530e7d7, 0x4c644b65, 0x4d65601c, 0x60ec2401, 0x601c1ea4,
    0x049b2301, 0xd2294298, 0x60984b62, 0xd0252900, 0x08c8074c, 0xd0002c00, 0x495d1c40, 0x495e6019,
    0x21116019, 0x6a1961d9, 0xd4fc0789, 0x21104c5c, 0x28003cff, 0x6a1dd014, 0xd4fc07ad, 0x611d6815,
    0x615d6855, 0x3208619c, 0x07ad6a1d, 0x6a1dd4fc, 0x066d1e40, 0x69d8d4ed, 0x61d84388, 0xbd302001,
    0x07806a18, 0x69d8d4fc, 0x61d84388, 0xbd302000, 0x4b45b530, 0x601c4c43, 0x24014d44, 0x1ea460ec,
    0x074b601c, 0x2b0008c9, 0x1c49d000, 0x4c454b41, 0x4c45605c, 0x2411605c, 0x6a1c61dc, 0xd4fc07a4,
    0x4d3f6098, 0x3dff2410, 0x6a18e014, 0xd4fc0780, 0x61186810, 0x61586850, 0x3208619d, 0x07806a18,
    0x6a18d4fc, 0x06401e49, 0x69d8d404, 0x61d843a0, 0xbd302001, 0xd1e82900, 0x07806a18, 0x69d8d4fc,
    0x61d843a0, 0xbd302000, 0x4604b510, 0x49254826, 0x4a266001, 0x60d12101, 0x60011e89, 0x03604b25,
    0xf7ffd506, 0x4827ff18, 0x48276058, 0xe0036058, 0x6018481f, 0x60184820, 0x61d82011, 0x609c6a18,
    0x6198481f, 0x07806a18, 0x6a18d5fc, 0xd4fc0780, 0x21106a18, 0x28000680, 0xdb0369d8, 0x61d84388,
    0xbd102001, 0x61d84388, 0xbd102000, 0xf7ffb500, 0x2801fef2, 0x4815d00b, 0xffc6f7ff, 0xd0072801,
    0x02c02081, 0xffc0f7ff, 0xd0032801, 0xbd002000, 0xbd002002, 0xbd002003, 0x55aa6996, 0x40080000,
    0x40080400, 0x1acce551, 0x40046500, 0x40046400, 0x8ace0246, 0x40081000, 0x9bdf1357, 0x000051ae,
    0x00005ea1, 0x7153bfd9, 0x0642cea8, 0x00040400, 0x00000000
};

//// Start address of flash
//static const uint32_t flash_start = 0x00000000;
//// Size of flash
//static const uint32_t flash_size = 0x00040000;

/**
* List of start and size for each size of flash sector - even indexes are start, odd are size
* The size will apply to all sectors between the listed address and the next address
* in the list.
* The last pair in the list will have sectors starting at that address and ending
* at address flash_start + flash_size.
*/
//static const uint32_t sectors_info[] = {
//    0x00000000, 0x00000400,
//};

static const program_target_t flash = {
    0x20000021, // Init
    0x20000051, // UnInit
    0x20000077, // EraseChip
    0x200000d3, // EraseSector
    0x2000012f, // ProgramPage
    0x200001b1, // ProgramInfoPage
    0x2000028d, // EraseInfo

    // BKPT : start of blob + 1
    // RSB  : blob start + header + rw data offset
    // RSP  : stack pointer
    {
        0x20000001,
        0x200002f0,
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
    .flash_algo     = (program_target_t *) &flash,
};

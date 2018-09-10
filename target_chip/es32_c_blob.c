/* Flash OS Routines (Automagically Generated)
 * Copyright (c) 2009-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

//// target information
//target_cfg_t swd_target_device = {
//    .sector_size    = 1024,
//    .sector_cnt     = (0x40000 / 1024),
//    .flash_start    = 0,
//    .flash_end      = 0x00040000,
//    .ram_start      = 0x20000000,
//    .ram_end        = 0x20008000,
//    .flash_algo     = (program_target_t *) &flash,
//};


static const uint32_t STM32F407_FLM[] = {
    0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2,

    /*0x020*/ 0xe000300, 0xd3022820L, 0x1d000940, 0x28104770, 0x900d302, 0x47701cc0, 0x47700880, 0x49414842,
    /*0x040*/ 0x49426041, 0x21006041, 0x68c16001, 0x431122f0, 0x694060c1, 0xd4060680L, 0x493d483e, 0x21066001,
    /*0x060*/ 0x493d6041, 0x20006081, 0x48374770, 0x5426901, 0x61014311, 0x47702000, 0x4833b510, 0x24046901,
    /*0x080*/ 0x61014321, 0x3a26901, 0x61014311, 0x4a314933, 0x6011e000, 0x3db68c3, 0x6901d4fb, 0x610143a1,
    /*0x0A0*/ 0xbd102000L, 0xf7ffb530L, 0x4927ffbb, 0x23f068ca, 0x60ca431a, 0x610c2402, 0x700690a, 0x43020e40,
    /*0x0C0*/ 0x6908610a, 0x431003e2, 0x48246108, 0xe0004a21L, 0x68cd6010, 0xd4fb03edL, 0x43a06908, 0x68c86108,
    /*0x0E0*/ 0xf000600, 0x68c8d003, 0x60c84318, 0xbd302001L, 0x4d15b570, 0x8891cc9, 0x8968eb, 0x433326f0,
    /*0x100*/ 0x230060eb, 0x4b16612b, 0x692ce017, 0x612c431c, 0x60046814, 0x3e468ec, 0x692cd4fc, 0x640864,
    /*0x120*/ 0x68ec612c, 0xf240624, 0x68e8d004, 0x60e84330, 0xbd702001L, 0x1d121d00, 0x29001f09, 0x2000d1e5,
    /*0x140*/ 0xbd70, 0x45670123, 0x40023c00, 0xcdef89abL, 0x5555, 0x40003000, 0xfff, 0xaaaa,
    /*0x160*/ 0x201, 0x0,
};

static const program_target_t flash = {
    0x2000003D, // Init
    0x2000006B, // UnInit
    0x20000079, // EraseChip
    0x200000A5, // EraseSector
    0x200000F1, // ProgramPage

// RSB : base adreess is address of Execution Region PrgData in map file
//       to access global/static data
// RSP : Initial stack pointer

    {0x20000001, 0x20000020 + 0x00000144, 0x20002000}, // {breakpoint, RSB, RSP}

    0x20001000, // program_buffer
    0x20000000, // algo_start
    0x00000170, // algo_size
    STM32F407_FLM,// image
    512        // ram_to_flash_bytes_to_be_written
};

target_cfg_t swd_target_device = {
    .sector_size    = 1024,
    .sector_cnt     = (0x100000 / 1024),
    .flash_start    = 0x08000000,
    .flash_end      = 0x08100000,
    .ram_start      = 0x20000000,
    .ram_end        = 0x20020000,
    .flash_algo     = (program_target_t *) &flash,
};

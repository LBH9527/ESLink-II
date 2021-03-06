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
static const uint32_t ES32F0XXX_flash_prog_blob[] = {
    0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2,
    0x48624963, 0x4a636008, 0x60102001, 0x49636048, 0x600a4a61, 0x60d04a62, 0x60081e80, 0x49614862,
    0x49626001, 0x69c16001, 0x43112202, 0x200061c1, 0x485d4770, 0x6001495b, 0x6001495c, 0x220269c1,
    0x61c14311, 0x49544855, 0x4a556001, 0x60d12101, 0x60011e89, 0x47702000, 0x494f4850, 0x49506001,
    0x60ca2201, 0x60011e91, 0x494e484f, 0x494f6001, 0x68016001, 0xd1172900, 0x61c12111, 0x07c96a01,
    0x6a01d012, 0xd4fc0789, 0x61814949, 0x07896a01, 0x6a01d5fc, 0xd4fc0789, 0x22106a01, 0x290006c9,
    0xdb0369c1, 0x61c14391, 0x47702001, 0x61c14391, 0x47702000, 0x0349211f, 0xd2014288, 0x47702000,
    0x4a354936, 0x4b36600a, 0x60da2201, 0x600a1e92, 0x4a344935, 0x4a35600a, 0x2211600a, 0x6a0a61ca,
    0x48346088, 0x6a086188, 0xd5fc0780, 0x07806a08, 0x6a08d4fc, 0x20100682, 0x69ca2a00, 0x4382db03,
    0x200161ca, 0x43824770, 0xe7d761ca, 0x4b23b530, 0x601c4c21, 0x24014d22, 0x1ea460ec, 0x2301601c,
    0x4298049b, 0x4b20d227, 0x29006098, 0x074cd023, 0x2c0008c8, 0x1c40d000, 0x6019491a, 0x6019491b,
    0x61d92111, 0x07896a19, 0x4d1ad4fc, 0x3dff2410, 0xd0122800, 0x68116a19, 0x68516119, 0x619d6159,
    0x6a193208, 0xd4fc0789, 0x1e406a19, 0xd4ef0649, 0x43a069d8, 0x200161d8, 0x6a18bd30, 0xd4fc0780,
    0x43a069d8, 0x200061d8, 0x0000bd30, 0x55aa6996, 0x40080000, 0x40080400, 0x1acce551, 0x40046500,
    0x40046400, 0x8ace0246, 0x40081000, 0x9bdf1357, 0x000051ae, 0x00005ea1, 0x00000000
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
    0x20000053, // UnInit
    0x20000079, // EraseChip
    0x200000d5, // EraseSector
    0x2000012d, // ProgramPage

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
    sizeof(ES32F0XXX_flash_prog_blob),   // prog_blob size
    ES32F0XXX_flash_prog_blob,           // address of prog_blob
    0x00000400       // ram_to_flash_bytes_to_be_written
};

// target information
target_cfg_t test_target_device = {
    .sector_size    = 1024,
    .sector_cnt     = (0x40000 / 1024),
    .flash_start    = 0,
    .flash_end      = 0x00040000,
    .ram_start      = 0x20000000,
    .ram_end        = 0x20008000,
    .flash_algo     = (program_target_t *) &flash,
};


#ifndef __TARGET_INFO_H
#define __TARGET_INFO_H


#define CHIP_CHECKSUM_ADDR      0x23            //根据IDE Space 修改   
#define CHIP_INFO1_ADDR         0x00100100    
#define CHIP_INFO1_SIZE         256 
 
//CFG_DEBUG 位
#define CHIP_CFG_WORD0_ADDR         0x4
#define CHIP_CFG_DEBUG_Pos          15
#define CHIP_CFG_DEBUG_Msk          (1 << CHIP_CFG_DEBUG_Pos)

#endif

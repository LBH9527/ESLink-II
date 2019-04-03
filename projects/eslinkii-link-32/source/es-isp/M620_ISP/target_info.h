#ifndef __TARGET_INFO_H
#define __TARGET_INFO_H

struct info_part_map
{
    uint32_t addr;
    uint32_t size;
};  

//1.芯片信息根据芯片手动调整的数据信息     //根据IDE Space 修改       
#define CHIP_CHECKSUM_ADDR          0x20    

//2.CHIP INFO的信息需要根据芯片的xml文件来确认 
#define CHIP_INFO_PART1_ADDR        0x00
#define CHIP_INFO_PART1_SIZE        48     //12--->48字节长度

#define CHIP_INFO_PART2_ADDR        0x200
#define CHIP_INFO_PART2_SIZE        4     //1--->4字节长度

static const struct info_part_map info_part_map[] =
{
    {CHIP_INFO_PART1_ADDR, CHIP_INFO_PART1_SIZE},
    {CHIP_INFO_PART2_ADDR, CHIP_INFO_PART2_SIZE},
};        

//CFG_DEBUG 位
#define CHIP_CFG_WORD0_ADDR         0x4
#define CHIP_CFG_DEBUG_Pos          15
#define CHIP_CFG_DEBUG_Msk          (1 << CHIP_CFG_DEBUG_Pos)
#endif

#ifndef __TARGET_INFO_H
#define __TARGET_INFO_H


#define CHIP_INFO_ADDR          0x00001800  
#define CHIP_LV1_ADDR           0x00001C00

#define CHIP_CHECKSUM_ADDR      0x00001C40
#define CHIP_USERID_ADDR        0x00001C44

//配置字地址信息，此信息需要根据XML文件修改。

#define CHIP_INFO_PART1_ADDR         CHIP_INFO_ADDR
#define CHIP_INFO_PART1_SIZE         48           //字节长度

#define CHIP_INFO_PART2_ADDR         CHIP_LV1_ADDR
#define CHIP_INFO_PART2_SIZE         8              

#define CHIP_INFO_PART3_ADDR         (CHIP_LV1_ADDR+8)
#define CHIP_INFO_PART3_SIZE         8              

#define CHIP_INFO_PART4_ADDR         (CHIP_CHECKSUM_ADDR)
#define CHIP_INFO_PART4_SIZE         8              

#define CHIP_INFO_PART_SIZE          4
#define CHIP_INFO_SIZE              ( CHIP_INFO_PART1_SIZE + CHIP_INFO_PART2_SIZE   \
                                        + CHIP_INFO_PART3_SIZE + CHIP_INFO_PART4_SIZE)


#endif
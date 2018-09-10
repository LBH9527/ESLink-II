#ifndef __ISP_M939_H
#define __ISP_M939_H


#define STATUS_CHECK_CD    0xF0                                                //Unlock Check Command
#define ISP_MODE_CD        0xF1                                                //ISP Mode Select Command
#define ENCRYPT_LOAD_CD    0xFF                                                //Encrytion Command Download

#define ID_CHECK_CD        0x699655FA                                          //ID Check Command
#define ID_CODE_VAL        0x4D93914F                                          //ID Code//0x4D62012E//

#define AREA_SET_CD        0xE1//0                                                //Area Select Command
#define AREA_CHECK_CD      0xE0//1                                                //Area Check Command
#define SET_ADDR_CD        0xE3//2                                                //Address Set Command
#define READ_ADDR_CD       0xE2//3
#define WRITE_BUFF0_CD     0xE5//4                                                //Write Buff Command
#define READ_BUFF0_CD      0xE4//5                                                //Read Buff Command
#define WRITE_BUFF1_CD     0xE7//6                                                //Write Buff Command
#define READ_BUFF1_CD      0xE6//7                                                //Read Buff Command

#define READ_FLASH_CD      0xE8                                                //Read Data Command
#define READ_FLASH_PLUS_CD 0xEA//9                                                //Read Data Command
#define READ_FLASH1_CD     0xEC//A                                                //Read Data1 Command
#define WRITE_INFO_BUFF_CD 0xEF//B                                                //Write Buff Command

#define ERASE_CHECK_CD     0xC0                                                //Erase Check Command
#define ERASE_CODE         0xC1                                                //Erase User Code 
#define ERASE_INF0_UNLOCK  0xB169  
#define ERASE_INF0         0xC2                                                //Erase Information Area 0 Command
#define ERASE_INF1         0xC3                                                //Erase Information Area 1 Command
#define ERASE_INF2         0xC4                                                //Erase Information Area 2 Command
#define ERASE_INF3         0xC5                                                //Erase Information Area 3 Command
#define PROG_CHECK_CD      0xC8                                                //Programme Check Command
#define WORD_PRG_CD        0xC9                                                //Word Programme Command
#define WORD_ADD_PLUS_PRG  0xCA                                                 //先加后编
#define WORD_PRG_ADD_PLUS  0xCB                                                 //先编后加

#define UNLOCK_CHECK_VAL   0xA0                                                //Unlock Success Value
#define MODE_CHECK_VAL     0xA1                                                //ISP Mode Check Value

#define CODE_AREA_SELECT   0x4B 
#define INFOR_AREA_SELECT  0xBB                                                //Information Area Select Value for Reading and Programming

#define USER_AREA_VAL      0xF0                                                //User Area Select Value with Erasing
#define INF0R_AREA_VAL     0xE1                                                //Information Area 0 Select Value with Erasing
#define INF1R_AREA_VAL     0xD2                                                //Information Area 1 Select Value with Erasing
#define INF2R_AREA_VAL     0xC3                                                //Information Area 2 Select Value with Erasing
#define INF3R_AREA_VAL     0xB4                                                //Information Area 3 Select Value with Erasing

#define ENCRYPTION_VAL     0xA5A5A5                                            //Encrytion Code

#define ERASE_OK_VAL       0x69                                                //Erase-completely Value                                            
#define PROG_CHECK_VAL     0x87                                                //Programme Check Value

#define CRC_CTRL_WR        0xD1//0                                             //
#define CRC_CTRL_RD        0xD0//1                                             //
#define CRC_EN             0xA5                                                //
#define CRC_DIS            0x5A                                                //

#define CRC_STATE_CHECK    0xD2
#define CRC_TRIGER         0xD3//2                                             //

#define CRC_SIZE_WR        0xD5                                                //
#define CRC_SIZE_RD        0xD4//6                                             //

#define CRC_VALUE_READ     0xD6//8                                             //
#define CRC_ADDR_Read      0xD8//9                                             //

#define USER_INFO_START_ADDR	0x00000400
#define CFG_ENCPAGE0_ADDR		0x00000810
#define CFG_ENCPAGE1_ADDR		0x00000818	




#endif


#ifndef ISP_DEF_H
#define ISP_DEF_H










#define STATUS_CHECK_CMD    0xF0                                                //Unlock Check Command
#define ISP_MODE_CMD        0xF1                                                //ISP Mode Select Command
#define ENCRYPT_LOAD_CD    0xFF                                                //Encrytion Command Download

#define ID_CHECK_CD        0x699655FA                                          //ID Check Command
#define ID_CODE_VAL        0x4D93914F                                          //ID Code//0x4D62012E//

#define AREA_CHECK_CD       0xE0//1 
#define AREA_SET_CMD        0xE1//0                                                //Area Select Command‘
#define CODE_AREA           0x4B
#define INFO_AREA           0xBB
#define ADDR_CHECK_CMD        0xE2//3     
#define ADDR_SET_CMD        0xE3//2 
#define READ_BUFF0_CD      0xE4//5    
#define DATA0_WRITE_CMD     0xE5//4 
#define READ_BUFF1_CD      0xE6//7  //Read Buff Command
#define DATA1_WRITE_CMD     0xE7//6  

#define FLASH_READ_CMD      0xE8   
#define FLASH_READ_PLUS_CMD 0xEA//9                                                //Read Data Command
#define FLASH_READ1_CMD     0xEC//A 

                                              //Area Check Command
                                               //Address Set Command

                                               //Write Buff Command
                                              //Write Buff Command
                                               //Read Buff Command

                                            //Read Data Command
                                               //Read Data1 Command
#define WRITE_INFO_BUFF_CD 0xEF//B                                                //Write Buff Command

#define ERASE_CHECK_CMD     0xC0                                                //Erase Check Command
#define ERASE_CODE_CMD     0xC1                                                //Erase User Code 
#define ERASE_INF0_UNLOCK  0xB169  
#define ERASE_INF0_CMD         0xC2                                                //Erase Information Area 0 Command
#define ERASE_INF1         0xC3                                                //Erase Information Area 1 Command
#define ERASE_INF2         0xC4                                                //Erase Information Area 2 Command
#define ERASE_INF3         0xC5  

//编程命令
#define PROG_CHECK_VAL     0x87
#define PROG_CHECK_CMD     0xC8                                                //Programme Check Command
#define PRG_CMD        0xC9                                                //Word Programme Command
#define PLUS_PRG_CMD  0xCA                                                 //先加后编
#define PRG_PLUS_CMD  0xCB                                                 //先编后加

 
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

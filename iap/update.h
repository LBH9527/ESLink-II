#ifndef _UPDATE_H
#define _UPDATE_H
 
typedef struct
{
    bool isActive;
    uint32_t address;
    uint32_t size;
    uint32_t sector_szie;
    uint8_t option;
}bootloader_context_t;

/*******************************************************************************
*函数名：stay_in_bootloader
* 描述 ：
* 输入 ：
* 输出 ：TRUE:在boot中运行。 FALSE：跳转到APP
*******************************************************************************/
uint8_t stay_in_bootloader(void);

/*******************************************************************************
*函数名：application_check_and_run
* 描述 ：判断并跳转到APP。
* 输入 ：
* 输出 ：
*******************************************************************************/
uint8_t application_check_and_run(void);

uint8_t update_app_init(uint32_t app_type) ;
/*******************************************************************************
*函数名：update_app_program
* 描述 ：更新APP编程。上位机会根据HEX解析出真实的地址并编程。
* 输入 ：
* 输出 ：0 - OK,  1 - Failed 
*******************************************************************************/
uint8_t update_app_program(uint32_t app_type,uint32_t addr, uint8_t *data, uint32_t size ) ;
uint8_t ofl_update_app_program(uint32_t app_type, uint32_t addr, uint8_t *data, uint32_t size );
void update_app_program_end(void);
uint8_t get_update_app_checksum(uint32_t *checksum);
uint32_t get_offlink_app_version(void);

#endif

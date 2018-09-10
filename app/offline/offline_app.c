 //offline的接口函数
 //用于提供接口，创建任务
 //按键选择要烧录的脱机工程，获取脱机工程文件的配置信息
#include "string.h"
#include "RTL.h"
#include "tasks.h" 
#include "es_common.h"
#include "offline.h" 
#include "offline_def.h" 

 extern OS_SEM key_sem;
 



//脱机编程初始化
void ofl_prg_get_timing(void)
{
    void *target;
    
    ofl_prj_open(0);
    ofl_prj_read_timing_info(target);
    ofl_prj_read_timing(target);     
}

void ofl_prg_process()
{
    
    
    
}

__task void offline_process_event(void)
{
    os_itv_set(100); //周期唤醒的时间间隔:1000mS
       //挂载文件系统
//    prj_file_init();
//ofl_prj_init();
    prj_file_init(); //挂载文件系统
    os_sem_wait (&key_sem, 0xFFFF);
        
    ofl_get_prj_table();
    while(1)
    {
          printf("test");
        os_itv_wait ();
    }
 
}


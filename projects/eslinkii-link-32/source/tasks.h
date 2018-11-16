

#ifndef TASK_H
#define TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#define NO_TIMEOUT  (0xffff)

#define LOWEST_PRIORITY             (1)     /* Priority 0 is reserved for the RTX idle task */
#define HIGHEST_PRIORITY            (254)   /* Priority 255 is reserved by RTX */

/*******************************************************************************
                                任务优先级
*******************************************************************************/
#define MAIN_TASK_PRIORITY              (5)
#define OFFLINE_TASK_PRIORITY           (4)     //脱机任务

#define ESLINK_TASK_PRIORITY            (3)    //联机任务
#define KEY_TASK_PRIORITY               (2)  
#define DISPALY_TASK_PRIORITY           (1)    
   

/*******************************************************************************
                                任务栈空间
*******************************************************************************/
// trouble here is that reset for different targets is implemented differently so all targets
//  have to use the largest stack or these have to be defined in multiple places... Not ideal
//  may want to move away from threads for some of these behaviours to optimize mempory usage (RAM)
#define MAIN_TASK_STACK             (600)
#define KEY_TASK_STACK              (200)
#define OLED_DISPLAY_TASK_STACK     (400)
#define ESLINK_TASK_STACK           (200)

//#define DAP_TASK_STACK      (272)  
#define OFFLINE_TASK_STACK          (2500)




#ifdef __cplusplus
}
#endif

#endif

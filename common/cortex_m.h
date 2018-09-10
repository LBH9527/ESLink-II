
#ifndef CORTEX_M_H
#define CORTEX_M_H

#include "MK22F12810.h"
#include "es_common.h"

#include "stdbool.h"
/*
*   开关全局中断
*/
#define ENABLE_INT()	__set_PRIMASK(0)	/* ?????? */
#define DISABLE_INT()	__set_PRIMASK(1)	/* ?????? */
/**
 * \brief 关闭本地CPU中断
 *
 * \attention
 * - 关闭本地CPU中断的时间一定要短
 * - 必须与 am_int_cpu_unlock() 函数搭配使用
 *
 * \return 中断关闭信息
 *
 * \par 示例：
 * \code
 * uint32_t key;
 * 
 * key = am_int_cpu_lock();
 * // do something.
 * am_int_cpu_unlock(key);
 * \endcode
 *
 * \par 嵌套使用示例：
 * \code 
 * uint32_t key0, key1;
 *
 * key0 = am_int_cpu_lock();  // 中断马上关闭
 * key1 = am_int_cpu_lock();  // 中断保持关闭
 * 
 * am_int_cpu_unlock(key1);   // 中断保持关闭
 * am_int_cpu_unlock(key0);   // 中断马上打开
 * \endcode
 */
uint32_t es_int_cpu_lock(void);

/**
 * \brief 打开本地CPU中断
 *
 * \attention
 *  必须与 am_int_cpu_lock() 函数搭配使用
 *
 * \param[in] key : 中断关闭信息
 * \return    无
 */
void es_int_cpu_unlock(uint32_t key);


/**
 * \brief 系统软复位
 */
static inline void SystemSoftReset(void)
{
    NVIC_SystemReset();
}

typedef int cortex_int_state_t;

__attribute__((always_inline))
static cortex_int_state_t cortex_int_get_and_disable(void)
{
    cortex_int_state_t state;
    state = __disable_irq();
    return state;
}

__attribute__((always_inline))
static void cortex_int_restore(cortex_int_state_t state)
{
    if (!state) {
        __enable_irq();
    }
}

__attribute__((always_inline))
static bool cortex_in_isr(void)
{
    return (__get_xPSR() & 0x1F) != 0;
}




/*
 * Some macros define
 */
#ifndef HWREG32
#define HWREG32(x)          (*((volatile rt_uint32_t *)(x)))
#endif
#ifndef HWREG16
#define HWREG16(x)          (*((volatile rt_uint16_t *)(x)))
#endif
#ifndef HWREG8
#define HWREG8(x)           (*((volatile rt_uint8_t *)(x)))
#endif

#endif

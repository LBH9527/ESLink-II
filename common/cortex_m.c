#include "cortex_m.h"

__attribute__((weak))
void SystemReset(void)
{
    NVIC_SystemReset();
}

#ifdef __CC_ARM
__asm uint32_t es_int_cpu_lock (void)
{
    MRS     r0, PRIMASK
    CPSID   I
    BX      LR
}

__asm void es_int_cpu_unlock (uint32_t key)
{
    MSR     PRIMASK, r0
    BX      LR
}

#else

#ifdef __GNUC__

uint32_t es_int_cpu_lock (void)
{
    uint32_t key;

    asm volatile("MRS   r0, PRIMASK");
    asm volatile("MOV   %0, r0":"=r"(key));
    asm volatile("CPSID I");

    return key;
}

void es_int_cpu_unlock (uint32_t key)
{
    asm volatile("MSR PRIMASK, %0"::"r"(key));
    asm volatile("CPSIE I");
}

#endif
#endif

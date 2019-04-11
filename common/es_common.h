

/**
 * \file
 * \brief 通用工具宏定义头文件
 *
 */

#ifndef __ES_COMMON_H
#define __ES_COMMON_H


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus  */

/******************************************************************************/

/**
 * \name 常用宏定义
 * @{
 */

/**
 * \brief 求结构体成员的偏移
 * \attention 不同平台上，由于成员大小和内存对齐等原因，
 *            同一结构体成员的偏移可能是不一样的
 *
 * \par 示例
 * \code
 *  struct my_struct = {
 *      int  m1;
 *      char m2;
 *  };
 *  int offset_m2;
 *
 *  offset_m2 = ES_OFFSET(struct my_struct, m2);
 * \endcode
 */
#define OFFSET(structure, member)    ((size_t)(&(((structure *)0)->member)))

/** \brief 求结构体成员的偏移，同 \ref ES_OFFSET 一样 */
#ifndef offsetof
#define offsetof(type, member)           ES_OFFSET(type, member)
#endif

/**
 * \brief 通过结构体成员指针获取包含该结构体成员的结构体
 *
 * \param ptr    指向结构体成员的指针
 * \param type   结构体类型
 * \param member 结构体中该成员的名称
 *
 * \par 示例
 * \code
 *  struct my_struct = {
 *      int  m1;
 *      char m2;
 *  };
 *  struct my_struct  my_st;
 *  char             *p_m2 = &my_st.m2;
 *  struct my_struct *p_st = ES_CONTAINER_OF(p_m2, struct my_struct, m2);
 * \endcode
 */
#define CONTAINER_OF(ptr, type, member) \
            ((type *)((char *)(ptr) - offsetof(type,member)))

/**
 * \brief 通过结构体成员指针获取包含该结构体成员的结构体，
 *        同 \ref ES_CONTAINER_OF 一样
 */
#ifndef container_of
#define container_of(ptr, type, member)     ES_CONTAINER_OF(ptr, type, member)
#endif

/** \brief 预取 */
#ifndef prefetch
#define prefetch(x) (void)0
#endif

/**
 * \brief 计算结构体成员的大小
 *
 * \code
 *  struct a = {
 *      uint32_t m1;
 *      uint32_t m2;
 *  };
 *  int size_m2;
 *
 *  size_m2 = ES_MEMBER_SIZE(a, m2);   //size_m2 = 4
 * \endcode
 */
#define MEMBER_SIZE(structure, member)  (sizeof(((structure *)0)->member))

/**
 * \brief 计算数组元素个数
 *
 * \code
 *  int a[] = {0, 1, 2, 3};
 *  int element_a = ES_NELEMENTS(a);    // element_a = 4
 * \endcode
 */
#define NELEMENTS(array)               (sizeof (array) / sizeof ((array) [0]))

/**
 * \brief 永久循环
 *
 * 写永远循环的语句块时大部分程序员喜欢用while(1)这样的语句，
 * 而有些喜欢用for(;;)这样的语句。while(1)在某些老的或严格的编译器上
 * 可能会出现警告，因为1是数字而不是逻辑表达式
 *
 * \code
 * ES_FOREVER {
 *     ; // 循环做一些事情或满足某些条件后跳出
 * }
 * \endcode
 */
#define ES_FOREVER  for (;;)

/******************************************************************************/

/**
 * \brief min 和 max 是 C++ 的标准函数，由 C++ 的库提供。
 */
#ifndef __cplusplus

/**
 * \brief 获取2个数中的较大的数值
 * \param x 数字1
 * \param y 数字2
 * \return 2个数中的较大的数值
 * \note 参数不能使用++或--操作
 */
#ifndef MAX
#define MAX(x, y)               (((x) < (y)) ? (y) : (x))
#endif

/**
 * \brief 获取2个数中的较小的数值
 * \param x 数字1
 * \param y 数字2
 * \return 2个数中的较小数值
 * \note 参数不能使用++或--操作
 */
#ifndef MIN
#define MIN(x, y)               (((x) < (y)) ? (x) : (y))
#endif

#endif /* __cplusplus */

/**
 * \brief 向上舍入
 *
 * \param x     被运算的数
 * \param align 对齐因素，必须为2的乘方
 *
 * \code
 * int size = ROUND_UP(15, 4);   // size = 16
 * \endcode
 */
#define ROUND_UP(x, align)   (((int) (x) + (align - 1)) & ~(align - 1))

/**
 * \brief 向下舍入
 *
 * \param x     被运算的数
 * \param align 对齐因素，必须为2的乘方
 *
 * \code
 * int size = ROUND_DOWN(15, 4);   // size = 12
 * \endcode
 */
#define ROUND_DOWN(x, align) ((int)(x) & ~(align - 1))

/** \brief 倍数向上舍入 */
#define DIV_ROUND_UP(n, d)   (((n) + (d) - 1) / (d))

/**
 * \brief 测试是否对齐
 *
 * \param x     被运算的数
 * \param align 对齐因素，必须为2的乘方
 *
 * \code
 * if (ES_ALIGNED(x, 4) {
 *     ; // x对齐
 * } else {
 *     ; // x不对齐
 * }
 * \endcode
 */
#define ALIGNED(x, align)    (((int)(x) & (align - 1)) == 0)

/******************************************************************************/

/** \brief 把符号转换为字符串 */
#define ES_STR(s)               #s

/** \brief 把宏展开后的结果转换为字符串 */
#define ES_XSTR(s)              ES_STR(s)

/******************************************************************************/

/** \brief 将1字节BCD数据转换为16进制数据 */
#define BCD_TO_HEX(val)     (((val) & 0x0f) + ((val) >> 4) * 10)

/** \brief 将1字节16进制数据转换为BCD数据 */
#define HEX_TO_BCD(val)     ((((val) / 10) << 4) + (val) % 10)

/******************************************************************************/
#define ITEM_NUM(items)     (sizeof(items) / sizeof(items[0]))

#define MB(size)                        ((size) * 1024 * 1024)

#define KB(size)                        ((size) * 1024)




//发送事件
#define flag_send(c,b)      ( (c) |= (b) ) 
//接收事件   
#define flag_recv(c,b)        ( ( (c) & (b)) >0 )
//清除事件
#define flag_clr(c,b)      ( (c) &= ~(b) )

//#define LONG_TO_BIG_ENDIAN_ADDRESS(n, m)       \
//    {                                              \
//        m[0] = ((((uint32_t)(n)) >> 24U) & 0xFFU); \
//        m[1] = ((((uint32_t)(n)) >> 16U) & 0xFFU); \
//        m[2] = ((((uint32_t)(n)) >> 8U) & 0xFFU);  \
//        m[3] = (((uint32_t)(n)) & 0xFFU);          \
//    }



#if defined(LIB_DEBUG)
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((char *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(char * file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif
  
#if defined(LIB_DEBUG)
    #include <stdio.h>
    #define LIB_TRACE  printf
#else
    #define LIB_TRACE(...)
#endif                



/** @} */


/** 
 * @}  
 */
//#include "am_bitops.h"
//#include "am_byteops.h"
//#include "am_errno.h"
#include "es_types.h"
#include "es_list.h"
#include "ustdlib.h"

/*******************************************************************************
  函数声明
*******************************************************************************/
uint32_t check_sum(uint32_t number,uint8_t *pData);

#ifdef __cplusplus
}
#endif    /* __cplusplus */

#endif /* __AM_COMMON_H */

/* end of file */

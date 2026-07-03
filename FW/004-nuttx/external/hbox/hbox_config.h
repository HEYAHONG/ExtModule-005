#ifndef __HBOX_CONFIG_H__
#define __HBOX_CONFIG_H__
#include "stdlib.h"

/*
 * 启用初始化节
 */
#define HRUNTIME_USING_INIT_SECTION 1

/*
 * 启用循环节
 */
#define HRUNTIME_USING_LOOP_SECTION 1

/*
 * 启用符号节
 */
#define HRUNTIME_USING_SYMBOL_SECTION 1

#if defined(CONFIG_SYSTEM_ARGTABLE3)

/*
 * 使用系统argtable3
 */
#if !defined(H3RDPARTY_USING_SYSTEM_ARGTABLE3)
#define H3RDPARTY_USING_SYSTEM_ARGTABLE3 1
#endif

#endif


#endif  // __HBOX_CONFIG_H__

#ifndef __HBOX_CONFIG_H__
#define __HBOX_CONFIG_H__

#define HDEFAULTS_TICK_GET hbox_tick_get
#define HDEFAULTS_MUTEX_LOCK hbox_enter_critical
#define HDEFAULTS_MUTEX_UNLOCK hbox_exit_critical




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


/*
 * 启用hdefaults的libc包装优化
 */
#define HDEFAULTS_LIBC_OPTIMIZE_LEVEL 1

/*
 * 启用hputchar
 */
#define HPUTCHAR console_putchar

/*
 * 启用hgetchar
 */
#define HGETCHAR console_getchar

/*
 * 配置hshell
 */
#define HSHELL_CONTEXT_BUFFER_SIZE  384
#define HSHELL_MAX_ARGC             8
#define HSHELL_MAX_HISTORY_COUNT    2

/*
 * 启用hthrd
 */
#define HTHRD_USING_FREERTOS        1
#define HTHRD_FREERTOS_FREERTOS_HEADER "freertos/FreeRTOS.h"
#define HTHRD_FREERTOS_TASK_HEADER     "freertos/task.h"
#define HTHRD_FREERTOS_STACK_SIZE      (4096)


#endif  // __HBOX_CONFIG_H__

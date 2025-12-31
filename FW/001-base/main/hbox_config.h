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


#endif  // __HBOX_CONFIG_H__

#ifndef __HBOX_CONFIG_H__
#define __HBOX_CONFIG_H__


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
 * 关闭部分不兼容的syscall
 */
#define HDEFAULTS_SYSCALL_NO_HMKDIR 1
#define HDEFAULTS_SYSCALL_NO_HFSYNC 1

#endif  // __HBOX_CONFIG_H__

#ifndef __GLOBALCONFIG_H__
#define __GLOBALCONFIG_H__
#include "hbox.h"
#ifdef __cplusplus
extern "C"
{
#endif
/*
 * 全局配置
 */

/** \brief 加载全局配置
 *
 * \return hcjson_t* 全局配置
 *
 */
hcjson_t * globalconfig_load(void);


/** \brief 保存全局配置
 *
 *
 */
void globalconfig_save(void);


#ifdef __cplusplus
}
#endif


#endif

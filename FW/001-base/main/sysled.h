#ifndef __SYSLED_H_INCLUDED__
#define __SYSLED_H_INCLUDED__
#include "stdint.h"
#include "inttypes.h"
#include "stdlib.h"
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/** \brief SYSLED获取亮灭表
 *
 * \param index uint8_t 灯编号
 * \return uint64_t 亮灭表，每bits表示100ms，共64bits
 *
 */
uint64_t sysled_get_table(uint8_t index);

/** \brief SYSLED设置亮灭表
 *
 * \param index uint8_t 灯编号
 * \param table uint64_t 亮灭表，每bits表示100ms，共64bits
 *
 */
void sysled_set_table(uint8_t index,uint64_t table);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __SYSLED_H_INCLUDED__

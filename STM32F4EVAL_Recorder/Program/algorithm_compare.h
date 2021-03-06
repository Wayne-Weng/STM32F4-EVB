/* #include "algorithm_compare.h" */

#ifndef __ALGORITHM_COMPARE_H
#define __ALGORITHM_COMPARE_H

#include "stm32f4xx.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
u8 CmpArr_U8( uc8 *Arr1, uc8 *Arr2, u32 dataLen );
u8 CmpArr_U16( uc16 *Arr1, uc16 *Arr2, u32 dataLen );
u8 CmpArr_U8U16( uc8 *Arr1, uc16 *Arr2, u32 dataLen );
/*=====================================================================================================*/
/*=====================================================================================================*/
#endif

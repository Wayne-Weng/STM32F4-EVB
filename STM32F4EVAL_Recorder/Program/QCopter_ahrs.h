/* #include "QCopterFC_ahrs.h" */

#ifndef __QCOPTERFC_AHRS_H
#define __QCOPTERFC_AHRS_H

#include "stm32f4xx.h"
#include "algorithm_quaternion.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
#define SampleRate      ((float)0.0025f)   // 400Hz
#define SampleRateHelf  ((float)0.00125f)  // 400Hz
/*=====================================================================================================*/
/*=====================================================================================================*/
typedef struct {
  s16 X;
  s16 Y;
  s16 Z;
  s16 OffsetX;
  s16 OffsetY;
  s16 OffsetZ;
  float TrueX;
  float TrueY;
  float TrueZ;
} Sensor;

void AHRS_Init( Quaternion *pNumQ, EulerAngle *pAngE );
void AHRS_Update( void );

extern Sensor Acc;
extern Sensor Gyr;
/*=====================================================================================================*/
/*=====================================================================================================*/
#endif	

/* #include "interface.h" */

#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "stm32f4xx.h"
#include "stm32f4_evb_i2c_tsc.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
#define KEY_WAKE  PAI(0)
#define KEY       PII(8)
#define KEY_TAMP  PCI(13)

#define LED_1 PFO(7)
#define LED_2 PFO(8)
#define LED_3 PFO(9)
#define LED_4 PFO(10)
/*=====================================================================================================*/
/*=====================================================================================================*/
#define ACC_X_OFFSET ((s16)0)
#define ACC_Y_OFFSET ((s16)0)
#define ACC_Z_OFFSET ((s16)8192)
#define GYR_X_OFFSET ((s16)0)
#define GYR_Y_OFFSET ((s16)0)
#define GYR_Z_OFFSET ((s16)0)

#define SD_BUF_SIZE 128

typedef __IO enum {
  Mode_GyrCorrect,
  Mode_AccCorrect,  // 僅在水平狀態下做校正
  Mode_MagCorrect,
  Mode_Quaternion,
  Mode_Algorithm
} Sensor_Mode;

void GPIO_Config( void );
void Windows_Ctrl( void );
void SENSOR_InitInfo( void );
void SDCARD_InitInfo( void );
void SDCARD_Recorder( void );

extern s16 ReadIMU[8];
extern u16 TouchBuf[3];
extern s16 WaveFormData[4];
extern Sensor_Mode SensorMode;
/*=====================================================================================================*/
/*=====================================================================================================*/
#endif

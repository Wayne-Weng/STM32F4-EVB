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

#define WaveFromNum_X 347
#define WaveFromNum_Y 8

#define WaveName_Acc 0
#define WaveName_Gyr 1
#define WaveName_Ang 2

#define WINDOW_H 240
#define WINDOW_L 400

#define ButMenu_W	95
#define ButMenu_L	20

#define SelMenu_1	(4)
#define SelMenu_2	(SelMenu_1+ButMenu_W+4) // 4 + 95 + 4 = 103
#define SelMenu_3	(SelMenu_2+ButMenu_W+4) // 103 + 95 + 4 = 202
#define SelMenu_4	(SelMenu_3+ButMenu_W+4) // 202 + 95 + 4 = 301

#define Frame_L (SelMenu_4+ButMenu_W+4)   // 301 + 95 + 4 = 400
#define Frame_H (WINDOW_H-ButMenu_L-5)    // 240 - 20 - 5 = 215

#define RealFrame_L (Frame_L-4) // 400 - 4 = 396
#define RealFrame_H (Frame_H-4) // 215 - 4 = 211

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
void Windows_InitInfo( u8 Enable );
void Windows_WorkInfo( void );
void Windows_InitSDCard( void );
void Windows_WorkSDCard( void );
void Windows_InitWaveFrom( void );
void Windows_WorkWaveFrom( void );
void Windows_InitCtrl( void );
void Windows_WorkCtrl( void );

void SD_Recorder( void );

extern u8 RecorderFileNo;
extern u8 WaveName;
extern s16 ReadIMU[8];
extern u16 TouchBuf[3];
extern s16 WaveFormData[4];
extern Sensor_Mode SensorMode;
extern u8 SelChannal[4];
extern u16 RecorderFreq;
extern u8 DelRecorderFile;
/*=====================================================================================================*/
/*=====================================================================================================*/
#endif

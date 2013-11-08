/*=====================================================================================================*/
/*=====================================================================================================*/
#include <stdlib.h>
#include "stm32f4_system.h"
#include "stm32f4_usart.h"
#include "stm32f4_i2c.h"
#include "stm32f4_sdio.h"
#include "stm32f4_evb_i2c_tsc.h"
#include "system_windows.h"
#include "system_waveForm.h"
#include "module_mpu6050.h"
#include "module_ssd1963.h"
#include "algorithm_compare.h"
#include "ff.h"
#include "diskio.h"
#include "interface.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
#define FSM_TFT  0
#define FSM_UART 1
#define FSM_DATA 2

#define SampleRate_100Hz  1680000 // 168MHz / 1680000 =  100Hz =   10 ms
#define SampleRate_200Hz  840000  // 168MHz / 840000  =  200Hz =    5 ms
#define SampleRate_400Hz  420000  // 168MHz / 420000  =  400Hz =  2.5 ms
#define SampleRate_500Hz  336000  // 168MHz / 336000  =  500Hz =    2 ms
#define SampleRate_800Hz  210000  // 168MHz / 210000  =  800Hz = 1.25 ms
#define SampleRate_1000Hz 168000  // 168MHz / 168000  = 1000Hz =    1 ms

#define RecorderFreq_10Hz  50
#define RecorderFreq_50Hz  10
#define RecorderFreq_100Hz 5

#define RecorderFileName  ("RECORDER.DAT")

#define Axis_X 8
#define Axis_Y 3

FATFS fatfs;
FRESULT res;
FILINFO finfo;
DIR dirs;
FIL file;

u8 DelRecorderFile = 1;
u8 SelChannal[4] = {1, 3, 8, 10};
u8 RecorderFileNo = 0;
//vu16* SD_Buf = 0;
u16 SD_Buf[SD_BUF_SIZE] = {0};
vu8 SD_Flag = 0;
u16 RecorderFreq = RecorderFreq_10Hz;
extern WaveForm_Struct WaveForm;
/*=====================================================================================================*/
/*=====================================================================================================*/
void System_Init( void )
{
  u32 i = 0;
  u8 RecorderInfo[16] = {0};

  u8 ReadID = 0;
  u8 InitSta = ERROR;

  SystemInit();
  GPIO_Config();
  I2C_Config();
  LCD_Init();

  Delay_10ms(10);
  Windows_Init();

  LCD_PutStr(Axis_X, Axis_Y+16*0, (u8*)"------------------------------------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*1, (u8*)"                Init Information                ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*2, (u8*)"------------------------------------------------", ASCII1608, WHITE, BLACK);

  // SD Card Init
  LCD_PutStr(Axis_X, Axis_Y+16*3, (u8*)" SD Init ... ", ASCII1608, WHITE, BLACK);
  while(SD_Init() != SD_OK) {
    LCD_PutStr(Axis_X+8*13, Axis_Y+16*3, (u8*)"Failed!!", ASCII1608, WHITE, BLACK);
  }
  LCD_PutStr(Axis_X+8*13, Axis_Y+16*3, (u8*)"OK!!", ASCII1608, WHITE, BLACK);

  LCD_PutStr(Axis_X,      Axis_Y+16*4, (u8*)" Capacity      : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*17, Axis_Y+16*4, Type_D, 5, SDCardInfo.CardCapacity>>20, WHITE, BLACK);
  LCD_PutStr(Axis_X+8*22, Axis_Y+16*4, (u8*)" MB", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*5, (u8*)" CardBlockSize : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*17, Axis_Y+16*5, Type_D, 5, SDCardInfo.CardBlockSize, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*6, (u8*)" CardType      : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*17, Axis_Y+16*6, Type_D, 5, SDCardInfo.CardType, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*7, (u8*)" RCA           : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*17, Axis_Y+16*7, Type_D, 5, SDCardInfo.RCA, WHITE, BLACK);

  res = f_mount(0, &fatfs);
  res = f_opendir(&dirs, "0:/");

  RecorderInfo[0] = 120;
  RecorderInfo[2] = 'H';
  RecorderInfo[3] = 'z';
  RecorderInfo[6] = 'S';
  RecorderInfo[7] = 'i';
  RecorderInfo[8] = 'z';
  RecorderInfo[9] = 'e';

  res = f_open(&file, RecorderFileName, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
  if(res==FR_OK) {
    f_lseek(&file, 0);
    res = f_write(&file, RecorderInfo, 16, &i);
  }
  else if(res==FR_NO_FILE) {
    f_open(&file, RecorderFileName, FA_CREATE_NEW | FA_READ | FA_WRITE);
    res = f_write(&file, RecorderInfo, 16, &i);
  }
  else {
    LCD_PutStr(Axis_X+8*17, Axis_Y+16*3, (u8*)" ... FatFs failed!!", ASCII1608, WHITE, BLACK);
  }
  f_close(&file);

  // MPU-6050 Init
  LCD_PutStr(Axis_X, Axis_Y+16*9, (u8*)" MPU-6050 Init ... ", ASCII1608, WHITE, BLACK);
  InitSta = MPU6050_Init();
  if(InitSta != SUCCESS)
    LCD_PutStr(Axis_X+8*19, Axis_Y+16*9, (u8*)"Failed!!", ASCII1608, WHITE, BLACK);
  else
    LCD_PutStr(Axis_X+8*19, Axis_Y+16*9, (u8*)"OK!!", ASCII1608, WHITE, BLACK);

  LCD_PutStr(Axis_X, Axis_Y+16*10, (u8*)" Device ID     : 0x", ASCII1608, WHITE, BLACK);
  I2C_DMA_ReadReg(MPU6050_I2C_ADDR,  MPU6050_WHO_AM_I, &ReadID, 1);
  LCD_PutNum(Axis_X+8*19, Axis_Y+16*10,  Type_H, 2, ReadID, WHITE, BLACK);

  // nRF24L01 Init
  LCD_PutStr(Axis_X, Axis_Y+16*12, (u8*)" nRF24L01 Init ... ", ASCII1608, WHITE, BLACK);
  InitSta = ERROR;
  if(InitSta != SUCCESS)
    LCD_PutStr(Axis_X+8*19, Axis_Y+16*12, (u8*)"Failed!!", ASCII1608, WHITE, BLACK);
  else
    LCD_PutStr(Axis_X+8*19, Axis_Y+16*12, (u8*)"OK!!", ASCII1608, WHITE, BLACK);

  if(SysTick_Config(SampleRate_500Hz)) {
    while(1);
  }

  /* Wait Correction */
  while(SensorMode != Mode_Algorithm) {
    LED_4 = 1;
  }
  LED_4 = 0;
}
/*=====================================================================================================*/
/*=====================================================================================================*/
int main( void )
{
  u32 i = 0;
  u8 FSM_State = FSM_TFT;
  u8 RecorderInfo[16] = {0};

  System_Init();

  while(1) {
    LED_1 = !LED_1;
    switch(FSM_State) {
      case FSM_TFT:
        Windows_Ctrl();
        FSM_State = FSM_UART;
        break;

      case FSM_UART:
        
        FSM_State = FSM_DATA;
        break;

      case FSM_DATA:
        switch(SelChannal[0]) {
          case 0:
            if(DelRecorderFile == 1) {

              res = f_mount(0, &fatfs);
              res = f_opendir(&dirs, "0:/");

              RecorderInfo[0] = 120;
              RecorderInfo[2] = 'H';
              RecorderInfo[3] = 'z';
              RecorderInfo[6] = 'S';
              RecorderInfo[7] = 'i';
              RecorderInfo[8] = 'z';
              RecorderInfo[9] = 'e';

              res = f_open(&file, RecorderFileName, FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
              res = f_write(&file, RecorderInfo, 16, &i);

              f_close(&file);

              DelRecorderFile = 0;
              SD_Flag = 0;
            }
            SD_Recorder();
            break;
          case 1:

            break;
        }
        switch(SelChannal[1]) {
          case 3:
            RecorderFreq = RecorderFreq_10Hz;
            break;
          case 4:
            RecorderFreq = RecorderFreq_50Hz;
            break;
          case 5:
            RecorderFreq = RecorderFreq_100Hz;
            break;
        }
        switch(SelChannal[2]) {
          case 6:
            WaveName = WaveName_Acc;
            break;
          case 7:
            WaveName = WaveName_Gyr;
            break;
          case 8:
            WaveName = WaveName_Ang;
            break;
        }
        switch(SelChannal[3]) {
          case 9:
            WaveForm.Scale[0] = 50;
            WaveForm.Scale[1] = 50;
            WaveForm.Scale[2] = 50;
            break;
          case 10:
            WaveForm.Scale[0] = 100;
            WaveForm.Scale[1] = 100;
            WaveForm.Scale[2] = 100;
            break;
          case 11:
            WaveForm.Scale[0] = 180;
            WaveForm.Scale[1] = 180;
            WaveForm.Scale[2] = 180;
            break;
        }
        FSM_State = FSM_TFT;
        break;
    }
  }
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void GPIO_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOI | 
	                       RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOF, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOF, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOI, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  LED_1 = 0;
	LED_2 = 0;
	LED_3 = 0;
	LED_4 = 0;
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_Ctrl( void )
{
	static u8 menu_select = Menu_INFO;

  u16 TouchX = TouchBuf[0];
  u16 TouchY = TouchBuf[1];
  u16 TouchZ = TouchBuf[2];

	if(KEY_WAKE==1) {
    menu_select++;
    if(menu_select>4) menu_select = Menu_INFO;
    Windows_SelMenu(menu_select);
    Delay_10ms(5);
  }
  else if(KEY_TAMP==0) {
    menu_select--;
    if(menu_select<1) menu_select = Menu_CTRL;
    Windows_SelMenu(menu_select);
    Delay_10ms(5);
  }

  if(TouchZ>20) {
    if(TouchX<150) {
      if(TouchY<256) {
        menu_select = Menu_CTRL;
        Windows_SelMenu(menu_select);
      }
      else if((TouchY<512)&&(TouchY>255)) {
        menu_select = Menu_WAVE;
        Windows_SelMenu(menu_select);
      }
      else if((TouchY<768)&&(TouchY>511)) {
        menu_select = Menu_SDCARD;
        Windows_SelMenu(menu_select);
      }
      else if(TouchY>767) {
        menu_select = Menu_INFO;
        Windows_SelMenu(menu_select);
      }
    }
  }

  Windows_Interface(menu_select);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_InitInfo( u8 Enable )
{
  LCD_PutStr(Axis_X, Axis_Y+16*0, (u8*)"------------------------------------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*1, (u8*)"                Init Information                ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*2, (u8*)"------------------------------------------------", ASCII1608, WHITE, BLACK);

  // SD Card Init
  LCD_PutStr(Axis_X, Axis_Y+16*3, (u8*)" SD Init ... ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X+8*13, Axis_Y+16*3, (u8*)"OK!!", ASCII1608, WHITE, BLACK);

  LCD_PutStr(Axis_X,      Axis_Y+16*4, (u8*)" Capacity      : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*17, Axis_Y+16*4, Type_D, 5, SDCardInfo.CardCapacity>>20, WHITE, BLACK);
  LCD_PutStr(Axis_X+8*22, Axis_Y+16*4, (u8*)" MB", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*5, (u8*)" CardBlockSize : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*17, Axis_Y+16*5, Type_D, 5, SDCardInfo.CardBlockSize, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*6, (u8*)" CardType      : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*17, Axis_Y+16*6, Type_D, 5, SDCardInfo.CardType, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*7, (u8*)" RCA           : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*17, Axis_Y+16*7, Type_D, 5, SDCardInfo.RCA, WHITE, BLACK);

  // MPU-6050 Init
  LCD_PutStr(Axis_X, Axis_Y+16*9, (u8*)" MPU-6050 Init ... ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X+8*19, Axis_Y+16*9, (u8*)"OK!!", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*10, (u8*)" Device ID     : 0x68", ASCII1608, WHITE, BLACK);

  // nRF24L01 Init
  LCD_PutStr(Axis_X, Axis_Y+16*12, (u8*)" nRF24L01 Init ... ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X+8*19, Axis_Y+16*12, (u8*)"Failed!!", ASCII1608, WHITE, BLACK);
}
void Windows_WorkInfo( void )
{
  // None
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_InitSDCard( void )
{
  u8 i = 0;
  u8 FileNum = 0;
  u8 RecorderFileNameBuf[] = RecorderFileName;

  LCD_PutStr(Axis_X, Axis_Y+16*0, (u8*)"------------------------------------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*1, (u8*)"                    SD Card                     ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*2, (u8*)"------------------------------------------------", ASCII1608, WHITE, BLACK);

  res = f_mount(0, &fatfs);
  res = f_opendir(&dirs, "0:/");

  LCD_PutStr(Axis_X, Axis_Y+16*3, (u8*)" File name : ", ASCII1608, WHITE, BLACK);
  do {
    res = f_readdir(&dirs, &finfo);
    if(!finfo.fname[0])
      break;
    if(FileNum<9) {
      i = CmpArr_U8((u8*)finfo.fname, RecorderFileNameBuf, sizeof(RecorderFileNameBuf));
      if(i == SUCCESS)
        RecorderFileNo = FileNum;
      f_open(&file, finfo.fname, FA_OPEN_EXISTING | FA_READ);
      LCD_PutStr(Axis_X,     Axis_Y+16*(4+FileNum), (u8*)" ... ", ASCII1608, WHITE, BLACK);
      LCD_PutStr(Axis_X+8*5, Axis_Y+16*(4+FileNum), (u8*)finfo.fname, ASCII1608, WHITE, BLACK);
      LCD_PutNum(400-8*17,   Axis_Y+16*(4+FileNum), Type_D, 10, file.fsize, WHITE, BLACK);
      LCD_PutStr(400-8*7,    Axis_Y+16*(4+FileNum), (u8*)" Bytes", ASCII1608, WHITE, BLACK);
      f_close(&file);
    }
    else {
      FileNum = 0;
      break;
    }
    FileNum++;
  } while(finfo.fname[0]);
}
void Windows_WorkSDCard( void )
{
  f_open(&file, RecorderFileName, FA_OPEN_EXISTING | FA_READ);
  LCD_PutNum(400-8*17, Axis_Y+16*(4+RecorderFileNo), Type_D, 10, file.fsize, WHITE, BLACK);
  f_close(&file);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_InitWaveFrom( void )
{
  LCD_DrawRectangleFull(2, 2, RealFrame_L, RealFrame_H, BLACK);
  LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*0, (u8*)"------", ASCII1608, WHITE, BLACK);
  switch(WaveName) {
    case WaveName_Acc:
      LCD_PutStr(WaveFromNum_X+4, WaveFromNum_Y+8*2, (u8*)" Acc ", ASCII1608, WHITE, BLACK);
      break;
    case WaveName_Gyr:
      LCD_PutStr(WaveFromNum_X+4, WaveFromNum_Y+8*2, (u8*)" Gyr ", ASCII1608, WHITE, BLACK);
      break;
    case WaveName_Ang:
      LCD_PutStr(WaveFromNum_X+4, WaveFromNum_Y+8*2, (u8*)" Ang ", ASCII1608, WHITE, BLACK);
      break;
  }
  LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*4,  (u8*)"------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*7,  (u8*)"Axis-X", ASCII1608, RED, BLACK);
  LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*12, (u8*)"Axis-Y", ASCII1608, GREEN, BLACK);
  LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*17, (u8*)"Axis-Z", ASCII1608, BLUE, BLACK);
  LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*22, (u8*)"------", ASCII1608, WHITE, BLACK);
}
void Windows_WorkWaveFrom( void )
{
  WaveForm.Data[0] = WaveFormData[0];
  WaveForm.Data[1] = WaveFormData[1];
  WaveForm.Data[2] = WaveFormData[2];
  LCD_PutNum(WaveFromNum_X, WaveFromNum_Y+8*9,  Type_I, 5, WaveForm.Data[0], RED, BLACK);
  LCD_PutNum(WaveFromNum_X, WaveFromNum_Y+8*14, Type_I, 5, WaveForm.Data[1], GREEN, BLACK);
  LCD_PutNum(WaveFromNum_X, WaveFromNum_Y+8*19, Type_I, 5, WaveForm.Data[2], BLUE, BLACK);
  WaveFormPrint(&WaveForm);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
#define ButtonColor     GREEN
#define ButtonFontColor BLACK
void Windows_InitCtrl( void )
{
  LCD_PutStr(Axis_X, Axis_Y+16*0, (u8*)"------------------------------------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*1, (u8*)"                     Config                     ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*2, (u8*)"------------------------------------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*3+24*1-14, (u8*)" SD Recorder   : ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*4+24*2-14, (u8*)" Recorder Freq : ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*5+24*3-14, (u8*)" Wave Select   : ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*6+24*4-14, (u8*)" Wave Scale    : ", ASCII1608, WHITE, BLACK);

  Windows_DrawButtonMenuB(0, 0, WHITE);
  Windows_DrawButtonMenuB(1, 0, WHITE);
  Windows_DrawButtonFontB(0, WHITE, BLACK);
  Windows_DrawButtonFontB(1, WHITE, BLACK);

  Windows_DrawButtonMenuB(3, 0, WHITE);
  Windows_DrawButtonMenuB(4, 0, WHITE);
  Windows_DrawButtonMenuB(5, 0, WHITE);
  Windows_DrawButtonFontB(3, WHITE, BLACK);
  Windows_DrawButtonFontB(4, WHITE, BLACK);
  Windows_DrawButtonFontB(5, WHITE, BLACK);

  Windows_DrawButtonMenuB(6, 0, WHITE);
  Windows_DrawButtonMenuB(7, 0, WHITE);
  Windows_DrawButtonMenuB(8, 0, WHITE);
  Windows_DrawButtonFontB(6, WHITE, BLACK);
  Windows_DrawButtonFontB(7, WHITE, BLACK);
  Windows_DrawButtonFontB(8, WHITE, BLACK);

  Windows_DrawButtonMenuB(9,  0, WHITE);
  Windows_DrawButtonMenuB(10, 0, WHITE);
  Windows_DrawButtonMenuB(11, 0, WHITE);
  Windows_DrawButtonFontB(9,  WHITE, BLACK);
  Windows_DrawButtonFontB(10, WHITE, BLACK);
  Windows_DrawButtonFontB(11, WHITE, BLACK);

  Windows_DrawButtonSelB(0, SelChannal[0], ButtonFontColor, ButtonColor);
  Windows_DrawButtonSelB(1, SelChannal[1], ButtonFontColor, ButtonColor);
  Windows_DrawButtonSelB(2, SelChannal[2], ButtonFontColor, ButtonColor);
  Windows_DrawButtonSelB(3, SelChannal[3], ButtonFontColor, ButtonColor);
}
void Windows_WorkCtrl( void )
{
  u8 SelButton = 0;
  u16 TouchX = TouchBuf[0];
  u16 TouchY = TouchBuf[1];
  u16 TouchZ = TouchBuf[2];

  LCD_PutNum(352, Axis_Y+16*0, Type_D, 5, TouchX, RED,   BLACK);
  LCD_PutNum(352, Axis_Y+16*1, Type_D, 5, TouchY, GREEN, BLACK);
  LCD_PutNum(352, Axis_Y+16*2, Type_D, 5, TouchZ, BLUE,  BLACK);

  if(TouchZ>60) {
    if((TouchX>650) && (TouchX<730)) {
      if((TouchY>470) && (TouchY<630)) {
        SelButton = 0;
        Windows_DrawButtonSelB(0, SelButton, ButtonFontColor, ButtonColor);
      }
      else if((TouchY>280) && (TouchY<440)) {
        SelButton = 1;
        Windows_DrawButtonSelB(0, SelButton, ButtonFontColor, ButtonColor);
        DelRecorderFile = 1;
        SD_Flag = 0;
      }
    }
    else if((TouchX>500) && (TouchX<580)) {
      if((TouchY>470) && (TouchY<630)) {
        SelButton = 3;
        Windows_DrawButtonSelB(1, SelButton, ButtonFontColor, ButtonColor);
      }
      else if((TouchY>280) && (TouchY<440)) {
        SelButton = 4;
        Windows_DrawButtonSelB(1, SelButton, ButtonFontColor, ButtonColor);
      }
      else if((TouchY>90) && (TouchY<250)) {
        SelButton = 5;
        Windows_DrawButtonSelB(1, SelButton, ButtonFontColor, ButtonColor);
      }
    }
    else if((TouchX>350) && (TouchX<430)) {
      if((TouchY>470) && (TouchY<630)) {
        SelButton = 6;
        Windows_DrawButtonSelB(2, SelButton, ButtonFontColor, ButtonColor);
      }
      else if((TouchY>280) && (TouchY<440)) {
        SelButton = 7;
        Windows_DrawButtonSelB(2, SelButton, ButtonFontColor, ButtonColor);
      }
      else if((TouchY>90) && (TouchY<250)) {
        SelButton = 8;
        Windows_DrawButtonSelB(2, SelButton, ButtonFontColor, ButtonColor);
      }

    }
    else if((TouchX>200) && (TouchX<280)) {
      if((TouchY>470) && (TouchY<630)) {
        SelButton = 9;
        Windows_DrawButtonSelB(3, SelButton, ButtonFontColor, ButtonColor);
      }
      else if((TouchY>280) && (TouchY<440)) {
        SelButton = 10;
        Windows_DrawButtonSelB(3, SelButton, ButtonFontColor, ButtonColor);
      }
      else if((TouchY>90) && (TouchY<250)) {
        SelButton = 11;
        Windows_DrawButtonSelB(3, SelButton, ButtonFontColor, ButtonColor);
      }
    }
  }
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void SD_Recorder( void )
{
  u32 i = 0;
  s16 SD_WriteBuf[SD_BUF_SIZE] = {0};

  if(SD_Flag==1) {
    SD_Flag = 0;

    for(i=0; i<SD_BUF_SIZE; i++)
      SD_WriteBuf[i] = SD_Buf[i];

    f_open(&file, RecorderFileName, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    f_lseek(&file, file.fsize);
    f_write(&file, SD_WriteBuf, SD_BUF_SIZE, &i);
    f_close(&file);
  }
}
/*=====================================================================================================*/
/*=====================================================================================================*/

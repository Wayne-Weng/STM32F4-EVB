/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "stm32f4_usart.h"
#include "stm32f4_i2c.h"
#include "stm32f4_sdio.h"
#include "stm32f4_evb_i2c_tsc.h"
#include "system_windows.h"
#include "module_mpu6050.h"
#include "module_ssd1963.h"
#include "ff.h"
#include "diskio.h"
#include "interface.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
#define TOUCH_EN  0
#define SENSOR_EN 1

FATFS fatfs;
FRESULT res;
FILINFO finfo;
DIR dirs;
FIL file;
/*=====================================================================================================*/
/*=====================================================================================================*/
void System_Init( void )
{
  SystemInit();
  GPIO_Config();
#if SENSOR_EN
  I2C_Config();
#endif
  LCD_Init();
#if TOUCH_EN
	TSC_Config();
#endif
  Delay_10ms(10);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_Ctrl( void )
{
	static u8 menu_select = Menu_INIT;
#if TOUCH_EN
	TSC_STATE *tsc_info;
#endif
	if(KEY_WAKE==1) {
    menu_select++;
    if(menu_select>4) menu_select = Menu_INIT;
    Windows_SelMenu(menu_select);
    Delay_10ms(5);
  }
  else if(KEY_TAMP==0) {
    menu_select--;
    if(menu_select<1) menu_select = Menu_INFO;
    Windows_SelMenu(menu_select);
    Delay_10ms(5);
  }
#if TOUCH_EN
  tsc_info = TSC_TS_GetState();

  if(tsc_info->Z>20) {
    if(tsc_info->X<150) {
      if(tsc_info->Y<256) {
        menu_select = Menu_INFO;
        Windows_SelMenu(menu_select);
      }
      else if((tsc_info->Y<512)&&(tsc_info->Y>255)) {
        menu_select = Menu_WAVE;
        Windows_SelMenu(menu_select);
      }
      else if((tsc_info->Y<768)&&(tsc_info->Y>511)) {
        menu_select = Menu_SDCARD;
        Windows_SelMenu(menu_select);
      }
      else if(tsc_info->Y>767) {
        menu_select = Menu_INIT;
        Windows_SelMenu(menu_select);
      }
    }
  }
#endif
  Windows_Interface(menu_select);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
int main( void )
{
  System_Init();
  Windows_Init();
  Delay_10ms(10);
//  SDCARD_InitInfo();
#if SENSOR_EN
  SENSOR_InitInfo();
#endif
  while(1) {
    Windows_Ctrl();
    LED_1 = ~LED_1;
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
#define Axis_X 10
#define Axis_Y 10
void SENSOR_InitInfo( void )
{
  u8 ReadID = 0;
  u8 InitSta = ERROR;

  LCD_PutStr(Axis_X, Axis_Y+16*0, (u8*)"-----------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*1, (u8*)" MPU-6050 demo",          ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*2, (u8*)"-----------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*3, (u8*)" MPU-6050 Init ... ",     ASCII1608, WHITE, BLACK);
  InitSta = MPU6050_Init();
  if(InitSta != SUCCESS)
    LCD_PutStr(Axis_X+8*19, Axis_Y+16*3, (u8*)"Failed!!", ASCII1608, WHITE, BLACK);
  else
    LCD_PutStr(Axis_X+8*19, Axis_Y+16*3, (u8*)"OK!!", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*4, (u8*)" MPU-6050 ID = 0x", ASCII1608, WHITE, BLACK);
  I2C_DMA_ReadReg(MPU6050_I2C_ADDR,  MPU6050_WHO_AM_I, &ReadID, 1);
  LCD_PutNum(Axis_X+8*17, Axis_Y+16*4,  Type_H, 2, ReadID, WHITE, BLACK);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void SDCARD_InitInfo( void )
{
  u32 i = 0;
  u8 FileNum = 0;
  u8 ReadBuf[128] = {0};
  u8 WriteData[50] = "ABCDEFG";

  // SD Card Init Info
  LCD_PutStr(Axis_X, Axis_Y+16*0, (u8*)"-----------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*1, (u8*)" SDIO SD Fatfs demo",     ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*2, (u8*)"-----------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*3, (u8*)" SD Init ... ", ASCII1608, WHITE, BLACK);
  while(SD_Init() != SD_OK) {
    LCD_PutStr(Axis_X+8*13, Axis_Y+16*3, (u8*)"Failed!!", ASCII1608, WHITE, BLACK);
    while(1) {
      LED_1 = ~LED_1;
      Delay_100ms(2);
    }
  }
  LCD_PutStr(Axis_X+8*13, Axis_Y+16*3, (u8*)"OK!!", ASCII1608, WHITE, BLACK);

  LCD_PutStr(Axis_X,      Axis_Y+16*5,  (u8*)"---- SD Init Info -----", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*6,  (u8*)" Capacity : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*12, Axis_Y+16*6,  Type_D, 5, SDCardInfo.CardCapacity>>20, WHITE, BLACK);
  LCD_PutStr(Axis_X+8*17, Axis_Y+16*6,  (u8*)" MB", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*7,  (u8*)" CardBlockSize : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*17, Axis_Y+16*7,  Type_D, 5, SDCardInfo.CardBlockSize, WHITE, BLACK);
  LCD_PutStr(Axis_X+8*22, Axis_Y+16*7,  (u8*)" ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*8,  (u8*)" CardType : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*12, Axis_Y+16*8,  Type_D, 5, SDCardInfo.CardType, WHITE, BLACK);
  LCD_PutStr(Axis_X+8*17, Axis_Y+16*8,  (u8*)" ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*9,  (u8*)" RCA : ", ASCII1608, WHITE, BLACK);
  LCD_PutNum(Axis_X+8*7,  Axis_Y+16*9,  Type_D, 5, SDCardInfo.RCA, WHITE, BLACK);
  LCD_PutStr(Axis_X+8*12, Axis_Y+16*9,  (u8*)" ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X,      Axis_Y+16*10, (u8*)"-----------------------", ASCII1608, WHITE, BLACK);

  //////////////////// Wait
  while(KEY_WAKE != 1) {
    LED_2 = ~LED_2;
    Delay_100ms(2);
  }
  LCD_Clear(BLACK);

  LCD_PutStr(Axis_X, Axis_Y+16*0, (u8*)"-----------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*1, (u8*)" SD_Card Read Directory", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*2, (u8*)"-----------------------", ASCII1608, WHITE, BLACK);

  res = f_mount(0, &fatfs);
  res = f_opendir(&dirs, "0:/");
  res = f_readdir(&dirs, &finfo);
  while(res!= FR_OK) {
    LCD_PutStr(Axis_X, Axis_Y+16*3, (u8*)" Fatfs failed!!", ASCII1608, WHITE, BLACK);
    while(1) {
      LED_1 = ~LED_1;
      Delay_100ms(2);
    }
  }
  LCD_PutStr(Axis_X, Axis_Y+16*3, (u8*)" File name : ", ASCII1608, WHITE, BLACK);

  while(finfo.fname[0]) {
    f_readdir(&dirs, &finfo);
    if(!finfo.fname[0])
      break;
    if(FileNum<11) {
      LCD_PutStr(Axis_X,     Axis_Y+16*(3+FileNum), (u8*)" ... ", ASCII1608, WHITE, BLACK);
      LCD_PutStr(Axis_X+8*5, Axis_Y+16*(3+FileNum), (u8*)finfo.fname, ASCII1608, WHITE, BLACK);
    }
    else if((FileNum>=11) && (FileNum<22)) {
      LCD_PutStr(Axis_X+200,     Axis_Y+16*(3+FileNum-11), (u8*)" ... ", ASCII1608, WHITE, BLACK);
      LCD_PutStr(Axis_X+8*5+200, Axis_Y+16*(3+FileNum-11), (u8*)finfo.fname, ASCII1608, WHITE, BLACK);
    }
    else {
      FileNum = 0;
      break;
    }
    FileNum++;
  }

  //////////////////// Wait
  while(KEY_WAKE != 1) {
    LED_3 = ~LED_3;
    Delay_100ms(2);
  }
  LCD_Clear(BLACK);

  LCD_PutStr(Axis_X, Axis_Y+16*0, (u8*)"-----------------------", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*1, (u8*)" SD_Card Write Data",     ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X, Axis_Y+16*2, (u8*)"-----------------------", ASCII1608, WHITE, BLACK);

  LCD_PutStr(Axis_X, Axis_Y+16*3, (u8*)" f_open ... ", ASCII1608, WHITE, BLACK);
  res = f_open(&file,"SDCard_K.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
  if(res==FR_OK)
    LCD_PutStr(Axis_X+8*12, Axis_Y+16*3, (u8*)"OK!!", ASCII1608, WHITE, BLACK);
  else
    LCD_PutStr(Axis_X+8*12, Axis_Y+16*3, (u8*)"failed!!", ASCII1608, WHITE, BLACK);

  LCD_PutStr(Axis_X, Axis_Y+16*4, (u8*)" f_write ... ", ASCII1608, WHITE, BLACK);
  res = f_write(&file, WriteData, 20, &i); 
  if(res==FR_OK)
    LCD_PutStr(Axis_X+8*13, Axis_Y+16*4, (u8*)"OK!!", ASCII1608, WHITE, BLACK);
  else
    LCD_PutStr(Axis_X+8*13, Axis_Y+16*4, (u8*)"failed!!", ASCII1608, WHITE, BLACK);

  file.fptr = 0;

  LCD_PutStr(Axis_X, Axis_Y+16*5, (u8*)" f_read ... ", ASCII1608, WHITE, BLACK);
  res = f_read(&file, ReadBuf, 100, &i);
  if(res==FR_OK)
    LCD_PutStr(Axis_X+8*12, Axis_Y+16*5, (u8*)"OK!!", ASCII1608, WHITE, BLACK);
  else
    LCD_PutStr(Axis_X+8*12, Axis_Y+16*5, (u8*)"failed!!", ASCII1608, WHITE, BLACK);

  f_close(&file);

  LCD_PutStr(Axis_X, Axis_Y+16*7, (u8*)" Read String : ", ASCII1608, WHITE, BLACK);
  LCD_PutStr(Axis_X+8*1, Axis_Y+16*8, (u8*)ReadBuf, ASCII1608, WHITE, BLACK);

  //////////////////// Wait
  while(KEY_WAKE != 1) {
    LED_4 = ~LED_4;
    Delay_100ms(2);
  }
  LCD_Clear(BLACK);
}
/*=====================================================================================================*/
/*=====================================================================================================*/

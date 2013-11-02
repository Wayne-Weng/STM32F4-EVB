/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "stm32f4_usart.h"
#include "stm32f4_sdio.h"
#include "system_windows.h"
#include "module_ssd1963.h"
#include "ff.h"
#include "diskio.h"
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
void GPIO_Config( void );
/*=====================================================================================================*/
/*=====================================================================================================*/
FATFS fatfs;
FRESULT res;
FILINFO finfo;
DIR dirs;
FIL file;
/*=====================================================================================================*/
/*=====================================================================================================*/
void System_Init( void )
{
  /* Hardware */
  SystemInit();
  GPIO_Config();
  LCD_Init();
  Delay_10ms(10);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
int main( void )
{
  u32 i = 0;
  u8 FileNum = 0;
  u8 ReadBuf[128] = {0};
  u8 WriteData[50] = "ABCDEFG";

  System_Init();

  // SD Card Init Info
  #define Axis_X 10
  #define Axis_Y 0
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

  while(1) {
    LED_4 = ~LED_4;
    Delay_100ms(2);
  }
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void GPIO_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOI|
	                       RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOF, ENABLE);

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

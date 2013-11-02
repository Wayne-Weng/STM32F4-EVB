/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "stm32f4_sdio.h"
#include "system_windows.h"
#include "module_ssd1963.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
#define KEY_WAKE  PAI(0)
#define KEY       PII(8)
#define KEY_TAMP  PCI(13)

#define LED1 PFO(7)
#define LED2 PFO(8)
#define LED3 PFO(9)
#define LED4 PFO(10)
/*=====================================================================================================*/
/*=====================================================================================================*/
void GPIO_Config( void );
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
  u8 menu_select = Menu_CONF;

  System_Init();
  Windows_DrawMenu();

  while(1){
    if(KEY_WAKE==1) {
      menu_select++;
      if(menu_select>4) menu_select = Menu_CONF;
      Windows_SelMenu(menu_select);
      Delay_100ms(1);
    }else if(KEY_TAMP==0) {
      menu_select--;
      if(menu_select<1) menu_select = Menu_CTRL;
      Windows_SelMenu(menu_select);
      Delay_100ms(1);
    }
    Windows_Interface(menu_select);
    LED1 = ~LED1;
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

  LED1 = 0;
	LED2 = 0;
	LED3 = 0;
	LED4 = 0;

}
/*=====================================================================================================*/
/*=====================================================================================================*/

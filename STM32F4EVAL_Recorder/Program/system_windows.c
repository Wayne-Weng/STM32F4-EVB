/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "stm32f4_usart.h"
#include "system_windows.h"
#include "system_waveForm.h"
#include "module_mpu6050.h"
#include "module_ssd1963.h"
#include "algorithm_mathUnit.h"
#include "interface.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
uc8 CTRL_SelStr[4][3][16] = {
  {"ON",    "OFF",  ""},
  {"10 Hz", "50 Hz", "100 Hz"},
  {"Acc",   "Gyro", "Ang"},
  {"50",    "100",  "180"}
};
const float CTRL_SelMed[12] = {
  1.0f, 1.5f, 0.0f,
  2.5f, 2.5f, 3.0f,
  1.5f, 2.0f, 1.5f,
  1.0f, 1.5f, 1.5f
};

WaveForm_Struct WaveForm;
u8 WaveName;
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_Init( void )
{
	WaveForm.Channel = 3;
	WaveForm.WindowColor = WHITE;
	WaveForm.BackColor = BLACK;
	WaveForm.Data[0] = 0;
	WaveForm.Data[1] = 0;
	WaveForm.Data[2] = 0;
	WaveForm.Scale[0] = 100;
	WaveForm.Scale[1] = 100;
	WaveForm.Scale[2] = 100;
	WaveForm.PointColor[0] = RED;
	WaveForm.PointColor[1] = GREEN;
	WaveForm.PointColor[2] = BLUE;

  WaveName = WaveName_Ang;
	Windows_DrawMenu();
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_DrawButtonA( u16 CoordiX, u16 CoordiY, u32 Color )
{
	LCD_DrawRectangle(CoordiX, CoordiY, ButMenu_W, ButMenu_L, Color);
	LCD_DrawRectangle(CoordiX+1, CoordiY+1, ButMenu_W-2, ButMenu_L-2, Color);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
#define ButtinB_W 70
#define ButtinB_L 20
void Windows_DrawButtonB( u16 CoordiX, u16 CoordiY, u32 Color )
{
	LCD_DrawRectangle(CoordiX, CoordiY, ButtinB_W, ButtinB_L, Color);
	LCD_DrawRectangle(CoordiX+1, CoordiY+1, ButtinB_W-2, ButtinB_L-2, Color);
}
void Windows_DrawButtonMenuB( u8 Coordi, u8 Fill, u32 Color )
{
  u8 x = 0, y = 0;

  x = (u8)(Coordi%3);
  y = (u8)(Coordi/3);

  if(Fill)
    LCD_DrawRectangleFull(150+82*x, 58+40*y, ButtinB_W+1, ButtinB_L+1, Color);
  else
    Windows_DrawButtonB(150+82*x, 58+40*y, Color);
}
void Windows_DrawButtonFontB( u8 Coordi, u32 FColor, u32 BColor )
{
  u8 x = 0, y = 0;

  x = (u8)(Coordi%3);
  y = (u8)(Coordi/3);

  LCD_PutStr(185+82*x-8*CTRL_SelMed[Coordi], 61+40*y, (u8*)CTRL_SelStr[y][x], ASCII1608, FColor, BColor);
}
void Windows_DrawButtonSelB( u8 Channal, u8 SelButton, u32 ButtonFontColor, u32 ButtonColor )
{
  Windows_DrawButtonMenuB(SelChannal[Channal], 1, BLACK);
  Windows_DrawButtonMenuB(SelChannal[Channal], 0, WHITE);
  Windows_DrawButtonFontB(SelChannal[Channal], WHITE, BLACK);
  SelChannal[Channal] = SelButton;
  Windows_DrawButtonMenuB(SelChannal[Channal], 1, ButtonColor);
  Windows_DrawButtonFontB(SelChannal[Channal], ButtonFontColor, ButtonColor);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_DrawMenu( void )
{
  //Draw Frame
  LCD_DrawRectangle(0, 0, Frame_L-1, Frame_H-1, WHITE);
  LCD_DrawRectangle(1, 1, Frame_L-3, Frame_H-3, WHITE);
  //Draw Button and Strings
	Windows_DrawButtonA(SelMenu_1, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_1+2+(ButMenu_W>>1)-16,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"INFO",   ASCII1608, WHITE, BLACK);
	Windows_DrawButtonA(SelMenu_2, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_2+2+(ButMenu_W>>1)-24,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"SDCARD", ASCII1608, WHITE, BLACK);
	Windows_DrawButtonA(SelMenu_3, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_3+2+(ButMenu_W>>1)-16,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"WAVE",   ASCII1608, WHITE, BLACK);
	Windows_DrawButtonA(SelMenu_4, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_4+2+(ButMenu_W>>1)-16,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"CTRL",   ASCII1608, WHITE, BLACK);
  
  Windows_SelMenu(1);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void EVENT_FIFO( uc8 *newStr )
{
  u8 i=0,j=0;
  static u8 Line[13][50]={0};
  
  for(i=12;i>0;i--){
    for(j=0;j<50;j++){
      Line[i][j] = Line[i-1][j];
    }
  }
  for(j=0;j<50;j++){
      Line[0][j] = newStr[j];
  }
  for(i=0;i<13;i++){
    LCD_PutStr( 2,  16*i+2,  Line[i], ASCII1608,  WHITE,  BLACK);
  }
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_SelMenu( u8 menuType )
{
  u16 StartCoordi = SelMenu_1+2;
  static u16 last_Coordi = SelMenu_1+2;
  
  switch(menuType){
    case Menu_INFO:
      StartCoordi = SelMenu_1+2;
      break;
    case Menu_SDCARD:
      StartCoordi = SelMenu_2+2;
      break;
    case Menu_WAVE:
      StartCoordi = SelMenu_3+2;
      break;
    case Menu_CTRL:
      StartCoordi = SelMenu_4+2;
      break;
  }

  //Redraw last line
  LCD_DrawLineX(last_Coordi,Frame_H-1, ButMenu_W-3, WHITE);
	LCD_DrawLineX(last_Coordi,Frame_H-2, ButMenu_W-3, WHITE);
  LCD_DrawLineX(last_Coordi,Frame_H+2, ButMenu_W-3, WHITE);
  LCD_DrawLineX(last_Coordi,Frame_H+3, ButMenu_W-3, WHITE);
  
  //Clear line segment to black
  LCD_DrawLineX(StartCoordi,Frame_H-1, ButMenu_W-3, BLACK);
	LCD_DrawLineX(StartCoordi,Frame_H-2, ButMenu_W-3, BLACK);
  LCD_DrawLineX(StartCoordi,Frame_H+2, ButMenu_W-3, BLACK);
  LCD_DrawLineX(StartCoordi,Frame_H+3, ButMenu_W-3, BLACK);
  
  LCD_DrawLineY(last_Coordi-2,Frame_H, 2, BLACK);
	LCD_DrawLineY(last_Coordi-1,Frame_H, 2, BLACK);
  LCD_DrawLineY(last_Coordi-2+ButMenu_W,Frame_H, 2, BLACK);
  LCD_DrawLineY(last_Coordi-2+ButMenu_W-1,Frame_H, 2, BLACK);
  
  LCD_DrawLineY(StartCoordi-2,Frame_H, 2, WHITE);
	LCD_DrawLineY(StartCoordi-1,Frame_H, 2, WHITE);
  LCD_DrawLineY(StartCoordi-2+ButMenu_W,Frame_H, 2, WHITE);
  LCD_DrawLineY(StartCoordi-2+ButMenu_W-1,Frame_H, 2, WHITE);

  //store the current Coordi
  last_Coordi = StartCoordi;
  
  switch(menuType){

/************************** FSM INFO **************************************/
		case Menu_INFO:
      LCD_DrawRectangleFull(2, 2, RealFrame_L, RealFrame_H, BLACK);
      Windows_InitInfo(0);
			break;
    
/************************** FSM SDCARD ************************************/
		case Menu_SDCARD:
			LCD_DrawRectangleFull(2, 2, RealFrame_L, RealFrame_H, BLACK);
      Windows_InitSDCard();
			break;

/************************** FSM WAVE **************************************/
		case Menu_WAVE:
      LCD_DrawRectangleFull(2, 2, RealFrame_L, RealFrame_H, BLACK);
      Windows_InitWaveFrom();
			break;
    
/************************** FSM CTRL **************************************/
		case Menu_CTRL:
			LCD_DrawRectangleFull(2, 2, RealFrame_L, RealFrame_H, BLACK);
      Windows_InitCtrl();
			break;
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_Interface( u8 Menu_Sel )
{
	switch(Menu_Sel) {

/************************** FSM INFO **************************************/
		case Menu_INFO:
      Windows_WorkInfo();
			break;

/************************** FSM SDCARD ************************************/
		case Menu_SDCARD:
      Windows_WorkSDCard();
			break;
    
/************************** FSM WAVE **************************************/
		case Menu_WAVE:
      Windows_WorkWaveFrom();
			break;

/************************** FSM CTRL **************************************/
		case Menu_CTRL:
      Windows_WorkCtrl();
			break;
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*/

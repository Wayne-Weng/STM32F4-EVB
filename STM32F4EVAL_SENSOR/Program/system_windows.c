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
WaveForm_Struct WaveForm;
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

	Windows_DrawMenu();
}
/*=====================================================================================================*/
/*=====================================================================================================*/
#define WINDOW_H 240
#define WINDOW_L 400

#define ButMenu_W	95
#define ButMenu_L	20
void Windows_DrawButton( u16 CoordiX, u16 CoordiY, u32 Color )
{
	LCD_DrawRectangle(CoordiX, CoordiY, ButMenu_W, ButMenu_L, Color);
	LCD_DrawRectangle(CoordiX+1, CoordiY+1, ButMenu_W-2, ButMenu_L-2, Color);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
#define SelMenu_1	(4)
#define SelMenu_2	(SelMenu_1+ButMenu_W+4) // 4 + 95 + 4 = 103
#define SelMenu_3	(SelMenu_2+ButMenu_W+4) // 103 + 95 + 4 = 202
#define SelMenu_4	(SelMenu_3+ButMenu_W+4) // 202 + 95 + 4 = 301

#define Frame_L (SelMenu_4+ButMenu_W+4)   // 301 + 95 + 4 = 400
#define Frame_H (WINDOW_H-ButMenu_L-5)    // 240 - 20 - 5 = 215

#define RealFrame_L (Frame_L-4) // 400 - 4 = 396
#define RealFrame_H (Frame_H-4) // 215 - 4 = 211

void Windows_DrawMenu( void )
{
  //Draw Frame
  LCD_DrawRectangle(0, 0, Frame_L-1, Frame_H-1, WHITE);
  LCD_DrawRectangle(1, 1, Frame_L-3, Frame_H-3, WHITE);
  //Draw Button and Strings
	Windows_DrawButton(SelMenu_1, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_1+2+(ButMenu_W>>1)-16,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"INIT", ASCII1608, WHITE, BLACK);
	Windows_DrawButton(SelMenu_2, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_2+2+(ButMenu_W>>1)-24,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"SDCARD", ASCII1608, WHITE, BLACK);
	Windows_DrawButton(SelMenu_3, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_3+2+(ButMenu_W>>1)-16,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"WAVE", ASCII1608, WHITE, BLACK);
	Windows_DrawButton(SelMenu_4, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_4+2+(ButMenu_W>>1)-16,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"INFO", ASCII1608, WHITE, BLACK);
  
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
#define WaveFromNum_X 347
#define WaveFromNum_Y 8
void Windows_SelMenu( u8 menuType )
{
  u16 StartCoordi = SelMenu_1+2;
  static u16 last_Coordi = SelMenu_1+2;
  
  switch(menuType){
    case Menu_INIT:
      StartCoordi = SelMenu_1+2;
      break;
    case Menu_SDCARD:
      StartCoordi = SelMenu_2+2;
      break;
    case Menu_WAVE:
      StartCoordi = SelMenu_3+2;
      break;
    case Menu_INFO:
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

/************************** FSM CONF **************************************/
		case Menu_INIT:
      LCD_DrawRectangleFull(2, 2, RealFrame_L, RealFrame_H, BLACK);
			break;
    
/************************** FSM EVENT **************************************/
		case Menu_SDCARD:
			LCD_DrawRectangleFull(2, 2, RealFrame_L, RealFrame_H, BLACK);
			break;
    
/************************** FSM INFO **************************************/
		case Menu_WAVE:
      LCD_DrawRectangleFull(2, 2, RealFrame_L, RealFrame_H, BLACK);
      LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*0,  (u8*)"------", ASCII1608, WHITE, BLACK);
      LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*2,  (u8*)" SINE ", ASCII1608, WHITE, BLACK);
      LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*4,  (u8*)"------", ASCII1608, WHITE, BLACK);
      LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*7,  (u8*)"Axis-X", ASCII1608, RED, BLACK);
      LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*12, (u8*)"Axis-Y", ASCII1608, GREEN, BLACK);
      LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*17, (u8*)"Axis-Z", ASCII1608, BLUE, BLACK);
      LCD_PutStr(WaveFromNum_X, WaveFromNum_Y+8*22, (u8*)"------", ASCII1608, WHITE, BLACK);
			break;
    
/************************** FSM CTRL *************************************/
		case Menu_INFO:
			LCD_DrawRectangleFull(2, 2, RealFrame_L, RealFrame_H, BLACK);
			break;
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_Interface( u8 Menu_Sel )
{
  static s16 Sita = 0;

	switch(Menu_Sel) {

/************************** FSM CONF **************************************/
		case Menu_INIT:
			break;
    
/************************** FSM EVENT **************************************/
		case Menu_SDCARD:
			break;
    
/************************** FSM INFO **************************************/
		case Menu_WAVE:
      Sita++;
      if(Sita == 360) Sita = 0;
      WaveForm.Data[0] = ReadIMU[0];
      WaveForm.Data[1] = ReadIMU[1];
      WaveForm.Data[2] = ReadIMU[2];
      LCD_PutNum(WaveFromNum_X, WaveFromNum_Y+8*9,  Type_I, 5, WaveForm.Data[0], RED, BLACK);
      LCD_PutNum(WaveFromNum_X, WaveFromNum_Y+8*14, Type_I, 5, WaveForm.Data[1], GREEN, BLACK);
      LCD_PutNum(WaveFromNum_X, WaveFromNum_Y+8*19, Type_I, 5, WaveForm.Data[2], BLUE, BLACK);
      WaveFormPrint(&WaveForm);
			break;
/************************** FSM CTRL *************************************/
		case Menu_INFO:
			break;
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*/

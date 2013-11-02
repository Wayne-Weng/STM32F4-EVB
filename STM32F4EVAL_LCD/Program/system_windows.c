/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "system_windows.h"
#include "stm32f4_usart.h"
#include "module_ssd1963.h"
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
#define SelMenu_1	4
#define SelMenu_2	SelMenu_1+ButMenu_W+4
#define SelMenu_3	SelMenu_2+ButMenu_W+4
#define SelMenu_4	SelMenu_3+ButMenu_W+4

#define Frame_L SelMenu_4+ButMenu_W+4
#define Frame_H WINDOW_H-ButMenu_L-5
void Windows_DrawMenu( void )
{
  //Draw Frame
  LCD_DrawRectangle(0, 0, Frame_L-1, Frame_H-1, WHITE);
  LCD_DrawRectangle(1, 1, Frame_L-3, Frame_H-3, WHITE);
  //Draw Button and Strings
	Windows_DrawButton(SelMenu_1, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_1+2+(ButMenu_W>>1)-16,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"CONF", ASCII1608, WHITE, BLACK);
	Windows_DrawButton(SelMenu_2, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_2+2+(ButMenu_W>>1)-20,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"EVENT", ASCII1608, WHITE, BLACK);
	Windows_DrawButton(SelMenu_3, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_3+2+(ButMenu_W>>1)-16-1,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"INFO", ASCII1608, WHITE, BLACK);
	Windows_DrawButton(SelMenu_4, WINDOW_H-ButMenu_L-3, WHITE);
	LCD_PutStr(SelMenu_4+2+(ButMenu_W>>1)-16,WINDOW_H-(ButMenu_L>>1)-8-2, (u8*)"CTRL", ASCII1608, WHITE, BLACK);
  
  Windows_SelMenu(1);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_SelMenu( u8 menuType )
{
  u16 StartCoordi = SelMenu_1+2;
  static u16 last_Coordi = SelMenu_1+2;
  
  switch(menuType){
    case Menu_CONF:
      StartCoordi = SelMenu_1+2;
      break;
    case Menu_EVENT:
      StartCoordi = SelMenu_2+2;
      break;
    case Menu_INFO:
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

/************************** FSM CONF **************************************/
		case Menu_CONF:
      
     LCD_DrawRectangleFull(2, 2, Frame_L-4, Frame_H-4, BLACK);
     
     LCD_PutStr( 10,  10,  (u8*)"PID", ASCII1608,  WHITE,  BLACK);
     LCD_PutStr( 10,  10+16,  (u8*)"PID_P : ", ASCII1608,  WHITE,  BLACK);
     LCD_PutStr( 10,  10+16*2,  (u8*)"PID_I : ", ASCII1608,  WHITE,  BLACK);
     LCD_PutStr( 10,  10+16*3,  (u8*)"PID_D : ", ASCII1608,  WHITE,  BLACK);
    
			break;
    
/************************** FSM EVENT **************************************/
		case Menu_EVENT:
			LCD_DrawRectangleFull(2, 2, Frame_L-4, Frame_H-4, BLACK);
			break;
    
/************************** FSM INFO **************************************/
		case Menu_INFO:
      LCD_DrawRectangleFull(2, 2, Frame_L-4, Frame_H-4, BLUE);
			break;
    
/************************** FSM CTRL *************************************/
		case Menu_CTRL:
			LCD_DrawRectangleFull(2, 2, Frame_L-4, Frame_H-4, GREEN);
			break;
	}
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
void Windows_Interface( u8 Menu_Sel )
{
  static u32 i=0;
  u8 str[49]={0};
  
	switch(Menu_Sel) {

/************************** FSM CONF **************************************/
		case Menu_CONF:
      LCD_PutNum( 10+8*8,  10+16*1, Type_D, 3, i, WHITE, BLACK);
      LCD_PutNum( 10+8*8,  10+16*2, Type_D, 3, i>>1, WHITE, BLACK);
      LCD_PutNum( 10+8*8,  10+16*3, Type_D, 3, i>>2, WHITE, BLACK);
      i++;
      if(i==255) i=0;
			break;
    
/************************** FSM EVENT **************************************/
		case Menu_EVENT:
      NumToChar(Type_D, 6, str, i);
      EVENT_FIFO((u8*)"asd1asdasdasd23as23d123as1dasd2ert56ec94690b8q3bv");
			break;
    
/************************** FSM INFO **************************************/
		case Menu_INFO:
			break;
    
/************************** FSM CTRL *************************************/
		case Menu_CTRL:
			break;
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*/

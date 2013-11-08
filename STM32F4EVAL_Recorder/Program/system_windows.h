/* #include "system_windows.h" */

#ifndef __SYSTEM_WINDOWS_H
#define __SYSTEM_WINDOWS_H

#include "stm32f4xx.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
#define Menu_INFO   1
#define Menu_SDCARD 2
#define Menu_WAVE   3
#define Menu_CTRL   4
/*=====================================================================================================*/
/*=====================================================================================================*/
void Windows_Init( void );
void Windows_DrawMenu( void );
void Windows_SelMenu( u8 menuType );
void Windows_Interface( u8 Menu_Sel );
void Windows_DrawButtonB( u16 CoordiX, u16 CoordiY, u32 Color );
void Windows_DrawButtonMenuB( u8 Coordi, u8 Fill, u32 Color );
void Windows_DrawButtonFontB( u8 Coordi, u32 FColor, u32 BColor );
void Windows_DrawButtonSelB( u8 Channal, u8 SelButton, u32 ButtonFontColor, u32 ButtonColor );
/*=====================================================================================================*/
/*=====================================================================================================*/
#endif

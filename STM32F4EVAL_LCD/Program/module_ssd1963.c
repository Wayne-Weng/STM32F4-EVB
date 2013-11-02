/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "stm32f4_usart.h"
#include "module_ssd1963.h"
#include "module_fontlib.h"
#include "arm_math.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
typedef struct {
	__IO uint16_t LCD_REG;
	__IO uint16_t LCD_RAM;
} LCD_TypeDef;
/*=====================================================================================================*/
/*=====================================================================================================*/
#define Bank1_SRAM2_ADDR	0x64000000
#define LCD_BASE			    0x68000000
#define LCD					      ((LCD_TypeDef *) LCD_BASE)
#define MAX_POLY_CORNERS	200
#define POLY_Y(Z)			    ((int32_t)((Points + Z)->X))
#define POLY_X(Z)			    ((int32_t)((Points + Z)->Y))
#define LCD_WriteRAM(var)	(LCD->LCD_RAM = var)
#define LCD_ReadRAM			  (LCD->LCD_RAM)
/*=====================================================================================================*/
/*=====================================================================================================*/
static uint16_t *vram_buffer;
/*=====================================================================================================*/
/*=====================================================================================================*/
void LCD_CtrlLinesConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF, ENABLE);

/*-- GPIO Configuration ------------------------------------------------------*/
	/* SRAM Data lines,  NOE and NWE configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 |
								  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15 | GPIO_Pin_4 |GPIO_Pin_5;;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
								  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
								  GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);
	/* SRAM Address lines configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource0, GPIO_AF_FSMC);
	/* NE3 configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource10, GPIO_AF_FSMC);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void LCD_FSMCConfig(void)
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  p;

	/* Enable FSMC clock */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

	/*-- FSMC Configuration ------------------------------------------------------*/
	/*----------------------- SRAM Bank 3 ----------------------------------------*/
	/* FSMC_Bank1_NORSRAM4 configuration */
	p.FSMC_AddressSetupTime			= 1;
	p.FSMC_AddressHoldTime			= 0;
	p.FSMC_DataSetupTime			= 9;
	p.FSMC_BusTurnAroundDuration	= 0;
	p.FSMC_CLKDivision				= 0;
	p.FSMC_DataLatency				= 0;
	p.FSMC_AccessMode				= FSMC_AccessMode_A;
	/* Color LCD configuration ------------------------------------
     LCD configured as follow:
        - Data/Address MUX = Disable
        - Memory Type = SRAM
        - Data Width = 16bit
        - Write Operation = Enable
        - Extended Mode = Enable
        - Asynchronous Wait = Disable */
	FSMC_NORSRAMInitStructure.FSMC_Bank						= FSMC_Bank1_NORSRAM3;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux			= FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType				= FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth			= FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode			= FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait			= FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity		= FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode					= FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive			= FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation			= FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal				= FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode				= FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst				= FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct	= &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct		= &p;
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
	/* Enable FSMC NOR/SRAM Bank3 */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void DMA_Config(void)
{
	DMA_InitTypeDef  DMA_InitStructure;

	DMA_DeInit(DMA2_Stream1);
	while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE)
	{
	}
	DMA_DeInit(DMA2_Stream3);
	while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE)
	{
	}
	DMA_InitStructure.DMA_Channel				= DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr	= 0x64000000;
	DMA_InitStructure.DMA_Memory0BaseAddr		= 0x68000002;
	DMA_InitStructure.DMA_DIR					= DMA_DIR_MemoryToMemory;
	DMA_InitStructure.DMA_BufferSize			= 48000;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Enable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_Low;
	DMA_InitStructure.DMA_FIFOMode				= DMA_FIFOMode_Enable;         
	DMA_InitStructure.DMA_FIFOThreshold			= DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst			= DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst		= DMA_MemoryBurst_Single;
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
	DMA_InitStructure.DMA_Channel				= DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr	= 0x64000000 + 96000;
	DMA_InitStructure.DMA_Memory0BaseAddr		= 0x68000002;
	DMA_InitStructure.DMA_DIR					= DMA_DIR_MemoryToMemory;
	DMA_InitStructure.DMA_BufferSize			= 48000;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Enable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_Low;
	DMA_InitStructure.DMA_FIFOMode				= DMA_FIFOMode_Enable;         
	DMA_InitStructure.DMA_FIFOThreshold			= DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst			= DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst		= DMA_MemoryBurst_Single;
	DMA_Init(DMA2_Stream3, &DMA_InitStructure);

	return;
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void PSRAM_Init(void)
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  p;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOs clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF |
						   RCC_AHB1Periph_GPIOG, ENABLE);
	/* Enable FSMC clock */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

/*-- GPIOs Configuration -----------------------------------------------------*/
/*
 +-------------------+--------------------+------------------+------------------+
 | PD0  <-> FSMC_D2  | PE0  <-> FSMC_NBL0 | PF0 <-> FSMC_A0  | PG0 <-> FSMC_A10 |
 | PD1  <-> FSMC_D3  | PE1  <-> FSMC_NBL1 | PF1 <-> FSMC_A1  | PG1 <-> FSMC_A11 |
 | PD4  <-> FSMC_NOE | PE2  <-> FSMC_A23  | PF2 <-> FSMC_A2  | PG2 <-> FSMC_A12 |
 | PD5  <-> FSMC_NWE | PE3  <-> FSMC_A19  | PF3 <-> FSMC_A3  | PG3 <-> FSMC_A13 |
 | PD8  <-> FSMC_D13 | PE4  <-> FSMC_A20  | PF4 <-> FSMC_A4  | PG4 <-> FSMC_A14 |
 | PD9  <-> FSMC_D14 | PE5  <-> FSMC_A21  | PF5 <-> FSMC_A5  | PG5 <-> FSMC_A15 |
 | PD10 <-> FSMC_D15 | PE6  <-> FSMC_A22  | PF12 <-> FSMC_A6 | PG9 <-> FSMC_NE2 |
 | PD11 <-> FSMC_A16 | PE7  <-> FSMC_D4   | PF13 <-> FSMC_A7 |------------------+
 | PD12 <-> FSMC_A17 | PE8  <-> FSMC_D5   | PF14 <-> FSMC_A8 |
 | PD13 <-> FSMC_A18 | PE9  <-> FSMC_D6   | PF15 <-> FSMC_A9 |
 | PD14 <-> FSMC_D0  | PE10 <-> FSMC_D7   |------------------+
 | PD15 <-> FSMC_D1  | PE11 <-> FSMC_D8   |
 +-------------------| PE12 <-> FSMC_D9   |
                     | PE13 <-> FSMC_D10  |
                     | PE14 <-> FSMC_D11  |
                     | PE15 <-> FSMC_D12  |
                     +--------------------+
*/
	/* GPIOD configuration */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_4  | GPIO_Pin_5  |
									  GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 |
									  GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	/* GPIOE configuration */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource2, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource3, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);
	
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3 |
									  GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_6  | GPIO_Pin_7 |
									  GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11|
									  GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	/* GPIOF configuration */
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource2, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource3, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource13, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource15, GPIO_AF_FSMC);
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  |
									  GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_12 | GPIO_Pin_13 |
									  GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	/* GPIOG configuration */
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource2, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource3, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3 |
									  GPIO_Pin_4  | GPIO_Pin_5  |GPIO_Pin_9;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	/*-- FSMC Configuration ------------------------------------------------------*/
	p.FSMC_AddressSetupTime			= 2;
	p.FSMC_AddressHoldTime			= 0;
	p.FSMC_DataSetupTime			= 11;
	p.FSMC_BusTurnAroundDuration	= 1;
	p.FSMC_CLKDivision				= 0;
	p.FSMC_DataLatency				= 0;
	p.FSMC_AccessMode				= FSMC_AccessMode_A;
	FSMC_NORSRAMInitStructure.FSMC_Bank						= FSMC_Bank1_NORSRAM2;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux			= FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType				= FSMC_MemoryType_PSRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth			= FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode			= FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait			= FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity		= FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode					= FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive			= FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation			= FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal				= FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode				= FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst				= FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct	= &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct		= &p;
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
	/*!< Enable FSMC Bank1_SRAM2 Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM2, ENABLE);
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : SSD1963_Init
**功能 : SSD1963 Init
**輸入 : None
**輸出 : None
**使用 : SSD1963_Init()
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_Init( void )
{
	PSRAM_Init();
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	vram_buffer = (uint16_t *)Bank1_SRAM2_ADDR;
	memset(vram_buffer, 0x00, 0x800000);
	/* Configure the LCD Control pins --------------------------------------------*/
	LCD_CtrlLinesConfig();
	/* Configure the FSMC Parallel interface -------------------------------------*/
	LCD_FSMCConfig();
	DMA_Config();
	Delay_10ms(1); /* delay 10 ms */
	LCD->LCD_REG = 0x01; // soft reset
	LCD->LCD_REG = 0xD0;
	LCD->LCD_RAM = 0x07;
	LCD->LCD_RAM = 0x01;
	LCD->LCD_RAM = 0x08;
	LCD->LCD_REG = 0xD1;
	LCD->LCD_RAM = 0x00;
	LCD->LCD_RAM = 0x71;
	LCD->LCD_RAM = 0x19;
	LCD->LCD_REG = 0xD2; // power setting
	LCD->LCD_RAM = 0x01;
	LCD->LCD_RAM = 0x44;
	LCD->LCD_REG = 0x11; // Exit Sleep
	Delay_10ms(10);
	LCD->LCD_REG = 0x36; // set address mode
	LCD->LCD_RAM = 0x28;
	LCD->LCD_REG = 0x3A; // set pixel format
	LCD->LCD_RAM = 0x55;
	LCD->LCD_REG = 0x2A; // set column address
	LCD->LCD_RAM = 0x00;
	LCD->LCD_RAM = 0x00;
//	LCD->LCD_RAM = 0x00;
//	LCD->LCD_RAM = 0xEF;
	LCD->LCD_RAM = 0x01;
	LCD->LCD_RAM = 0x8F;
	LCD->LCD_REG = 0x2B; // set page address
	LCD->LCD_RAM = 0x00;
	LCD->LCD_RAM = 0x00;
//	LCD->LCD_RAM = 0x01;
//	LCD->LCD_RAM = 0x8F;
	LCD->LCD_RAM = 0x00;
	LCD->LCD_RAM = 0xEF;
	LCD->LCD_REG = 0x29; //display on
	
  LCD_Clear(BLACK);
}

/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_WriteData
**功能 : Write Data
**輸入 : WriteData
**輸出 : None
**使用 : 
**=====================================================================================================*/
/*=====================================================================================================*/
static void LCD_WriteData( u32 WriteData )
{
	LCD->LCD_RAM = WriteData;
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_WriteCom
**功能 : Write Command or Address
**輸入 : WriteCom
**輸出 : None
**使用 : 
**=====================================================================================================*/
/*=====================================================================================================*/
static void LCD_WriteCom( u32 WriteCom )
{
	LCD->LCD_REG = WriteCom;
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_WriteColor
**功能 : Write Color
**輸入 : Color
**輸出 : None
**使用 : 
**=====================================================================================================*/
/*=====================================================================================================*/
static void LCD_WriteColor( u32 Color )
{
	u8 Color_R, Color_G, Color_B;

	Color_R = (u8)(Color >> 16);
	Color_G = (u8)(Color >> 8);
	Color_B = (u8)Color;

	LCD_WriteData((Color_R << 8) | (Color_G));
	LCD_WriteData((Color_B << 8) | (Color_R));
	LCD_WriteData((Color_G << 8) | (Color_B));
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_Clear
**功能 : Clear Windows
**輸入 : Color
**輸出 : None
**使用 : LCD_Clear(BLACK)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_Clear( u32 Color )
{
	u32 Point = 0;

	LCD_SetWindow(0,480-1,0,272-1);

	LCD_WriteCom(Com_WriteMemoryStart);
 	LCD_WriteCom(Com_WriteMemoryStart);	

	Point = TFT_L*TFT_W;

	while(Point--)
		LCD_WriteColor(Color);
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_SetWindow
**功能 : Set Window
**輸入 : CoordiX_START, CoordiX_END, CoordiY_START, CoordiY_END
**輸出 : None
**使用 : LCD_SetWindow(0, 479, 0, 271);
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_SetWindow( u16 CoordiX_START, u16 CoordiX_END, u16 CoordiY_START, u16 CoordiY_END )
{
	TFT_REG = (Com_SetColumnAddress);
	TFT_RAM = (CoordiX_START>>8);
	TFT_RAM = (CoordiX_START&0x00ff);
	TFT_RAM = (CoordiX_END>>8);
	TFT_RAM = (CoordiX_END&0x00ff);
	TFT_REG = (Com_SetPageAddress);
	TFT_RAM = (CoordiY_START>>8);
	TFT_RAM = (CoordiY_START);
	TFT_RAM = (CoordiY_END>>8);
	TFT_RAM = (CoordiY_END&0x00ff);
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_DrawPoint
**功能 : Draw Point
**輸入 : CoordiX, CoordiY, Color
**輸出 : None
**使用 : LCD_DrawPoint(CoordiX, CoordiY, Color)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_DrawPoint( u16 CoordiX, u16 CoordiY, u32 Color )
{
	LCD_WriteCom(Com_SetColumnAddress);
	LCD_WriteData(CoordiX>>8);					// 起始地址X
	LCD_WriteData(CoordiX);
	LCD_WriteData(CoordiX>>8);					// 結束地址X
	LCD_WriteData(CoordiX);
  LCD_WriteCom(Com_SetPageAddress);
	LCD_WriteData(CoordiY>>8);					// 起始地址Y
	LCD_WriteData(CoordiY);
	LCD_WriteData(CoordiY>>8);					// 結束地址Y
	LCD_WriteData(CoordiY);
	LCD_WriteCom(Com_WriteMemoryStart);

	LCD_WriteColor(Color);
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_DrawLine
**功能 : Draw Line
**輸入 : StartX, StartY, EndX, EndY, Color
**輸出 : None
**使用 : LCD_DrawLine(StartX, StartY, EndX, EndY, Color)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_DrawLine( u16 StartX, u16 StartY, u16 EndX, u16 EndY, u32 Color )
{
	u16 i;
	s16 DeltaX, DeltaY;
	double Slope;

	DeltaX = EndX - StartX;
	DeltaY = EndY - StartY;

	// 計算 Slope
	if(DeltaX == 0)
		Slope = 0;
	else
		Slope = (double)DeltaY/(double)DeltaX;

	DeltaX = fabs(DeltaX);
	DeltaY = fabs(DeltaY);

	// 畫線
	if(EndX<StartX) {
		if(Slope<0) {
			if(DeltaX>DeltaY) {
				for(i=0; i<=DeltaX; i++)
					LCD_DrawPoint(EndX+i, EndY+(s16)(((double)i*Slope)-0.5), Color);
			}
			else {
				for(i=0; i<=DeltaY; i++)
					LCD_DrawPoint(EndX-(s16)(((double)i/Slope)-0.5), EndY-i, Color);
			}
		}
		else {
			if(DeltaX>DeltaY) {
				for(i=0; i<=DeltaX; i++)
					LCD_DrawPoint(EndX+i, EndY+(s16)(((double)i*Slope)+0.5), Color);
			}
			else {
				for(i=0; i<=DeltaY; i++)
					LCD_DrawPoint(EndX+(s16)(((double)i/Slope)+0.5), EndY+i, Color);
			}
		}
	}
	else if(EndX==StartX) {
		if(EndY>StartY) {
			for(i=0; i<=DeltaY; i++)
				LCD_DrawPoint(StartX, StartY+i, Color);
		}
		else {
			for(i=0; i<=DeltaY; i++)
				LCD_DrawPoint(EndX, EndY+i, Color);
		}
	}
	else {
		if(Slope<0) {
			if(DeltaX>DeltaY) {
				for(i=0; i<=DeltaX; i++)
					LCD_DrawPoint(StartX+i, StartY+(s16)(((double)i*Slope)-0.5), Color);
			}
			else {
				for(i=0; i<=DeltaY; i++)
					LCD_DrawPoint(StartX-(s16)(((double)i/Slope)-0.5), StartY-i, Color);
			}
		}
		else {
			if(DeltaX>DeltaY) {
				for(i=0; i<=DeltaX; i++)
					LCD_DrawPoint(StartX+i, StartY+(s16)(((double)i*Slope)+0.5), Color);
			}
			else {
				for(i=0; i<=DeltaY; i++)
					LCD_DrawPoint(StartX+(s16)(((double)i/Slope)+0.5), StartY+i, Color);
			}
		}
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_DrawLineX
**功能 : Draw X-Axis Line
**輸入 : CoordiX, CoordiY, Length, Color
**輸出 : None
**使用 : LCD_DrawLine(CoordiX, CoordiY, Length, Color)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_DrawLineX( u16 CoordiX, u16 CoordiY, u16 Length, u32 Color )
{
	u16 i;

	for(i=0; i<Length; i++)
		LCD_DrawPoint(CoordiX+i, CoordiY, Color);
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_DrawLineY
**功能 : Draw Y-Axis Line
**輸入 : CoordiX, CoordiY, Length, Color
**輸出 : None
**使用 : LCD_DrawLine(CoordiX, CoordiY, Length, Color)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_DrawLineY( u16 CoordiX, u16 CoordiY, u16 Length, u32 Color )
{
	u16 i;

	for(i=0; i<Length; i++)
		LCD_DrawPoint(CoordiX, CoordiY+i, Color);
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_DrawRectangle
**功能 : Draw Rectangle
**輸入 : CoordiX, CoordiY, Length, Width, Color
**輸出 : None
**使用 : LCD_DrawRectangle(CoordiX, CoordiY, Length, Width, Color)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_DrawRectangle( u16 CoordiX, u16 CoordiY, u16 Length, u16 Width, u32 Color )
{
	u16 i;

	for(i=0; i<=Length; i++) {
		LCD_DrawPoint(CoordiX+i, CoordiY, Color);
		LCD_DrawPoint(CoordiX+i, CoordiY+Width, Color);
	}
	for(i=1; i<Width; i++) {
		LCD_DrawPoint(CoordiX, CoordiY+i, Color);
		LCD_DrawPoint(CoordiX+Length, CoordiY+i, Color);
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_DrawRectangleFull
**功能 : Draw Rectangle
**輸入 : CoordiX, CoordiY, Length, Width, Color
**輸出 : None
**使用 : LCD_DrawRectangleFull(CoordiX, CoordiY, Length, Width, Color)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_DrawRectangleFull( u16 CoordiX, u16 CoordiY, u16 Length, u16 Width, u32 Color )
{
	u16 i, j;

	for(i=0; i<Width; i++)
		for(j=0; j<Length; j++)
			LCD_DrawPoint(CoordiX+j, CoordiY+i, Color);
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : TFT_DrawCircle
**功能 : Draw Circle
**輸入 : CoordiX, CoordiY, Radius, Color
**輸出 : None
**使用 : TFT_DrawCircle(CoordiX, CoordiY, Radius, Color)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_DrawCircle( u16 CoordiX, u16 CoordiY, u16 Radius, u32 Color )
{
	int a,b;
	int di;

	a=0;
	b=Radius;
	di=3-(Radius<<1);

	while(a<=b) {
		LCD_DrawPoint(CoordiX-b, CoordiY-a, Color);             //3
		LCD_DrawPoint(CoordiX+b, CoordiY-a, Color);             //0
		LCD_DrawPoint(CoordiX-a, CoordiY+b, Color);             //1
		LCD_DrawPoint(CoordiX-b, CoordiY-a, Color);             //7
		LCD_DrawPoint(CoordiX-a, CoordiY-b, Color);             //2
		LCD_DrawPoint(CoordiX+b, CoordiY+a, Color);             //4
		LCD_DrawPoint(CoordiX+a, CoordiY-b, Color);             //5
		LCD_DrawPoint(CoordiX+a, CoordiY+b, Color);             //6
		LCD_DrawPoint(CoordiX-b, CoordiY+a, Color);
		a++;
		// 使用Bresenham算法畫圓
		if(di<0)
			di += 4*a+6;
		else {
			di+=10+4*(a-b);
			b--;
		}
		LCD_DrawPoint(CoordiX+a, CoordiY+b, Color);
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_PutChar1206
**功能 : Put Char
**輸入 : CoordiX, CoordiY, ChWord, FontColor, BackColor
**輸出 : None
**使用 : LCD_PutChar1206(x, y, "C", WHITE, BLACK)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_PutChar1206( u16 CoordiX, u16 CoordiY, u8* ChWord, u32 FontColor, u32 BackColor )
{
	u8 Tmp_Char = 0;
	u16 i = 0, j = 0;

	for(i=0; i<12; i++) {
		Tmp_Char = ASCII_12x6[*ChWord-32][i];
		for(j=0; j<6; j++) {
			if(((Tmp_Char>>j)&0x01) == 0x01)
				LCD_DrawPoint(CoordiX+j, CoordiY+i, FontColor);	// 字符顏色
			else
				LCD_DrawPoint(CoordiX+j, CoordiY+i, BackColor);	// 背景顏色
		}
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_PutChar1608
**功能 : Put Char
**輸入 : CoordiX, CoordiY, ChWord, FontColor, BackColor
**輸出 : None
**使用 : LCD_PutChar1608(x, y, "C", WHITE, BLACK)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_PutChar1608( u16 CoordiX, u16 CoordiY, u8* ChWord, u32 FontColor, u32 BackColor )
{
	u8 Tmp_Char = 0;
	u16 i = 0, j = 0;

	for(i=0; i<16; i++) {
		Tmp_Char = ASCII_16x8[*ChWord-32][i];
		for(j=0; j<8; j++) {
			if(((Tmp_Char>>(7-j))&0x01) == 0x01)
				LCD_DrawPoint(CoordiX+j, CoordiY+i, FontColor);	// 字符顏色
			else
				LCD_DrawPoint(CoordiX+j, CoordiY+i, BackColor);	// 背景顏色
		}
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_PutChar1608_
**功能 : Put Char
**輸入 : CoordiX, CoordiY, ChWord, FontColor, BackColor
**輸出 : None
**使用 : LCD_PutChar1608(x, y, "C", WHITE, BLACK)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_PutChar1608_( u16 CoordiX, u16 CoordiY, u8* ChWord, u32 FontColor, u32 BackColor )
{
	u8 Tmp_Char = 0;
	u16 i = 0, j = 0;

	for(i=0; i<16; i++) {
		Tmp_Char = ASCII_16x8_[*ChWord-32][i];
		for(j=0; j<8; j++) {
			if(((Tmp_Char>>j)&0x01) == 0x01)
				LCD_DrawPoint(CoordiX+j, CoordiY+i, FontColor);	// 字符顏色
			else
				LCD_DrawPoint(CoordiX+j, CoordiY+i, BackColor);	// 背景顏色
		}
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : LCD_PutStr
**功能 : Put String
**輸入 : CoordiX, CoordiY, ChWord, FontStyle, FontColor, BackColor
**輸出 : None
**使用 : LCD_PutStr(x, y, "PUT CHAR", ASCII1206, WHITE, BLACK)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_PutStr( u16 CoordiX, u16 CoordiY, u8 *ChWord, u8 FontStyle, u32 FontColor, u32 BackColor )
{
	u16 OffsetX = 0;

	switch(FontStyle) {
		case ASCII1206:
			while(*ChWord) {
				LCD_PutChar1206(CoordiX+OffsetX, CoordiY, ChWord, FontColor, BackColor);
				ChWord++;
				OffsetX += 6;
			}
			break;
		case ASCII1608:
			while(*ChWord) {
				LCD_PutChar1608(CoordiX+OffsetX, CoordiY, ChWord, FontColor, BackColor);
				ChWord++;
				OffsetX += 8;
			}
			break;
		case ASCII1608_:
			while(*ChWord) {
				LCD_PutChar1608_(CoordiX+OffsetX, CoordiY, ChWord, FontColor, BackColor);
				ChWord++;
				OffsetX += 8;
			}
			break;
	}
}
/*=====================================================================================================*/
/*=====================================================================================================*
**函數 : TFT_PutNum
**功能 : Put Number
**輸入 : oordiX, CoordiY, Type, Length, NumData, FontColor, BackColor
**輸出 : None
**使用 : TFT_PutNum(CoordiX, CoordiY, Type_D, Length, NumData, WHITE, BLACK)
**=====================================================================================================*/
/*=====================================================================================================*/
void LCD_PutNum( u16 CoordiX, u16 CoordiY, u8 Type, u8 Length, u32 NumData, u32 FontColor, u32 BackColor )
{
	u8 TmpNumber[16] = {0};

	NumToChar(Type, Length, TmpNumber, NumData);
	LCD_PutStr(CoordiX, CoordiY, TmpNumber, ASCII1608, FontColor, BackColor);
}
/*=====================================================================================================*/
/*=====================================================================================================*/

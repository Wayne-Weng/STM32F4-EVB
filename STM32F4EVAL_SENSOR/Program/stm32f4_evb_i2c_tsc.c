
#include "stm32f4_evb_i2c_tsc.h"

#define TIMEOUT_MAX    0x3000 /*<! The value of the maximal timeout for I2C waiting loops */

TSC_STATE TSC_State;              /*<! The global structure holding the TS state */

uint32_t TSC_TimeOut = TIMEOUT_MAX; /*<! Value of Timeout when I2C communication fails */

static void TSC_DMA_Config(TSC_DMADirection_TypeDef Direction, uint8_t* buffer);

#define DeviceAddress 0x90	// A1=A0=0
#define I2C_WRITE	DeviceAddress
#define	I2C_READ	DeviceAddress + 1

void TSC_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB1PeriphClockCmd(TSC_I2C_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(TSC_I2C_SCL_GPIO_CLK | TSC_I2C_SDA_GPIO_CLK, ENABLE);

	GPIO_PinAFConfig(TSC_I2C_SCL_GPIO_PORT, TSC_I2C_SCL_SOURCE, TSC_I2C_SCL_AF);
	GPIO_PinAFConfig(TSC_I2C_SCL_GPIO_PORT, TSC_I2C_SDA_SOURCE, TSC_I2C_SCL_AF);

	GPIO_InitStructure.GPIO_Pin		= TSC_I2C_SCL_PIN | TSC_I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_Init(TSC_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);
	
	if ((TSC_I2C->CR1 & I2C_CR1_PE) != I2C_CR1_PE)
	{
		I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;
		I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;
		I2C_InitStructure.I2C_OwnAddress1         = 0xA0;
		I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;
		I2C_InitStructure.I2C_AcknowledgedAddress	= I2C_AcknowledgedAddress_7bit;
		I2C_InitStructure.I2C_ClockSpeed          = I2C_SPEED;
		I2C_Init(TSC_I2C, &I2C_InitStructure);
	}
}


uint8_t HWI2CWrite(uint8_t Command)
{
	/* Enable the I2C peripheral */
	I2C_GenerateSTART(TSC_I2C, ENABLE);

	/* Test on SB Flag */
	TSC_TimeOut = TIMEOUT_MAX;
	while (I2C_GetFlagStatus(TSC_I2C,I2C_FLAG_SB) == RESET)
	{
		if (TSC_TimeOut-- == 0) return(TSC_TimeoutUserCallback());
	}

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(TSC_I2C, I2C_WRITE, I2C_Direction_Transmitter);

	/* Test on ADDR Flag */
	TSC_TimeOut = TIMEOUT_MAX;
	while (!I2C_CheckEvent(TSC_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (TSC_TimeOut-- == 0) return(TSC_TimeoutUserCallback());
	}

	/* Transmit the first address for r/w operations */
	I2C_SendData(TSC_I2C, Command);

	/* Test on TXE FLag (data dent) */
	TSC_TimeOut = TIMEOUT_MAX;
	while ((!I2C_GetFlagStatus(TSC_I2C,I2C_FLAG_TXE)) && (!I2C_GetFlagStatus(TSC_I2C,I2C_FLAG_BTF)))
	{
		if (TSC_TimeOut-- == 0) return(TSC_TimeoutUserCallback());
	}

	/* Send STOP Condition */
	I2C_GenerateSTOP(TSC_I2C, ENABLE);

	return (1);
}

uint8_t HWI2CRead(uint8_t *bytesBuf)
{
	uint8_t TSC_BufferRX[2] = {0x00, 0x00};

	/* Configure DMA Peripheral */
	TSC_DMA_Config(TSC_DMA_RX, (uint8_t*)TSC_BufferRX);

	/* Enable DMA NACK automatic generation */
	I2C_DMALastTransferCmd(TSC_I2C, ENABLE);

	/* Enable the I2C peripheral */
	I2C_GenerateSTART(TSC_I2C, ENABLE);

	/* Test on SB Flag */
	TSC_TimeOut = TIMEOUT_MAX;
	while (!I2C_GetFlagStatus(TSC_I2C,I2C_FLAG_SB))
	{
		if (TSC_TimeOut-- == 0) return(TSC_TimeoutUserCallback());
	}

	/* Send TSCxpander address for read */
	I2C_Send7bitAddress(TSC_I2C, I2C_READ, I2C_Direction_Receiver);

	/* Test on ADDR Flag */
	TSC_TimeOut = TIMEOUT_MAX;
	while (!I2C_CheckEvent(TSC_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
		if (TSC_TimeOut-- == 0) return(TSC_TimeoutUserCallback());
	}

	/* Enable I2C DMA request */
	I2C_DMACmd(TSC_I2C,ENABLE);

	/* Enable DMA RX Channel */
	DMA_Cmd(TSC_DMA_RX_STREAM, ENABLE);

	/* Wait until DMA Transfer Complete */
	TSC_TimeOut = 2 * TIMEOUT_MAX;
	while (!DMA_GetFlagStatus(TSC_DMA_RX_STREAM,TSC_DMA_RX_TCFLAG))
	{
		if (TSC_TimeOut-- == 0) return(TSC_TimeoutUserCallback());
	}

	/* Send STOP Condition */
	I2C_GenerateSTOP(TSC_I2C, ENABLE);

	/* Disable DMA RX Channel */
	DMA_Cmd(TSC_DMA_RX_STREAM, DISABLE);

	/* Disable I2C DMA request */
	I2C_DMACmd(TSC_I2C,DISABLE);

	/* Clear DMA RX Transfer Complete Flag */
	DMA_ClearFlag(TSC_DMA_RX_STREAM,TSC_DMA_RX_TCFLAG);

	/* return a pointer to the TSC_Buffer */
	bytesBuf[0] = (uint8_t)TSC_BufferRX[0];
	bytesBuf[0] = (uint8_t)TSC_BufferRX[0];

	return(1);
}

uint16_t TSC2003Read(uint8_t source)
{
	uint8_t pCommand, pWords[] = {0, 0};

	// command byte= [C3 C2 C1 C0 0 1 0 0]
	pCommand = ((source << 4) | 0x04);
	HWI2CWrite(pCommand);
	HWI2CRead(pWords);

	return ((uint16_t) (pWords[0] << 2) | (uint16_t) (pWords[1] >> 6));
}

TSC_STATE* TSC_TS_GetState(void)
{
	uint16_t xDiff, yDiff , x , y;
	static uint16_t _x = 0, _y = 0;

  // Read X Axis ([C3 C2 C1 C0]=[1100]
  x = TSC2003Read(0xC);
  // Read Y Axis ([C3 C2 C1 C0]=[1101]
  y = TSC2003Read(0xD);

  xDiff = x > _x? (x - _x): (_x - x);
  yDiff = y > _y? (y - _y): (_y - y);
  if (xDiff + yDiff > 5) {
    _x = x;
    _y = y;
  }

	TSC_State.X = _x;
	TSC_State.Y = _y;
	TSC_State.Z = TSC2003Read(0xE);

	return &TSC_State;
}

static void TSC_DMA_Config(TSC_DMADirection_TypeDef Direction, uint8_t* buffer)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(TSC_DMA_CLK, ENABLE);
	DMA_InitStructure.DMA_Channel				= TSC_DMA_CHANNEL;
	DMA_InitStructure.DMA_PeripheralBaseAddr	= TSC_I2C_DR;
	DMA_InitStructure.DMA_Memory0BaseAddr		= (uint32_t)buffer;
	DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode					= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority				= DMA_Priority_Low;
	DMA_InitStructure.DMA_FIFOMode				= DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold			= DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst			= DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst		= DMA_PeripheralBurst_Single;
	if (Direction == TSC_DMA_RX)
	{
		DMA_InitStructure.DMA_DIR				= DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_BufferSize		= 2;
		DMA_DeInit(TSC_DMA_RX_STREAM);
		DMA_Init(TSC_DMA_RX_STREAM, &DMA_InitStructure);
	}
	else if (Direction == TSC_DMA_TX)
	{
		DMA_InitStructure.DMA_DIR				= DMA_DIR_MemoryToPeripheral;
		DMA_InitStructure.DMA_BufferSize		= 1;
		DMA_DeInit(TSC_DMA_TX_STREAM);
		DMA_Init(TSC_DMA_TX_STREAM, &DMA_InitStructure);
	}
}

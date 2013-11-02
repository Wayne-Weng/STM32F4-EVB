
#ifndef __STM32F4_EVB_TSC_H
#define __STM32F4_EVB_TSC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

typedef struct
{
	uint16_t TouchDetected;
	uint16_t X;
	uint16_t Y;
	uint16_t Z;
} TSC_STATE;

typedef enum
{
	TSC_OK = 0,
	TSC_FAILURE,
	TSC_TIMEOUT,
	PARAM_ERROR,
	TSC1_NOT_OPERATIONAL,
	TSC2_NOT_OPERATIONAL
} TSC_Status_TypDef;

typedef enum
{
	TSC_DMA_TX = 0,
	TSC_DMA_RX = 1
} TSC_DMADirection_TypeDef;

#ifdef USE_Delay
#include "main.h"

  #define _delay_     Delay  /* !< User can provide more timing precise _delay_ function
                                   (with 10ms time base), using SysTick for example */
#else
  #define _delay_     delay      /* !< Default _delay_ function with less precise timing */
#endif

/*------------------------------------------------------------------------------
    Hardware Configuration
------------------------------------------------------------------------------*/
/**
  * @brief  I2C port definitions
  */
#define TSC_I2C							I2C1
#define TSC_I2C_CLK						RCC_APB1Periph_I2C1
#define TSC_I2C_SCL_PIN					GPIO_Pin_6
#define TSC_I2C_SCL_GPIO_PORT			GPIOB
#define TSC_I2C_SCL_GPIO_CLK			RCC_AHB1Periph_GPIOB
#define TSC_I2C_SCL_SOURCE				GPIO_PinSource6
#define TSC_I2C_SCL_AF					GPIO_AF_I2C1
#define TSC_I2C_SDA_PIN					GPIO_Pin_9
#define TSC_I2C_SDA_GPIO_PORT			GPIOB
#define TSC_I2C_SDA_GPIO_CLK			RCC_AHB1Periph_GPIOB
#define TSC_I2C_SDA_SOURCE				GPIO_PinSource9
#define TSC_I2C_SDA_AF					GPIO_AF_I2C1
#define TSC_I2C_DR						((uint32_t)0x40005410)

/* I2C clock speed configuration (in Hz)
  WARNING:
   Make sure that this define is not already declared in other files (ie.
  STM32F4_EVB.h file). It can be used in parallel by other modules. */
#ifndef I2C_SPEED
 #define I2C_SPEED						100000
#endif /* I2C_SPEED */

/**
  * @brief  TSC DMA definitions
  */
#define TSC_DMA_CLK						RCC_AHB1Periph_DMA1
#define TSC_DMA_CHANNEL					DMA_Channel_1
#define TSC_DMA_TX_STREAM				DMA1_Stream6
#define TSC_DMA_RX_STREAM				DMA1_Stream0
#define TSC_DMA_TX_TCFLAG				DMA_FLAG_TCIF6
#define TSC_DMA_RX_TCFLAG				DMA_FLAG_TCIF0

/**
  * @brief  IO Expander Interrupt line on EXTI
  */
#define TSC_IT_PIN						GPIO_Pin_2
#define TSC_IT_GPIO_PORT				GPIOB
#define TSC_IT_GPIO_CLK					RCC_AHB1Periph_GPIOB
#define TSC_IT_EXTI_PORT_SOURCE			EXTI_PortSourceGPIOB
#define TSC_IT_EXTI_PIN_SOURCE			EXTI_PinSource2
#define TSC_IT_EXTI_LINE				EXTI_Line2
#define TSC_IT_EXTI_IRQn				EXTI2_IRQn

/**
  * @brief  Interrupt source configuration definitons
  */
#define TSC_ITSRC_TSC					0x01  /* IO_Exapnder 1 */

/** @defgroup STM32F4_EVB_TSC_Exported_Functions
  * @{
  */

/**
  * @brief  Configuration and initialization functions
  */
void TSC_Config(void);
uint8_t TSC_ITConfig(uint32_t TSC_ITSRC_Source);

#ifdef USE_TIMEOUT_USER_CALLBACK
 uint8_t TSC_TimeoutUserCallback(void);
#else
 #define TSC_TimeoutUserCallback()  TSC_TIMEOUT
#endif /* USE_TIMEOUT_USER_CALLBACK */

TSC_STATE* TSC_TS_GetState(void);

uint8_t TSC_TS_Config(void);
uint8_t HWI2CWrite(uint8_t Command);
uint8_t HWI2CRead(uint8_t *bytesBuf);


#ifdef __cplusplus
}
#endif
#endif /* __STM32F4_EVB_TSC_H */

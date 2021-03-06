/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "stm32f4_i2c.h"
#include "stm32f4_sdio.h"
#include "stm32f4_evb_i2c_tsc.h"
#include "module_mpu6050.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
s16 ReadIMU[8] = {0};
u16 TouchBuf[3] = {0};
/*=====================================================================================================*/
/*=====================================================================================================*/
void SysTick_Handler( void )
{
  MPU6050_Read(ReadIMU);
  TSC_ReadData(TouchBuf);
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void DMA1_Stream0_IRQHandler( void )
{
	I2C_RX_DMA_IRQ();
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void DMA1_Stream6_IRQHandler( void )
{
	I2C_TX_DMA_IRQ();
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void SDIO_IRQHandler( void )
{
  SD_ProcessIRQSrc();
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void DMA2_Stream3_IRQHandler( void )
{
  SD_ProcessDMAIRQ();
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void HardFault_Handler( void )
{
  while(1);
}
void MemManage_Handler( void )
{
  while(1);
}
void BusFault_Handler( void )
{
  while(1);
}
void UsageFault_Handler( void )
{
  while(1);
}
void SVC_Handler( void )
{
  while(1);
}
void DebugMon_Handler( void )
{
  while(1);
}
void PendSV_Handler( void )
{
  while(1);
}
void NMI_Handler( void )
{
  while(1);
}
/*=====================================================================================================*/
/*=====================================================================================================*/

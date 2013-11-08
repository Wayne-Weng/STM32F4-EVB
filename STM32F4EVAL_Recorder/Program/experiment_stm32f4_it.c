/*=====================================================================================================*/
/*=====================================================================================================*/
#include "stm32f4_system.h"
#include "stm32f4_i2c.h"
#include "stm32f4_sdio.h"
#include "stm32f4_evb_i2c_tsc.h"
#include "QCopter_ahrs.h"
#include "module_mpu6050.h"
#include "algorithm_moveAve.h"
#include "algorithm_mathUnit.h"
#include "interface.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
extern u16 SD_Buf[SD_BUF_SIZE];
extern vu8 SD_Flag;

s16 WaveFormData[4] = {0};
s16 ReadIMU[8] = {0};
u16 TouchBuf[3] = {0};

Sensor_Mode SensorMode = Mode_GyrCorrect;
/*=====================================================================================================*/
/*=====================================================================================================*/
void SysTick_Handler( void )
{
  static u32 Count = 0;
  static u32 Correction_Time = 0;

  static s16 ACC_FIFO[3][256] = {0};
  static s16 GYR_FIFO[3][256] = {0};

  MPU6050_Read(ReadIMU);
  TSC_ReadData(TouchBuf);

  Acc.X  = (s16)(ReadIMU[0]);
  Acc.Y  = (s16)(ReadIMU[1]);
  Acc.Z  = (s16)(ReadIMU[2]);
  Gyr.X  = (s16)(ReadIMU[4]);
  Gyr.Y  = (s16)(ReadIMU[5]);
  Gyr.Z  = (s16)(ReadIMU[6]);

  /* Offset */
  Acc.X -= Acc.OffsetX;
  Acc.Y -= Acc.OffsetY;
  Acc.Z -= Acc.OffsetZ;
  Gyr.X -= Gyr.OffsetX;
  Gyr.Y -= Gyr.OffsetY;
  Gyr.Z -= Gyr.OffsetZ;

  #define MovegAveFIFO_Size 250
  switch(SensorMode) {

  /************************** Mode_CorrectGyr **************************************/
    case Mode_GyrCorrect:
      /* Simple Moving Average */
      Gyr.X = (s16)MoveAve_SMA(Gyr.X, GYR_FIFO[0], MovegAveFIFO_Size);
      Gyr.Y = (s16)MoveAve_SMA(Gyr.Y, GYR_FIFO[1], MovegAveFIFO_Size);
      Gyr.Z = (s16)MoveAve_SMA(Gyr.Z, GYR_FIFO[2], MovegAveFIFO_Size);

      Correction_Time++;  // 等待 FIFO 填滿空值 or 填滿靜態資料
      if(Correction_Time == 400) {
        Gyr.OffsetX += (Gyr.X - GYR_X_OFFSET);  // 角速度為 0dps
        Gyr.OffsetY += (Gyr.Y - GYR_Y_OFFSET);  // 角速度為 0dps
        Gyr.OffsetZ += (Gyr.Z - GYR_Z_OFFSET);  // 角速度為 0dps

        Correction_Time = 0;
        SensorMode = Mode_AccCorrect;
      }
      break;
 
  /************************** Mode_CorrectAcc **************************************/
    case Mode_AccCorrect:
      Acc.X = (s16)MoveAve_SMA(Acc.X, ACC_FIFO[0], MovegAveFIFO_Size);
      Acc.Y = (s16)MoveAve_SMA(Acc.Y, ACC_FIFO[1], MovegAveFIFO_Size);
      Acc.Z = (s16)MoveAve_SMA(Acc.Z, ACC_FIFO[2], MovegAveFIFO_Size);

      Correction_Time++;  // 等待 FIFO 填滿空值 or 填滿靜態資料
      if(Correction_Time == 400) {
        Acc.OffsetX += (Acc.X - ACC_X_OFFSET);  // 重力加速度為 0g
        Acc.OffsetY += (Acc.Y - ACC_Y_OFFSET);  // 重力加速度為 0g
        Acc.OffsetZ += (Acc.Z - ACC_Z_OFFSET);  // 重力加速度為 1g

        Correction_Time = 0;
        SensorMode = Mode_Quaternion;   // Mode_CorrectMag
      }
      break;

    case Mode_MagCorrect:
      break;

  /************************** Algorithm Mode **************************************/
    case Mode_Quaternion:
      /* To Physical */
      Acc.TrueX = Acc.X*MPU6050A_4g;      // g/LSB
      Acc.TrueY = Acc.Y*MPU6050A_4g;      // g/LSB
      Acc.TrueZ = Acc.Z*MPU6050A_4g;      // g/LSB
      Gyr.TrueX = Gyr.X*MPU6050G_2000dps; // dps/LSB
      Gyr.TrueY = Gyr.Y*MPU6050G_2000dps; // dps/LSB
      Gyr.TrueZ = Gyr.Z*MPU6050G_2000dps; // dps/LSB

      AngE.Pitch = toDeg(atan2f(Acc.TrueY, Acc.TrueZ));
      AngE.Roll  = toDeg(-asinf(Acc.TrueX));
      AngE.Yaw   = 0;

      Quaternion_ToNumQ(&NumQ, &AngE);

      SensorMode = Mode_Algorithm;
      break;

  /************************** Algorithm Mode ****************************************/
    case Mode_Algorithm:

      /* 加權移動平均法 Weighted Moving Average */
      Acc.X = (s16)MoveAve_WMA(Acc.X, ACC_FIFO[0], 8);
      Acc.Y = (s16)MoveAve_WMA(Acc.Y, ACC_FIFO[1], 8);
      Acc.Z = (s16)MoveAve_WMA(Acc.Z, ACC_FIFO[2], 8);
      Gyr.X = (s16)MoveAve_WMA(Gyr.X, GYR_FIFO[0], 8);
      Gyr.Y = (s16)MoveAve_WMA(Gyr.Y, GYR_FIFO[1], 8);
      Gyr.Z = (s16)MoveAve_WMA(Gyr.Z, GYR_FIFO[2], 8);

      /* To Physical */
      Acc.TrueX = Acc.X*MPU6050A_4g;      // g/LSB
      Acc.TrueY = Acc.Y*MPU6050A_4g;      // g/LSB
      Acc.TrueZ = Acc.Z*MPU6050A_4g;      // g/LSB
      Gyr.TrueX = Gyr.X*MPU6050G_2000dps; // dps/LSB
      Gyr.TrueY = Gyr.Y*MPU6050G_2000dps; // dps/LSB
      Gyr.TrueZ = Gyr.Z*MPU6050G_2000dps; // dps/LSB

      /* Get Attitude Angle */
      AHRS_Update();

      switch(WaveName) {
        case WaveName_Acc:  // mg/LSB
          WaveFormData[0] = (s16)(Acc.TrueX*1000.0f);
          WaveFormData[1] = (s16)(Acc.TrueY*1000.0f);
          WaveFormData[2] = (s16)(Acc.TrueZ*1000.0f);
          break;
        case WaveName_Gyr:  // 0.1dps/LSB
          WaveFormData[0] = (s16)(Gyr.TrueX*10.0f);
          WaveFormData[1] = (s16)(Gyr.TrueY*10.0f);
          WaveFormData[2] = (s16)(Gyr.TrueZ*10.0f);
          break;
        case WaveName_Ang:  // 0.01deg/LSB
          WaveFormData[0] = (s16)(AngE.Pitch*100.0f);
          WaveFormData[1] = (s16)(AngE.Roll*100.0f);
          WaveFormData[2] = (s16)(AngE.Yaw*100.0f);
          break;
      }

      if(DelRecorderFile == 0) {
        Correction_Time++;
        if(Correction_Time == RecorderFreq) {
          Correction_Time = 0;
          SD_Buf[Count] = WaveFormData[1];
          Count++;
          if(Count == SD_BUF_SIZE) {
            Count = 0;
            SD_Flag = 1;
          }
        }
      }
      else {
        Count = 0;
        Correction_Time = 0;
        SD_Flag = 0;
      }
      break;
  }
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

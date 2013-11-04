
#include "stm32f4_evb_i2c_tsc.h"
#include "stm32f4_i2c.h"

#define TSC_I2C_ADDR 0x90	// A1=A0=0

static uint16_t TSC2003Read(uint8_t source)
{
	uint8_t pCommand, pWords[] = {0, 0};

	// command byte= [C3 C2 C1 C0 0 1 0 0]
	pCommand = ((source << 4) | 0x04);
  I2C_WriteBytes(TSC_I2C_ADDR, &pCommand, 1);
  I2C_ReadBytes(TSC_I2C_ADDR, pWords, 2);

	return ((uint16_t) (pWords[0] << 2) | (uint16_t) (pWords[1] >> 6));
}

void TSC_ReadData( u16 *ReadData )
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

	ReadData[0] = _x;
	ReadData[1] = _y;
	ReadData[2] = TSC2003Read(0xE);
}

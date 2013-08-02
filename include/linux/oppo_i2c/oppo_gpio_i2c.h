#ifndef _OPPO_GPIO_I2C_H
#define _OPPO_GPIO_I2C_H

typedef unsigned char   BYTE;
typedef unsigned short  UINT16;
typedef unsigned char   UINT8;
typedef UINT8  BOOL;


extern BOOL gpio_i2c0_readData(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount,BYTE *prData);
extern BOOL gpio_i2c0_writeData(BYTE bDevice, UINT16 u2Data_Addr,BYTE bDataCount, BYTE *prData);

extern BOOL gpio_i2c1_readData(BYTE bDevice, UINT16 u2Data_Addr, BYTE bDataCount,BYTE *prData);
extern BOOL gpio_i2c1_writeData(BYTE bDevice, UINT16 u2Data_Addr,BYTE bDataCount, BYTE *prData);

#endif

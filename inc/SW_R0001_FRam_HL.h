#ifndef _SW_R0001_FRAM_HL_
#define _SW_R0001_FRAM_HL_

#include "SW_R0001_Config.h"

extern const PORT_INF FRAM_SDA_PORT;
extern const PORT_INF FRAM_SCL_PORT;

#define FRAM_I2C_SCL_H            FRAM_SCL_PORT.Port->BSRR = FRAM_SCL_PORT.Pin
#define FRAM_I2C_SCL_L            FRAM_SCL_PORT.Port->BRR  = FRAM_SCL_PORT.Pin

#define FRAM_I2C_SDA_H            FRAM_SDA_PORT.Port->BSRR = FRAM_SDA_PORT.Pin
#define FRAM_I2C_SDA_L            FRAM_SDA_PORT.Port->BRR  = FRAM_SDA_PORT.Pin

#define FRam_I2CGetScl()          FRAM_SCL_PORT.Port->IDR  & FRAM_SCL_PORT.Pin
#define FRam_I2CGetSda()          FRAM_SDA_PORT.Port->IDR  & FRAM_SDA_PORT.Pin

void FRam_InitInterface(void);

void FRam_I2CDelay(void);

BOOL FRam_I2CStart(void);
void FRam_I2CStop(void);

void FRam_I2CAck(void);
void FRam_I2CNoAck(void);

BOOL FRam_I2CWaitAck(void);
u8 FRam_I2CReceiveByte(void);
void FRam_I2CSendByte(u8 SendByte);
void FRam_Delayms(u32 n);





#endif


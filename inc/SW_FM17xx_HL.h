#ifndef _SW_FM17XX_HL_
#define _SW_FM17XX_HL_

#include "SW_R0001_Config.h"

extern const PORT_INF FM17XX_RST_COM;
extern const PORT_INF FM17XX_CS_COM;
#define FM17xx_CS_Low()      FM17XX_CS_COM.Port->BRR |= (FM17XX_CS_COM.Pin)
#define FM17xx_CS_High()     FM17XX_CS_COM.Port->BSRR |= (FM17XX_CS_COM.Pin)
#define FM17xx_PowerLow()    FM17XX_RST_COM.Port->BRR |= (FM17XX_RST_COM.Pin)
#define FM17xx_PowerHigh()   FM17XX_RST_COM.Port->BSRR |= (FM17XX_RST_COM.Pin)

#define FM17XX_PORT             SPI1

void FM17xx_InitInterface(void);
u8 FM17xx_WriteByte(u8 v);
void FM17xx_Delay16us(u32 us);

#endif
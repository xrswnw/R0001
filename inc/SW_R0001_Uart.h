#ifndef _SW_R0001_UART_
#define _SW_R0001_UART_

#include "SW_R0001_Uart_HL.h"

extern UART_RCVFRAME g_sUartRcvTempFrame;
extern UART_RCVFRAME g_sUartRcvFrame;



void Uart_Init(u32 bud);
#endif

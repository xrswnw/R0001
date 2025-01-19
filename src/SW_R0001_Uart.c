#include "SW_R0001_Uart_HL.h"

UART_RCVFRAME g_sUartRcvTempFrame = {0};
UART_RCVFRAME g_sUartRcvFrame = {0};


void Uart_Init(u32 bud)
{
    Uart_InitInterface(bud);
    Uart_ConfigInt();
    Uart_EnableInt(ENABLE, DISABLE);
    Uart_InitTimer(bud);
    Uart_StopRcvTim();
    g_sUartRcvFrame.state = UART_STAT_IDLE;
}
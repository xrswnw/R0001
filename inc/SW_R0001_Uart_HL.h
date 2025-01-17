#ifndef _SW_R0001_UART_HL_
#define _SW_R0001_UART_HL_

#include "SW_R0001_Config.h"
#include "SW_Uart_Receive.h"

#define UART_PORT               USART1
#define UART_BAUDRARE           38400

#define Uart_ReadByte()         ((u16)(UART_PORT->DR & (u16)0x01FF))
extern const PORT_INF UART_485_CTRL;
#define Uart_Enable485Rx()      (UART_485_CTRL.Port->BRR = UART_485_CTRL.Pin)
#define Uart_Enable485Tx()      (UART_485_CTRL.Port->BSRR = UART_485_CTRL.Pin)


void Uart_InitInterface(u32 baudrate);
void Uart_EnableInt(FunctionalState rxState, FunctionalState txState);
void Uart_ConfigInt(void);
void Uart_WriteByte(u8 ch);
void Uart_WriteBuffer(u8 *pBuffer, u32 len);

#define Uart_EnableTxInt()      do{USART_ClearITPendingBit(UART_PORT, USART_FLAG_TC); USART_ITConfig(UART_PORT, USART_IT_TC, ENABLE);}while(0)
#define Uart_DisableTxInt()     do{USART_ClearITPendingBit(UART_PORT, USART_FLAG_TC); USART_ITConfig(UART_PORT, USART_IT_TC, DISABLE);}while(0)

#endif

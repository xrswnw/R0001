#ifndef _SW_BOOT_UART_HL_
#define _SW_BOOT_UART_HL_

#include "SW_Boot_Config.h"

#define UART_PORT               USART1
#define UART_BAUDRARE           38400

#define Uart_ReadByte()         ((u16)(UART_PORT->DR & (u16)0x01FF))


void Uart_InitInterface(u32 baudrate);
void Uart_EnableInt(FunctionalState rxState, FunctionalState txState);
void Uart_ConfigInt(void);
void Uart_WriteByte(u8 ch);
void Uart_WriteBuffer(u8 *pBuffer, u32 len);

#define Uart_EnableTxInt()      do{USART_ClearITPendingBit(UART_PORT, USART_FLAG_TC); USART_ITConfig(UART_PORT, USART_IT_TC, ENABLE);}while(0)
#define Uart_DisableTxInt()     do{USART_ClearITPendingBit(UART_PORT, USART_FLAG_TC); USART_ITConfig(UART_PORT, USART_IT_TC, DISABLE);}while(0)

#endif

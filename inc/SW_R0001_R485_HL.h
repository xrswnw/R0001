#ifndef _SW_R0001_R485_HL_
#define _SW_R0001_R485_HL_

#include "SW_R0001_Config.h"
#include "SW_Uart_Receive.h"

#define R485_PORT               USART2
#define R485_BAUDRARE           38400

#define R485_ReadByte()         ((u16)(R485_PORT->DR & (u16)0x01FF))
extern const PORT_INF R485_IO_CTRL;
#define R485_EnableRx()      (R485_IO_CTRL.Port->BRR = R485_IO_CTRL.Pin)
#define R485_EnableTx()      (R485_IO_CTRL.Port->BSRR = R485_IO_CTRL.Pin)


void R485_InitInterface(u32 baudrate);
void R485_EnableInt(FunctionalState rxState, FunctionalState txState);
void R485_ConfigInt(void);
void R485_WriteByte(u8 ch);
void R485_WriteBuffer(u8 *pBuffer, u32 len);

#define R485_EnableTxInt()      do{USART_ClearITPendingBit(R485_PORT, USART_FLAG_TC); USART_ITConfig(R485_PORT, USART_IT_TC, ENABLE);}while(0)
#define R485_DisableTxInt()     do{USART_ClearITPendingBit(R485_PORT, USART_FLAG_TC); USART_ITConfig(R485_PORT, USART_IT_TC, DISABLE);}while(0)

#endif

#ifndef _SW_R0001_UART_HL_
#define _SW_R0001_UART_HL_

#include "SW_R0001_Config.h"
#include "SW_Uart_Receive.h"

#define UART_PORT               USART1
#define UART_BAUDRARE           115200

#define Uart_ReadByte()         ((u16)(UART_PORT->DR & (u16)0x01FF))

extern const PORT_INF UART_485_CTRL;
#define Uart_Enable485Rx()      (UART_485_CTRL.Port->BRR = UART_485_CTRL.Pin)
#define Uart_Enable485Tx()      (UART_485_CTRL.Port->BSRR = UART_485_CTRL.Pin)


#define UART_RCV_TIMER                  TIM4
#define UART_RCV_TIM_UART               20000 
#define UART_RCV_TIM_9600               4010  //3.5 * 11 * (1/9600) * 1000
#define UART_RCV_TIM_9600M              1750  //超过19200bps都采用1750us
#define UART_RCV_TIM_PRESCALER          119   //120Mhz
#define UART_RCV_TIMER_INT              TIM4_IRQn
#define Uart_ClearRcvTimFlag()          (UART_RCV_TIMER)->SR = (uint16_t)(~TIM_FLAG_Update)
#define Uart_StartRcvTim()              (UART_RCV_TIMER)->CR1 |= TIM_CR1_CEN
#define Uart_StopRcvTim()               (UART_RCV_TIMER)->CR1 &= (~TIM_CR1_CEN)
#define Uart_UpdateTimerPeriod(t)       ((UART_RCV_TIMER)->ARR = (t), (UART_RCV_TIMER)->EGR = TIM_PSCReloadMode_Immediate)
#define Uart_ResetTimCnt()              UART_RCV_TIMER->CNT = 1

void Uart_InitInterface(u32 baudrate);
void Uart_EnableInt(FunctionalState rxState, FunctionalState txState);
void Uart_ConfigInt(void);
void Uart_WriteByte(u8 ch);
void Uart_WriteBuffer(u8 *pBuffer, u32 len);

#define Uart_EnableTxInt()      do{USART_ClearITPendingBit(UART_PORT, USART_FLAG_TC); USART_ITConfig(UART_PORT, USART_IT_TC, ENABLE);}while(0)
#define Uart_DisableTxInt()     do{USART_ClearITPendingBit(UART_PORT, USART_FLAG_TC); USART_ITConfig(UART_PORT, USART_IT_TC, DISABLE);}while(0)



void Uart_InitTimer(u32 baudrate);
#endif

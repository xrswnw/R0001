#include "SW_R0001_Uart_HL.h"

#define UART_INT_CHANNEL        USART1_IRQn

const PORT_INF UART_PORT_TX = {GPIOA, GPIO_Pin_9};
const PORT_INF UART_PORT_RX = {GPIOA, GPIO_Pin_10};
void Uart_InitInterface(u32 baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    USART_DeInit(UART_PORT);

    GPIO_InitStructure.GPIO_Pin = UART_PORT_TX.Pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(UART_PORT_TX.Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = UART_PORT_RX.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART_PORT_RX.Port, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    //Configure USART
    USART_Init(UART_PORT, &USART_InitStructure);
    // Enable the USART
    USART_Cmd(UART_PORT, ENABLE);
}


void Uart_InitTimer(u32 baudrate)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    if(baudrate == 9600)
    {
        TIM_TimeBaseStructure.TIM_Period = UART_RCV_TIM_9600;
    }
    else
    {
        TIM_TimeBaseStructure.TIM_Period = UART_RCV_TIM_9600M;
    }
    TIM_TimeBaseStructure.TIM_Prescaler = UART_RCV_TIM_PRESCALER;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(UART_RCV_TIMER, &TIM_TimeBaseStructure);

    TIM_ITConfig(UART_RCV_TIMER, TIM_IT_Update, ENABLE);
    TIM_Cmd(UART_RCV_TIMER, DISABLE);

    NVIC_InitStructure.NVIC_IRQChannel = UART_RCV_TIMER_INT;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INT_PRIORITY_UART_RX >> 2;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = INT_PRIORITY_UART_RX >> 2;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  	NVIC_Init(&NVIC_InitStructure);
}
void Uart_ConfigInt(void)
{
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    NVIC_InitStructure.NVIC_IRQChannel = UART_INT_CHANNEL;

    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = INT_PRIORITY_UART_RX >> 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = INT_PRIORITY_UART_RX & 0x03;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

void Uart_EnableInt(FunctionalState rxState, FunctionalState txState)
{
    USART_ClearITPendingBit(UART_PORT, USART_IT_RXNE | USART_FLAG_TXE);

    USART_ITConfig(UART_PORT, USART_IT_RXNE, rxState);
    USART_ITConfig(UART_PORT, USART_IT_TC, txState);
}

void Uart_WriteByte(u8 ch)
{
    while(((UART_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
	(UART_PORT)->DR = (ch & (u16)0x01FF);
}

void Uart_Delayms(u32 n)
{
    n *= 0x3800;
    n++;
    while(n--);
}

void Uart_WriteBuffer(u8 *pBuffer, u32 len)
{
    u32 i = 0;
    for(i = 0; i < len; i++)
    {
        while(((UART_PORT)->SR & USART_FLAG_TXE) == (u16)RESET);
	    (UART_PORT)->DR = (pBuffer[i] & (u16)0x01FF);         
    }
}


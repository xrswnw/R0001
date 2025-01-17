#ifndef _SW_R0001_CONFIG_
#define _SW_R0001_CONFIG_

#include "SW_Type.h"
#include "SW_Stdlib.h"
#include "stdio.h"

//�趨����ʱ��
#define SYS_CLOCK_FREQ              72000000
#define SYS_TYPE_HSR100             1

//STM32ƽ̨���
#if (defined(_ANYID_STM32_) || defined(_ANYID_GD32_))
    #include "stm32f10x.h"

    //�˿ڶ���
    typedef struct Port_Inf
    {
        GPIO_TypeDef* Port;
        u16 Pin;
    } PORT_INF;
#endif


#define SYS_ENABLE_WDT              0

#define STM32_CPUID_LEN                 12
#define STM32_CPUID_ADDR                (0x1FFFF7E8)

//���ȼ�������2:2
#define INT_PRIORITY_GROUP              2
#define INT_PRIORITY_STICK              15  //STICK�����ȼ����
#define INT_PRIORITY_UART_RX            14  //���ڽ���
#define INT_PRIORITY_UART_TXDMA         8  //���ڷ���DMA
#define INT_PRIORITY_DOUTDMA            4   //ADDMA
#define INT_PRIORITY_EM4094_DIN         0   //15693�ķ��Ͷ�ʱ�����ȼ����
#define INT_PRIORITY_SAM_IO             0   //SAM��IO���ݣ���INT_PRIORITY_EM4094_DIN����ͬһ��ʱ�̹���
#define INT_PRIORITY_AUDIO_IO           0

#define Sys_EnableInt()                 __enable_irq()
#define Sys_DisableInt()                __disable_irq()

#endif



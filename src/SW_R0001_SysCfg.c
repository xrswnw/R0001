#include "SW_R0001_SysCfg.h"


u32 g_nSysState = 0;

#if SYS_ENABLE_WDT
    #define SYS_ENABLE_TEST         0
#else
    #define SYS_ENABLE_TEST         1
#endif


void Sys_Delayms(u32 n)
{
    n *= 0x3800;
    n++;
    while(n--);
}

void Sys_CfgClock(void)
{
    ErrorStatus HSEStartUpStatus = ERROR;

    RCC_DeInit();
    //Enable HSE
    RCC_HSEConfig(RCC_HSE_ON);

    //Wait till HSE is ready
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {
        //HCLK = SYSCLK = 72M
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        //PCLK2 = HCLK = 72M
        RCC_PCLK2Config(RCC_HCLK_Div1);

        //PCLK1 = HCLK/2 = 36M
        RCC_PCLK1Config(RCC_HCLK_Div2);

        //ADCCLK = PCLK2/2
        RCC_ADCCLKConfig(RCC_PCLK2_Div2);

        // Select USBCLK source 72 / 1.5 = 48M
        RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

        //Flash 2 wait state
        FLASH_SetLatency(FLASH_Latency_2);

        //Enable Prefetch Buffer
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        //PLLCLK = 12MHz * 6 = 72 MHz
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);    //PLL在最后设置

        //Enable PLL
        RCC_PLLCmd(ENABLE);

        //Wait till PLL is ready
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        //Select PLL as system clock source
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        //Wait till PLL is used as system clock source
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
}

void Sys_CfgPeriphClk(FunctionalState state)
{
    //
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOB|
                           RCC_APB2Periph_GPIOC |
                           RCC_APB2Periph_AFIO |
                           RCC_APB2Periph_USART1 |
                           RCC_APB2Periph_SPI1, state);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, state);
}

void Sys_CfgNVIC(void)
{
    //NVIC_InitTypeDef NVIC_InitStructure;
#ifdef  VECT_TAB_RAM
    //Set the Vector Table base location at 0x20000000
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  //VECT_TAB_FLASH
    //Set the Vector Table base location at 0x08000000
#ifdef _ANYID_BOOT_STM32_
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x4000);
#else
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
#endif
#endif

    //Configure the Priority Group to 2 bits
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

const PORT_INF SYS_RUNLED_COM       = {GPIOA, GPIO_Pin_14};
const PORT_INF SYS_ALARMBUZZER_COM  = {GPIOB, GPIO_Pin_15};
const PORT_INF SYS_ALARMLED_COM     = {GPIOA, GPIO_Pin_8};
void Sys_CtrlIOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = SYS_ALARMLED_COM.Pin;
    GPIO_Init(SYS_ALARMLED_COM.Port, &GPIO_InitStructure);

    //RUN
    GPIO_InitStructure.GPIO_Pin = SYS_RUNLED_COM.Pin;
    GPIO_Init(SYS_RUNLED_COM.Port, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = SYS_ALARMBUZZER_COM.Pin;
    GPIO_Init(SYS_ALARMBUZZER_COM.Port, &GPIO_InitStructure);
}

void Sys_Init(void)
{
    Sys_CfgClock();
    Sys_CfgNVIC();
    Sys_CfgPeriphClk(ENABLE);

#if SYS_ENABLE_WDT
    WDG_InitIWDG();
#endif
    //禁止中断
    Sys_DisableInt();
    Sys_CtrlIOInit();

    Sys_RunLedOn();
    Sys_AlarmLedOn();
    Sys_AlarmBuzOn();
    
    FM17xx_InitInterface();
    
    Device_Init();
       
    Uart_Init(UART_BAUDRARE);

    //SysTick 初始化 5ms
    STick_InitSysTick();

    //系统空闲状态
    a_SetState(g_nSysState, SYS_STAT_IDLE);

    Sys_RunLedOff();
    Sys_AlarmLedOff();
    Sys_AlarmBuzOff();

    //使能中断
    Sys_EnableInt();
}

u32 g_nAlarmDelayTime = 0;
u32 sysTime = 0;
void Sys_LedTask(void)
{
    if(a_CheckStateBit(g_nSysState, SYS_STAT_RUNLED))
    {
        sysTime++;
        
        if(sysTime & 0x01)
        {
            Sys_RunLedOn();
        }
        else
        {
            Sys_RunLedOff();
        }
        a_ClearStateBit(g_nSysState, SYS_STAT_RUNLED);
        //Uart_WriteBuffer((u8 *)&g_nSysTick, 4);
    #if SYS_ENABLE_WDT
        WDG_FeedIWDog();
    #endif
    }

    if(a_CheckStateBit(g_nSysState, SYS_STAT_ALARMLED))
    {
        g_nAlarmDelayTime = g_nSysTick;
        Sys_AlarmLedOn();
        if(a_CheckStateBit(g_nSysState, SYS_STAT_ALARMBUZZER))
        {
            Sys_AlarmBuzOn();
            a_ClearStateBit(g_nSysState, SYS_STAT_ALARMBUZZER);
        } 
        a_ClearStateBit(g_nSysState, SYS_STAT_ALARMLED);
        a_SetStateBit(g_nSysState, SYS_STAT_ALARMDLY);
    }

    if(a_CheckStateBit(g_nSysState, SYS_STAT_ALARMDLY))
    {
        if(g_nAlarmDelayTime + 20 < g_nSysTick)
        {
            a_ClearStateBit(g_nSysState, SYS_STAT_ALARMDLY);

            Sys_AlarmLedOff();
            Sys_AlarmBuzOff();
        }
    }
    
}

void Sys_UartTask(void)
{
      if(USART_GetFlagStatus(UART_PORT, USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
    {
        USART_ClearFlag(UART_PORT, USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);
        Uart_EnableInt(DISABLE, DISABLE);
        Uart_Init(UART_BAUDRARE);
    }

    //串口数据帧解析
  if(Uart_IsRcvFrame(g_sUartRcvFrame))
    {
        u16 startPos = 0, txLen = 0;
        
        memcpy(&g_sUartRcvTempFrame, &g_sUartRcvFrame, sizeof(g_sUartRcvFrame));
        Uart_ResetFrame(&g_sUartRcvFrame);
        if(Uart_UsrCheckFrame(g_sUartRcvTempFrame.buffer, g_sUartRcvTempFrame.length, &startPos))
        {
            g_sReaderRspFrame.com = READER_COM_UART;
            txLen = Device_ProcessUartFrame(g_sUartRcvTempFrame.buffer + startPos, g_sUartRcvTempFrame.length);
            if(txLen > 0)
            {
                a_SetStateBit(g_nSysState, SYS_STAT_UARTTX);
            }
        }
    }
    
    if(a_CheckStateBit(g_nSysState, SYS_STAT_UARTTX))
    {
        Uart_WriteBuffer(g_sDevicerRspFrame.buffer, g_sDevicerRspFrame.len);
        a_ClearStateBit(g_nSysState, SYS_STAT_ALARMLED);
        
        if(g_sDevicerRspFrame.cmd == UART_CMD_RESET)
        {
            Device_Delayms(20);
            Sys_SoftReset();
        }
    }
}

void Sys_DeviceTask()
{
    if(a_CheckStateBit(g_sDeviceOp.state, DEVICE_STAT_IDLE))
    {
        FM17xx_OpenAntenna();
        Sys_Delayms(2);
        Device_AutoTask();
    }

    if(a_CheckStateBit(g_nSysState, SYS_STAT_MONITOR_EXPORT))
    {
        a_ClearStateBit(g_nSysState, SYS_STAT_MONITOR_EXPORT);
        Device_MonitorExport(g_sDeviceOp.proofFlag);
    }
    
    if(a_CheckStateBit(g_sDeviceOp.state, DEVICE_STAT_TX))
    {
        if(g_sDeviceOp.index < g_sDeviceOp.num)
        {
            if(Device_Transm(&g_sDeviceOp))
            {
                a_SetState(g_sDeviceOp.state, DEVICE_STAT_RX);
            }
        }
    }
    
    if(a_CheckStateBit(g_sDeviceOp.state, DEVICE_STAT_RX))
    {
        Device_Receive(&g_sDeviceOp);
        a_SetState(g_sDeviceOp.state, DEVICE_STAT_STEP);
    }
    
    if(a_CheckStateBit(g_sDeviceOp.state, DEVICE_STAT_STEP))
    {
        BOOL b = FALSE;
        b = Device_Step(&g_sDeviceOp);
        if((!b) || (g_sDeviceOp.index >= g_sDeviceOp.num))
        {
            a_SetState(g_sDeviceOp.state, DEVICE_STAT_DELAY);
            g_sDeviceOp.delay = g_nSysTick;
            FM17xx_CloseAntenna();
            Sys_Delayms(1);
            if(g_sDeviceOp.index >= g_sDeviceOp.num && b == TRUE)       //操作成功
            {
                if(g_sDeviceOp.bProof == TRUE || g_sDeviceOp.bRepeatTag == TRUE)
                {
                    g_sDeviceOpTagInfo.bTagOk = TRUE;
                    a_SetStateBit(g_nSysState, SYS_STAT_ALARMBUZZER);
                    memcpy(&g_sDeviceOpTagInfo.okTag, &g_sDeviceOp.tag, sizeof(ISO14443A_UID));   //操作成功标签
                }
            }
            
            if(g_sDeviceOp.tagNum)
            {
                a_SetStateBit(g_nSysState, SYS_STAT_ALARMLED);
            }
        }
        else
        {
            a_SetState(g_sDeviceOp.state, DEVICE_STAT_TX);
        }
    }
    
    if(a_CheckStateBit(g_sDeviceOp.state, DEVICE_STAT_DELAY))
    {
        if(g_sDeviceOp.delay + DEVICE_OP_DELAY_TICK < g_nSysTick)
        {
            a_SetState(g_sDeviceOp.state, DEVICE_STAT_IDLE);
        }
    }
}
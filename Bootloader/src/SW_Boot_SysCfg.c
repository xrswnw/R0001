#include "SW_Boot_SysCfg.h"


u32 g_nSysState = 0;

u32 g_nDeviceNxtEraseAddr = 0;
u32 g_nDeviceNxtDownloadAddr = 0;


#define SYS_BOOT_VER_SIZE               50
const u8 SYS_BOOT_VERSION[SYS_BOOT_VER_SIZE] = "Boot V3.0_20030600 GD32F1xx";

void Sys_Delayms(u32 n)             //系统延时n毫秒
{
    n *= 0x3800;
    n++;
    while(n--);
}

void Sys_CfgClock(void)
{
    ErrorStatus HSEStartUpStatus;

    RCC_DeInit();
    //Enable HSE
    RCC_HSEConfig(RCC_HSE_ON);

    //Wait till HSE is ready
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {
        //HCLK = SYSCLK = 67.8M
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        //PCLK2 = HCLK = 67.8M
        RCC_PCLK2Config(RCC_HCLK_Div1);

        //PCLK1 = HCLK/2 = 33.9M
        RCC_PCLK1Config(RCC_HCLK_Div2);

        //ADCCLK = PCLK2/2
        RCC_ADCCLKConfig(RCC_PCLK2_Div2);

        //Flash 2 wait state
        FLASH_SetLatency(FLASH_Latency_2);

        //Enable Prefetch Buffer
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        //PLLCLK = 12.00MHz * 6 = 72.0 MHz
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
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                          RCC_APB2Periph_GPIOB |
                          RCC_APB2Periph_GPIOC |
                          RCC_APB2Periph_AFIO |
                          RCC_APB2Periph_USART1, state);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, state);
}

void Sys_CfgNVIC(void)
{
    //NVIC_InitTypeDef NVIC_InitStructure;
#ifdef  VECT_TAB_RAM
    //Set the Vector Table base location at 0x20000000
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  //VECT_TAB_FLASH
    //Set the Vector Table base location at 0x08000000
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
#endif
    //Configure the Priority Group to 2 bits
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

const PORT_INF SYS_RUNLED_COM = {GPIOB, GPIO_Pin_5};
const PORT_INF SYS_ALARMLED_COM = {GPIOB, GPIO_Pin_9};
void Sys_CtrlIOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    //RUN
    GPIO_InitStructure.GPIO_Pin = SYS_RUNLED_COM.Pin;
    GPIO_Init(SYS_RUNLED_COM.Port, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = SYS_ALARMLED_COM.Pin;
    GPIO_Init(SYS_ALARMLED_COM.Port, &GPIO_InitStructure);
}

void Sys_Init(void)
{
#if SYS_ENABLE_WDT
    WDG_InitIWDG();
#endif
    //
    Sys_CfgClock();

    Sys_CfgNVIC();
    Sys_CfgPeriphClk(ENABLE);

    //禁止中断
    Sys_DisableInt();

    Sys_CtrlIOInit();
    Sys_RunLedOn();
    Sys_AlarmLedOn();

    Sys_Delayms(10);        //FRAM上电必须延时10ms

    if(Sys_ReadBootParamenter(DEVICE_APP_PAR_START) == FALSE)   //如果存储区失败，就去备份区再去读取
    {
        Sys_Delayms(1000);
        if(Sys_ReadBootParamenter(DEVICE_APPPAR_BACK_START) == TRUE)
        {
            Sys_Delayms(500);
            Sys_WriteBootParamenter(DEVICE_APP_PAR_START);      //并且再次写入存储区
        }
        else
        {
            Sys_WriteBootParamenter(DEVICE_APP_PAR_START);
            Sys_WriteBootParamenter(DEVICE_APPPAR_BACK_START);
        }
    }
    
    if(g_sSysBootParamenter.appState == SYS_BOOT_APP_OK)
    {
        if(Sys_CheckVersion() == FALSE)
        {
            g_sSysBootParamenter.appState = SYS_BOOT_APP_FAIL;
        }
    }
    else if(g_sSysBootParamenter.appState == SYS_BOOT_APP_NONE)
    {
        g_sSysBootParamenter.appState = SYS_BOOT_APP_FAIL;
        if(Sys_CheckVersion() == TRUE && Sys_CheckAppEmpty() == FALSE)          //状态错误，但是版本信息校验正确
        {
            g_sSysBootParamenter.appState = SYS_BOOT_APP_OK;
        }
    }
    
    Uart_InitInterface(UART_BAUDRARE);
    Uart_ConfigInt();
    Uart_EnableInt(ENABLE, DISABLE);

    //SysTick 初始化 5ms
    STick_InitSysTick();
    Sys_RunLedOff();
    Sys_AlarmLedOff();

    //系统空闲状态
    if(g_sSysBootParamenter.appState == FRAM_BOOT_APP_OK)
    {
        a_SetState(g_nSysState, SYS_STAT_IDLE);
    }
    else
    {
        a_SetState(g_nSysState, SYS_STAT_DOWNLOAD);
    }

    //使能中断
    Sys_EnableInt();
}

void Sys_LedTask(void)
{
    if(a_CheckStateBit(g_nSysState, SYS_STAT_RUNLED))
    {
        static u32 ledTimes = 0;

        a_ClearStateBit(g_nSysState, SYS_STAT_RUNLED);

        ledTimes++;

        if(ledTimes & 0x01)
        {
            Sys_AlarmLedOn();
        }
        else
        {
            Sys_AlarmLedOff();
        }

    #if SYS_ENABLE_WDT
        WDG_FeedIWDog();
    #endif
    }
}

typedef  void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;
void Sys_Jump(u32 address)
{
    u32 stackAddr = 0;
    Sys_DisableInt();
    stackAddr = *((vu32 *)address);
    //查看栈地址是否在RAM区或者CCRAM区
    if((stackAddr & 0x2FFE0000) == 0x20000000)
    {
        JumpAddress = *(vu32 *)(address + 4);
        Jump_To_Application = (pFunction) JumpAddress;

        __set_MSP(*(vu32 *)address);
        Jump_To_Application();
    }
    //else
    {
        a_SetState(g_nSysState, SYS_STAT_IDLE);
        g_nSysTick = 0;
    }
    //while(1)
    {
    #if SYS_ENABLE_WDT
        WDG_FeedIWDog();
    #endif
    }
    Sys_EnableInt();
}

void Sys_BootTask(void)
{
    if(!a_CheckStateBit(g_nSysState, SYS_STAT_DOWNLOAD))
    {
        if(g_nSysTick > 40 && g_sSysBootParamenter.appState == FRAM_BOOT_APP_OK)                //TICK = STICK_TIME_MS = 5MS, 延时200ms等待boot选择
        {
            a_SetStateBit(g_nSysState, SYS_STAT_JMP);
        }
    }

    if(a_CheckStateBit(g_nSysState, SYS_STAT_JMP))
    {
        Sys_RunLedOff();
        Sys_Jump(SYS_APP_START_ADDR);
    }
}

void Sys_UartTask(void)
{
    //串口错误处理:重新初始化
    if(USART_GetFlagStatus(UART_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
    {
        USART_ClearFlag(UART_PORT, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);
        Uart_InitInterface(UART_BAUDRARE);
        Uart_ConfigInt();
        Uart_EnableInt(ENABLE, DISABLE);
    }

    //串口数据帧解析
    if(Uart_IsRcvFrame(g_sUartRcvFrame))
    {
        Sys_ProcessBootFrame(&g_sUartRcvFrame, SYS_COM_UART);
        Uart_ResetFrame(g_sUartRcvFrame);
    }
    
    if(Uart_IsRcvFrame(g_sR485RcvFrame))
    {
        Sys_ProcessBootFrame(&g_sR485RcvFrame, SYS_COM_R485);
        Uart_ResetFrame(g_sR485RcvFrame);
    }
}

BOOL Sys_CheckVersion(void)
{
    BOOL b = FALSE;
    u8 *p = (u8 *)SYS_BOOT_VER_ADDR;
    u8 i = 0, c = 0;
        
    if(memcmp(p, SYS_VER_HEAD, SYS_VER_HEAD_SIZE) == 0) //设备型号正确
    {
        for(i = SYS_VER_HEAD_SIZE; i < SYS_VERSION_SIZE; i++)
        {
            c = *p++;
            if((c < ' ' || c > 127) && (c != 0x00))
            {
                break;
            }
        }
        if(i == SYS_VERSION_SIZE)
        {
            b = TRUE;
        }
    }
    
    if(b == FALSE)
    {
        if(memcmp((u8 *)SYS_BOOT_HARDTYPE_ADDR, SYS_VER_HARD_TYPE, sizeof(SYS_VER_HARD_TYPE) - 1) == 0) //设备硬件型号正确也可以
        {
            b = TRUE;
        }
    }
    return b;
}

#define SYS_BOOT_CHK_APP_LEN               128
BOOL Sys_CheckAppEmpty(void)
{
    BOOL b = FALSE;
    u16 i = 0;
    u32 *p = (u32 *)(SYS_APP_START_ADDR + 512); //检查第一个扇区的最后512直接
    for(i = 0; i < SYS_BOOT_CHK_APP_LEN; i++)
    {
        if(p[i] != 0xFFFFFFFF)
        {
            break;
        }
    }
    if(i == SYS_BOOT_CHK_APP_LEN)
    {
        b = TRUE;
    }
    
    return b;
}

void Sys_ProcessBootFrame(UART_RCVFRAME *pRcvFrame, u8 com)
{
    u16 crc1 = 0, crc2 = 0;
    memcpy(&g_sUartTempRcvFrame, pRcvFrame, sizeof(UART_RCVFRAME));


    crc1 = Uart_GetFrameCrc(g_sUartTempRcvFrame.buffer, g_sUartTempRcvFrame.index);
    crc2 = a_GetCrc(g_sUartTempRcvFrame.buffer + UART_FRAME_POS_LEN, g_sUartTempRcvFrame.index - 4);

    if(crc1 == crc2)
    {
        u8 cmd = g_sUartTempRcvFrame.buffer[UART_FRAME_POS_CMD];
        u16 destAddr = 0;
        
        destAddr = *((u16 *)(pRcvFrame->buffer + UART_FRAME_POS_DESTADDR));
        if((destAddr != SYS_FRAME_BROADCAST_ADDR) && (destAddr != g_sSysBootParamenter.addr))
        {
            return;
        }
        switch(cmd)
        {
            case UART_FRAME_CMD_RESET:
                g_nSysTick = 0;
                g_sUartTxFrame.len = Uart_RspReset();
                break;
            case UART_FRAME_CMD_ERASE:
                if(a_CheckStateBit(g_nSysState, SYS_STAT_DOWNLOAD))
                {
                    BOOL bOk = FALSE;
                    u32 addr = 0;
                    u8 sector = 0;
                    
                    sector = g_sUartTempRcvFrame.buffer[UART_FRAME_POS_PAR];
                    addr = SYS_APP_START_ADDR + (sector << 10);         //每个扇区1K
                    
                    if(addr >= SYS_APP_START_ADDR)
                    {
                        if(g_nDeviceNxtEraseAddr == addr)               //擦除地址必须是连续的，否则会有区域未擦除
                        {
                            g_nDeviceNxtEraseAddr = addr + (1 << 10);   //每个扇区1K
                            
                            bOk = Uart_EraseFlash(addr); 
                            g_sUartTxFrame.len = Uart_RspErase(bOk);
                        }
                    }
                }
                break;
            case UART_FRAME_CMD_DL:
                if(a_CheckStateBit(g_nSysState, SYS_STAT_DOWNLOAD))
                {
                    BOOL bCheck = FALSE;
                    u32 addr = 0;
                    u32 size = 0;

                    if(g_sUartTempRcvFrame.buffer[UART_FRAME_POS_LEN] == 0x00)
                    {
                        bCheck = (BOOL)(g_sUartTempRcvFrame.buffer[UART_FRAME_POS_PAR + 0]);
                        addr = *((u32 *)(g_sUartTempRcvFrame.buffer + UART_FRAME_POS_PAR + 1));
                        size = *((u32 *)(g_sUartTempRcvFrame.buffer + UART_FRAME_POS_PAR + 5));
                        if(addr >= SYS_APP_START_ADDR)
                        {
                            //第一次不需要判定地址连续问题，因为boot程序是从后向前写数据，第一次不知道地址是什么
                            if(addr + size == g_nDeviceNxtDownloadAddr || g_nDeviceNxtDownloadAddr == 0)
                            {
                                //帧参数之前部分 + 基本参数(1 + 4 + 4) + size + crclen;
                                //frameLen = UART_FRAME_POS_PAR + 9 + size + 2;
                                Sys_AlarmLedOn();
                                Sys_RunLedOn();

                                if(BL_WriteImagePage(addr, g_sUartTempRcvFrame.buffer + UART_FRAME_POS_PAR + 9, size))
                                {
                                    g_sUartTxFrame.len = Uart_RspDownload(bCheck, addr, size);
                                }

                                Sys_AlarmLedOff();
                                Sys_RunLedOff();
                            }
                        }
                    }
                }
                break;
            case UART_FRAME_CMD_BOOT:
                g_sUartTxFrame.len = Uart_RspBoot();
                if(com == SYS_COM_UART)     //擦除需要时间，这里先发送响应帧
                {
                    Uart_WriteBuffer(g_sUartTxFrame.frame, g_sUartTxFrame.len);
                }
                else if(com == SYS_COM_R485)
                {
                    R485_WriteBuffer(g_sUartTxFrame.frame, g_sUartTxFrame.len);
                }
                g_sUartTxFrame.len = 0;
                a_ClearStateBit(g_nSysState, SYS_STAT_IDLE);
                a_SetStateBit(g_nSysState, SYS_STAT_DOWNLOAD);
                g_sSysBootParamenter.appState = FRAM_BOOT_APP_FAIL;
                Fram_WriteBootParamenter();
                FLASH_Unlock();
                
                Uart_EraseFlash(SYS_BOOT_VER_ADDR);          //版本信息区域擦除
                g_nDeviceNxtEraseAddr = SYS_APP_START_ADDR;
                g_nDeviceNxtDownloadAddr = 0;                   //boot是由后向前写入数据
                break;
            case UART_FRAME_CMD_JMP:
                if(Sys_CheckVersion() == TRUE)
                {
                    g_sUartTxFrame.len = Uart_RspJmp();
                    a_SetStateBit(g_nSysState, SYS_STAT_JMP);
                    g_sSysBootParamenter.appState = FRAM_BOOT_APP_OK;
                    Fram_WriteBootParamenter();
                    FLASH_Lock();
                }
                break;
            case UART_FRAME_CMD_VER:
                g_sUartTxFrame.len = Uart_RspFrame(g_sUartTxFrame.frame, cmd, (u8 *)SYS_BOOT_VERSION, SYS_BOOT_VER_SIZE, UART_FRAME_FLAG_OK, UART_FRAME_RSP_NOERR);
                break;
            default:
                break;
        }
    }
    if(g_sUartTxFrame.len)
    {
        if(com == SYS_COM_UART)
        {
            Uart_WriteBuffer(g_sUartTxFrame.frame, g_sUartTxFrame.len);
        }
        else if(com == SYS_COM_R485)
        {
            R485_WriteBuffer(g_sUartTxFrame.frame, g_sUartTxFrame.len);
        }
        Sys_Delayms(2);//等待最后一个字节发送完成
        g_sUartTxFrame.len = 0;
    }
}

SYS_BOOTPARAMS g_sSysBootParamenter = {0};
BOOL Sys_ReadBootParamenter(u32 addr)                                         //OK
{
    u8 i = 0;
    u8 *p = NULL;
    u8 *pDevicePar = NULL;
    u16 crc1 = 0, crc2 = 0;
    BOOL b = FALSE;

    p = (u8 *)(addr);
    pDevicePar = (u8 *)(&g_sSysBootParamenter);
    for(i = 0; i < sizeof(SYS_BOOTPARAMS); i++)
    {
        *(pDevicePar + i) = *(p + i);
    }

    crc1 = a_GetCrc((u8 *)pDevicePar, (sizeof(SYS_BOOTPARAMS)) - 4);
    crc2 = g_sSysBootParamenter.crc;

    //重新写入默认值
    if((crc1 != crc2))
    {
        memset(&g_sSysBootParamenter, 0, sizeof(g_sSysBootParamenter));
        g_sSysBootParamenter.appState = SYS_BOOT_APP_NONE;
    }
    else
    {
        b = TRUE;
    }
    
    return b;
}



BOOL Sys_WriteBootParamenter(u32 addr)
{
    u8 i = 0;
    BOOL b = FALSE;
    u16 *pDevicePar = NULL;
    FLASH_Status flashStatus = FLASH_BUSY;

    pDevicePar = (u16 *)(&g_sSysBootParamenter);

    g_sSysBootParamenter.crc = 0;
    g_sSysBootParamenter.crc = a_GetCrc((u8 *)pDevicePar, (sizeof(SYS_BOOTPARAMS)) - 4);

#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif
    FLASH_Unlock();
    flashStatus = FLASH_ErasePage(addr);
    if(flashStatus == FLASH_COMPLETE)
    {
        for(i = 0; i < (sizeof(SYS_BOOTPARAMS) >> 1) + 1; i++)
        {
            if(flashStatus != FLASH_COMPLETE)
            {
                break;
            }
            flashStatus = FLASH_ProgramHalfWord(addr + (i << 1), pDevicePar[i]);
        }

        if(i >= (sizeof(SYS_BOOTPARAMS) >> 1))
        {
            b = TRUE;
        }
    }
    FLASH_Lock();
    return b;
}
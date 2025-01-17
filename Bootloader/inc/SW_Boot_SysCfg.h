#ifndef _SW_BOOT_SYSCFG_
#define _SW_BOOT_SYSCFG_

#include "SW_Boot_Uart.h"
#include "SW_Boot_R485.h"
#include "SW_Boot_Fram.h"
#include "SW_Boot_SysTick_HL.h"

extern u32 g_nSysState;
#define SYS_STAT_IDLE           0x00000001	    //系统初始化后处于空闲状态
#define SYS_STAT_RUNLED         0x00000002
#define SYS_STAT_DOWNLOAD       0x00000040
#define SYS_STAT_JMP            0x00000080
#define SYS_STAT_RESET          0x00000100
#define SYS_STAT_ALARMLED       0x00000200

void Sys_Jump(u32 address);
typedef  void (*pSysFunction)(void);
//灯
extern const PORT_INF SYS_RUNLED_COM;
#define Sys_RunLedOff()             SYS_RUNLED_COM.Port->BSRR = SYS_RUNLED_COM.Pin
#define Sys_RunLedOn()              SYS_RUNLED_COM.Port->BRR = SYS_RUNLED_COM.Pin

extern const PORT_INF SYS_ALARMLED_COM;
#define Sys_AlarmLedOff()           SYS_ALARMLED_COM.Port->BSRR = SYS_ALARMLED_COM.Pin
#define Sys_AlarmLedOn()            SYS_ALARMLED_COM.Port->BRR = SYS_ALARMLED_COM.Pin

void Sys_Delay5ms(u32 n);

void Sys_CfgClock(void);
void Sys_CfgPeriphClk(FunctionalState state);
void Sys_CfgNVIC(void);

#define Sys_SoftReset()     (*((u32 *)0xE000ED0C) = 0x05fa0004)

void Sys_CtrlIOInit(void);
void Sys_Init(void);

void Sys_UartTask(void);
void Sys_ProcessUartFrame(u8 *pBuffer, u16 len);
void Sys_LedTask(void);
void Sys_UartTask(void);
void Sys_BootTask(void);
void Sys_ProcessBootFrame(UART_RCVFRAME *pRcvFrame, u8 com);

#define SYS_VER_HEAD                            "R0001 "
#define SYS_VER_HEAD_SIZE                       6
#define SYS_VER_HARD_TYPE                       "R001-SCH-MB-250100"
        
#define SYS_VERSION_SIZE                        50  
#define SYS_BOOT_VER_ADDR                       0x08005000          //版本固定在当前系统
#define SYS_BOOT_HARDTYPE_ADDR                  0x08005080          //硬件信息，同一个硬件信息可以下载类似的固件
#define SYS_APP_START_ADDR                      0x08004000
BOOL Sys_CheckVersion(void);
BOOL Sys_CheckAppEmpty(void);
#define SYS_FRAME_BROADCAST_ADDR                0xFFFF

#define DEVICE_APP_PAR_START                    (0x0800F800)
#define DEVICE_APPPAR_BACK_START                (0x0800F000)

#define SYS_BOOT_APP_OK                         0x5555
#define SYS_BOOT_APP_FAIL                       0xAAAA
#define SYS_BOOT_APP_NONE                       0xFFFF      //crc校验错误，或者IIC读取失败
typedef struct sysBootDevicePar{
    u16 appState;
    u16 addr;
    u32 br;
    u32 rfu;
    u32 crc;
}SYS_BOOTPARAMS;
extern SYS_BOOTPARAMS g_sSysBootParamenter;

BOOL Sys_ReadBootParamenter(u32 addr);
BOOL Sys_WriteBootParamenter(u32 addr);
#define SYS_COM_R485        0
#define SYS_COM_UART        1
#endif

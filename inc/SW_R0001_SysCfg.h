#ifndef _SW_R1001_SYSCFG_
#define _SW_R1001_SYSCFG_


#include "SW_R0001_Device.h"

extern u32 g_nAlarmDelay;

extern u32 g_nSysState;
#define SYS_STAT_IDLE           0x00000001	    //系统初始化后处于空闲状态
#define SYS_STAT_RUNLED         0x00000002
#define SYS_STAT_UARTTX         0x00000004
#define SYS_STAT_ALARMLED       0x00000010
#define SYS_STAT_ALARMDLY       0x00000020
#define SYS_STAT_AUTOUID        0x00000040

#define SYS_STAT_ALARMKEY       0x00000008
#define SYS_STAT_ALARMKDLY      0x00000400
//灯
extern const PORT_INF SYS_RUNLED_COM;
#define Sys_RunLedOff()         SYS_RUNLED_COM.Port->BSRR = SYS_RUNLED_COM.Pin
#define Sys_RunLedOn()          SYS_RUNLED_COM.Port->BRR = SYS_RUNLED_COM.Pin

extern const PORT_INF SYS_ALARMLED_COM;
#define Sys_AlarmLedOn()        SYS_ALARMLED_COM.Port->BRR = SYS_ALARMLED_COM.Pin
#define Sys_AlarmLedOff()       SYS_ALARMLED_COM.Port->BSRR = SYS_ALARMLED_COM.Pin

extern const PORT_INF SYS_ALARMBUZZER_COM;
#define Sys_AlarmBuzOn()        SYS_ALARMBUZZER_COM.Port->BRR = SYS_ALARMBUZZER_COM.Pin
#define Sys_AlarmBuzOff()       SYS_ALARMBUZZER_COM.Port->BSRR = SYS_ALARMBUZZER_COM.Pin

void Sys_Delay5ms(u32 n);
void Sys_CfgClock(void);
void Sys_CfgPeriphClk(FunctionalState state);
void Sys_CfgNVIC(void);

#define Sys_SoftReset()     (*((u32 *)0xE000ED0C) = 0x05fa0004)

void Sys_CtrlIOInit(void);
void Sys_Init(void);

void Sys_LedTask(void);
void Sys_ReaderTask(void);
void Sys_UartTask(void);
#endif

#ifndef _SW_R0001_FRAM_
#define _SW_R0001_FRAM_

#include "SW_FRam.h"

#define FRAM_BOOT_APP_OK                    0x5555
#define FRAM_BOOT_APP_FAIL                  0xAAAA

#define READER_BR_9600                      5
#define READER_BR_38400                     7
#define READER_BR_115200                    11
typedef struct framBootDevicePar{
    u16 appState;
    u16 br;
    u16 addr;
    u16 crc;
}FRAM_BOOTPARAMS;
extern FRAM_BOOTPARAMS g_sFramBootParamenter;

#define FRAME_MEM_SIZE                      8192    //8k
#define FRAME_BOOT_INFO_ADDR                (FRAME_MEM_SIZE - sizeof(FRAM_BOOTPARAMS))      //固件升级从480字节开始写，共512字节
#define FRAME_SYS_INFO_ADDR                 0x00

void Fram_ReadBootParamenter(void);
BOOL Fram_WriteBootParamenter(void);
u32 Fram_GetUartBaudrate(void);
#endif


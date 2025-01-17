#include "SW_R0001_SysCfg.h"

int main(void)
{
    Sys_Init();

    while(1)
    {
        Sys_LedTask();
        Sys_UartTask();
        Sys_DeviceTask();
    }
}


#ifdef  DEBUG
void assert_failed(u8* file, u32 line)
{
    while(1)
    {
    }
}
#endif

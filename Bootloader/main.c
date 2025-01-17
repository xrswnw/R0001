#include "SW_Boot_SysCfg.h"

int main(void)
{
    Sys_Init();
    while(1)
    {
        Sys_LedTask();
        Sys_BootTask();
        Sys_UartTask();
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

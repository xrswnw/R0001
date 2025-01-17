#include "SW_R0001_WDG_HL.h"


#define WDG_TIME                        (150 * 10)       //200ms * 10
#define WDG_UNIT_0800US                 IWDG_Prescaler_32


void WDG_InitIWDG(void)
{
    //Enable write access to IWDG_PR and IWDG_RLR registers
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    //IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz
    IWDG_SetPrescaler(WDG_UNIT_0800US);

    //Set counter reload value
    IWDG_SetReload(WDG_TIME);

    //Reload IWDG counter
    IWDG_ReloadCounter();

    //Enable IWDG (the LSI oscillator will be enabled by hardware)
    IWDG_Enable();

}

BOOL WDG_IsIWDGReset(void)
{
    BOOL b = FALSE;

    if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
    {
        RCC_ClearFlag();
        b = TRUE;
    }

    return b;
}


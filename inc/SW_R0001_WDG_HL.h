#ifndef _SW_R0001_WDG_HL_
#define _SW_R0001_WDG_HL_

#include "SW_R0001_Config.h"


void WDG_InitIWDG(void);
BOOL WDG_IsIWDGReset(void);


#define WDG_FeedIWDog()	IWDG_ReloadCounter()


#endif


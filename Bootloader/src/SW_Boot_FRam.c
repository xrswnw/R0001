#include "SW_Boot_FRam.h"

FRAM_BOOTPARAMS g_sFramBootParamenter = {0};

void Fram_ReadBootParamenter(void)
{
    BOOL b = FALSE;

    b = FRam_ReadBuffer(FRAME_BOOT_INFO_ADDR, sizeof(FRAM_BOOTPARAMS), (u8 *)(&g_sFramBootParamenter));
    if(b)
    {
        u16 crc1 = 0, crc2 = 0;

        crc1 = a_GetCrc((u8 *)(&g_sFramBootParamenter), (sizeof(FRAM_BOOTPARAMS)) - 2);
        crc2 = g_sFramBootParamenter.crc;

        //�������Ƿ���ȷ���������ȷ��ʹ��Ĭ�ϲ�������
        if(crc1 != crc2)
        {
            b = FALSE;
        }
    }

    if(b == FALSE)
    {
        g_sFramBootParamenter.appState = FRAM_BOOT_APP_FAIL;
	#if defined(_ANYID_D322P_)
		g_sFramBootParamenter.br = FRAM_BR_115200;
	#else
        g_sFramBootParamenter.br = FRAM_BR_38400;
	#endif
    }

    Sys_DisableInt();
}

BOOL Fram_WriteBootParamenter(void)
{
    BOOL b = FALSE;

    g_sFramBootParamenter.crc = 0;
    g_sFramBootParamenter.crc = a_GetCrc((u8 *)(&g_sFramBootParamenter), (sizeof(FRAM_BOOTPARAMS)) - 2);

    b = FRam_WriteBuffer(FRAME_BOOT_INFO_ADDR, sizeof(FRAM_BOOTPARAMS), (u8 *)(&g_sFramBootParamenter));

    return b;
}

u32 Fram_GetUartBaudrate(void)
{
    if(g_sFramBootParamenter.br == FRAM_BR_9600)
    {
        return 9600;
    }
    else if(g_sFramBootParamenter.br == FRAM_BR_115200)
    {
        return 115200;
    }
    else
    {
        return 38400;
    }
}
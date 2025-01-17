#include "SW_Boot_Reader.h"

READER_DEVICE_PARAMETER g_sDeviceParamenter = {0};

u32 Reader_GetUartBaudrate(void)
{
    if(g_sDeviceParamenter.controlLow.baudRate == READER_BAUDRATE9600)
    {
        return 9600;
    }
    else if(g_sDeviceParamenter.controlLow.baudRate == READER_BAUDRATE115200)
    {
        return 115200;
    }
    else
    {
        return 38400;
    }
}

BOOL Reader_WriteDeviceConfig(void)
{
    BOOL b = TRUE;
    u8 i = 0;
    u16 *pDevicePar = NULL;
    FLASH_Status flashStatus = FLASH_BUSY;

    pDevicePar = (u16 *)(&g_sDeviceParamenter);

    g_sDeviceParamenter.crc = 0;
    g_sDeviceParamenter.crc = a_GetCrc((u8 *)pDevicePar, (sizeof(READER_DEVICE_PARAMETER)) - 2);

    FLASH_Unlock();
    //如果发生错误就延时等待，并继续写数据，直到写数据成功
    //while(flashStatus != FLASH_COMPLETE)
    {
        flashStatus = FLASH_ErasePage(READER_DEVICE_PAR_START);
        for(i = 0; i < (sizeof(READER_DEVICE_PARAMETER) >> 1); i++)
        {
            if(flashStatus != FLASH_COMPLETE)
            {
                b = FALSE;
                break;
            }
            flashStatus = FLASH_ProgramHalfWord(READER_DEVICE_PAR_START + (i << 1), pDevicePar[i]);
        }
    }

    FLASH_Lock();

    return b;
}

void Reader_ReadDeviceConfig(void)
{
    u16 i = 0;
    u8 *p = NULL;
    u8 *pDevicePar = NULL;
    u16 crc1 = 0, crc2 = 0;
    BOOL b = TRUE;

    p = (u8 *)(READER_DEVICE_PAR_START);
    pDevicePar = (u8 *)(&g_sDeviceParamenter);
    FLASH_Unlock();
    for(i = 0; i < sizeof(READER_DEVICE_PARAMETER); i++)
    {
        *(pDevicePar + i) = *(p + i);
    }
    crc1 = a_GetCrc((u8 *)pDevicePar, (sizeof(READER_DEVICE_PARAMETER)) - 2);
    crc2 = g_sDeviceParamenter.crc;

    //检测参数是否正确，如果不正确，使用默认参数操作
    if(crc1 != crc2)
    {
        b = FALSE;
        memset(&g_sDeviceParamenter, 0, sizeof(g_sDeviceParamenter));

        g_sDeviceParamenter.workMode = READER_TYPE_ISO15693 | READER_MODE_INVENTORY;
	    g_sDeviceParamenter.addr = 0x0001;

	    g_sDeviceParamenter.controlLow.baudRate = READER_BAUDRATE38400;
	    g_sDeviceParamenter.controlLow.tagQuiet = READER_TAG_QUIET;
        g_sDeviceParamenter.controlLow.inventoryMode = READER_INVENTORY_TRIGGER;
        g_sDeviceParamenter.controlLow.uidTransMode = READER_UID_POSITIVE;
        g_sDeviceParamenter.controlLow.afiControl = READER_AFI_DISABLE;

	    //open bell
	    g_sDeviceParamenter.controlHigh.buzzer = READER_BUZZER_ENABLE;
        g_sDeviceParamenter.controlHigh.rfu = 0x00;
	    //
	    g_sDeviceParamenter.afiValue = 0x01;
	    //open RF
        g_sDeviceParamenter.rfCtrl = READER_RF_OPEN;
        g_sDeviceParamenter.appState = FRAM_BOOT_APP_FAIL;
    }
    FLASH_Lock();

    //重新写入默认值
    if(b == FALSE)
    {
        Reader_WriteDeviceConfig();
    }
}
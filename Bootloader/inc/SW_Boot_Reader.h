#ifndef _SW_BOOT_FRAM_
#define _SW_BOOT_FRAM_

#include "SW_FRam.h"


#define FRAM_BOOT_APP_OK                    0x5555
#define FRAM_BOOT_APP_FAIL                  0xAAAA

#define READER_BAUDRATE9600             5
#define READER_BAUDRATE38400            7
#define READER_BAUDRATE115200           11

struct CONTROL_LOW_BIT
{
    //0101:9600
    //0111:38400
    //1011:115200
    u8 baudRate:4;

    //0:quiet
    //1:no quiet
    u8 tagQuiet:1;

    //0:active
    //1:positive
    u8 uidTransMode:1;

    //0:enable afi
    //1:disable afi
    u8 afiControl:1;

    //0:auto
    //1:trigger
    u8 inventoryMode:1;
};

//
struct CONTROL_HIGH_BIT
{
    //0:disable
    //1:enable
    u8 buzzer:1;
    //
    u8 rfu:7;
};

#define READER_DISH_NUM             10
#define READER_DISH_NAME_LEN        20
#define READER_DISH_PRICE_LEN       6
#define READER_DISH_SN_LEN          3
typedef struct readerDish{
    u8 price[READER_DISH_PRICE_LEN + 1];
    u8 name[READER_DISH_NAME_LEN + 1];
    u8 position;
}READER_DISH;

#define READER_DEVICE_PAR_START         (0x0800F000) 
#define READER_TYPE_ISO15693            0x00
#define READER_TYPE_ISO14443A           0x10
#define READER_TYPE_MASK                0xF0

#define READER_MODE_INVENTORY           0x02
#define READER_MODE_EAS                 0x01
//
#define READER_AFI_DISABLE              0
#define READER_AFI_ENABLE               1

#define READER_TAG_QUIET                0
#define READER_TAG_NOQUIET              1

//
#define READER_INVENTORY_AUTO           0
#define READER_INVENTORY_TRIGGER        1

//
#define READER_BUZZER_ENABLE            1
#define READER_BUZZER_DISABLE           0

#define READER_RF_CLOSE                 0
#define READER_RF_OPEN                  1
#define READER_RF_RESET                 2

#define READER_UID_ACTIVE               0
#define READER_UID_POSITIVE             1
typedef struct flashDevicePar{
    //1:EAS,for user
    //2:Inventory,for user
    u8 workMode;

    u16 addr;

    struct CONTROL_LOW_BIT controlLow;
    struct CONTROL_HIGH_BIT controlHigh;

    u8 afiValue;

    //0:enable
    //1:disable
    u8 queryFlag;

    //0:close
    //1:open
    u8 rfCtrl;

    u8 dishNum;
    u8 curDishIndex;
    READER_DISH dish[READER_DISH_NUM];
    u16 appState;
    u16 crc;
}READER_DEVICE_PARAMETER;

extern READER_DEVICE_PARAMETER g_sDeviceParamenter;

//#define FRAME_MEM_SIZE                      8192    //8k
//#define FRAME_BOOT_INFO_ADDR                (FRAME_MEM_SIZE - sizeof(FRAM_BOOTPARAMS))      //固件升级从480字节开始写，共512字节
//#define FRAME_SYS_INFO_ADDR                 0x00

void Reader_ReadDeviceConfig(void);
BOOL Reader_WriteDeviceConfig(void);
u32 Fram_GetUartBaudrate(void);
#endif


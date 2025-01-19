#ifndef _SW_R0001_DEVICE_
#define _SW_R0001_DEVICE_

#include "SW_R0001_FRam.h"
#include "SW_R0001_Uart.h"
#include "SW_R0001_R485.h"
#include "SW_R0001_WDG_HL.h"
#include "SW_R0001_SysTick_HL.h"

#include "SW_FM17xx_ISO14443A.h"
extern ISO14443A_UID g_sReaderISO14443AUid;

extern u32 g_nReaderDishNum;

#define READER_RSPFRAME_LEN             256
#define READER_COM_UART                 0x00
typedef struct readerRspFrame{
    u8 len;
    u8 buffer[READER_RSPFRAME_LEN];
    u8 flag;
    u8 err;
    u16 destAddr;
    u8 cmd;
    u8 com;
}READER_RSPFRAME;
extern READER_RSPFRAME g_sReaderRspFrame;

extern u32 g_nReaderDelay;
//注意编译代码的时候，代码区和这个参数区有重合
#define READER_DEVICE_PAR_START         (0x0800F000)

#define READER_TYPE_ISO15693            0x00
#define READER_TYPE_ISO14443A           0x10
#define READER_TYPE_MASK                0xF0

#define READER_MODE_INVENTORY           0x02
#define READER_MODE_EAS                 0x01

#define READER_BAUDRATE9600             5
#define READER_BAUDRATE38400            7
#define READER_BAUDRATE115200           11
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

//"R323xyz  SV:mmnnll  HV:mmnnll"R323C SV:000101
//#define READER_VERSION              "R323 20030200 191200"
#define READER_VERSION_SIZE         50

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
    
    u16 crc;
}READER_DEVICE_PARAMETER;

extern READER_DEVICE_PARAMETER g_sDeviceParamenter;

#define READER_BOOT_APP_OK                    0x5555
#define READER_BOOT_APP_FAIL                  0xAAAA


#define READER_RSPFRAME_FLAG_OK             0x00
#define READER_RSPFRAME_FLAG_FAIL           0x01
#define READER_OPTAG_RESPONSE_NOERR         0x00
#define READER_OPTAG_RESPONSE_TAGERR        0x01
#define READER_OPTAG_RESPONSE_CRCERR        0x02
#define READER_OPTAG_RESPONSE_NORSP         0x03
#define READER_OPTAG_RESPONSE_PARERR        0x04

#define READER_FRAME_MIN_LEN                11
#define READER_RFRAME_MIN_LEN               14
#define READER_RFRAME_DIROP_MIN_LEN         22

#define READER_RFRAME_POS_HEAD1             0
#define READER_RFRAME_POS_HEAD2             1
#define READER_RFRAME_POS_LEN               2
#define READER_RFRAME_POS_SRCADDR           3
#define READER_RFRAME_POS_DESTADDR          5
#define READER_RFRAME_POS_FLAG              7
#define READER_RFRAME_POS_CMD               8
#define READER_RFRAME_POS_RFU               9
#define READER_RFRAME_POS_PAR               10


#define Reader_GetFrameLength(p)        (((p)[READER_FRAME_POS_LEN]) + 3)
#define Reader_GetFrameCrc(p, len)      (*((u16 *)(p + len - 2)))

#define READER_FRAME_RESPONSE_FLAG          0x1F
#define READER_FRAME_PARAM_RFU              0x00


#define READER_CMD_RESET                    0x04
#define READER_CMD_RF_CTRL                  0xF0
#define READER_CMD_SET_CFG                  0xF4
#define READER_CMD_GET_CFG                  0xF5
#define READER_CMD_GET_VERSION              0xF7
#define READER_CMD_GET_CPUID                0xF8

//ISO15693
#define RISO15693_CMD_READ_UID              0x11
#define RISO15693_CMD_READ_MBLOCK           0x22
#define RISO15693_CMD_WRITE_MBLOCK          0x23
#define RISO15693_CMD_LOCK_BLOCK            0x2C
#define RISO15693_CMD_WRITE_AFI             0x24
#define RISO15693_CMD_LOCK_AFI              0x25
#define RISO15693_CMD_WRITE_DSFID           0x26
#define RISO15693_CMD_LOCK_DSFID            0x27
#define RISO15693_CMD_GETINFO               0x28
#define RISO15693_CMD_DTU                   0x2F
#define RISO15693_CMD_DIROP                 0x30
#define RISO15693_CMD_DIRWB                 0x31

//获取UID
#define RISO14443A_CMD_GET_UID              0x16
//ISO14443-3
#define RISO14443A_CMD_AUTHRB               0x70
#define RISO14443A_CMD_AUTHWB               0x71
#define RISO14443A_CMD_AUTHRV               0x72
#define RISO14443A_CMD_AUTHWV               0x73
#define RISO14443A_CMD_AUTHV                0x74
#define RISO14443A_CMD_RM0BLOCK             0x75
#define RISO14443A_CMD_WM0BLOCK             0x76
#define RISO14443A_CMD_RATS                 0x77
#define RISO14443A_CMD_PCTRLESAM            0x78        //ESAM控制
#define RISO14443A_CMD_PPS                  0x79
#define RISO14443A_CMD_APDU                 0x7A
#define RISO14443A_CMD_HALT                 0x7B
#define RISO14443A_CMD_DESEL                0x7C
#define RISO14443A_CMD_SELESAMBR            0x7D
#define RISO14443A_CMD_DTU                  0x7F

#define RISO14443A_CMD_AUTHM0U2             0x80  //MIFARE Ultralight C 3DES + CBC

#define READER_FRAME_BROADCAST_ADDR         0xFFFF

#define READER_IM_MODE_UID                  0x01    //读UID
#define READER_IM_MODE_RBLOCK               0x03    //读UID+BLOCK
#define READER_IM_BLOCK_OLD_NUM             4
#define READER_IM_BLOCK_NUM                 ISO15693_OPBLOCK_MAX_NUM
#define READER_IM_OP_OK                     1
#define READER_IM_OP_FAIL                   0
#define READER_OP_UID_NUM                   1
/*typedef struct readerImParams{
    u8 mode;
    u8 uidNum;
    u8 uid[ISO15693_SIZE_UID];
    u8 blockNum;
    u8 blockAddr[READER_IM_BLOCK_NUM];
    u8 block[READER_IM_BLOCK_NUM][ISO15693_SIZE_BLOCK];
    u8 result[READER_IM_BLOCK_NUM];
}READER_IMPARAMS;
extern READER_IMPARAMS g_sReaderImParams;*/

void Reader_Init(void);
void Reader_ReadDeviceConfig(void);
BOOL Reader_WriteDeviceConfig(void);
u32 Reader_GetUartBaudrate(void);

#define READER_SAM_FRAME_BYTE_MAX          256
typedef struct readerSamFrame{
    u8 rxFrame[READER_SAM_FRAME_BYTE_MAX];
    u8 rxLen;
    u8 txFrame[READER_SAM_FRAME_BYTE_MAX];
    u8 txLen;
}READER_SAMFRANME;
extern READER_SAMFRANME g_sReaderSamFrame;

u8 Reader_ResponseFrame(u8 *pParam, u8 len, READER_RSPFRAME *pOpResult);
u8 Reader_GetCfgRspFrame(READER_RSPFRAME *pOpResult);
//u8 RISO15693_GetUidRspFrame(u8 *pUid, u8 num, READER_RSPFRAME *pOpResult);
//u8 RISO15693_GetTagInfoRspFrame(u8 *pParam, u8 len, READER_RSPFRAME *pOpResult);

u8 Reader_GetErrorType(u8 err);
u8 Reader_AutoUid(void);
//u8 RISO15693_DirOpRspFrame(READER_IMPARAMS *pImInfo, READER_RSPFRAME *pOpResult);
//u8 RISO15693_DirWriteTagRspFrame(READER_IMPARAMS *pImInfo, READER_RSPFRAME *pOpResult);
//u8 Reader_ProcessFrame(u8 *pFrame, u8 len);








  
    
    
    
    


















//----------------------Params-----------------//

#define DEVICE_VERSION_SIZE                            50

#define DEVICE_APP_PAR_START                    (0x0800F800)
#define DEVICE_APPPAR_BACK_START                (0x0800F000)

#define DEVICE_BOOT_APP_OK                      0x5555
#define DEVICE_BOOT_APP_FAIL                    0xAAAA
#define DEVICE_BOOT_APP_NONE                    0xFFFF
typedef struct deviceBootDevicePar{
    u16 appState;
    u16 addr;
    u32 br;
    u32 rfu;
    u32 crc;
}DEVICE_BOOTPARAMS;
extern DEVICE_BOOTPARAMS g_sDeviceBootParamenter;

BOOL Device_ReadBootParamenter(u32 addr);
BOOL Device_WriteBootParamenter(u32 addr);

#define DEVICE_PAR_START                    (0x0800E800)
#define DEVICE_PAR_BACK_START               (0x0800E000)

#define DEVICE_PAR_AUTH_BLOCK_NUM               2
#define DEVICE_PAR_AUTH_BLOCK_DATE_LEN          8
typedef struct deviceParams{
    u8 workMode;
    u16 addr;
    u8 blockAddr;
    u8 blockDate[DEVICE_PAR_AUTH_BLOCK_NUM][ISO14443A_M1BLOCK_LEN];
    u8 key[ISO14443A_M1_KEY_LEN];
    u16 crc;
}DEVICE_PARAMS;
extern DEVICE_PARAMS g_sDeviceParams;
BOOL Device_WriteParamenters(u32 addr);
BOOL Device_ReadParamenters(u32 addr); 

//------------------------------------IO-------------------------
extern const PORT_INF EXPORT_CH1_PORT;
#define Device_ExportCh1High()         EXPORT_CH1_PORT.Port->BSRR = EXPORT_CH1_PORT.Pin
#define Device_ExportCh1Low()          EXPORT_CH1_PORT.Port->BRR = EXPORT_CH1_PORT.Pin

extern const PORT_INF EXPORT_CH2_PORT;
#define Device_ExportCh2High()         EXPORT_CH2_PORT.Port->BSRR = EXPORT_CH2_PORT.Pin
#define Device_ExportCh2Low()          EXPORT_CH2_PORT.Port->BRR = EXPORT_CH2_PORT.Pin

//---------------------------------------//
#define DEVICE_ISO15693_OP_SIZE_BLOCK                   8
#define DEVICE_ISO15693_OP_BLOCK_MAX_NUM                3

#define DEVICE_IM_OP_NUM                                4
#define DEVICE_OP_MAX_NUM                               8

typedef struct deviceImParams{
    u8 mode;
    u8 blockNum;
    u8 blockAddr[DEVICE_IM_OP_NUM];
    u8 blockDate[DEVICE_PAR_AUTH_BLOCK_NUM][ISO14443A_M1BLOCK_LEN];
    u8 key[ISO14443A_M1_KEY_LEN];
}DEVICE_IMPARAMS;

#define DEVICE_OP_INVENTORY                 1
#define DEVICE_OP_ANTISHAKE                 2
#define DEVICE_OP_AUTH                      3
#define DEVICE_OP_READ                      4
#define DEVICE_OP_PROOF                     5

#define DEVICE_STAT_IDLE                    0x00000001
#define DEVICE_STAT_TX                      0x00000002
#define DEVICE_STAT_WAIT                    0x00000004
#define DEVICE_STAT_RX                      0x00000008
#define DEVICE_STAT_STEP                    0x00000010
#define DEVICE_STAT_DELAY                   0x00000040
    
#define DEVICE_RESULT_OK                    0x00
#define DEVICE_RESULT_ERR                   0x01

#define DEVICE_OP_DELAY_TICK                60

typedef struct deviceOperation{
    DEVICE_IMPARAMS imParams;
    u32 antiShakeTick;
    u32 delay;
    u32 state;
    u8 tagNum;
    ISO14443A_UID tag;
    u8 block[ISO14443A_M1BLOCK_LEN];
    u8 op[DEVICE_OP_MAX_NUM];
    u8 step;
    u8 num;
    u8 index;
    u8 cmd;
    u8 proofFlag;
    BOOL bRepeatTag;
    BOOL bProof;
    u8 rlt;
}DEVICE_OP;
extern DEVICE_OP g_sDeviceOp;
    
#define Device_ResetOp()         do{\
                                        g_sDeviceOp.state = DEVICE_STAT_IDLE;\
                                        g_sDeviceOp.index = 0;\
                                        g_sDeviceOp.num = 0;\
                                        g_sDeviceOp.step = 0;\
                                        g_sDeviceOp.bRepeatTag = FALSE;\
                                        g_sDeviceOp.bProof = FALSE;\
                                        g_sDeviceOp.tagNum = 0; \
                                        g_sDeviceOp.cmd = 0;\
                                    }while(0)
    
#define DEVICE_OP_TAG_REPEAT            3
typedef struct deviceTagInfo{
    ISO14443A_UID okTag;
    BOOL bTagOk;
    u32 opOkTick;
    
    ISO14443A_UID tag;
    
    u32 noSwitchDishTagTimer;
    u8 bSwitchDishTag;
    u8 switchMode;
    u16 switchDishCode;
    
    ISO14443A_UID shakeTag;
    u8 noShakeUidTimers;
    u8 bAntiShake;
    u32 antiShakeTick;

    u8 rbIndex;
    u8 wbIndex;
    u8 repeat;
    u16 wSofEnd;
}DEVICE_OPTAGINFO;
extern DEVICE_OPTAGINFO g_sDeviceOpTagInfo;
    
#define Device_ResetOpTagInfo()     do{\
                                        g_sDeviceOpTagInfo.rbIndex = 0;\
                                        g_sDeviceOpTagInfo.wbIndex = 0;\
                                        g_sDeviceOpTagInfo.repeat = 0;\
                                        }while(0)

                                          
void Device_AutoTask();
BOOL Device_Transm(DEVICE_OP *pOpInfo);
u8 Device_Receive(DEVICE_OP *pOpInfo);
BOOL Device_Step(DEVICE_OP *pOpInfo);



#define DEVICE_RSPFRAME_LEN             256

typedef struct deviceRspFrame{
    u8 len;
    u8 buffer[DEVICE_RSPFRAME_LEN];
    u8 flag;
    u8 err;
    u16 destAddr;
    u8 cmd;
    u8 com;
}DEVICE_RSPFRAME;
extern DEVICE_RSPFRAME g_sDevicerRspFrame;

u16 Device_ProcessUartFrame(u8 *pFrame, u16 len);
u16 Device_ResponseFrame(u8 *pParam, u16 len, DEVICE_RSPFRAME *pRspFrame);
void Device_Init(void);
void Device_Delayms(u32 n);



#define Device_ClearOkTag()         {memset(&g_sDeviceOpTagInfo.okTag, 0, sizeof(ISO14443A_UID)); g_sDeviceOpTagInfo.bTagOk = FALSE; }while(0)
void Device_CheckRemoveokTag(DEVICE_OP *pOpInfo, DEVICE_OPTAGINFO *pOpTagInfo);


#define DEVICE_EXPORT_CH1_AMAR                                  0x01
#define DEVICE_EXPORT_CH2_AMAR                                  0x02

void Device_MonitorExport(u8 status);

extern u8 Device_ExportSatus;
#endif

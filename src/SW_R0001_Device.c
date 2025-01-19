#include "SW_R0001_Device.h"

u32 g_nReaderDelay = 0;
READER_SAMFRANME g_sReaderSamFrame = {0};
READER_DEVICE_PARAMETER g_sDeviceParamenter = {0};
READER_RSPFRAME g_sReaderRspFrame = {0};
ISO14443A_UID g_sReaderISO14443AUid = {0};




u8 Reader_GetCfgRspFrame(READER_RSPFRAME *pOpResult)
{
    u8 pos = 0;
    u16 crc = 0;

    pOpResult->buffer[pos++] = 0x7E; // frame head first byte
    pOpResult->buffer[pos++] = 0x55; // frame haed second byte
    pOpResult->buffer[pos++] = 0xFF;   // length
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >> 0) & 0xff;
    pOpResult->buffer[pos++] = (pOpResult->destAddr >>8) & 0xFF;
    pOpResult->buffer[pos++] = READER_FRAME_RESPONSE_FLAG;
    pOpResult->buffer[pos++] = pOpResult->cmd;               // cmd
    pOpResult->buffer[pos++] = READER_FRAME_PARAM_RFU;     // RFU

    pOpResult->buffer[pos++] = g_sDeviceParamenter.workMode;
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 0) & 0xFF;
    pOpResult->buffer[pos++] = (g_sDeviceParamenter.addr >> 8) & 0xFF;
    pOpResult->buffer[pos++] = *((u8 *)(&(g_sDeviceParamenter.controlLow)));
    pOpResult->buffer[pos++] = *((u8 *)(&(g_sDeviceParamenter.controlHigh)));
    pOpResult->buffer[pos++] = g_sDeviceParamenter.afiValue;

    pOpResult->buffer[2] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节

    crc = a_GetCrc(pOpResult->buffer + 2, pos - 2); //从LEN开始计算crc
    pOpResult->buffer[pos++] = crc & 0xFF;
    pOpResult->buffer[pos++] = (crc >> 8) & 0xFF;

    g_sReaderRspFrame.len = pos;

    return pos;
}

u8 RISO14443A_GetUidRspFrame(ISO14443A_UID *pISO14443AUid, READER_RSPFRAME *pOpResult)
{
    u8 pos = 0;
    u16 crc = 0;

    g_sReaderRspFrame.buffer[pos++] = 0x7E; // frame head first byte
    g_sReaderRspFrame.buffer[pos++] = 0x55; // frame haed second byte
    g_sReaderRspFrame.buffer[pos++] = 0xFF;   // length
    g_sReaderRspFrame.buffer[pos++] = (g_sDeviceParamenter.addr >> 0) & 0xFF;
    g_sReaderRspFrame.buffer[pos++] = (g_sDeviceParamenter.addr >> 8) & 0xFF;
    g_sReaderRspFrame.buffer[pos++] = (pOpResult->destAddr >> 0) & 0xff;
    g_sReaderRspFrame.buffer[pos++] = (pOpResult->destAddr >>8) & 0xFF;
    g_sReaderRspFrame.buffer[pos++] = READER_FRAME_RESPONSE_FLAG;
    g_sReaderRspFrame.buffer[pos++] = pOpResult->cmd;               // cmd
    g_sReaderRspFrame.buffer[pos++] = READER_FRAME_PARAM_RFU;     // RFU

    if(pOpResult->flag == READER_RSPFRAME_FLAG_OK)
    {
        g_sReaderRspFrame.buffer[pos++] = (pISO14443AUid->type >> 0) & 0xFF;
        g_sReaderRspFrame.buffer[pos++] = (pISO14443AUid->type >> 8) & 0xFF; 
        g_sReaderRspFrame.buffer[pos++] = pISO14443AUid->len;
        a_memcpy(pISO14443AUid->uid, g_sReaderRspFrame.buffer + pos, ISO14443A_TRIPLE_UID_LEN);
        pos += ISO14443A_TRIPLE_UID_LEN;
        g_sReaderRspFrame.buffer[pos++] = pISO14443AUid->sak;
    }
    g_sReaderRspFrame.buffer[pos++] = READER_FRAME_PARAM_RFU;
    g_sReaderRspFrame.buffer[pos++] = READER_FRAME_PARAM_RFU;
    
    g_sReaderRspFrame.buffer[2] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节
    
    crc = a_GetCrc(g_sReaderRspFrame.buffer + 2, pos - 2); //从LEN开始计算crc
    g_sReaderRspFrame.buffer[pos++] = crc & 0xFF;
    g_sReaderRspFrame.buffer[pos++] = (crc >> 8) & 0xFF;

    g_sReaderRspFrame.len = pos;

    return pos;
}

/*


u8 g_aTempBuffer[UART_BUFFER_MAX_LEN] = {0};
u8 Reader_ProcessFrame(u8 *pFrame, u8 len)
{
    u8 cmd = 0;
    u16 destAddr = 0;
    u8 temp = 0;
    u8 state = FM17XX_STAT_ERROR;
    u8 paramsLen = 0;

    cmd = *(pFrame + UART_FRAME_POS_CMD);
    destAddr = *((u16 *)(pFrame + UART_FRAME_POS_DESTADDR));
    if((destAddr != READER_FRAME_BROADCAST_ADDR) && (destAddr != g_sDeviceParamenter.addr))
    {
        return 0;
    }

    g_sReaderRspFrame.destAddr = *((u16 *)(pFrame + UART_FRAME_POS_SRCADDR));
    g_sReaderRspFrame.cmd = cmd;
    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NORSP;
    g_sReaderRspFrame.len = 0;

    paramsLen = len - READER_FRAME_MIN_LEN;
    switch(cmd)
    {
        case RISO14443A_CMD_GET_UID:
            a_memset(&g_sReaderISO14443AUid, 0, sizeof(g_sReaderISO14443AUid));
            if(paramsLen == 1 && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {                
                g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_TAGERR;
                temp = pFrame[UART_FRAME_POS_PAR];
                if(temp == 0x00)
                {
                    state = ISO14443A_GetUid(&g_sReaderISO14443AUid, ISO14443A_CMD_REQIDL);
                }
                else
                {
                    state = ISO14443A_GetUid(&g_sReaderISO14443AUid, ISO14443A_CMD_REQALL);
                }
                if(state == RC663_STAT_OK)
                {
                    if(g_sDeviceParamenter.controlLow.tagQuiet == READER_TAG_QUIET)
                    {
                        ISO14443A_Halt();
                    }
                    
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                }
                else
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                    g_sReaderRspFrame.err = Reader_GetErrorType(state);
                }
                g_sReaderRspFrame.len = RISO14443A_GetUidRspFrame(&g_sReaderISO14443AUid, &g_sReaderRspFrame);
            }  
            break;
        case RISO14443A_CMD_AUTHRB:
            //KeyType key addr num
            if(paramsLen == (1 + ISO14443A_M1_KEY_LEN + 1 + 1) && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                u8 keyType = 0;
                u8 key[ISO14443A_M1_KEY_LEN] = {0};
                u8 blockAddr = 0;
                u8 block[ISO14443A_M1BLOCK_LEN] = {0};

                keyType = pFrame[UART_FRAME_POS_PAR];
                a_memcpy(pFrame + UART_FRAME_POS_PAR + 1, key, ISO14443A_M1_KEY_LEN);
                blockAddr = pFrame[UART_FRAME_POS_PAR + 1 + ISO14443A_M1_KEY_LEN];
                
                if(keyType == ISO14443A_CMD_AUTHENT_A || keyType == ISO14443A_CMD_AUTHENT_B)
                {
                    if(g_sReaderISO14443AUid.len > ISO14443A_SIGNAL_UID_LEN)
                    {
                        state = ISO14443A_AuthM1(g_sReaderISO14443AUid.uid + 3, keyType, key, blockAddr);
                    }
                    else
                    {
                        state = ISO14443A_AuthM1(g_sReaderISO14443AUid.uid, keyType, key, blockAddr);
                    }
                    
                    if(state == RC663_STAT_OK)
                    {
                        state = ISO14443A_ReadMifareBlock(blockAddr, block);
                        if(state == RC663_STAT_OK)
                        {
                            g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                            g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                            g_sReaderRspFrame.len = Reader_ResponseFrame(block, ISO14443A_M1BLOCK_LEN, &g_sReaderRspFrame);
                        }
                    }

                    if(state != RC663_STAT_OK)
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                        g_sReaderRspFrame.err = Reader_GetErrorType(state);
                        g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                    }
                }
            }  
            break;
        case RISO14443A_CMD_AUTHWB:
            //KeyType key addr num block
            if(paramsLen == (1 + ISO14443A_M1_KEY_LEN + 1 + 1 + ISO14443A_M1BLOCK_LEN) && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                u8 keyType = 0;
                u8 key[ISO14443A_M1_KEY_LEN] = {0};
                u8 blockAddr = 0;
                u8 block[ISO14443A_M1BLOCK_LEN] = {0};

                keyType = pFrame[UART_FRAME_POS_PAR];
                a_memcpy(pFrame + UART_FRAME_POS_PAR + 1, key, ISO14443A_M1_KEY_LEN);
                blockAddr = pFrame[UART_FRAME_POS_PAR + 1 + ISO14443A_M1_KEY_LEN];
                a_memcpy(pFrame + UART_FRAME_POS_PAR + 1 + ISO14443A_M1_KEY_LEN + 2, block, ISO14443A_M1BLOCK_LEN);
                if(keyType == ISO14443A_CMD_AUTHENT_A || keyType == ISO14443A_CMD_AUTHENT_B)
                {
                    if(g_sReaderISO14443AUid.len > ISO14443A_SIGNAL_UID_LEN)
                    {
                        state = ISO14443A_AuthM1(g_sReaderISO14443AUid.uid + 3, keyType, key, blockAddr);
                    }
                    else
                    {
                        state = ISO14443A_AuthM1(g_sReaderISO14443AUid.uid, keyType, key, blockAddr);
                    }
                    
                    if(state == RC663_STAT_OK)
                    {
                        state = ISO14443A_WriteMifareBlock16(blockAddr, block);
                        if(state == RC663_STAT_OK)
                        {
                            g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                            g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                            g_sReaderRspFrame.len = Reader_ResponseFrame(0, 0, &g_sReaderRspFrame);
                        }
                    }

                    if(state != RC663_STAT_OK)
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                        g_sReaderRspFrame.err = Reader_GetErrorType(state);
                        g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                    }
                }
            }  
            break;
        case RISO14443A_CMD_AUTHRV:
            //KeyType key addr
            if(paramsLen == (1 + ISO14443A_M1_KEY_LEN + 1) && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                u8 keyType = 0;
                u8 key[ISO14443A_M1_KEY_LEN] = {0};
                u8 blockAddr = 0;
                u8 block[ISO14443A_M1BLOCK_LEN] = {0};

                keyType = pFrame[UART_FRAME_POS_PAR];
                a_memcpy(pFrame + UART_FRAME_POS_PAR + 1, key, ISO14443A_M1_KEY_LEN);
                blockAddr = pFrame[UART_FRAME_POS_PAR + 1 + ISO14443A_M1_KEY_LEN];
                if(keyType == ISO14443A_CMD_AUTHENT_A || keyType == ISO14443A_CMD_AUTHENT_B)
                {                    
                    if(g_sReaderISO14443AUid.len > ISO14443A_SIGNAL_UID_LEN)
                    {
                        state = ISO14443A_AuthM1(g_sReaderISO14443AUid.uid + 3, keyType, key, blockAddr);
                    }
                    else
                    {
                        state = ISO14443A_AuthM1(g_sReaderISO14443AUid.uid, keyType, key, blockAddr);
                    }
                    
                    if(state == RC663_STAT_OK)
                    {
                        state = ISO14443A_ReadMifareBlock(blockAddr, block);
                        if(state == RC663_STAT_OK)
                        {
                            if(ISO14443A_CheckValueFormat(block) == RC663_STAT_OK)
                            {
                                block[ISO14443A_M1VALUE_LEN] = block[3 * ISO14443A_M1VALUE_LEN];
                                g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                                g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                                g_sReaderRspFrame.len = Reader_ResponseFrame(block, ISO14443A_M1VALUE_LEN + 1, &g_sReaderRspFrame);
                            }
                            else
                            {
                                g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                                g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_TAGERR;
                                g_sReaderRspFrame.len = Reader_ResponseFrame(block, ISO14443A_M1VALUE_LEN + 1, &g_sReaderRspFrame);
                            }
                        }
                    }

                    if(state != RC663_STAT_OK)
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                        g_sReaderRspFrame.err = Reader_GetErrorType(state);
                        g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                    }
                }
            }  
            break;
        case RISO14443A_CMD_AUTHWV:
            //KeyType key addr value transaddr
            if(paramsLen == (1 + ISO14443A_M1_KEY_LEN + 1 + ISO14443A_M1VALUE_LEN + 1) && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                u8 keyType = 0;
                u8 key[ISO14443A_M1_KEY_LEN] = {0};
                u8 blockAddr = 0;
                int value = 0;
                u8 backupAddr = 0;

                keyType = pFrame[UART_FRAME_POS_PAR];
                memcpy(key, pFrame + UART_FRAME_POS_PAR + 1, ISO14443A_M1_KEY_LEN);
                blockAddr = pFrame[UART_FRAME_POS_PAR + 1 + ISO14443A_M1_KEY_LEN];
                memcpy(&value, pFrame + UART_FRAME_POS_PAR + 2 + ISO14443A_M1_KEY_LEN, ISO14443A_M1VALUE_LEN);
                backupAddr = pFrame[UART_FRAME_POS_PAR + 2 + ISO14443A_M1_KEY_LEN + ISO14443A_M1VALUE_LEN];

                ISO14443A_FormatValue(backupAddr, value, g_aTempBuffer);
                if(keyType == ISO14443A_CMD_AUTHENT_A || keyType == ISO14443A_CMD_AUTHENT_B)
                {                    
                    if(g_sReaderISO14443AUid.len > ISO14443A_SIGNAL_UID_LEN)
                    {
                        state = ISO14443A_AuthM1(g_sReaderISO14443AUid.uid + 3, keyType, key, blockAddr);
                    }
                    else
                    {
                        state = ISO14443A_AuthM1(g_sReaderISO14443AUid.uid, keyType, key, blockAddr);
                    }
                    
                    if(state == RC663_STAT_OK)
                    {
                        state = ISO14443A_WriteMifareBlock16(blockAddr, g_aTempBuffer);
                        if(state == RC663_STAT_OK)
                        {
                            g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                            g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                            g_sReaderRspFrame.len = Reader_ResponseFrame(0, 0, &g_sReaderRspFrame);
                        }
                    }

                    if(state != RC663_STAT_OK)
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                        g_sReaderRspFrame.err = Reader_GetErrorType(state);
                        g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                    }
                }
            }  
            break;
        case RISO14443A_CMD_AUTHV:
            //KeyType key addr value opcode transaddr
            if(paramsLen == (1 + ISO14443A_M1_KEY_LEN + 1 + ISO14443A_M1VALUE_LEN + 1 + 1) && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                u8 keyType = 0;
                u8 key[ISO14443A_M1_KEY_LEN] = {0};
                u8 blockAddr = 0;
                int value = 0;
                u8 transAddr = 0;
                u8 opCode = 0;

                keyType = pFrame[UART_FRAME_POS_PAR];
                a_memcpy(pFrame + UART_FRAME_POS_PAR + 1, key, ISO14443A_M1_KEY_LEN);
                blockAddr = pFrame[UART_FRAME_POS_PAR + 1 + ISO14443A_M1_KEY_LEN];
                a_memcpy(pFrame + UART_FRAME_POS_PAR + 1 + ISO14443A_M1_KEY_LEN + 1, &value, ISO14443A_M1VALUE_LEN);
                opCode = pFrame[UART_FRAME_POS_PAR + 1 + ISO14443A_M1_KEY_LEN + 1 + ISO14443A_M1VALUE_LEN];
                transAddr = pFrame[UART_FRAME_POS_PAR + 1 + ISO14443A_M1_KEY_LEN + 1 + ISO14443A_M1VALUE_LEN + 1];
                
                if((keyType == ISO14443A_CMD_AUTHENT_A || keyType == ISO14443A_CMD_AUTHENT_B) &&
                   (opCode == ISO14443A_CMD_DECREMENT || opCode == ISO14443A_CMD_INCREMENT || opCode == ISO14443A_CMD_RESTORE))
                {                    
                    if(g_sReaderISO14443AUid.len > ISO14443A_SIGNAL_UID_LEN)
                    {
                        state = ISO14443A_AuthM1(g_sReaderISO14443AUid.uid + 3, keyType, key, blockAddr);
                    }
                    else
                    {
                        state = ISO14443A_AuthM1(g_sReaderISO14443AUid.uid, keyType, key, blockAddr);
                    }
                    
                    if(state == RC663_STAT_OK)
                    {
                        state = ISO14443A_Value(opCode, (u8 *)(&value), transAddr, blockAddr);
                        if(state == RC663_STAT_OK)
                        {
                            g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                            g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                            g_sReaderRspFrame.len = Reader_ResponseFrame(0, 0, &g_sReaderRspFrame);
                        }
                    }

                    if(state != RC663_STAT_OK)
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                        g_sReaderRspFrame.err = Reader_GetErrorType(state);
                        g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                    }
                }
            }  
            break;
        case RISO14443A_CMD_RM0BLOCK:
            //keyType [key] addr num
            if(paramsLen == (1 + 1 + 1) || paramsLen == (1 + ISO14443A_M0KEY_LEN + 1 + 1) && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                u8 keyType = 0;
                u8 key[ISO14443A_M0KEY_LEN] = {0};
                u8 blockAddr = 0;
                u8 blockNum = 0;
                u8 block[ISO14443A_MOBLOCKNUM_MAX * ISO14443A_M0BLOCK_LEN] = {0};
                u8 pos = 0;
                
                pos = UART_FRAME_POS_PAR;

                keyType = pFrame[pos++];
                if(keyType)
                {
                    a_memcpy(pFrame + pos, key, ISO14443A_M0KEY_LEN);
                    pos += ISO14443A_M0KEY_LEN;
                }
                blockAddr = pFrame[pos++];
                blockNum = pFrame[pos++];
                
                state = RC663_STAT_OK;
                //不处理NTAG的密码模式
                if(keyType)
                {
                    //u8 pck[MIFARE_M0PCK_LEN] = {0};
                    //error = MIFARE_AuthM0(key, pck);
                }

                if(state == RC663_STAT_OK)
                {
                    u8 i = 0;
                    u8 m1BlockNum = 0;
                    m1BlockNum = blockNum >> 2;
                    if(blockNum & 0x03)
                    {
                        m1BlockNum++;
                    }

                    for(i = 0; i < m1BlockNum; i++)
                    {
                        state = ISO14443A_ReadMifareBlock(blockAddr + i * 4, block + i * ISO14443A_M1BLOCK_LEN);
                        if(state != RC663_STAT_OK)
                        {
                            break;
                        }
                    }
                }
                
                if(state == RC663_STAT_OK)
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                    g_sReaderRspFrame.len = Reader_ResponseFrame(block, blockNum * ISO14443A_M0BLOCK_LEN, &g_sReaderRspFrame);
                }
                else
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                    g_sReaderRspFrame.err = Reader_GetErrorType(state);
                    g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                }
            }  
            break;
        case RISO14443A_CMD_WM0BLOCK:
            if(paramsLen >= (1 + 1 + 1 + ISO14443A_M0BLOCK_LEN) && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                u8 keyType = 0;
                u8 key[ISO14443A_M0KEY_LEN] = {0};
                u8 blockAddr = 0;
                u8 blockNum = 0;
                u8 block[ISO14443A_MOBLOCKNUM_MAX * ISO14443A_M0BLOCK_LEN] = {0};
                u8 pos = 0;
                
                pos = UART_FRAME_POS_PAR;

                keyType = pFrame[pos++];
                if(keyType)
                {
                    a_memcpy(pFrame + pos, key, ISO14443A_M0KEY_LEN);
                    pos += ISO14443A_M0KEY_LEN;
                }
                blockAddr = pFrame[pos++];
                blockNum = pFrame[pos++];
                a_memcpy(pFrame + pos, block, ISO14443A_M0BLOCK_LEN * blockNum);
                pos += ISO14443A_M0BLOCK_LEN * blockNum;
                
                state = RC663_STAT_OK;
                //不处理NTAG的密码模式
                if(keyType)
                {
                    //u8 pck[MIFARE_M0PCK_LEN] = {0};
                    //error = MIFARE_AuthM0(key, pck);
                }

                if(state == RC663_STAT_OK)
                {
                    u8 i = 0;
              
                    for(i = 0; i < blockNum; i++)
                    {
                        state = ISO14443A_WriteMifareBlock4(blockAddr + i, block + i * ISO14443A_M0BLOCK_LEN);
                        if(state != RC663_STAT_OK)
                        {
                            break;
                        }
                    }
                }
                if(state == RC663_STAT_OK)
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                    g_sReaderRspFrame.len = Reader_ResponseFrame(0, 0, &g_sReaderRspFrame);
                }
                else
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                    g_sReaderRspFrame.err = Reader_GetErrorType(state);
                    g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                }
            }  
            break;
        case RISO14443A_CMD_RATS:
            if(paramsLen == 0 && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                u8 len = 0;

                state = ISO14443A_Rats(g_aTempBuffer, &len);
                if(state == RC663_STAT_OK)
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                    g_sReaderRspFrame.len = Reader_ResponseFrame(g_sRc663Frame.frame, g_sRc663Frame.rxLen, &g_sReaderRspFrame);
                }
                else
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                    g_sReaderRspFrame.err = Reader_GetErrorType(state);
                    g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                }
            }  
            break;            
        case RISO14443A_CMD_DTU:
            //txLen txBit frame timeout rxLen rxBit
            if(paramsLen >= 6 && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                u8 txLen = 0;
                u8 txBit = 0;
                u8 *pTxFrame = NULL;
                u8 rxBit = 0;
                u8 pos = 0;
                u32 timeout = 0;
                vu8 rxLen = 0;
                pos = UART_FRAME_POS_PAR;

                txLen = pFrame[pos++];
                txBit = pFrame[pos++];
                if(txLen > 0)
                {
                    pTxFrame = pFrame + pos;
                }
                pos += txLen;
                timeout = pFrame[pos++];
                timeout += (pFrame[pos++] << 8);
                rxLen = pFrame[pos++];
                rxBit = pFrame[pos++];

                state = ISO14443A_Dtu(pTxFrame, txLen, txBit, &rxBit, timeout);

                if(state == RC663_STAT_OK)
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                    g_sRc663Frame.frame[g_sRc663Frame.rxLen] = rxBit;
                    g_sReaderRspFrame.len = Reader_ResponseFrame(g_sRc663Frame.frame, g_sRc663Frame.rxLen + 1, &g_sReaderRspFrame);
                }
                else
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                    g_sReaderRspFrame.err = Reader_GetErrorType(state);
                    g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                }
            }
            break;
        case RISO14443A_CMD_PCTRLESAM:
            if(paramsLen == 2)
            {
                u8 index = 0;
                u8 ctrl = 0;

                memset(&g_sSamBitFrame, 0, sizeof(g_sSamBitFrame));
                memset(&g_sSamByteFrame, 0, sizeof(g_sSamByteFrame));

                index = pFrame[UART_FRAME_POS_PAR];
                ctrl = pFrame[UART_FRAME_POS_PAR + 1];
                if(ctrl)
                {
                    if(index == SAM_INDEX_1)    //上电或者复位，波特率都恢复到9600
                    {
                        g_nESAM1Speed = SAM_SPEED_9600;
                    }
                    
                    if((ctrl & SAM_CTRL_MASK) == SAM_CTRL_POW_ON)
                    {
                        SAM_CtrlPower(ENABLE, index);
                    }
                    else if((ctrl & SAM_CTRL_MASK) == SAM_CTRL_POW_RST)
                    {
                        SAM_Reset(index);
                    }
                    
                    SAM_RcvFrame(index, g_sReaderSamFrame.rxFrame, &(g_sReaderSamFrame.rxLen));
                    if(g_sReaderSamFrame.rxLen > 0)
                    {
                        if((ctrl & SAM_CTRL_LOW_SPEED) == 0x00)
                        {
                            if(SAM_Enable115200(index))
                            {
                                if(index == SAM_INDEX_1)    //成功设置高速模式
                                {
                                    g_nESAM1Speed = SAM_SPEED_115200;
                                }

                                g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                                g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                                g_sReaderRspFrame.len = Reader_ResponseFrame(g_sReaderSamFrame.rxFrame, g_sReaderSamFrame.rxLen, &g_sReaderRspFrame);
                            }
                            else
                            {
                                g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                                g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NORSP;
                                g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                            }
                        }
                        else
                        {
                            g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                            g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                            g_sReaderRspFrame.len = Reader_ResponseFrame(g_sReaderSamFrame.rxFrame, g_sReaderSamFrame.rxLen, &g_sReaderRspFrame);
                        }
                    }
                    else
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                        g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NORSP;
                        g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                    }
                }
                else
                {
                    SAM_CtrlPower(DISABLE, index);

                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                    g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                }
            }
            break;
        case RISO14443A_CMD_PPS:
            if(paramsLen > 2)
            {
                u8 index = 0;
                
                memset(&g_sSamBitFrame, 0, sizeof(g_sSamBitFrame));
                memset(&g_sSamByteFrame, 0, sizeof(g_sSamByteFrame));
                
                index = pFrame[UART_FRAME_POS_PAR];
                if(index > 0)
                {
                    if(SAM_SendRcvFrame(index, pFrame + UART_FRAME_POS_PAR + 2, pFrame[UART_FRAME_POS_PAR + 1], g_sSamByteFrame.rxFrame, &(g_sSamByteFrame.rxLength)))
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                        g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                    }
                    else
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                        g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NORSP;
                    }
                    g_sReaderRspFrame.len = Reader_ResponseFrame(g_sSamByteFrame.rxFrame, g_sSamByteFrame.rxLength, &g_sReaderRspFrame);
                }
            }
            break;
        case RISO14443A_CMD_SELESAMBR:
            if(paramsLen == 2)
            {
                u8 index = 0;
                
                index = pFrame[UART_FRAME_POS_PAR];
                if(index > 0)
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                    if(pFrame[UART_FRAME_POS_PAR] == SAM_INDEX_1 && pFrame[UART_FRAME_POS_PAR + 1] <= SAM_SPEED_115200)
                    {
                        g_nESAM1Speed = pFrame[UART_FRAME_POS_PAR + 1];
                    }
                    else
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                        g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_PARERR;
                    }
                    g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                }
            }
            break;
        case RISO14443A_CMD_APDU:
            if(paramsLen > 0)
            {
                u8 len = 0;
                u8 index = 0;

                index = pFrame[UART_FRAME_POS_PAR];
                if(index == 0 && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN)
                {
                    paramsLen += 1;      //PCB CID
                    pFrame[UART_FRAME_POS_PAR - 1] = ISO14443A_PCB_IBLOCK | (g_sISO1444A4.pcb & ISO14443A_IPCB_NUMMASK);
                    pFrame[UART_FRAME_POS_PAR - 0] = ISO14443A_CID;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_TAGERR;
                    state = ISO14443A_TransAPDU(pFrame + UART_FRAME_POS_PAR - 1, paramsLen, g_sReaderSamFrame.rxFrame, &len);
                    if(state == RC663_STAT_OK)
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                        g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                        //去掉CRC2字节和PCB的2字节
                        g_sReaderRspFrame.len = Reader_ResponseFrame(g_sReaderSamFrame.rxFrame + 2, len - 4, &g_sReaderRspFrame);
                    }
                    else
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                        g_sReaderRspFrame.err = Reader_GetErrorType(state);
                        g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                    }
                }
                else
                {
                    BOOL b = TRUE;
                    memset(&g_sSamBitFrame, 0, sizeof(g_sSamBitFrame));
                    memset(&g_sSamByteFrame, 0, sizeof(g_sSamByteFrame));
                    memcpy(g_sReaderSamFrame.txFrame, pFrame + UART_FRAME_POS_PAR + 1, paramsLen - 1);
                    g_sReaderSamFrame.txLen = paramsLen - 1;

                    b = SAM_Apdu(index, g_sReaderSamFrame.txFrame, g_sReaderSamFrame.txLen, g_sReaderSamFrame.rxFrame, &(g_sReaderSamFrame.rxLen));
                    if(b)
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                        g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                        g_sReaderRspFrame.len = Reader_ResponseFrame(g_sReaderSamFrame.rxFrame, g_sReaderSamFrame.rxLen, &g_sReaderRspFrame);
                    }
                    else
                    {
                        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                        g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NORSP;
                        g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                    }
                }
            }
            break;

        case RISO14443A_CMD_DESEL:
            if(paramsLen == 0 && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN)
            {
                paramsLen += 2;      //PCB CID
                pFrame[UART_FRAME_POS_PAR - 2] = ISO14443A_PCB_SBLOCK | ISO14443A_SPCB_DSEL;
                pFrame[UART_FRAME_POS_PAR - 1] = ISO14443A_CID;
                g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_TAGERR;
                state = ISO14443A_TransAPDU(pFrame + UART_FRAME_POS_PAR - 2, paramsLen, g_sReaderSamFrame.rxFrame, &(g_sReaderSamFrame.rxLen));
                //DSEL没有返回
                //if(state == RC663_STAT_OK)
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                    g_sReaderRspFrame.len = Reader_ResponseFrame(0, 0, &g_sReaderRspFrame);
                }
            }
            break;
        case RISO14443A_CMD_HALT:
            if(paramsLen == 0 && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_TAGERR;
                state = ISO14443A_Halt();
                if(state == RC663_STAT_OK)
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                    g_sReaderRspFrame.len = Reader_ResponseFrame(0, 0, &g_sReaderRspFrame);
                } 
                else
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                    g_sReaderRspFrame.err = Reader_GetErrorType(state);
                    g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                }
            }
            break;
        case RISO14443A_CMD_AUTHM0U2:
            if(paramsLen == ISO14443A_M0U2KEY_LEN && g_sDeviceParamenter.rfCtrl == READER_RF_OPEN && (g_sDeviceParamenter.workMode & READER_TYPE_MASK) == READER_TYPE_ISO14443A)
            {
                g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_TAGERR;
                state = ISO14443A_AuthUltralightC(pFrame + UART_FRAME_POS_PAR);
                if(state == RC663_STAT_OK)
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_OK;
                    g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_NOERR;
                    g_sReaderRspFrame.len = Reader_ResponseFrame(0, 0, &g_sReaderRspFrame);
                } 
                else
                {
                    g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
                    g_sReaderRspFrame.err = Reader_GetErrorType(state);
                    g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
                }
            }
            break;
        default:
            break;
    }
    if(g_sReaderRspFrame.len == 0)
    {
        g_sReaderRspFrame.flag = READER_RSPFRAME_FLAG_FAIL;
        g_sReaderRspFrame.err = READER_OPTAG_RESPONSE_PARERR;
        g_sReaderRspFrame.len = Reader_ResponseFrame(NULL, 0, &g_sReaderRspFrame);
    }
    return g_sReaderRspFrame.len;
}*/

//-------------------------------------

const u8 DEVICE_VERSION[DEVICE_VERSION_SIZE]@0x08005000 = "R0001 25011800 R001-SCH-MB-250100";
const u8 DEVICE_HARD_TYPE[DEVICE_VERSION_SIZE]@0x08005080 = "R001-SCH-MB-250100";

DEVICE_BOOTPARAMS g_sDeviceBootParamenter = {0};
DEVICE_PARAMS g_sDeviceParams = {0};
DEVICE_OP g_sDeviceOp = {0};
DEVICE_OPTAGINFO g_sDeviceOpTagInfo = {0};
DEVICE_RSPFRAME g_sDevicerRspFrame = {0};
u8 Device_ExportSatus = 0;
void Device_Delayms(u32 n)
{
    n *= 0x3800;
    n++;
    while(n--);
}

const PORT_INF EXPORT_CH1_PORT = {GPIOB, GPIO_Pin_13};
const PORT_INF EXPORT_CH2_PORT = {GPIOB, GPIO_Pin_14};

void Device_ExportInit()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = EXPORT_CH1_PORT.Pin | EXPORT_CH2_PORT.Pin;
    GPIO_Init(EXPORT_CH1_PORT.Port, &GPIO_InitStructure);
   
    //Device_ExportCh1High();
    Device_ExportCh1Low();
    
    //Device_ExportCh2High();
    Device_ExportCh2Low();
}


void Device_Init(void)
{
    Device_ExportInit();
    FM17xx_Reset();
    ISO14443A_Init();
    //FM17xx_OpenAntenna() ;
    if(Device_ReadParamenters(DEVICE_PAR_START) == FALSE || Device_ReadParamenters(DEVICE_PAR_BACK_START) == FALSE)
    {
        Device_WriteParamenters(DEVICE_PAR_START);
        Device_WriteParamenters(DEVICE_PAR_BACK_START);
    }
    
    if(Device_ReadBootParamenter(DEVICE_APP_PAR_START) == FALSE || (g_sDeviceBootParamenter.appState != DEVICE_BOOT_APP_OK) || 
       Device_ReadBootParamenter(DEVICE_APPPAR_BACK_START) == FALSE || g_sDeviceBootParamenter.addr ^ g_sDeviceParams.addr)
    {
        g_sDeviceBootParamenter.addr = g_sDeviceParams.addr;
        g_sDeviceBootParamenter.appState = DEVICE_BOOT_APP_OK;
        Device_WriteBootParamenter(DEVICE_APP_PAR_START);
        Device_WriteBootParamenter(DEVICE_APPPAR_BACK_START);
    }
    
    a_SetState(g_sDeviceOp.state, DEVICE_STAT_IDLE);
}

BOOL Device_ReadBootParamenter(u32 addr)                                         //OK
{
    u8 i = 0;
    u8 *p = NULL;
    u8 *pDevicePar = NULL;
    u16 crc1 = 0, crc2 = 0;
    BOOL b = FALSE;

    p = (u8 *)(addr);
    pDevicePar = (u8 *)(&g_sDeviceBootParamenter);

    for(i = 0; i < sizeof(DEVICE_BOOTPARAMS); i++)
    {
        *(pDevicePar + i) = *(p + i);
    }

    crc1 = a_GetCrc((u8 *)pDevicePar, (sizeof(DEVICE_BOOTPARAMS)) - 4);
    crc2 = g_sDeviceBootParamenter.crc;

    //重新写入默认值
    if((crc1 != crc2))
    {
        memset(&g_sDeviceBootParamenter, 0, sizeof(DEVICE_BOOTPARAMS));
        g_sDeviceBootParamenter.addr = g_sDeviceParams.addr;
        g_sDeviceBootParamenter.appState = DEVICE_BOOT_APP_OK;
    }
    else
    {
        b = TRUE;
    }

    return b;
}

BOOL Device_WriteBootParamenter(u32 addr)
{
    u16 i = 0;
    BOOL b = FALSE;
    u16 *pDevicePar = NULL;
    FLASH_Status flashStatus = FLASH_BUSY;

    pDevicePar = (u16 *)(&g_sDeviceBootParamenter);

    g_sDeviceBootParamenter.crc = 0;
    g_sDeviceBootParamenter.crc = a_GetCrc((u8 *)pDevicePar, (sizeof(DEVICE_BOOTPARAMS)) - 4);

#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif

    FLASH_Unlock();
    flashStatus = FLASH_ErasePage(addr);
    if(flashStatus == FLASH_COMPLETE)
    {
        for(i = 0; i < (sizeof(DEVICE_BOOTPARAMS) >> 1) + 1; i++)
        {
            if(flashStatus != FLASH_COMPLETE)
            {
                break;
            }
            flashStatus = FLASH_ProgramHalfWord(addr + (i << 1), pDevicePar[i]);
        }

        if(i >= (sizeof(DEVICE_BOOTPARAMS) >> 1))
        {
            b = TRUE;
        }
    }
    FLASH_Lock();
    return b;
}

BOOL Device_ReadParamenters(u32 addr)                                         //OK
{
    u16 i = 0;
    u8 *p = NULL;
    u8 *pDevicePar = NULL;
    u16 crc1 = 0, crc2 = 0;
    BOOL b = FALSE;

    p = (u8 *)(addr);
    pDevicePar = (u8 *)(&g_sDeviceParams);

    for(i = 0; i < sizeof(DEVICE_PARAMS); i++)
    {
        *(pDevicePar + i) = *(p + i);
    }

    crc1 = a_GetCrc((u8 *)pDevicePar, (sizeof(DEVICE_PARAMS)) - 2);
    crc2 = g_sDeviceParams.crc;

    
    if((crc1 != crc2))
    {
        memset(&g_sDeviceParams, 0, sizeof(DEVICE_PARAMS));
        
        g_sDeviceParams.addr = 0x01;
        g_sDeviceParams.blockAddr = 0x08;                   //扇区二块一                         ，校验前八字节
        
        g_sDeviceParams.blockDate[0][0] = 0x21; g_sDeviceParams.blockDate[0][1] = 0x21; g_sDeviceParams.blockDate[0][2] = 0x21; g_sDeviceParams.blockDate[0][3] = 0x21;
        g_sDeviceParams.blockDate[0][4] = 0x21; g_sDeviceParams.blockDate[0][5] = 0x21; g_sDeviceParams.blockDate[0][6] = 0x21; g_sDeviceParams.blockDate[0][7] = 0x21;
        
        g_sDeviceParams.blockDate[1][0] = 0x52; g_sDeviceParams.blockDate[1][1] = 0x52; g_sDeviceParams.blockDate[1][2] = 0x52; g_sDeviceParams.blockDate[1][3] = 0x52;
        g_sDeviceParams.blockDate[1][4] = 0x52; g_sDeviceParams.blockDate[1][5] = 0x52; g_sDeviceParams.blockDate[1][6] = 0x52; g_sDeviceParams.blockDate[1][7] = 0x52;
        
        g_sDeviceParams.key[0] = 0x13; g_sDeviceParams.key[1] = 0x14; g_sDeviceParams.key[2] = 0x13; 
        g_sDeviceParams.key[3] = 0x14; g_sDeviceParams.key[4] = 0x13; g_sDeviceParams.key[5] = 0x14; 
    }
    else
    {
        b = TRUE;
    }

    return b;
}

BOOL Device_WriteParamenters(u32 addr)
{
    u16 i = 0;
    BOOL b = FALSE;
    u16 *pDevicePar = NULL;
    FLASH_Status flashStatus = FLASH_BUSY;

    pDevicePar = (u16 *)(&g_sDeviceParams);
    g_sDeviceParams.crc = 0;
    g_sDeviceParams.crc = a_GetCrc((u8 *)pDevicePar, (sizeof(DEVICE_PARAMS)) - 2);

#if SYS_ENABLE_WDT
    WDG_FeedIWDog();
#endif

    FLASH_Unlock();
    flashStatus = FLASH_ErasePage(addr);
    if(flashStatus == FLASH_COMPLETE)
    {
        for(i = 0; i < (sizeof(DEVICE_PARAMS) >> 1) + 1; i++)
        {
            if(flashStatus != FLASH_COMPLETE)
            {
                break;
            }
            flashStatus = FLASH_ProgramHalfWord(addr + (i << 1), pDevicePar[i]);
        }

        if(i >= (sizeof(DEVICE_PARAMS) >> 1))
        {
            b = TRUE;
        }
    }
    FLASH_Lock();
    return b;
}

void Device_AutoTask()
{
    DEVICE_OP *pOpInfo = NULL;
    u8 num = 0;
    
    pOpInfo = &g_sDeviceOp;
    Device_ResetOp();
    Device_ResetOpTagInfo();

	pOpInfo->op[num++] = DEVICE_OP_INVENTORY;

    pOpInfo->op[num++] = DEVICE_OP_ANTISHAKE;
    
    pOpInfo->op[num++] = DEVICE_OP_AUTH;
    
    pOpInfo->op[num++] = DEVICE_OP_READ;
    
    pOpInfo->op[num++] = DEVICE_OP_PROOF;

    pOpInfo->antiShakeTick = 0;
    pOpInfo->num = num;
    memcpy(pOpInfo->imParams.key, g_sDeviceParams.key, ISO14443A_M1_KEY_LEN);
    memcpy(pOpInfo->imParams.blockDate, g_sDeviceParams.blockDate, DEVICE_PAR_AUTH_BLOCK_NUM * ISO14443A_M1BLOCK_LEN);
    pOpInfo->imParams.blockAddr[0] = g_sDeviceParams.blockAddr;
    a_SetState(pOpInfo->state, DEVICE_STAT_TX);
}

BOOL Device_Transm(DEVICE_OP *pOpInfo)
{
    DEVICE_OPTAGINFO *pOpTagInfo = NULL;
    BOOL b = TRUE;
    u8 op = 0;

    op = pOpInfo->op[pOpInfo->index];
    pOpTagInfo = &g_sDeviceOpTagInfo;
    switch(op)
    {
        case DEVICE_OP_INVENTORY:
            if(ISO14443A_GetUid(&pOpInfo->tag, ISO14443A_CMD_REQALL) == FM17XX_STAT_OK)
            {
                pOpInfo->tagNum = 1;
                memcpy(&pOpTagInfo->tag, &pOpInfo->tag, sizeof(ISO14443A_UID));
                //ISO14443A_Halt();
                pOpInfo->rlt = DEVICE_RESULT_OK;
            }
            else
            {
                pOpInfo->rlt = DEVICE_RESULT_ERR;
                pOpInfo->tagNum = 0;
            }
            break;
        case DEVICE_OP_ANTISHAKE:
            pOpInfo->rlt = DEVICE_RESULT_OK;
            break;
        case DEVICE_OP_AUTH:
            pOpInfo->rlt = ISO14443A_AuthM1(pOpInfo->tag.uid, ISO14443A_CMD_AUTHENT_A, pOpInfo->imParams.key, pOpInfo->imParams.blockAddr[pOpTagInfo->rbIndex]);
            break;
        case DEVICE_OP_READ:
            pOpInfo->rlt = ISO14443A_ReadMifareBlock(pOpInfo->imParams.blockAddr[pOpTagInfo->rbIndex], pOpInfo->block);
            break;
        case DEVICE_OP_PROOF:
            pOpInfo->rlt = DEVICE_RESULT_OK;
            break;
        /*case READER_OP_READSINGLEBLOCK:
            pReaderOp->rlt = ISO15693_ReadBlock(pOpTagInfo->uid, 1, pReaderOp->imParams.blockAddr[pOpTagInfo->rbIndex], pReaderOp->block + (pOpTagInfo->rbIndex << 2), ISO15693_SIZE_BLOCK);
            break;
        case READER_OP_CHKDISH:
            if(g_sDeviceParams.testMode == READER_TEST_ENABLE)
            {
                pReaderOp->rlt = ISO15693_ReadBlock(pOpTagInfo->uid, 1, READER_TEST_BLOCK_ADDR, pReaderOp->block, ISO15693_SIZE_BLOCK);
            }
            else
            {
                pReaderOp->rlt = ISO15693_ReadBlock(pOpTagInfo->uid, 1, pDish->block.addr[pOpTagInfo->rbIndex], pReaderOp->block, ISO15693_SIZE_BLOCK);
            }
            break;*/
        default:
            b = FALSE;
            break;
    }
    Device_Delayms(1);
    return b;
}

u8 Device_Receive(DEVICE_OP *pOpInfo)
{
    u8 op = 0;

    op = pOpInfo->op[pOpInfo->index];
    switch(op)
    {
        case DEVICE_OP_INVENTORY:
        case DEVICE_OP_ANTISHAKE:
        case DEVICE_OP_PROOF:
        case DEVICE_OP_READ:
        case DEVICE_OP_AUTH:
        default:
            break;
    }
    return pOpInfo->rlt;
}


u8 Device_SkipAntiShake(DEVICE_OP *pOpInfo)
{
    u8 i = 0;
    for(i = 0; i < pOpInfo->num; i++)
    {
        if(pOpInfo->op[i] == DEVICE_OP_ANTISHAKE)
        {
            i++;
            break;
        }
    }
    return i;
}


BOOL Device_Step(DEVICE_OP *pOpInfo)
{
    BOOL b = TRUE;
    DEVICE_OPTAGINFO *pOpTagInfo = NULL;
    u8 op = 0;
    u8 rlt = 0;

    pOpTagInfo = &g_sDeviceOpTagInfo;
    op = pOpInfo->op[pOpInfo->index];
    rlt = pOpInfo->rlt;
    switch(op)
    {
        case DEVICE_OP_INVENTORY:
			if(rlt == DEVICE_RESULT_OK)
			{
				pOpTagInfo->repeat = 0;
                pOpInfo->index++;
                if(memcmp(&pOpInfo->tag, &pOpTagInfo->okTag, sizeof(ISO14443A_UID)) == 0)
                {
                    pOpTagInfo->opOkTick = g_nSysTick;
                    pOpInfo->bRepeatTag = TRUE;
                    pOpInfo->index = pOpInfo->num;
                    b = TRUE;
                }
                else if(memcmp(&pOpInfo->tag, &pOpTagInfo->shakeTag, sizeof(ISO14443A_UID)) == 0)
                {
                    pOpTagInfo->noShakeUidTimers = 0;
                    if(pOpTagInfo->antiShakeTick + pOpInfo->antiShakeTick < g_nSysTick)
                    {
                        pOpTagInfo->bAntiShake = FALSE;  //清空防抖信息
                        memset(&pOpTagInfo->shakeTag, 0, sizeof(ISO14443A_UID));
                        pOpInfo->index = Device_SkipAntiShake(pOpInfo); //跳过防抖
                    }
                    else
                    {
                        b = FALSE; 
                    }
                }
            }
			else
			{
				if(pOpTagInfo->repeat >= DEVICE_OP_TAG_REPEAT)
				{
					pOpTagInfo->repeat = 0;
					b = FALSE;
					if(pOpTagInfo->bTagOk)
					{
						Device_CheckRemoveokTag(pOpInfo, pOpTagInfo);
					}
					if(pOpTagInfo->bAntiShake)                                  //多次不能读取shakeUID，清空shakeUidBuffer
					{
						//Reader_CheckRemoveAntiShakeTag(pOpTagInfo);
					}
				}
                else
                {
                    pOpTagInfo->repeat++;
                }
			}
			break;
        case DEVICE_OP_ANTISHAKE: 
            if(rlt == DEVICE_RESULT_OK)
            {
                pOpTagInfo->repeat = 0;
                if(FALSE)
                {
                    pOpInfo->index++;
                }
                else
                {
                    memcpy(&pOpTagInfo->shakeTag, &pOpInfo->tag, sizeof(ISO14443A_UID));
                    pOpTagInfo->noShakeUidTimers = 0;
                    pOpTagInfo->bAntiShake = TRUE;
                    pOpTagInfo->antiShakeTick = g_nSysTick;
                    b = FALSE;  //防抖直接退出
                }
            }
            else
            {
            	pOpTagInfo->repeat++;
            	if(pOpTagInfo->repeat >= DEVICE_OP_TAG_REPEAT)
            	{
                    pOpTagInfo->repeat = 0;
                    b = FALSE;
            	}
            }
        break;
        case DEVICE_OP_AUTH:
            if(rlt == DEVICE_RESULT_OK)
            {
                pOpTagInfo->repeat = 0;
                pOpInfo->index++;
                b = TRUE;  
            }
            else
            {
            	pOpTagInfo->repeat += DEVICE_OP_TAG_REPEAT;                 //关闭
            	if(pOpTagInfo->repeat >= DEVICE_OP_TAG_REPEAT)
            	{
                    pOpTagInfo->repeat = 0;
                    b = FALSE;
            	}
            }
        break;
        case DEVICE_OP_READ: 
            if(rlt == DEVICE_RESULT_OK)
            {
                pOpTagInfo->repeat = 0;
                pOpInfo->index++;
                b = TRUE;  
            }
            else
            {
            	pOpTagInfo->repeat++;
            	if(pOpTagInfo->repeat >= DEVICE_OP_TAG_REPEAT)
            	{
                    pOpTagInfo->repeat = 0;
                    b = FALSE;
            	}
            }
            break;
        case DEVICE_OP_PROOF:
            if(rlt == DEVICE_RESULT_OK)
            {
                pOpTagInfo->repeat = 0;
                pOpInfo->index++;
                if(memcmp(pOpInfo->block, pOpInfo->imParams.blockDate[0], DEVICE_PAR_AUTH_BLOCK_DATE_LEN) == 0)
                {
                    pOpInfo->bProof = TRUE;
                    pOpInfo->proofFlag = DEVICE_EXPORT_CH1_AMAR;
                    b = TRUE;
                }
                else if(memcmp(pOpInfo->block, pOpInfo->imParams.blockDate[1], DEVICE_PAR_AUTH_BLOCK_DATE_LEN) == 0)
                {
                    pOpInfo->bProof = TRUE;
                    pOpInfo->proofFlag = DEVICE_EXPORT_CH2_AMAR;
                    b = TRUE;
                }
                else
                {
                    pOpInfo->bProof = FALSE;
                }
            }
            else
            {
            	pOpTagInfo->repeat++;
            	if(pOpTagInfo->repeat >= DEVICE_OP_TAG_REPEAT)
            	{
                    pOpTagInfo->repeat = 0;
                    b = FALSE;
            	}
            }
        break;
        default:
            b = FALSE;
            break;
    }
    return b;
}


u16 Device_ProcessUartFrame(u8 *pFrame, u16 len)
{
    u8 cmd = 0;

    u16 destAddr = 0;
    u16 paramsLen = 0;
    u8 *pParams = NULL;
    BOOL bOpLcm = FALSE;
    BOOL bRfOperation = FALSE;

    destAddr = *((u16 *)(pFrame + UART_FRAME_POS_DESTADDR));
    if((destAddr != 0xFFFF) && (destAddr != g_sDeviceParams.addr))
    {
        return 0;
    }

    g_sDevicerRspFrame.destAddr = *((u16 *)(pFrame + UART_FRAME_POS_SRCADDR));
    g_sDevicerRspFrame.len = 0;
    cmd = *(pFrame + UART_FRAME_POS_CMD);

    g_sDevicerRspFrame.cmd = cmd;
    g_sDevicerRspFrame.flag = UART_FRAME_FLAG_OK;
    g_sDevicerRspFrame.err = UART_FRAME_RSP_NOERR;

    if(pFrame[UART_FRAME_POS_LEN] == 0)
    {
        paramsLen = *((u16 *)(pFrame + UART_FRAME_POS_PAR));
        pParams = pFrame + UART_FRAME_POS_PAR + 2;
    }
    else
    {
        paramsLen = len - UART_FRAME_MIN_LEN;
        pParams = pFrame + UART_FRAME_POS_PAR;
    }
    
    switch(cmd)
    {
        case UART_CMD_RESET:
            if(paramsLen == 0)
            {
                g_sDevicerRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDevicerRspFrame);
            }
            break;
        case UART_CMD_GET_VERSION:
            if(paramsLen == 0)
            {
                g_sDevicerRspFrame.len = Device_ResponseFrame((u8 *)DEVICE_VERSION, DEVICE_VERSION_SIZE, &g_sDevicerRspFrame);
            }
            break;
		case UART_CMD_GET_CPUID:
            if(paramsLen == 0)
            {
                g_sDevicerRspFrame.len = Device_ResponseFrame((u8 *)STM32_CPUID_ADDR, STM32_CPUID_LEN, &g_sDevicerRspFrame);
            }
            break;
        default:
            break;
    }
    
    if(g_sDevicerRspFrame.len == 0 && bOpLcm == FALSE && bRfOperation == FALSE)
    {
        g_sDevicerRspFrame.flag = UART_FRAME_FLAG_FAIL;
        g_sDevicerRspFrame.err = UART_FRAME_RSP_PARAMSERR;
        g_sDevicerRspFrame.len = Device_ResponseFrame(NULL, 0, &g_sDevicerRspFrame);
    }
    
    return g_sDevicerRspFrame.len;
}

u16 Device_ResponseFrame(u8 *pParam, u16 len, DEVICE_RSPFRAME *pRspFrame)
{
    u16 pos = 0;
    u16 crc = 0;

    pRspFrame->buffer[pos++] = UART_FRAME_FLAG_HEAD1; // frame head first byte
    pRspFrame->buffer[pos++] = UART_FRAME_FLAG_HEAD2; // frame haed second byte
    pRspFrame->buffer[pos++] = 0x00;   // length
    pRspFrame->buffer[pos++] = (g_sDeviceParams.addr >> 0) & 0xFF;
    pRspFrame->buffer[pos++] = (g_sDeviceParams.addr >> 8) & 0xFF;
    pRspFrame->buffer[pos++] = (pRspFrame->destAddr >> 0) & 0xFF;
    pRspFrame->buffer[pos++] = (pRspFrame->destAddr >> 8) & 0xFF;
    pRspFrame->buffer[pos++] = UART_FRAME_RESPONSE_FLAG;
    pRspFrame->buffer[pos++] = pRspFrame->cmd;               // cmd
    pRspFrame->buffer[pos++] = UART_FRAME_PARAM_RFU;     // RFU

    if(len > UART_FRAME_DATA_MAX_LEN)
    {
        pRspFrame->buffer[pos++] = (len >> 0) & 0xFF;
        pRspFrame->buffer[pos++] = (len >> 8) & 0xFF;

        memcpy(pRspFrame->buffer + pos, pParam, len);
        pos += len;
    }
    else
    {
        memcpy(pRspFrame->buffer + pos, pParam, len);
        pos += len;
        pRspFrame->buffer[pos++] = pRspFrame->flag;
        pRspFrame->buffer[pos++] = pRspFrame->err;
        pRspFrame->buffer[UART_FRAME_POS_LEN] = pos - 3 + 2; //减去帧头7E 55 LEN 的三个字节，加上CRC的两个字节
    }

    crc = a_GetCrc(pRspFrame->buffer + UART_FRAME_POS_LEN, pos - UART_FRAME_POS_LEN); //从LEN开始计算crc
    pRspFrame->buffer[pos++] = crc & 0xFF;
    pRspFrame->buffer[pos++] = (crc >> 8) & 0xFF;

    pRspFrame->len = pos;

    return pos;
}

void Device_CheckRemoveokTag(DEVICE_OP *pOpInfo, DEVICE_OPTAGINFO *pOpTagInfo)
{
    if(pOpTagInfo->opOkTick + 100 < g_nSysTick)
    {
        Device_ClearOkTag();
        g_sDeviceOp.proofFlag = 0;
    }
}

void Device_MonitorExport(u8 status)
{   
    static u8 oldStatus = 0;
    
    if(status ^ oldStatus)
    {
        oldStatus = status;
        
        if((status & DEVICE_EXPORT_CH1_AMAR) == DEVICE_EXPORT_CH1_AMAR)
        {
            Device_ExportCh1High();
        }
        else if((status & DEVICE_EXPORT_CH2_AMAR) == DEVICE_EXPORT_CH2_AMAR)
        {
            Device_ExportCh2High();
        }
        else 
        {
            Device_ExportCh1Low();
            Device_ExportCh2Low();    
        
        }
    }
}

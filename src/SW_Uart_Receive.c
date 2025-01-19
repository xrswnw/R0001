#include "SW_Uart_Receive.h"


BOOL Uart_CheckSpecialFrame(UART_RCVFRAME *pRcvFrame)
{
    BOOL b = FALSE;
    if(pRcvFrame->length >= UART_FRAME_MIN_LEN && pRcvFrame->length < UART_BUFFER_MAX_LEN)
    {
        u16 crc1 = 0;
        u16 crc2 = 0;

        crc1 = Uart_GetFrameCrc(pRcvFrame->buffer, pRcvFrame->length);
        crc2 = a_GetCrc(pRcvFrame->buffer + UART_FRAME_POS_LEN, pRcvFrame->length - 4);
        if(crc1 == crc2)
        {
            b = TRUE;
        }
    }

    return b;
}


u16 Uart_UsrCheckFrame(u8 *pBuffer, u16 len, u16 *pStartPos)
{
    u16 pos = 0;
    u16 frameLen = 0;
    if(len >= UART_FRAME_MIN_LEN)
    {
        while(pos + UART_FRAME_MIN_LEN <= len)
        {
            if(pBuffer[pos + 0] == UART_FRAME_FLAG_HEAD1 && pBuffer[pos + 1] == UART_FRAME_FLAG_HEAD2)
            {
                u16 crc1 = 0, crc2 = 0;
                frameLen = 0;
                if(pBuffer[pos + UART_FRAME_POS_LEN])
                {
                    frameLen = pBuffer[pos + UART_FRAME_POS_LEN] + 3;
                }
                else
                {
                    frameLen = pBuffer[pos + UART_FRAME_POS_PAR + 1];
                    frameLen <<= 8;
                    frameLen |= pBuffer[pos + UART_FRAME_POS_PAR + 0];

                    frameLen += UART_FRAME_MIN_LEN + 2;
                }

                crc1 = a_GetCrc(pBuffer + pos + UART_FRAME_POS_LEN, frameLen - 4);
                crc2 = pBuffer[pos + frameLen - 1];
                crc2 <<= 8;
                crc2 |= pBuffer[pos + frameLen - 2];
                if(crc1 == crc2)
                {
                    break;
                }
                else
                {
                    frameLen = 0;
                }
            }
            pos++;
        }
    }

    return frameLen;
}

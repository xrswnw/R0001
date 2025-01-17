#include "SW_Uart_Receive.h"

void Uart_ReceiveFrame(u8 byte, UART_RCVFRAME *pRcvFrame)
{
    static u16 specialLen = 0;
    switch(pRcvFrame->state)
    {
        case UART_STAT_IDLE:
            if(byte == UART_FRAME_FLAG_HEAD1)
            {
                pRcvFrame->state = UART_STAT_HEAD1;
                pRcvFrame->buffer[0] = UART_FRAME_FLAG_HEAD1;
            }
            break;  
        case UART_STAT_HEAD1:
            if(byte == UART_FRAME_FLAG_HEAD2)
            {
                pRcvFrame->state = UART_STAT_HEAD2;
                pRcvFrame->buffer[1] = UART_FRAME_FLAG_HEAD2;
            }
            else if(byte == UART_FRAME_FLAG_HEAD1)
            {
                pRcvFrame->state = UART_STAT_HEAD1;
            }
            else
            {
                pRcvFrame->state = UART_STAT_IDLE;
            }
            break;
        case UART_STAT_HEAD2:
            if(UART_FRAME_DATA_MIN_LEN <= byte && byte <= UART_FRAME_DATA_MAX_LEN)
            {
                pRcvFrame->state = UART_STAT_DATA;
                pRcvFrame->length = byte + 3;
                pRcvFrame->buffer[2] = byte;
                pRcvFrame->index = 3;
            }
            else if(byte == 0) //им?и║a??ив?
            {
                pRcvFrame->state = UART_STAT_DATA;
                pRcvFrame->length = 0;
                pRcvFrame->buffer[2] = byte;
                pRcvFrame->index = 3;
            }
            else
            {
                pRcvFrame->state = UART_STAT_IDLE;
            }
            break;
        case UART_STAT_DATA:
            pRcvFrame->buffer[pRcvFrame->index++] = byte;

            if(pRcvFrame->length == 0)
            {
                if(pRcvFrame->index == 10)
                {
                    specialLen = byte;
                }
                else if(pRcvFrame->index == 11)
                {
                    specialLen += byte * 256;
                    pRcvFrame->length = specialLen + 2 + 11;
                    if(pRcvFrame->length > UART_BUFFER_MAX_LEN)
                    {
                        pRcvFrame->state = UART_STAT_IDLE;
                    }
                }

            }
            else if(pRcvFrame->index >= pRcvFrame->length)
            {
                pRcvFrame->state = UART_STAT_END;
            }
            break;
    }

    pRcvFrame->idleTime = 0;
}

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
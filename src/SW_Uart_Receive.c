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
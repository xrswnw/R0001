#ifndef _SW_UART_RECEIVE_
#define _SW_UART_RECEIVE_


#include "SW_R0001_Config.h"

#define UART_BUFFER_MAX_LEN             (1024 + 512)
#define UART_FRAME_DATA_MAX_LEN         244
#define UART_FRAME_DATA_MIN_LEN         8
#define UART_FRAME_FLAG_HEAD1           0x7E
#define UART_FRAME_FLAG_HEAD2           0x55


#define UART_FLAG_RCV                   0x80

#define UART_STAT_TO                    0x02
#define UART_STAT_OVR                   0x01
#define UART_STAT_END                   0x03
#define UART_STAT_IDLE                  0x40
#define UART_STAT_RCV                   0x80
#define UART_STAT_MSK                   0xC0

#define UART_FRAME_POS_HEAD1            0
#define UART_FRAME_POS_HEAD2            1
#define UART_FRAME_POS_LEN              2
#define UART_FRAME_POS_SRCADDR          3
#define UART_FRAME_POS_DESTADDR         5
#define UART_FRAME_POS_CMD              7
#define UART_FRAME_POS_RFU              8
#define UART_FRAME_POS_PAR              9

#define UART_FRAME_MIN_LEN              11
#define UART_FRAME_RESPONSE_FLAG        0x1F
#define UART_FRAME_PARAM_RFU            0x00

#define UART_FRAME_FLAG_OK              0x00
#define UART_FRAME_FLAG_FAIL            0x01
#define UART_FRAME_RSP_NOERR            0x00
#define UART_FRAME_RSP_TAGERR           0x01
#define UART_FRAME_RSP_CRCERR           0x02
#define UART_FRAME_RSP_NORSP            0x03
#define UART_FRAME_RSP_PARAMSERR        0x04

typedef struct uartRcvFrame{
    u8 state;
    u16 index;
    u16 length;
    u16 dataLen;
    u8 buffer[UART_BUFFER_MAX_LEN];
    u32 idleTime;
}UART_RCVFRAME;




#define Uart_IncIdleTime(t, rcvFrame)      do{\
                                                if((rcvFrame).state & UART_FLAG_RCV)\
                                                {\
                                                    (rcvFrame).idleTime += (t);\
                                                    if((rcvFrame).idleTime >= (20 * t))\
                                                    {\
                                                        (rcvFrame).state = UART_STAT_TO;\
                                                    }\
                                                }\
                                            }while(0)
                                              
#define Uart_ReceiveFrame(byte, rcvFrame) do{\
                                                if((rcvFrame).state & UART_STAT_MSK)\
                                                {\
                                                    (rcvFrame).state |= UART_STAT_RCV;\
                                                    (rcvFrame).buffer[(rcvFrame).length++] = (byte);\
                                                    if((rcvFrame).length == UART_BUFFER_MAX_LEN)\
                                                    {\
                                                        (rcvFrame).state = UART_STAT_OVR;\
                                                    }\
                                                }\
                                             }while(0)
  
#define Uart_IsRcvFrame(rcvFrame)           (((rcvFrame).state == UART_STAT_TO || (rcvFrame).state == UART_STAT_END))

#define Uart_ResetFrame(rcvFrame)           do{(rcvFrame)->state = UART_STAT_IDLE; (rcvFrame)->length = 0; (rcvFrame)->index = 0; (rcvFrame)->idleTime = 0;}while(0)
#define Uart_GetFrameCrc(p, len)            (*((u16 *)(p + len - 2)))
#define Uart_GetFrameLength(p)              (((p)[UART_FRAME_POS_LEN]) + 3)

                                               
                                               
                                               
                                               
                                               
                                               
                                               
                                               
                                               
//--------------------------------cmd

#define UART_CMD_RESET                    0x04
#define UART_CMD_GET_CPUID                0xF8
#define UART_CMD_GET_VERSION              0xF7
                                               
                                               
                                               
                                               

BOOL Uart_CheckSpecialFrame(UART_RCVFRAME *pRcvFrame);
void Uart_ReceiveBootFrame(u8 byte, UART_RCVFRAME *pRcvFrame);

u16 Uart_UsrCheckFrame(u8 *pBuffer, u16 len, u16 *pStartPos);
#endif


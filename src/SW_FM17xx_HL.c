#include "SW_FM17xx_HL.h"

const PORT_INF FM17XX_RST_COM ={GPIOB, GPIO_Pin_0};
const PORT_INF FM17XX_CS_COM ={GPIOA, GPIO_Pin_4};
const PORT_INF FM17XX_PORT_SPI ={GPIOA, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7};

void FM17xx_InitInterface(void)
{
    SPI_InitTypeDef   SPI_InitStructure = {0};
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //cs
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = FM17XX_CS_COM.Pin;
    GPIO_Init(FM17XX_CS_COM.Port, &GPIO_InitStructure);
    FM17xx_CS_High();
    //rst
    GPIO_InitStructure.GPIO_Pin = FM17XX_RST_COM.Pin;
    GPIO_Init(FM17XX_RST_COM.Port, &GPIO_InitStructure);
    FM17xx_PowerHigh();
    
    //SPI
    GPIO_InitStructure.GPIO_Pin = FM17XX_PORT_SPI.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(FM17XX_PORT_SPI.Port, &GPIO_InitStructure);
    
    SPI_Cmd(FM17XX_PORT, DISABLE);
    
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex ;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master ;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b ;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low ;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge ;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft ;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;  // 9MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB ;
    SPI_InitStructure.SPI_CRCPolynomial = 7 ;
    SPI_Init(FM17XX_PORT, &SPI_InitStructure);

    SPI_Cmd(FM17XX_PORT, ENABLE);
}

u8 FM17xx_WriteByte(u8 v)
{
    u8 r = 0;

    if(SPI_I2S_GetFlagStatus(FM17XX_PORT, SPI_I2S_FLAG_RXNE) == SET)
    {
        r = SPI_I2S_ReceiveData(FM17XX_PORT);
    }

    while(SPI_I2S_GetFlagStatus(FM17XX_PORT, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(FM17XX_PORT, v);
    while(SPI_I2S_GetFlagStatus(FM17XX_PORT, SPI_I2S_FLAG_RXNE) == RESET);
    
    r = SPI_I2S_ReceiveData(FM17XX_PORT);

    return r;
}

void FM17xx_Delay16us(u32 us)
{
    u32 delay = 0;
    while(us--)
    {
        delay = 190;
        while(delay--);
    }
}
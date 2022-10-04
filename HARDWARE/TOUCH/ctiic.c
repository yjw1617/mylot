#include "ctiic.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//���ݴ�����-IIC ��������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/7
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//�޸�˵��
//V1.1 20140721
//1,�޸�CT_IIC_Read_Byte����,�����ݸ���.
//2,�޸�CT_IIC_Wait_Ack����,��֧��MDK��-O2�Ż�.
//////////////////////////////////////////////////////////////////////////////////

//����I2C�ٶȵ���ʱ
void CT_Delay(void)
{
    delay_us(2);
}
//���ݴ���оƬIIC�ӿڳ�ʼ��
void CT_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOB_CLK_ENABLE();   		//ʹ��GPIOBʱ��
    __HAL_RCC_GPIOF_CLK_ENABLE();   		//ʹ��GPIOFʱ��

    //PB0,PF11
    GPIO_Initure.Pin = GPIO_PIN_0;			//PB0����Ϊ�������
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_OD; //��©���
    GPIO_Initure.Pull = GPIO_PULLUP;        //����
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;   //����
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);

    GPIO_Initure.Pin = GPIO_PIN_11;			//PF11�����������
    HAL_GPIO_Init(GPIOF, &GPIO_Initure);

    CT_IIC_SDA = 1;
    CT_IIC_SCL = 1;
}
//����IIC��ʼ�ź�
void CT_IIC_Start(void)
{
    CT_IIC_SDA = 1;
    CT_IIC_SCL = 1;
    CT_Delay();
    CT_IIC_SDA = 0; //START:when CLK is high,DATA change form high to low
    CT_Delay();
    CT_IIC_SCL = 0; //ǯסI2C���ߣ�׼�����ͻ��������
    CT_Delay();
}
//����IICֹͣ�ź�
void CT_IIC_Stop(void)
{
    CT_IIC_SDA = 0; //STOP:when CLK is high DATA change form low to high
    CT_Delay();
    CT_IIC_SCL = 1;
    CT_Delay();
    CT_IIC_SDA = 1; //����I2C���߽����ź�
    CT_Delay();
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 CT_IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    u8 rack = 0;

    CT_IIC_SDA = 1;
    CT_Delay();
    CT_IIC_SCL = 1;
    CT_Delay();

    while (CT_READ_SDA)
    {
        ucErrTime++;

        if (ucErrTime > 250)
        {
            CT_IIC_Stop();
            rack = 1;
            break;
        }

        CT_Delay();
    }

    CT_IIC_SCL = 0; //ʱ�����0
    CT_Delay();
    return rack;
}
//����ACKӦ��
void CT_IIC_Ack(void)
{
    CT_IIC_SDA = 0;
    CT_Delay();
    CT_IIC_SCL = 1;
    CT_Delay();
    CT_IIC_SCL = 0;
    CT_Delay();
    CT_IIC_SDA = 1;
    CT_Delay();
}
//������ACKӦ��
void CT_IIC_NAck(void)
{
    CT_IIC_SDA = 1;
    CT_Delay();
    CT_IIC_SCL = 1;
    CT_Delay();
    CT_IIC_SCL = 0;
    CT_Delay();
}
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
void CT_IIC_Send_Byte(u8 txd)
{
    u8 t;

    for (t = 0; t < 8; t++)
    {
        CT_IIC_SDA = (txd & 0x80) >> 7;
        CT_Delay();
        CT_IIC_SCL = 1;
        CT_Delay();
        CT_IIC_SCL = 0;
        txd <<= 1;
    }

    CT_IIC_SDA = 1;
}
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
u8 CT_IIC_Read_Byte(unsigned char ack)
{
    u8 i, receive = 0;

    for (i = 0; i < 8; i++ )
    {
        receive <<= 1;

        CT_IIC_SCL = 1;
        CT_Delay();

        if (CT_READ_SDA)receive++;

        CT_IIC_SCL = 0;
        CT_Delay();
    }

    if (!ack)CT_IIC_NAck();//����nACK
    else CT_IIC_Ack(); //����ACK

    return receive;
}






























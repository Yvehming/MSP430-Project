/*
 * uart.c
 *
 *  Created on: 2021年7月25日
 *      Author: USER
 */
#include "uart.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "stdio.h"
uint8_t len,flag = 0;
uint8_t receivedByte = 0x00;
uint8_t receiveData[60],sendData[60];
void myuart_init()
{
    //P3.3,4 = USCI_A0 TXD/RXD
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,GPIO_PIN3 + GPIO_PIN4);

    //Baudrate = 9600, clock freq =4MHz
    //UCBRx = 26, UCBRFx = 1, UCBRSx = 0, UCOS16 = 1
    USCI_A_UART_initParam param = {0};
    param.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
    param.clockPrescalar = 26;
    param.firstModReg = 1;
    param.secondModReg = 0;
    param.parity = USCI_A_UART_NO_PARITY;
    param.msborLsbFirst = USCI_A_UART_LSB_FIRST;
    param.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
    param.uartMode = USCI_A_UART_MODE;
    param.overSampling = USCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;
    flag = 0;
    if (STATUS_FAIL == USCI_A_UART_init(USCI_A0_BASE, &param))
    {
      return;
    }

    //Enable UART module for operation
    USCI_A_UART_enable(USCI_A0_BASE);

    //Enable Receive Interrupt
    USCI_A_UART_clearInterrupt(USCI_A0_BASE,USCI_A_UART_RECEIVE_INTERRUPT);
    USCI_A_UART_enableInterrupt(USCI_A0_BASE,USCI_A_UART_RECEIVE_INTERRUPT);
}


void u_printf(char* fmt,...)
{
    uint16_t i,j;
    va_list ap;
    va_start(ap,fmt);
    vsprintf((char*)sendData,fmt,ap);
    va_end(ap);
    i=strlen((const char*)sendData);        //此次发送数据的长度
    for(j=0;j<i;j++)                            //循环发送数据
    {
        USCI_A_UART_transmitData(USCI_A0_BASE,sendData[i]);
    }
}



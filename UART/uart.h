/*
 * uart.h
 *
 *  Created on: 2021Äê7ÔÂ25ÈÕ
 *      Author: USER
 */

#ifndef UART_H_
#define UART_H_
#include "driverlib.h"

extern uint8_t len,flag,receivedByte,receiveData[60];

void myuart_init(void);
void u_printf(char* fmt,...);


#endif /* UART_H_ */

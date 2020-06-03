#ifndef IVISOR_MODBUS_USART_CLIENT_H_
#define IVISOR_MODBUS_USART_CLIENT_H_

#include <stdint.h>


/*******************************************************************************
* Function Name  : uart_init
* Description    : Init UART.
* Input          : baudrate
* Return         : none.
*******************************************************************************/
bool uartClient_init(uint32_t baudRate);

/*******************************************************************************
* Function Name  : uart_send
* Description    : send buffer.
* Input          : pointer to data buffer, number of bytes to be sent
* Return         : 0 if busy.
*******************************************************************************/
int8_t uart_send(uint8_t * pData, uint16_t dataLength);

/*******************************************************************************
* Function Name  : mbrtuTmr_Init
* Description    : Init modbus timer.
* Input          : interchar interval (T3.5) in microseconds
* Return         : Status.
*******************************************************************************/
int8_t  mbrtuTmr_init(uint32_t intercharTime);

/*******************************************************************************
* Function Name  : mbrtuTmr_startIntercharTimer
* Description    : .
* Input          : none
* Return         : none.
*******************************************************************************/
void  mbrtuTmr_startIntercharTimer(void);

#endif /*IVISOR_MODBUS_USART_CLIENT_H_*/

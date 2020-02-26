/**************************************************************************//**
 * @file     BT-05.h
 * @brief    BT-05 Library header
 * @version  V1.00
 * @date     23. February 2020
 ******************************************************************************/

/*********************************************
     BT05 (CC41-A) BLE Module HAL Library
     for USART Full-Duplex mode
*********************************************/

#define HUART_NUMBER		huart3
#define RX_BUF_LENGTH		10

#include "main.h"
#include "MLT-BT05.h"
#include "stm32f1xx_hal.h"

#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef HUART_NUMBER;

uint8_t rxBufUART[RX_BUF_LENGTH];


const uint32_t baudList[5] = {9600, 19200, 38400, 57600, 115200};					// Supported baud rates

typedef enum StatusList
{
	OK		 			= 0x00U,
	ERR_TIMEOUT 		= 0x01U,
	ERR_STRCMP 			= 0x02U,
	ERR_BAUD_INCORRECT 	= 0x03U,
	ERR_SET_BAUD		= 0x04U,
	ERR_NO_BAUD_ACK		= 0x05U,
	ERR_NO_RESPONSE		= 0x09U
} StatusList;


void UART_SendStringCRLF(char *data, uint8_t len) {
	HAL_UART_Transmit(&HUART_NUMBER, (uint8_t*)data, len, 50);
	HAL_UART_Transmit(&HUART_NUMBER, (uint8_t*)"\r\n", 2, 50);
}

uint8_t UART_ReceiveStringCRLF(uint8_t len) {
	for (uint8_t i=0; i<RX_BUF_LENGTH; i++)
		rxBufUART[i] = 0;
	__HAL_UART_FLUSH_DRREGISTER(&HUART_NUMBER);
	if (HAL_UART_Receive(&HUART_NUMBER, rxBufUART, (len+2), 50) == HAL_OK) {
		rxBufUART[len] = 0;
		rxBufUART[len+1] = 0;
		return OK;
	}
	else
		return ERR_TIMEOUT;
}

void UART_ChangeBaudRate(uint32_t baud) {
	HUART_NUMBER.Init.BaudRate = baud;
	HAL_UART_Init(&HUART_NUMBER);
	for (uint16_t i=0; i<100; i++)
		__NOP();
}

uint8_t BT05_CheckPresence(void) {
	UART_SendStringCRLF("AT", 2);
	if (!UART_ReceiveStringCRLF(2))
		return (!strcmp("OK", (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	else
		return ERR_TIMEOUT;
}

uint8_t BT05_FindRightBaud(void) {
	for (uint8_t baudNumber=4; baudNumber<9; baudNumber++) {
		UART_ChangeBaudRate(baudList[baudNumber-4]);
		if (!BT05_CheckPresence())
			return baudNumber;
		HAL_Delay(10);																// Replace with osDelay
	}
	return ERR_NO_RESPONSE;
}

uint8_t BT05_SetBaud(uint32_t baud) {
	UART_ChangeBaudRate(baud);
	if (!BT05_CheckPresence())
		return OK;
	else {
		HAL_Delay(10);																// Replace with osDelay
		if (BT05_FindRightBaud() != ERR_NO_RESPONSE) {
			HAL_Delay(10);															// Replace with osDelay
			switch(baud) {
			case 9600:
				UART_SendStringCRLF("AT+BAUD4", 8);
				break;
			case 19200:
				UART_SendStringCRLF("AT+BAUD5", 8);
				break;
			case 38400:
				UART_SendStringCRLF("AT+BAUD6", 8);
				break;
			case 57600:
				UART_SendStringCRLF("AT+BAUD7", 8);
				break;
			case 115200:
				UART_SendStringCRLF("AT+BAUD8", 8);
				break;
			default:
				return ERR_BAUD_INCORRECT;
			}
			if (!UART_ReceiveStringCRLF(7) && !strncmp("+BAUD=", (char*)&rxBufUART, 6)) {
				UART_ChangeBaudRate(baud);
				HAL_Delay(200);														// Replace with osDelay
				return !BT05_CheckPresence() ? OK : ERR_NO_BAUD_ACK;
			}
			else
				return ERR_SET_BAUD;
		}
		else
			return ERR_NO_RESPONSE;
	}
}






/**************************************************************************//**
 * @file     BT-05.h
 * @brief    BT-05 Library header
 * @version  V1.00
 * @date     23. February 2020
 ******************************************************************************/

/***************************************
     MLT-BT05 BLE Module HAL Library
     for USART Full-Duplex mode
***************************************/

#define RX_BUF_LENGTH		10


#include "main.h"
#include "MLT-BT05.h"
#include "stm32f1xx_hal.h"

#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart3;

const uint16_t baudList[5] = {0, 1200, 2400, 4800, 9600};
uint8_t rxBufUART[RX_BUF_LENGTH];

typedef enum StatusList
{
	OK		 			= 0x00U,
	ERR_TIMEOUT 		= 0x01U,
	ERR_STRCMP 			= 0x02U,
	ERR_BAUD_INCORRECT 	= 0x03U,
	ERR_SET_BAUD		= 0x04U,
	ERR_NOT_RESPONDING	= 0x05U
} StatusList;


void UART_SendStringCRLF(char *data, uint8_t len) {
	HAL_UART_Transmit(&huart3, (uint8_t*)data, len, 50);
	HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n", 2, 50);
}

uint8_t UART_ReceiveStringCRLF(uint8_t len) {
	for (uint8_t i=0; i<RX_BUF_LENGTH; i++)
		rxBufUART[i] = 0;
	__HAL_UART_FLUSH_DRREGISTER(&huart3);
	if (HAL_UART_Receive(&huart3, rxBufUART, (len+2), 50) == HAL_OK) {
		rxBufUART[len] = 0;
		rxBufUART[len+1] = 0;
		return OK;
	}
	else
		return ERR_TIMEOUT;
}

void UART_ChangeBaudRate(uint32_t baud) {
	huart3.Init.BaudRate = baud;
	HAL_UART_Init(&huart3);
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
	for (uint8_t baudNumber=1; baudNumber<=4; baudNumber++) {
		UART_ChangeBaudRate(baudList[baudNumber]);
		if (!BT05_CheckPresence())
			return baudNumber;
		HAL_Delay(15);																// Replace with osDelay
	}
	return ERR_NOT_RESPONDING;
}

uint8_t BT05_SetBaud(uint32_t baud) {
	UART_ChangeBaudRate(baud);
	if (!BT05_CheckPresence())
		return OK;
	else {
		HAL_Delay(15);																// Replace with osDelay
		if (BT05_FindRightBaud() != ERR_NOT_RESPONDING) {
			HAL_Delay(15);															// Replace with osDelay
			switch(baud) {
			case 1200:
				UART_SendStringCRLF("AT+BAUD1", 8);
				break;
			case 2400:
				UART_SendStringCRLF("AT+BAUD2", 8);
				break;
			case 4800:
				UART_SendStringCRLF("AT+BAUD3", 8);
				break;
			case 9600:
				UART_SendStringCRLF("AT+BAUD4", 8);
				break;
			default:
				return ERR_BAUD_INCORRECT;
			}
			if (!UART_ReceiveStringCRLF(7) && !strncmp("+BAUD=", (char*)&rxBufUART, 6)) {
				UART_ChangeBaudRate(baud);
				HAL_Delay(15);													// Replace with osDelay
				return OK;
			}
			else
				return ERR_SET_BAUD;
		}
		else
			return ERR_NOT_RESPONDING;
	}
}






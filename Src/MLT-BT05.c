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

uint8_t rxBufUART[RX_BUF_LENGTH];

typedef enum
{
	OK		 			= 0x00U,
	ERR_TIMEOUT 		= 0x01U,
	ERR_STRCMP 			= 0x02U,
	ERR_BAUD_INCORRECT 	= 0x03U,
	ERR_SET_BAUD		= 0x04U
} StatusList;


void UART_SendStringCRLF(char *data, uint8_t len) {
	HAL_UART_Transmit(&huart3, (uint8_t*)data, len, 50);
	HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n", 2, 50);
}

uint8_t UART_ReceiveStringCRLF(uint8_t len) {
	for(uint8_t i=0; i<RX_BUF_LENGTH; i++)
		rxBufUART[i] = 0;
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
	if(!UART_ReceiveStringCRLF(2))
		return (!strcmp("OK", (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	else
		return ERR_TIMEOUT;
}

uint8_t BT05_SetBaud(uint32_t baud) {
	UART_ChangeBaudRate(baud);
	if (!BT05_CheckPresence())
		return OK;
	else {
		HAL_Delay(25);																// Replace with osDelay
		UART_ChangeBaudRate(9600);

		UART_SendStringCRLF("AT", 2);
		uint8_t anusEbanogoPsa[10];
		for (uint8_t anusCounter=0; anusCounter<10; anusCounter++)
			anusEbanogoPsa[anusCounter] = 0;
		HAL_UART_Receive(&huart3, anusEbanogoPsa, 4, 50);
		if (anusEbanogoPsa[0] == 0)
			UART_SendStringCRLF("PIZDA", 5);

		HAL_Delay(100);

		if (!BT05_CheckPresence()) {
			HAL_Delay(25);															// Replace with osDelay
			char txBufUART[10];
			uint8_t baudNumber;
			switch (baud) {
			case 115200:
				baudNumber = 0;
			case 57600:
				baudNumber = 1;
			case 38400:
				baudNumber = 2;
			case 19200:
				baudNumber = 3;
			case 9600:
				baudNumber = 4;
			default:
				return ERR_BAUD_INCORRECT;
			}
			sprintf(txBufUART, "AT+BAUD%d", baudNumber);
			UART_SendStringCRLF(txBufUART, strlen(txBufUART));
			UART_ChangeBaudRate(baud);
			if (!UART_ReceiveStringCRLF(7) && !strcmp("+BAUD=", (char*)&rxBufUART))
				return OK;
			else
				return ERR_SET_BAUD;
		}
	}
	return 1;
}






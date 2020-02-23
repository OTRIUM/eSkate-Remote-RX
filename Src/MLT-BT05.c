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

#include "main.h"
#include "LC12S.h"
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart3;

void UART_SendString(char *data, uint8_t len) {
	HAL_UART_Transmit(&huart3, (uint8_t*)data, len, 100);
	HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n", 2, 100);
}

/*

uint8_t* UART_ReceiveString(uint8_t len) {
	uint8_t buf[len];
	if (HAL_UART_Receive(&huart3, buf, len, 100) == HAL_OK);		// Что за хуйню я пишу?

}

*/

uint8_t BT05_CheckPresence(void) {
	UART_SendString("AT", 2);

}

/*****************************************************************************
 * @file     MLT-BT05.c
 * @brief    BT-05 Library header
 * @version  V1.00
 * @date     23. February 2020
 *****************************************************************************/

/*********************************************
     BT05 (CC41-A) BLE Module HAL Library
     for USART Full-Duplex mode
*********************************************/

#define BT05_HUART_NUMBER		huart3

#define BT05_DESIRED_BAUD		38400												// 9600 - 19200 - 38400 - 57600 - 115200
#define BT05_DESIRED_NAME		"eSkateRX"											// 8 characters max
#define BT05_DESIRED_PIN		"200489"											// 6 digits only
#define BT05_DESIRED_ROLE		"0"													// 0 - slave, 1 - master
#define BT05_DESIRED_UUID		"0xF0E0"											// BLE UUID
#define BT05_DESIRED_CHAR		"0xF0E1"											// BLE Characteristic
#define BT05_DESIRED_NOTI		"0"													// Send notification message when module is connected. 0 - off, 1 - on

#define BT05_UART_BUF_LENGTH	25


#include "main.h"
#include "MLT-BT05.h"
#include "stm32f1xx_hal.h"

#include <string.h>
#include <stdio.h>


extern UART_HandleTypeDef BT05_HUART_NUMBER;

uint8_t BT05_deviceAddress[17];
static uint8_t rxBufUART[BT05_UART_BUF_LENGTH];

static const uint32_t baudList[5] = {9600, 19200, 38400, 57600, 115200};			// Supported baud rates

typedef enum BT05_StatusList
{
	OK		 			= 0x00U,
	ERR_TIMEOUT 		= 0x01U,
	ERR_STRCMP 			= 0x02U,
	ERR_BAUD_INCORRECT 	= 0x03U,
	ERR_SET_BAUD		= 0x04U,
	ERR_NO_BAUD_ACK		= 0x05U,
	ERR_TOO_LONG		= 0x06U,
	ERR_SET_NAME		= 0x07U,
	ERR_NAME_ACK		= 0x08U,
	ERR_NO_RESPONSE		= 0x09U,
	ERR_PIN_LEN			= 0x0AU,
	ERR_PIN_ACK			= 0x0BU,
	ERR_SET_PIN			= 0x0CU,
	ERR_ROLE_LEN		= 0x0DU,
	ERR_ROLE_ACK		= 0x0EU,
	ERR_SET_ROLE		= 0x0FU,
	ERR_UUID_LEN		= 0x10U,
	ERR_UUID_ACK		= 0x11U,
	ERR_SET_UUID		= 0x12U,
	ERR_CHAR_LEN		= 0x13U,
	ERR_CHAR_ACK		= 0x14U,
	ERR_SET_CHAR		= 0x15U,
	ERR_NOTI_LEN		= 0x16U,
	ERR_NOTI_ACK		= 0x17U,
	ERR_SET_NOTI		= 0x18U,
	ERR_GET_ADDR		= 0x19U,
	ERR_BT05_CONFIGURE	= 0x20U
} BT05_StatusList;

uint8_t BT05_Configure(void) {														// Configure the module using AT-commands
	HAL_Delay(250);
	if (!BT05_SendCommands())
		return OK;
	HAL_Delay(100);
	return !BT05_SendCommands() ? OK : ERR_BT05_CONFIGURE;
}

uint8_t BT05_SendCommands(void) {													// Send configuration commands
	uint8_t errCounter = 0;
	errCounter += BT05_SetBaud(BT05_DESIRED_BAUD);
	HAL_Delay(10);
	errCounter += BT05_SetName(BT05_DESIRED_NAME);
	HAL_Delay(10);
	errCounter += BT05_SetPin(BT05_DESIRED_PIN);
	HAL_Delay(10);
	errCounter += BT05_SetRole(BT05_DESIRED_ROLE);
	HAL_Delay(10);
	errCounter += BT05_SetUUID(BT05_DESIRED_UUID);
	HAL_Delay(10);
	errCounter += BT05_SetCHAR(BT05_DESIRED_CHAR);
	HAL_Delay(10);
	errCounter += BT05_SetNoti(BT05_DESIRED_NOTI);
	HAL_Delay(10);
	return errCounter;
}

void BT05_UART_SendStringCRLF(char *data) {											// Send a string of chars with CR&LF
	uint8_t len = strlen(data);
	HAL_UART_Transmit(&BT05_HUART_NUMBER, (uint8_t*)data, len, 50);
	HAL_UART_Transmit(&BT05_HUART_NUMBER, (uint8_t*)"\r\n", 2, 50);
}

uint8_t BT05_UART_ReceiveStringCRLF(uint8_t len) {									// Receive N bytes of data
	for (uint8_t i=0; i<BT05_UART_BUF_LENGTH; i++)
		rxBufUART[i] = 0;
	__HAL_UART_FLUSH_DRREGISTER(&BT05_HUART_NUMBER);
	if (HAL_UART_Receive(&BT05_HUART_NUMBER, rxBufUART, (len+2), 50) == HAL_OK) {
		rxBufUART[len] = 0;
		rxBufUART[len+1] = 0;
		return OK;
	}
	return ERR_TIMEOUT;
}

void BT05_UART_ChangeBaudRate(uint32_t baud) {										// Change the baudrate of selected UART
	BT05_HUART_NUMBER.Init.BaudRate = baud;
	HAL_UART_Init(&BT05_HUART_NUMBER);
	for (uint16_t i=0; i<100; i++)
		__NOP();
}

uint8_t BT05_CheckPresence(void) {													// Check whether the module is present using AT command
	BT05_UART_SendStringCRLF("AT");
	if (!BT05_UART_ReceiveStringCRLF(2))
		return (!strcmp("OK", (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	return ERR_TIMEOUT;
}

uint8_t BT05_FindRightBaud(void) {													// Find correct baudrate using bruteforce and AT command
	for (uint8_t baudNumber=4; baudNumber<9; baudNumber++) {
		BT05_UART_ChangeBaudRate(baudList[baudNumber-4]);
		if (!BT05_CheckPresence())
			return baudNumber;
		HAL_Delay(10);
	}
	return ERR_NO_RESPONSE;
}

uint8_t BT05_SetBaud(uint32_t baud) {												// Set the module to desired baudrate
	BT05_UART_ChangeBaudRate(baud);
	if (!BT05_CheckPresence())
		return OK;
	HAL_Delay(10);
	if (BT05_FindRightBaud() != ERR_NO_RESPONSE) {
		HAL_Delay(10);
		switch(baud) {
		case 9600:
			BT05_UART_SendStringCRLF("AT+BAUD4");
			break;
		case 19200:
			BT05_UART_SendStringCRLF("AT+BAUD5");
			break;
		case 38400:
			BT05_UART_SendStringCRLF("AT+BAUD6");
			break;
		case 57600:
			BT05_UART_SendStringCRLF("AT+BAUD7");
			break;
		case 115200:
			BT05_UART_SendStringCRLF("AT+BAUD8");
			break;
		default:
			return ERR_BAUD_INCORRECT;
		}
		if (!BT05_UART_ReceiveStringCRLF(7) && !strncmp("+BAUD=", (char*)&rxBufUART, 6)) {
			BT05_UART_ChangeBaudRate(baud);
			HAL_Delay(200);
			return !BT05_CheckPresence() ? OK : ERR_NO_BAUD_ACK;
		}
		return ERR_SET_BAUD;
	}
	return ERR_NO_RESPONSE;
}

uint8_t BT05_CheckName(char *data) {												// Check current module name
	uint8_t len = strlen(data);
	BT05_UART_SendStringCRLF("AT+NAME");
	if (!BT05_UART_ReceiveStringCRLF(6+len)) {
		for (uint8_t i=6; i<BT05_UART_BUF_LENGTH; i++) {
			rxBufUART[i-6] = rxBufUART[i];
			rxBufUART[i] = 0;
		}
		return (!strcmp(data, (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	}
	return ERR_TIMEOUT;
}

uint8_t BT05_SetName(char *data) {													// Set new name to the module
	uint8_t len = strlen(data);
	if (len > 8)
		return ERR_TOO_LONG;
	if (!BT05_CheckName(data))
		return OK;
	char txBufUART[BT05_UART_BUF_LENGTH];
	for (uint8_t i=0; i<BT05_UART_BUF_LENGTH; i++)
			txBufUART[i] = 0;
	sprintf(txBufUART, "AT+NAME%s", data);
	BT05_UART_SendStringCRLF(txBufUART);
	if (!BT05_UART_ReceiveStringCRLF(6+len)) {
		sprintf(txBufUART, "+NAME=%s", data);
		HAL_Delay(150);
		return (!strcmp(txBufUART, (char*)&rxBufUART)) ? OK : ERR_NAME_ACK;
	}
	return ERR_SET_NAME;
}

uint8_t BT05_CheckPin(char *data) {													// Check current module pin (used for pairing)
	uint8_t len = strlen(data);
	BT05_UART_SendStringCRLF("AT+PIN");
	if (!BT05_UART_ReceiveStringCRLF(5+len)) {
		for (uint8_t i=5; i<BT05_UART_BUF_LENGTH; i++) {
			rxBufUART[i-5] = rxBufUART[i];
			rxBufUART[i] = 0;
		}
		return (!strcmp(data, (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	}
	return ERR_TIMEOUT;
}

uint8_t BT05_SetPin(char *data) {													// Set new pin to the module (used for pairing)
	uint8_t len = strlen(data);
	if (len != 6)
		return ERR_PIN_LEN;
	if (!BT05_CheckPin(data))
		return OK;
	char txBufUART[BT05_UART_BUF_LENGTH];
	for (uint8_t i=0; i<BT05_UART_BUF_LENGTH; i++)
		txBufUART[i] = 0;
	sprintf(txBufUART, "AT+PIN%s", data);
	BT05_UART_SendStringCRLF(txBufUART);
	if (!BT05_UART_ReceiveStringCRLF(5+len)) {
		sprintf(txBufUART, "+PIN=%s", data);
		return (!strcmp(txBufUART, (char*)&rxBufUART)) ? OK : ERR_PIN_ACK;
	}
	return ERR_SET_PIN;
}

uint8_t BT05_CheckRole(char *data) {												// Check the role of the module (0 - slave, 1 - master)
	uint8_t len = strlen(data);
	BT05_UART_SendStringCRLF("AT+ROLE");
	if (!BT05_UART_ReceiveStringCRLF(6+len))
		return (data[0] == rxBufUART[6]) ? OK : ERR_STRCMP;
	return ERR_TIMEOUT;
}

uint8_t BT05_SetRole(char *data) {													// Set the role of the module (0 - slave, 1 - master)
	uint8_t len = strlen(data);
	if (len != 1)
		return ERR_ROLE_LEN;
	if (!BT05_CheckRole(data))
		return OK;
	char txBufUART[BT05_UART_BUF_LENGTH];
	for (uint8_t i=0; i<BT05_UART_BUF_LENGTH; i++)
		txBufUART[i] = 0;
	sprintf(txBufUART, "AT+ROLE%s", data);
	BT05_UART_SendStringCRLF(txBufUART);
	if (!BT05_UART_ReceiveStringCRLF(6+len)) {
		sprintf(txBufUART, "+ROLE=%s", data);
		HAL_Delay(700);
		return (!strcmp(txBufUART, (char*)&rxBufUART)) ? OK : ERR_ROLE_ACK;
	}
	return ERR_SET_ROLE;
}

uint8_t BT05_CheckUUID(char *data) {												// Check BLE UUID
	uint8_t len = strlen(data);
	BT05_UART_SendStringCRLF("AT+UUID");
	if (!BT05_UART_ReceiveStringCRLF(6+len)) {
		for (uint8_t i=6; i<BT05_UART_BUF_LENGTH; i++) {
			rxBufUART[i-6] = rxBufUART[i];
			rxBufUART[i] = 0;
		}
		return (!strcmp(data, (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	}
	return ERR_TIMEOUT;
}

uint8_t BT05_SetUUID(char *data) {													// Set new BLE UUID
	uint8_t len = strlen(data);
	if (len != 6)
		return ERR_UUID_LEN;
	if (!BT05_CheckUUID(data))
		return OK;
	char txBufUART[BT05_UART_BUF_LENGTH];
	for (uint8_t i=0; i<BT05_UART_BUF_LENGTH; i++)
		txBufUART[i] = 0;
	sprintf(txBufUART, "AT+UUID%s", data);
	BT05_UART_SendStringCRLF(txBufUART);
	if (!BT05_UART_ReceiveStringCRLF(6+len)) {
		sprintf(txBufUART, "+UUID=%s", data);
		return (!strcmp(txBufUART, (char*)&rxBufUART)) ? OK : ERR_UUID_ACK;
	}
	return ERR_SET_UUID;
}

uint8_t BT05_CheckCHAR(char *data) {												// Check BLE characteristic
	uint8_t len = strlen(data);
	BT05_UART_SendStringCRLF("AT+CHAR");
	if (!BT05_UART_ReceiveStringCRLF(6+len)) {
		for (uint8_t i=6; i<BT05_UART_BUF_LENGTH; i++) {
			rxBufUART[i-6] = rxBufUART[i];
			rxBufUART[i] = 0;
		}
		return (!strcmp(data, (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	}
	return ERR_TIMEOUT;
}

uint8_t BT05_SetCHAR(char *data) {													// Set new BLE characteristic
	uint8_t len = strlen(data);
	if (len != 6)
		return ERR_CHAR_LEN;
	if (!BT05_CheckCHAR(data))
		return OK;
	char txBufUART[BT05_UART_BUF_LENGTH];
	for (uint8_t i=0; i<BT05_UART_BUF_LENGTH; i++)
		txBufUART[i] = 0;
	sprintf(txBufUART, "AT+CHAR%s", data);
	BT05_UART_SendStringCRLF(txBufUART);
	if (!BT05_UART_ReceiveStringCRLF(6+len)) {
		sprintf(txBufUART, "+CHAR=%s", data);
		return (!strcmp(txBufUART, (char*)&rxBufUART)) ? OK : ERR_CHAR_ACK;
	}
	return ERR_SET_CHAR;
}

uint8_t BT05_CheckNoti(char *data) {												// Check whether notifications enabled or not
	uint8_t len = strlen(data);
	BT05_UART_SendStringCRLF("AT+NOTI");
	if (!BT05_UART_ReceiveStringCRLF(6+len))
		return (data[0] == rxBufUART[6]) ? OK : ERR_STRCMP;
	return ERR_TIMEOUT;
}

uint8_t BT05_SetNoti(char *data) {													// Turn on or off notification about BLE device connection
	uint8_t len = strlen(data);
	if (len != 1)
		return ERR_NOTI_LEN;
	if (!BT05_CheckNoti(data))
		return OK;
	char txBufUART[BT05_UART_BUF_LENGTH];
	for (uint8_t i=0; i<BT05_UART_BUF_LENGTH; i++)
		txBufUART[i] = 0;
	sprintf(txBufUART, "AT+NOTI%s", data);
	BT05_UART_SendStringCRLF(txBufUART);
	if (!BT05_UART_ReceiveStringCRLF(6+len)) {
		sprintf(txBufUART, "+NOTI=%s", data);
		HAL_Delay(50);
		return (!strcmp(txBufUART, (char*)&rxBufUART)) ? OK : ERR_NOTI_ACK;
	}
	return ERR_SET_NOTI;
}

uint8_t BT05_GetAddress(void) {														// Get module address (will be written to BT05_deviceAddress)
	BT05_UART_SendStringCRLF("AT+ADDR");
	if (!BT05_UART_ReceiveStringCRLF(23)) {
		for (uint8_t i=0; i<18; i++)
			BT05_deviceAddress[i] = rxBufUART[i+6];
		return OK;
	}
	return ERR_GET_ADDR;
}

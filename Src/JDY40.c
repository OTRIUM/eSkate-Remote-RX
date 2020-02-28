/*****************************************************************************
 * @file     JDY40.c
 * @brief    JDY40 Library header
 * @version  V2.00
 * @date     29. August 2019
 *****************************************************************************/

/***************************************
     JDY40 RF Module HAL Library
     for USART Full-Duplex mode
***************************************/

#define JDY40_HUART_NUMBER			huart1

#define JDY40_DESIRED_BAUD			9600											// 2400 - 4800 - 9600 - 14400 - 19200
#define JDY40_DESIRED_RFID			"1488"											// RF ID, 0000 to FFFF
#define JDY40_DESIRED_DVID			"1337"											// Device ID, 0000 to FFFF
#define JDY40_DESIRED_RFC			"064"											// RF Channel, 001 to 128
#define JDY40_DESIRED_POWER			"9"												// RF Power, 0 to 9
#define JDY40_DESIRED_MODE			"A0"											// Device mode, A0 - C0 - C1 - C2 - C3 - C4 - C5, RTFM for more details

#define __JDY40_SLEEP_ON			HAL_GPIO_WritePin(RF_CS_GPIO_Port, RF_CS_Pin, GPIO_PIN_SET)			// Low == Active; High == Sleep
#define __JDY40_SLEEP_OFF			HAL_GPIO_WritePin(RF_CS_GPIO_Port, RF_CS_Pin, GPIO_PIN_RESET)
#define __JDY40_CONFIGMODE_ON		HAL_GPIO_WritePin(RF_SET_GPIO_Port, RF_SET_Pin, GPIO_PIN_RESET)		// Low == Config Mode; High == Communication Mode
#define __JDY40_CONFIGMODE_OFF		HAL_GPIO_WritePin(RF_SET_GPIO_Port, RF_SET_Pin, GPIO_PIN_SET)

#define JDY40_UART_BUF_LENGTH		25


#include "main.h"
#include "JDY40.h"
#include "stm32f1xx_hal.h"

#include <string.h>
#include <stdio.h>


extern UART_HandleTypeDef JDY40_HUART_NUMBER;

uint8_t rxBufUART[JDY40_UART_BUF_LENGTH];

static const uint32_t baudList[5] = {2400, 4800, 9600, 14400, 19200};				// Supported baud rates

typedef enum JDY40_StatusList
{
	OK		 			= 0x00U,
	ERR_TIMEOUT 		= 0x01U,
	ERR_STRCMP 			= 0x02U,
	ERR_BAUD_INCORRECT 	= 0x03U,
	ERR_SET_BAUD		= 0x04U,
	ERR_NO_BAUD_ACK		= 0x05U,
	ERR_RFID_LEN		= 0x06U,
	ERR_NO_RESPONSE		= 0x07U,
	ERR_SET_RFID		= 0x08U,
	ERR_DVID_LEN		= 0x09U,
	ERR_SET_DVID		= 0x0AU,
	ERR_RFC_LEN			= 0x0BU,
	ERR_SET_RFC			= 0x0CU,
	ERR_POWE_LEN		= 0x0DU,
	ERR_SET_POWE		= 0x0FU,
	ERR_CLSS_LEN		= 0x10U,
	ERR_SET_CLSS		= 0x11U,
	ERR_JDY40_CONFIGURE	= 0x12U

} JDY40_StatusList;

uint8_t JDY40_Configure(void) {														// Configure the module using AT-commands
	__JDY40_SLEEP_OFF;
	__JDY40_CONFIGMODE_ON;
	HAL_Delay(250);
	if (!JDY40_SendCommands()) {
		HAL_Delay(10);
		__JDY40_CONFIGMODE_OFF;
		HAL_Delay(5);
		return OK;
	}
	HAL_Delay(100);
	if (!JDY40_SendCommands()) {
		HAL_Delay(10);
		__JDY40_CONFIGMODE_OFF;
		HAL_Delay(5);
		return OK;
	}
	HAL_Delay(10);
	__JDY40_CONFIGMODE_OFF;
	HAL_Delay(5);
	return ERR_JDY40_CONFIGURE;
}


uint8_t JDY40_SendCommands(void) {													// Send configuration commands
	uint8_t errCounter = 0;
	errCounter += JDY40_SetBaud(JDY40_DESIRED_BAUD);
	HAL_Delay(10);
	errCounter += JDY40_SetRFID(JDY40_DESIRED_RFID);
	HAL_Delay(10);
	errCounter += JDY40_SetDVID(JDY40_DESIRED_DVID);
	HAL_Delay(10);
	errCounter += JDY40_SetRFC(JDY40_DESIRED_RFC);
	HAL_Delay(10);
	errCounter += JDY40_SetPOWE(JDY40_DESIRED_POWER);
	HAL_Delay(10);
	errCounter += JDY40_SetCLSS(JDY40_DESIRED_MODE);
	HAL_Delay(10);
	return errCounter;
}

void JDY40_UART_SendStringCRLF(char *data) {										// Send a string of chars with CR&LF
	uint8_t len = strlen(data);
	HAL_UART_Transmit(&JDY40_HUART_NUMBER, (uint8_t*)data, len, 50);
	HAL_UART_Transmit(&JDY40_HUART_NUMBER, (uint8_t*)"\r\n", 2, 50);
}

uint8_t JDY40_UART_ReceiveStringCRLF(uint8_t len) {									// Receive N bytes of data
	for (uint8_t i=0; i<JDY40_UART_BUF_LENGTH; i++)
		rxBufUART[i] = 0;
	__HAL_UART_FLUSH_DRREGISTER(&JDY40_HUART_NUMBER);
	if (HAL_UART_Receive(&JDY40_HUART_NUMBER, rxBufUART, (len+2), 50) == HAL_OK) {
		rxBufUART[len] = 0;
		rxBufUART[len+1] = 0;
		return OK;
	}
	return ERR_TIMEOUT;
}

void JDY40_UART_ChangeBaudRate(uint32_t baud) {										// Change the baudrate of selected UART
	JDY40_HUART_NUMBER.Init.BaudRate = baud;
	HAL_UART_Init(&JDY40_HUART_NUMBER);
	for (uint16_t i=0; i<100; i++)
		__NOP();
}

uint8_t JDY40_CheckPresence(void) {													// Check whether the module is present using AT command
	JDY40_UART_SendStringCRLF("AT+BAUD");
	if (!JDY40_UART_ReceiveStringCRLF(7))
		return (!strncmp("+BAUD=", (char*)&rxBufUART, 6)) ? OK : ERR_STRCMP;
	return ERR_TIMEOUT;
}

uint8_t JDY40_FindRightBaud(void) {													// Find correct baudrate using bruteforce and AT command
	for (uint8_t baudNumber=2; baudNumber<7; baudNumber++) {
		JDY40_UART_ChangeBaudRate(baudList[baudNumber-2]);
		if (!JDY40_CheckPresence())
			return baudNumber;
		HAL_Delay(25);
	}
	return ERR_NO_RESPONSE;
}

uint8_t JDY40_SetBaud(uint16_t baud) {												// Set the module to desired baudrate
	JDY40_UART_ChangeBaudRate(baud);
	if (!JDY40_CheckPresence())
		return OK;
	HAL_Delay(10);
	if (JDY40_FindRightBaud() != ERR_NO_RESPONSE) {
		HAL_Delay(10);
		switch(baud) {
		case 2400:
			JDY40_UART_SendStringCRLF("AT+BAUD2");
			break;
		case 4800:
			JDY40_UART_SendStringCRLF("AT+BAUD3");
			break;
		case 9600:
			JDY40_UART_SendStringCRLF("AT+BAUD4");
			break;
		case 14400:
			JDY40_UART_SendStringCRLF("AT+BAUD5");
			break;
		case 19200:
			JDY40_UART_SendStringCRLF("AT+BAUD6");
			break;
		default:
			return ERR_BAUD_INCORRECT;
		}
		if (!JDY40_UART_ReceiveStringCRLF(2) && !strcmp("OK", (char*)&rxBufUART)) {
			JDY40_UART_ChangeBaudRate(baud);
			HAL_Delay(400);
			JDY40_CheckPresence();													// This empty call is necessary because the module is a bit fucked up and it doesn't response correctly for the first time after the baudrate change
			return !JDY40_CheckPresence() ? OK : ERR_NO_BAUD_ACK;
		}
		return ERR_SET_BAUD;
	}
	return ERR_NO_RESPONSE;
}

uint8_t JDY40_CheckRFID(char *data) {
	uint8_t len = strlen(data);
	JDY40_UART_SendStringCRLF("AT+RFID");
	if (!JDY40_UART_ReceiveStringCRLF(6+len)) {
		for (uint8_t i=6; i<JDY40_UART_BUF_LENGTH; i++) {
			rxBufUART[i-6] = rxBufUART[i];
			rxBufUART[i] = 0;
		}
		return (!strcmp(data, (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	}
	return ERR_TIMEOUT;
}

uint8_t JDY40_SetRFID(char *data) {
	uint8_t len = strlen(data);
	if (len != 4)
		return ERR_RFID_LEN;
	if (!JDY40_CheckRFID(data))
			return OK;
	HAL_Delay(10);
	char txBufUART[JDY40_UART_BUF_LENGTH];
	for (uint8_t i=0; i<JDY40_UART_BUF_LENGTH; i++)
			txBufUART[i] = 0;
	sprintf(txBufUART, "AT+RFID%s", data);
	JDY40_UART_SendStringCRLF(txBufUART);
	return (!JDY40_UART_ReceiveStringCRLF(2) && !strcmp("OK", (char*)&rxBufUART)) ? OK : ERR_SET_RFID;
}

uint8_t JDY40_CheckDVID(char *data) {
	uint8_t len = strlen(data);
	JDY40_UART_SendStringCRLF("AT+DVID");
	if (!JDY40_UART_ReceiveStringCRLF(6+len)) {
		for (uint8_t i=6; i<JDY40_UART_BUF_LENGTH; i++) {
			rxBufUART[i-6] = rxBufUART[i];
			rxBufUART[i] = 0;
		}
		return (!strcmp(data, (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	}
	return ERR_TIMEOUT;
}

uint8_t JDY40_SetDVID(char *data) {
	uint8_t len = strlen(data);
	if (len != 4)
		return ERR_DVID_LEN;
	if (!JDY40_CheckDVID(data))
			return OK;
	HAL_Delay(10);
	char txBufUART[JDY40_UART_BUF_LENGTH];
	for (uint8_t i=0; i<JDY40_UART_BUF_LENGTH; i++)
			txBufUART[i] = 0;
	sprintf(txBufUART, "AT+DVID%s", data);
	JDY40_UART_SendStringCRLF(txBufUART);
	return (!JDY40_UART_ReceiveStringCRLF(2) && !strcmp("OK", (char*)&rxBufUART)) ? OK : ERR_SET_DVID;
}

uint8_t JDY40_CheckRFC(char *data) {
	uint8_t len = strlen(data);
	JDY40_UART_SendStringCRLF("AT+RFC");
	if (!JDY40_UART_ReceiveStringCRLF(5+len)) {
		for (uint8_t i=5; i<JDY40_UART_BUF_LENGTH; i++) {
			rxBufUART[i-5] = rxBufUART[i];
			rxBufUART[i] = 0;
		}
		return (!strcmp(data, (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	}
	return ERR_TIMEOUT;
}

uint8_t JDY40_SetRFC(char *data) {
	uint8_t len = strlen(data);
	if (len != 3)
		return ERR_RFC_LEN;
	if (!JDY40_CheckRFC(data))
			return OK;
	HAL_Delay(10);
	char txBufUART[JDY40_UART_BUF_LENGTH];
	for (uint8_t i=0; i<JDY40_UART_BUF_LENGTH; i++)
			txBufUART[i] = 0;
	sprintf(txBufUART, "AT+RFC%s", data);
	JDY40_UART_SendStringCRLF(txBufUART);
	return (!JDY40_UART_ReceiveStringCRLF(2) && !strcmp("OK", (char*)&rxBufUART)) ? OK : ERR_SET_RFC;
}

uint8_t JDY40_CheckPOWE(char *data) {
	uint8_t len = strlen(data);
	JDY40_UART_SendStringCRLF("AT+POWE");
	if (!JDY40_UART_ReceiveStringCRLF(6+len)) {
		for (uint8_t i=6; i<JDY40_UART_BUF_LENGTH; i++) {
			rxBufUART[i-6] = rxBufUART[i];
			rxBufUART[i] = 0;
		}
		return (!strcmp(data, (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	}
	return ERR_TIMEOUT;
}

uint8_t JDY40_SetPOWE(char *data) {
	uint8_t len = strlen(data);
	if (len != 1)
		return ERR_POWE_LEN;
	if (!JDY40_CheckPOWE(data))
			return OK;
	HAL_Delay(10);
	char txBufUART[JDY40_UART_BUF_LENGTH];
	for (uint8_t i=0; i<JDY40_UART_BUF_LENGTH; i++)
			txBufUART[i] = 0;
	sprintf(txBufUART, "AT+POWE%s", data);
	JDY40_UART_SendStringCRLF(txBufUART);
	return (!JDY40_UART_ReceiveStringCRLF(2) && !strcmp("OK", (char*)&rxBufUART)) ? OK : ERR_SET_POWE;
}

uint8_t JDY40_CheckCLSS(char *data) {
	uint8_t len = strlen(data);
	JDY40_UART_SendStringCRLF("AT+CLSS");
	if (!JDY40_UART_ReceiveStringCRLF(6+len)) {
		for (uint8_t i=6; i<JDY40_UART_BUF_LENGTH; i++) {
			rxBufUART[i-6] = rxBufUART[i];
			rxBufUART[i] = 0;
		}
		return (!strcmp(data, (char*)&rxBufUART)) ? OK : ERR_STRCMP;
	}
	return ERR_TIMEOUT;
}

uint8_t JDY40_SetCLSS(char *data) {
	uint8_t len = strlen(data);
	if (len != 2)
		return ERR_CLSS_LEN;
	if (!JDY40_CheckCLSS(data))
			return OK;
	HAL_Delay(10);
	char txBufUART[JDY40_UART_BUF_LENGTH];
	for (uint8_t i=0; i<JDY40_UART_BUF_LENGTH; i++)
			txBufUART[i] = 0;
	sprintf(txBufUART, "AT+CLSS%s", data);
	JDY40_UART_SendStringCRLF(txBufUART);
	return (!JDY40_UART_ReceiveStringCRLF(2) && !strncmp("OK", (char*)&rxBufUART, 6)) ? OK : ERR_SET_CLSS;
}

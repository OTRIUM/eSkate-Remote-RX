/******************************************************************************
 * @file     JDY40.h
 * @brief    JDY40 Library header
 * @version  V2.00
 * @date     29. August 2019
 ******************************************************************************/

#ifndef JDY40_H
#define JDY40_H

#include "stm32f1xx_hal.h"

uint8_t JDY40_Configure(void);

uint8_t JDY40_SendCommands(void);

void JDY40_UART_SendStringCRLF(char *data);

uint8_t JDY40_UART_ReceiveStringCRLF(uint8_t len);

void JDY40_UART_ChangeBaudRate(uint32_t baud);

uint8_t JDY40_CheckPresence(void);

uint8_t JDY40_FindRightBaud(void);

uint8_t JDY40_SetBaud(uint16_t baud);

uint8_t JDY40_CheckRFID(char *data);

uint8_t JDY40_SetRFID(char *data);

uint8_t JDY40_CheckDVID(char *data);

uint8_t JDY40_SetDVID(char *data);

uint8_t JDY40_CheckRFC(char *data);

uint8_t JDY40_SetRFC(char *data);

uint8_t JDY40_CheckPOWE(char *data);

uint8_t JDY40_SetPOWE(char *data);

uint8_t JDY40_CheckCLSS(char *data);

uint8_t JDY40_SetCLSS(char *data);

#endif

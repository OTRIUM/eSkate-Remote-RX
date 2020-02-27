/******************************************************************************
 * @file     MLT-BT05.h
 * @brief    BT-05 Library header
 * @version  V1.00
 * @date     23. February 2020
 ******************************************************************************/

#ifndef MLTBT05_H
#define MLTBT05_H

#include "stm32f1xx_hal.h"

uint8_t BT05_Configure(void);

uint8_t BT05_SendCommands(void);

void UART_SendStringCRLF(char *data);

uint8_t UART_ReceiveStringCRLF(uint8_t len);

void UART_ChangeBaudRate(uint32_t baud);

uint8_t BT05_CheckPresence(void);

uint8_t BT05_FindRightBaud(void);

uint8_t BT05_SetBaud(uint32_t baud);

uint8_t BT05_CheckName(char *data);

uint8_t BT05_SetName(char *data);

uint8_t BT05_CheckPin(char *data);

uint8_t BT05_SetPin(char *data);

uint8_t BT05_CheckRole(char *data);

uint8_t BT05_SetRole(char *data);

uint8_t BT05_CheckUUID(char *data);

uint8_t BT05_SetUUID(char *data);

uint8_t BT05_CheckCHAR(char *data);

uint8_t BT05_SetCHAR(char *data);

uint8_t BT05_CheckNoti(char *data);

uint8_t BT05_SetNoti(char *data);

uint8_t BT05_GetAddress(void);

#endif

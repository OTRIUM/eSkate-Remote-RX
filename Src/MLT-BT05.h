/**************************************************************************//**
 * @file     BT-05.h
 * @brief    BT-05 Library header
 * @version  V1.00
 * @date     23. February 2020
 ******************************************************************************/

#ifndef MLTBT05_H
#define MLTBT05_H

#endif

#include "stm32f1xx_hal.h"

uint8_t LC12S_CheckSum_Calculation(void);

void LC12S_Init(void);
void LC12S_Enable_Sleep_Mode(void);
void LC12S_Disable_Sleep_Mode(void);

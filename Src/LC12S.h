/**************************************************************************//**
 * @file     LC12S.h
 * @brief    LC12S Library header
 * @version  V1.00
 * @date     15. August 2019
 ******************************************************************************/

#ifndef LC12S_H
#define LC12S_H

#endif

#include "stm32f1xx_hal.h"

uint8_t LC12S_CheckSum_Calculation(void);

void LC12S_Init(void);
void LC12S_Enable_Sleep_Mode(void);
void LC12S_Disable_Sleep_Mode(void);

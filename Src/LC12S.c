/**************************************************************************//**
 * @file     LC12S.h
 * @brief    LC12S Library header
 * @version  V1.00
 * @date     15. August 2019
 ******************************************************************************/

/***************************************
     LC12S RF Module HAL Library
     for USART 1 Full-Duplex mode
***************************************/

#include "main.h"
#include "LC12S.h"
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

#define __LC12S_SLEEP_ON			HAL_GPIO_WritePin(GPIOA, RF_CS_Pin, GPIO_PIN_SET)			// Low == Active; High == Sleep
#define __LC12S_SLEEP_OFF			HAL_GPIO_WritePin(GPIOA, RF_CS_Pin, GPIO_PIN_RESET)
#define __LC12S_CONFIGMODE_ON		HAL_GPIO_WritePin(GPIOA, RF_SET_Pin, GPIO_PIN_RESET)		// Low == Config Mode; High == Communication Mode
#define __LC12S_CONFIGMODE_OFF		HAL_GPIO_WritePin(GPIOA, RF_SET_Pin, GPIO_PIN_SET)

enum {
	LC12S_COMMAND_BYTE_0		= 0xAA,
	LC12S_COMMAND_BYTE_1 		= 0x5A,
	LC12S_SELF_ID_FIRST_BYTE	= 0x12,		/* Self ID = 0x1234 */
	LC12S_SELF_ID_SECOND_BYTE 	= 0x34,
	LC12S_NET_ID_FIRST_BYTE		= 0x14,		/* Net ID = 0x1488 	*/
	LC12S_NET_ID_SECOND_BYTE 	= 0x88,
	LC12S_RESERVED_BYTE_0		= 0x00,
	LC12S_POWER_BYTE			= 0x00,		/* 0x00 = 12 dBm	*/
	LC12S_RESERVED_BYTE_1		= 0x00,
	LC12S_BAUDRATE_BYTE			= 0x04,		/* 0x04 = 9600 bps	*/
	LC12S_RESERVED_BYTE_2		= 0x00,
	LC12S_RF_CHANNEL_BYTE		= 0x32,		/* Channel 32		*/
	LC12S_RESERVED_BYTE_3		= 0x00,
	LC12S_RESERVED_BYTE_4		= 0x00,
	LC12S_RESERVED_BYTE_5		= 0x00,
	LC12S_LENGTH_BYTE			= 0x12,
	LC12S_RESERVED_BYTE_6		= 0x00,
	LC12S_CHECK_SUM_BYTE		= 0x00
};

uint8_t LC12S_Registers_Table [18] = {
	LC12S_COMMAND_BYTE_0,
	LC12S_COMMAND_BYTE_1,
	LC12S_SELF_ID_FIRST_BYTE,
	LC12S_SELF_ID_SECOND_BYTE,
	LC12S_NET_ID_FIRST_BYTE,
	LC12S_NET_ID_SECOND_BYTE,
	LC12S_RESERVED_BYTE_0,
	LC12S_POWER_BYTE,
	LC12S_RESERVED_BYTE_1,
	LC12S_BAUDRATE_BYTE,
	LC12S_RESERVED_BYTE_2,
	LC12S_RF_CHANNEL_BYTE,
	LC12S_RESERVED_BYTE_3,
	LC12S_RESERVED_BYTE_4,
	LC12S_RESERVED_BYTE_5,
	LC12S_LENGTH_BYTE,
	LC12S_RESERVED_BYTE_6,
	LC12S_CHECK_SUM_BYTE
};

uint8_t LC12S_CheckSum_Calculation(void) {
	uint16_t bytesSum = 0;
	for (uint8_t i=0; i<17; i++)
		bytesSum += LC12S_Registers_Table[i];
	return (bytesSum % 8);
}

void LC12S_Init(void) {
	__LC12S_SLEEP_OFF;
	__LC12S_CONFIGMODE_ON;

	HAL_Delay(10);

	LC12S_Registers_Table[17] = LC12S_CheckSum_Calculation();

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
	  Error_Handler();
	}

	HAL_UART_Transmit(&huart1, LC12S_Registers_Table, 18, 1000);
	HAL_Delay(50);

	HAL_Delay(10);

	__LC12S_CONFIGMODE_OFF;
}

void LC12S_Enable_Sleep_Mode(void) {
	__LC12S_SLEEP_ON;
}

void LC12S_Disable_Sleep_Mode(void) {
	__LC12S_SLEEP_OFF;
}

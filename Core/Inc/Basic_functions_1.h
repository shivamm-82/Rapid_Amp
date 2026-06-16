/*
 * Basic_functions_1.h
 *
 *  Created on: Dec 23, 2025
 *      Author: Admin
 */

#include "main.h"
#ifndef INC_BASIC_FUNCTIONS_1_H_
#define INC_BASIC_FUNCTIONS_1_H_

#define MOVING_BUFFER_SIZE_TEMP 50  // Number of samples for the running average
#define MOVING_BUFFER_SIZE_PHOTO 2000
//uint16_t adcBuffer[ADC_BUFFER_SIZE] = {0};  // Circular buffer for ADC readings
//uint8_t bufferIndex = 0;  // Position in buffer
//uint32_t sum=0;  // Sum of ADC values for averaging


/************LED excite parameter start ************************/
/* Define LED IDs */
//#define SIZE_OF_PHOTO_DATA_CAPTURE 30
//typedef struct{
//	uint32_t Buff[SIZE_OF_PHOTO_DATA_CAPTURE];
//}POSITION;

typedef enum
{
  GPIO_PIN_LOW = 0u,
  GPIO_PIN_HIGH
} GPIO_Pin_Status;

typedef enum {
    LED_1 = 1,
    LED_2,
    LED_3,
    LED_4,
	LED_5,
	LED_6,
	LED_7,
	LED_MAX,
} LED_ID;

/* Struct to hold GPIO config for each LED */
typedef struct {
    GPIO_TypeDef* port;
    uint16_t      pin;
} LED_Config_t;

/* Map each LED ID to its port and pin */
static const LED_Config_t LED_Map[LED_MAX] = {
    [LED_1] = { D1_Excite_LED_GPIO_Port, D1_Excite_LED_Pin },
    [LED_2] = { D2_Excite_LED_GPIO_Port, D2_Excite_LED_Pin },
    [LED_3] = { D3_Excite_LED_GPIO_Port, D3_Excite_LED_Pin },
    [LED_4] = { D4_Excite_LED_GPIO_Port, D4_Excite_LED_Pin },
	[LED_5] = { D5_Excite_LED_GPIO_Port, D5_Excite_LED_Pin },
	[LED_6] = { D6_Excite_LED_GPIO_Port, D6_Excite_LED_Pin },
	[LED_7] = { D7_Excite_LED_GPIO_Port, D7_Excite_LED_Pin },
};
/************LED excite parameter END ************************/




typedef struct
{
float ADC_sense_TEMP;
float ADC_sense_Photo_out;

}ADC_read;

typedef struct
{
	volatile uint16_t buffer_index;
	volatile uint16_t Buffer_TEMP[MOVING_BUFFER_SIZE_TEMP];
	volatile uint16_t Buffer_PHOTO[MOVING_BUFFER_SIZE_PHOTO];
	volatile uint16_t ADC_sense_Temp;
	volatile uint16_t ADC_sense_photo_out;
	volatile uint32_t sum;
}Moving_average;






#endif /* INC_BASIC_FUNCTIONS_1_H_ */



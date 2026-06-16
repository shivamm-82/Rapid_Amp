/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Photo_out_ADC1_ch12_Pin GPIO_PIN_2
#define Photo_out_ADC1_ch12_GPIO_Port GPIOC
#define NTC_Temp_Sense_ADC1_CH13_Pin GPIO_PIN_3
#define NTC_Temp_Sense_ADC1_CH13_GPIO_Port GPIOC
#define Buzzer_Pin GPIO_PIN_0
#define Buzzer_GPIO_Port GPIOA
#define QR_EN_Pin GPIO_PIN_4
#define QR_EN_GPIO_Port GPIOA
#define Turn_off_mosfet_for_power_off_Pin GPIO_PIN_5
#define Turn_off_mosfet_for_power_off_GPIO_Port GPIOC
#define Heater_PWM_T3CH3_Pin GPIO_PIN_0
#define Heater_PWM_T3CH3_GPIO_Port GPIOB
#define SWITCH_INT_Button_Pin GPIO_PIN_1
#define SWITCH_INT_Button_GPIO_Port GPIOB
#define SWITCH_INT_Button_EXTI_IRQn EXTI1_IRQn
#define D1_Excite_LED_Pin GPIO_PIN_10
#define D1_Excite_LED_GPIO_Port GPIOB
#define D2_Excite_LED_Pin GPIO_PIN_11
#define D2_Excite_LED_GPIO_Port GPIOB
#define RB__LED_RED_Pin GPIO_PIN_12
#define RB__LED_RED_GPIO_Port GPIOB
#define RB_LED_BLUE_Pin GPIO_PIN_13
#define RB_LED_BLUE_GPIO_Port GPIOB
#define RED_LED_Pin GPIO_PIN_6
#define RED_LED_GPIO_Port GPIOC
#define Green_LED_Pin GPIO_PIN_7
#define Green_LED_GPIO_Port GPIOC
#define D5_Excite_LED_Pin GPIO_PIN_8
#define D5_Excite_LED_GPIO_Port GPIOC
#define D3_Excite_LED_Pin GPIO_PIN_9
#define D3_Excite_LED_GPIO_Port GPIOC
#define D4_Excite_LED_Pin GPIO_PIN_15
#define D4_Excite_LED_GPIO_Port GPIOA
#define UART_3_TX_BLE_communication_Pin GPIO_PIN_10
#define UART_3_TX_BLE_communication_GPIO_Port GPIOC
#define UART_3_RX_BLE_communication_Pin GPIO_PIN_11
#define UART_3_RX_BLE_communication_GPIO_Port GPIOC
#define D7_Excite_LED_Pin GPIO_PIN_4
#define D7_Excite_LED_GPIO_Port GPIOB
#define D6_Excite_LED_Pin GPIO_PIN_5
#define D6_Excite_LED_GPIO_Port GPIOB
#define USB_pull_up_pb8_Pin GPIO_PIN_8
#define USB_pull_up_pb8_GPIO_Port GPIOB
#define Hall_Sensor_OUT_Pin GPIO_PIN_9
#define Hall_Sensor_OUT_GPIO_Port GPIOB
#define Hall_Sensor_OUT_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

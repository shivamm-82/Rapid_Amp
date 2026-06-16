/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "string.h"
#include <stdint.h>
#include <NTC_calculate_temp.h>
#include <PID.h>
#include "Basic_functions_1.h"
#include "Result_interpretation.h"
#include <math.h>
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
volatile uint16_t ADC_VAL_DMA[2]={0},ADC_Average_result[2]={0};
volatile uint32_t ADC_VAL=0,count_button=0,TIM4_count=0,LID_Count=0,Second_count_in_Timer=0,minute_counter_in_Timer=0;
volatile uint8_t PRE_HEAT_FLAG=0,Count_Excite_LED_position=0,START_Reading_Switch=0,Data_Aquisition_flag_photo=0,Minutes_is_less_30_minutes=0,BUZZER_FLAG=0,SOMETHING_DETECTED=0,GET_USB_Status=0;
float Current_Temp=0.0f,Set_Target_Temp=62.0f;
volatile uint32_t three_sec_count=0;

//BLE
volatile   uint8_t DATA_RECIEVED = 0; //for ble data reciving
uint8_t local_buff_BLE[10] = {0};
//BLE variables
#define RX_BLE_BUFF 1
uint8_t RX_buff[RX_BLE_BUFF];


//from usbd_conf.c  file
volatile uint8_t CABLE_PLUGGED=0,CABLE_REMOVED=0;
extern volatile uint8_t START_COMMAND_FROM_PC,STOP_COMMAND_FROM_PC;


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
typedef enum
{
  TEMP_OK       = 0x00U,
  TEMP_ERROR    = 0x01U,
  TEMP_SHORT    = 0x02U,
  TEMP_OPEN     = 0x03U
} TEMP_StatusTypeDef;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */
void Preheat_loop(void);
TEMP_StatusTypeDef check_Temprature_sensor_available(uint32_t ADC_VAL);
void Turn_off_Rest_of_the_Turned_on_LED(LED_ID id,GPIO_Pin_Status status);
void Trigger_LED_Channel(LED_ID id,GPIO_Pin_Status status);
void Turn_off_All_LED(void);
void Turn_on_All_LED(void);
void Trigger_led_Aquire_data(uint8_t temp_count);
void Reset_buffer_index(void);
void Buzzer_Test_Complete_tone(void);
void Buzzer_power_on_tone(void);
void send_usb_string(const char* str);
void send_BLE_using_USART3_string(const char* str);
void Excite_Aquisition_and_calculate_Result(POSITION *well_ptrs[NUM_CHANNELS]);
void Device_ready_Tone(void);

Moving_average Average_TEMP={0},Average_Photo_D={0};

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/*
 ADC sampling time = (sampling time + conversion cycle )/ADC_CLOCK_FRQ

 ADC sampling time = (239.5 + 12.5)/0.5Mhz =0.48ms

 adc will fire after every 0.48ms and result will get store in moving average

 * */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{

/*******************average of Photo Diode DATA Start **********************************/
	if(Data_Aquisition_flag_photo)
	{
	uint16_t STORE_LOCAL_ADC_DATA_ONCE_PHOTO[1]= {0};
	STORE_LOCAL_ADC_DATA_ONCE_PHOTO[0] = ADC_VAL_DMA[0];
	Average_Photo_D.sum-=Average_Photo_D.Buffer_PHOTO[Average_Photo_D.buffer_index];
	Average_Photo_D.Buffer_PHOTO[Average_Photo_D.buffer_index] = STORE_LOCAL_ADC_DATA_ONCE_PHOTO[0];
	Average_Photo_D.sum += STORE_LOCAL_ADC_DATA_ONCE_PHOTO[0];
	Average_Photo_D.buffer_index = (Average_Photo_D.buffer_index + 1)%MOVING_BUFFER_SIZE_PHOTO;   //UPDATE circuler  index
	ADC_Average_result[0] = Average_Photo_D.sum / MOVING_BUFFER_SIZE_PHOTO;
	}


/*******************average of Photo Diode DATA END **********************************/


/*******************average of TEMP DATAe DATA Start **********************************/

	//average of TEMP DATA
	uint16_t STORE_LOCAL_ADC_DATA_ONCE_TEMP_NTC[1]= {0};
	STORE_LOCAL_ADC_DATA_ONCE_TEMP_NTC[0] = ADC_VAL_DMA[1];
	Average_TEMP.sum-=Average_TEMP.Buffer_TEMP[Average_TEMP.buffer_index];
	Average_TEMP.Buffer_TEMP[Average_TEMP.buffer_index] = STORE_LOCAL_ADC_DATA_ONCE_TEMP_NTC[0];
	Average_TEMP.sum += STORE_LOCAL_ADC_DATA_ONCE_TEMP_NTC[0];
	Average_TEMP.buffer_index = (Average_TEMP.buffer_index + 1)%MOVING_BUFFER_SIZE_TEMP;   //UPDATE circuler  index
	ADC_Average_result[1] = Average_TEMP.sum / MOVING_BUFFER_SIZE_TEMP;

/*******************average of TEMP DATA DATA END **********************************/
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{

  /****************************Preheating 1 sec counter & MINUTE COUNTER Start *****************************************************/
	if(START_Reading_Switch){minute_counter_in_Timer++;}
	Second_count_in_Timer++;
	 if((Second_count_in_Timer >=20) && PRE_HEAT_FLAG)
	 {
		 //Pre Heat LED indicate every 1 sec toggle
		 HAL_GPIO_TogglePin(RB__LED_RED_GPIO_Port, RB__LED_RED_Pin);
		 Second_count_in_Timer = 0;
	 }
	 else if((Second_count_in_Timer >=20) && PRE_HEAT_FLAG == 0)
	 	 {
	 	//on going  TEST LED indicate every 1 Sec toggle
		 if(!SOMETHING_DETECTED){HAL_GPIO_TogglePin(RB_LED_BLUE_GPIO_Port, RB_LED_BLUE_Pin);}   //THIS IS USED WHEN TEST IS COMPLETED TO STOP BLINKING BLUE LED REFER RESULT_INTERPRETATION.C

	 		 //buzzer off
	 		  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off

	 		 //Buzzer toggle
	 		 if(START_Reading_Switch && BUZZER_FLAG && !PRE_HEAT_FLAG){
	 		 HAL_GPIO_TogglePin(Buzzer_GPIO_Port, Buzzer_Pin);
	 	  	 BUZZER_FLAG =0;
	 		 }
	 		 Second_count_in_Timer = 0;
	 	 }
	 else
	 {
		if(Second_count_in_Timer > 15000)
		{
			Second_count_in_Timer = 0;  //reset
		}
	 }

	 if(minute_counter_in_Timer > 6000)
	 {
		 minute_counter_in_Timer = 0;
	 }
 /****************************Preheating 1 Sec counter & MINUTE COUNTER END *****************************************************/







  /****************************Button logic 3 second press power cut off Start *****************************************************/
	 if (htim->Instance == TIM4) // Replace TIMx with your configured timer (e.g., htim2.Instance)
	  {
		 if((HAL_GPIO_ReadPin(SWITCH_INT_Button_GPIO_Port, SWITCH_INT_Button_Pin) == 0) && count_button)
		 {
		 TIM4_count++;// Do your elapsed time task here (e.g., toggle LED)
		 if(TIM4_count > 60)
		 {
			 //turn off mcu power after 3 sec
			 HAL_GPIO_WritePin(Turn_off_mosfet_for_power_off_GPIO_Port, Turn_off_mosfet_for_power_off_Pin, 0);
		 }
		 }
		 else
		 {
			 TIM4_count=0;
		 }
	  }
	/****************************Button logic 3 second press power cut off END *****************************************************/







	 /****************************Calculate Temperature every 50ms and Heater PID power Start **************************************/
	 static PIDController Heater_Power = {0};
	 if(check_Temprature_sensor_available(ADC_Average_result[1]) != TEMP_OK)
		{
			  Error_Handler();
	    }
	    Current_Temp=Calculate_temprature(ADC_Average_result[1]);
	    PID_Decision(&Heater_Power, 200.0f, 0.01f,0.0f, Set_Target_Temp);   //target power iS 5W
	    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, PID_Compute_for_Heater_Power(&Heater_Power, Current_Temp, 1));  // TIM1->CCR1 = duty;
    /****************************Calculate Temperature every 50ms and Heater PID power END ****************************************/



   /*********************************USB FLAG CONNECTED DISCONNED RESPOND HERE START***********************************************/
	    if(START_COMMAND_FROM_PC)
	   	    	 {
	   	    		START_COMMAND_FROM_PC =0;
	   	    		START_Reading_Switch = 1;  //start from PC command
	   	    		BUZZER_FLAG = 1;  //send start command in main loop
	   	    	    send_usb_string("{0x70,0x04}\n"); //ack start
	   	    	 }
	   	    	 else if(STOP_COMMAND_FROM_PC)
	   	    	 {
	   	    		 STOP_COMMAND_FROM_PC = 0;
	   	    		 send_usb_string("{0x70,0x05}\n"); //ack stop
	   	    		 Error_Handler();
	   	    	 }

   /*********************************USB FLAG CONNECTED DISCONNED RESPOND HERE END***********************************************/
}


//BLE interrupt uart 3
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  	if(huart->Instance == USART3)
  	{
  		static uint8_t store_data_in_buffer = 0;
  		static uint8_t count_recieved_packets = 0;

  		if(RX_buff[0] == 0x7B)   // {
  		{
  			store_data_in_buffer = 1;
  		}
  		else if((RX_buff[0] == 0x7D) && store_data_in_buffer == 1)  // {
  		{
  			store_data_in_buffer = 0;
  			local_buff_BLE[count_recieved_packets++] = RX_buff[0];
  			 if(strcmp((char*)local_buff_BLE,"{0x04}") == 0)  //start command
  			        	 {
  			        		send_BLE_using_USART3_string("{0x70,0x04}\n"); //ack start
  			        		 count_recieved_packets = 0;
  			        		START_Reading_Switch = 1;  //start from PC command
  			        		memset(local_buff_BLE,0,10);
  			        	 }
  			        	 else if(strcmp((char*)local_buff_BLE,"{0x05}") == 0)
  			        	 {
  			        		 send_BLE_using_USART3_string("{0x70,0x05}\n"); //ack start
  			        		 count_recieved_packets = 0;
  			        		 memset(local_buff_BLE,0,10);
  			        		 Error_Handler();
  			        	 }
  		}
  		 if(store_data_in_buffer)
  		{
  			local_buff_BLE[count_recieved_packets++] = RX_buff[0];
  		}
  		else if(strcmp((char*)RX_buff,"}") == 0 && store_data_in_buffer == 1)
  		{
  			store_data_in_buffer = 0;
  		}


  		//different logic
//         if(RX_buff[0] == 0x0A)    //new line
//         {
//        	 DATA_RECIEVED = 1;
//        	 count_recieved_packets = 0; //reset index
//         }
//         else
//         {
//        	 local_buff_BLE[count_recieved_packets] = RX_buff[0];
//        	 if(count_recieved_packets++ > 9)
//        	 {
//        		 memset(local_buff_BLE,0,10);
//        		 count_recieved_packets = 0;
//        	 }
//
//         }
//
//         if(DATA_RECIEVED)
//         {
//        	 DATA_RECIEVED = 0;
//        	 if(strcmp((char*)local_buff_BLE,"{0x04}") == 0)  //start command
//        	 {
//        		send_BLE_using_USART3_string("{0x70,0x04}\n"); //ack start
//        		 count_recieved_packets = 0;
//        		START_Reading_Switch = 1;  //start from PC command
//        		memset(local_buff_BLE,0,10);
//        	 }
//        	 else if(strcmp((char*)local_buff_BLE,"{0x05}") == 0)
//        	 {
//        		 send_BLE_using_USART3_string("{0x70,0x05}\n"); //ack start
//        		 count_recieved_packets = 0;
//        		 memset(local_buff_BLE,0,10);
//        		 Error_Handler();
//        	 }
//         }



//
//            if(strcmp((char*)RX_buff,"{0x04}") == 0)
//            {
//            	send_BLE_using_USART3_string("{0x70,0x04}\n"); //ack start
//            }
//            else if(strcmp((char*)RX_buff,"{0x05}") == 0)
//            {
//            	send_BLE_using_USART3_string("{0x70,0x05}\n"); //ack start
//            }
//            memset(RX_buff,0,sizeof(RX_buff));
            HAL_UART_Receive_IT(&huart3,RX_buff,RX_BLE_BUFF);
  	}
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USB_DEVICE_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */




  /***************************************************************************************
   *
   *
   *  All Setup and initialization  Start
   *
   * **************************************************************************************/

  //start BLE interrupt reciever in UART3
  HAL_UART_Receive_IT(&huart3,RX_buff,RX_BLE_BUFF);

//Set Power pin high to get MCU power
  HAL_GPIO_WritePin(Turn_off_mosfet_for_power_off_GPIO_Port, Turn_off_mosfet_for_power_off_Pin, 1);
//Reset all LED to Low
  HAL_GPIO_WritePin(RB_LED_BLUE_GPIO_Port, RB_LED_BLUE_Pin, 1); //off
  HAL_GPIO_WritePin(RB__LED_RED_GPIO_Port, RB__LED_RED_Pin, 1); //off
  HAL_GPIO_WritePin(Green_LED_GPIO_Port, Green_LED_Pin, 1);  //off
  HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, 1);  //off
  HAL_GPIO_WritePin(QR_EN_GPIO_Port, QR_EN_Pin, 1);  //off
  HAL_GPIO_WritePin(USB_pull_up_pb8_GPIO_Port, USB_pull_up_pb8_Pin, 1);  //off



  //buzzer off
  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off
  HAL_Delay(2000);

  //Turn off All Excitation LEDs
  Turn_off_All_LED();

  //Start DMA for ADC scanning
  HAL_Delay(10);
  HAL_ADCEx_Calibration_Start(&hadc1);  // ← add this
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_VAL_DMA, 2);
  HAL_Delay(10);


  //Toggle buzzer thrice indicates power on
  Buzzer_power_on_tone();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  POSITION WELL_1={0},WELL_2={0},WELL_3={0},WELL_4={0},WELL_5={0},WELL_6={0},WELL_7={0};
  // Pointer array — no extra memory, just pointers to existing structs
  POSITION *wells[7] = {&WELL_1, &WELL_2, &WELL_3, &WELL_4,&WELL_5, &WELL_6, &WELL_7};



  //Heater PWM Start
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);



  //PCR result initialization all bufferes gets reset here
   PCR_Init(wells);
  /***************************************************************************************
   *
   *
   *  All Setup and initialization  END
   *
   * **************************************************************************************/














  /******************************Preheat loop********************************************************/

  Preheat_loop();


  /****************************Preheat loop END***********************************************************/













  //Dont blink On going test LED until it started from Any trigger source
  SOMETHING_DETECTED = 1 ;





  /*****************************************************************************************************

   * FOR EXCITING LEDS
   * Moving TO  clockwise from well 1 to well 7
   *
   *   well 1 - LED5
   *   well 2 - LED6
   *   well 3 - LED7
   *   well 4 - LED4
   *   well 5 - LED2
   *   well 6 - LED3
   *   well 7 - LED1
   *
   * *************************************************************************************/

  HAL_Delay(1000);
  send_usb_string("{0x03}\n"); //send Insert Cartridge and close Lid to PC
  send_BLE_using_USART3_string("{0x03}\n");  //send Insert Cartridge and close Lid BLE
  HAL_Delay(1000);


  Device_ready_Tone();
  HAL_Delay(1000);



  HAL_GPIO_WritePin(RB_LED_BLUE_GPIO_Port, RB_LED_BLUE_Pin, 0); //on
  while(!START_Reading_Switch)
  {
	  HAL_Delay(100);
  }
  //blink On going test LED until it started from Any trigger source
   SOMETHING_DETECTED = 0 ;

  while (1)
  {

	     while(START_Reading_Switch)
	     {

	    	 if(BUZZER_FLAG)
	    	 {
	    		 send_usb_string("{0x06}\n");  //send test running at first when it start command recieved from pc or BLE or device button press
	    		 send_BLE_using_USART3_string("{0x06}\n");  //send test running at first when it start command recieved from pc or BLE or device button press
	    		 HAL_Delay(500);
	    	 }
	     //tEST Running};
		  if(minute_counter_in_Timer >= 1220 && Minutes_is_less_30_minutes < 30)  //every 1 min it will execute
		  {
			  send_usb_string("{0x06}\n"); //tEST Running send every minute to pC
			  send_BLE_using_USART3_string("{0x06}\n"); //tEST Running send every minute to BLE
			  Excite_Aquisition_and_calculate_Result(wells);
		  }

		  //


	     }




    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV16;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 48-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 48-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 50000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
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
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Buzzer_Pin|QR_EN_Pin|D4_Excite_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Turn_off_mosfet_for_power_off_Pin|RED_LED_Pin|Green_LED_Pin|D5_Excite_LED_Pin
                          |D3_Excite_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, D1_Excite_LED_Pin|D2_Excite_LED_Pin|RB__LED_RED_Pin|RB_LED_BLUE_Pin
                          |D7_Excite_LED_Pin|D6_Excite_LED_Pin|USB_pull_up_pb8_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Buzzer_Pin QR_EN_Pin D4_Excite_LED_Pin */
  GPIO_InitStruct.Pin = Buzzer_Pin|QR_EN_Pin|D4_Excite_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Turn_off_mosfet_for_power_off_Pin RED_LED_Pin Green_LED_Pin D5_Excite_LED_Pin
                           D3_Excite_LED_Pin */
  GPIO_InitStruct.Pin = Turn_off_mosfet_for_power_off_Pin|RED_LED_Pin|Green_LED_Pin|D5_Excite_LED_Pin
                          |D3_Excite_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : SWITCH_INT_Button_Pin */
  GPIO_InitStruct.Pin = SWITCH_INT_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SWITCH_INT_Button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : D1_Excite_LED_Pin D2_Excite_LED_Pin RB__LED_RED_Pin RB_LED_BLUE_Pin
                           D7_Excite_LED_Pin D6_Excite_LED_Pin USB_pull_up_pb8_Pin */
  GPIO_InitStruct.Pin = D1_Excite_LED_Pin|D2_Excite_LED_Pin|RB__LED_RED_Pin|RB_LED_BLUE_Pin
                          |D7_Excite_LED_Pin|D6_Excite_LED_Pin|USB_pull_up_pb8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Hall_Sensor_OUT_Pin */
  GPIO_InitStruct.Pin = Hall_Sensor_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Hall_Sensor_OUT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void Excite_Aquisition_and_calculate_Result(POSITION *well_ptrs[NUM_CHANNELS])
{
	                  minute_counter_in_Timer = 0;
				      Minutes_is_less_30_minutes++;

				      //for Pluslife Excitation PCB

				                      Reset_buffer_index();
				  					  Trigger_led_Aquire_data(1);
				  					  well_ptrs[0]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];
				  	             //     call_final_results();

				  	                  Reset_buffer_index();
				  	                  Trigger_led_Aquire_data(2);
				  	                  well_ptrs[1]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];

				  					  Reset_buffer_index();
				  					  Trigger_led_Aquire_data(3);
				  					  well_ptrs[2]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];


				  					  Reset_buffer_index();
				  					  Trigger_led_Aquire_data(5);
				  					  well_ptrs[3]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];

				  					  Reset_buffer_index();
				  					  Trigger_led_Aquire_data(6);
				  					  well_ptrs[4]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];

				  					  Reset_buffer_index();
				  					  Trigger_led_Aquire_data(7);
				  					  well_ptrs[5]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];

				  					  Reset_buffer_index();
				  					  Trigger_led_Aquire_data(4);
				  					  well_ptrs[6]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];





				      //for original LED PCb

//				      Reset_buffer_index();
//					  Trigger_led_Aquire_data(5);
//					  well_ptrs[0]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];
//	             //     call_final_results();
//
//	                  Reset_buffer_index();
//	                  Trigger_led_Aquire_data(6);
//	                  well_ptrs[1]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];
//
//					  Reset_buffer_index();
//					  Trigger_led_Aquire_data(7);
//					  well_ptrs[2]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];
//
//					  Reset_buffer_index();
//					  Trigger_led_Aquire_data(4);
//					  well_ptrs[3]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];
//
//					  Reset_buffer_index();
//					  Trigger_led_Aquire_data(2);
//					  well_ptrs[4]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];
//
//					  Reset_buffer_index();
//					  Trigger_led_Aquire_data(3);
//					  well_ptrs[5]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];
//
//					  Reset_buffer_index();
//					  Trigger_led_Aquire_data(1);
//					  well_ptrs[6]->Buff[Minutes_is_less_30_minutes -1] = ADC_Average_result[0];

					  Turn_off_All_LED();

					  //Result
					  PCR_OnMinuteTick(Minutes_is_less_30_minutes);


					  Count_Excite_LED_position = 0;
}

// Sending different strings based on condition
void send_usb_string(const char* str) {
  GET_USB_Status = CDC_Transmit_FS((uint8_t*)str, strlen(str));
 //   HAL_Delay(10); // Small delay to ensure transmission
}


// Sending different strings based on condition
void send_BLE_using_USART3_string(const char* str) {
  HAL_UART_Transmit(&huart3, (uint8_t*)str, strlen(str), 200);
 // HAL_Delay(10); // Small delay to ensure transmission
}

void Device_ready_Tone(void)
{
	      HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 1);  //off
		  HAL_Delay(150);
		  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off
		  HAL_Delay(150);
		  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 1);  //off
		  HAL_Delay(150);
		  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off
		  HAL_Delay(150);
		  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 1);  //off
		  HAL_Delay(150);
		  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off
		  HAL_Delay(150);
}


void Buzzer_power_on_tone(void)
{
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 1);  //off
	  HAL_Delay(2000);
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off
	  HAL_Delay(2000);
//	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 1);  //off
//	  HAL_Delay(500);
//	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off
//	  HAL_Delay(500);
//	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 1);  //off
//	  HAL_Delay(500);
//	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off
}

void Buzzer_Test_Complete_tone(void)
{
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 1);  //off
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 1);  //off
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 1);  //off
	  HAL_Delay(500);
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, 0);  //off
}
void Reset_buffer_index(void)
{
	  Moving_average  Average_Photo_D={0};
}
void Trigger_led_Aquire_data(uint8_t temp_count)
{
	//  HAL_Delay (100);

	  Turn_off_Rest_of_the_Turned_on_LED(temp_count,GPIO_PIN_LOW);
	  HAL_Delay (1000);
	  Data_Aquisition_flag_photo = 1;
	  Trigger_LED_Channel(temp_count,GPIO_PIN_HIGH);
	  HAL_Delay (1000);
	  Data_Aquisition_flag_photo = 0;
}
void Turn_on_All_LED(void)
{
	      Trigger_LED_Channel(1,GPIO_PIN_HIGH);
		  Trigger_LED_Channel(2,GPIO_PIN_HIGH);
		  Trigger_LED_Channel(3,GPIO_PIN_HIGH);
		  Trigger_LED_Channel(4,GPIO_PIN_HIGH);
		  Trigger_LED_Channel(5,GPIO_PIN_HIGH);
		  Trigger_LED_Channel(6,GPIO_PIN_HIGH);
		  Trigger_LED_Channel(7,GPIO_PIN_HIGH);

}


void Turn_off_All_LED(void)
{
	  Trigger_LED_Channel(1,GPIO_PIN_LOW);
	  Trigger_LED_Channel(2,GPIO_PIN_LOW);
	  Trigger_LED_Channel(3,GPIO_PIN_LOW);
	  Trigger_LED_Channel(4,GPIO_PIN_LOW);
	  Trigger_LED_Channel(5,GPIO_PIN_LOW);
	  Trigger_LED_Channel(6,GPIO_PIN_LOW);
	  Trigger_LED_Channel(7,GPIO_PIN_LOW);
}


void Turn_off_Rest_of_the_Turned_on_LED(LED_ID id,GPIO_Pin_Status status)
{
	for(uint8_t count_led = 1; count_led < 8 ; count_led++)
	{
  	if(count_led != id)
  	{
  		HAL_GPIO_WritePin(LED_Map[count_led].port, LED_Map[count_led].pin, status);
  	}
	}
}
void Trigger_LED_Channel(LED_ID id,GPIO_Pin_Status status)
{
    if (id >= 8) return;  // bounds check

    HAL_GPIO_WritePin(LED_Map[id].port, LED_Map[id].pin, status);
}



TEMP_StatusTypeDef check_Temprature_sensor_available(uint32_t ADC_VAL)
{

     if(ADC_VAL > 4000)
   	{
   		return TEMP_OPEN;
   	}
   	else if(ADC_VAL < 100)
   	{
   		return TEMP_SHORT;
   	}
   	else
   	{
   		return TEMP_OK;
   	}

}

void Preheat_loop(void)
{
	PIDController Heater_Power;

   #define PREHEAT_TIMEOUT_MS   480000UL   // 240,000 ticks = 240,000ms = 4 minutes
   #define PREHEAT_STABLE_COUNTS 5         // must hold temp for N consecutive ticks
   #define PREHEAT_TARGET_DEGC  59.0f
	//send device id and name to PC
	 send_usb_string("{0x00}\n"); //device id
	 HAL_Delay(20);
	 send_usb_string("[0x00,11112222,Natsight RapidAmp]\n");
	 HAL_Delay(20);

	 //send device id and name to BLE
	 send_BLE_using_USART3_string("{0x70,0x00}\n"); //device id
     HAL_Delay(20);
     send_BLE_using_USART3_string("[0x00,11112222,Natsight RapidAmp]\n");
	 HAL_Delay(20);


	//Set pre_heat flag
	PRE_HEAT_FLAG = 1;
    three_sec_count = HAL_GetTick();

    //this will count for 4 minutes after that i will throw error or once heater achieves target for N times it will get out of the loop
    uint32_t preheat_start_count = three_sec_count;
    uint8_t Count_N_times_target_temp = 0;
	//Start 50ms timer interrupt
	 HAL_TIM_Base_Start_IT(&htim4);
	do
	{

			       if(check_Temprature_sensor_available(ADC_Average_result[1]) != TEMP_OK)
			       {
			    	    Error_Handler();
			       }
			       Current_Temp=Calculate_temprature(ADC_Average_result[1]);

			       if (Current_Temp >= PREHEAT_TARGET_DEGC)
			       {
			    	   Count_N_times_target_temp++;
			       }
			       else
			       {
			    	   Count_N_times_target_temp = 0;
			       }

			       PID_Decision(&Heater_Power, 500.0f, 0.01f,0.0f, Set_Target_Temp);   //target power iS 5W
			       __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, PID_Compute_for_Heater_Power(&Heater_Power, Current_Temp, 1));  // TIM1->CCR1 = duty;


			       //timout check for 4 minutes
			       if((HAL_GetTick() - preheat_start_count) > PREHEAT_TIMEOUT_MS)
			       {
			    	   send_usb_string("{0xE5}\n"); //Hardware Fault to PC
			    	   send_BLE_using_USART3_string("{0xE5}\n"); //Hardware Fault  BLE
			    	   Error_Handler();
			       }


                   //send every 3 sec data to USB
			       if((HAL_GetTick() - three_sec_count) > 3000){
				   send_usb_string("{0x01}\n"); //pre-heating to PC
				   send_BLE_using_USART3_string("{0x01}\n"); //pre-heating to BLE
			       HAL_Delay(20);
			       three_sec_count = HAL_GetTick(); // Reset timer
			       }


	}while(Count_N_times_target_temp < PREHEAT_STABLE_COUNTS);

	//Reset pre_heat Flag
	PRE_HEAT_FLAG= 0;
	//buzzer trigger
	BUZZER_FLAG = 1;
	HAL_Delay(1000);
	//Turn off RED LED (this indicate preheat is done)
	HAL_GPIO_WritePin(RB__LED_RED_GPIO_Port, RB__LED_RED_Pin, 1); //off
	//Turn on Blue LED to indicate Device is Ready
	HAL_GPIO_WritePin(RB_LED_BLUE_GPIO_Port, RB_LED_BLUE_Pin, 0); //off


	//preheat is done now device is Ready
	 send_usb_string("{0x02}\n"); //Ready to PC
	 send_BLE_using_USART3_string("{0x02}\n"); //Ready to BLE
	 HAL_Delay(20);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	 	  HAL_GPIO_TogglePin(Green_LED_GPIO_Port, Green_LED_Pin);
	  	  HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
	      HAL_GPIO_TogglePin(RB_LED_BLUE_GPIO_Port, RB_LED_BLUE_Pin);
	 	  HAL_GPIO_TogglePin(RB__LED_RED_GPIO_Port, RB__LED_RED_Pin);
	 	  send_usb_string("{0x22}\n"); //Temperature Error to PC
	 	 send_BLE_using_USART3_string("{0x22}\n"); //Temperature Error to BLE
	 	  HAL_Delay(20);
	 	  //Heater off
	 	  TIM3->CCR3 = 0;
	  	  HAL_Delay(1000);

  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

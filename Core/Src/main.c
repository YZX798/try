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
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "moto.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu6050.h"
#include "stdlib.h"
#include "stdio.h"
#include "encoder.h"
#include "param.h"
#include "PID.h"
#include "gray.h"
#include "KEY.h"
#include "pot.h"
#include "State.h"
#include "BLE.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


volatile uint8_t tick;
volatile uint8_t flag_1ms;
volatile uint8_t flag_5ms;
volatile uint8_t flag_10ms;
volatile uint8_t flag_20ms;
volatile uint8_t flag_50ms;
volatile uint8_t flag_100ms;
volatile uint8_t flag_200ms;
float PWM_L_OUT=0;
float PWM_R_OUT=0;
float pot=0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	Param_Init();
	Motor_InitAll();
	PID_InitAll();
	
	Encoder_Init(&g_encoder);
	MPU_Object_Init(&g_mpu);
	Key_Init(&g_KEY);
	pot_Init(&g_pot);
	Tuner_Init(&g_tuner);
	State_Init(&g_State);
	
	OLED_Init();
	OLED_Clear();
	MPU_Init();
	mpu_dmp_init();
	Gray_Init(&g_gray);

	g_param.stand_hope = 0;
	for (uint8_t i = 0; i < 10; i++) 
	{
		{
			uint16_t t = 0;
			while (mpu_dmp_get_data(&g_mpu.pitch, &g_mpu.roll, &g_mpu.yaw) != 0)
			{
				if (++t > 500) break;
				HAL_Delay(1);
			}
		}
		g_param.stand_hope += g_mpu.roll;
		HAL_Delay(10);
	}
	g_param.stand_hope /= 10;
	g_stand.Hope = g_param.stand_hope;
	
	
	
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
	HAL_TIM_Base_Start_IT(&htim1);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	
	
	if(flag_5ms)
	{
		flag_5ms = 0;
		
	}

	
	if(flag_20ms)
	{
		flag_20ms = 0;
		g_pot.get(&g_pot);
		BLE_Send(g_mpu.roll,g_param.stand_hope,PWM_L_OUT/500,g_encoder.L_encoder*10);
	}
	
	if(flag_50ms)
	{
		flag_50ms = 0;
	}
	
	if(flag_200ms)
	{
		flag_200ms = 0;
		if(g_State.task_state==TASK_1)	OLED_Show_T1();
		else OLED_Show_T1();
	}
	
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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1) {
		tick++;
		
	if(tick % 5 == 0)
	{
		g_mpu.MPU_GET(&g_mpu);
		g_stand.update(&g_stand, g_mpu.roll, g_mpu.gyro_x);
		g_State.Control(&g_State);
		
	}
	
	if (tick % 10  == 0)
	{
		g_gray.Gary_BACK(&g_gray);
    if (g_State.run_state == RUN_FWD)
        g_turn.update(&g_turn, (float)g_gray.gray_return, 0);
    else
        g_turn.reset(&g_turn);
	}
	
	if (tick % 20  == 0)  
	{
		
		g_State.State_choose(&g_State);
		
		g_encoder.Read_Encoder(&g_encoder);
		
		if (g_State.run_state == RUN_FWD)
			g_turn.update(&g_turn, (float)g_gray.gray_return , 0);
		else
			g_turn.reset(&g_turn);

		g_State.Speed_Loop(&g_State,g_encoder.L_encoder,g_encoder.R_encoder);
	}
	
		if (tick % 1   == 0)  flag_1ms   = 1;
		if (tick % 5   == 0)  flag_5ms   = 1;
		if (tick % 20   == 0) flag_20ms  = 1;
		if (tick % 50  == 0)  flag_50ms  = 1;
		if (tick % 100 == 0)  flag_100ms = 1;
		if (tick % 200 == 0){ flag_200ms = 1;	tick = 0;}
	}
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

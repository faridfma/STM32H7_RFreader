/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "cmsis_os.h"
#include "lwip.h"
#include "main.h"
#include "stm32h7xx_it.h"
#include "string.h"
#include "stdbool.h"
#include <stdlib.h>
#include <stdio.h>
#include "RFIDreader.h"
#include "LoRa.h"
#include <time.h>
#include <sys/time.h>




#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */



volatile uint16_t RxDataSize = 0;

//EXTERNS
extern volatile bool DataReceivedFlag;
extern volatile bool RxTagsDataCompleted;
extern uint8_t aRxBuffer[RxBuf_SIZE];
extern volatile uint8_t LoRaRxBuffer[LoRaRxBuf_SIZE];
extern volatile uint8_t LoRaTxBuffer[LoRaRxBuf_SIZE];

/* Private variables ---------------------------------------------------------*/

volatile bool DataTransmitedFlag = false;
volatile bool LoRaDataReceivedFlag = false;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_uart4_rx;
DMA_HandleTypeDef hdma_uart4_tx;



static void UpdateRxDataSize(volatile uint16_t *Size);


/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MainTask */
osThreadId_t MainTaskHandle;
const osThreadAttr_t MainTask_attributes = {
  .name = "MainTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_UART5_Init(void);
static void MX_UART4_Init(void);
void StartDefaultTask(void *argument);
void MainTaskInit(void *argument);

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

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();


  /* Configure the system clock */
  SystemClock_Config();


  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_UART5_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of MainTask */
  MainTaskHandle = osThreadNew(MainTaskInit, NULL, &MainTask_attributes);

  /* Start scheduler */
  osKernelStart();


  while (1)
  {

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

	 /** Supply configuration update enable */
	 HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

	 /** Configure the main internal regulator output voltage */
	 __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	 /* Wait for the VOSRDY flag */
	 while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
	 {
	 }

	 /** Macro to configure the PLL clock source */
	 __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);

	 /** Initializes the RCC Oscillators according to the specified parameters in the RCC_OscInitTypeDef structure. */
	 RCC_OscInitStruct.OscillatorType = (RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI);
	 RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;

	/** INSTANTEL-FW-TEAM: Use a variable assignment here (rather than constant), to compensate for revisions. */
	RCC_OscInitStruct.HSICalibrationValue = 64;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 50;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 6;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
								   RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
		Error_Handler();
	}

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{
	  /* USER CODE END UART4_Init 1 */
	  huart4.Instance = UART4;
	  huart4.Init.BaudRate = 19200;
	  huart4.Init.WordLength = UART_WORDLENGTH_8B;
	  huart4.Init.StopBits = UART_STOPBITS_1;
	  huart4.Init.Parity = UART_PARITY_NONE;
	  huart4.Init.Mode = UART_MODE_TX_RX;
	  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
	  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

	  if(HAL_UART_DeInit(&huart4) != HAL_OK)
	  {
	    Error_Handler();
	  }

	  if (HAL_UART_Init(&huart4) != HAL_OK)
	  {
	    Error_Handler();
	  }

	  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
	  {
	    Error_Handler();
	  }

	  /* USER CODE BEGIN UART4_Init 2 */
}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{
	  /* USER CODE END UART5_Init 1 */
	  huart5.Instance = UART5;
	  huart5.Init.BaudRate = 921600;//115200;
	  huart5.Init.WordLength = UART_WORDLENGTH_8B;
	  huart5.Init.StopBits = UART_STOPBITS_1;
	  huart5.Init.Parity = UART_PARITY_NONE;
	  huart5.Init.Mode = UART_MODE_TX_RX;
	  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
	  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	  huart5.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	  if (HAL_UART_Init(&huart5) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  if (HAL_UARTEx_SetTxFifoThreshold(&huart5, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  if (HAL_UARTEx_SetRxFifoThreshold(&huart5, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  if (HAL_UARTEx_DisableFifoMode(&huart5) != HAL_OK)
	  {
	    Error_Handler();
	  }

	  /* USER CODE BEGIN UART5_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  /* USER CODE END USART1_Init 1 */

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 921600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
	Error_Handler();
  }

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

   /* DMA interrupt init */
   /* DMA1_Stream0_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
   /* DMA1_Stream1_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
   /* DMA1_Stream2_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
   /* DMA1_Stream3_IRQn interrupt configuration */
   HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
   HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

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
	  __HAL_RCC_GPIOE_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOF_CLK_ENABLE();
	  __HAL_RCC_GPIOH_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOD_CLK_ENABLE();
	  __HAL_RCC_GPIOG_CLK_ENABLE();


	  /*Configure GPIO pins : EN_RL3_MCU_Pin EN_RL4_MCU_Pin EN_RL5_MCU_Pin EN_RL6_MCU_Pin
	                           OPTO1_MCU_Pin EN_RL1_MCU_Pin EN_RL2_MCU_Pin */
	  GPIO_InitStruct.Pin = EN_RL1_MCU_Pin | EN_RL2_MCU_Pin | EN_RL3_MCU_Pin | EN_RL4_MCU_Pin | EN_RL5_MCU_Pin | EN_RL6_MCU_Pin | OPTO1_MCU_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

//    /*Configure GPIO pin Output Level */
//	  HAL_GPIO_WritePin(GPIOE, EN_RL3_MCU_Pin|EN_RL4_MCU_Pin|EN_RL5_MCU_Pin|EN_RL6_MCU_Pin
//	                          |OPTO1_MCU_Pin|EN_RL1_MCU_Pin|EN_RL2_MCU_Pin, GPIO_PIN_RESET);

//	  /*Configure GPIO pins : MUX_SELECT_Pin TAG_READER_RESETn_Pin GREEN_LED2_CNTRL_Pin */
//	  GPIO_InitStruct.Pin = MUX_SELECT_Pin| GREEN_LED2_CNTRL_Pin; //| TAG_READER_RESETn_Pin;
//	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	  GPIO_InitStruct.Pull = GPIO_NOPULL;
//	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pins : MUX_SELECT_Pin*/
	  GPIO_InitStruct.Pin = MUX_SELECT_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  HAL_GPIO_WritePin(MUX_SELECT_GPIO_Port, MUX_SELECT_Pin, GPIO_PIN_RESET); //Dry contact board controls the relays

	  /*Configure GPIO pins : GREEN_LED2_CNTRL_Pin */
	  GPIO_InitStruct.Pin = GREEN_LED2_CNTRL_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pins : TAG_READER_RESETn*/
	  GPIO_InitStruct.Pin = TAG_READER_RESETn_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(TAG_READER_RESETn_GPIO_Port, TAG_READER_RESETn_Pin, GPIO_PIN_RESET);
	  HAL_Delay(1000);
	  HAL_GPIO_WritePin(TAG_READER_RESETn_GPIO_Port, TAG_READER_RESETn_Pin, GPIO_PIN_SET);
	  HAL_Delay(1000);

	  /*Configure GPIO pins : OPTO2_MCU_Pin OPTO3_MCU_Pin OPTO4_MCU_Pin EN_DCDC1_MCU_Pin
	                           EN_DCDC2_MCU_Pin */
	  GPIO_InitStruct.Pin = OPTO2_MCU_Pin|OPTO3_MCU_Pin|OPTO4_MCU_Pin|EN_DCDC1_MCU_Pin
	                          |EN_DCDC2_MCU_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

//	  /*Configure GPIO pin Output Level */
//	  HAL_GPIO_WritePin(GPIOF, OPTO2_MCU_Pin|OPTO3_MCU_Pin|OPTO4_MCU_Pin|EN_DCDC1_MCU_Pin
//	 	                          |EN_DCDC2_MCU_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pin : RF_SW_CNTRL_Pin */
	  GPIO_InitStruct.Pin = RF_SW_CNTRL_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(RF_SW_CNTRL_GPIO_Port, &GPIO_InitStruct);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(RF_SW_CNTRL_GPIO_Port, RF_SW_CNTRL_Pin, GPIO_PIN_RESET);  // RF swicth path is LoRA path when this this pin is high otherwise the path is for RFID Out

	  /*Configure GPIO pin : LoRa_RESETn_Pin */
	  GPIO_InitStruct.Pin = LoRa_RESETn_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(LoRa_RESETn_GPIO_Port, &GPIO_InitStruct);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(LoRa_RESETn_GPIO_Port, LoRa_RESETn_Pin, GPIO_PIN_SET);

	  /*Configure GPIO pin : RED_LED_ALRM_Pin */
	  GPIO_InitStruct.Pin = RED_LED_ALRM_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(RED_LED_ALRM_GPIO_Port, &GPIO_InitStruct);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(RED_LED_ALRM_GPIO_Port, RED_LED_ALRM_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pin : GREEN_LED1_CNTRL_Pin */
	  GPIO_InitStruct.Pin = GREEN_LED1_CNTRL_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GREEN_LED1_CNTRL_GPIO_Port, &GPIO_InitStruct);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GREEN_LED1_CNTRL_GPIO_Port, GREEN_LED1_CNTRL_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pins : GREEN_LED2_CNTRL_Pin */
	  GPIO_InitStruct.Pin = GREEN_LED2_CNTRL_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GREEN_LED2_CNTRL_GPIO_Port, GREEN_LED2_CNTRL_Pin, GPIO_PIN_RESET);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
//  MX_LWIP_Init();
//
//  //newly added
//  const char* message = "Hello UDP message!\n\r";
//
//  LOCK_TCPIP_CORE();
//  ip_addr_t PC_IPADDR;
//  IP_ADDR4(&PC_IPADDR, 192, 168, 1, 100);
//
//  struct udp_pcb* my_udp = udp_new();
//  udp_connect(my_udp, &PC_IPADDR, 8);
//  struct pbuf* udp_buffer = NULL;
//  UNLOCK_TCPIP_CORE();


  osDelay(5000);
  /* Infinite loop */
  for(;;)
  {

	HAL_GPIO_TogglePin(GREEN_LED1_CNTRL_GPIO_Port, GREEN_LED1_CNTRL_Pin);
	HAL_GPIO_TogglePin(GREEN_LED2_CNTRL_GPIO_Port, GREEN_LED2_CNTRL_Pin);

//	HAL_GPIO_TogglePin(EN_RL1_MCU_GPIO_Port, EN_RL1_MCU_Pin);
//	HAL_GPIO_TogglePin(EN_RL2_MCU_GPIO_Port, EN_RL2_MCU_Pin);
//	HAL_GPIO_TogglePin(EN_RL3_MCU_GPIO_Port, EN_RL3_MCU_Pin);
//	HAL_GPIO_TogglePin(EN_RL4_MCU_GPIO_Port, EN_RL4_MCU_Pin);
//	HAL_GPIO_TogglePin(EN_RL5_MCU_GPIO_Port, EN_RL5_MCU_Pin);
//	HAL_GPIO_TogglePin(EN_RL6_MCU_GPIO_Port, EN_RL6_MCU_Pin);


//	HAL_GPIO_TogglePin(OPTO1_MCU_GPIO_Port, OPTO1_MCU_Pin);
//	HAL_GPIO_TogglePin(OPTO2_MCU_GPIO_Port, OPTO2_MCU_Pin);
//	HAL_GPIO_TogglePin(OPTO3_MCU_GPIO_Port, OPTO3_MCU_Pin);
//	HAL_GPIO_TogglePin(OPTO4_MCU_GPIO_Port, OPTO4_MCU_Pin);
//
//	HAL_GPIO_TogglePin(EN_DCDC1_MCU_GPIO_Port, EN_DCDC1_MCU_Pin);
//	HAL_GPIO_TogglePin(EN_DCDC2_MCU_GPIO_Port, EN_DCDC2_MCU_Pin);



//    /* !! PBUF_RAM is critical for correct operation !! */
//	udp_buffer = pbuf_alloc(PBUF_TRANSPORT, strlen(message), PBUF_RAM);
//
//	 if (udp_buffer != NULL)
//	 {
//		memcpy(udp_buffer->payload, message, strlen(message));
//		udp_send(my_udp, udp_buffer);
//		pbuf_free(udp_buffer);
//	 }

    osDelay(1000);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_MainTaskInit */
/**
* @brief Function implementing the MainTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MainTaskInit */
void MainTaskInit(void *argument)
{

   volatile uint32_t odr;
   volatile uint32_t flag = false;
   HAL_StatusTypeDef status;



	//request RFID Module info
  //   SetUpRFIDreader();

	status = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)aRxBuffer, RxBuf_SIZE);
	if(status!= HAL_OK)
	{

			if(huart1.ReceptionType != HAL_UART_RECEPTION_TOIDLE)
			{
				printf("huart1.ReceptionType is set to 0; it will be set to 1 \n\r");

				huart1.ReceptionType = HAL_UART_RECEPTION_TOIDLE;
			    __HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_IDLEF);
			     ATOMIC_SET_BIT(huart1.Instance->CR1, USART_CR1_IDLEIE);
			}
			else
			{
			  printf("DMA Error1 \n\r");
			}
	}

	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);


   QueryLoRaModule();



  /* Infinite loop */
  for(;;)
  {


	do
	{
	   HAL_GPIO_WritePin(RF_SW_CNTRL_GPIO_Port, RF_SW_CNTRL_Pin, GPIO_PIN_RESET);
	   odr = GPIOB->ODR;
	   flag = (odr)& (0x00000001);
	   HAL_Delay(100);
	}while(flag == 1);


	RxTagsDataCompleted = false;
	//SetUpRFIDreader();
	ScanForTags();

    osDelay(2000);

	do
    {
	   HAL_GPIO_WritePin(RF_SW_CNTRL_GPIO_Port, RF_SW_CNTRL_Pin, GPIO_PIN_SET);
	   odr = GPIOB->ODR;
	   flag = (odr)& (0x00000001);
	   HAL_Delay(100);
	}while(flag == 0);

	ReadRcvMsgRRSI();
	SendLoraMsg();

    osDelay(2000);
  }
  /* USER CODE END MainTaskInit */
}


///* MPU Configuration */
//
//static void MPU_Config(void)
//{
//  MPU_Region_InitTypeDef MPU_InitStruct = {0};
//
//  /* Disables the MPU */
//  HAL_MPU_Disable();
//
//  /** Initializes and configures the Region and the memory to be protected
//  */
//  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
//  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
//  MPU_InitStruct.BaseAddress = 0x0;
//  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
//  MPU_InitStruct.SubRegionDisable = 0x87;
//  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
//  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
//  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
//  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
//  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
//  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
//
//  HAL_MPU_ConfigRegion(&MPU_InitStruct);
//
//  /** Initializes and configures the Region and the memory to be protected
//  */
//  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
//  MPU_InitStruct.BaseAddress = 0x30000000;
//  MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
//  MPU_InitStruct.SubRegionDisable = 0x0;
//  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
//  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
//  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
//
//  HAL_MPU_ConfigRegion(&MPU_InitStruct);
//
//  /** Initializes and configures the Region and the memory to be protected
//  */
//  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
//  MPU_InitStruct.Size = MPU_REGION_SIZE_512B;
//  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
//  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
//  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
//
//  HAL_MPU_ConfigRegion(&MPU_InitStruct);
//  /* Enables the MPU */
//  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
//
//}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
	  MPU_Region_InitTypeDef MPU_InitStruct = {0};

	  /* Disables the MPU */
	  HAL_MPU_Disable();

	  /** Initializes and configures the Region and the memory to be protected
	  */
	  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	  MPU_InitStruct.BaseAddress = 0x0;
	  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
	  MPU_InitStruct.SubRegionDisable = 0x87;
	  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
	  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
	  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

	  HAL_MPU_ConfigRegion(&MPU_InitStruct);

	  /** Initializes and configures the Region and the memory to be protected
	  */
	  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
	  MPU_InitStruct.BaseAddress = 0x30000000;
	  MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
	  MPU_InitStruct.SubRegionDisable = 0x0;
	  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
	  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;

	  HAL_MPU_ConfigRegion(&MPU_InitStruct);

	  /** Initializes and configures the Region and the memory to be protected
	  */
	  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
	  MPU_InitStruct.Size = MPU_REGION_SIZE_512B;
	  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

	  HAL_MPU_ConfigRegion(&MPU_InitStruct);
	  /* Enables the MPU */
	  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

#ifdef  USE_FULL_ASSERT
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


/**
  * @brief  Tx Transfer completed callback
  * @param  huart: UART handle.
  * @note   This example shows a simple way to report end of DMA Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	DataTransmitedFlag = true;
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{

	HAL_StatusTypeDef status;

	if (huart->Instance == USART1)
	{

		 status = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)aRxBuffer, RxBuf_SIZE);
		 if(status!= HAL_OK)
		 {
			if(huart1.ReceptionType != HAL_UART_RECEPTION_TOIDLE)
			{
				printf("huart1.ReceptionType is set to 0; it will be set to 1 \n\r");

				huart1.ReceptionType = HAL_UART_RECEPTION_TOIDLE;
				__HAL_UART_CLEAR_FLAG(&huart1, UART_CLEAR_IDLEF);
				 ATOMIC_SET_BIT(huart1.Instance->CR1, USART_CR1_IDLEIE);
			}
			else
			{
			  printf("DMA Error1 \n\r");
			}
		 }

		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);


		UpdateRxDataSize(&Size);

		ProcessReceivedData(Size);

		DataReceivedFlag = true;
	}

	if (huart->Instance == UART4)
	{
		LoRaDataReceivedFlag = true;
		ProcessRxLoRaMsg((uint8_t*)LoRaRxBuffer, Size);
	}

}



static void UpdateRxDataSize(volatile uint16_t *Size)
{

	RxDataSize = *Size;

}
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART1 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}



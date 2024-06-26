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
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_hid.h"
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
#define BUFSIZE 256
char	SendBuffer[BUFSIZE];
int	Counter;
int KeyState=0;



// Global variables
uint8_t indata[2];
uint8_t outdata[2] = {0,0};
uint8_t lis_id;
int8_t AccelX;
int8_t AccelY;
int8_t AccelZ;
HAL_StatusTypeDef SPIStatus;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int16_t min_xval = 128;
int16_t max_xval = -128;
int16_t min_yval = 128;
int16_t max_yval = -128;

int16_t newxval = 0;
int16_t newyval = 0;

uint8_t button_flag = 0;

extern USBD_HandleTypeDef hUsbDeviceFS;

typedef struct
{
	uint8_t button;
	int8_t mouse_x;
	int8_t mouse_y;
	int8_t wheel;
} mouseHID;

mouseHID mousehid = {0,0,0,0};

void Calibrate (void) //Kalibravimas
{
	for (int i=0; i<50; i++)
	{

		outdata[0] = 0x29 | 0x80;
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
		HAL_SPI_TransmitReceive(&hspi1, &outdata, &indata, 2, HAL_MAX_DELAY);
		AccelX = indata[1];

		outdata[0] = 0x2B | 0x80;
		HAL_SPI_TransmitReceive(&hspi1, &outdata, &indata, 2, HAL_MAX_DELAY);
		AccelY = indata[1];

		outdata[0] = 0x2D | 0x80 ;
		HAL_SPI_TransmitReceive(&hspi1, &outdata, &indata, 2, HAL_MAX_DELAY);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
		AccelZ = indata[1];

		min_xval = MIN(min_xval, AccelX);
		max_xval = MAX(max_xval, AccelX);
		min_yval = MIN(min_yval, AccelY);
		max_yval = MAX(max_yval, AccelY);
		HAL_Delay (100);
	}

	// Uzdega melyna kai kalibravimas atliktas
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, 1);
}


// Skaitymas kada paspaustas mygtukas

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_0)
	{
		button_flag = 1;
	}
}



/* USER CODE END 0 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  HAL_Init();
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  SystemClock_Config();
  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */


  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
  outdata[0] = 0x0f | 0x80;
  HAL_SPI_TransmitReceive(&hspi1, &outdata, &indata, 2, HAL_MAX_DELAY);
  lis_id = indata[1];
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_Delay(500);


  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
  outdata[0] = 0x20;
  outdata[1] = 0x47;
  HAL_SPI_TransmitReceive(&hspi1, &outdata, &indata, 2, HAL_MAX_DELAY);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_Delay(500);
  outdata[1] = 0x00;


  Calibrate();
  /* USER CODE END 2 */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  outdata[0] = 0x29 | 0x80;
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
	  HAL_SPI_TransmitReceive(&hspi1, &outdata, &indata, 2, HAL_MAX_DELAY);
	  AccelX = indata[1];

	  outdata[0] = 0x2B | 0x80;
	  HAL_SPI_TransmitReceive(&hspi1, &outdata, &indata, 2, HAL_MAX_DELAY);
	  AccelY = indata[1];

	  outdata[0] = 0x2D | 0x80;
	  HAL_SPI_TransmitReceive(&hspi1, &outdata, &indata, 2, HAL_MAX_DELAY);
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
	  AccelZ = indata[1];

	 KeyState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);

	 if (AccelX < min_xval)
	 	 {
	 	   newxval = AccelX - min_xval;
	 	 }

	 	else if (AccelX > max_xval)
	 	 {
	 	   newxval = AccelX - max_xval;
	 	 }

	 	if (AccelY < min_yval)
	 	{
	 	  newyval = AccelY - min_yval;
	 	}

	 	else if (AccelY > max_yval)
	 	{
	 	 newyval = AccelY - max_yval;
	 	}

	 	if ((newxval > 10) || (newxval <-10))
	 	{
	 	 mousehid.mouse_y = (newxval/10);
	 	}
	 	else mousehid.mouse_y = 0;

	 	if ((newyval > 10) || (newyval <-10))
	 	{
	 	 mousehid.mouse_x= (newyval)/10;
	 	}
	 	else mousehid.mouse_x = 0;

	 	if (button_flag==1)
	 	{
	 	 mousehid.button = 1; //left click = 1; right click = 2
	 	 USBD_HID_SendReport(&hUsbDeviceFS, &mousehid, sizeof (mousehid));
	 	 HAL_Delay (50);
	 	 mousehid.button = 0;
	 	 USBD_HID_SendReport(&hUsbDeviceFS,&mousehid, sizeof (mousehid));
	 	 button_flag =0;
	 	}

	 	USBD_HID_SendReport(&hUsbDeviceFS,&mousehid, sizeof (mousehid));
	 	HAL_Delay(10);
	   }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

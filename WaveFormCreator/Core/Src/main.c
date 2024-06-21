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
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "ad9833.h"

#include <string.h>
#include <stdio.h>
#include "stdint.h"
#include "stm32f1xx_hal_tim.h"

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
uint8_t Rx_buff[100];
uint32_t Rx_Length;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart3,(uint8_t *)&ch,1,0xFFFF);//阻塞方式打印
    return ch;
}

//----------显示波形数据
void OLED_ShowWaveParam(unsigned int WaveMode, unsigned char amp, double Freq, unsigned int Phase) {
    //扬声器发声
    //HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_0);
    //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

    //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);

    char buffer[20];

    OLED_CLS();
    OLED_ShowStr(0, 0, "WaveFormCreator", 1);

    // Display WaveMode
    switch (WaveMode) {
        case SIN_WAVE:
            OLED_ShowStr(0, 1, "WaveMode: Sine", 1);
            break;
        case TRI_WAVE:
            OLED_ShowStr(0, 1, "WaveMode: Tri", 1);
            break;
        case SQU_WAVE:
            OLED_ShowStr(0, 1, "WaveMode: Squ", 1);
            break;
        default:
            OLED_ShowStr(0, 1, "WaveMode: Unknown", 1);
            break;
    }

    // Display amplitude in mV
    snprintf(buffer, sizeof(buffer), "amp = %umV", amp);
    OLED_ShowStr(0, 2, buffer, 1);

    // Display frequency in kHz or Hz
    if (Freq >= 1000) {
        snprintf(buffer, sizeof(buffer), "Freq = %.3f kHz", Freq / 1000.0);
    } else {
        snprintf(buffer, sizeof(buffer), "Freq = %.1f Hz", Freq);
    }
    OLED_ShowStr(0, 3, buffer, 1);

    // Display phase in degrees
    snprintf(buffer, sizeof(buffer), "Phase = %u deg", Phase);
    OLED_ShowStr(0, 4, buffer, 1);

}
//-----

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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
    OLED_Init();
    OLED_CLS();
    AD9833_WaveSeting(1000,0,SIN_WAVE,0);
    AD9833_AmpSet(0);
    //中断使能
    __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);// 中断使能
    HAL_UART_Receive_DMA(&huart1,(uint8_t*)Rx_buff,100);//设置中断
    OLED_ShowStr(0,0,"WaveFormCreator",1);
    OLED_ShowStr(0,1,"Waiting for signal.",1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

//      HAL_UART_Transmit(&huart3,"This is uart3.\r\n",20,1000);
//      HAL_UART_Transmit(&huart1,"This is uart1.\r\n",20,1000);
      printf("Waiting for signal.\r\n");//注意这里输出的时候需要加上\r\n，不然无法输�???????
      HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
      /*
      OLED_ShowCN(0,0,0);
      OLED_ShowCN(16,0,1);
      OLED_ShowCN(32,0,2);
      OLED_ShowCN(48,0,3);
      OLED_ShowCN(64,0,4);
      OLED_ShowCN(80,0,5);
      OLED_ShowCN(96,0,6);
      OLED_ShowCN(112,0,7);
      */
      HAL_Delay(500);
      // Parameters     : x,y -- 起始点坐(x:0~127, y:0~7); ch[] -- 要显示的字符; TextSize -- 字符大小(1:6*8 ; 2:8*16)



      //OLED_ShowWaveParam(SIN_WAVE,100,100.2,0);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
      HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_2);
      HAL_Delay(1000);
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

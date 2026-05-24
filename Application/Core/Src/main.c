#include "main.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* -----------------------------------------------
       Step 1: Configure the main PLL
       Source : HSE (8MHz external crystal on Discovery)
       Target : 168MHz CPU clock
       Formula: (HSE / PLLM) * PLLN / PLLP
                (8 / 8) * 336 / 2 = 168MHz ✅
    ----------------------------------------------- */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState            = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM            = 8;
    RCC_OscInitStruct.PLL.PLLN            = 336;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ            = 7;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* -----------------------------------------------
       Step 2: Configure CPU, AHB, APB bus clocks
       HCLK  (CPU)  = 168MHz
       APB1         = 42MHz  (max for APB1)
       APB2         = 84MHz  (max for APB2)
    ----------------------------------------------- */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK
                                     | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1
                                     | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    /* Flash latency 5 = required for 168MHz */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();        // stop all interrupts
    while(1)
    {
        // Blink RED LED rapidly to signal something went wrong
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
        for(volatile uint32_t i = 0; i < 500000; i++);
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    // If you see this message, the bootloader jump worked!
    HAL_UART_Transmit(&huart2, (uint8_t*)"App is running!\r\n", 17, HAL_MAX_DELAY);

    while (1) {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15); // blink blue LED
        HAL_Delay(300);
    }
}

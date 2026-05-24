#include "main.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>
#include <stdio.h>

/* -------------------------------------------------------
 * Memory Map
 * Bootloader : 0x08000000 (Sector 0, 16KB)
 * Application: 0x08004000 (Sector 1 onwards)
 * ------------------------------------------------------- */
#define APP_START_ADDRESS   0x08004000UL

/* Typedef for a function pointer that takes no args, returns void */
typedef void (*pFunction)(void);

/* Simple UART print helper */
void BL_Print(const char *msg) {
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/* Check if a valid application exists at APP_START_ADDRESS.
 * A valid app has its Stack Pointer value inside RAM range.
 * STM32F4 RAM: 0x20000000 - 0x2002FFFF                      */
uint8_t BL_IsAppValid(void) {
    uint32_t sp = *(volatile uint32_t*)APP_START_ADDRESS;
    if ((sp >= 0x20000000) && (sp <= 0x2002FFFF)) {
        return 1; // Looks like a real app
    }
    return 0;
}

/* Jump to the application */
void BL_JumpToApp(void) {
    /* Step 1: Read the Reset Handler address from the app's vector table */
    uint32_t appResetHandler = *(volatile uint32_t*)(APP_START_ADDRESS + 4);

    /* Step 2: Create a function pointer to it */
    pFunction jumpToApp = (pFunction)appResetHandler;

    BL_Print("Jumping to application...\r\n");
    HAL_Delay(100); // Let UART finish

    /* Step 3: Set the Stack Pointer to the app's stack pointer */
    __set_MSP(*(volatile uint32_t*)APP_START_ADDRESS);

    /* Step 4: Disable all interrupts and reset peripherals */
    __disable_irq();
    for(int i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF; // Disable interrupts
        NVIC->ICPR[i] = 0xFFFFFFFF; // Clear pending interrupts
    }
    HAL_RCC_DeInit();
    HAL_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Step 5: Relocate the vector table to the app's location */
    SCB->VTOR = APP_START_ADDRESS;

    /* Step 6: JUMP! */
    jumpToApp();
}


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

    BL_Print("\r\n=== Custom Bootloader v1.0 ===\r\n");

    /*  **Decision Time**
     * Here you decide WHEN to stay in bootloader vs jump to the application.
     * what to do: hold the USER button (Blue button on board)
     * on reset to stay in bootloader mode.
     * else, jump to the app immediately.                              */

    // Read USER button, on STM32F4 Discovery its on PA0, active HIGH.
    HAL_Delay(50); // debounce

    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {
        BL_Print("Button held, staying in bootloader mode only.\r\n");
        // future work: will add firmware update logic here
        while (1) {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12); // blink green LED
            HAL_Delay(200);
        }
    }

    /* No button pressed, try to boot the application */
    BL_Print("Checking for valid application...\n");
    // future work: will add CRC check logic in BL_IsAppValid
    if (BL_IsAppValid()) {
    	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
    	HAL_Delay(1000);              // ← increase to 1000ms
    	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET); // ← turn off cleanly
    	HAL_Delay(100);
        BL_Print("Valid app found!\r\n");
        BL_JumpToApp();
    } else {
        BL_Print("ERROR: No valid application found, please check it\n");
        BL_Print("Please flash an application to 0x08004000\n");
        while (1) {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14); // blink red LED
            HAL_Delay(500);
        }
    }
}

/*
 * gpio.c
 *
 *  Created on: May 13, 2026
 *      Author: jeettewatia
 */


#include "gpio.h"

void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* -----------------------------------------------
       Step 1: Enable GPIO clocks
       Must enable clock BEFORE configuring any pin!
    ----------------------------------------------- */
    __HAL_RCC_GPIOA_CLK_ENABLE();   // For PA0 (User Button)
    __HAL_RCC_GPIOD_CLK_ENABLE();   // For PD12-15 (LEDs)

    /* -----------------------------------------------
       Step 2: Configure LEDs as Output
       PD12 = Green  LED
       PD13 = Orange LED
       PD14 = Red    LED
       PD15 = Blue   LED
    ----------------------------------------------- */
    /* Set all LEDs LOW (off) at start */
    HAL_GPIO_WritePin(GPIOD,
                      GPIO_PIN_12 | GPIO_PIN_13 |
                      GPIO_PIN_14 | GPIO_PIN_15,
                      GPIO_PIN_RESET);

    GPIO_InitStruct.Pin   = GPIO_PIN_12 | GPIO_PIN_13 |
                            GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;  // Push-Pull output
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* -----------------------------------------------
       Step 3: Configure User Button as Input
       PA0 = User Button (active HIGH on Discovery)
       When pressed → PA0 reads 1
       When not pressed → PA0 reads 0
    ----------------------------------------------- */
    GPIO_InitStruct.Pin  = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}



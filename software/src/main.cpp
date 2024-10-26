/**
 ******************************************************************************
 * @file    main.c
 * @author  MCU Application Team
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) Puya Semiconductor Co.
 * All rights reserved.</center></h2>
 *
 * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "py32f0xx_bsp_printf.h"
#include "stdbool.h"

/* Parameters ----------------------------------------------------------------*/
const int32_t THRESHOLD_LEFT  = 80000;
const int32_t THRESHOLD_RIGHT = 150000;

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int32_t data_in        = 0;
int32_t data_left      = 0;
int32_t cal_data_left  = 0;
bool cal_left          = false;
bool pressed_left      = false;
int32_t data_right     = 0;
int32_t cal_data_right = 0;
bool cal_right         = false;
bool pressed_right     = false;

bool read_right        = false;
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_LedConfig(void);

int main(void) {
    HAL_Init();

    APP_LedConfig();

    for (int i = 0; i < 10; i++) {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
            for (int i = 0; i < 27; i++) {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
            }
        }
    }

    while (1) {
        while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {
        }
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
            unsigned long value = 0;
            uint8_t data[3]     = {0};
            uint8_t filler      = 0x00;
            if (read_right) {
                for (int i = 0; i < 27; i++) {
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
                    if (i < 8) {
                        data[2] = (data[2] << 1) | (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET ? 0 : 1);
                    } else if (i >= 8 && i < 16) {
                        data[1] = (data[1] << 1) | (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET ? 0 : 1);
                    } else if (i >= 16 && i < 24) {
                        data[0] = (data[0] << 1) | (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET ? 0 : 1);
                    }
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
                }

                if (data[2] & 0x80) {
                    filler = 0xFF;
                } else {
                    filler = 0x00;
                }
                value      = value << 8 | filler;
                value      = value << 8 | data[2];
                value      = value << 8 | data[1];
                value      = value << 8 | data[0];

                data_right = value;

                if (!cal_right) {
                    cal_data_right = data_right;
                    cal_right      = true;
                }

                if (data_right - cal_data_right >= THRESHOLD_RIGHT) {
                    pressed_right = true;
                } else {
                    pressed_right = false;
                }

                read_right = false;
            } else {
                for (int i = 0; i < 26; i++) {
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
                    if (i < 8) {
                        data[2] = (data[2] << 1) | (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET ? 0 : 1);
                    } else if (i >= 8 && i < 16) {
                        data[1] = (data[1] << 1) | (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET ? 0 : 1);
                    } else if (i >= 16 && i < 24) {
                        data[0] = (data[0] << 1) | (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET ? 0 : 1);
                    }
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
                }

                if (data[2] & 0x80) {
                    filler = 0xFF;
                } else {
                    filler = 0x00;
                }
                value     = value << 8 | filler;
                value     = value << 8 | data[2];
                value     = value << 8 | data[1];
                value     = value << 8 | data[0];

                data_left = value;

                if (!cal_left) {
                    cal_data_left = data_left;
                    cal_left      = true;
                }

                if (data_left - cal_data_left >= THRESHOLD_LEFT) {
                    pressed_left = true;
                } else {
                    pressed_left = false;
                }

                read_right = true;
            }
        }
        // left : 32000000
        if (pressed_left) {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
        }
        // HAL_Delay(500);
        // HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
        // printf("echo\r\n");
    }
}

static void APP_LedConfig(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin   = GPIO_PIN_5;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = GPIO_PIN_1;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = GPIO_PIN_0;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
}

void APP_ErrorHandler(void) {
    while (1)
        ;
}

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line) {
    while (1) {
    }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/

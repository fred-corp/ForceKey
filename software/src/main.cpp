/**
 ******************************************************************************
 * @file    main.cpp
 * @author  Frédéric Druppel
 * @brief   Main program body
 ******************************************************************************
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <PY32F002Ax5.h>
#include <py32f0xx_hal.h>
#include <py32f0xx_hal_gpio.h>
#include <stdbool.h>

/* Private define ------------------------------------------------------------*/
#define SCK_PIN GPIO_PIN_4
#define SCK_PORT GPIOA
#define LEFT_PIN GPIO_PIN_3
#define LEFT_PORT GPIOA
#define RIGHT_PIN GPIO_PIN_14
#define RIGHT_PORT GPIOA

#define DOT_PIN GPIO_PIN_1
#define DOT_PORT GPIOA
#define DASH_PIN GPIO_PIN_13
#define DASH_PORT GPIOA

#define HIGH GPIO_PIN_SET
#define LOW GPIO_PIN_RESET

/* Parameters ----------------------------------------------------------------*/
const int32_t THRESHOLD_ON_LEFT   = 250000;
const int32_t THRESHOLD_OFF_LEFT  = 150000;
const int32_t THRESHOLD_ON_RIGHT  = 250000;
const int32_t THRESHOLD_OFF_RIGHT = 150000;

/* Private variables ---------------------------------------------------------*/
int32_t data_left_in   = 0;
int32_t cal_data_left  = 0;
bool cal_left          = false;
bool pressed_left      = false;
int32_t data_right_in  = 0;
int32_t cal_data_right = 0;
bool cal_right         = false;
bool pressed_right     = false;

/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_LedConfig(void);

int main(void) {
    HAL_Init();

    APP_LedConfig();

    for (int i = 0; i < 10; i++) {
        while (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == HIGH || HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == HIGH) {
        }
        if (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == LOW && HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == LOW) {
            for (int i = 0; i < 27; i++) {
                HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, HIGH);
                HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, LOW);
            }
        }
    }

    while (1) {
        while (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == HIGH || HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == HIGH) {
        }
        if (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == LOW && HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == LOW) {
            uint32_t value_left   = 0;
            uint8_t data_left[3]  = {0};
            uint8_t filler_left   = 0x00;
            uint32_t value_right  = 0;
            uint8_t data_right[3] = {0};
            uint8_t filler_right  = 0x00;

            for (int i = 0; i < 25; i++) {
                HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, HIGH);
                if (i < 8) {
                    data_left[2]  = (data_left[2] << 1) | (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == LOW ? 0 : 1);
                    data_right[2] = (data_right[2] << 1) | (HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == LOW ? 0 : 1);
                } else if (i >= 8 && i < 16) {
                    data_left[1]  = (data_left[1] << 1) | (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == LOW ? 0 : 1);
                    data_right[1] = (data_right[1] << 1) | (HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == LOW ? 0 : 1);
                } else if (i >= 16 && i < 24) {
                    data_left[0]  = (data_left[0] << 1) | (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == LOW ? 0 : 1);
                    data_right[0] = (data_right[0] << 1) | (HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == LOW ? 0 : 1);
                }
                HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, LOW);
            }

            if (data_left[2] & 0x80) {
                filler_left = 0xFF;
            } else {
                filler_left = 0x00;
            }
            value_left = value_left << 8 | filler_left;
            value_left = value_left << 8 | data_left[2];
            value_left = value_left << 8 | data_left[1];
            value_left = value_left << 8 | data_left[0];

            if (data_right[2] & 0x80) {
                filler_right = 0xFF;
            } else {
                filler_right = 0x00;
            }
            value_right   = value_right << 8 | filler_right;
            value_right   = value_right << 8 | data_right[2];
            value_right   = value_right << 8 | data_right[1];
            value_right   = value_right << 8 | data_right[0];

            data_left_in  = value_left;
            data_right_in = value_right;

            if (!cal_left) {
                cal_data_left = data_left_in;
                cal_left      = true;
            }

            if (!cal_right) {
                cal_data_right = data_right_in;
                cal_right      = true;
            }

            if (data_left_in - cal_data_left >= THRESHOLD_ON_LEFT) {
                pressed_left = true;
            } else if (data_left_in - cal_data_left <= THRESHOLD_OFF_LEFT) {
                pressed_left = false;
            }

            if (data_right_in - cal_data_right >= THRESHOLD_ON_RIGHT) {
                pressed_right = true;
            } else if (data_right_in - cal_data_right <= THRESHOLD_OFF_RIGHT) {
                pressed_right = false;
            }

            if (pressed_left) {
                HAL_GPIO_WritePin(DOT_PORT, DOT_PIN, HIGH);
            } else {
                HAL_GPIO_WritePin(DOT_PORT, DOT_PIN, LOW);
            }

            if (pressed_right) {
                HAL_GPIO_WritePin(DASH_PORT, DASH_PIN, HIGH);
            } else {
                HAL_GPIO_WritePin(DASH_PORT, DASH_PIN, LOW);
            }

            // if (pressed_left || pressed_right) {
            //     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, LOW);
            // } else {
            //     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, HIGH);
            // }

            // HAL_Delay(10);
        }
    }
}

static void APP_LedConfig(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // GPIO_InitStruct.Pin   = GPIO_PIN_5;
    // GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStruct.Pull  = GPIO_PULLUP;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    // HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = SCK_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SCK_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = LEFT_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LEFT_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = RIGHT_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(RIGHT_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = DOT_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DOT_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = DASH_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DASH_PORT, &GPIO_InitStruct);

    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, HIGH);
    HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, LOW);
    HAL_GPIO_WritePin(DOT_PORT, DOT_PIN, LOW);
    HAL_GPIO_WritePin(DASH_PORT, DASH_PIN, LOW);
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

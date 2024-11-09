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

/* Options -------------------------------------------------------------------*/
#define ONE_PADDLE  // Define this if for single paddle operation (compatible with two paddles)
#define TWO_PADDLES // Define this if for two paddles operation (need to have two paddles connected)

/* Pin definitions -----------------------------------------------------------*/
#define SCK_PIN GPIO_PIN_4
#define SCK_PORT GPIOA
#define LEFT_PIN GPIO_PIN_3
#define LEFT_PORT GPIOA
#define RIGHT_PIN GPIO_PIN_14
#define RIGHT_PORT GPIOA

#define DOT_PIN GPIO_PIN_13
#define DOT_PORT GPIOA
#define DASH_PIN GPIO_PIN_1
#define DASH_PORT GPIOA

/* Private define ------------------------------------------------------------*/
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
static void setup(void);
static void loop(void);
static bool paddles_ready(void);
static void read_paddles(bool *pressed_left, bool *pressed_right);

/* Compile checks -----------------------------------------------------------*/
// Raise compile error if one or both paddles are not connected
#if !defined(TWO_PADDLES) && !defined(ONE_PADDLE)
#error "Please define TWO_PADDLES or ONE_PADDLE"
#endif

/* Main function -------------------------------------------------------------*/

int main(void) {
    HAL_Init();

    HAL_Delay(100);

    setup();

    while (1) {
        loop();
    }
}

static void setup(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

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

#ifdef TWO_PADDLES
    GPIO_InitStruct.Pin   = RIGHT_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(RIGHT_PORT, &GPIO_InitStruct);
#endif

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

    HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, LOW);
    HAL_GPIO_WritePin(DOT_PORT, DOT_PIN, LOW);
    HAL_GPIO_WritePin(DASH_PORT, DASH_PIN, LOW);

    // Set HX711 Gain and Channel a few times to stabilize the output
    for (int i = 0; i < 10; i++) {
        while (!paddles_ready()) {
        }
        if (paddles_ready()) {
            for (int i = 0; i < 27; i++) {
                HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, HIGH);
                HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, LOW);
            }
        }
    }
}

static void loop(void) {
    while (!paddles_ready()) {
    }
    if (paddles_ready()) {
        read_paddles(&pressed_left, &pressed_right);

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
    }
}

static bool paddles_ready(void) {
#ifdef TWO_PADDLES
    return (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == LOW && HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == LOW);
#else
    return (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == LOW);
#endif
}

static void read_paddles(bool *_pressed_left, bool *_pressed_right) {
    uint32_t value_left  = 0;
    uint8_t data_left[3] = {0};
    uint8_t filler_left  = 0x00;
#ifdef TWO_PADDLES
    uint32_t value_right  = 0;
    uint8_t data_right[3] = {0};
    uint8_t filler_right  = 0x00;
#endif

    for (int i = 0; i < 25; i++) {
        HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, HIGH);
        if (i < 8) {
            data_left[2] = (data_left[2] << 1) | (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == LOW ? 0 : 1);
#ifdef TWO_PADDLES
            data_right[2] = (data_right[2] << 1) | (HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == LOW ? 0 : 1);
#endif
        } else if (i >= 8 && i < 16) {
            data_left[1] = (data_left[1] << 1) | (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == LOW ? 0 : 1);
#ifdef TWO_PADDLES
            data_right[1] = (data_right[1] << 1) | (HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == LOW ? 0 : 1);
#endif
        } else if (i >= 16 && i < 24) {
            data_left[0] = (data_left[0] << 1) | (HAL_GPIO_ReadPin(LEFT_PORT, LEFT_PIN) == LOW ? 0 : 1);
#ifdef TWO_PADDLES
            data_right[0] = (data_right[0] << 1) | (HAL_GPIO_ReadPin(RIGHT_PORT, RIGHT_PIN) == LOW ? 0 : 1);
#endif
        }
        HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, LOW);
    }

    if (data_left[2] & 0x80) {
        filler_left = 0xFF;
    } else {
        filler_left = 0x00;
    }
    value_left   = value_left << 8 | filler_left;
    value_left   = value_left << 8 | data_left[2];
    value_left   = value_left << 8 | data_left[1];
    value_left   = value_left << 8 | data_left[0];

    data_left_in = value_left;

    if (!cal_left) {
        cal_data_left = data_left_in;
        cal_left      = true;
    }

    if (data_left_in - cal_data_left >= THRESHOLD_ON_LEFT) {
        *_pressed_left = true;
    } else if (data_left_in - cal_data_left <= THRESHOLD_OFF_LEFT) {
        *_pressed_left = false;
    }

#ifdef TWO_PADDLES
    if (data_right[2] & 0x80) {
        filler_right = 0xFF;
    } else {
        filler_right = 0x00;
    }
    value_right   = value_right << 8 | filler_right;
    value_right   = value_right << 8 | data_right[2];
    value_right   = value_right << 8 | data_right[1];
    value_right   = value_right << 8 | data_right[0];

    data_right_in = value_right;

    if (!cal_right) {
        cal_data_right = data_right_in;
        cal_right      = true;
    }

    if (data_right_in - cal_data_right >= THRESHOLD_ON_RIGHT) {
        *_pressed_right = true;
#ifdef ONE_PADDLE
        return;
#endif
    } else if (data_right_in - cal_data_right <= THRESHOLD_OFF_RIGHT) {
        *_pressed_right = false;
    }
#endif

#ifdef ONE_PADDLE
    if (data_left_in - cal_data_left <= -THRESHOLD_ON_LEFT) {
        *_pressed_right = true;
    } else if (data_left_in - cal_data_left >= -THRESHOLD_OFF_LEFT) {
        *_pressed_right = false;
    }
#endif
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

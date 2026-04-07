/**
 * @file stm32f4_display.c
 * @brief Portable functions to interact with the display system FSM library. 
 * 
 * This file implements the hardware-specific functionality to interface with 
 * the RGB display system for the STM32F4 platform using PWM via TIM4. 
 * All portable display functions are defined here and are used by the FSM display logic.
 * 
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 07/04/25
 */

/* Standard C includes */
#include <stdio.h>

/* HW dependent includes */
#include "port_display.h"
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_display.h"

/* Defines ------------------------------------------------------------------*/

/**
 * @brief Maximum number of supported displays.
 */
#define MAX_DISPLAY_IDS 1  

/**
 * @brief Array to store the last color set on each display.
 */
static rgb_color_t last_display_colors[MAX_DISPLAY_IDS];

/* Typedefs ------------------------------------------------------------------*/

/**
 * @brief Internal structure that maps RGB components to their GPIO ports and pins.
 */
typedef struct 
{
    GPIO_TypeDef *p_port_red;   /**< GPIO port for red component */
    uint8_t pin_red;            /**< GPIO pin for red component */
    GPIO_TypeDef *p_port_green; /**< GPIO port for green component */
    uint8_t pin_green;          /**< GPIO pin for green component */
    GPIO_TypeDef *p_port_blue;  /**< GPIO port for blue component */
    uint8_t pin_blue;           /**< GPIO pin for blue component */
} stm32f4_display_hw_t;

/* Global variables ----------------------------------------------------------*/

/**
 * @brief Hardware configuration array for available displays.
 */
static stm32f4_display_hw_t displays_arr[] = {
    [PORT_REAR_PARKING_DISPLAY_ID] = {
        .p_port_red = STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO,
        .pin_red = STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN,
        .p_port_green = STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO,
        .pin_green = STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN,
        .p_port_blue = STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO,
        .pin_blue = STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN,
    },
};

/* Private functions --------------------------------------------------------*/

/**
 * @brief Retrieves a pointer to the display hardware configuration by ID.
 * 
 * @param display_id ID of the display.
 * @return stm32f4_display_hw_t* Pointer to the display hardware structure, or NULL if invalid.
 */
static stm32f4_display_hw_t *_stm32f4_display_get(uint32_t display_id)
{
    if (display_id < sizeof(displays_arr) / sizeof(displays_arr[0]))
    {
        return &displays_arr[display_id];
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Configures the TIM4 timer for PWM output to control RGB channels.
 * 
 * @param display_id ID of the display to configure.
 */
static void _timer_pwm_config(uint32_t display_id) {
    if (display_id == 0) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

        TIM4->CR1 &= ~TIM_CR1_CEN;
        TIM4->CR1 |= TIM_CR1_ARPE;

        TIM4->CNT = 0;
        TIM4->PSC = 4;                    
        TIM4->ARR = 63999;                

        TIM4->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC3E | TIM_CCER_CC4E);

        TIM4->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP |
                        TIM_CCER_CC3P | TIM_CCER_CC3NP |
                        TIM_CCER_CC4P | TIM_CCER_CC4NP);

        TIM4->CCMR1 &= ~TIM_CCMR1_OC1M;
        TIM4->CCMR1 |= (6 << TIM_CCMR1_OC1M_Pos);
        TIM4->CCMR1 |= TIM_CCMR1_OC1PE;

        TIM4->CCMR2 &= ~TIM_CCMR2_OC3M;
        TIM4->CCMR2 |= (6 << TIM_CCMR2_OC3M_Pos);
        TIM4->CCMR2 |= TIM_CCMR2_OC3PE;

        TIM4->CCMR2 &= ~TIM_CCMR2_OC4M;
        TIM4->CCMR2 |= (6 << TIM_CCMR2_OC4M_Pos);
        TIM4->CCMR2 |= TIM_CCMR2_OC4PE;

        TIM4->EGR |= TIM_EGR_UG;
        TIM4->CNT = 0;
    }
}

/* Public functions ---------------------------------------------------------*/

/**
 * @brief Initializes the specified display.
 * 
 * Configures the GPIO pins and alternate functions for PWM output, and initializes the timer.
 * 
 * @param display_id ID of the display to initialize.
 */
void port_display_init(uint32_t display_id){
    stm32f4_display_hw_t *p_display = _stm32f4_display_get(display_id);

    stm32f4_system_gpio_config(p_display->p_port_red, p_display->pin_red, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config(p_display->p_port_green, p_display->pin_green, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config(p_display->p_port_blue, p_display->pin_blue, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);

    stm32f4_system_gpio_config_alternate(p_display->p_port_red, p_display->pin_red, STM32F4_REAR_PARKING_DISPLAY_AF);
    stm32f4_system_gpio_config_alternate(p_display->p_port_green, p_display->pin_green, STM32F4_REAR_PARKING_DISPLAY_AF);
    stm32f4_system_gpio_config_alternate(p_display->p_port_blue, p_display->pin_blue, STM32F4_REAR_PARKING_DISPLAY_AF);

    _timer_pwm_config(display_id);
    port_display_set_rgb(display_id, COLOR_OFF);
}

/**
 * @brief Sets the RGB color for the specified display.
 * 
 * This function translates the color values to PWM duty cycles and applies them to the appropriate timer channels.
 * 
 * @param display_id ID of the display.
 * @param color RGB color to set.
 */
void port_display_set_rgb(uint32_t display_id, rgb_color_t color) {
    if (display_id < MAX_DISPLAY_IDS) {
        last_display_colors[display_id] = color;
    }

    if (display_id == PORT_REAR_PARKING_DISPLAY_ID) {
        TIM4->CR1 &= ~TIM_CR1_CEN;

        if (color.r == 0 && color.g == 0 && color.b == 0) {
            TIM4->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC3E | TIM_CCER_CC4E);
        } else {
            if (color.r == 0) {
                TIM4->CCER &= ~TIM_CCER_CC1E;
                TIM4->CCR1 = 0;
            } else {
                TIM4->CCR1 = ((TIM4->ARR + 1) * color.r) / PORT_DISPLAY_RGB_MAX_VALUE;
                TIM4->CCER |= TIM_CCER_CC1E;
            }

            if (color.g == 0) {
                TIM4->CCER &= ~TIM_CCER_CC3E;
                TIM4->CCR3 = 0;
            } else {
                TIM4->CCR3 = ((TIM4->ARR + 1) * color.g) / PORT_DISPLAY_RGB_MAX_VALUE;
                TIM4->CCER |= TIM_CCER_CC3E;
            }

            if (color.b == 0) {
                TIM4->CCER &= ~TIM_CCER_CC4E;
                TIM4->CCR4 = 0;
            } else {
                TIM4->CCR4 = ((TIM4->ARR + 1) * color.b) / PORT_DISPLAY_RGB_MAX_VALUE;
                TIM4->CCER |= TIM_CCER_CC4E;
            }

            TIM4->EGR |= TIM_EGR_UG;
            TIM4->CR1 |= TIM_CR1_CEN;
        }
    }
}

/**
 * @brief Gets the last RGB color set for the specified display.
 * 
 * @param display_id ID of the display.
 * @return rgb_color_t Last color that was set. Returns black if display_id is invalid.
 */
rgb_color_t port_display_get_rgb(uint32_t display_id) {
    if (display_id < MAX_DISPLAY_IDS) {
        return last_display_colors[display_id];
    } else {
        return (rgb_color_t){0, 0, 0};
    }
}


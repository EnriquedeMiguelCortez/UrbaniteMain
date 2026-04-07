/**
 * @file stm32f4_display.h
 * @brief Header for stm32f4_display.c file.
 * 
 * This file contains hardware-specific definitions for controlling the RGB display
 * system on the STM32F4 platform. It defines GPIO ports and pins used for RGB
 * LED control, as well as alternate function configuration.
 * 
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 07/04/25
 */

#ifndef STM32F4_DISPLAY_SYSTEM_H_
#define STM32F4_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f4xx.h"

/* Defines and enums --------------------------------------------------------*/

/**
 * @brief GPIO port for the red component of the RGB LED (rear parking display).
 */
#define STM32F4_REAR_PARKING_DISPLAY_RGB_R_GPIO GPIOB

/**
 * @brief GPIO pin number for the red component.
 */
#define STM32F4_REAR_PARKING_DISPLAY_RGB_R_PIN 6

/**
 * @brief GPIO port for the green component of the RGB LED (rear parking display).
 */
#define STM32F4_REAR_PARKING_DISPLAY_RGB_G_GPIO GPIOB

/**
 * @brief GPIO pin number for the green component.
 */
#define STM32F4_REAR_PARKING_DISPLAY_RGB_G_PIN 8

/**
 * @brief GPIO port for the blue component of the RGB LED (rear parking display).
 */
#define STM32F4_REAR_PARKING_DISPLAY_RGB_B_GPIO GPIOB

/**
 * @brief GPIO pin number for the blue component.
 */
#define STM32F4_REAR_PARKING_DISPLAY_RGB_B_PIN 9

/**
 * @brief Alternate function number used for RGB pin configuration.
 */
#define STM32F4_REAR_PARKING_DISPLAY_AF 2

#endif /* STM32F4_DISPLAY_SYSTEM_H_ */
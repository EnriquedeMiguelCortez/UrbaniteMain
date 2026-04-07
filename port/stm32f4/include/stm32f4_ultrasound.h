/**
 * @file stm32f4_ultrasound.h
 * @brief Header for stm32f4_ultrasound.c file.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 03/03/25
 */
#ifndef STM32F4_ULTRASOUND_H_
#define STM32F4_ULTRASOUND_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

#include <stdint.h>
#include "stm32f4xx.h"

/* HW dependent includes */

/* Defines and enums ----------------------------------------------------------*/
/* Defines */

/** GPIO and pin configuration for the ultrasound sensor trigger */
#define STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO GPIOB ///< GPIO port for the trigger signal
#define STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN 0 ///< GPIO pin for the trigger signal

/** GPIO and pin configuration for the ultrasound sensor echo */
#define STM32F4_REAR_PARKING_SENSOR_ECHO_GPIO GPIOA ///< GPIO port for the echo signal
#define STM32F4_REAR_PARKING_SENSOR_ECHO_PIN 1 ///< GPIO pin for the echo signal
#define STM32F4_REAR_PARKING_SENSOR_ECHO_AF 1 ///< Alternate function for the echo signal

/** Timer configuration for the ultrasound sensor */
#define STM32F4_REAR_PARKING_TRIGGER_TIMER TIM3 ///< Timer associated with the trigger signal
#define STM32F4_REAR_PARKING_ECHO_TIMER TIM2 ///< Timer associated with the echo signal
#define STM32F4_REAR_PARKING_MEASUREMENT_TIMER TIM5 ///< Timer used for measurement calculations

/** Timer settings */
#define TIMER_PSC 15  ///< Prescaler value for a 1 µs tick with a 16 MHz clock
#define TIMER_ARR 0xFFFF  ///< Maximum value to prevent overflow

/* Function prototypes and explanation -------------------------------------------------*/
/**
 * @brief Auxiliary function to change the GPIO and pin of the trigger pin of an ultrasound transceiver. This function is used for testing purposes mainly although it can be used in the final implementation if needed.
 *
 * @param ultrasound_id ID of the trigger signal to change.
 * @param p_port New GPIO port for the trigger signal.
 * @param pin New GPIO pin for the trigger signal.
 *
 */
void stm32f4_ultrasound_set_new_trigger_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin);

/**
 * @brief Auxiliary function to change the GPIO and pin of the echo pin of an ultrasound transceiver. This function is used for testing purposes mainly although it can be used in the final implementation if needed.
 *
 * @param ultrasound_id ID of the echo signal to change.
 * @param p_port New GPIO port for the echo signal.
 * @param pin New GPIO pin for the echo signal.
 *
 */
void stm32f4_ultrasound_set_new_echo_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin);


#endif /* STM32F4_ULTRASOUND_H_ */

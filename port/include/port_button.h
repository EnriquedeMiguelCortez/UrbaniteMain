/**
 * @file port_button.h
 * @brief Header for the portable functions to interact with the HW of the buttons. The functions must be implemented in the platform-specific code.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 22/02/25
 */

#ifndef PORT_BUTTON_H_
#define PORT_BUTTON_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Defines and enums ----------------------------------------------------------*/
/* Defines */

/**
 * @brief Identifier for the parking mode button.
 */
#define PORT_PARKING_BUTTON_ID 0

/**
 * @brief Debounce time for button press detection (in milliseconds).
 */
#define PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS 0

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Initializes the hardware configuration for the specified button.
 *
 * @param button_id Identifier of the button (index in the buttons array).
 */
void port_button_init (uint32_t button_id); 

/**
 * @brief Checks if the specified button has been pressed.
 *
 * @param button_id Identifier of the button (index in the buttons array).
 * @return true if the button is pressed, false otherwise.
 */
bool port_button_get_pressed (uint32_t button_id); 

/**
 * @brief Reads the current value of the GPIO pin associated with the button.
 *
 * @param button_id Identifier of the button (index in the buttons array).
 * @return true if the GPIO value is high, false if it is low.
 */
bool port_button_get_value (uint32_t button_id);

/**
 * @brief Sets the pressed status of the specified button.
 *
 * @param button_id Identifier of the button (index in the buttons array).
 * @param pressed New pressed status to assign to the button.
 */
void port_button_set_pressed (uint32_t button_id, bool pressed); 

/**
 * @brief Checks if the interrupt for the specified button is pending.
 *
 * @param button_id Identifier of the button (index in the buttons array).
 * @return true if an interrupt is pending, false otherwise.
 */
bool port_button_get_pending_interrupt (uint32_t button_id); 

/**
 * @brief Clears the pending interrupt flag for the specified button.
 *
 * @param button_id Identifier of the button (index in the buttons array).
 */
void port_button_clear_pending_interrupt (uint32_t button_id); 

/**
 * @brief Disables interrupts for the specified button.
 *
 * @param button_id Identifier of the button (index in the buttons array).
 */
void port_button_disable_interrupts (uint32_t button_id); 

#endif
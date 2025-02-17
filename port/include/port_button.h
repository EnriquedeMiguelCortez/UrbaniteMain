/**
 * @file port_button.h
 * @brief Header for the portable functions to interact with the HW of the buttons. The functions must be implemented in the platform-specific code.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

#ifndef PORT_BUTTON_H_
#define PORT_BUTTON_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Defines and enums ----------------------------------------------------------*/
/* Defines */
// Define here all the button identifiers that are used in the system

#define PORT_PARKING_BUTTON_ID /* ID of the button for reversing and for entering parking mode*/
#define PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS /* Debounce time of the button in ms*/

/* Function prototypes and explanation -------------------------------------------------*/

void port_button_init (uint32_t button_id); /* Configure the HW specifications of a given button*/

bool port_button_get_pressed (uint32_t button_id); /* 	Return the status of the button (pressed or not)*/

bool port_button_get_value (uint32_t button_id); /* Get the value of the GPIO connected to the button*/

void port_button_set_pressed (uint32_t button_id, bool pressed); /* Set the status of the button (pressed or not)*/	

bool port_button_get_pending_interrupt (uint32_t button_id); /* Get the status of the interrupt line connected to the button*/

void port_button_clear_pending_interrupt (uint32_t button_id); /* Clear the pending interrupt of the button*/

void port_button_disable_interrupts (uint32_t button_id); /* Disable the interrupts of the button*/

#endif
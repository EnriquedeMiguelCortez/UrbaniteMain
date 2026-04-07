/**
 * @file fsm_button.h
 * @brief Header for fsm_button.c file.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 27/02/25
 */

#ifndef FSM_BUTTON_H_
#define FSM_BUTTON_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>

/* Other includes */
#include "fsm.h"

/* Defines and enums ----------------------------------------------------------*/
/**
 * @enum FSM_BUTTON
 * @brief Enumeration representing button states in the FSM.
 */
enum FSM_BUTTON {
  BUTTON_RELEASED = 0,      /**< Button is not pressed */
  BUTTON_RELEASED_WAIT,     /**< Button released, waiting for debounce */
  BUTTON_PRESSED,           /**< Button is pressed */
  BUTTON_PRESSED_WAIT       /**< Button pressed, waiting for debounce */
};

/* Typedefs --------------------------------------------------------------------*/
/**
* @struct fsm_button_t
* @brief Structure representing the button FSM.
* 
* The details of this structure are defined in the corresponding source file (fsm_button.c).
*/
typedef struct fsm_button_t fsm_button_t;

/* Function prototypes and explanation -------------------------------------------------*/

/**
* @brief Gets the duration for which the button has been in its current state.
* @param p_fsm Pointer to the button FSM instance.
* @return Duration in milliseconds.
*/
uint32_t fsm_button_get_duration(fsm_button_t *p_fsm);

/**
* @brief Resets the button duration counter.
* @param p_fsm Pointer to the button FSM instance.
*/
void fsm_button_reset_duration(fsm_button_t *p_fsm);

/**
* @brief Gets the debounce time for the button.
* @param p_fsm Pointer to the button FSM instance.
* @return Debounce time in milliseconds.
*/
uint32_t fsm_button_get_debounce_time_ms(fsm_button_t *p_fsm);

/**
* @brief Initializes the button FSM instance.
* @param p_fsm_button Pointer to the button FSM instance.
* @param debounce_time Debounce duration in milliseconds.
* @param button_id Identifier for the button.
*/
void fsm_button_init(fsm_button_t *p_fsm_button, uint32_t debounce_time, uint32_t button_id);

/**
* @brief Creates and initializes a new button FSM instance.
* @param debounce_time Debounce duration in milliseconds.
* @param button_id Identifier for the button.
* @return Pointer to the newly created button FSM instance.
*/
fsm_button_t *fsm_button_new(uint32_t debounce_time, uint32_t button_id);

/**
* @brief Processes the FSM state transition based on button input.
* @param p_fsm Pointer to the button FSM instance.
*/
void fsm_button_fire(fsm_button_t *p_fsm);

/**
* @brief Destroys and frees memory allocated for a button FSM instance.
* @param p_fsm Pointer to the button FSM instance.
*/
void fsm_button_destroy(fsm_button_t *p_fsm);

/**
* @brief Retrieves the inner FSM structure of the button FSM.
* @param p_fsm Pointer to the button FSM instance.
* @return Pointer to the inner FSM structure.
*/
fsm_t *fsm_button_get_inner_fsm(fsm_button_t *p_fsm);

/**
* @brief Gets the current state of the button FSM.
* @param p_fsm Pointer to the button FSM instance.
* @return The current state as an integer from FSM_BUTTON enum.
*/
uint32_t fsm_button_get_state(fsm_button_t *p_fsm);

/**
 * @brief Checks whether the FSM has experienced recent activity.
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @return true if recent state changes or button events occurred.
 */
bool fsm_button_check_activity(fsm_button_t *p_fsm);	

#endif
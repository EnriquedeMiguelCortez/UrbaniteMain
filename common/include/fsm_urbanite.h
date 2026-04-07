/**
 * @file fsm_urbanite.h
 * @brief Header for fsm_urbanite.c file. Contains the definition and interface of the Urbanite FSM (Finite State Machine).
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 30/04/25
 */

#ifndef FSM_URBANITE_H_
#define FSM_URBANITE_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include "fsm_button.h"
#include "fsm_display.h"
#include "fsm_ultrasound.h"

/* Defines and enums ----------------------------------------------------------*/

/**
 * @enum FSM_URBANITE
 * @brief Enum representing the states of the Urbanite FSM.
 */
enum FSM_URBANITE {
    OFF = 0,              /**< System is turned off */
    MEASURE,              /**< System is measuring distance */
    SLEEP_WHILE_OFF,      /**< System is off and in low power mode */
    SLEEP_WHILE_ON        /**< System is on and in low power mode */
};

/* Typedefs --------------------------------------------------------------------*/

/**
 * @typedef fsm_urbanite_t
 * @brief Forward declaration of Urbanite FSM structure.
 */
typedef struct fsm_urbanite_t fsm_urbanite_t;

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Creates and initializes a new Urbanite FSM instance.
 * 
 * @param p_fsm_button Pointer to the button FSM instance.
 * @param on_off_press_time_ms Duration (in ms) needed to turn system ON/OFF.
 * @param pause_display_time_ms Duration (in ms) needed to pause/resume display.
 * @param special_time_ms Duration (in ms) to activate special mode.
 * @param reset_time_ms Duration (in ms) to reset the system.
 * @param p_fsm_ultrasound_rear Pointer to rear ultrasound FSM.
 * @param p_fsm_display_rear Pointer to rear display FSM.
 * @return Pointer to the initialized Urbanite FSM instance.
 */
fsm_urbanite_t *fsm_urbanite_new(fsm_button_t *p_fsm_button, uint32_t on_off_press_time_ms,
    uint32_t pause_display_time_ms, uint32_t special_time_ms, uint32_t reset_time_ms,
    fsm_ultrasound_t *p_fsm_ultrasound_rear, fsm_display_t *p_fsm_display_rear);

/**
 * @brief Triggers one step of the Urbanite FSM based on current inputs and conditions.
 * 
 * @param p_fsm Pointer to the Urbanite FSM instance.
 */
void fsm_urbanite_fire(fsm_urbanite_t *p_fsm);

/**
 * @brief Frees the resources associated with the Urbanite FSM instance.
 * 
 * @param p_fsm Pointer to the Urbanite FSM instance to be destroyed.
 */
void fsm_urbanite_destroy(fsm_urbanite_t *p_fsm);

#endif /* FSM_URBANITE_H_ */
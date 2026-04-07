/**
 * @file fsm_display.h
 * @brief Header for fsm_display.c file.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 12/04/25
 */

#ifndef FSM_DISPLAY_SYSTEM_H_
#define FSM_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdint.h>
#include <stdbool.h>
#include "fsm.h"

/* Defines and enums ----------------------------------------------------------*/
/* Enums */

/**
 * @enum FSM_DISPLAY_SYSTEM
 * @brief Possible states of the display FSM.
 */
enum FSM_DISPLAY_SYSTEM {
    WAIT_DISPLAY = 0,     /**< Waiting state before updating the display. */
    SET_DISPLAY_ON,       /**< State to turn the display on. */
    SET_DISPLAY_OFF       /**< State to turn the display off. */
};

/* Defines and enums ----------------------------------------------------------*/

/**
 * @def DANGER_MIN_CM
 * @brief Minimum distance (in cm) for DANGER level.
 */
#define DANGER_MIN_CM 0

/**
 * @def WARNING_MIN_CM
 * @brief Minimum distance (in cm) for WARNING level.
 */
#define WARNING_MIN_CM 25

/**
 * @def NO_PROBLEM_MIN_CM
 * @brief Minimum distance (in cm) for NO PROBLEM level.
 */
#define NO_PROBLEM_MIN_CM 50

/**
 * @def INFO_MIN_CM
 * @brief Minimum distance (in cm) for INFO level.
 */
#define INFO_MIN_CM 150

/**
 * @def OK_MIN_CM
 * @brief Minimum distance (in cm) for OK level.
 */
#define OK_MIN_CM 175

/**
 * @def OK_MAX_CM
 * @brief Maximum distance (in cm) for OK level.
 */
#define OK_MAX_CM 200

/* Typedefs ------------------------------------------------------------------*/
/**
 * @struct fsm_display_t
 * @brief Opaque type representing an instance of the display FSM.
 */
typedef struct fsm_display_t fsm_display_t;

/* Function Prototypes ------------------------------------------------------*/

/**
 * @brief Creates a new instance of the display FSM.
 * @param display_id ID of the display.
 * @return Pointer to the newly created display FSM instance.
 */
fsm_display_t *fsm_display_new(uint32_t display_id);

/**
 * @brief Destroys a display FSM instance and frees its memory.
 * @param p_fsm Pointer to the display FSM instance.
 */
void fsm_display_destroy(fsm_display_t *p_fsm);

/**
 * @brief Sets the current measured distance for the display FSM.
 * @param p_fsm Pointer to the display FSM instance.
 * @param distance_cm Distance in centimeters.
 */
void fsm_display_set_distance(fsm_display_t *p_fsm, uint32_t distance_cm);

/**
 * @brief Triggers a transition in the display FSM.
 * @param p_fsm Pointer to the display FSM instance.
 */
void fsm_display_fire(fsm_display_t *p_fsm);

/**
 * @brief Gets the current pause status of the FSM.
 * @param p_fsm Pointer to the display FSM instance.
 * @return True if paused, false otherwise.
 */
bool fsm_display_get_status(fsm_display_t *p_fsm);

/**
 * @brief Sets the pause status of the FSM.
 * @param p_fsm Pointer to the display FSM instance.
 * @param pause True to pause, false to resume.
 */
void fsm_display_set_status(fsm_display_t *p_fsm, bool pause);

/**
 * @brief Checks if the FSM is active.
 * @param p_fsm Pointer to the display FSM instance.
 * @return True if active, false otherwise.
 */
bool fsm_display_check_activity(fsm_display_t *p_fsm);

/**
 * @brief Retrieves the internal FSM structure.
 * @param p_fsm Pointer to the display FSM instance.
 * @return Pointer to the internal FSM.
 */
fsm_t *fsm_display_get_inner_fsm(fsm_display_t *p_fsm);

/**
 * @brief Gets the current state of the display FSM.
 * @param p_fsm Pointer to the display FSM instance.
 * @return Current state as a uint32_t.
 */
uint32_t fsm_display_get_state(fsm_display_t *p_fsm);

/**
 * @brief Sets the current state of the display FSM.
 * @param p_fsm Pointer to the display FSM instance.
 * @param state New state to set.
 */
void fsm_display_set_state(fsm_display_t *p_fsm, int8_t state);

#endif /* FSM_DISPLAY_SYSTEM_H_ */
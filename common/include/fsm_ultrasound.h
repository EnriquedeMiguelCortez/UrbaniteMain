/**
 * @file fsm_ultrasound.h
 * @brief Header for fsm_ultrasound.c file.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 10/03/25
 */

#ifndef FSM_ULTRASOUND_H_
#define FSM_ULTRASOUND_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

#include <stdint.h>
#include <stdbool.h>
#include "fsm.h"

/* Defines and enums ----------------------------------------------------------*/

/**
 * @brief Number of measurements used for distance calculation.
 */
#define FSM_ULTRASOUND_NUM_MEASUREMENTS   5

/**
 * @enum FSM_ULTRASOUND
 * @brief Enumeration representing the states of the ultrasound FSM.
 */
enum FSM_ULTRASOUND {
    WAIT_START = 0,   /**< Waiting for the measurement process to start. */
    TRIGGER_START,    /**< Sending the trigger signal to start measurement. */
    WAIT_ECHO_START,  /**< Waiting for the echo signal to start. */
    WAIT_ECHO_END,    /**< Waiting for the echo signal to end. */
    SET_DISTANCE      /**< Calculating and setting the measured distance. */
};

/* Typedefs --------------------------------------------------------------------*/
/**
 * @struct fsm_ultrasound_t
 * @brief Structure representing the ultrasound FSM.
 * 
 * The structure is defined in the corresponding source file (fsm_ultrasound.c).
 */
typedef struct fsm_ultrasound_t fsm_ultrasound_t;

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Set the state of the ultrasound FSM.
 *
 * This function sets the current state of the ultrasound FSM.
 * 
 * @param p_fsm Pointer to an `fsm_ultrasound_t` struct.
 * @param state New state of the ultrasound FSM.
 */
void fsm_ultrasound_set_state(fsm_ultrasound_t *p_fsm, int8_t state);

/**
 * @brief Creates and initializes a new ultrasound FSM instance.
 * @param ultrasound_id Identifier for the ultrasound sensor.
 * @return Pointer to the newly created ultrasound FSM instance.
 */
fsm_ultrasound_t *fsm_ultrasound_new (uint32_t ultrasound_id);

/**
 * @brief Destroys and frees memory allocated for an ultrasound FSM instance.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 */
void fsm_ultrasound_destroy(fsm_ultrasound_t *p_fsm);

/**
 * @brief Retrieves the last measured distance.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 * @return Measured distance in units (e.g., millimeters).
 */
uint32_t fsm_ultrasound_get_distance(fsm_ultrasound_t *p_fsm);

/**
 * @brief Triggers the FSM to process state transitions.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 */
void fsm_ultrasound_fire(fsm_ultrasound_t *p_fsm);

/**
 * @brief Gets the status of the ultrasound FSM.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 * @return True if active, false otherwise.
 */
bool fsm_ultrasound_get_status(fsm_ultrasound_t *p_fsm);

/**
 * @brief Sets the status of the ultrasound FSM.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 * @param status New status (true for active, false for inactive).
 */
void fsm_ultrasound_set_status(fsm_ultrasound_t *p_fsm, bool status);

/**
 * @brief Checks if the ultrasound FSM is ready for a new measurement.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 * @return True if ready, false otherwise.
 */
bool fsm_ultrasound_get_ready(fsm_ultrasound_t *p_fsm);

/**
 * @brief Checks if a new measurement is available.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 * @return True if a new measurement is ready, false otherwise.
 */
bool fsm_ultrasound_get_new_measurement_ready (fsm_ultrasound_t *p_fsm);

/**
 * @brief Stops the ultrasound FSM operations.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 */
void fsm_ultrasound_stop (fsm_ultrasound_t *p_fsm);

/**
 * @brief Starts the ultrasound FSM operations.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 */
void fsm_ultrasound_start (fsm_ultrasound_t *p_fsm);

/**
 * @brief Retrieves the inner FSM structure.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 * @return Pointer to the inner FSM structure.
 */
fsm_t *fsm_ultrasound_get_inner_fsm (fsm_ultrasound_t *p_fsm);

/**
 * @brief Gets the current state of the ultrasound FSM.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 * @return The current state as an integer from FSM_ULTRASOUND enum.
 */
uint32_t fsm_ultrasound_get_state (fsm_ultrasound_t *p_fsm);

/**
 * @brief Checks if there is activity in the ultrasound FSM.
 * @param p_fsm Pointer to the ultrasound FSM instance.
 * @return True if activity is detected, false otherwise.
 */
bool fsm_ultrasound_check_activity (fsm_ultrasound_t *p_fsm);

#endif /* FSM_ULTRASOUND_H_ */
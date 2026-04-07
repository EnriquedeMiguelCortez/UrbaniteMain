/**
 * @file fsm_ultrasound.c
 * @brief Ultrasound sensor FSM main file.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 10/03/25
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */
#include <stdlib.h>
#include <string.h>

/* HW dependent includes */
#include "port_ultrasound.h"
#include "port_system.h"
#include "fsm.h"
#include "fsm_ultrasound.h"

/* Project includes */

/* Typedefs --------------------------------------------------------------------*/

/**
 * @struct fsm_ultrasound_t
 * @brief Structure to represent the state of the ultrasound FSM.
 */
struct fsm_ultrasound_t {
    fsm_t f;                                 /**< Base FSM object */
    uint32_t distance_cm;                    /**< Median calculated distance in cm */
    bool status;                             /**< Sensor status: active/inactive */
    bool new_measurement;                    /**< Flag for new measurement availability */
    uint32_t ultrasound_id;                  /**< Sensor identifier */
    uint32_t distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS]; /**< Buffer for raw distances */
    uint32_t distance_idx;                   /**< Index for inserting into the distance buffer */
};

/* Private functions -----------------------------------------------------------*/
// Comparison function for qsort
int _compare(const void *a, const void *b)
{
    return (*(uint32_t *)a - *(uint32_t *)b);
}

/* State machine input or transition functions */
/**
 * @brief Check if ultrasound trigger is ready.
 */
static bool check_on (fsm_t *p_this){

    fsm_ultrasound_t *ultrasound_fsm = (fsm_ultrasound_t *)p_this; 
    return port_ultrasound_get_trigger_ready(ultrasound_fsm->ultrasound_id);

}

/**
 * @brief Check if ultrasound sensor is turned off.
 */
static bool check_off(fsm_t *p_this){
    fsm_ultrasound_t *ultrasound_fsm = (fsm_ultrasound_t *)p_this;
    bool status = ultrasound_fsm->status;
    return !status;
}	

/**
 * @brief Check if ultrasound trigger phase has ended.
 */
static bool check_trigger_end(fsm_t *p_this){
    fsm_ultrasound_t *ultrasound_fsm = (fsm_ultrasound_t *)p_this; 
    return port_ultrasound_get_trigger_end(ultrasound_fsm->ultrasound_id);

}	

/**
 * @brief Check if echo start tick has been registered.
 */
static bool check_echo_init(fsm_t *p_this){
    fsm_ultrasound_t *ultrasound_fsm = (fsm_ultrasound_t *)p_this; 
    return port_ultrasound_get_echo_init_tick(ultrasound_fsm->ultrasound_id) > 0;
    
}	

/**
 * @brief Check if echo pulse has been received.
 */
static bool check_echo_received(fsm_t *p_this){
    fsm_ultrasound_t *ultrasound_fsm = (fsm_ultrasound_t *)p_this; 
    return port_ultrasound_get_echo_received(ultrasound_fsm->ultrasound_id);
}	

/**
 * @brief Check if new measurement is ready.
 */
static bool check_new_measurement(fsm_t *p_this){
    fsm_ultrasound_t *ultrasound_fsm = (fsm_ultrasound_t *)p_this; 
    return port_ultrasound_get_trigger_ready(ultrasound_fsm->ultrasound_id);
}	

/**
 * @brief Dummy activity check (always false).
 */
bool fsm_ultrasound_check_activity(fsm_ultrasound_t *p_fsm)	{
    return false;
}



/* State machine output or action functions */

/**
 * @brief Start a new ultrasound measurement.
 */
static void do_start_measurement(fsm_t *p_this){
    fsm_ultrasound_t *ultrasound_fsm = (fsm_ultrasound_t *)p_this; 
    port_ultrasound_start_measurement(ultrasound_fsm->ultrasound_id);
}	

/**
 * @brief Stop the ultrasound trigger timer.
 */
static void do_stop_trigger(fsm_t *p_this){
    fsm_ultrasound_t *ultrasound_fsm = (fsm_ultrasound_t *)p_this; 
    port_ultrasound_stop_trigger_timer(ultrasound_fsm->ultrasound_id);
    port_ultrasound_set_trigger_end(ultrasound_fsm->ultrasound_id, false);
}	

/**
 * @brief Process echo data, calculate and store distance.
 */
static void do_set_distance(fsm_t *p_this){
    fsm_ultrasound_t *ultrasound_fsm = (fsm_ultrasound_t *)p_this; 

    // 1. Retrieve echo parameters
    uint32_t echo_init = port_ultrasound_get_echo_init_tick(ultrasound_fsm->ultrasound_id);
    uint32_t echo_end = port_ultrasound_get_echo_end_tick(ultrasound_fsm->ultrasound_id);
    uint32_t echo_overflows = port_ultrasound_get_echo_overflows(ultrasound_fsm->ultrasound_id);

    // 2. Calculate the total time (including overflows)
    uint32_t echo_time = (echo_end + (echo_overflows * 65536) - echo_init);
   

    // 3. Calculate distance in cm (Speed of sound = 34300 cm/s, Time is in microseconds)
    uint32_t distance_cm = (echo_time * 343) / (2 * 10000); 

    // 4. Store the distance in the array
    ultrasound_fsm->distance_arr[ultrasound_fsm->distance_idx] = distance_cm;

    // 5. If array is full, sort and compute median
    if (ultrasound_fsm->distance_idx == FSM_ULTRASOUND_NUM_MEASUREMENTS - 1) {
        qsort(ultrasound_fsm->distance_arr, FSM_ULTRASOUND_NUM_MEASUREMENTS, sizeof(uint32_t), _compare);

        // 6. Compute median
        if (FSM_ULTRASOUND_NUM_MEASUREMENTS % 2 == 0) {
            ultrasound_fsm->distance_cm = (ultrasound_fsm->distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS / 2 - 1] +
                                  ultrasound_fsm->distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS / 2]) / 2;
        } else {
            ultrasound_fsm->distance_cm = ultrasound_fsm->distance_arr[FSM_ULTRASOUND_NUM_MEASUREMENTS / 2];
        }

        // 7. Set flag for new measurement
        ultrasound_fsm->new_measurement = true;
    }

    // 8. Increase index and reset if necessary
    ultrasound_fsm->distance_idx = (ultrasound_fsm->distance_idx + 1) % FSM_ULTRASOUND_NUM_MEASUREMENTS;

    // 9. Stop echo timer
    port_ultrasound_stop_echo_timer(ultrasound_fsm->ultrasound_id);

    // 10. Reset echo ticks
    port_ultrasound_reset_echo_ticks(ultrasound_fsm->ultrasound_id);
    
}

/**
 * @brief Stop the ultrasound measurement.
 */
 static void do_stop_measurement(fsm_t *p_this){
    fsm_ultrasound_t *ultrasound_fsm = (fsm_ultrasound_t *)p_this;
    port_ultrasound_stop_ultrasound(ultrasound_fsm->ultrasound_id);
}	

/**
 * @brief Initiate a new measurement.
 */
static void do_start_new_measurement(fsm_t *p_this){
    do_start_measurement(p_this);
}	 

/**
 * @brief FSM state transitions for ultrasound sensor.
 */
fsm_trans_t fsm_trans_ultrasound[] = {
    {WAIT_START, check_on, TRIGGER_START, do_start_measurement},
    {TRIGGER_START, check_trigger_end, WAIT_ECHO_START, do_stop_trigger},
    {WAIT_ECHO_START, check_echo_init, WAIT_ECHO_END, NULL},
    {WAIT_ECHO_END, check_echo_received, SET_DISTANCE, do_set_distance},
    {SET_DISTANCE, check_new_measurement, TRIGGER_START, do_start_new_measurement},
    {SET_DISTANCE, check_off, WAIT_START, do_stop_measurement},
    {-1,NULL,-1,NULL}
};




/* Other auxiliary functions */
/**
 * @brief Initialize ultrasound FSM structure and sensor.
 * @param p_fsm_ultrasound Pointer to FSM structure
 * @param ultrasound_id ID of the sensor
 */
void fsm_ultrasound_init(fsm_ultrasound_t *p_fsm_ultrasound, uint32_t ultrasound_id)
{
    // Initialize the FSM
    fsm_init(&p_fsm_ultrasound->f, fsm_trans_ultrasound);

    /* TODO alumnos: */
    // Initialize the fields of the FSM structure
    // 2. Initialize fields
    p_fsm_ultrasound->distance_cm = 0;
    p_fsm_ultrasound->distance_idx = 0;
    memset(p_fsm_ultrasound->distance_arr, 0, sizeof(p_fsm_ultrasound->distance_arr));
    p_fsm_ultrasound->ultrasound_id = ultrasound_id;

     // 3. Initialize status and new_measurement flags to false
     p_fsm_ultrasound->status = false;
     p_fsm_ultrasound->new_measurement = false;
 
     // 4. Initialize the ultrasound hardware using the given ID
     port_ultrasound_init(ultrasound_id);
}

/* Public functions -----------------------------------------------------------*/
/**
 * @brief Allocate and initialize new ultrasound FSM.
 * @param ultrasound_id ID of the sensor
 * @return Pointer to new FSM structure
 */
fsm_ultrasound_t *fsm_ultrasound_new(uint32_t ultrasound_id)
{
    fsm_ultrasound_t *p_fsm_ultrasound = malloc(sizeof(fsm_ultrasound_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
    fsm_ultrasound_init(p_fsm_ultrasound, ultrasound_id);                  /* Initialize the FSM */
    return p_fsm_ultrasound;
}

/* FSM-interface functions. These functions are used to interact with the FSM */
/**
 * @brief Execute FSM transition.
 * @param p_fsm Pointer to FSM structure
 */
void fsm_ultrasound_fire(fsm_ultrasound_t *p_fsm)	
{
    fsm_fire((fsm_t *)p_fsm); 
}

/**
 * @brief Free memory used by the FSM.
 * @param p_fsm Pointer to FSM structure
 */
void fsm_ultrasound_destroy(fsm_ultrasound_t *p_fsm)
{
    free(&p_fsm->f);
}

/**
 * @brief Get inner FSM structure.
 * @param p_fsm Pointer to ultrasound FSM
 * @return Pointer to internal fsm_t structure
 */
fsm_t* fsm_ultrasound_get_inner_fsm(fsm_ultrasound_t *p_fsm){
    return &p_fsm->f;
}

/**
 * @brief Get current state of the FSM.
 * @param p_fsm Pointer to FSM
 * @return Current FSM state
 */
uint32_t fsm_ultrasound_get_state(fsm_ultrasound_t *p_fsm) {
    return p_fsm->f.current_state;
}

/**
 * @brief Get last valid distance measurement.
 * @param p_fsm Pointer to FSM
 * @return Measured distance in cm
 */
uint32_t fsm_ultrasound_get_distance(fsm_ultrasound_t *p_fsm) {
    uint32_t distance = p_fsm->distance_cm;

    p_fsm->new_measurement = false;

    return distance;
}

/**
 * @brief Stop the ultrasound FSM.
 * @param p_fsm Pointer to FSM
 */
void fsm_ultrasound_stop(fsm_ultrasound_t *p_fsm) {
    // 1. Reset the status field to indicate the sensor is inactive
    p_fsm->status = false;

    // 2. Call the function to stop the ultrasound sensor
    port_ultrasound_stop_ultrasound(p_fsm->ultrasound_id);
}

/**
 * @brief Start the ultrasound FSM.
 * @param p_fsm Pointer to FSM
 */
void fsm_ultrasound_start(fsm_ultrasound_t *p_fsm) {
    // 1. Set the status field to indicate the sensor is active
    p_fsm->status = true;

    // 2. Reset the distance index
    p_fsm->distance_idx = 0;

    // 3. Reset the last measured distance
    p_fsm->distance_cm = 0;

    // 4. Reset the echo ticks
    port_ultrasound_reset_echo_ticks(p_fsm->ultrasound_id);

    // 5. Set the trigger ready to indicate the sensor is ready for a new measurement
    port_ultrasound_set_trigger_ready(p_fsm->ultrasound_id, true);

    // 6. Start the new measurement timer to provoke the first interrupt
    port_ultrasound_start_new_measurement_timer();
}

/**
 * @brief Get current status of the ultrasound sensor.
 * @param p_fsm Pointer to FSM
 * @return true if active, false otherwise
 */
bool fsm_ultrasound_get_status(fsm_ultrasound_t *p_fsm) {
    return p_fsm->status;
}

/**
 * @brief Set the ultrasound sensor status.
 * @param p_fsm Pointer to FSM
 * @param status New status to set
 */
void fsm_ultrasound_set_status(fsm_ultrasound_t *p_fsm, bool status) {
    p_fsm->status = status;
}

/**
 * @brief Check if sensor is ready for new trigger.
 * @param p_fsm Pointer to FSM
 * @return true if ready, false otherwise
 */
bool fsm_ultrasound_get_ready(fsm_ultrasound_t *p_fsm) {
    // 1. Call the port function with the ultrasound ID and return the result
    return port_ultrasound_get_trigger_ready(p_fsm->ultrasound_id);
}

/**
 * @brief Check if a new measurement is available.
 * @param p_fsm Pointer to FSM
 * @return true if new measurement is ready, false otherwise
 */
bool fsm_ultrasound_get_new_measurement_ready(fsm_ultrasound_t *p_fsm) {
    return p_fsm->new_measurement;
}



// Other auxiliary functions
/**
 * @brief Manually set the FSM to a specific state.
 * @param p_fsm Pointer to FSM
 * @param state New state to assign
 */
void fsm_ultrasound_set_state(fsm_ultrasound_t *p_fsm, int8_t state)
{
    p_fsm->f.current_state = state;
}
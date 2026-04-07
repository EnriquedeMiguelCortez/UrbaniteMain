/**
 * @file fsm_urbanite.c
 * @brief Display system FSM main file.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 30/04/25
 */

 /* Includes ------------------------------------------------------------------*/
/* Standard C includes */

#include <stdlib.h>
#include <stdio.h>

/* HW dependent includes */

#include "port_system.h"
#include "fsm.h"
#include "fsm_urbanite.h"

/* Project includes */

/* Typedefs --------------------------------------------------------------------*/

/**
 * @struct fsm_urbanite_t
 * @brief Struct representing the Urbanite finite state machine.
 */
struct fsm_urbanite_t {
    fsm_t f;                                /**< Base FSM object */
    bool is_paused;                         /**< Indicates if display is paused */
    uint32_t on_off_press_time_ms;          /**< Threshold press time to toggle ON/OFF */
    fsm_button_t *p_fsm_button;             /**< Pointer to button FSM */
    fsm_display_t *p_fsm_display_rear;      /**< Pointer to rear display FSM */
    fsm_ultrasound_t *p_fsm_ultrasound_rear;/**< Pointer to rear ultrasound FSM */
    uint32_t pause_display_time_ms;         /**< Threshold press time to pause display */
    uint32_t reset_time_ms;                 /**< Press duration threshold for reset */
    uint32_t special_time_ms;               /**< Press duration threshold for special mode */
    bool special_mode_enabled;              /**< Indicates if special mode is active */
    uint32_t total_measurements;            /**< Total distance measurements taken */
    uint32_t total_on_time_ms;              /**< Total time system has been ON */
    uint32_t total_pause_time_ms;           /**< Total time display has been paused */
    uint32_t pause_count;                   /**< Number of times display was paused */
    uint32_t last_on_timestamp;             /**< Timestamp of last ON event */
    uint32_t last_pause_timestamp;          /**< Timestamp of last PAUSE event */
};
/* Private functions -----------------------------------------------------------*/

/* State machine input or transition functions */

/**
 * @brief Check if button is pressed ON.
 * @param p_this Pointer to the FSM.
 * @return true if ON condition met, false otherwise.
 */
static bool check_on(fsm_t *p_this){ 
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;

    fsm_button_t *p_fsm_button = urbanite_fsm->p_fsm_button;
    uint32_t duration = fsm_button_get_duration(p_fsm_button);
    uint32_t on_off_press_time = urbanite_fsm->on_off_press_time_ms;
    uint32_t special_time = urbanite_fsm->special_time_ms;
    

    if (duration > on_off_press_time && duration < special_time){
        return true;
    } else {
        return false;
    }
}	


/**
 * @brief Check if button is pressed OFF.
 * @param p_this Pointer to the FSM.
 * @return true if OFF condition met, false otherwise.
 */
static bool check_off(fsm_t *p_this){
    return check_on(p_this);
}	


/**
 * @brief Check if a new distance measurement is ready.
 * @param p_this Pointer to the FSM.
 * @return true if a new measurement is ready.
 */
static bool check_new_measure(fsm_t *p_this){
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;
    fsm_ultrasound_t *p_fsm_ultrasound_rear = urbanite_fsm->p_fsm_ultrasound_rear;
    return fsm_ultrasound_get_new_measurement_ready(p_fsm_ultrasound_rear);
}	


/**
 * @brief Check if button press corresponds to pause condition.
 * @param p_this Pointer to the FSM.
 * @return true if pause condition met.
 */
static bool check_pause_display	(fsm_t *p_this){ 
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;

    fsm_button_t *p_fsm_button = urbanite_fsm->p_fsm_button; 
    uint32_t duration = fsm_button_get_duration(p_fsm_button);
    uint32_t pause_display_time = urbanite_fsm->pause_display_time_ms;
    uint32_t on_off_press_time = urbanite_fsm->on_off_press_time_ms;

    if (duration > pause_display_time &&  duration < on_off_press_time){
        return true;
    } else {
        return false;
    }
}	

/**
 * @brief Check if there is any user or system activity.
 * @param p_this Pointer to the FSM.
 * @return true if any activity is detected.
 */
static bool check_activity(fsm_t *p_this){ 
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;

    fsm_button_t *p_fsm_button = urbanite_fsm->p_fsm_button; 
    fsm_ultrasound_t *p_fsm_ultrasound_rear = urbanite_fsm->p_fsm_ultrasound_rear;
    fsm_display_t *p_fsm_display_rear = urbanite_fsm->p_fsm_display_rear;
    bool status_button = fsm_button_check_activity(p_fsm_button);
    bool status_ultrasound = fsm_ultrasound_check_activity(p_fsm_ultrasound_rear);
    bool status_display = fsm_display_check_activity(p_fsm_display_rear);

    if (status_button == true || status_display == true || status_ultrasound == true){
        return true;
    } else {
        return false;
    }
}	

/**
 * @brief Check if there is no activity.
 * @param p_this Pointer to the FSM.
 * @return true if no activity is detected.
 */
static bool check_no_activity(fsm_t *p_this){
    return !check_activity(p_this);
}	

/**
 * @brief Check if a new measurement triggered activity.
 * @param p_this Pointer to the FSM.
 * @return true if triggered by a measurement.
 */
static bool check_activity_in_measure(fsm_t *p_this){
    return check_new_measure(p_this);
}	

/**
 * @brief Check if button press corresponds to reset.
 * @param p_this Pointer to the FSM.
 * @return true if reset condition met.
 */
static bool check_reset(fsm_t *p_this){
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;

    fsm_button_t *p_fsm_button = urbanite_fsm->p_fsm_button; 
    uint32_t duration = fsm_button_get_duration(p_fsm_button);
    uint32_t reset_time = urbanite_fsm->reset_time_ms;
    
    if (duration >  reset_time){
        return true;
    } else {
        return false;
    }
    
}

/**
 * @brief Check if button press corresponds to special mode.
 * @param p_this Pointer to the FSM.
 * @return true if special mode condition met.
 */
static bool check_special_mode(fsm_t *p_this){
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;

    fsm_button_t *p_fsm_button = urbanite_fsm->p_fsm_button;
    uint32_t duration = fsm_button_get_duration(p_fsm_button);
    uint32_t special_time = urbanite_fsm->special_time_ms;
    uint32_t reset_time = urbanite_fsm->reset_time_ms;

    if (duration > special_time && duration < reset_time){
        return true;
    } else {
        return false;
    }
}

/* State machine output or action functions */

/**
 * @brief Action to initialize system and start measurement.
 * @param p_this Pointer to the FSM.
 */
static void do_start_up_measure(fsm_t *p_this){ 
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;

    fsm_button_t *p_fsm_button = urbanite_fsm->p_fsm_button; 
    fsm_ultrasound_t *p_fsm_ultrasound_rear = urbanite_fsm->p_fsm_ultrasound_rear;
    fsm_display_t *p_fsm_display_rear = urbanite_fsm->p_fsm_display_rear;
    urbanite_fsm->last_on_timestamp = port_system_get_millis();

    fsm_button_reset_duration(p_fsm_button);

    fsm_ultrasound_start(p_fsm_ultrasound_rear);

    fsm_display_set_status(p_fsm_display_rear, true);

    printf("[URBANITE][%ld] Urbanite system ON\n", port_system_get_millis());
}	

/**
 * @brief Action to stop the Urbanite system.
 * @param p_this Pointer to the FSM.
 */
static void do_stop_urbanite(fsm_t *p_this){ 
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;

    fsm_button_t *p_fsm_button = urbanite_fsm->p_fsm_button; 
    fsm_ultrasound_t *p_fsm_ultrasound_rear = urbanite_fsm->p_fsm_ultrasound_rear;
    fsm_display_t *p_fsm_display_rear = urbanite_fsm->p_fsm_display_rear;

    fsm_button_reset_duration(p_fsm_button);

    fsm_ultrasound_stop(p_fsm_ultrasound_rear);

    fsm_display_set_status(p_fsm_display_rear, false);

    urbanite_fsm->is_paused = false;
    uint32_t now = port_system_get_millis();
    urbanite_fsm->total_on_time_ms += now - urbanite_fsm->last_on_timestamp;

    printf("[URBANITE][%ld] Urbanite system OFF\n", port_system_get_millis());
}	

/**
 * @brief Action to pause or resume the display.
 * @param p_this Pointer to the FSM.
 */
static void do_pause_display(fsm_t *p_this){ 
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;

    fsm_button_t *p_fsm_button = urbanite_fsm->p_fsm_button; 
    fsm_display_t *p_fsm_display_rear = urbanite_fsm->p_fsm_display_rear;

    fsm_button_reset_duration(p_fsm_button);

    urbanite_fsm->is_paused = !(urbanite_fsm->is_paused);

    fsm_display_set_status(p_fsm_display_rear, urbanite_fsm->is_paused);

    if (urbanite_fsm->is_paused == true) {
        urbanite_fsm->pause_count++;
        urbanite_fsm->last_pause_timestamp = port_system_get_millis();
        printf("[URBANITE][%ld] Urbanite system display PAUSE\n", port_system_get_millis());
    } else {
        uint32_t now = port_system_get_millis();
        urbanite_fsm->total_pause_time_ms += now - urbanite_fsm->last_pause_timestamp;
        printf("[URBANITE][%ld] Urbanite system display RESUME\n", port_system_get_millis());
    }
}	

/**
 * @brief Display the measured distance.
 * @param p_this Pointer to the FSM.
 */
static void do_display_distance(fsm_t *p_this){ 
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;

    fsm_ultrasound_t *p_fsm_ultrasound_rear = urbanite_fsm->p_fsm_ultrasound_rear; 
    fsm_display_t *p_fsm_display_rear = urbanite_fsm->p_fsm_display_rear;

    int32_t distance_cm = fsm_ultrasound_get_distance(p_fsm_ultrasound_rear);
    urbanite_fsm->total_measurements++;

    if (urbanite_fsm->is_paused == true) {
        if (distance_cm < (WARNING_MIN_CM / 2)) {
            fsm_display_set_distance(p_fsm_display_rear, distance_cm);
            fsm_display_set_status(p_fsm_display_rear, true);
        } else {
            fsm_display_set_status(p_fsm_display_rear, false); 
        }
    } else {
        fsm_display_set_distance(p_fsm_display_rear, distance_cm);
        fsm_display_set_status(p_fsm_display_rear, true);
    }

    printf("[URBANITE][%ld] Distance: %ld cm\n", port_system_get_millis(), distance_cm);
}	

/**
 * @brief Put the system to sleep while OFF.
 * @param p_this Pointer to the FSM.
 */
static void do_sleep_off(fsm_t *p_this){
    port_system_sleep();
}

/**
 * @brief Put the system to sleep while measuring.
 * @param p_this Pointer to the FSM.
 */
static void do_sleep_while_measure(fsm_t *p_this){ // No sé si está bien
    port_system_sleep();
}	

/**
 * @brief Put the system to sleep while OFF (duplicate, separate state).
 * @param p_this Pointer to the FSM.
 */
static void do_sleep_while_off(fsm_t *p_this){
    port_system_sleep();
}	

/**
 * @brief Put the system to sleep while ON.
 * @param p_this Pointer to the FSM.
 */
static void do_sleep_while_on(fsm_t *p_this){
    port_system_sleep();
}	

/**
 * @brief Perform a full reset of the Urbanite system.
 * @param p_this Pointer to the FSM.
 */
static void do_reset_urbanite(fsm_t *p_this){
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;

    fsm_ultrasound_stop(urbanite_fsm->p_fsm_ultrasound_rear);
    fsm_display_set_status(urbanite_fsm->p_fsm_display_rear, false);
    fsm_button_reset_duration(urbanite_fsm->p_fsm_button);
    urbanite_fsm->is_paused = false;

    urbanite_fsm->total_measurements = 0;
    urbanite_fsm->total_on_time_ms = 0;
    urbanite_fsm->total_pause_time_ms = 0;
    urbanite_fsm->pause_count = 0;
    urbanite_fsm->last_on_timestamp = 0;
    urbanite_fsm->last_pause_timestamp = 0;
    urbanite_fsm->special_mode_enabled = false;

    // Restart the LED

    fsm_display_set_status(urbanite_fsm->p_fsm_display_rear, false);
    fsm_display_set_state(urbanite_fsm->p_fsm_display_rear, false);

    printf("[URBANITE][%ld] Urbanite system display RESET\n", port_system_get_millis());
}

/**
 * @brief Activate special mode and print stats.
 * @param p_this Pointer to the FSM.
 */
static void do_activate_special_mode(fsm_t *p_this){ 
    fsm_urbanite_t *urbanite_fsm = (fsm_urbanite_t *)p_this;
    urbanite_fsm->special_mode_enabled = true;

    fsm_button_reset_duration(urbanite_fsm->p_fsm_button);

    // Accumulate the time that the system is on
    if (urbanite_fsm->last_on_timestamp > 0) {
        urbanite_fsm->total_on_time_ms += port_system_get_millis() - urbanite_fsm->last_on_timestamp;
        urbanite_fsm->last_on_timestamp = port_system_get_millis(); 
    }

    // Accumulate the time that the system is paused
    if (urbanite_fsm->is_paused && urbanite_fsm->last_pause_timestamp > 0) {
        urbanite_fsm->total_pause_time_ms += port_system_get_millis() - urbanite_fsm->last_pause_timestamp;
        urbanite_fsm->last_pause_timestamp = port_system_get_millis(); 
    }

    if (urbanite_fsm->special_mode_enabled) {
        printf("[URBANITE][%ld] SPECIAL MODE ACTIVATED\n", port_system_get_millis());
        printf("[STATS] Medidas: %lu\n", urbanite_fsm->total_measurements);
        printf("[STATS] Tiempo encendido: %lu ms\n", urbanite_fsm->total_on_time_ms);
        printf("[STATS] Pausas: %lu veces (%lu ms)\n", urbanite_fsm->pause_count, urbanite_fsm->total_pause_time_ms);
    }
}

/* Transition table -----------------------------------------------------------*/
/**
 * @brief Transition table for the Urbanite FSM.
 */
fsm_trans_t fsm_trans_urbanite[] = {
    {OFF, check_reset, OFF, do_reset_urbanite},                   
    {OFF, check_on, MEASURE, do_start_up_measure},
    {OFF, check_no_activity, SLEEP_WHILE_OFF, do_sleep_off},

    {SLEEP_WHILE_OFF, check_reset, OFF, do_reset_urbanite},         
    {SLEEP_WHILE_OFF, check_activity, OFF, NULL},
    {SLEEP_WHILE_OFF, check_no_activity, SLEEP_WHILE_OFF, do_sleep_while_off},

    {MEASURE, check_reset, OFF, do_reset_urbanite},                
    {MEASURE, check_special_mode, MEASURE, do_activate_special_mode},
    {MEASURE, check_pause_display, MEASURE, do_pause_display},
    {MEASURE, check_new_measure, MEASURE, do_display_distance},
    {MEASURE, check_off, OFF, do_stop_urbanite},
    {MEASURE, check_no_activity, SLEEP_WHILE_ON, do_sleep_while_measure},

    {SLEEP_WHILE_ON, check_reset, OFF, do_reset_urbanite},          
    {SLEEP_WHILE_ON, check_activity_in_measure, MEASURE, NULL},
    {SLEEP_WHILE_ON, check_no_activity, SLEEP_WHILE_ON, do_sleep_while_on},

    {-1, NULL, -1, NULL}
};


/* Other auxiliary functions */

/**
 * @brief Initialize Urbanite FSM structure.
 * @param p_fsm_urbanite Pointer to Urbanite FSM.
 * @param p_fsm_button Pointer to button FSM.
 * @param on_off_press_time_ms Time threshold for ON/OFF.
 * @param pause_display_time_ms Time threshold for pause.
 * @param special_time_ms Time threshold for special mode.
 * @param reset_time_ms Time threshold for reset.
 * @param p_fsm_ultrasound_rear Pointer to ultrasound FSM.
 * @param p_fsm_display_rear Pointer to display FSM.
 */
static void fsm_urbanite_init(fsm_urbanite_t *p_fsm_urbanite, fsm_button_t *p_fsm_button, uint32_t on_off_press_time_ms,
    uint32_t pause_display_time_ms, uint32_t special_time_ms, uint32_t reset_time_ms, fsm_ultrasound_t *p_fsm_ultrasound_rear, fsm_display_t *p_fsm_display_rear){
        fsm_init(&p_fsm_urbanite->f, fsm_trans_urbanite);

        p_fsm_urbanite->p_fsm_button = p_fsm_button;
        p_fsm_urbanite->on_off_press_time_ms = on_off_press_time_ms;
        p_fsm_urbanite->pause_display_time_ms = pause_display_time_ms;
        p_fsm_urbanite->special_time_ms = special_time_ms;
        p_fsm_urbanite->reset_time_ms = reset_time_ms;
        p_fsm_urbanite->p_fsm_ultrasound_rear = p_fsm_ultrasound_rear;
        p_fsm_urbanite->p_fsm_display_rear = p_fsm_display_rear;

        p_fsm_urbanite->is_paused = false;

        p_fsm_urbanite->total_measurements = 0;
        p_fsm_urbanite->total_on_time_ms = 0;
        p_fsm_urbanite->total_pause_time_ms = 0;
        p_fsm_urbanite->pause_count = 0;
        p_fsm_urbanite->last_on_timestamp = 0;
        p_fsm_urbanite->last_pause_timestamp = 0;
} 	

/* Public functions -----------------------------------------------------------*/


/**
 * @brief Allocate and initialize a new Urbanite FSM.
 * @param p_fsm_button Pointer to button FSM.
 * @param on_off_press_time_ms ON/OFF press threshold.
 * @param pause_display_time_ms Pause press threshold.
 * @param special_time_ms Special mode press threshold.
 * @param reset_time_ms Reset press threshold.
 * @param p_fsm_ultrasound_rear Pointer to ultrasound FSM.
 * @param p_fsm_display_rear Pointer to display FSM.
 * @return Pointer to the initialized Urbanite FSM.
 */
fsm_urbanite_t* fsm_urbanite_new(fsm_button_t *p_fsm_button, uint32_t on_off_press_time_ms, uint32_t pause_display_time_ms, uint32_t special_time_ms, uint32_t reset_time_ms,
    fsm_ultrasound_t *p_fsm_ultrasound_rear, fsm_display_t *p_fsm_display_rear){
        fsm_urbanite_t *p_fsm_urbanite = malloc(sizeof(fsm_urbanite_t)); /* Do malloc to reserve memory of all other FSM elements, although it is interpreted as fsm_t (the first element of the structure) */
        fsm_urbanite_init(p_fsm_urbanite, p_fsm_button, on_off_press_time_ms, pause_display_time_ms, special_time_ms, reset_time_ms, p_fsm_ultrasound_rear, p_fsm_display_rear);                  /* Initialize the FSM */
        return p_fsm_urbanite;
}	

/**
 * @brief Execute one iteration of the Urbanite FSM.
 * @param p_fsm Pointer to Urbanite FSM.
 */
void fsm_urbanite_fire(fsm_urbanite_t *p_fsm){
    fsm_fire((fsm_t *)p_fsm); 
}


/**
 * @brief Free memory allocated for the Urbanite FSM.
 * @param p_fsm Pointer to Urbanite FSM.
 */
void fsm_urbanite_destroy(fsm_urbanite_t *p_fsm)
{
    free(&p_fsm->f);
}





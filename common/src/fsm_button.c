/**
 * @file fsm_button.c
 * @brief Button FSM main file.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 27/02/25
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

/* HW dependent includes */
#include "port_button.h"
#include "port_system.h"

/* Project includes */
#include "fsm_button.h"
#include <stdlib.h>

/**
 * @struct fsm_button_t
 * @brief Structure representing the button FSM.
 * 
 * Contains the state machine instance, debounce timing variables,
 * press duration, and the unique button identifier.
 */
struct fsm_button_t
{
    fsm_t f;                   /**< Finite state machine instance for the button */
    uint32_t debounce_time_ms; /**< Debounce time in milliseconds */
    uint32_t next_timeout;     /**< Next debounce timeout timestamp in milliseconds */
    uint32_t tick_pressed;     /**< System tick recorded when the button was pressed */
    uint32_t duration;         /**< Duration the button has been pressed (in milliseconds) */
    uint32_t button_id;        /**< Unique identifier for the button */
};


/* State machine input or transition functions */

/**
 * @brief Checks if the button is pressed.
 * 
 * @param p_this pointer to the state machine.
 * @return true if the button is pressed, false if not.
 */
static bool check_button_pressed(fsm_t *p_this) { 
    
    fsm_button_t *button_fsm = (fsm_button_t *)p_this; 
    return port_button_get_pressed(button_fsm->button_id); 
}

/**
 * @brief Checks if the button has been released.
 * 
 * @param p_this pointer to the state machine.
 * @return true if the button has been released, false if not.
 */
static bool check_button_released(fsm_t *p_this) { 
    
    fsm_button_t *button_fsm = (fsm_button_t *)p_this;
    return !port_button_get_pressed(button_fsm->button_id); // Return true if button is not pressed (released)
}

/**
 * @brief Checks if the timeout for debounce has passed.
 * 
 * @param p_this pointer to the state machine.
 * @return true if the timeout has passed, false if not.
 */
static bool check_timeout(fsm_t *p_this) { 
    
    fsm_button_t *button_fsm = (fsm_button_t *)p_this;

    uint32_t current_time = port_system_get_millis();

    // Return whether the current time has surpassed the next timeout value (for debouncing)
    return current_time >= button_fsm->next_timeout;
}

/**
 * @brief Checks whether the FSM has experienced recent activity.
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @return true if recent state changes or button events occurred.
 */
bool fsm_button_check_activity(fsm_button_t *p_fsm) {
    uint32_t current_state = fsm_button_get_state(p_fsm);
    if (current_state == BUTTON_RELEASED) { 
        return false;
    } else {
        return true;
    }
}	
/* State machine output or action functions */

/**
 * @brief Stores the system tick when the button is pressed.
 * 
 * @param p_this pointer to the state machine.
 */
static void do_store_tick_pressed(fsm_t * p_this)
{
    fsm_button_t *p_button = (fsm_button_t *)(p_this);
    uint32_t t = port_system_get_millis(); 

    p_button->tick_pressed = t; // Store the time when button was pressed
    p_button->next_timeout = t + p_button->debounce_time_ms; // Set the timeout for the debounce duration
}

/**
 * @brief Stores the duration that the button has been pressed.
 * 
 * @param p_this pointer to the state machine.
 */
static void do_set_duration(fsm_t *p_this){
    fsm_button_t *p_button = (fsm_button_t *)(p_this);
    uint32_t t = port_system_get_millis(); 

    p_button->duration = t - p_button->tick_pressed; // Calculate the duration of the button press
    p_button->next_timeout = t + p_button->debounce_time_ms; 
}

/**
 * @brief Transition table for the button FSM.
 * 
 * Each transition describes a state, the condition for the transition, 
 * the next state, and any action to be performed when the transition occurs.
 */
fsm_trans_t fsm_trans_button[] = {
    {BUTTON_RELEASED, check_button_pressed, BUTTON_PRESSED_WAIT, do_store_tick_pressed}, // Transition from released to pressed
    {BUTTON_PRESSED_WAIT, check_timeout, BUTTON_PRESSED, NULL}, // Wait for debounce to complete before considering it pressed
    {BUTTON_PRESSED, check_button_released, BUTTON_RELEASED_WAIT, do_set_duration}, // Button released, store the duration
    {BUTTON_RELEASED_WAIT, check_timeout, BUTTON_RELEASED, NULL}, // Wait for debounce to complete before considering it released
    {-1,NULL,-1,NULL} // End of transition table
};


/* Other auxiliary functions */

/**
 * @brief Returns the duration the button has been pressed.
 * 
 * @param p_fsm Pointer to the button FSM.
 * @return The duration the button has been pressed in milliseconds.
 */
uint32_t fsm_button_get_duration(fsm_button_t * p_fsm)
{
    return p_fsm->duration;
}

/**
 * @brief Resets the duration value to 0.
 * 
 * @param p_fsm Pointer to the button FSM.
 */
void fsm_button_reset_duration(fsm_button_t * p_fsm)
{
    p_fsm->duration = 0;
}	

/**
 * @brief Gets the debounce time in milliseconds.
 * 
 * @param p_fsm Pointer to the button FSM.
 * @return The debounce time in milliseconds.
 */
uint32_t fsm_button_get_debounce_time_ms(fsm_button_t * p_fsm)
{
    return p_fsm->debounce_time_ms;
}	

/**
 * @brief Initializes the button FSM with a specified debounce time and button ID.
 * 
 * @param p_fsm_button Pointer to the button FSM to initialize.
 * @param debounce_time The debounce time in milliseconds.
 * @param button_id Unique ID for the button.
 */
void fsm_button_init(fsm_button_t *p_fsm_button, uint32_t debounce_time, uint32_t button_id)
{
    fsm_init(&p_fsm_button->f, fsm_trans_button); // Initialize the FSM with the transition table

    /* TODO alumnos: */
    p_fsm_button->debounce_time_ms = debounce_time; 
    p_fsm_button->button_id = button_id; 

    p_fsm_button->tick_pressed = 0; 
    p_fsm_button->duration = 0; 

    port_button_init(button_id); 
}

/* Public functions -----------------------------------------------------------*/

/**
 * @brief Creates a new button FSM object.
 * 
 * This function allocates memory for a new button FSM structure, initializes it,
 * and returns a pointer to it.
 * 
 * @param debounce_time The debounce time in milliseconds.
 * @param button_id Unique ID for the button.
 * @return Pointer to the newly created button FSM.
 */
fsm_button_t *fsm_button_new(uint32_t debounce_time, uint32_t button_id)
{
    fsm_button_t *p_fsm_button = malloc(sizeof(fsm_button_t)); /* Allocate memory for the FSM */
    fsm_button_init(p_fsm_button, debounce_time, button_id);   /* Initialize the FSM */
    return p_fsm_button;                                       /* Return pointer to the FSM object */
}

/* FSM-interface functions. These functions are used to interact with the FSM */

/**
 * @brief Fires the state machine to make a transition.
 * 
 * @param p_fsm Pointer to the button FSM.
 */
void fsm_button_fire(fsm_button_t *p_fsm)
{
    fsm_fire(&p_fsm->f); // Transition the FSM state based on current conditions
}

/**
 * @brief Destroys the button FSM object and frees allocated memory.
 * 
 * @param p_fsm Pointer to the button FSM to destroy.
 */
void fsm_button_destroy(fsm_button_t *p_fsm)
{
    free(&p_fsm->f); // Free the allocated memory for the FSM
}

/**
 * @brief Retrieves the internal state machine of the button FSM.
 * 
 * @param p_fsm Pointer to the button FSM.
 * @return Pointer to the internal FSM object.
 */
fsm_t *fsm_button_get_inner_fsm(fsm_button_t *p_fsm)
{
    return &p_fsm->f; // Return pointer to the internal FSM
}

/**
 * @brief Gets the current state of the button FSM.
 * 
 * @param p_fsm Pointer to the button FSM.
 * @return The current state of the FSM.
 */
uint32_t fsm_button_get_state(fsm_button_t *p_fsm)
{
    return p_fsm->f.current_state; // Return the current state of the FSM
}






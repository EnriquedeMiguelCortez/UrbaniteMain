/**
 * @file fsm_display.c
 * @brief Display system FSM main file.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 12/04/25
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/* HW dependent includes */

#include "port_display.h"
#include "port_system.h"
#include "fsm.h"
#include "fsm_display.h"

/* Project includes */

/* Typedefs --------------------------------------------------------------------*/

/**
 * @struct fsm_display_t
 * @brief FSM structure for controlling a display system.
 */
struct fsm_display_t {
    fsm_t f;                        /**< Base FSM structure */
    int32_t distance_cm;           /**< Measured distance in centimeters */
    bool new_color;                /**< Flag indicating a new color is needed */
    bool status;                   /**< On/off status of the display */
    bool idle;                     /**< Whether the FSM is idle */
    uint32_t display_id;           /**< Display identifier */
    uint32_t blink_period_ms;      /**< Blink interval in milliseconds */
    uint32_t last_blink_time;      /**< Last blink timestamp */
    rgb_color_t current_color;     /**< Current RGB color */
    bool blinking;                 /**< Whether the display is blinking */
};

/* Private functions -----------------------------------------------------------*/

/**
 * @brief Computes the RGB color based on the measured distance.
 * @param[out] p_color Pointer to the RGB color to set.
 * @param[in] distance_cm Distance in centimeters.
 */
void _compute_display_levels(rgb_color_t *p_color, int32_t 	distance_cm){
    int32_t max_distance = 200; 
    int32_t min_distance = 0;    
    
    // Normalized distance between 0 and 1
    float normalized_distance = (float)(distance_cm - min_distance) / (max_distance - min_distance);
    
    if (normalized_distance < 0) normalized_distance = 0;
    if (normalized_distance > 1) normalized_distance = 1;

    float factor = normalized_distance * normalized_distance;  

    p_color->r = (uint8_t)(PORT_DISPLAY_RGB_MAX_VALUE * (1 - factor)); 
  
    p_color->g = (uint8_t)(PORT_DISPLAY_RGB_MAX_VALUE * (1 - fabs(0.5 - factor) * 2));  

    p_color->b = (uint8_t)(PORT_DISPLAY_RGB_MAX_VALUE * factor);  
}	

/**
 * @brief Computes blink period based on distance.
 * 
 * Closer distances result in faster blinking.
 * 
 * @param distance_cm Distance in centimeters.
 * @return Blink period in milliseconds.
 */
uint32_t _compute_blink_period(int32_t distance_cm) {
    float normalized = (float)(distance_cm) / 200.0f;
    return 200 + (uint32_t)(800 * normalized); 
}

/* State machine input/transition checks --------------------------------------*/

/**
 * @brief Checks whether the FSM should be active.
 * 
 * @param p_this Pointer to the base FSM instance.
 * @return true if display is active.
 */
static bool check_active(fsm_t *p_this) {
    fsm_display_t *display_fsm = (fsm_display_t *)p_this; 
    return display_fsm->status;
}

/**
 * @brief Checks if a new color should be computed and applied.
 * 
 * @param p_this Pointer to the base FSM instance.
 * @return true if a new color is requested.
 */
static bool check_set_new_color(fsm_t *p_this){
    fsm_display_t *display_fsm = (fsm_display_t *)p_this; 
    return display_fsm->new_color;
}	

/**
 * @brief Checks if the display should be turned off.
 * 
 * @param p_this Pointer to the base FSM instance.
 * @return true if the display is off.
 */
static bool check_off(fsm_t *p_this) {
    fsm_display_t *display_fsm = (fsm_display_t *)p_this; 
    return !(display_fsm->status);
}

/**
 * @brief Checks if the blink timer has expired.
 * 
 * @param p_this Pointer to the base FSM instance.
 * @return true if it's time to toggle the display for blinking.
 */
static bool check_blink_timer_expired(fsm_t *p_this) {
    fsm_display_t *fsm = (fsm_display_t *)p_this;
    return fsm->blinking &&
        ((port_system_get_millis() - fsm->last_blink_time) >= fsm->blink_period_ms);
}

/**
 * @brief Always returns true for FSM activity.
 * @param p_fsm Pointer to the FSM.
 * @return Always true.
 */
bool fsm_display_check_activity(fsm_display_t *p_fsm) { 
    return true;
}	

/* State machine output/actions -----------------------------------------------*/

/**
 * @brief Computes and applies new color and blink period.
 * 
 * @param p_this Pointer to the FSM instance.
 */
static void do_set_color(fsm_t *p_this){
    fsm_display_t *display_fsm = (fsm_display_t *)p_this;

    rgb_color_t new_color = {0, 0, 0};
    _compute_display_levels(&new_color, display_fsm->distance_cm);

    bool color_changed = (new_color.r != display_fsm->current_color.r ||
                          new_color.g != display_fsm->current_color.g ||
                          new_color.b != display_fsm->current_color.b);

    if (color_changed) {
        display_fsm->current_color = new_color;
        port_display_set_rgb(display_fsm->display_id, new_color);
    }

    uint32_t new_blink_period = _compute_blink_period(display_fsm->distance_cm);
    if (new_blink_period != display_fsm->blink_period_ms) {
        display_fsm->blink_period_ms = new_blink_period;
        display_fsm->blinking = (new_blink_period > 0);
        display_fsm->last_blink_time = port_system_get_millis();
    }

    display_fsm->new_color = false;
    display_fsm->idle = true;
}

/**
 * @brief Turns the display on using current color.
 * 
 * @param p_this Pointer to the FSM instance.
 */
static void do_set_display_on(fsm_t *p_this){
    fsm_display_t *fsm = (fsm_display_t *)p_this;

    rgb_color_t current = fsm->current_color;
    rgb_color_t previous = port_display_get_rgb(fsm->display_id);

    if (previous.r != current.r || previous.g != current.g || previous.b != current.b) {
        port_display_set_rgb(fsm->display_id, current);
        fsm->last_blink_time = port_system_get_millis();
    }
}

/**
 * @brief Turns the display off (RGB = 0).
 * 
 * @param p_this Pointer to the FSM instance.
 */
static void do_set_display_off(fsm_t *p_this){
    fsm_display_t *fsm = (fsm_display_t *)p_this;

    rgb_color_t previous = port_display_get_rgb(fsm->display_id);
    rgb_color_t off = (rgb_color_t){0, 0, 0};

    if (previous.r != 0 || previous.g != 0 || previous.b != 0) {
        port_display_set_rgb(fsm->display_id, off);
        fsm->last_blink_time = port_system_get_millis();
    }
}

/**
 * @brief Final state action to ensure display is off.
 * 
 * @param p_this Pointer to the FSM instance.
 */
static void do_set_off(fsm_t *p_this){
    fsm_display_t *display_fsm = (fsm_display_t *)p_this; 
    port_display_set_rgb(display_fsm->display_id, COLOR_OFF);
    display_fsm->idle = false;
}

/* Transition table -----------------------------------------------------------*/

fsm_trans_t fsm_trans_display[] = {
    {WAIT_DISPLAY, check_active, SET_DISPLAY_ON, do_set_display_on},

    {SET_DISPLAY_ON, check_blink_timer_expired, SET_DISPLAY_OFF, do_set_display_off},
    {SET_DISPLAY_ON, check_set_new_color, SET_DISPLAY_ON, do_set_color},

    {SET_DISPLAY_OFF, check_blink_timer_expired, SET_DISPLAY_ON, do_set_display_on},
    {SET_DISPLAY_OFF, check_set_new_color, SET_DISPLAY_ON, do_set_color},

    {SET_DISPLAY_ON, check_off, WAIT_DISPLAY, do_set_off},
    {SET_DISPLAY_OFF, check_off, WAIT_DISPLAY, do_set_off},

    {-1,NULL,-1,NULL}
};

/* Public/Utility functions ---------------------------------------------------*/

/**
 * @brief Initializes the display FSM instance.
 * 
 * @param p_fsm_display Pointer to the FSM instance.
 * @param display_id ID of the display to manage.
 */
static void fsm_display_init(fsm_display_t *p_fsm_display, uint32_t display_id) {
    fsm_init(&p_fsm_display->f, fsm_trans_display);
    p_fsm_display->display_id = display_id;
    p_fsm_display->distance_cm = -1;
    p_fsm_display->new_color = false;
    p_fsm_display->status = false;
    p_fsm_display->idle = false;
    p_fsm_display->blink_period_ms = 0;
    p_fsm_display->last_blink_time = 0;
    p_fsm_display->blinking = false;
    port_display_init(display_id);
}

/**
 * @brief Creates a new instance of the display FSM.
 * 
 * @param display_id ID of the display.
 * @return Pointer to the newly created FSM instance.
 */
fsm_display_t *fsm_display_new(uint32_t display_id) {
    fsm_display_t *p_fsm_display = malloc(sizeof(fsm_display_t));
    fsm_display_init(p_fsm_display, display_id);
    return p_fsm_display;
}

/**
 * @brief Executes one step of the FSM.
 * 
 * @param p_fsm Pointer to the FSM instance.
 */
void fsm_display_fire(fsm_display_t *p_fsm) {
    fsm_fire((fsm_t *)p_fsm);
}

/**
 * @brief Destroys the FSM instance.
 * 
 * @param p_fsm Pointer to the FSM instance.
 */
void fsm_display_destroy(fsm_display_t *p_fsm) {
    free(&p_fsm->f);
}

/**
 * @brief Gets the internal base FSM.
 * 
 * @param p_fsm Pointer to the display FSM.
 * @return Pointer to the inner base FSM.
 */
fsm_t* fsm_display_get_inner_fsm(fsm_display_t *p_fsm){
    return &p_fsm->f;
}

/**
 * @brief Gets the current FSM state.
 * 
 * @param p_fsm Pointer to the display FSM.
 * @return Current FSM state.
 */
uint32_t fsm_display_get_state(fsm_display_t *p_fsm) {
    return p_fsm->f.current_state;
}

/**
 * @brief Sets the FSM state manually.
 * 
 * @param p_fsm Pointer to the display FSM.
 * @param state State value to set.
 */
void fsm_display_set_state(fsm_display_t *p_fsm, int8_t state){
    p_fsm->f.current_state = state;
}

/**
 * @brief Gets the current measured distance.
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @return Distance in cm.
 */
uint32_t fsm_display_get_distance(fsm_display_t *p_fsm) {
    return p_fsm->distance_cm;
}

/**
 * @brief Sets the distance and flags the FSM to update color.
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @param distance_cm Distance to set.
 */
void fsm_display_set_distance(fsm_display_t *p_fsm, uint32_t distance_cm){
    p_fsm->distance_cm = distance_cm;
    p_fsm->new_color = true;
}

/**
 * @brief Gets the display's on/off status.
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @return true if display is on.
 */
bool fsm_display_get_status(fsm_display_t *p_fsm){
    return p_fsm->status;
}	

/**
 * @brief Sets the display's on/off status.
 * 
 * @param p_fsm Pointer to the FSM instance.
 * @param status New status (true = on).
 */
void fsm_display_set_status(fsm_display_t *p_fsm, bool status) {
    p_fsm->status = status;
}

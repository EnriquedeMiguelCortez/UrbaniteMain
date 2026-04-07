/**
 * @file main.c
 * @brief Main file for the Urbanite FSM system. Initializes hardware, creates FSM instances, and enters the execution loop.
 * 
 * This file sets up all the modules (button, ultrasound, display, and urbanite FSMs)
 * and continuously fires them in the main loop to perform their respective tasks.
 * 
 * @author Sistemas Digitales II
 * @date 2025-01-01
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C libraries */
#include <stdio.h>  // For printf
#include <stdlib.h>
#include <stdint.h>

/* HW libraries and FSM modules */
#include "port_system.h"
#include "port_button.h"
#include "port_ultrasound.h"
#include "port_display.h"
#include "fsm.h"
#include "fsm_button.h"
#include "fsm_ultrasound.h"
#include "fsm_display.h"
#include "fsm_urbanite.h"

/* Defines ------------------------------------------------------------------*/

/**
 * @def URBANITE_RESET_TIME_MS
 * @brief Time in milliseconds required to reset the Urbanite system.
 */
#define URBANITE_RESET_TIME_MS 5000

/**
 * @def URBANITE_SPECIAL_TIME_MS
 * @brief Time in milliseconds required to activate the special mode.
 */
#define URBANITE_SPECIAL_TIME_MS 3000

/**
 * @def URBANITE_ON_OFF_PRESS_TIME_MS
 * @brief Time in milliseconds required to turn the system on or off.
 */
#define URBANITE_ON_OFF_PRESS_TIME_MS 1000

/**
 * @def URBANITE_PAUSE_DISPLAY_TIME_MS
 * @brief Time in milliseconds required to pause or resume the display.
 */
#define URBANITE_PAUSE_DISPLAY_TIME_MS 500

/**
 * @brief Application entry point.
 * 
 * Initializes all subsystems and FSM modules, and then enters an infinite loop
 * in which the FSMs are continuously updated.
 * 
 * @return int Exit status (not used in embedded systems).
 */
int main(void)
{
    /* Initialize hardware platform */
    port_system_init();

    /* Create and initialize FSM modules */
    fsm_button_t* p_fsm_button = fsm_button_new(PORT_PARKING_BUTTON_DEBOUNCE_TIME_MS, PORT_PARKING_BUTTON_ID);
    fsm_ultrasound_t* p_fsm_ultrasound = fsm_ultrasound_new(PORT_REAR_PARKING_SENSOR_ID);
    fsm_display_t* p_fsm_display = fsm_display_new(PORT_REAR_PARKING_DISPLAY_ID);

    /* Create and initialize the Urbanite FSM */
    fsm_urbanite_t* p_fsm_urbanite = fsm_urbanite_new(
        p_fsm_button,
        URBANITE_ON_OFF_PRESS_TIME_MS,
        URBANITE_PAUSE_DISPLAY_TIME_MS,
        URBANITE_SPECIAL_TIME_MS,
        URBANITE_RESET_TIME_MS,
        p_fsm_ultrasound,
        p_fsm_display
    );

    /* Infinite execution loop */
    while (1)
    {
        /* Fire each FSM to evaluate state transitions and perform actions */
        fsm_button_fire(p_fsm_button);
        fsm_ultrasound_fire(p_fsm_ultrasound);
        fsm_display_fire(p_fsm_display);
        fsm_urbanite_fire(p_fsm_urbanite);
    }


    /* Cleanup resources (not executed due to infinite loop) */
    fsm_button_destroy(p_fsm_button);
    fsm_ultrasound_destroy(p_fsm_ultrasound);
    fsm_display_destroy(p_fsm_display);
    fsm_urbanite_destroy(p_fsm_urbanite);
    
    /* Code below is unreachable, added for completeness */
    return 0;
}

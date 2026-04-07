#include <stdio.h>

#include "fsm_display.h"
#include "port_display.h"
#include "port_system.h"
#include "stm32f4_system.h"

/* Defines */
#define PORT_REAR_PARKING_DISPLAY_ID 0 /*!< Ultrasound sensor identifier @hideinitializer */

int main(void)
{
    // Initialize the system
    port_system_init();

    // Initialize the button FSM
    // Reserve space memory in the heap for the FSM
    fsm_display_t *p_fsm_display_rear = fsm_display_new(PORT_REAR_PARKING_DISPLAY_ID);
fsm_display_set_status(p_fsm_display_rear, true);
        fsm_display_set_distance(p_fsm_display_rear, 3);
    while (1)
    {
        // In every iteration, we activate the display and fire the FSM
        

            fsm_display_fire(p_fsm_display_rear);
            port_system_delay_ms(10);
    }

    return 0;
}

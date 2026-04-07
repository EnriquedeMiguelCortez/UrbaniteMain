/**
 * @file interr.c
 * @brief Interrupt service routines for the STM32F4 platform.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2025-01-01
 */
// Include HW dependencies:
#include "port_system.h"
#include "stm32f4_system.h"
#include "port_button.h"
#include <stdio.h>
#include "port_ultrasound.h"
#include "stm32f4_ultrasound.h"

// Include headers of different port elements:


//------------------------------------------------------
// INTERRUPT SERVICE ROUTINES
//------------------------------------------------------
/**
 * @brief Interrupt service routine for the System tick timer (SysTick).
 *
 * @note This ISR is called when the SysTick timer generates an interrupt.
 * The program flow jumps to this ISR and increments the tick counter by one millisecond.
 *
 * > **TO-DO alumnos:**
 * >
 * > ✅ 1. **Increment the System tick counter `msTicks` in 1 count.** To do so, use the function `port_system_get_millis()` and `port_system_set_millis()`.
 *
 * @warning **The variable `msTicks` must be declared volatile!** Just because it is modified by a call of an ISR, in order to avoid [*race conditions*](https://en.wikipedia.org/wiki/Race_condition). **Added to the definition** after *static*.
 *
 */
void SysTick_Handler(void)
{
    port_system_set_millis(port_system_get_millis() + 1); // Increment system tick counter
}

 /**
 * @brief ISR for the external interrupt line EXTI15_10.
 *
 * This interrupt is triggered by the parking button. It toggles the button state
 * and clears the interrupt flag to avoid repeated triggers.
 */
void EXTI15_10_IRQHandler(void)
{
    port_system_systick_resume();
    /* Check if the interrupt is pending for the parking button */
    if (port_button_get_pending_interrupt(PORT_PARKING_BUTTON_ID))
    {
        bool button_state = port_button_get_value(PORT_PARKING_BUTTON_ID);

        /* Toggle the button pressed state */
        port_button_set_pressed(PORT_PARKING_BUTTON_ID, !button_state); 

        /* Clear the interrupt flag to avoid re-entering */
        port_button_clear_pending_interrupt(PORT_PARKING_BUTTON_ID);
    }
    /* Ensure interrupt flag is cleared */
    port_button_clear_pending_interrupt(PORT_PARKING_BUTTON_ID);
}

/**
 * @brief ISR for Timer 3 (TIM3).
 *
 * This interrupt is triggered when the timer update event occurs.
 * It is used to handle the ultrasound trigger timeout.
 */
void TIM3_IRQHandler(void) {
    if (TIM3->SR & TIM_SR_UIF) {   
        /* Clear the update interrupt flag (UIF) to prevent retriggering */
        TIM3->SR &= ~TIM_SR_UIF;   

        /* Indicate that the ultrasound trigger time has expired */
        port_ultrasound_set_trigger_end(PORT_REAR_PARKING_SENSOR_ID, true);  

        
    }
    
}

/**
 * @brief ISR for Timer 2 (TIM2).
 *
 * This ISR handles both overflow events and capture events for ultrasound echo measurement.
 */
void TIM2_IRQHandler(void){
    port_system_systick_resume();
    
    uint32_t ultrasound_id = PORT_REAR_PARKING_SENSOR_ID; // Ultrasound sensor ID

    /* Handle overflow event */
    if (TIM2->SR & TIM_SR_UIF){
        uint32_t overflows = port_ultrasound_get_echo_overflows(ultrasound_id);
        port_ultrasound_set_echo_overflows(ultrasound_id, overflows + 1);
        TIM2->SR &= ~TIM_SR_UIF; // Clear interrupt flag
    }

    /* Handle capture event */
    if (TIM2->SR & TIM_SR_CC2IF)
    {
        // Read the value of the CCR2 register to get the current tick
        uint32_t current_tick = TIM2->CCR2;

        // Check if the echo signal has not started yet
        if (port_ultrasound_get_echo_init_tick(ultrasound_id) == 0 && port_ultrasound_get_echo_end_tick(ultrasound_id) == 0)
        {
            // Update the echo_init_tick with the current tick
            port_ultrasound_set_echo_init_tick(ultrasound_id, current_tick);
        }
        else
        {
            // Update the echo_end_tick with the current tick
            port_ultrasound_set_echo_end_tick(ultrasound_id, current_tick);

            // Set the echo_received flag to true
            port_ultrasound_set_echo_received(ultrasound_id, true);
        }
    }
}	

/**
 * @brief ISR for Timer 5 (TIM5).
 *
 * This interrupt handles the ultrasound sensor trigger readiness.
 */
void TIM5_IRQHandler(void) {
   /* Clear the update interrupt flag (UIF) in the status register */ 
   TIM5->SR &= ~TIM_SR_UIF;  

   /* Retrieve and update trigger readiness */
    port_ultrasound_set_trigger_ready(PORT_REAR_PARKING_SENSOR_ID, true);
    
}


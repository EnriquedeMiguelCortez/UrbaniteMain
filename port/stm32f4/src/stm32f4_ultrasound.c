/**
 * @file stm32f4_ultrasound.c
 * @brief Portable functions to interact with the ultrasound FSM library. All portable functions must be implemented in this file.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 20/02/25
 */

/* Standard C includes */

#include <stdio.h>
#include <math.h>
#include <stddef.h>


/* HW dependent includes */

#include "port_ultrasound.h"
#include "port_system.h"

/* Microcontroller dependent includes */

#include "stm32f4_system.h"
#include "stm32f4_ultrasound.h"

/* Typedefs --------------------------------------------------------------------*/


typedef struct  
{
    uint8_t echo_alt_fun;           // Alternate function for the echo signal
    uint32_t echo_end_tick;         // Tick time when the echo signal was received (end)
    uint32_t echo_init_tick;        // Tick time when the echo signal was received (start)
    uint32_t echo_overflows;        // Number of timer overflows during the echo signal
    uint8_t echo_pin;               // Pin/line where the echo signal is connected
    bool echo_received;             // Flag indicating if the echo signal has been received
    GPIO_TypeDef* p_echo_port;      // GPIO port where the echo signal is connected
    GPIO_TypeDef* p_trigger_port;   // GPIO port where the trigger signal is connected
    bool trigger_end;               // Flag indicating if the trigger signal has ended
    uint8_t trigger_pin;            // Pin/line where the trigger signal is connected
    bool trigger_ready;             // Flag indicating if a new measurement can be started
} stm32f4_ultrasound_hw_t;

/// @brief 


/* Global variables */

static stm32f4_ultrasound_hw_t 	ultrasounds_arr [] = { /* Array of elements that represents the HW characteristics of the ultrasounds connected to the STM32F4 platform*/
    [PORT_REAR_PARKING_SENSOR_ID] = {
        .p_trigger_port = STM32F4_REAR_PARKING_SENSOR_TRIGGER_GPIO,
        .trigger_pin = STM32F4_REAR_PARKING_SENSOR_TRIGGER_PIN,
        .p_echo_port = STM32F4_REAR_PARKING_SENSOR_ECHO_GPIO,
        .echo_pin = STM32F4_REAR_PARKING_SENSOR_ECHO_PIN,
        .echo_alt_fun = STM32F4_REAR_PARKING_SENSOR_ECHO_AF
    },
};

/* Private functions ----------------------------------------------------------*/
/**
 * @brief Returns the pointer to the ultrasound sensor configuration.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @return Pointer to the sensor configuration, or NULL if the ID is invalid.
 */
stm32f4_ultrasound_hw_t *_stm32f4_ultrasound_get(uint32_t ultrasound_id)
{
    // Return the pointer to the ultrasound sensor if the ID is valid, else return NULL.
    if (ultrasound_id < sizeof(ultrasounds_arr) / sizeof(ultrasounds_arr[0]))
    {
        return &ultrasounds_arr[ultrasound_id];
    }
    else
    {
        return NULL;
    }
}

/**
 * @brief Configures the trigger timer (TIM3) for the ultrasound sensor.
 * This timer generates the trigger pulse for the ultrasound.
 */
static void _timer_trigger_setup(void) {
    TIM_TypeDef *timer = STM32F4_REAR_PARKING_TRIGGER_TIMER;

    // Enable clock for the TIM3 timer
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    
    // Disable the timer to configure it
    timer->CR1 &= ~TIM_CR1_CEN;
    timer->CR1 |= TIM_CR1_ARPE;  // Enable auto-reload preload
    
    timer->CNT = 0;  // Reset the timer counter

    // Calculate the number of timer ticks needed for the trigger pulse
    double systemCoreClock = (double)SystemCoreClock;
    double pulse_activacion = (double)PORT_PARKING_SENSOR_TRIGGER_UP_US;
    double timer_frequency = systemCoreClock / 1000000.0;
    double ticks_needed = pulse_activacion * timer_frequency;

    // Calculate the prescaler and auto-reload values
    double prescaler_value = (systemCoreClock / (ticks_needed * 65536.0)) - 1.0;
    prescaler_value = round(prescaler_value);
    double auto_reload_value = ticks_needed / (prescaler_value + 1.0);
    auto_reload_value = round(auto_reload_value);

    // Adjust prescaler and auto-reload values if necessary
    if (auto_reload_value > 65535.0) {
        prescaler_value += 1.0;
        auto_reload_value = ticks_needed / (prescaler_value + 1.0);
        auto_reload_value = round(auto_reload_value);
    }
    
    // Set the calculated prescaler and auto-reload values
    timer->ARR = auto_reload_value;
    timer->PSC = prescaler_value;
    timer->EGR |= TIM_EGR_UG;  // Update the timer settings
    timer->SR = ~TIM_SR_UIF;   // Clear the update interrupt flag
    timer->DIER |= TIM_DIER_UIE; // Enable the update interrupt
    NVIC_SetPriority(TIM3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0)); // Set interrupt priority
}

/**
 * @brief Configures the echo timer (TIM2) for the ultrasound sensor.
 * This timer is used to capture the time when the echo signal is received.
 * @param ultrasound_id Identifier of the ultrasound sensor.
 */
static void _timer_echo_setup(uint32_t ultrasound_id) {
    TIM_TypeDef *timer = STM32F4_REAR_PARKING_ECHO_TIMER;

    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID) {
        // Enable clock for the TIM2 timer
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    
        // Configure the prescaler and auto-reload values
        timer->PSC = 15;  // PSC = (16MHz / 1MHz) - 1 = 15
        timer->ARR = 65535;  // ARR should be 65535 to count up to 65536 µs
    
        // Configure the capture input for echo signal
        timer->CCMR1 &= ~(0x3 << TIM_CCMR1_CC2S_Pos); 
        timer->CCMR1 |= (0x1 << TIM_CCMR1_CC2S_Pos);  // Set channel 2 in input capture mode
        timer->CCMR1 &= ~TIM_CCMR1_IC2F;  // Disable filter
        timer->CCER |= (1 << TIM_CCER_CC2P_Pos | 1 << TIM_CCER_CC2NP_Pos);  // Capture on both edges
        timer->CCMR1 &= ~(TIM_CCMR1_IC2PSC);  // No prescaler
        timer->CCER &= ~TIM_CCER_CC1E;  // Disable channel 1
        timer->DIER |= TIM_DIER_CC2IE;  // Enable capture interrupt for channel 2
        timer->DIER |= TIM_DIER_UIE;    // Enable update interrupt for overflow
        NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));  // Set interrupt priority
    
        timer->CCER |= TIM_CCER_CC2E;  // Enable capture on channel 2
        timer->CR1 &= ~TIM_CR1_CEN;    // Ensure the timer is disabled
    }
}

/**
 * @brief Configures a new timer (TIM5) for the measurement timeout.
 */
static void _timer_new_measurement_setup() {
    TIM_TypeDef *timer = STM32F4_REAR_PARKING_MEASUREMENT_TIMER;

    // Enable the clock for the timer
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    timer->CR1 &= ~TIM_CR1_CEN;  // Disable the timer before configuration
    timer->CR1 |= TIM_CR1_ARPE;  // Enable auto-reload preload
    timer->CNT = 0;  // Reset the timer counter

    // Calculate the timeout value for the measurement
    double systemCoreClock = (double)SystemCoreClock;
    double pulse_activacion = (double)PORT_PARKING_SENSOR_TIMEOUT_MS;
    double timer_frequency = systemCoreClock / 1000.0;
    double ticks_needed = pulse_activacion * timer_frequency;

    // Calculate prescaler and auto-reload values for the timeout
    double prescaler_value = ((ticks_needed / (65536.0) + 1.0) - 1.0);
    prescaler_value = round(prescaler_value);
    double auto_reload_value = (ticks_needed / (prescaler_value + 1.0)) - 1.0;
    auto_reload_value = round(auto_reload_value);

    // Adjust if the auto-reload value exceeds the maximum limit
    if (auto_reload_value > 65535.0) {
        prescaler_value += 1.0;
        auto_reload_value = (ticks_needed / (prescaler_value + 1.0)) - 1.0;
        auto_reload_value = round(auto_reload_value);
    }

    // Set the prescaler and auto-reload values
    timer->ARR = auto_reload_value;
    timer->PSC = prescaler_value;
    timer->EGR |= TIM_EGR_UG;  // Trigger an update to apply the settings
    timer->SR = ~TIM_SR_UIF;   // Clear the update interrupt flag
    timer->DIER |= TIM_DIER_UIE;  // Enable the update interrupt
    NVIC_SetPriority(TIM5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));  // Set interrupt priority
}

	
/* Public functions -----------------------------------------------------------*/

/**
 * @brief Initializes the ultrasound sensor (trigger, echo, and timers).
 * @param ultrasound_id The ID of the ultrasound sensor to initialize.
 */
void port_ultrasound_init(uint32_t ultrasound_id)
{
    // Get the ultrasound sensor configuration using the provided ultrasound ID
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);

    /* TO-DO alumnos: Additional initialization can be added here */

    // Initialize the state variables
    p_ultrasound->echo_end_tick = 0;
    p_ultrasound->echo_init_tick = 0;
    p_ultrasound->echo_received = false;
    p_ultrasound->trigger_end = false;
    p_ultrasound->trigger_ready = true;

    /* Trigger pin configuration */
    // Configure the GPIO for the trigger pin as an output
    stm32f4_system_gpio_config(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, STM32F4_GPIO_MODE_OUT, STM32F4_GPIO_PUPDR_NOPULL);

    /* Echo pin configuration */
    // Configure the GPIO for the echo pin as an alternate function (input)
    stm32f4_system_gpio_config(p_ultrasound->p_echo_port, p_ultrasound->echo_pin, STM32F4_GPIO_MODE_AF, STM32F4_GPIO_PUPDR_NOPULL);
    // Set the alternate function for the echo pin
    stm32f4_system_gpio_config_alternate(p_ultrasound->p_echo_port, p_ultrasound->echo_pin, p_ultrasound->echo_alt_fun);

    /* Configure timers for trigger and echo measurements */
    _timer_trigger_setup();  // Setup timer for trigger
    _timer_echo_setup(ultrasound_id);  // Setup timer for echo signal capture
    _timer_new_measurement_setup();  // Setup timer for new measurement cycle
}    

/**
 * @brief Stops the trigger timer and sets the trigger pin to low.
 * @param ultrasound_id The ID of the ultrasound sensor to stop.
 */
void port_ultrasound_stop_trigger_timer(uint32_t ultrasound_id)	
{
    // Get the ultrasound sensor configuration
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);

    // Set the trigger pin to low (stopping the trigger pulse)
    stm32f4_system_gpio_write(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, false);

    // Disable the trigger timer (stop counting)
    TIM3->CR1 &= ~TIM_CR1_CEN;
}

/* Getter and setter functions for ultrasound state variables */

/**
 * @brief Gets the current trigger ready state.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @return True if the trigger is ready, false otherwise.
 */
bool port_ultrasound_get_trigger_ready(uint32_t ultrasound_id) {
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->trigger_ready;
}

/**
 * @brief Sets the trigger ready state.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @param trigger_ready The new state for the trigger ready flag.
 */
void port_ultrasound_set_trigger_ready(uint32_t ultrasound_id, bool trigger_ready) {
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->trigger_ready = trigger_ready;
}

/**
 * @brief Gets the current trigger end state.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @return True if the trigger has ended, false otherwise.
 */
bool port_ultrasound_get_trigger_end(uint32_t ultrasound_id) {
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->trigger_end;
}

/**
 * @brief Sets the trigger end state.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @param trigger_end The new state for the trigger end flag.
 */
void port_ultrasound_set_trigger_end(uint32_t ultrasound_id, bool trigger_end) {
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->trigger_end = trigger_end;
}

/* Utility functions for setting up new GPIOs */

/**
 * @brief Sets a new trigger GPIO pin and port for the ultrasound sensor.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @param p_port The new port for the trigger pin.
 * @param pin The new pin for the trigger.
 */
void stm32f4_ultrasound_set_new_trigger_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->p_trigger_port = p_port;
    p_ultrasound->trigger_pin = pin;
}

/**
 * @brief Stops the echo timer for the ultrasound sensor.
 * @param ultrasound_id The ID of the ultrasound sensor.
 */
void port_ultrasound_stop_echo_timer(uint32_t ultrasound_id){
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID) {
        // Disable the echo timer (TIM2) for the rear parking sensor
        TIM2->CR1 &= ~TIM_CR1_CEN;
    }
}

/**
 * @brief Resets the echo ticks (initial and end ticks) for the ultrasound sensor.
 * @param ultrasound_id The ID of the ultrasound sensor.
 */
void port_ultrasound_reset_echo_ticks(uint32_t ultrasound_id){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_init_tick = 0;
    p_ultrasound->echo_end_tick = 0;
    p_ultrasound->echo_overflows = 0;
    p_ultrasound->echo_received = false;
}

/**
 * @brief Sets a new echo GPIO pin and port for the ultrasound sensor.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @param p_port The new port for the echo pin.
 * @param pin The new pin for the echo.
 */
void stm32f4_ultrasound_set_new_echo_gpio(uint32_t ultrasound_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->p_echo_port = p_port;
    p_ultrasound->echo_pin = pin;
}

/**
 * @brief Gets the initial echo tick value (start of echo signal).
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @return The initial echo tick value.
 */
uint32_t port_ultrasound_get_echo_init_tick(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_init_tick;
}	

/**
 * @brief Sets the initial echo tick value (start of echo signal).
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @param echo_init_tick The new initial echo tick value.
 */
void port_ultrasound_set_echo_init_tick(uint32_t ultrasound_id, uint32_t echo_init_tick){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_init_tick = echo_init_tick;
}	

/**
 * @brief Gets the final echo tick value (end of echo signal).
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @return The final echo tick value.
 */
uint32_t port_ultrasound_get_echo_end_tick(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_end_tick;
}	

/**
 * @brief Sets the final echo tick value (end of echo signal).
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @param echo_end_tick The new final echo tick value.
 */
void port_ultrasound_set_echo_end_tick(uint32_t ultrasound_id, uint32_t echo_end_tick){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_end_tick = echo_end_tick;
}	

/**
 * @brief Checks if the echo signal has been received.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @return True if echo signal received, false otherwise.
 */
bool port_ultrasound_get_echo_received(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_received;
}	

/**
 * @brief Sets the echo received state.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @param echo_received The new state for the echo received flag.
 */
void port_ultrasound_set_echo_received(uint32_t ultrasound_id, bool echo_received)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_received = echo_received;
}	

/**
 * @brief Gets the number of echo overflows for the ultrasound sensor.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @return The number of echo overflows.
 */
uint32_t port_ultrasound_get_echo_overflows(uint32_t ultrasound_id)
{
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    return p_ultrasound->echo_overflows;
}	

/**
 * @brief Sets the number of echo overflows for the ultrasound sensor.
 * @param ultrasound_id The ID of the ultrasound sensor.
 * @param echo_overflows The new number of echo overflows.
 */
void port_ultrasound_set_echo_overflows(uint32_t ultrasound_id, uint32_t echo_overflows){
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);
    p_ultrasound->echo_overflows = echo_overflows;
}	

/**
 * @brief Starts the ultrasound measurement process.
 * This includes setting the trigger pin high, enabling the timers, and enabling interrupts.
 * @param ultrasound_id The ID of the ultrasound sensor to start the measurement for.
 */
void port_ultrasound_start_measurement(uint32_t ultrasound_id){
    // Get the ultrasound sensor configuration
    stm32f4_ultrasound_hw_t *p_ultrasound = _stm32f4_ultrasound_get(ultrasound_id);

    // Set the trigger_ready flag to false (measurement is in progress)
    port_ultrasound_set_trigger_ready(ultrasound_id, false);

    // Reset the counters for the trigger timer, echo timer, and new measurement timer
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID) {
        TIM3->CNT = 0;  // Reset the counter for the trigger timer (TIM3)
        TIM2->CNT = 0;  // Reset the counter for the echo timer (TIM2)
    }
    TIM5->CNT = 0;  // Reset the counter for the new measurement timer (TIM5)

    // Set the trigger pin high to send the trigger pulse
    stm32f4_system_gpio_write(p_ultrasound->p_trigger_port, p_ultrasound->trigger_pin, true);

    // Enable interrupts for the timers
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID) {
        NVIC_EnableIRQ(TIM3_IRQn);  // Enable interrupt for trigger timer (TIM3)
        NVIC_EnableIRQ(TIM2_IRQn);  // Enable interrupt for echo timer (TIM2)
    }
    NVIC_EnableIRQ(TIM5_IRQn);  // Enable interrupt for new measurement timer (TIM5)

    // Enable the timers to start counting
    if (ultrasound_id == PORT_REAR_PARKING_SENSOR_ID) {
        TIM3->CR1 |= TIM_CR1_CEN;  // Enable the trigger timer (TIM3)
        TIM2->CR1 |= TIM_CR1_CEN;  // Enable the echo timer (TIM2)
    }
    TIM5->CR1 |= TIM_CR1_CEN;  // Enable the new measurement timer (TIM5)
}

/**
 * @brief Starts the new measurement timer.
 * This is used to track when a new measurement cycle is required.
 */
void port_ultrasound_start_new_measurement_timer(void){
    // Enable the interrupt for the new measurement timer (TIM5)
    NVIC_EnableIRQ(TIM5_IRQn);

    // Enable the new measurement timer
    TIM5->CR1 |= TIM_CR1_CEN;
}	

/**
 * @brief Stops the new measurement timer.
 * Disables the timer and its interrupts.
 */
void port_ultrasound_stop_new_measurement_timer(void){
    // Disable the new measurement timer (TIM5)
    TIM5->CR1 &= ~TIM_CR1_CEN;

    // Disable the interrupt for the new measurement timer (TIM5)
    NVIC_DisableIRQ(TIM5_IRQn);
}	

/**
 * @brief Stops all ultrasound timers (trigger, echo, and new measurement).
 * This function stops all timers and resets the echo ticks.
 * @param ultrasound_id The ID of the ultrasound sensor to stop.
 */
void port_ultrasound_stop_ultrasound(uint32_t ultrasound_id){
     // Stop the trigger timer
     port_ultrasound_stop_trigger_timer(ultrasound_id);

     // Stop the echo timer
     port_ultrasound_stop_echo_timer(ultrasound_id);

     // Stop the new measurement timer
     port_ultrasound_stop_new_measurement_timer();

     // Reset the echo ticks
     port_ultrasound_reset_echo_ticks(ultrasound_id);
}	


/**
 * @file stm32f4_button.c
 * @brief Portable functions to interact with the button FSM library. All portable functions must be implemented in this file.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 22/02/25
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

/* HW dependent includes */
#include "port_button.h" 
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_button.h"
#include <stddef.h>

/* Microcontroller dependent includes */
// TO-DO alumnos: include the necessary files to interact with the GPIOs


/* Typedefs --------------------------------------------------------------------*/

/**
 * @brief Struct representing the hardware configuration of a button.
 */
typedef struct {
    GPIO_TypeDef *p_port;   /**< Pointer to GPIO port */
    uint8_t pin;            /**< GPIO pin number */
    uint8_t pupd_mode;      /**< Pull-up/pull-down config */
    bool flag_pressed;      /**< Button press flag */
} stm32f4_button_hw_t;

/* Global variables ------------------------------------------------------------*/

/**
 * @brief Array storing the hardware configuration of all buttons.
 */
static stm32f4_button_hw_t buttons_arr[] = { 
    [PORT_PARKING_BUTTON_ID] = {.p_port = STM32F4_PARKING_BUTTON_GPIO, .pin = STM32F4_PARKING_BUTTON_PIN, .pupd_mode = STM32F4_GPIO_PUPDR_NOPULL},
}; 

/* Private functions ----------------------------------------------------------*/
/**
 * @brief Get the button status struct with the given ID.
 *
 * This function retrieves the button hardware configuration for a given button ID.
 * It checks if the button ID is valid and returns a pointer to the corresponding structure.
 *
 * @param button_id Button ID.
 *
 * @return Pointer to the button state struct.
 * @return NULL If the button ID is not valid.
 */
stm32f4_button_hw_t *_stm32f4_button_get(uint32_t button_id)
{
    // Validate the button ID and return the corresponding button configuration
    if (button_id < sizeof(buttons_arr) / sizeof(buttons_arr[0]))
    {
        return &buttons_arr[button_id];
    }
    else
    {
        return NULL;
    }
}

/* Public functions -----------------------------------------------------------*/

/**
 * @brief Initializes the button with the given ID.
 *
 * This function configures the GPIO port and pin of the button, sets the pull-up/pull-down
 * configuration, and configures the external interrupt for both rising and falling edges.
 *
 * @param button_id Button ID.
 */
void port_button_init(uint32_t button_id)
{
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    if (p_button == NULL) {
        return; 
    }

    /* TO-DO alumnos: Implement GPIO and EXTI configuration */
    // Configure the GPIO as an input with no pull-up/pull-down
    stm32f4_system_gpio_config(STM32F4_PARKING_BUTTON_GPIO, STM32F4_PARKING_BUTTON_PIN, STM32F4_GPIO_MODE_IN, STM32F4_GPIO_PUPDR_NOPULL);
    
    // Configure GPIO as an external interrupt on both edges
    stm32f4_system_gpio_config_exti(STM32F4_PARKING_BUTTON_GPIO, STM32F4_PARKING_BUTTON_PIN, STM32F4_TRIGGER_BOTH_EDGE | STM32F4_TRIGGER_ENABLE_INTERR_REQ);
    
    // Enable EXTI interrupt for the button pin
    stm32f4_system_gpio_exti_enable(STM32F4_PARKING_BUTTON_PIN, 1, 0);
}

/**
 * @brief Sets a new GPIO port and pin for a given button ID.
 *
 * This function allows setting a new GPIO port and pin for the button, which may be useful
 * if the button's hardware configuration changes or needs to be reconfigured.
 *
 * @param button_id Button ID.
 * @param p_port Pointer to the GPIO port.
 * @param pin GPIO pin number.
 */
void stm32f4_button_set_new_gpio(uint32_t button_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    
    p_button->p_port = p_port;
    p_button->pin = pin;
}

/**
 * @brief Checks if a button is pressed.
 *
 * This function checks the status of the button and returns whether the button is currently
 * pressed, based on the internal flag for the button.
 *
 * @param button_id Button ID.
 * @return True if the button is pressed, false otherwise.
 */
bool port_button_get_pressed(uint32_t button_id)
{
    stm32f4_button_hw_t *p;
    p =(stm32f4_button_hw_t *) _stm32f4_button_get(button_id);
    
    return p -> flag_pressed;
}	

/**
 * @brief Reads the value of a button (pressed or not pressed).
 *
 * This function directly reads the GPIO value of the button to determine if it is pressed.
 * It interacts with the hardware to read the state of the pin.
 *
 * @param button_id Button ID.
 * @return True if the button is pressed, false otherwise.
 */
bool port_button_get_value(uint32_t button_id) 
{
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    if (p_button == NULL) {
        return false; 
    }

    uint32_t pin = p_button -> pin;
    return stm32f4_system_gpio_read(p_button->p_port, pin);
}

/**
 * @brief Sets the pressed flag for a button.
 *
 * This function allows setting the pressed flag for a button, which indicates whether the
 * button is pressed or not based on the logic of the FSM.
 *
 * @param button_id Button ID.
 * @param pressed Boolean indicating if the button is pressed.
 */
void port_button_set_pressed(uint32_t button_id, bool pressed)
{
    // Retrieve the button hardware configuration
    stm32f4_button_hw_t *p;
    p =(stm32f4_button_hw_t *) _stm32f4_button_get(button_id);
    
    p -> flag_pressed = pressed;
}	

/**
 * @brief Checks if a button has a pending interrupt.
 *
 * This function checks if there is a pending interrupt for the given button. It reads the
 * interrupt pending register (PR) to determine if the button's pin has triggered an interrupt.
 *
 * @param button_id Button ID.
 * @return True if there is a pending interrupt, false otherwise.
 */
bool port_button_get_pending_interrupt(uint32_t button_id) 
{
    stm32f4_button_hw_t *p = _stm32f4_button_get(button_id);
    
    // Get the pin and check if the interrupt is pending
    uint32_t pin = p -> pin;
    uint32_t mask = 1 << pin;
    return (EXTI->PR & mask) != 0;
}	

/**
 * @brief Clears the pending interrupt for a button.
 *
 * This function clears the pending interrupt for the given button by writing to the
 * pending interrupt register (PR), ensuring that the interrupt flag is reset.
 *
 * @param button_id Button ID.
 */
void port_button_clear_pending_interrupt(uint32_t button_id)
{
    stm32f4_button_hw_t *p = _stm32f4_button_get(button_id);
    
    // Get the pin and clear the pending interrupt flag
    uint32_t pin = p -> pin;
    uint32_t mask = 1 << pin;
    EXTI->PR = mask;
}	

/**
 * @brief Disables interrupts for a button.
 *
 * This function disables interrupts for the given button. It deactivates the external interrupt
 * for the button's GPIO pin, stopping any further interrupt generation.
 *
 * @param button_id Button ID.
 */
void port_button_disable_interrupts(uint32_t button_id)
{
    stm32f4_button_hw_t *p = _stm32f4_button_get(button_id);
    
    uint32_t pin = p -> pin;
    stm32f4_system_gpio_exti_disable(pin);
}	

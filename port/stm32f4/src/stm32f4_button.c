/**
 * @file stm32f4_button.c
 * @brief Portable functions to interact with the button FSM library. All portable functions must be implemented in this file.
 * @author alumno1
 * @author alumno2
 * @date fecha
 */

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

/* HW dependent includes */
#include "port_button.h" // Used to get general information about the buttons (ID, etc.)
#include "port_system.h" // Used to get the system tick
#include "stm32f4_system.h"
#include "stm32f4_button.h"

/* Microcontroller dependent includes */
// TO-DO alumnos: include the necessary files to interact with the GPIOs


/* Typedefs --------------------------------------------------------------------*/
typedef struct
{
    GPIO_TypeDef *p_port;
    uint8_t pin;
    uint8_t pupd_mode;
    bool flag_pressed;
} stm32f4_button_hw_t;

/* Global variables ------------------------------------------------------------*/

static stm32f4_button_hw_t buttons_arr[] = { /* Array reprsenting the HW of each button in our system*/
    [PORT_PARKING_BUTTON_ID] = {.p_port = STM32F4_PARKING_BUTTON_GPIO, .pin = STM32F4_PARKING_BUTTON_PIN, .pupd_mode = STM32F4_GPIO_PUPDR_NOPULL},
}; 

/* Private functions ----------------------------------------------------------*/
/**
 * @brief Get the button status struct with the given ID.
 *
 * @param button_id Button ID.
 *
 * @return Pointer to the button state struct.
 * @return NULL If the button ID is not valid.
 */
stm32f4_button_hw_t *_stm32f4_button_get(uint32_t button_id)
{
    // Return the pointer to the button with the given ID. If the ID is not valid, return NULL.
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
void port_button_init(uint32_t button_id)
{
    // Retrieve the button struct using the private function and the button ID
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);


    /* TO-DO alumnos */
    stm32f4_system_gpio_config(STM32F4_PARKING_BUTTON_GPIO, STM32F4_PARKING_BUTTON_PIN, STM32F4_GPIO_MODE_IN, STM32F4_GPIO_PUPDR_NOPULL);
    stm32f4_system_gpio_config_exti(STM32F4_PARKING_BUTTON_GPIO, STM32F4_PARKING_BUTTON_PIN, STM32F4_TRIGGER_BOTH_EDGE | STM32F4_TRIGGER_ENABLE_INTERR_REQ);
    stm32f4_system_gpio_exti_enable(STM32F4_PARKING_BUTTON_PIN, 1, 0);
}

void stm32f4_button_set_new_gpio(uint32_t button_id, GPIO_TypeDef *p_port, uint8_t pin)
{
    stm32f4_button_hw_t *p_button = _stm32f4_button_get(button_id);
    p_button->p_port = p_port;
    p_button->pin = pin;
}


bool port_button_get_pressed(uint32_t button_id)
{
    stm32f4_button_hw_t *p;
    p =(stm32f4_button_hw_t *) _stm32f4_button_get(button_id);
    return p -> flag_pressed;
}	

bool port_button_get_value(uint32_t button_id) 
{
    

    
}


void port_button_set_pressed(uint32_t button_id, bool pressed)
{
    stm32f4_button_hw_t *p;
    p =(stm32f4_button_hw_t *) _stm32f4_button_get(button_id) ;
    p -> flag_pressed = pressed;
}	

bool port_button_get_pending_interrupt(uint32_t button_id) 
{
    stm32f4_button_hw_t *p = _stm32f4_button_get(button_id) ;
    uint32_t pin = p -> pin;
    uint32_t mask = 1 << pin;
    return EXTI->PR & mask != 0;
}	

void port_button_clear_pending_interrupt(uint32_t button_id)
{
    stm32f4_button_hw_t *p = _stm32f4_button_get(button_id) ;
    uint32_t pin = p -> pin;
    uint32_t mask = 1 << pin;
    if (EXTI->PR & (mask)) {
        EXTI->PR = (mask); 
    }
}	

void port_button_disable_interrupts(uint32_t button_id){
    stm32f4_button_hw_t *p = _stm32f4_button_get(button_id) ;
    uint32_t pin = p -> pin;
    uint32_t mask = 1 << pin;
    stm32f4_system_gpio_exti_disable(pin);
}	


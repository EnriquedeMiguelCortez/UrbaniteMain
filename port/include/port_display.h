/**
 * @file port_display.h
 * @brief Header for the portable functions to interact with the HW of the display system.
 * 
 * These functions must be implemented in the platform-specific code to handle the display system's
 * initialization and RGB LED control.
 * 
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 07/04/25
 */

#ifndef PORT_DISPLAY_SYSTEM_H_
#define PORT_DISPLAY_SYSTEM_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Typedefs ------------------------------------------------------------------*/

/**
 * @brief Structure to represent an RGB color.
 */
typedef struct {
    uint8_t r; /**< Red component (0-255) */
    uint8_t g; /**< Green component (0-255) */
    uint8_t b; /**< Blue component (0-255) */
} rgb_color_t;

/* Defines and enums --------------------------------------------------------*/

/**
 * @brief Identifier for the rear parking display.
 */
#define PORT_REAR_PARKING_DISPLAY_ID 0 

/**
 * @brief Maximum value for any RGB color component.
 */
#define PORT_DISPLAY_RGB_MAX_VALUE 255

/**
 * @brief Predefined color constants.
 */
#define COLOR_RED       (rgb_color_t){255, 0, 0}   /**< Red color */
#define COLOR_GREEN     (rgb_color_t){0, 255, 0}   /**< Green color */
#define COLOR_BLUE      (rgb_color_t){0, 0, 255}   /**< Blue color */
#define COLOR_YELLOW    (rgb_color_t){96, 96, 0}   /**< Yellow color */
#define COLOR_TURQUOISE (rgb_color_t){25, 90, 82}  /**< Turquoise color */
#define COLOR_OFF       (rgb_color_t){0, 0, 0}     /**< Display off (no color) */

/* Function prototypes ------------------------------------------------------*/

/**
 * @brief Initializes the display system with a given display ID.
 * 
 * This function must be called before using any other display functions.
 * 
 * @param display_id Identifier of the display to initialize.
 */
void port_display_init(uint32_t display_id);

/**
 * @brief Sets the RGB color of the specified display.
 * 
 * @param display_id Identifier of the display.
 * @param color RGB color to set.
 */
void port_display_set_rgb(uint32_t display_id, rgb_color_t color);

/**
 * @brief Gets the current RGB color of the specified display.
 * 
 * @param display_id Identifier of the display.
 * @return rgb_color_t Current RGB color of the display.
 */
rgb_color_t port_display_get_rgb(uint32_t display_id);

#endif /* PORT_DISPLAY_SYSTEM_H_ */
/**
 * @file port_ultrasound.h
 * @brief Header for the portable functions to interact with the HW of the ultrasound sensors. The functions must be implemented in the platform-specific code.
 * @author Enrique De Miguel Cortez
 * @author Sergio Rojas Castilla
 * @date 03/03/25
 */
#ifndef PORT_ULTRASOUND_H_
#define PORT_ULTRASOUND_H_

/* Includes ------------------------------------------------------------------*/
/* Standard C includes */

#include <stdint.h>
#include <stdbool.h>

/* Defines and enums ----------------------------------------------------------*/

/**
 * @brief ID of the rear parking ultrasound sensor.
 */
#define PORT_REAR_PARKING_SENSOR_ID  0

/**
 * @brief Duration of the trigger signal in microseconds.
 */
#define PORT_PARKING_SENSOR_TRIGGER_UP_US  10

/**
 * @brief Timeout duration between measurements in milliseconds.
 */
#define PORT_PARKING_SENSOR_TIMEOUT_MS 100

/**
 * @brief Speed of sound in air, in meters per second.
 */
#define SPEED_OF_SOUND_MS  343

/* Function prototypes and explanation -------------------------------------------------*/

/**
 * @brief Configure the hardware specifications of a given ultrasound sensor.
 * 
 * This function initializes the hardware configuration for the ultrasound sensor, such as
 * setting up the GPIO pins, timers, or communication protocols.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to initialize.
 */
void port_ultrasound_init (uint32_t ultrasound_id);

/**
 * @brief Start a new measurement with the ultrasound sensor.
 * 
 * This function triggers the ultrasound sensor to begin a new distance measurement by
 * sending a trigger signal to the sensor. It typically involves a pulse on the trigger pin.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to start the measurement.
 */
void port_ultrasound_start_measurement (uint32_t ultrasound_id);

/**
 * @brief Stop the timer that controls the trigger signal.
 * 
 * This function halts the timer responsible for sending the trigger signal to the ultrasound sensor,
 * which might be used to limit the pulse duration or manage trigger timing.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor whose trigger timer is to be stopped.
 */
void port_ultrasound_stop_trigger_timer (uint32_t ultrasound_id);

/**
 * @brief Stop the timer that controls the echo signal.
 * 
 * This function halts the timer responsible for tracking the echo signal duration,
 * which is used to measure the round-trip time for the sound pulse.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor whose echo timer is to be stopped.
 */
void port_ultrasound_stop_echo_timer (uint32_t ultrasound_id);

/**
 * @brief Start the timer that controls the new measurement timing.
 * 
 * This function starts a timer that manages the interval between consecutive measurements.
 * 
 * @param None
 */
void port_ultrasound_start_new_measurement_timer (void);

/**
 * @brief Stop the timer that controls the new measurement timing.
 * 
 * This function stops the timer that manages the interval between consecutive measurements.
 * 
 * @param None
 */
void port_ultrasound_stop_new_measurement_timer (void);

/**
 * @brief Reset the time ticks of the echo signal after the distance has been calculated.
 * 
 * Once the distance measurement has been completed, this function resets the timer ticks
 * that were used to capture the echo signal.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor whose echo time ticks are to be reset.
 */
void port_ultrasound_reset_echo_ticks (uint32_t ultrasound_id);

/**
 * @brief Stop all timers of the ultrasound sensor and reset the echo ticks.
 * 
 * This function stops all active timers related to the ultrasound sensor (trigger, echo, etc.)
 * and resets any accumulated echo ticks. This ensures that the sensor is reset and ready for
 * a fresh measurement cycle.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to stop and reset.
 */
void port_ultrasound_stop_ultrasound (uint32_t ultrasound_id);

/**
 * @brief Get the readiness of the trigger signal.
 * 
 * This function checks whether the ultrasound sensor is ready to initiate a new measurement,
 * based on the state of the trigger signal.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to check trigger readiness.
 * @return True if the ultrasound sensor is ready to start a new measurement; false otherwise.
 */
bool port_ultrasound_get_trigger_ready (uint32_t ultrasound_id);

/**
 * @brief Set the readiness of the trigger signal.
 * 
 * This function sets the state of the trigger signal, indicating whether the sensor is ready
 * to initiate a new measurement.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to set trigger readiness.
 * @param trigger_ready The desired trigger readiness state (true or false).
 */
void port_ultrasound_set_trigger_ready (uint32_t ultrasound_id, bool trigger_ready);

/**
 * @brief Get the status of the trigger signal.
 * 
 * This function checks if the time allocated for the trigger signal has ended,
 * indicating that the trigger process is complete and the signal is low.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to check trigger status.
 * @return True if the trigger signal has ended, false otherwise.
 */
bool port_ultrasound_get_trigger_end (uint32_t ultrasound_id);

/**
 * @brief Set the status of the trigger signal.
 * 
 * This function updates the status of the trigger signal, marking the completion of the
 * trigger process. It sets the trigger signal as either high or low depending on the input.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to set trigger status.
 * @param trigger_end The desired trigger end status (true for finished, false for ongoing).
 */
void port_ultrasound_set_trigger_end (uint32_t ultrasound_id, bool trigger_end);

/**
 * @brief Get the time tick when the initiation of the echo signal was received.
 * 
 * This function retrieves the timestamp (in time ticks) when the echo signal was first detected,
 * which helps in calculating the round-trip time of the sound pulse.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to retrieve the echo init time.
 * @return The timestamp of when the echo signal was received.
 */
uint32_t port_ultrasound_get_echo_init_tick (uint32_t ultrasound_id);

/**
 * @brief Set the time tick when the initiation of the echo signal was received.
 * 
 * This function sets the timestamp of when the echo signal is received, which is important for
 * calculating the distance based on the round-trip time.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to set the echo init time.
 * @param echo_init_tick The timestamp of when the echo signal was received.
 */
void port_ultrasound_set_echo_init_tick (uint32_t ultrasound_id, uint32_t echo_init_tick);

/**
 * @brief Get the time tick when the echo signal ended.
 * 
 * This function retrieves the timestamp of when the echo signal ends, which is required
 * to calculate the round-trip time.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to retrieve the echo end time.
 * @return The timestamp of when the echo signal ended.
 */
uint32_t port_ultrasound_get_echo_end_tick (uint32_t ultrasound_id);

/**
 * @brief Set the time tick when the echo signal ended.
 * 
 * This function sets the timestamp of when the echo signal ended, which helps in determining
 * the round-trip time and subsequently the distance.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to set the echo end time.
 * @param echo_end_tick The timestamp of when the echo signal ended.
 */
void port_ultrasound_set_echo_end_tick (uint32_t ultrasound_id, uint32_t echo_end_tick);

/**
 * @brief Get whether the echo signal has been received.
 * 
 * This function checks if the echo signal has been received from the ultrasound sensor, indicating
 * the sensor has completed its measurement process.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to check echo reception.
 * @return True if the echo has been received, false otherwise.
 */
bool port_ultrasound_get_echo_received (uint32_t ultrasound_id);

/**
 * @brief Set whether the echo signal has been received.
 * 
 * This function sets the flag indicating whether the echo signal has been received or not.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to set echo received status.
 * @param echo_received True if the echo has been received, false otherwise.
 */
void port_ultrasound_set_echo_received (uint32_t ultrasound_id, bool echo_received);

/**
 * @brief Get the number of overflows that have occurred during the echo signal measurement.
 * 
 * This function retrieves the number of overflows that occurred in the timer used to measure
 * the echo signal time. This is important for cases where the echo signal duration exceeds
 * the timer's capacity.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to get echo overflows.
 * @return The number of overflows that occurred.
 */
uint32_t port_ultrasound_get_echo_overflows (uint32_t ultrasound_id);

/**
 * @brief Set the number of overflows that have occurred during the echo signal measurement.
 * 
 * This function sets the number of overflows that occurred in the timer used to measure
 * the echo signal time.
 * 
 * @param ultrasound_id The identifier for the ultrasound sensor to set echo overflows.
 * @param echo_overflows The number of overflows that occurred.
 */
void port_ultrasound_set_echo_overflows (uint32_t ultrasound_id, uint32_t echo_overflows);

#endif /* PORT_ULTRASOUND_H_ */

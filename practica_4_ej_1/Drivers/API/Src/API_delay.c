#include "API_delay.h"
#include "stm32f4xx_hal.h"


/**
 * @brief inits the given @delay_t
 *
 *
 * @param delay delay_t struct to be initialized in this function
 * @param duration to be set for the given delay
 *
 * @note in case of a null pointer, this function does nothing
 */
void delayInit( delay_t * delay, tick_t duration ) {
	if (delay == NULL) {
		return;
	}

	delay->running = false;
	delay->duration = duration;
}

/**
 * @brief returns the current state of the given delay
 *
 * @param delay delay_t struct to get the current state
 * @return true if the delay time has already been met, otherwise false
 *
 * @note:
 *  - If the delay time has already been met, 'running' state of delay_t will be set to false
 *  - In case of a null pointer, it returns false
 */
bool_t delayRead( delay_t * delay ) {
	if (delay == NULL) {
		return false;
	}

	if ( !delay->running ) {
		delay->running = true;
		delay->startTime = HAL_GetTick();
		return false;
	}

	if (HAL_GetTick() - delay->startTime > delay->duration ) {
		delay->running = false;
		return true;
	}

	return false;
}

/**
 * @brief changes the duration of the given delay_t
 *
 * @param delay to be modified
 * @param duration to be set in the delay_t
 *
 * @note in case of a null pointer this function does nothing
 */
void delayWrite( delay_t * delay, tick_t duration ) {
    if (delay == NULL) {
        return;
    }

	delay->duration = duration;
}


/**
 * @brief returns the state of the given delay
 *
 * @param delay from which the state is obtained
 *
 * @return FALSE if the given delay is NULL or it it's state is 'not running', otherwise TRUE
 */
bool_t delayIsRunning(delay_t * delay) {
	return delay != NULL && delay->running;
}



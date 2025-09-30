#ifndef API_INC_API_ACTIONS_H_
#define API_INC_API_ACTIONS_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

bool led_action(uint8_t* action);

void help_action();

void status_action();

void ping_action();

void baud_rate_action();

void clear_action();

#endif /* API_INC_API_ACTIONS_H_ */

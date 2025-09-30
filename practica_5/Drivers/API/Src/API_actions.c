#include "API_actions.h"
#include "API_uart.h"
#include <string.h>

#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA

// LED command args
static uint8_t TURN_ON_LED_CMD[] = "ON";
static uint8_t TURN_OFF_LED_CMD[] = "OFF";
static uint8_t TOGGLE_LED_CMD[] = "TOGGLE";

// Error messages
static uint8_t PONG_RESPONSE[] =  "\r\nPONG";
static uint8_t BAUD_RATE_RESPONSE[] =  "\r\n9600";
static uint8_t CLEAR_RESPONSE[] =  "\033[2J";
static uint8_t HELP_RESPONSE[] =
		"\r\nCOMMANDS:\r\n"
			"\tHELP: prints the available commands\r\n"
			"\tPING: PONG answers\r\n"
			"\tBAUD_RATE: returns the baud rate for UART\r\n"
			"\tCLEAR: clear serial monitor screen\r\n"
			"\tLED ON|OFF|TOGGLE: performs one of the given actions over the on-board LED";


/**
 * @brief performs an action over the on-board LED
 *
 * @param action: action to be performed, must be ON, OFF or TOGGLE
 *
 * @return TRUE if the action cab be performed correctly, otherwise false
 *
 */
bool led_action(uint8_t* action) {
	if (!strcmp((char*)action, (char*)TURN_ON_LED_CMD)) {
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		return true;
	}

	if (!strcmp((char*)action, (char*)TURN_OFF_LED_CMD)) {
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		return true;
	}

	if (!strcmp((char*)action, (char*)TOGGLE_LED_CMD)) {
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		return true;
	}

	return false;
}

/**
 * @brief prints the commands that cmdparser accepts
 *
 */
void help_action() {
	uartSendString(HELP_RESPONSE);
}

/**
 * @brief simple function that answers 'PONG'
 *
 */
void ping_action() {
	uartSendString(PONG_RESPONSE);
}

/**
 * @brief sends the baud rate of the UART handler
 *
 */
void baud_rate_action() {
	uartSendString(BAUD_RATE_RESPONSE);
}

/**
 * @brief clears the serial monitor screen
 *
 */
void clear_action() {
	uartSendString(CLEAR_RESPONSE);
}

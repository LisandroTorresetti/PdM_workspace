#include "API_cmdparser.h"
#include "API_uart.h"
#include "API_delay.h"
#include "API_actions.h"
#include <string.h>

#define CMD_MAX_LINE 64
#define CMD_MAX_TOKENS 5 // CMD + Arg1 ... Arg4
#define ERR_MSG_MAX_LENGTH 50

typedef enum {
  CMD_OK,
  CMD_ERR_TIMEOUT,
  CMD_ERR_OVERFLOW,
  CMD_ERR_UNKNOWN,
  CMD_ERR_SYNTAX,
  CMD_ERR_ARG,
  CMD_IDLE,
  CMD_RECV,
  CMD_PARSE,
  CMD_EXEC,
} cmd_status_t;

// Array of commands that the cmdparser can process
static uint8_t *VALID_CMDS[] = {
    (uint8_t *)"HELP",
    (uint8_t *)"PING",
    (uint8_t *)"LED",
    (uint8_t *)"BAUD_RATE",
    (uint8_t *)"CLEAR"
};

static uint8_t PROMPT[] = "\r\n> ";

static cmd_status_t parser_status;

static uint8_t cmd_buffer_idx;
// cmd_buffer: this is where the data coming from UART is stored.
static uint8_t cmd_buffer[CMD_MAX_LINE];
// cmd_tokens: the first element is the command and the rest are the arguments
static uint8_t cmd_tokens[CMD_MAX_TOKENS][CMD_MAX_LINE];

// variables to trigger a Timeout Error in case the user does not finish sending the command
static tick_t TIMEOUT = 10000;
static delay_t cmd_timeout_delay;

static bool idle_check_flag;

// Prototypes
static void cmdparser_reset();
static void set_idle_status();

static void handle_idle_status();
static void handle_recv_status();
static void handle_error_status();
static void handle_parse_status();
static void handle_exec_status();

static bool is_valid_char(uint8_t character);
static void clear_buffer(uint8_t* buffer);
static void echo(uint8_t* pstring);


/**
 * @brief inits the cmdparser
 *
 * @return true if the cmdparser was initialized correctly, otherwise false
 *
 */
bool cmdparser_init() {
	if (!uartInit()) {
		return false;
	}

	set_idle_status();
	cmd_buffer_idx = 0;

	delayInit(&cmd_timeout_delay, TIMEOUT);
	return true;
}

/**
 * @brief reads the command that the user send and handle the state of the cmdparser
 *
 * @note in case of being in an invalid state, the FSM is reseted
 */
void cmdparser_read_cmd() {
	switch (parser_status) {
	case CMD_IDLE:
		if (!idle_check_flag) {
			uartSendString(PROMPT);
			idle_check_flag = true;
		}

		handle_idle_status();
		break;
	case CMD_RECV:
		handle_recv_status();
		break;
	case CMD_PARSE:
		handle_parse_status();
		break;
	case CMD_OK:
		parser_status = CMD_EXEC;
		break;
	case CMD_EXEC:
		handle_exec_status();
		break;
	case CMD_ERR_TIMEOUT:
		handle_error_status();
		break;
	case CMD_ERR_OVERFLOW:
		handle_error_status();
		break;
	case CMD_ERR_ARG:
		handle_error_status();
		break;
	case CMD_ERR_UNKNOWN:
		handle_error_status();
		break;
	case CMD_ERR_SYNTAX:
		handle_error_status();
		break;
	default:
		cmdparser_reset();
	}
}

/**
 * @brief puts the cmdparser in an IDLE state
 *
 */
void set_idle_status() {
	parser_status = CMD_IDLE;
	idle_check_flag = false;
}

/**
 * @brief resets the cmdparser
 *
 * This function clear buffers, put the cmdparser in an IDLE state and restarts the timeout delay
 *
 */
void cmdparser_reset() {
	set_idle_status();
	clear_buffer(cmd_buffer);
	cmd_buffer_idx = 0;
	delayInit(&cmd_timeout_delay, TIMEOUT);
}

/**
 * @brief handles the IDLE status
 *
 * Waits for a command, and if it receive one, it transitions to RECV state and the command is copied into the buffer.
 * Otherwise, it remains in the same state.
 *
 */
void handle_idle_status() {
	uint8_t raw_cmd_buffer[CMD_MAX_LINE];
	clear_buffer(raw_cmd_buffer);
	uartReceiveStringSize(raw_cmd_buffer, CMD_MAX_LINE);

	uint8_t *pointer = raw_cmd_buffer;
	if (strlen((char*)pointer)) {
		echo(raw_cmd_buffer);
		while(*pointer) {
			if (!is_valid_char(*pointer)) {
				parser_status = CMD_ERR_SYNTAX;
				return;
			}

			cmd_buffer[cmd_buffer_idx++] = *pointer++;
		}

		parser_status = CMD_RECV;
		// Init delay to check timeout errors
		delayRead(&cmd_timeout_delay);
	}
}

/**
 * @brief handles the RECV status
 *
 * This function continues receiving and copying the user's command until a line break or carriage return occurs, and if so, it moves to state CMD_PARSE.
 *
 * @note This function can move to the following error states:
 * - CMD_ERR_TIMEOUT: if the newline o carriage returns never occurs within a time interval
 * - CMD_ERR_OVERFLOW: if the command length is greater than the max allowed length
 * - CMD_ERR_SYNTAX: if an invalid character was sent
 *
 */
void handle_recv_status() {
	uint8_t raw_cmd_buffer[CMD_MAX_LINE];
	clear_buffer(raw_cmd_buffer);
	uartReceiveStringSize(raw_cmd_buffer, 1);

	if (delayRead(&cmd_timeout_delay)) {
		parser_status = CMD_ERR_TIMEOUT;
		return;
	}

	uint8_t *pointer = raw_cmd_buffer;
	if (strlen((char*)pointer)) {
		echo(raw_cmd_buffer);

		while(*pointer) {
			if (cmd_buffer_idx >= CMD_MAX_LINE) {
				parser_status = CMD_ERR_OVERFLOW;
				return;
			}

			if (!is_valid_char(*pointer)) {
				parser_status = CMD_ERR_SYNTAX;
				return;
			}

			if (*pointer == '\n' || *pointer == '\r') {
				cmd_buffer[cmd_buffer_idx] = '\0';
				parser_status = CMD_PARSE;
				return;
			}

			cmd_buffer[cmd_buffer_idx++] = *pointer++;
		}
	}
}

/**
 * @brief parses the command entered by the user
 *
 * If the command is valid, it puts the cmdparser in an CMD_OK state, otherwise to error states can be set:
 * - CMD_ERR_ARG: if the command has more args than the allowed amount (4)
 * - CMD_ERR_UNKNOWN: if the command is unknown
 *
 */
void handle_parse_status() {
	for (uint8_t idx = 0; idx < CMD_MAX_TOKENS; idx++) {
		clear_buffer(cmd_tokens[idx]);
	}

	uint8_t token_idx = 0;
	uint8_t idx = 0;
	uint8_t *cmd_iterator = cmd_buffer;
	while (*cmd_iterator) {
		if (*cmd_iterator == ' ') {
			if (token_idx >= CMD_MAX_TOKENS) {
				parser_status = CMD_ERR_ARG;
				return;
			}

			cmd_tokens[token_idx][idx] = '\0';

			// Copy next cmd token
			token_idx++;
			idx = 0;

			while (*cmd_iterator == ' ') cmd_iterator++;
			continue;
		}

		uint8_t character = *cmd_iterator++;
		if (character >= 'a' && character <= 'z') {
			character = character - ('a' - 'A');
		}

		cmd_tokens[token_idx][idx++] = character;
	}

	// Check if command exists
	bool cmd_exists = false;
	uint8_t amount_of_commands = sizeof(VALID_CMDS) / sizeof(VALID_CMDS[0]);
	for (int idx = 0; idx < amount_of_commands; idx++) {
		if (!strcmp((char*)VALID_CMDS[idx], (char*)cmd_tokens[0])) {
			cmd_exists = true;
			break;
		}
	}

	parser_status = cmd_exists ? CMD_OK : CMD_ERR_UNKNOWN;
}

/**
 * @brief runs the user command
 *
 * If the command is executed correctly, it resets the cmdparser. Otherwise it puts the cdmparse in the corresponding error state
 *
 */
void handle_exec_status() {
	char* char_cmd = (char*) cmd_tokens[0];
	if (!strcmp(char_cmd, "LED")) {
		uint8_t amount_of_args = 0;
		for (uint8_t idx = 1; idx < CMD_MAX_TOKENS; idx++) {
			if (*cmd_tokens[idx] == '\0'){
				break;
			}

			amount_of_args++;
		}

		if (amount_of_args > 1) {
			parser_status = CMD_ERR_ARG;
			return;
		}

		if (!led_action(cmd_tokens[1])) {
			parser_status = CMD_ERR_ARG;
			return;
		}

	} else if (!strcmp(char_cmd, "HELP")) {
		help_action();
	} else if (!strcmp(char_cmd, "PING")) {
		ping_action();
	} else if (!strcmp(char_cmd, "BAUD_RATE")) {
		baud_rate_action();
	} else if (!strcmp(char_cmd, "CLEAR")) {
		clear_action();
	} else {
		parser_status = CMD_ERR_UNKNOWN;
		return;
	}

	cmdparser_reset();
}

/**
 * @brief handles all possible errors from cmdparser
 *
 * Sends a message depending on the error that occurred and then resets the cmdparser
 *
 */
void handle_error_status() {
	uint8_t error_msg[ERR_MSG_MAX_LENGTH];

	switch (parser_status) {
	case CMD_ERR_TIMEOUT:
		strcpy((char*)error_msg, "\n\rERROR: timeout");
		break;
	case CMD_ERR_OVERFLOW:
		strcpy((char*)error_msg, "\n\rERROR: line too long");
		break;
	case CMD_ERR_ARG:
		strcpy((char*)error_msg, "\n\rERROR: bad args");
		break;
	case CMD_ERR_UNKNOWN:
		strcpy((char*)error_msg, "\n\rERROR: unknown cmd");
		break;
	case CMD_ERR_SYNTAX:
		strcpy((char*)error_msg, "\n\rERROR: syntax");
		break;
	default:
		strcpy((char*)error_msg, "\n\rERROR: unknown");
	}

	uartSendString(error_msg);
	cmdparser_reset();
}


/**
 * @brief inits the given buffer with '\0'
 *
 * @param buffer: buffer to fill with '\0'
 */
void clear_buffer(uint8_t* buffer) {
	for (uint32_t idx = 0; idx < CMD_MAX_LINE; idx++) {
		buffer[idx] = '\0';
	}
}

/**
 * @brief checks if the given character is valid
 *
 * @note valid characters are: \n, \r, \0, _, ' ', and letters (uppercase or lowercase)
 *
 * @param character to be check
 */
bool is_valid_char(uint8_t character) {
    if (character == '\n' || character == '\r' || character == '\0' || character == '_' || character == ' ') {
        return true;
    }

    if (character >= 'A' && character <= 'Z') {
        return true;
    }

    if (character >= 'a' && character <= 'z') {
        return true;
    }

    return false;
}

/**
 * @brief sends the given character using UART
 *
 * @param pstring: character to be sent
 *
 */
void echo(uint8_t* pstring) {
	uartSendString(pstring);
}


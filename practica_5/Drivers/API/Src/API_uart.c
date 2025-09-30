#include "API_uart.h"
#include "stm32f4xx_hal.h"

// Error messages
static uint8_t RX_ERROR_MSG[] =  "error receiving message";
static uint8_t TX_ERROR_MSG[] =  "error transmitting the message";
static uint8_t INTERNAL_ERROR_MSG[] =  "internal error";

// Rx/Tx timeout
static const uint32_t TIMEOUT = 1000;

static UART_HandleTypeDef uart_handler;

static const uint8_t CONFIG_JSON[] =
		"\033[2J{\r\n"
			"\t\"BaudRate\": 9600,\r\n"
			"\t\"WordLength\": \"UART_WORDLENGTH_9B\",\r\n"
			"\t\"StopBits\": \"UART_STOPBITS_1\",\r\n"
			"\t\"Parity\": \"UART_PARITY_ODD\",\r\n"
			"\t\"HwFlowCtl\": \"UART_HWCONTROL_NONE\",\r\n"
			"\t\"Mode\": \"UART_MODE_TX_RX\",\r\n"
			"\t\"OverSampling\": \"UART_OVERSAMPLING_16\",\r\n"
			"\t\"Instance\": \"USART2\"\r\n"
		"}\r\n";

// Prototypes
static void send_error_msg(uint8_t* error_msg);
static uint16_t get_string_length(const uint8_t* pstring);

/**
 * @brief Initializes the UART peripheral.
 *
 * Configures the UART with the following settings:
 * - Baud rate: 9600
 * - Word length: 9 bits
 * - Stop bits: 1
 * - Parity: Odd
 * - Hardware flow control: None
 * - Mode: TX/RX
 * - Oversampling: 16
 *
 * After initialization, it sends a JSON string over UART describing
 * the configuration.
 *
 * @return TRUE if the UART was successfully initialized,
 *         FALSE otherwise.
 */
bool uartInit() {
	UART_InitTypeDef uart_init_config = {
			.BaudRate = 9600,
			.WordLength = UART_WORDLENGTH_9B,
			.StopBits = UART_STOPBITS_1,
			.Parity = UART_PARITY_ODD,
			.HwFlowCtl = UART_HWCONTROL_NONE,
			.Mode = UART_MODE_TX_RX,
			.OverSampling = UART_OVERSAMPLING_16,
	};

	uart_handler.Instance = USART2;
	uart_handler.Init = uart_init_config;

	if (HAL_UART_Init(&uart_handler) != HAL_OK) {
		return false;
	}

	uint16_t config_length = get_string_length(CONFIG_JSON);
	if (HAL_UART_Transmit(&uart_handler, CONFIG_JSON, config_length, TIMEOUT) != HAL_OK) {
		send_error_msg(TX_ERROR_MSG);
		return false;
	}

	return true;
}

/**
 * @brief Sends a null-terminated string over UART.
 *
 * The function determines the string length by searching for the
 * null terminator (`'\0'`) before transmitting.
 *
 * @param pstring  Pointer to the null-terminated string to transmit.
 *
 * @note if a NULL pointer is received, an error message is sent
 *
 */
void uartSendString(uint8_t* pstring) {
	if (pstring == NULL) {
		send_error_msg(INTERNAL_ERROR_MSG);
		return;
	}

	uint16_t str_length = get_string_length(pstring);
	uartSendStringSize(pstring, str_length);
}

/**
 * @brief Sends a fixed-size string over UART.
 *
 * Transmits exactly size bytes from the given string buffer.
 *
 * @param pstring  Pointer to the string buffer to transmit.
 * @param size     Number of bytes to transmit.
 *
 * @note if a NULL pointer is received, an error message is sent
 *
 */
void uartSendStringSize(uint8_t* pstring, uint16_t size) {
	if (pstring == NULL || size == 0) {
		send_error_msg(INTERNAL_ERROR_MSG);
		return;
	}


	if (HAL_UART_Transmit(&uart_handler, pstring, size, TIMEOUT) != HAL_OK) {
		send_error_msg(TX_ERROR_MSG);
	}
}

/**
 * @brief Receives a fixed-size string over UART.
 *
 * Reads exactly @p size bytes from UART into the given buffer.
 *
 * @param pstring  Pointer to the buffer where received data will be stored.
 * @param size     Number of bytes to receive, must be greater than 0.
 *
 * @note if a NULL pointer is received, an error message is sent
 *
 */
void uartReceiveStringSize(uint8_t* pstring, uint16_t size) {
	if (pstring == NULL || size == 0) {
		send_error_msg(INTERNAL_ERROR_MSG);
		return;
	}

	if (HAL_UART_Receive(&uart_handler, pstring, size, TIMEOUT) != HAL_OK) {
		// For exercise one uncomment the following line
		//send_error_msg(RX_ERROR_MSG);
	}
}

/**
 * @brief sends an error message
 * *
 * @param erro_msg  Message to be sent
 *
 */
void send_error_msg(uint8_t* error_msg) {
	uint16_t msg_length = get_string_length(error_msg);
	uartSendStringSize(error_msg, msg_length);
}

/**
 * @brief returns the string length
 * *
 * @param pstring: string whose length you want to obtain
 *
 * @return uint16_t the length of the string
 *
 */
uint16_t get_string_length(const uint8_t* pstring) {
	uint16_t counter = 0;
	if (pstring == NULL) {
		return counter;
	}

	while (pstring[counter] != '\0') {
		counter++;
	}

	return counter;
}




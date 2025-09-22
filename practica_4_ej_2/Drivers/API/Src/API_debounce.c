#include "API_debounce.h"
#include "stm32f4xx_hal.h"

#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC

// debounceState_t represents the possible states of the button
typedef enum{
	BUTTON_UP,
	BUTTON_FALLING,
	BUTTON_DOWN,
	BUTTON_RAISING,
} debounceState_t;


static debounceState_t button_state;
static bool button_pressed = false;


/**
 * @brief inits the button_state variable with its corresponding initial value
 *
 */
void debounceFSM_init() {
	button_state = BUTTON_UP;
}


/**
 * @brief returns if the button was pressed or not
 *
 * @return TRUE if the button was pressed, otherwise FALSE
 *
 * @note when this function is called, button_pressed is set to false
 *
 */
bool readKey() {
	bool result = button_pressed;
	button_pressed = false;
	return result;

}

/**
 * @brief handles the state of the button, and changes it when necessary.
 *
 * When a transition from FALLING to DOWN is going to be made, button_pressed is set to true
 *
 * @note in case of a not known button state value, the default action is set it to BUTTON_UP and set button_pressed to false
 *
 */
void debounceFSM_update() {
	GPIO_PinState current_state = !HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

	switch (button_state) {
	case BUTTON_UP:
		if (current_state == GPIO_PIN_SET) {
			button_state = BUTTON_FALLING;
		}

		break;

	case BUTTON_DOWN:
		if (current_state == GPIO_PIN_RESET) {
			button_state = BUTTON_RAISING;
		}

		break;

	case BUTTON_RAISING:
		if (current_state == GPIO_PIN_RESET) {
			button_state = BUTTON_UP;
		} else {
			button_state = BUTTON_DOWN;
		}

		break;

	case BUTTON_FALLING:
		if (current_state == GPIO_PIN_SET) {
			button_state = BUTTON_DOWN;
			button_pressed = true;
		} else {
			button_state = BUTTON_UP;
		}

		break;

	default:
		button_state = BUTTON_UP;
		button_pressed = false;
		break;
	}

}

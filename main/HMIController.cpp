/**
 * @file HMIController.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief 
 * @version v0.0.1
 * @date 2024-01-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "HMIController.hpp"


static char LOGTAG[]="HMIController";


HMIController::HMIController(ConfigurationManager &configurationManager):
  Task("HMIController", 3072, 5),
  m_UserButton(GPIOClass( SMARTFIT_USER_BUTTON, GPIO_MODE_INPUT , GPIO_PULLUP_ONLY)),
  m_LED1(GPIOClass( SMARTFIT_LED_1, GPIO_MODE_OUTPUT, GPIO_PULLDOWN_ONLY, GPIO_DRIVE_CAP_DEFAULT)),
  m_LED2(GPIOClass( SMARTFIT_LED_2, GPIO_MODE_OUTPUT, GPIO_PULLDOWN_ONLY, GPIO_DRIVE_CAP_DEFAULT)),
  m_LED3(GPIOClass( SMARTFIT_LED_3, GPIO_MODE_OUTPUT, GPIO_PULLDOWN_ONLY, GPIO_DRIVE_CAP_DEFAULT)),
  m_configurationManager(configurationManager) {

	ESP_LOGI(LOGTAG, "Starting...");

	//is this a reset?
	checkReset();

	Task::start();

	//set blink to signalice the start-up process
	setLED1State(HMIController::LEDState::BLINK_FAST);
}

HMIController::~HMIController() {
	ESP_LOGI(LOGTAG, "Exiting...");
}

void HMIController::run(void *data) {

	while(1) {

		// HMI-Handling will be done every 10ms
		// This provides a stable timebase for input debouncing and potential LED dimming/blinking
		FreeRTOS::sleep(10);		//Sleep for 10ms

		userButtonDebounce();		//Check the user Button

		handleLED1State();
		handleLED2State();
		handleLED3State();
	}
}

bool HMIController::isButtonPressed() {
	return m_UserButtonPressed;
}

bool HMIController::userButtonWasPressed(void) {
	if (m_UserButtonWasPressed) {
		m_UserButtonWasPressed = false;
		return true;
	}
	return false;
}

bool HMIController::userButtonWasReleased(void) {
	if (m_UserButtonWasReleased) {
		m_UserButtonWasReleased = false;
		return true;
	}
	return false;
}

void HMIController::setLED1State(LEDState newState) {
	m_LEDStates[0] = newState;
}

void HMIController::setLED2State(LEDState newState) {
	m_LEDStates[1] = newState;
}

void HMIController::setLED3State(LEDState newState) {
	m_LEDStates[2] = newState;
}

void HMIController::userButtonDebounce(void) {
	static uint8_t userButtonCounter=0;
	if (m_UserButton.read() == 0) { 	// Button pressed
		if (userButtonCounter < 10) {	//Must be pressed for 10*10ms
			userButtonCounter++;
		}else{
			if (!m_UserButtonPressed){
				m_UserButtonPressed = true;
				m_UserButtonWasPressed = true;
			}
		}
	}else{								//Button released
		if (userButtonCounter > 0) {	//Must be released for 10*10ms
			userButtonCounter--;
		}else{
			if (m_UserButtonPressed) {
				m_UserButtonPressed = false;
				m_UserButtonWasReleased = true;
			}
		}
	}
}
void HMIController::handleLED1State(void) {
	static uint8_t blinkCounter=0;

	switch(m_LEDStates[0]) {
	case LEDState::OFF:
		m_LED1.reset();
		break;
	case LEDState::BLINK_SLOW:
		blinkCounter++;
		if (blinkCounter >  100) blinkCounter = 0;
		if (blinkCounter > 50)  {
			m_LED1.set();
		}else{
			m_LED1.reset();
		}
		break;
	case LEDState::BLINK_FAST:
		blinkCounter++;
		if (blinkCounter >  20) blinkCounter = 0;
		if (blinkCounter > 10)  {
			m_LED1.set();
		}else{
			m_LED1.reset();
		}
		break;
	case LEDState::ON:
		m_LED1.set();
		break;
	}
}

void HMIController::handleLED2State(void) {
	static uint8_t blinkCounter=0;

	switch(m_LEDStates[1]) {
	case LEDState::OFF:
		m_LED2.reset();
		break;
	case LEDState::BLINK_SLOW:
		blinkCounter++;
		if (blinkCounter >  100) blinkCounter = 0;
		if (blinkCounter > 50)  {
			m_LED2.set();
		}else{
			m_LED2.reset();
		}
		break;
	case LEDState::BLINK_FAST:
		blinkCounter++;
		if (blinkCounter >  20) blinkCounter = 0;
		if (blinkCounter > 10)  {
			m_LED2.set();
		}else{
			m_LED2.reset();
		}
		break;
	case LEDState::ON:
		m_LED2.set();
		break;
	}
}

void HMIController::handleLED3State() {
	static uint8_t blinkCounter=0;

	switch(m_LEDStates[2]) {
	case LEDState::OFF:
		m_LED3.reset();
		break;
	case LEDState::BLINK_SLOW:
		blinkCounter++;
		if (blinkCounter >  100) blinkCounter = 0;
		if (blinkCounter > 50)  {
			m_LED3.set();
		}else{
			m_LED3.reset();
		}
		break;
	case LEDState::BLINK_FAST:
		blinkCounter++;
		if (blinkCounter >  20) blinkCounter = 0;
		if (blinkCounter > 10)  {
			m_LED3.set();
		}else{
			m_LED3.reset();
		}
		break;
	case LEDState::ON:
		m_LED3.set();
		break;
	}
}

void HMIController::checkReset(){
	if (m_UserButton.read() == 0) {
		//blinking to show the comming reset
		for(int i = 1; i< 50; i++){
			m_LED1.set();
			m_LED2.set();
			m_LED3.set();
			FreeRTOS::sleep(1000/i);
			m_LED1.reset();
			m_LED2.reset();
			m_LED3.reset();
			FreeRTOS::sleep(1000/i);
		}
		//coutdown end -> reset if still pressed
		if (m_UserButton.read() == 0) {
			m_LED1.set();
			m_LED2.set();
			m_LED3.set();
			m_configurationManager.factoryReset();
			FreeRTOS::sleep(5000);
		}
		m_LED1.reset();
		m_LED2.reset();
		m_LED3.reset();
		FreeRTOS::sleep(3000);
	}
}

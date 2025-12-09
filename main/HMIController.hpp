/**
 * @brief
 *
 * @file HMIController.hpp
 * @author your name
 * @date 2018-08-02
 */

#ifndef HMI_CONTROLLER_H_
#define HMI_CONTROLLER_H_

//#include "FreeRTOSTimer.hpp"
#include "FreeRTOS.hpp"
#include "GPIO.hpp"
#include "Task.hpp"
#include "System.hpp"
#include "esp_log.h"

#include "I2CArbiter.hpp"
#include "BQ32000.hpp"
#include "Definitions.hpp"
#include "Timer.hpp"
#include "TimerInterface.h"

#include "ConfigurationManager.hpp"

class HMIController:public Task {
public:
	HMIController(ConfigurationManager &configurationManager);
	~HMIController(void);

	enum class LEDState {OFF, ON, BLINK_SLOW, BLINK_FAST};

	bool isButtonPressed(void);
	bool userButtonWasPressed(void);
	bool userButtonWasReleased(void);

	void setLED1State(LEDState newState);
	void setLED2State(LEDState newState);
	void setLED3State(LEDState newState);


private:
	void run(void *data);

	bool 	m_UserButtonPressed{false};
	bool 	m_UserButtonWasPressed{false};
	bool 	m_UserButtonWasReleased{false};

	void	userButtonDebounce(void);


	void 	handleLED1State(void);
	void 	handleLED2State(void);
	void 	handleLED3State(void);

	bool 			m_isStarted{false};
    //FreeRTOSTimer* 	m_HMITimer;
	LEDState		m_LEDStates[3]{LEDState::OFF,LEDState::OFF,LEDState::OFF};

	GPIOClass 		m_UserButton;
	GPIOClass 		m_LED1;
	GPIOClass 		m_LED2;
	GPIOClass 		m_LED3;

	ConfigurationManager	&m_configurationManager;

	void			checkReset();
};

#endif

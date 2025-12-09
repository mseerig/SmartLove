# Useage of FreeRTOS Timer and Task

Blink a LED with Timer.
The timer is started from a FreeRTOS-Task.

```cpp
#include <string>
#include <esp_log.h>

#include "Task.h"
#include "FreeRTOS.h"
#include "FreeRTOSTimer.h"
#include "GPIO.h"

#include "sdkconfig.h"

using namespace std;

extern "C" {
void app_main(void);
}

void callback(FreeRTOSTimer *pTimer);

class Main_Task: public Task {

public:
	bool gpioState;
	gpio_num_t gpioNum;
	Main_Task(){
		gpioState=false;
		gpioNum=(gpio_num_t) 16;
	}

	void run(void *args) override {
		ESP32CPP::GPIO::setOutput(gpioNum);

		FreeRTOSTimer timer((char*)"Blink Timer", 1000/portTICK_PERIOD_MS, true, this, callback);
		timer.start(1/portTICK_PERIOD_MS);

		stop(); //stops the Task -> only the timer is working...
	}
}
;

static Main_Task *main_Task;

void callback(FreeRTOSTimer *pTimer) {
	Main_Task* main = (Main_Task*)pTimer->getData();

	 main->gpioState=! main->gpioState;

	if(main->gpioState) ESP32CPP::GPIO::high(main->gpioNum);
	else ESP32CPP::GPIO::low(main->gpioNum);

    ESP_LOGD("Callback", "%d", main->gpioState);
}

void app_main(void) {

	main_Task = new Main_Task();
	main_Task->start();

}
```
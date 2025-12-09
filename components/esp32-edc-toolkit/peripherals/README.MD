# Examples
## UART
- send and receive something
```cpp
UART uart(uartNum,baud,rxPin,txPin);
uart.send("HALLO");

FreeRTOS::sleep(WAIT_FOR_ANSWER_DELAY);

std::string res = "";
while(uart.hasData()){
	res += (char)uart.readByte();
	FreeRTOS::sleep(15);
}
```

## SDCard and SPIFFS
- Mont SDCard
- write to file
- read from file

```cpp
#include <fstream>
#include "SDCard.h"

/** MOUNT SD **/
SDCard sd;
sd.initSPI(SDMMC_FREQ_DEFAULT, PIN_NUM_MISO, PIN_NUM_MOSI, PIN_NUM_CLK,
PIN_NUM_CS);
sd.mount("/sdcard");

/** WRITE TO FILE **/
std::ofstream outFile;
outFile.open("/sdcard/foo.txt", std::fstream::out | std::fstream::app);
if (outFile) {
	outFile << "Hallo Welt! \n";
} else {
	ESP_LOGD("APP", "Failed to open file for writing");
}
outFile.close();

/** READ FROM FILE **/
std::ifstream inFile;
inFile.open("/sdcard/foo.txt", std::fstream::in);
if (inFile) {
	int i=0;
	std::string buffer = "";
	while (!inFile.eof()) {
		i++;
		std::getline(inFile, buffer); //get a line
		ESP_LOGD("APP", "Read from file [line %d]: %s", i, buffer.c_str());
	}
} else {
	ESP_LOGD("APP", "Failed to open file for reading");
}
inFile.close();
```
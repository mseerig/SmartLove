# Documentation

# Examples
## JSON

```cpp
#include <string>
#include <esp_log.h>

#include "Task.h"
#include "FreeRTOS.h"
#include "sdkconfig.h"

#include "JSON.h"

#include "sdkconfig.h"

using namespace std;

extern "C" {
void app_main(void);
}

class Main_Task: public Task {
	void run(void *args) override {
		string m_json;
		JSON json;

		/* CREATE OBJECT */
		printf("\n\nCREATE OBJECT\n");

		JsonObject json_obj_out = json.createObject();
		json_obj_out.setInt("test", 123);

		JsonArray json_arr_out = json.createArray();
		json_arr_out.addString("rot");
		json_arr_out.addString("grün");
		json_arr_out.addString("blau");

		json_obj_out.setArray("farben", json_arr_out);
		m_json = json_obj_out.toString();

		printf("%s\n", m_json.c_str());
		/*   OUTPUT:    */
		//		{
		//		"test": 123,
		//		"farben": ["rot", "gr?n", "blau"]
		//		}

		/* PHRASE OBJECT */
		printf("\n\nPHRASE OBJECT\n");

		JsonObject json_obj_in = json.parseObject(m_json);
		int test = json_obj_in.getInt("test");
		printf("test: %d\n", test);

		JsonArray json_arr_in = json_obj_in.getArray("farben");
		for (int i = 0; i < json_arr_in.size(); i++) {
			string item = json_arr_in.getString(i);
			printf("ArreyItem[%d]: %s\n", i, item.c_str());
		}

		for (;;)
			FreeRTOS::sleep(20000);
	}
};

static Main_Task *main_Task;

void app_main(void) {

	main_Task = new Main_Task();
	main_Task->start();

}
```

## JSON_RPC

```cpp
#include <string>
#include <esp_log.h>

#include "Task.h"
#include "FreeRTOS.h"
#include "sdkconfig.h"

#include "JSON_RPC.h"
#include "JSON.h"

extern "C" {
void app_main(void);
}

// Simple test class
class Parameter{
    public:
        Parameter();
        int test;
};

Parameter::Parameter(){
    test = 12345; // Simple test member
}

// This is my JSON-RPC callback function!
int helloWorld(JsonObject *io, void* data){
	//get my pointer to my Parameter class
    Parameter* m_para = (Parameter*) data; // not needed in this example

    // looking for the given parameters
    // returning these, if it is an string
    if(io->isStringItem("params")) io->setString("result", io->getString("params"));
    else return JSONRPC_INVALID_PARAMETER; // return an error, if not

    return 0; // means success
}

class Main_Task: public Task {
	void run(void *args) override {
		Parameter para; // test class

		//create JSON-RPC server
		JSON_RPC rpc;
		// add my callback function to the parser
		rpc.addMethod(helloWorld, "helloWorld", &para);

		// create example JSON-RPC call
		JsonObject jsonObject = JSON::createObject();
		jsonObject.setString("jsonrpc", JSONRPC_VERSION);
		jsonObject.setString("method", "helloWorld");
		jsonObject.setString("params", "Halo123");
		jsonObject.setString("id", "1");

		ESP_LOGD("App","JSON-RPC Request: %s",jsonObject.toString().c_str());

		std::string response;

		for(;;){
			// parse my given JSON-RPC request string in to a JSON-RPC Response string
			response = rpc.parse(jsonObject.toString());

			ESP_LOGD("App","JSON-RPC Response: %s", response.c_str());
			FreeRTOS::sleep(1000);
		}
	}
}
;

static Main_Task *main_Task;

void app_main(void) {

	main_Task = new Main_Task();
	main_Task->start();

}
```
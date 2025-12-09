/*
 * JSON_RPC.cpp
 *
 *  Created on: 23.02.2018
 *      Author: marcel.seerig
 */

#include "JSON_RPC.hpp"

#include <string>
#include <vector>
#include <utility>
#include <tuple>

#include <esp_log.h>

#include "sdkconfig.h"

#include "Memory.hpp"

static const char *LOG_TAG = "JSON_RPC";

/**
 * @brief JSON-RPC class constructor.
 */
JSON_RPC::JSON_RPC() {

} // JSON_RPC

/**
 * @brief JSON-RPC destructor.
 */
JSON_RPC::~JSON_RPC() {
	methodMapper.erase(methodMapper.begin(), methodMapper.end());
} // ~JSON_RPC

/**
 * @brief Adding a function to the function mapper.
 * @param [in] The callback function, you want to call.
 * @param [in] A string reverence to the function. (JSON-RPC method name)
 * This should be the normally same like the function name.
 * @return N/A.
 */
void JSON_RPC::addMethod(callback_function method, std::string name,
		void* data) {
	methodMapper.push_back(make_tuple(method, name, data));
} // addMethod

/**
 * @brief Removing a function from the function mapper.
 * @param [in] A string reverence to the function. (JSON-RPC method name)
 * @return N/A.
 */
void JSON_RPC::removeMethod(std::string name) {
	int element = 0;
	while (std::get < 1 > (methodMapper[element]) != name) {
		element++;
		if (element > methodMapper.size())
			return;
	}
	methodMapper.erase(methodMapper.begin() + element);
} // removeMethod

/**
 * @brief Internal call function, which looks for the function in the
 * function router and call that function. Otherwise, a negative error code
 * is returned.
 * @param [in] A string reverence to the function. (JSON-RPC method name)
 * @param [in] JsonObject with an element "params" from the JSON-RPC request.
 * @return Zero for success, or JSONRPC_INVALID_PARAMETER.
 * This function will return JSONRPC_INTERNAL_ERROR by it self, if the given
 * function name is not listed in the function mapper.
 */
int JSON_RPC::callMethod(std::string name, JsonVariant& input, JsonObject& output) {
	int element = 0;
	while (std::get < 1 > (methodMapper[element]) != name) {
		element++;
		if (element > methodMapper.size()) {
			return JSONRPC_METHOD_NOT_FOUND;
		}
	}

	callback_function fkt = std::get < 0 > (methodMapper[element]);
	void* para = std::get < 2 > (methodMapper[element]);

	return fkt(input, output, para);
} // callMethod

/**
 * @brief Parse the JSON-RPC request.
 * Batch calls should be solved here in to single calls. (not supported jet)
 * @param [in] JSON_RPC request as string.
 * @return JSON-RPC response as string.
 */
std::string JSON_RPC::parse(std::string request) {
	DynamicJsonDocument input(2048);
	DynamicJsonDocument output(2048);
    DeserializationError err = deserializeJson(input, request);

	// is Json vaild?
	if(err==DeserializationError::Ok){

		JsonArray inArr = input.as<JsonArray>();
		JsonObject inObj = input.as<JsonObject>();

		if(inArr.isNull()){
			// parse single obj
			if(!inObj.isNull()){
				JsonObject outObj = output.to<JsonObject>();
				parseSingleMsg(inObj, outObj);

				std::string outStr;
				serializeJson(output, outStr);
				return outStr;
			}else{
				return parseError();
			}
		}else{
			// parse batch msg
			JsonArray outArray = output.to<JsonArray>();
			for(JsonVariant v : inArr) {
    			JsonObject _inObj = v.as<JsonObject>();

				if(!_inObj.isNull()){
					JsonObject outObj = outArray.createNestedObject();
					parseSingleMsg(_inObj, outObj);
				}else{
					return parseError();
				}
			}

			std::string outStr;
			serializeJson(output, outStr);
			return outStr;
		}
	}

	return parseError();
} // parse

/**
 * @brief returns a JSON_RPC parse error msg as string
 *
 * @return std::string error msg
 */
std::string JSON_RPC::parseError(){
	StaticJsonDocument<20> doc;
	JsonObject obj = doc.as<JsonObject>();
	obj["jsonrpc"] = JSONRPC_VERSION;
	setError(obj, JSONRPC_PARSE_ERROR);
	obj["id"] = nullptr;
	std::string outStr;
	serializeJson(doc, outStr);
	return outStr;
} // parseError

/**
 * @brief Parse a single JSON-RPC message an call the given function.
 * If needed, error responses are created here.
 * @param [in] JSON-RPC response object of an single call.
 * @param [out] JSON-RPC request object of an single call.
 * @return N/A.
 */
void JSON_RPC::parseSingleMsg(JsonObject& inObj, JsonObject& outObj) {

	outObj["jsonrpc"] = JSONRPC_VERSION;
	if(inObj.containsKey("id")) outObj["id"] = inObj["id"];
	else outObj["id"] = nullptr;

	// all jsonrpc items included?
	if (outObj["jsonrpc"] == JSONRPC_VERSION && inObj.getMember("method").is<const char*>()) {

		//call function an get result/error
		JsonVariant input = inObj.getMember("params");
		int ret = callMethod(inObj["method"], input, outObj);

		// if result ok, or userdefined error
		if( ret >= 0 || ( ret <= -32000 && ret >= -32099 )){

			// map the result/error from function throw..
			// only if this msg is no notification
			if(inObj.containsKey("id")){
				if(!outObj.containsKey("result") && !outObj.containsKey("error")){
					ESP_LOGE(LOG_TAG, "The function result is empty, but this was no notification!");
				}
			}

		}else{
			// other errors
			setError(outObj, ret);
		}

	} else {
		// JSONRPC_INVALID_REQUEST
		setError(outObj, JSONRPC_INVALID_REQUEST);
	}
} // parseSingleMsg

/**
 * @brief Creating a error sub object item by the error code.
 * @param [out] JSON-RPC response object, were the error goes.
 * @param [in] JSON-RPC defined error code.
 * @return N/A.
 */
void JSON_RPC::setError(JsonObject& obj, int errorCode){
	JsonObject err = obj.createNestedObject("error");
	err["code"] = errorCode;
	err["message"] = errorCodeToString(errorCode);
} // setError

/**
 * @brief Generating the string equivalent to the error code.
 * @param [in] JSON-RPC defined error code.
 * @return error code as string.
 */
std::string JSON_RPC::errorCodeToString(int errorCode){
	switch(errorCode){
		case JSONRPC_PARSE_ERROR      : return "Parse error";
		case JSONRPC_INVALID_REQUEST  : return "Invalid Request";
		case JSONRPC_METHOD_NOT_FOUND : return "Method not found";
		case JSONRPC_INVALID_PARAMETER: return "Invalid params";
		case JSONRPC_INTERNAL_ERROR   : return "Internal error";
		default: return "Server error";
	}
} // errorCodeToString

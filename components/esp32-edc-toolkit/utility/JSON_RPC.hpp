/*
 * JSON_RPC.h
 *
 *  Created on: 23.02.2018
 *      Author: marcel.seerig
 *
 *  This is a JSON-RPC 2.0 implementation using the cJSON over layering cpp class.
 *  Official Specification: http://www.jsonrpc.org/specification
 *
 * * NOTICE:
 * * The "rpc call Batch" is currently not supported!
 *
 * * Example project: https://github.com/mseerig/ESP32-JSON-RPC-example-project
 *
 */

#ifndef MAIN_JSON_RPC_H_
#define MAIN_JSON_RPC_H_

#include "ArduinoJson.h"

#include <string>
#include <vector>
#include <utility>
#include <tuple>

#define JSONRPC_VERSION                "2.0"    // MUST be exactly "2.0".
#define JSONRPC_PARSE_ERROR            -32700   // Invalid JSON was received by the server. An error occurred on the server while parsing the JSON text.
#define JSONRPC_INVALID_REQUEST        -32600   // The JSON sent is not a valid Request object.
#define JSONRPC_METHOD_NOT_FOUND       -32601   // The method does not exist / is not available.
#define JSONRPC_INVALID_PARAMETER      -32602   // Invalid method parameter(s).
#define JSONRPC_INTERNAL_ERROR         -32603   // Internal JSON-RPC error.
//#define JSONRPC_SERVER_ERROR -32000 to -32099 Server error Reserved for implementation-defined server-errors.

typedef int (*callback_function)(JsonVariant& input, JsonObject& output, void* data);

/**
 * @brief JSON-RPC manager an parser class.
 */
class JSON_RPC{
public:
	JSON_RPC();
	~JSON_RPC();

	void addMethod(callback_function method, std::string name, void* data=nullptr);
	void removeMethod(std::string name);

	std::string parse(std::string request);

private:
	std::vector<std::tuple<callback_function,std::string, void*> > methodMapper; // (function router)
	std::string parseError();
	void        parseSingleMsg(JsonObject& inObj, JsonObject& outObj);
	int         callMethod(std::string name, JsonVariant& input, JsonObject& output);
	void        setError(JsonObject& obj, int errorCode);
	std::string errorCodeToString(int errorCode);

}; // JSON_RPC

#endif /* MAIN_JSON_RPC_H_ */

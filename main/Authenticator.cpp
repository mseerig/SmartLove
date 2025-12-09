/**
 * @file Authenticator.cpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief Handles the api access tokens for the JSON_RPC client.
 * @version 0.1
 * @date 2019-03-18
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "Definitions.hpp"
#include "Authenticator.hpp"
#include "NVS.hpp"
#include "System.hpp"
#include "ArduinoJson.h"
#include "SHA256.hpp"
#include <string>
#include <algorithm> // std::reverse
#include <vector>
#include <utility>
#include "esp_log.h"

static char TAG[] = "Authenticator";

/**
 * @brief Construct a new Authenticator:: Authenticator object
 *
 */
Authenticator::Authenticator(ConfigurationManager &configurationManager):
	m_configurationManager(configurationManager){

	ESP_LOGI(TAG, "Starting...");

	readVault();
}

/**
 * @brief Destroy the Authenticator:: Authenticator object
 *
 */
Authenticator::~Authenticator(){

}

void Authenticator::readVault(void){
	m_userList.erase(m_userList.begin(), m_userList.end());
	//read NVS Data
	ESP_LOGV(TAG, "read NVS");
	std::string _userData = "";
	if(m_configurationManager.getAuthenticatorNVS().get("userData", &_userData) != ESP_OK) {
		ESP_LOGD(TAG, "NVS userData not vialed!");
		reset();
		return;
	}
	ESP_LOGV(TAG, "vault: '%s'", _userData.c_str());
	//validate NVS Data
	ESP_LOGV(TAG, "validate NVS");
	std::string crc = "";
	if(m_configurationManager.getAuthenticatorNVS().get("CRC", &crc) != ESP_OK) {
		ESP_LOGD(TAG, "NVS crc not vialed!");
		reset();
		return;
	}
	ESP_LOGV(TAG, "calculate hash NVS");
	SHA256 sha;
	sha.update((unsigned char*)_userData.c_str(), _userData.length());
	if(sha.getResultHash() != crc){
		ESP_LOGD(TAG, "NVS crc hash and calculated hash are not identical!");
		reset();
		return;
	}
	//read users in to list
	ESP_LOGV(TAG, "read users to list");
	const size_t capacity = 30 + JSON_OBJECT_SIZE(2) + (JSON_ARRAY_SIZE(AUTHENTICATOR_MAX_NUMBER_OF_USERS + 1) * JSON_OBJECT_SIZE(3));
	DynamicJsonDocument userData(capacity);
	DeserializationError error = deserializeJson(userData, _userData);
	if (error) reset();

	if(!userData.containsKey("numUsers") || !userData.containsKey("userList")){
		ESP_LOGD(TAG, "Can not read 'numUsers', or 'userList'!");
		reset();
		return;
	}
	int numUsers = userData["numUsers"];
	JsonArray userList = userData.getMember("userList").as<JsonArray>();

	ESP_LOGV(TAG, "found '%d' users in vault.", numUsers);

	for(JsonVariant v : userList) {
		//add user if all properies are available, else skip
    	JsonObject user = v.as<JsonObject>();
		if(user.containsKey("username") &&
		user.containsKey("password") &&
		user.containsKey("authLevel")){

			addUser(
				user["username"],
				user["password"],
				(auth_level_t)user["authLevel"].as<int>()
			);

		}else ESP_LOGE(TAG, "A user was skipped in read process!");
	}

	m_initDone = true;
	ESP_LOGD(TAG, "Users are added!");
}

void Authenticator::commit(void){
	m_configurationManager.getAuthenticatorNVS().erase(); //clean vault

	//parse userData in to my json format
	const size_t capacity = 30 + JSON_OBJECT_SIZE(2) + (JSON_ARRAY_SIZE(AUTHENTICATOR_MAX_NUMBER_OF_USERS + 1) * JSON_OBJECT_SIZE(3));
	DynamicJsonDocument userData(capacity);

	userData["numUsers"] = m_userList.size();
	JsonArray userList = userData.createNestedArray("userList");

	for(auto it = m_userList.begin(); it != m_userList.end() ; ++it){
		JsonObject user = userList.createNestedObject();
		user["username"] = it->getUsername();
		user["password"] = it->getPassword();
		user["authLevel"] = (int)it->getAuthLevel();
	}

	//now, calculating the CRC and store it
	std::string outStr;
	serializeJson(userData, outStr);

	SHA256 sha;
	sha.update((unsigned char*)outStr.c_str(), outStr.length());
	m_configurationManager.getAuthenticatorNVS().set("CRC", sha.getResultHash());
	m_configurationManager.getAuthenticatorNVS().set("userData", outStr);

	ESP_LOGD(TAG, "All users are stored with success!");
}

Account* Authenticator::getUser(std::string username){
	for(int i = 0; i<m_userList.size(); i++) {
		if(m_userList.at(i).getUsername() == username) return &m_userList.at(i);
	}
	return nullptr;
}

auth_state_t Authenticator::addUser(std::string username, std::string password, auth_level_t authLevel){
	if(getUser(username) != nullptr){
		ESP_LOGD(TAG, "Add user not allowed, user sill exist!");
		return AUTH_STATE_USER_SILL_EXISTS;
	}
	if(m_userList.size() >= AUTHENTICATOR_MAX_NUMBER_OF_USERS){
		ESP_LOGD(TAG, "Database is full!");
		return AUTH_STATE_NOT_ALLOWED;
	}
	m_userList.push_back(Account(username, password, authLevel));
	if(m_initDone) commit();
	return AUTH_STATE_OK;
}

auth_state_t Authenticator::deleteUser(std::string username){
	for(auto it = m_userList.begin(); it != m_userList.end() ; ++it){
		if (it->getUsername() == username) {
			m_userList.erase(it);
			return AUTH_STATE_OK;
		}
	}
	return AUTH_STATE_USER_NOT_FOUND;
}

auth_state_t Authenticator::changeUsername(std::string username, std::string newUsername){
	Account* user = getUser(username);
	if(user == nullptr) return AUTH_STATE_USER_NOT_FOUND;
	user->setUsername(newUsername);
	commit();
	return AUTH_STATE_OK;
}

auth_state_t Authenticator::changePassword(std::string username, std::string newPassword){
	Account* user = getUser(username);
	if(user == nullptr) return AUTH_STATE_USER_NOT_FOUND;
	user->setPassword(newPassword);
	commit();
	return AUTH_STATE_OK;
}

auth_state_t Authenticator::changeUsernameAndPassword(std::string username, std::string newUsername, std::string newPassword){
	Account* user = getUser(username);
	if(user == nullptr) return AUTH_STATE_USER_NOT_FOUND;
	user->setUsername(newUsername);
	user->setPassword(newPassword);
	commit();
	return AUTH_STATE_OK;
}

auth_state_t Authenticator::login(std::string username, std::string password, std::string* token){
	Account* user = getUser(username);
	if(user == nullptr) return AUTH_STATE_USER_NOT_FOUND;
	if(user->getPassword() != password) return AUTH_STATE_INVALIDED_PASSWORD;
	if(user->getToken() == "") user->generateToken();
	*token = user->getToken();

	return AUTH_STATE_OK;
}

auth_state_t Authenticator::logout(std::string username){
	Account* user = getUser(username);
	if(user == nullptr) return AUTH_STATE_USER_NOT_FOUND;
	user->deleteToken();
	return AUTH_STATE_OK;
}

auth_state_t Authenticator::check(std::string username, std::string token, auth_level_t authLevel){
	Account* user = getUser(username);
	if(user == nullptr) return AUTH_STATE_USER_NOT_FOUND;
	if(user->getToken() != token) return AUTH_STATE_INVALIDED_TOKEN;
	if(token == "") return AUTH_STATE_INVALIDED_TOKEN;
	if(user->getAuthLevel() < authLevel) return AUTH_STATE_SECURITY_LEVEL_TO_LOW;

	return AUTH_STATE_OK;
}

/**
 * @brief reset all users and commit to database.
 *
 */
void Authenticator::reset(void){
	ESP_LOGD(TAG, "delete all users!");

	// add user
	m_userList.erase(m_userList.begin(), m_userList.end());
	addUser(FRONTEND_USER_NAME, System::getDeviceID(), AUTH_LEVEL_USER);

	// add admin
	std::string devID = System::getDeviceID();
	std::reverse(devID.begin(), devID.end());
	addUser(FRONTEND_ADMIN_NAME, devID, AUTH_LEVEL_ADMIN);

	commit();
}

/**
 * @brief convert a state to an inform string
 *
 * @param state current state
 * @return std::string inform string
 */
std::string Authenticator::stateToString(auth_state_t state){
	switch(state){
		case AUTH_STATE_OK: return "OK";
		case AUTH_STATE_NOT_ALLOWED: return "access denied!";
		case AUTH_STATE_USER_NOT_FOUND: return "user not found!";
		case AUTH_STATE_INVALIDED_PASSWORD: return "invalid password!";
		case AUTH_STATE_INVALIDED_TOKEN: return "invalid api token!";
		case AUTH_STATE_SECURITY_LEVEL_TO_LOW: return "your security level is to low!";
		case AUTH_STATE_USER_SILL_EXISTS: return "username still exists!";
		default: return "Unknown state.";
	}
}

Account::Account(std::string username, std::string password, auth_level_t authLevel):
	m_username(username),
	m_password(password),
	m_authLevel(authLevel),
	m_token(""){

}

Account::~Account(){

}

void Account::generateToken(){
	m_token = System::getRandomString();
}
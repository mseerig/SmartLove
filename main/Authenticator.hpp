/**
 * @file Authenticator.hpp
 * @author Marcel Seerig (marcel.seerig@ed-chemnitz.de)
 * @brief Handles the api access tokens for the JSON_RPC client.
 * @version 0.1
 * @date 2019-03-18
 *
 * @copyright Copyright (c) 2019
 *
 */

#include "ConfigurationManager.hpp"
#include <string>
#include <vector>

#ifndef AUTHENTICATOR_HPP
#define AUTHENTICATOR_HPP

#define AUTHENTICATOR_MAX_NUMBER_OF_USERS 5

typedef enum {
    AUTH_STATE_OK = 0x0,
    AUTH_STATE_NOT_ALLOWED,
	AUTH_STATE_USER_NOT_FOUND,
	AUTH_STATE_INVALIDED_PASSWORD,
	AUTH_STATE_INVALIDED_TOKEN,
	AUTH_STATE_SECURITY_LEVEL_TO_LOW,
	AUTH_STATE_USER_SILL_EXISTS,
	AUTH_STATE_MAX,
} auth_state_t;

typedef enum {
    AUTH_LEVEL_ANY = 0x0,
	AUTH_LEVEL_USER = 0x7,
    AUTH_LEVEL_ADMIN = 0xF,
} auth_level_t;

class Account{
	public:
		Account(std::string username, std::string password, auth_level_t authLevel);
		~Account();
		std::string 	getUsername(void){return m_username;}
		std::string 	getPassword(void){return m_password;}
		auth_level_t 	getAuthLevel(void){return m_authLevel;}
		void 			setUsername(std::string username){m_username = username;}
		void 			setPassword(std::string password){m_password = password;}
		std::string 	getToken(void){return m_token;}
		void 			generateToken();
		void			deleteToken(){m_token = "";}

		std::string 	m_username;
		std::string 	m_password;
		auth_level_t	m_authLevel;
		std::string 	m_token;
};

class Authenticator{
	public:
		Authenticator(ConfigurationManager &configurationManager);
		~Authenticator();

		void			reset(void);
		void 			commit(void);

		auth_state_t 	login(std::string username, std::string password, std::string* token);
		auth_state_t 	check(std::string username, std::string token, auth_level_t authLevel);
		auth_state_t	logout(std::string username);

		Account*		getUser(std::string username);
		auth_state_t	addUser(std::string username, std::string password, auth_level_t authLevel);
		auth_state_t 	deleteUser(std::string username);
		auth_state_t	changeUsername(std::string username, std::string newUsername);
		auth_state_t	changePassword(std::string username, std::string newPassword);
		auth_state_t 	changeUsernameAndPassword(std::string username, std::string newPassword, std::string newUsername);

		static std::string 	stateToString(auth_state_t state);

	private:

		void readVault(void);
		ConfigurationManager&	m_configurationManager;
		std::vector<Account>	m_userList;
		bool					m_initDone{false};
};

#endif
#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT 21

/**
 * @brief Some FTP server return codes as an enum
 * 
 * Taken from: https://en.wikipedia.org/wiki/List_of_FTP_server_return_codes
 * 
 */
typedef enum {
	// 200 Series (Success)
	READY_NEW_USER = 220,
	USER_LOGGED_IN = 230,
	USER_LOGGED_OUT = 231,
	ENTERING_PASSIVE_MODE = 227,
	
	// 300 Series (Command accepted, on hold)
	USER_OK = 331,
	NEED_ACCOUNT = 332,

	// 400 Series (Command not accepted)
	INVALID_USER_OR_PASS = 430,

	// 500 Series (Syntax error)
	SYNTAX_ERR = 501,
	BAD_SEQUENCE = 503,
	NOT_LOGGED_IN = 530,
	FILE_UNAVAILABLE = 550
} server_response_code_t;

/**
 * @brief Program config structure
 * 
 * Holds the data required to log in to an FTP server,
 * adhering to the following example:
 * ftp://[<user>:<password>@]<host>/<url-path>
 */
typedef struct config {
	char* user; // Username
	char* pass; // Password
	char* host; // Host
	char* url_path; // URL path
} config_t;

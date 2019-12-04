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
#include <ctype.h>

#define FTP_PORT 21  // FTP server port
#define RES_SIZE 3   // Response code size

/**
 * @brief Some FTP server return codes as an enum
 *
 * Taken from: https://en.wikipedia.org/wiki/List_of_FTP_server_return_codes
 *
 */
typedef enum server_response_code {
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
 * Holds data required to access an FTP server.
 * The details stored in this struct will be used for
 * the following connection string:
 * ftp://[user[:password]@]host/url-path
 */
typedef struct config {
  char* user;      // Username
  char* password;  // Password
  char* host;      // Host
  char* url_path;  // URL path
} config_t;

/**
 * @brief Set the config struct
 *
 * @param config Config struct to be modified
 * @param argv Command line arguments
 * @return int Zero on success, negative on error
 */
int set_config(config_t* config, char** argv);

/**
 * @brief Get an FTP response as status code integer
 *
 * Note: Reads all bytes until EOL.
 * Use only when you are expecting a response.
 *
 * @param socketfd FTP socket file descriptor
 * @return int Bytes parsed as integer, negative on error
 */
int get_response(int socketfd);

/**
 * @brief Send an FTP command
 * 
 * @param socketfd The control connection socket
 * @param command The FTP command
 * @param arg Command argument. If none, use NULL
 * @return int Zero on success, negative on error
 */
int send_command(int socketfd, const char* command, const char* arg);

/**
 * @brief Send the USER command followed by the PASS command
 * 
 * @param config 
 * @param socketfd 
 * @return int 
 */
int send_credentials(const config_t* config, int socketfd);

/**
 * @brief Run program
 * 
 * Calls all needed functions in the right order.
 * 
 * @param config Program configuration
 * @return int Zero on success, negative on error
 */
int run(const config_t* config);

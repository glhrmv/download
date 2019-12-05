#pragma once

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>

#define FTP_PORT 21        // FTP server port
#define FTP_CODE_SIZE 3    // Response code size
#define FTP_RES_SIZE 1024  // FTP response buffer size

/**
 * @brief Some FTP server return codes as an enum
 *
 * Taken from: https://en.wikipedia.org/wiki/List_of_FTP_server_return_codes
 *
 */
typedef enum server_response_code {
  // 100 Series (Positive preliminary)
  FILE_STATUS_OK = 150,

  // 200 Series (Positive completion)
  READY_NEW_USER = 220,
  USER_LOGGED_IN = 230,
  USER_LOGGED_OUT = 231,
  ENTERING_PASSIVE_MODE = 227,

  // 300 Series (Positive intermediate)
  USER_OK = 331,
  NEED_ACCOUNT = 332,

  // 400 Series (Transient negative completion)
  INVALID_USER_OR_PASS = 430,

  // 500 Series (Permanent negative completion)
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
 * @param config Program configuration to be modified
 * @param argv Command line arguments
 * @return int Zero on success, negative on error
 */
int set_config(config_t* config, char** argv);

/**
 * @brief Establish connection to a given IP address on a given port
 * 
 * @param socketfd Socket file descriptor
 * @param ip_addr IP address
 * @param port Port number
 * @return int Zero on success, negative on error
 */
int establish_connection(int socketfd, const char* ip_addr, int port);

/**
 * @brief Get an FTP response as status code integer
 *
 * Note: Reads all bytes until EOL.
 * Use only when you are expecting a response.
 *
 * @param socketfd FTP socket file descriptor
 * @return int FTP status code on success, negative on error
 */
int get_response(int socketfd);

/**
 * @brief Get an FTP response, place response buffer into buf
 *
 * Like get_response
 *
 * @param socketfd
 * @param buf
 * @return int FTP status code on success, negative on error
 */
int get_response_w_buf(int socketfd, char* buf);

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
 * @brief Enter passive mode, get file port
 * 
 * @param config Program configuration
 * @param socketfd The control connection socket
 * @return int File port to open data connection in on success, negative on error
 */
int send_pasv(const config_t* config, int socketfd);

/**
 * @brief Helper function used by send_pasv
 * 
 * Parses the 227 response string, which is 
 * similar to the following example:
 * > 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
 
 * This function will return the calculation of
 * p1 * 256 + p2, the data port to connect to.
 * 
 * 
 * @param pasv_res Passive mode response string
 * @return int File port to open data connection in on success, negative on error
 */
int parse_pasv_port(char* pasv_res);

/**
 * @brief Send retrieve command
 * 
 * @param config Program configuration
 * @param socketfd The control connection socket
 * @return int Zero on success, negative on error
 */
int send_retr(const config_t* config, int socketfd);

/**
 * @brief Download file from server on port
 * 
 * The file is stored on $HOME/Downloads/file,
 * Where file is the basepath of the config's url-path
 * 
 * @param config Program configuration
 * @param socketfd The data connection socket
 * @return int Zero on success, negative on error
 */
int download_file(const config_t* config, int socketfd);

/**
 * @brief Run program
 *
 * Calls all needed functions in the right order.
 *
 * @param config Program configuration
 * @return int Zero on success, negative on error
 */
int run(const config_t* config);

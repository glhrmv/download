#include "download.h"

int main(int argc, char** argv) {
  if (argc != 5) {
    printf("Usage: %s <user> <password> <host> <url-path>\n", argv[0]);
    return -1;
  }

  config_t config;
  if (set_config(&config, argv) < 0) return -1;

  return run(&config);
}

int set_config(config_t* config, char** argv) {
  config->user = argv[1];
  config->password = argv[2];
  config->host = argv[3];
  config->url_path = argv[4];
  return 0;
}

int establish_connection(int socketfd, const char* ip_addr, int port) {
  // Server address handling
  struct sockaddr_in s_addr;
  bzero((char*)&s_addr, sizeof(s_addr));
  s_addr.sin_family = AF_INET;
  // 32-bit Internet address network byte ordered
  s_addr.sin_addr.s_addr = inet_addr(ip_addr);
  // Server TCP port must be network byte ordered
  s_addr.sin_port = htons(port);

  if (connect(socketfd, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0) {
    perror("connect");
    return -1;
  }

  return 0;
}

int get_response(int socketfd) {
  char res[FTP_CODE_SIZE];
  int state = 0, i = 0;
  uint8_t c;

  while (state != 3) {
    read(socketfd, &c, 1);
    // Uncomment below line to see entire response
    printf("%c", c);
    switch (state) {
      // Wait for 3 digit number followed by ' ' or '-'
      case 0:
        if (isdigit(c)) {
          res[i] = c;
          i++;
        }

        if (c == ' ') {
          if (i != 3) return -1;

          i = 0;
          state = 1;
        }

        if (c == '-') {
          state = 2;
          i = 0;
        }
        break;
      // Read until EOL
      case 1:
        if (c == '\n') {
          state = 3;
        }
        break;
      // Waits for response code in multiple line responses
      case 2:
        if (c == res[i]) {
          i++;
        } else if (i == 3) {
          if (c == ' ')
            state = 1;
          else if (c == '-')
            i = 0;
        }
        break;
    }
  }

  fflush(stdout);
  return atoi(res);
}

int get_response_w_buf(int socketfd, char* buf) {
  char res[FTP_CODE_SIZE];
  memset(buf, 0, FTP_RES_SIZE);
  int state = 0, i = 0, buf_i = 0;
  uint8_t c;
  while (state != 3) {
    read(socketfd, &c, 1);
    buf[buf_i++] = c;
    // Uncomment below line to see entire response
    printf("%c", c);
    switch (state) {
      // Wait for 3 digit number followed by ' ' or '-'
      case 0:
        if (isdigit(c)) {
          res[i] = c;
          i++;
        }

        if (c == ' ') {
          if (i != 3) return -1;

          i = 0;
          state = 1;
        }

        if (c == '-') {
          state = 2;
          i = 0;
        }
        break;
      // Read until EOL
      case 1:
        if (c == '\n') {
          state = 3;
        }
        break;
      // Waits for response code in multiple line responses
      case 2:
        if (c == res[i]) {
          i++;
        } else if (i == 3) {
          if (c == ' ')
            state = 1;
          else if (c == '-')
            i = 0;
        }
        break;
    }
  }

  buf[buf_i - 1] = 0;
  fflush(stdout);
  return atoi(res);
}

int send_command(int socketfd, const char* command, const char* arg) {
  if (!write(socketfd, command, strlen(command))) return -1;

  if (arg != NULL)
    if (!write(socketfd, arg, strlen(arg))) return -1;

  if (!write(socketfd, "\n", 1)) return -1;

  return 0;
}

int send_credentials(const config_t* config, int socketfd) {
  // Send user
  printf("< user %s\n", config->user);
  if (send_command(socketfd, "user ", config->user) < 0) {
    fprintf(stderr, "Error sending user\n");
    return -1;
  }

  if (get_response(socketfd) != USER_OK) {
    fprintf(stderr, "Did not get USER OK (331)\n");
    return -1;
  }
  printf("Got USER OK\n");

  // Send password
  printf("< pass %s\n", config->password);
  if (send_command(socketfd, "pass ", config->password) < 0) {
    fprintf(stderr, "Error sending password\n");
    return -1;
  }

  if (get_response(socketfd) != USER_LOGGED_IN) {
    fprintf(stderr, "Did not get USER OK (230)\n");
    return -1;
  }
  printf("Got USER LOGGED IN\n");

  return 0;
}

int send_pasv(const config_t* config, int socketfd) {
  // Send passive
  printf("< pasv %s\n", config->user);
  if (send_command(socketfd, "pasv ", config->user) < 0) {
    fprintf(stderr, "Error sending user\n");
  }

  char pasv_res[FTP_RES_SIZE];
  if (get_response_w_buf(socketfd, pasv_res) != ENTERING_PASSIVE_MODE) {
    fprintf(stderr, "Did not get ENTERING PASSIVE MOE (227)\n");
    return -1;
  }

  int port = parse_pasv_port(pasv_res);
  if (port < 0) {
    fprintf(stderr, "Error calculating port number\n");
    return -1;
  }

  return port;
}

int send_retr(const config_t* config, int socketfd) {
  // Send passive
  printf("< retr %s\n", config->url_path);
  if (send_command(socketfd, "retr ", config->url_path) < 0) {
    fprintf(stderr, "Error sending retr\n");
    return -1;
  }

  if (get_response(socketfd) != FILE_STATUS_OK) {
    fprintf(stderr, "Did not get FILE STATUS OK (150)\n");
    return -1;
  }

  return 0;
}

int parse_pasv_port(char* pasv_res) {
  const char* input = pasv_res;
  const char* regex_string = "([0-9]{1,3}),([0-9]{1,3})\\)\\.";
  size_t max_groups = 3;
  regex_t compiled_regex;
  regmatch_t group_arr[max_groups];

  if (regcomp(&compiled_regex, regex_string, REG_EXTENDED))
    return -1;

  char src_cpy[strlen(input) + 1];
  if (regexec(&compiled_regex, input, max_groups, group_arr, 0) == 0) {
    for (unsigned int i = 0; i < max_groups; i++) {
      if ((size_t) group_arr[i].rm_so == (size_t)-1) break;  // No more groups

      memset(src_cpy, 0, strlen(input) + 1);
      strcpy(src_cpy, input);
      src_cpy[group_arr[i].rm_eo] = 0;
    }
  }

  int port_no = atoi(src_cpy + group_arr[1].rm_so) * 256 + atoi(src_cpy + group_arr[2].rm_so);

  regfree(&compiled_regex);

  return port_no;
}

int download_file(const config_t* config, int socketfd) {
  char filepath[1024];
  sprintf(filepath, "%s/Downloads/%s", getenv("HOME"), basename(config->url_path));
  FILE* file = fopen(filepath, "w+");
  if (file == NULL) return -1;

  printf("Downloading to %s ...\n", filepath);
  
  char buffer;
  while (read(socketfd, &buffer, 1))
    fwrite(&buffer, sizeof(buffer), 1, file);

  if (fclose(file)) return -1;

  return 0;
}

int run(const config_t* config) {
  struct hostent* h;
  if ((h = gethostbyname(config->host)) == NULL) {
    herror("gethostbyname");
    return -1;
  }

  char* ip_addr = inet_ntoa(*((struct in_addr*)h->h_addr));

  // Open control connection socket
  int control_socketfd;
  if ((control_socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return -1;
  }

  // Connect to server (control)
  printf("Connecting to %s:%d ... ", ip_addr, FTP_PORT);
  if (establish_connection(control_socketfd, ip_addr, FTP_PORT) < 0) {
    fprintf(stderr, "Error connecting to server\n");
    return -1;
  }
  printf("connected.\n");

  if (get_response(control_socketfd) != READY_NEW_USER) {
    fprintf(stderr, "Did not get READY NEW USER (220)\n");
    return -1;
  }

  // Send credentials
  if (send_credentials(config, control_socketfd) < 0) {
    fprintf(stderr, "Error sending credentials\n");
    return -1;
  }

  // Enter passive mode, retrieve file port
  int fileport = send_pasv(config, control_socketfd);
  if (fileport < 0) {
    fprintf(stderr, "Error entering passive mode\n");
    return -1;
  }

  // Open data connection socket
  int data_socketfd;
  if ((data_socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return -1;
  }

  // Connect to server (data)
  printf("Connecting to %s:%d ... ", ip_addr, fileport);
  if (establish_connection(data_socketfd, ip_addr, fileport) < 0) {
    fprintf(stderr, "Error connecting to server\n");
    return -1;
  }
  printf("connected.\n");

  // Send retrieve command
  if (send_retr(config, control_socketfd) < 0) {
    fprintf(stderr, "Error sending retrieve\n");
    return -1;
  }

  // Download file
  if (download_file(config, data_socketfd) < 0) {
    fprintf(stderr, "Error downloading file\n");
    return -1;
  }

  printf("File downloaded successfully.\n");

  close(data_socketfd);
  close(control_socketfd);

  return 0;
}

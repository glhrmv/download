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

int get_response(int socketfd) {
  char res[RES_SIZE];
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
  if (send_command(socketfd, "USER ", config->user) < 0) {
    fprintf(stderr, "Error sending user\n");
  }

  if (get_response(socketfd) != USER_OK) {
    fprintf(stderr, "Did not get USER OK (331)\n");
    return -1;
  }
  printf("Got USER OK\n");

  // Send password
  printf("< pass %s\n", config->password);
  if (send_command(socketfd, "PASS ", config->password) < 0) {
    fprintf(stderr, "Error sending password\n");
  }

  if (get_response(socketfd) != USER_LOGGED_IN) {
    fprintf(stderr, "Did not get USER OK (230)\n");
    return -1;
  }
  printf("Got USER LOGGED IN\n");

  return 0;
}

int run(const config_t* config) {
  struct hostent* h;
  if ((h = gethostbyname(config->host)) == NULL) {
    herror("gethostbyname");
    return -1;
  }

  char* ip_addr = inet_ntoa(*((struct in_addr*)h->h_addr));

  // Server address handling
  struct sockaddr_in s_addr;
  bzero((char*)&s_addr, sizeof(s_addr));
  s_addr.sin_family = AF_INET;
  // 32-bit Internet address network byte ordered
  s_addr.sin_addr.s_addr = inet_addr(ip_addr);
  // Server TCP port must be network byte ordered
  s_addr.sin_port = htons(FTP_PORT);

  // Open a TCP socket for the control connection
  int control_sockfd;
  if ((control_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error creating socket");
    return -1;
  }

  // Connect to server
  printf("Connecting to %s ... ", ip_addr);
  if (connect(control_sockfd, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0) {
    perror("Error connecting to server");
    return -1;
  }
  printf("connected.\n");

  if (get_response(control_sockfd) != READY_NEW_USER) {
    fprintf(stderr, "Did not get READY NEW USER (220)\n");
    return -1;
  }
  printf("Got READY NEW USER\n");

  if (send_credentials(config, control_sockfd) < 0) {
    fprintf(stderr, "Error sending credentials\n");
    return -1;
  }

  // TODO: Enter passive mode

  // TODO: Open file socket

  // TODO: Send retrieve command

  // TODO: Download file

  close(control_sockfd);

  return 0;
}

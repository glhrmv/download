#include "download.h"

int main(int argc, char** argv) {
  if (argc != 5) {
    printf("Usage: %s <user> <password> <host> <url-path>\n", argv[0]);
    return 1;
  }

  config_t config;
  if (set_config(&config, argv) != 0) return 1;

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
  char res[RES_SIZE] = {0};
  for (int i = 0; i < RES_SIZE; i++) {
    uint8_t b;
    if (!read(socketfd, &b, 1)) return 1;

    if (b < '0' || b > '9') continue;
    res[i] = b;
  }

  if (!res[0]) return -1;

  return atoi(res);
}

int run(const config_t* config) {
  struct hostent* h;
  if ((h = gethostbyname(config->host)) == NULL) {
    herror("gethostbyname");
    return 1;
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

  // Open a TCP socket
  int socketfd;
  if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error creating socket");
    return 1;
  }

  // Connect to server
  printf("Connecting to %s ... ", ip_addr);
  if (connect(socketfd, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0) {
    perror("Error connecting to server");
    return 1;
  }
  printf("connected.\n");

  // Get server response
  printf("Got response: %d\n", get_response(socketfd));
  printf("Got response: %d\n", get_response(socketfd));

  return 0;
}

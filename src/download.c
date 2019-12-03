#include "download.h"

int main(int argc, char** argv) {
  if (argc != 5) {
    printf("Usage: %s <user> <password> <host> <url-path>\n", argv[0]);
    return 1;
  }

  config_t config;
  if (set_config(&config, argv) < 0) exit(1);

  return run(&config);
}

int set_config(config_t* config, char** argv) {
  config->user = argv[1];
  config->pass = argv[2];
  config->host = argv[3];
  config->url_path = argv[4];
  return 0;
}

int run(const config_t* config) {
  char url[64];
  sprintf(url, "ftp://%s:%s@%s/%s", config->user, config->pass, config->host,
          config->url_path);
  printf("connecting to: %s\n", url);

  struct hostent* h;
  if ((h = gethostbyname(config->host)) == NULL) {
    herror("gethostbyname");
    exit(1);
  }

  printf("IP address: %s\n", inet_ntoa(*((struct in_addr*)h->h_addr)));

  int sockfd;
  struct sockaddr_in server_addr;

  exit(0);
}

#include "ftpclient.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char const *argv[]) {
  ftp_client_info info;
  memset(&info, 0, sizeof(info));

  char *url = "ftp://anonymous:pass@ftp.up.pt/pub/kodi/timestamp.txt";

  int url_err = parse_URL(&info, url);

  if (url_err) {
    log_url_err(url_err);
    return 1;
  }

  int ctrl_socket_fd = connect_to_host(&info);

  if (ctrl_socket_fd == -1) {
    printf("error: couldn't connect to host FTP control port\n");
  }

  return 0;
}

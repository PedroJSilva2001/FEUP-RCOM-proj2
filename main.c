#include "ftpclient.h"

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

int main(int argc, char const *argv[]) {
  ftp_client_info info;
  strcpy(info.host, "netlab1.fe.up.pt");
  strcpy(info.service, NULL);

  struct addrinfo *addrinfos = host_IPaddrinfos(info.host, info.service);

  if (addrinfos == NULL) {
    printf("error: couldn't find host IP address\n");
    return 1;
  }

  info.host_addrinfos = addrinfos;

  int cmd_sock_fd = connect_to_host(&info, FTP_TCP_CTRL_PORT);

  if (cmd_sock_fd < 0) {
    printf("error: couldn't connect to host ftp control port");
    freeaddrinfo(addrinfos);
    return 1;
  }

  in_port_t data_port = host_data_port();

  int data_sock_fd = connect_to_host(&info, data_port);

  if (cmd_sock_fd < 0) {
    printf("error: couldn't connect to host ftp data port");
    freeaddrinfo(addrinfos);
    return 1;
  }

  close(cmd_sock_fd);

  freeaddrinfo(addrinfos);
  
  return 0;
}

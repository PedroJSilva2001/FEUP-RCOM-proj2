#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#define FTP_TCP_CTRL_PORT 21

typedef struct {
  char user[99999];
  char password[99999];
  char host[99999];
  char service[999];
  struct addrinfo *host_addrinfos;
} ftp_client_info;

struct addrinfo *host_IPaddrinfos(char *host, char *service);

int connect_to_host(ftp_client_info *info, in_port_t port);

in_port_t host_data_port();

#endif
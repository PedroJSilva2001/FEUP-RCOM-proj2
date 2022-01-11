#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>

#define FTP_CTRL_PORT "21"
#define MAX_CODES 100

typedef struct {
  char *user;
  char *pass;
  char *host;
  char *path;
} ftp_client_info;

struct addrinfo *host_IPaddrinfos(char *host, char *port);

int connect_to_host(ftp_client_info *info);

char *host_data_port(char *socket_addr);

int parse_URL(ftp_client_info *info, const char *url);

int enter_passive_mode(int ctrl_socket_fd);

int read_code(FILE *stream);

int send_command(int socket_fd, char* message);

int save_file(int socket_fd, char* filename);

#endif
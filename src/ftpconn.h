#ifndef FTP_CONNECTION_H
#define FTP_CONNECTION_H

#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <regex.h>

typedef struct {
  char *user;
  char *pass;
  char *host;
  char *path;
} ftp_client_info;

struct addrinfo *host_IPaddrinfos(char *host, char *port);

int connect_to_host(ftp_client_info *info);

char *get_client_param(regmatch_t capt_group, const char *url);

int parse_URL(ftp_client_info *info, const char *url);

int connect_to_host_data_port(unsigned char *ip, unsigned char *port);

#endif
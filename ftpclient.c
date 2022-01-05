#include "ftpclient.h"

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <ctype.h>

FILE *stream;

int open_connect_socket(struct addrinfo *addr) {
  int socket_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

  if (socket_fd < 0) {
    fprintf(stderr, "socket: %s\n", strerror(errno));
    return -1;
  }

  if (connect(socket_fd, addr->ai_addr, (socklen_t) addr->ai_addrlen) < 0) {
    fprintf(stderr, "connect: %s\n", strerror(errno));
    close(socket_fd);
  }

  return socket_fd;  
}

struct addrinfo *host_IPaddrinfos(char *host, char *port) {
  struct addrinfo hints;
  struct addrinfo *infos;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family |= AF_INET; //IPv4
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags |= AI_CANONNAME;

  int err = getaddrinfo(host, port, &hints, &infos);

  if (err != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return NULL;
  }

  return infos;
}

int connect_to_host(ftp_client_info *info) {
  int socket_fd = -1;
  struct addrinfo *host_addrinfos = host_IPaddrinfos(info->host, FTP_CTRL_PORT);

  if (host_addrinfos == NULL) {
    fprintf(stderr, "error: couldn't find host IP socket address\n");
    return -1;
  }

  for (struct addrinfo *p = host_addrinfos; p != NULL; p = p->ai_next) {
    socket_fd = open_connect_socket(p);
    if (socket_fd != -1) {
      break;
    }
  }

  //don't forget to free host_addrinfos
  // freeaddrinfo(host_addrinfos);
  return socket_fd;
}

char *host_data_port(char *socket_addr) {  //PASSIVE
  //TODO: parse socket_addr, get port MSB AND LSB,
  // and return (MSB << 8u | LSB) as string
  return NULL;
}


int set_client_param(regmatch_t capt_group, char *url, char *buf, int bufsize) {
  int start = capt_group.rm_so;
  int end = capt_group.rm_eo;

  if (end-start > bufsize) {
    return 1;
  }

  memcpy(buf, &url[start], end-start);

  return 0;
}

int parse_URL(ftp_client_info *info, char *url) {
  regex_t regex;
  regmatch_t capt_groups[7];
  // this regex accepts invalid paths and domain names but we don't care because we catch them later
  const char *pattern = "ftp://(((.*):(.*)@)?([^/]+)/)(.+)";

  #define USER_CAPT_GROUP 3
  #define PASS_CAPT_GROUP 4
  #define HOST_CAPT_GROUP 5
  #define PATH_CAPT_GROUP 6
  
  #define MISSING_GROUP(capts, group) (capts[group].rm_eo == capts[group].rm_so)

  regcomp(&regex, pattern, REG_EXTENDED);

  int res = regexec(&regex, url, 7, capt_groups, 0);

  regfree(&regex);

  if (res != 0) {
    return INVALID_URL_FORMAT;
  }

  // the regex checks if host and path are missing
  if (set_client_param(capt_groups[HOST_CAPT_GROUP], 
                          url, info->host, MAX_HOSTNAME_SIZE)) {
    return HOSTNAME_TOO_BIG;
  }

  if (set_client_param(capt_groups[PATH_CAPT_GROUP],
                          url, info->path, MAX_FILEPATH_SIZE)) {
    return FILEPATH_TOO_BIG;
  }

  if (!MISSING_GROUP(capt_groups, USER_CAPT_GROUP)) {
    info->user_specified = 1;

    if (set_client_param(capt_groups[USER_CAPT_GROUP],
                          url, info->user, MAX_USER_SIZE)) {
      return USER_TOO_BIG;
    }
  }

  if (!MISSING_GROUP(capt_groups, PASS_CAPT_GROUP)) {
    info->pass_specified = 1;
    
    if (set_client_param(capt_groups[PASS_CAPT_GROUP],
                          url, info->pass, MAX_PASS_SIZE)) {
      return PASS_TOO_BIG;
    }
  }

  return 0;
}
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
#include <fcntl.h>

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
    return -1;
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


char *get_client_param(regmatch_t capt_group, const char *url) {
  int start = capt_group.rm_so;
  int end = capt_group.rm_eo;
  int size = end-start;

  if (size == 0) {
    return NULL;
  }

  char *param = malloc(sizeof(char)*size+1);

  memcpy(param, &url[start], size);

  param[size] = '\0';
  
  return param;
}

int parse_URL(ftp_client_info *info, const char *url) {
  regex_t regex;
  regmatch_t capt_groups[6];
  // this regex accepts invalid paths and domain names but we don't care because we catch them later
  const char *pattern = "ftp://((.*):(.*)@)?([^/]+)/(.+)";

  #define USER_CAPT_GROUP 2
  #define PASS_CAPT_GROUP 3
  #define HOST_CAPT_GROUP 4
  #define PATH_CAPT_GROUP 5
  
  regcomp(&regex, pattern, REG_EXTENDED);

  int res = regexec(&regex, url, 6, capt_groups, 0);

  regfree(&regex);

  if (res != 0) {
    return 1;
  }

  info->user = get_client_param(capt_groups[USER_CAPT_GROUP], url);
  info->pass = get_client_param(capt_groups[PASS_CAPT_GROUP], url);
  info->host = get_client_param(capt_groups[HOST_CAPT_GROUP], url);
  info->path = get_client_param(capt_groups[PATH_CAPT_GROUP], url);

  return 0;
}


int send_command(int socket_fd, char* message) {
    int bytes = write(socket_fd, message, strlen(message));
    if (bytes <= 0) {
      perror("write()");
      return -1;
    }
    return 0;
}

int save_file(int socket_fd, char* filename) {
  int file_fd = open(filename, O_WRONLY | O_CREAT, 0777);

  if (file_fd < 0)
    return -1;

  int bytes;
  char buf[1];

  while ((bytes = read(socket_fd, buf, sizeof(buf))) > 0) {
      if (write(file_fd, buf, bytes) < 0) {
        return -1;
      }
  }

  close(file_fd);
  return 0;
}
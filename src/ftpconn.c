#include "ftpconn.h"

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define FTP_CTRL_PORT "21"

int open_connect_socket(struct addrinfo *addr) {
  int socket_fd = socket(addr->ai_family, SOCK_STREAM, IPPROTO_TCP);

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

  for (struct addrinfo *a = host_addrinfos; a != NULL; a = a->ai_next) {
    socket_fd = open_connect_socket(a);

    if (socket_fd != -1) { break; }
  }

  freeaddrinfo(host_addrinfos);
  return socket_fd;
}

char *get_client_param(regmatch_t capt_group, const char *url) {
  int start = capt_group.rm_so;
  int end = capt_group.rm_eo;
  int size = end - start;

  if (size == 0) { return NULL; }

  char *param = malloc(sizeof(char) * size + 1);

  memcpy(param, &url[start], size);

  param[size] = '\0';
  
  return param;
}

int parse_URL(ftp_client_info *info, const char *url) {
  regex_t regex;
  regmatch_t capt_groups[6];
  // This regex accepts invalid paths and domain names, we will catch them later
  const char *pattern = "^ftp://((.*):(.*)@)?([^/]+)/(.+)";

  #define USER_CAPT_GROUP 2
  #define PASS_CAPT_GROUP 3
  #define HOST_CAPT_GROUP 4
  #define PATH_CAPT_GROUP 5
  
  regcomp(&regex, pattern, REG_EXTENDED);

  int res = regexec(&regex, url, 6, capt_groups, 0);

  regfree(&regex);

  if (res != 0) { return 1; }

  info->user = get_client_param(capt_groups[USER_CAPT_GROUP], url);
  info->pass = get_client_param(capt_groups[PASS_CAPT_GROUP], url);
  info->host = get_client_param(capt_groups[HOST_CAPT_GROUP], url);
  info->path = get_client_param(capt_groups[PATH_CAPT_GROUP], url);

  return 0;
}

int connect_to_host_data_port(unsigned char *ip, unsigned char *port) {
  struct sockaddr_in dataconn_addr;
  memset(&dataconn_addr, 0, sizeof(struct sockaddr_in));

  char ip_[16];
  sprintf(ip_, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  dataconn_addr.sin_family = AF_INET;
  inet_aton(ip_, &dataconn_addr.sin_addr);
  dataconn_addr.sin_port = htons(port[0] << 8u | port[1]);

  // If the server has load balancing this is rendered pointless as we can 
  // just use host_IPaddrinfos(host, port) to find the socket address
  // for the data connection because it would be hosted with the same name. 
  // Load balancing is not obligatory in the FTP standard so we need to create
  // the socket address struct to make it work for everyone. 
  struct addrinfo dataconn_addrinfo;

  dataconn_addrinfo.ai_family = AF_INET;
  dataconn_addrinfo.ai_addrlen = sizeof(dataconn_addr);
  dataconn_addrinfo.ai_addr = (struct sockaddr *) &dataconn_addr;

  int data_socket_fd = open_connect_socket(&dataconn_addrinfo);
  
  return data_socket_fd;
}
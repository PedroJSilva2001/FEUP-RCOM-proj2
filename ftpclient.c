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

struct addrinfo *host_IPaddrinfos(char *host, char *service) {
  struct addrinfo hints;
  struct addrinfo *infos;

  memset(&hints, 0, sizeof(hints));
  struct sockaddr_in6 ad;
  hints.ai_family |=  AF_UNSPEC; //IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags |= AI_CANONNAME;

  int err = getaddrinfo(host, service, &hints, &infos);

  if (err != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return NULL;
  }

  return infos;
}

int connect_to_host(ftp_client_info *info, in_port_t port) {
  int sock_fd;
  in_port_t net_ord_port = htons(port);

  struct addrinfo i = info->host_addrinfos[0]; // choose IP first

  sock_fd = socket(i.ai_family, SOCK_STREAM, IPPROTO_TCP);

  if (sock_fd < 0) {
    fprintf(stderr, "socket: %s\n", strerror(errno));
    return -1;
  }

  void *host_sockaddr;
  unsigned long size;

  if (i.ai_family == AF_INET) {
    ((struct sockaddr_in*) i.ai_addr)->sin_port = net_ord_port;
    host_sockaddr = &((struct sockaddr_in *) i.ai_addr)->sin_addr;
    size = sizeof(struct sockaddr_in);   
  } else {
    ((struct sockaddr_in6*) i.ai_addr)->sin6_port = net_ord_port;
    host_sockaddr = &((struct sockaddr_in6 *) i.ai_addr)->sin6_addr;
    size = sizeof(struct sockaddr_in6);   
  }

  if (connect(sock_fd, (struct sockaddr *) host_sockaddr, size) < 0) {
    fprintf(stderr, "connect: %s\n", strerror(errno));
    close(sock_fd);
    return -1;
  }

  return sock_fd;
}

in_port_t host_data_port(char *port) {  //PASSIVE
  char *bytes = strtok(port, '.');
  uint8_t MSB = bytes[0];
  uint8_t LSB = bytes[1];
  return MSB << 8u | LSB;
}

int parse_URL(ftp_client_info *info, char *url) {
  regex_t regex;
  regmatch_t capt_groups[5];
  const char *pattern = 
  "(?<protocol>ftp:\/\/)(?:(?<user>\w+):(?<password>\w+)@)?(?<host>(?:(?:\w+\.)+\w+)\/?)(?<url_path>(?:(?:[\w-]+\/)*)(?:(?:\w+\.)*\w+)\/?)?";

  #define PROTO_CAPT_GROUP 0
  #define USER_CAPT_GROUP  1
  #define PASS_CAPT_GROUP  2
  #define HOST_CAPT_GROUP  3
  #define PATH_CAPT_GROUP  4

  regcomp(&regex, pattern, REG_EXTENDED);
  /*
  (?<protocol>ftp:\/\/)(?:(?<user>\w+):(?<password>\w+)@)?(?<host>(?:(?:\w+\.)+\w+)\/?)(?<url_path>(?:(?:[\w-]+\/)*)(?:(?:\w+\.)*\w+)\/?)?
  
  */
}
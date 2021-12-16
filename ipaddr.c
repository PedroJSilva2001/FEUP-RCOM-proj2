#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

void dump_host_addrinfos(struct addrinfo *infos) {
  char addrstr[INET6_ADDRSTRLEN];
  void *sockaddr;
  struct addrinfo *i;

  for (i = infos; i != NULL; i = i->ai_next) {
    inet_ntop(i->ai_family, i->ai_addr->sa_data, addrstr, sizeof(addrstr));

    switch(i->ai_family) {
      
      case AF_INET:
        sockaddr = &((struct sockaddr_in *) i->ai_addr)->sin_addr;
      break;

      case AF_INET6:
        sockaddr = &((struct sockaddr_in6 *) i->ai_addr)->sin6_addr;
      break;
    }

    inet_ntop (i->ai_family, sockaddr, addrstr, INET6_ADDRSTRLEN);

    printf("IPv%d address: %s ; socket type: %i\n", i->ai_family == AF_INET6? 6 : 4,
              addrstr, i->ai_socktype);
  }
}


struct addrinfo *host_addrinfos(char *host_name, char *service) {
  struct addrinfo hints;
  struct addrinfo *infos, *info;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family |=  AF_UNSPEC; //IPv4 or IPv6
  hints.ai_socktype = 0;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  int err = getaddrinfo(host_name, service, &hints, &infos);

  if (err != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return NULL;
  }
  
  return infos;
}

int main(int argc, char *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <address to get IP address>\n", argv[0]);
    exit(-1);
  }
  //host_addrinfos("ftp://rcom:rcom@netlab1.fe.up.pt/pub.txt", "443");
  
  struct addrinfo * infos = host_addrinfos("www.google.com", "443");
  dump_host_addrinfos(infos);
  freeaddrinfo(infos);
  
  return 0;
}


/*
  for (i = infos; i != NULL; i = i->ai_next) {
  
    switch(info->ai_addr->sa_family) {
      
      case AF_INET:
        struct sockaddr_in *addr = (struct sockaddr_in *)info->ai_addr;
        printf("IP address: %s\n", inet_ntop(AF_INET, &addr->sin_addr, 
              ip_str, sizeof(ip_str)));
      break;

      case AF_INET6:
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)info->ai_addr;
        printf("IP address: %s\n", inet_ntop(AF_INET6, &addr6->sin6_addr, 
              ip_str, sizeof(ip_str)));
      break;
    }
  }

*/
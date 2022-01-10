#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <netdb.h>
#include <netinet/in.h>

#include <stdio.h>

#define FTP_CTRL_PORT "21"
#define MAX_CODES 100
#define MAX_HOSTNAME_SIZE 253
#define MAX_FILEPATH_SIZE 7000
#define MAX_USER_SIZE 1000
#define MAX_PASS_SIZE 1000


#define INVALID_URL_FORMAT 1
#define HOSTNAME_TOO_BIG   2
#define FILEPATH_TOO_BIG   3
#define USER_TOO_BIG       4
#define PASS_TOO_BIG       5

static const char *url_err_table[6] = {
                            NULL,
                            "The URL given as input is invalid\nIt must be in the format ftp://[<user>:<pass>@]<hostname>/<path>\n",
                            "Hostname is too big\n",
                            "Filepath is too big\n",
                            "Username is too big\n",
                            "Password is too big\n"   
                         };

typedef struct {
  char user[MAX_USER_SIZE];
  char pass[MAX_PASS_SIZE];
  char host[MAX_HOSTNAME_SIZE];
  char path[MAX_FILEPATH_SIZE];
  int user_specified;
  int pass_specified;
} ftp_client_info;

struct addrinfo *host_IPaddrinfos(char *host, char *port);

int connect_to_host(ftp_client_info *info);

char *host_data_port(char *socket_addr);

int parse_URL(ftp_client_info *info, char *url);

int enter_passive_mode(int ctrl_socket_fd);

int read_code(FILE *stream);

int send(int socket_fd, char* message);

int save_file(int socket_fd, char* filename);

#endif
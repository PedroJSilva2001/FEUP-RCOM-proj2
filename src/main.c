#include "ftpcom.h"
#include "ftpconn.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static ftp_client_info info;
static int ctrl_socket_fd;
static int data_socket_fd;

void close_ctrl_socket() {
  printf("ctrl\n");
  close(ctrl_socket_fd);
}

void close_data_socket() {
    printf("data\n");
  close(data_socket_fd);
}

void free_ftp_client_info() {
    printf("info\n");
  free(info.user);
  free(info.pass);
  free(info.host);
  free(info.path);
}

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    printf("error: invalid number of parameters\nUse case is: download <URL>\n");
    return 1;
  }

  int url_err = parse_URL(&info, argv[1]);

  atexit(free_ftp_client_info);

  if (url_err) {
    printf("error: the URL is incorrect\nIt should follow the format: ftp://[<user>:<pass>@]<host>/<path>");
    return 1;
  }

  ctrl_socket_fd = connect_to_host(&info);

  atexit(close_ctrl_socket);

  if (ctrl_socket_fd == -1) {
    printf("error: couldn't connect to host FTP control port\n");
    return 1;
  }

  if (check_connection_establishment(ctrl_socket_fd)) {
    printf("Connection to host has failed\n");
    return 1;
  }

  printf("Connection established to host successfully.\n");

  if (login(ctrl_socket_fd, &info)) {
    printf("error: couldn't log in to server\n");
    return 1;
  }

  printf("Logged in successfully.\n");

  unsigned char ip[4];
  unsigned char port[2];

  if (enter_passive_mode(ctrl_socket_fd, ip, port)) {
    printf("error: couldn't enter passive mode\n");
    return 1;
  }

  data_socket_fd = connect_to_host_data_port(ip, port);

  atexit(close_data_socket);

  if (data_socket_fd == -1) {
    printf("error: couldn't connect to host data port\n");
    return 1;
  }

  printf("Entered passive mode.\n");

  printf("Retrieving file...\n");

  if (retrieve_file(ctrl_socket_fd, data_socket_fd, &info)) {
    printf("error: couldn't retrieve file successfully\n");
    return 1;
  }

  printf("File saved successfully.\n");

  disconnect(ctrl_socket_fd);

  return 0;
}

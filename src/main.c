#include "ftpcom.h"
#include "ftpconn.h"

#include <stdio.h>

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    printf("error: invalid number of parameters\nUse case is: download <URL>\n");
    return 1;
  }

  ftp_client_info info;

  int url_err = parse_URL(&info, argv[1]);

  if (url_err) {
    printf("error: the URL is incorrect\nIt should follow the format: ftp://[<user>:<pass>@]<host>/<path>");
    return 1;
  }

  int ctrl_socket_fd = connect_to_host(&info);

  if (ctrl_socket_fd == -1) {
    printf("error: couldn't connect to host FTP control port\n");
    return 1;
  }

  printf("Connection established.\n");

  if (login(ctrl_socket_fd, &info) == -1) {
    printf("error: couldn't login\n");
    return 1;
  }

  printf("Logged in.\n");

  if (enter_passive_mode(ctrl_socket_fd) == -1) {
    printf("error: couldn't enter passive mode\n");
    return 1;
  }

  printf("Entered passive mode.\n");

  if (retrieve(ctrl_socket_fd, &info) == -1) {
    printf("error: couldn't retrieve\n");
    return 1;
  }

  printf("File retrived.\n");

  if (save_file(ctrl_socket_fd, "oi") == -1) {  // TODO: COMPLETE?
    printf("error: couldn't save file\n");
    return 1;
  }

  printf("File saved successfully.\n");

  if (disconnect(ctrl_socket_fd) == -1) {
    printf("error: couldn't disconnect\n");
    return 1;
  }

  printf("Disconnected successfully.\n");

  return 0;
}

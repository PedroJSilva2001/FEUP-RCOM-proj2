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

  return 0;
}

#include "ftpcom.h"
#include "ftpconn.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#define REP_IMAGE "I"
#define REP_ASCII "A"

static ftp_client_info info;
static int ctrl_socket_fd;
static int data_socket_fd;
static char *url = NULL;
static char *rep_type = REP_ASCII;


void close_ctrl_socket() {
  close(ctrl_socket_fd);
}

void close_data_socket() {
  close(data_socket_fd);
}

void free_ftp_client_info() {
  free(info.user);
  free(info.pass);
  free(info.host);
  free(info.path);
}

void print_usage(const char *argv[]) {
  printf("Usage:\n    %s [-I | --image] <url>\n", argv[0]);
  printf("    %s  -h | --help\n", argv[0]);
  printf("Options:\n");
  printf("    -h, --help     Show this help message and exit\n");
  printf("    -I, --image    Set transfer representation type to image/binary [default: ASCII type]\n");
}

int parse_args(int argc, const char *argv[]) {
  int help_flag = 0;
  int opt = 0;

  struct option longopts[] = {
        { "help", no_argument, &help_flag, 1 },
        { "image", no_argument, 0, 'I' },
        { 0, 0, 0, 0 }
  };

  opterr = 0;

  while((opt = getopt_long(argc, argv, "hI", longopts, NULL)) != -1) {
    switch (opt) {
      case 'h':
        help_flag = 1;
        break;

      case 'I':
        rep_type = REP_IMAGE;
        break;

      case '?':
        fprintf(stderr, "Unknown/Invalid option provided\n\n");
        return 1;
    }
  }

  if (help_flag) {
    if (argc == 2) {
      print_usage(argv);
      exit(0);
    } else {
      fprintf(stderr, "Invalid number of arguments\n\n");
      return 1;
    }
  } else {
    if (optind != argc-1) {
      fprintf(stderr, "Invalid number of arguments\n\n");
      return 1;
    } else {
      url = argv[optind];
    }
  }
  
  return 0;
}


int main(int argc, const char *argv[]) {
  if (parse_args(argc, argv)) {
    print_usage(argv);
    return 1;
  }

  int url_err = parse_URL(&info, url);

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
    printf("error: connection to host has failed\n");
    return 1;
  }

  printf("Connection established to host successfully.\n");

  if (login(ctrl_socket_fd, &info)) {
    printf("error: couldn't log in to server\n");
    return 1;
  }

  printf("Logged in successfully.\n");

  const char *rep = strcmp(rep_type, "A") == 0 ? "ASCII" : "Image";

  if (set_representation_type(ctrl_socket_fd, rep_type)) {
    printf("error: couldn't set representation type to %s type\n", rep);
    return 1;
  }

  printf("Representation type changed to %s type\n", rep);

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

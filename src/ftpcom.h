#ifndef FTPCOM_H
#define FTPCOM_H

#include <stdio.h>

#define CODE_SIZE 3
#define BASE_REPLY_LEN 10
#define SERVER_DISC 1
#define READ_ERROR 2
#define IMPL_ERROR 3

typedef struct {
  char *buf;
  int real_size;
  int strlen;
  char *code[CODE_SIZE+1];
} ftp_reply;

typedef enum {
  START,
  AWAITING_LINE_START,
  AWAITING_SEPARATOR,
  SINGLE_LINE,
  MULTI_LINE,
  REPLY_COMPLETE,
  AWAITING_NEWLINE
} ftp_reply_state;

int login(ftp_client_info);  // USER PASS

int enter_passive_mode(int ctrl_socket_fd);  // send PASV\r\n (without space in the middle)

int download(); // send RETR PATH\r\n

int send_command(int socket_fd, char* message);

int save_file(int socket_fd, char* filename);

ftp_reply create_reply_line();

void concat_to_reply(ftp_reply *reply, char *str, int n);

int read_reply(int ctrl_socket_fd, ftp_reply *reply);

#endif
#ifndef FTPCOM_H
#define FTPCOM_H

#include "ftpconn.h"

#include <stdio.h>

#define CODE_SIZE 3

typedef struct {
  char *text;
  unsigned int real_len;
  unsigned int text_len;
  char code[CODE_SIZE + 1];
} ftp_reply;

typedef enum {
  START,
  AWAITING_LINE_START,
  REPLY_COMPLETE,
  AWAITING_NEWLINE,
  AWAITING_END_NEWLINE,
} ftp_reply_state;

int login(int ctrl_socket_fd, ftp_client_info *info);

int enter_passive_mode(int ctrl_socket_fd, unsigned char *ip, unsigned char *port);

int retrieve_file(int ctrl_socket_fd, int data_socket_fd, ftp_client_info *info);

int send_command(int ctrl_socket_fd, char* command);

int send_command_fmt(int ctrl_socket_fd, const char *format, int format_len, char *param);

int save_file(int data_socket_fd, char* filename);

void create_reply(ftp_reply *reply);

void concat_to_reply(ftp_reply *reply, char *str, int n);

void free_reply(ftp_reply *reply);

int read_reply(int ctrl_socket_fd, ftp_reply *reply);

int assert_valid_code(char *code, char **valid_codes, int n);

void dump_and_free_reply(ftp_reply *reply);

int check_connection_establishment(int ctrl_socket_fd);

void disconnect(int ctrl_socket_fd);

#endif
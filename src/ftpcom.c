#include "ftpcom.h"
#include "ftpconn.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>

int login(int socket_fd, ftp_client_info *info) {
  // TODO
  char command_user[MAX_SIZE];
  sprintf(command_user, "user %s\r\n", info->user);

  if (send_command(socket_fd, command_user)) return -1;

  // TODO: RESPONSE READ

  char command_pass[MAX_SIZE]; 
  sprintf(command_pass, "pass %s\r\n", info->pass);

  if (send_command(socket_fd, command_pass)) return -1;

  // TODO: RESPONSE READ

  return 0;
}

int enter_passive_mode(int socket_fd) {
  // TODO
  if (send_command(socket_fd, "pasv\r\n")) return 1;

  // TODO: RESPONSE READ -> resulta num buf

  // PORT h1,h2,h3,h4,p1,p2
  int ip[4], ports[2];

  //if (scanf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).", &ip[0], &ip[1], &ip[2], &ip[3], &ports[0], &ports[1]) < 0) return -1;

  int port = ports[0] * 256 + ports[1];

  // TODO: CONNECT SOCKET
  return 0;
}

int retrieve(int socket_fd, ftp_client_info *info) {
  char command[MAX_SIZE];
  sprintf(command, "retr %s\r\n", info->path);

  if (send_command(socket_fd, command)) return -1;

  // TODO: RESPONSE READ

  return 0;
}

int send_command(int socket_fd, char* message) {
    int bytes = write(socket_fd, message, strlen(message));
    if (bytes <= 0) {
      perror("write()");
      return -1;
    }
    return 0;
}

int save_file(int socket_fd, char* filename) {
  int file_fd = open(filename, O_WRONLY | O_CREAT, 0777);

  if (file_fd < 0)
    return -1;

  int bytes;
  char buf[MAX_SIZE];

  while ((bytes = read(socket_fd, buf, sizeof(buf))) > 0) {
      if (write(file_fd, buf, bytes) < 0) {
        return -1;
      }
  }

  close(file_fd);
  return 0;
}

ftp_reply *create_reply() {
  ftp_reply *reply = malloc(sizeof(ftp_reply));
  
  reply->buf = malloc(sizeof(char)*BASE_REPLY_LEN+1);
  memset(reply->buf, 0, BASE_REPLY_LEN+1);
  memset(reply->code, 0, CODE_SIZE+1);
  reply->real_size = BASE_REPLY_LEN+1;
  reply->strlen = 0;

  return reply;
}

void concat_to_reply(ftp_reply *reply, char *str, int n) {
  if (reply->strlen+n >= reply->real_size) {  // >=
    reply->buf = realloc(reply->buf, 2*(reply->strlen+n));
    reply->real_size = 2*(reply->strlen+n);
  }

  strncat(reply->buf, str, n);
  reply->strlen += n;
}

int nl_1st_occurence_offset(char *buf, int n) {
  for (int i = 0; i < n; i++) {
    if (buf[i] == '\n') {
      return i;
    }
  }

  return -1;
}

int read_reply(int ctrl_socket_fd, ftp_reply **reply) {
  ftp_reply *rep = create_reply();
  ftp_reply_state reply_state = START;

  char lstart[LINE_START_SIZE+1];
  memset(lstart, 0, LINE_START_SIZE+1);
  int start_count = 0;

  char buf[BASE_REPLY_LEN];

  while (reply_state != REPLY_COMPLETE) {
    memset(buf, 0, BASE_REPLY_LEN);

    int bytes = recv(ctrl_socket_fd, buf, BASE_REPLY_LEN, 0);

    if (bytes == 0) { return SERVER_DISC; }

    if (bytes == -1) { return READ_ERROR; }

    for (ssize_t i = 0; i < bytes; i++) {
      if (reply_state == START) {
        if (start_count == CODE_SIZE) {

          lstart[start_count] = buf[i];

          // return error if we find more bytes after last newline 
          // (violation of ftp standard)
          if (!VALID_LINE_START(lstart)) { return IMPL_ERROR; }

          reply_state = LINE_SEPARATOR(lstart) == ' '?
                            AWAITING_END_NEWLINE : AWAITING_NEWLINE;
          start_count = 0;
          memcpy(rep->code, lstart, CODE_SIZE);
        } else {
          lstart[start_count++] = buf[i];
        }

        concat_to_reply(rep, &buf[i], 1);
      }

      else if (reply_state == AWAITING_LINE_START) {
        if (start_count == CODE_SIZE) {
          lstart[start_count] = buf[i];
          if (VALID_LINE_START(lstart) && LINE_SEPARATOR(lstart) == ' '
                && (strncmp(rep->code, lstart, CODE_SIZE) == 0)) {
            reply_state = AWAITING_END_NEWLINE;
          } else {
            reply_state = AWAITING_NEWLINE;
            start_count = 0;
          }
        } else {
          lstart[start_count++] = buf[i];
        }

        concat_to_reply(rep, &buf[i], 1);
      }

      else if (reply_state == AWAITING_NEWLINE) {
        int off = nl_1st_occurence_offset(&buf[i], bytes-i);

        // add all chars because we didnt find newline
        if (off == -1) {
          concat_to_reply(rep, &buf[i], bytes-i);
          break;
        }

        // add chars up to newline
        concat_to_reply(rep, &buf[i], off+1);
        reply_state = AWAITING_LINE_START;
        i += off;
      }

      else if (reply_state == AWAITING_END_NEWLINE) {
        int off = nl_1st_occurence_offset(&buf[i], bytes-i);

        if (off != -1 && i+off < bytes-1) { return IMPL_ERROR; }

        // add chars up to newline or end of buf (no newline was found)
        concat_to_reply(rep, &buf[i], bytes-i);

        if (off == -1) {
          continue;
        } else {
          reply_state = REPLY_COMPLETE;
          break;
        }
      }
    }
  }

  *reply = rep;
  printf("end = %s", rep->buf);
  return 0;
}
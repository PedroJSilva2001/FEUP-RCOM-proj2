#ifndef FTPCOM_H
#define FTPCOM_H

#include <stdio.h>

int enter_passive_mode(int ctrl_socket_fd);

int send_command(int socket_fd, char* message);

int save_file(int socket_fd, char* filename);

#endif
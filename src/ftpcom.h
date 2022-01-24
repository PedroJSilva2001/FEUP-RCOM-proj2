#ifndef FTPCOM_H
#define FTPCOM_H

#include "ftpconn.h"

#include <stdio.h>

#define CODE_SIZE 3

/**
 * Reply.
 **/
typedef struct {
  char *text;
  unsigned int real_len;
  unsigned int text_len;
  char code[CODE_SIZE + 1];
} ftp_reply;

/**
 * State machine to process replies.
 **/
typedef enum {
  START,
  AWAITING_LINE_START,
  REPLY_COMPLETE,
  AWAITING_NEWLINE,
  AWAITING_END_NEWLINE,
} ftp_reply_state;

/** @brief Logs in an user (or anonymous) in server.
 *  @param ctrl_socket_fd File descriptor of the socket.
 *  @param info The ftp_client_info struct.
 *  @return Returns 0 upon success, error codes otherwise.
 */
int login(int ctrl_socket_fd, ftp_client_info *info);

/** @brief Sets the connection to passive mode.
 *  @param ctrl_socket_fd File descriptor of the socket.
 *  @param ip Ip.
 *  @param port Port.
 *  @return Returns 0 upon success, error codes otherwise.
 */
int enter_passive_mode(int ctrl_socket_fd, unsigned char *ip, unsigned char *port);

/** @brief Retrieves a file from the connection.
 *  @param ctrl_socket_fd File descriptor of the control socket.
 *  @param data_socket_fd File descriptor of the data socket.
 *  @param info The ftp_client_info struct.
 *  @return Returns 0 upon success, error codes otherwise.
 */
int retrieve_file(int ctrl_socket_fd, int data_socket_fd, ftp_client_info *info);

/** @brief Send a command to socket.
 *  @param ctrl_socket_fd File descriptor of the socket.
 *  @param command Command to send to file descriptor.
 *  @return Returns 0 upon success, 1 otherwise.
 */
int send_command(int ctrl_socket_fd, char* command);

/** @brief Send a command to socket with a string format.
 *  @param ctrl_socket_fd File descriptor of the socket.
 *  @param format Format of the command to send.
 *  @param format_len Lemgth of the format.
 *  @param param Parameters to fill the format.
 *  @return Returns 0 upon success, 1 otherwise.
 */
int send_command_fmt(int ctrl_socket_fd, const char *format, int format_len, char *param);

/** @brief Saves the file retrieved.
 *  @param data_socket_fd File descriptor of the socket.
 *  @param filename Name of the file.
 *  @return Returns 0 upon success, 1 otherwise.
 */
int save_file(int data_socket_fd, char* filename);

/** @brief Creates a reply.
 *  @param reply The ftp_reply struct.
 */
void create_reply(ftp_reply *reply);

/** @brief Appends string to reply.
 *  @param reply The ftp_reply struct.
 *  @param str String to add to reply.
 *  @param n Length of string (str).
 */
void concat_to_reply(ftp_reply *reply, char *str, int n);

/** @brief Frees ftp_reply struct field 'text'.
 *  @param reply The ftp_reply struct.
 */
void free_reply(ftp_reply *reply);

/** @brief Reads a reply from the socket.
 *  @param ctrl_socket_fd File descriptor of the socket.
 *  @param reply The ftp_reply struct.
 *  @return Returns 0 upon success, error codes otherwise.
 */
int read_reply(int ctrl_socket_fd, ftp_reply *reply);

/** @brief Compares a code and checks if is valid.
 *  @param code Code to compare.
 *  @param valid_codes Valid codes.
 *  @param n Number of valid codes.
 *  @return Returns 0 upon success, 1 otherwise.
 */
int assert_valid_code(char *code, char **valid_codes, int n);

/** @brief Dumps and frees reply.
 *  @param reply The ftp_reply struct.
 */
void dump_and_free_reply(ftp_reply *reply);

/** @brief Checks if connection to soket was successfully estabilished.
 *  @param ctrl_socket_fd File descriptor of the socket.
 *  @return Returns 0 upon success, 1 otherwise.
 */
int check_connection_establishment(int ctrl_socket_fd);

/** @brief Disconnects the socket.
 *  @param ctrl_socket_fd File descriptor of the socket.
 */
void disconnect(int ctrl_socket_fd);

/** @brief Changes mode of transfer (ASCII or image).
 *  @param ctrl_socket_fd File descriptor of the socket.
 *  @param type Type of transfer.
 *  @return Returns 0 upon success, 1 otherwise.
 */
int set_representation_type(int ctrl_socket_fd, const char *type);

#endif
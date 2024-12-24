/**
 * @file server.h
 * @brief Header file for the HTTP server module.
 * @author Phillip - Sassmann
 * @date 24.12.2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <time.h>

void parse_arg(int argc, char *argv[], char **port, char **docroot, char **filename, int *portInt);

/**
 * @brief Handles received signals to set a quit flag for terminating processes.
 *
 * @details Modifies the global `quit` flag to true, allowing processes to exit gracefully.
 *
 * @param signal The signal received (e.g., `SIGINT`, `SIGTERM`).
 */
void handle_signal(int signal);

/**
 * @brief Prints an error message and program usage information, then exits.
 *
 * @details Displays an error message indicating incorrect usage or arguments, then
 * terminates the program.
 *
 * @param message Description of the encountered error.
 */
static void usage(char *message);


/**
 * @brief Handles a client request.
 * @details Reads the request from the client, parses it, and sends an appropriate HTTP response.
 * 
 * @param client_sock File descriptor for the connected client socket.
 * @param doc_root Path to the document root directory.
 * @param index_file Default file to serve when the root URL is requested.
 */
void handle_client(int client_sock, const char *doc_root, const char *index_file);

/**
 * @brief Sends an HTTP response to the client.
 * @details Constructs an HTTP response based on the status code and requested file.
 * 
 * @param stream File stream for the client connection.
 * @param status_code HTTP status code to send (e.g., 200, 404).
 * @param fileWrite Relative path to the requested file.
 * @param doc_root Path to the document root directory.
 */
void send_response(FILE *stream, int status_code, char *fileWrite, const char *doc_root);
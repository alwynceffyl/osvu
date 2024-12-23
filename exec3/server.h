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

void handle_client(int client_sock, const char *doc_root, const char *index_file);
void send_response(FILE *stream, int status_code, char *fileWrite, const char *doc_root);
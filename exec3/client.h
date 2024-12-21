#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdbool.h>

static void usage(char* errormsg);
void parse_url(char *url, char **filename, char **host) ;
void parse_arg(int argc, char *argv[], char **port, char **filename, char **url, char **directory) ;
int is_empty_or_spaces(const char *str);
int parse_header(char *str);
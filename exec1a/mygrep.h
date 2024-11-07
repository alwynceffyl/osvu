#ifndef MYGREP_H
#define MYGREP_H
/**
 * @file mygrep.h
 * @brief Header file for the `mygrep` program.
 *
 * This file contains function declarations and necessary includes for the `mygrep` program,
 * which searches for a specified keyword in lines of text from input files or stdin.
 * It supports case-insensitive searching with the `-i` option and output redirection with `-o`.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Prints usage information and exits the program.
 *
 * This function outputs usage instructions and an error message to stderr, then
 * terminates the program with EXIT_FAILURE.
 *
 * @param myprog The name of the program (usually argv[0]).
 * @param errormsg The specific error message to display.
 */
void usage(char *myprog, char* errormsg);

/**
 * @brief Reads lines from the input file and searches for the keyword.
 *
 * This function reads each line from the provided input file and checks if it contains
 * the keyword. Matching lines are written to the output file.
 *
 * @param input Input file pointer (either stdin or an opened file).
 * @param output Output file pointer (either stdout or an opened file).
 * @param caseInsensitive If true, the search is case-insensitive.
 * @param keyword The keyword to search for in each line.
 */
void readLine(FILE * input, FILE* output, bool caseInsensitive ,char* keyword);

/**
 * @brief Converts a string to uppercase.
 *
 * Allocates a new string where each character in the original string is converted
 * to uppercase. The caller is responsible for freeing the allocated memory.
 *
 * @param str The original string.
 * @return A newly allocated uppercase version of the string.
 */
char* toUpperCase(const char *str);

#endif // MYGREP_H

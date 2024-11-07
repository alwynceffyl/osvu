/**
 * @file mygrep.c
 * @author Phillip Sassmann
 * @date 4.11.2024
 *
 * @brief Main program module for a keyword-based search program.
 *
 * This program reads input from files or stdin and searches each line for a specific keyword.
 * If a line contains the keyword, it is printed to stdout or to an output file if specified.
 * When the `-i` option is included, the search becomes case-insensitive.
 */

#include "mygrep.h"  


/**
 * @brief Program entry point.
 *
 * This function processes command-line arguments to configure the search behavior,
 * such as case sensitivity and output file location. It then either reads from stdin
 * or opens specified input files, searching each line for the keyword.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Returns EXIT_SUCCESS on successful execution.
 */
int main(int argc,char *argv[]){
    char *myprog=argv[0];
    FILE* input=stdin;
    FILE* output=NULL;

    int opt;
    bool caseInsensitive=false;          
    char* keyword;

    while((opt=getopt(argc, argv, "io:"))!=-1){
        switch(opt){
            case 'i':
                    if(caseInsensitive) usage(myprog, "only one -i can be declared");
                	caseInsensitive=true;
                    break;
            case 'o':
                if(output != NULL) usage(myprog, "only one outputfile can be declared");
                    output = fopen(optarg, "w");
                    if(output == NULL) {
                        usage(myprog, "not able to open the outputfile.");
                    }
                    break;
            default: /* ? */
                usage(myprog,"invalid argument");
        }
    }
    if(optind==argc){
        usage(myprog,"no keyword provided");
    }
    output = output ==NULL ?  stdout : output;
    keyword=argv[optind];
    optind++;

    if(argc==optind) readLine(input, output, caseInsensitive, keyword);
    else{
        for(; optind<argc; optind++){
            input=fopen(argv[optind], "r");
            if(input == NULL) {
                usage(myprog,"unable to open one of the inputfiles.");
            }
            readLine(input, output, caseInsensitive, keyword);
            fclose(input);
        }
    }
    fclose(output);

    exit(EXIT_SUCCESS);
}

/**
 * @brief Prints usage information and exits.
 *
 * This function writes an error message and usage instructions to stderr,
 * then terminates the program with EXIT_FAILURE.
 *
 * @param myprog The name of the program (argv[0]).
 * @param errormsg The specific error message to be displayed.
 */
void usage(char* myprog, char* errormsg) {
    fprintf(stderr, "Usage: %s -i -o [outputfile] keyword [file ...]\nError: %s\n", myprog, errormsg);
    exit(EXIT_FAILURE);
}


/**
 * @brief Reads lines from input and searches for the keyword.
 *
 * This function reads each line from the given input file and checks if it contains
 * the specified keyword. If the line matches, it is written to the output file.
 *
 * @param input Input file pointer (stdin or an opened file).
 * @param output Output file pointer (stdout or an opened file).
 * @param caseInsensitive Boolean flag for case-insensitive search.
 * @param keyword The keyword to search for in each line.
 */
void readLine(FILE * input, FILE* output, bool caseInsensitive,char* keyword){
    char *line =NULL;
    size_t len=0;
    ssize_t nread;

    while((nread=getline(&line, &len, input))!=-1){

        if(strstr(line, keyword)!=NULL || (caseInsensitive &&  strstr(toUpperCase(line),toUpperCase(keyword))!=NULL ))
        {
            fwrite(line, nread, 1, output);
        }
    }
    free(line);
}


/**
 * @brief Converts a string to uppercase.
 *
 * Allocates a new string where each character in the original string is converted to uppercase.
 *
 * @param str The original string.
 * @return Returns a newly allocated uppercase version of the string.
 */
char* toUpperCase(const char *str) {

    char *upperStr = malloc(strlen(str) + 1); 
    if(upperStr==NULL){
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    char *ptr=upperStr; 

    while(*str!='\0'){
        *ptr++=tolower((unsigned char)*str++);
    }
    *ptr = '\0'; 
    return upperStr;
}




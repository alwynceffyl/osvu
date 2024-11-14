#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>


typedef struct mydata{
    unsigned int counter;
    char ** lines;
} mydata_t;

char** readInput(FILE *dataInput,unsigned int *count);
void merge(FILE* output, char** arr1, char** arr2, unsigned int counter1, unsigned int counter2);
void printArr(char **arr, unsigned int count);

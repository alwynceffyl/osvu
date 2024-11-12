
#include "forksort.h"


char *myprog;


int main(int argc, char *argv[]){
    
    myprog=argv[0];

    unsigned int count;
    char **line = readInput(&count);

   
    if(count==1){
        fprintf(stdout, "only one line btw \n");
        fprintf(stdout, "%s", line[0]);
        freeMemory(line,count);
        exit(EXIT_SUCCESS);
    }

    for(int i=0; i< count; i++){
        fprintf(stdout, "%s", line[i]);
    }
    
    freeMemory(line, count);
    exit(EXIT_SUCCESS);
}


char** readInput(unsigned int *count){
    FILE *input = stdin;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    unsigned int start=2;
    char** arr= (char**)malloc(start*sizeof(char*));
    if(arr==NULL){
        perror("failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    unsigned int counter=0;

    while ((nread = getline(&line, &len, input)) != -1) {
        if(counter==start){
            start *=2;
            arr =(char**) realloc(arr,  start* sizeof(char*));
            if (arr == NULL) {
                freeMemory(arr, counter);
                exit(EXIT_FAILURE);
            }
        }

        arr[counter] = (char *)malloc((nread+1) * sizeof(char));
        if(arr[counter]==NULL){
            perror("failed to allocate memory");
            freeMemory(arr, counter);
            exit(EXIT_FAILURE);
        }
        strncpy(arr[counter],line, nread);
        arr[counter][nread]='\0';
        counter++;
    }
    free(line);

    arr = (char**)realloc(arr, counter * sizeof(char*));
    if (arr == NULL) {
        perror("Final reallocation failed");
        freeMemory(arr, counter);
        exit(EXIT_FAILURE);
    }
   
    *count=counter;

    return arr;
}

void freeMemory(char** arr, unsigned count) {
    for (unsigned i = 0; i < count; i++) {
        free(arr[i]);
    }
    free(arr);
}

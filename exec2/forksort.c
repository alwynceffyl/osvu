
#include "forksort.h"



/**
 * @file forksort.c
 * @author Phillip Sassmann
 * @date 13.11.2024
 *
 * @brief This file contains functions for reading input, sorting them via forksort
 */


char *myprog;


int main(int argc, char **argv){
    myprog=argv[0];
    
    mydata_t data;
    data.lines = readInput(stdin, &data.counter);
    if(data.counter==1){
        fprintf(stdout, "%s\n", data.lines[0]);
        free(data.lines); 
        exit(EXIT_SUCCESS);
    }    


    // SPLIT DATA 
    mydata_t dataLeft, dataRight;
    dataLeft.counter=data.counter/2;
    dataRight.counter=data.counter -  data.counter/2;
    dataLeft.lines=(char **) malloc(sizeof(char*)  * (dataLeft.counter));
    dataRight.lines=(char **) malloc(sizeof(char*) * (dataRight.counter));

    if (dataLeft.lines == NULL || dataRight.lines == NULL) {
        perror("memory allocation failed for splitting data");
        free(data.lines);
        exit(EXIT_FAILURE);
    }
    
    for(int i=0; i<dataLeft.counter;i++){
        dataLeft.lines[i]=data.lines[i];
    }

    for(int i=0; i<dataRight.counter;i++){
        dataRight.lines[i]=data.lines[data.counter/2+i];
    }

    

    int pipeLeftIn[2], pipeLeftOut[2];
    int pipeRightIn[2], pipeRightOut[2];
    if (pipe(pipeLeftIn) == -1 || pipe(pipeLeftOut) == -1 ||
        pipe(pipeRightIn) == -1 || pipe(pipeRightOut) == -1) {
        perror("error creating pipes");
        free(dataLeft.lines);
        free(dataRight.lines);
        free(data.lines);
        exit(EXIT_FAILURE);
    }

    pid_t leftChild=fork();
    if(leftChild==-1){
        fprintf(stderr, "error in forking left child");
        free(dataLeft.lines);
        free(dataRight.lines);
        free(data.lines);
        exit(EXIT_FAILURE);
    }

    if(leftChild==0){
        dup2(pipeLeftIn[0], STDIN_FILENO);          // Redirect stdin to pipe
        dup2(pipeLeftOut[1], STDOUT_FILENO);        // Redirect stdout to pipe
        close(pipeLeftIn[1]);
        close(pipeLeftOut[0]);
        close(pipeRightIn[0]);
        close(pipeRightIn[1]);
        close(pipeRightOut[0]);
        close(pipeRightOut[1]);

        // Execute forksort recursively
        execlp(argv[0], argv[0], NULL);

        // If execlp fails
        perror("execlp failed");
        free(dataLeft.lines);
        free(dataRight.lines);
        free(data.lines);
        exit(EXIT_FAILURE);
    }
    
    pid_t rightChild=fork();
    if(rightChild==-1){
        fprintf(stderr, "error in forking right child");
        free(dataLeft.lines);
        free(dataRight.lines);
        free(data.lines);
        exit(EXIT_FAILURE);
    }

    if(rightChild==0){
        dup2(pipeRightIn[0], STDIN_FILENO);         // Redirect stdin to pipe
        dup2(pipeRightOut[1], STDOUT_FILENO);       // Redirect stdout to pipe
        close(pipeRightIn[1]);
        close(pipeRightOut[0]);
        close(pipeLeftIn[0]);
        close(pipeLeftIn[1]);
        close(pipeLeftOut[0]);
        close(pipeLeftOut[1]);

        // Execute forksort recursively
        execlp(argv[0], argv[0], NULL);

        // If execlp fails
        perror("execlp failed");
        free(dataLeft.lines);
        free(dataRight.lines);
        free(data.lines);
        exit(EXIT_FAILURE);
    }


    close(pipeLeftIn[0]);
    close(pipeLeftOut[1]);
    close(pipeRightIn[0]);
    close(pipeRightOut[1]);


    FILE *leftIn = fdopen(pipeLeftIn[1], "w");
    if (leftIn == NULL) {
        perror("Error opening pipe for writing (left)");
        free(dataLeft.lines);
        free(dataRight.lines);
        free(data.lines);
        exit(EXIT_FAILURE);
    }
    for(int i=0; i< dataLeft.counter;i++){
        fprintf(leftIn, "%s\n", dataLeft.lines[i]);
    }
    fclose(leftIn);

    FILE *rightIn = fdopen(pipeRightIn[1], "w");
    if (rightIn == NULL) {
        perror("Error opening pipe for writing (right)");
        free(dataLeft.lines);
        free(dataRight.lines);
        free(data.lines);
        exit(EXIT_FAILURE);
    }
    for(int i=0; i< dataRight.counter;i++){
        fprintf(rightIn, "%s\n", dataRight.lines[i]);
    }
    fclose(rightIn);

    mydata_t sortedLeft, sortedRight;
    FILE *leftOut = fdopen(pipeLeftOut[0], "r");
    if (leftOut == NULL) {
        perror("Error opening pipe for reading (left)");
        free(dataLeft.lines);
        free(dataRight.lines);
        free(data.lines);
        exit(EXIT_FAILURE);
    }
    sortedLeft.lines= readInput(leftOut, &sortedLeft.counter);
    fclose(leftOut);

    FILE *rightOut = fdopen(pipeRightOut[0], "r");
    if (rightOut == NULL) {
        perror("Error opening pipe for reading (right)");
        free(sortedLeft.lines);
        free(sortedRight.lines);
        free(dataLeft.lines);
        free(data.lines);
        exit(EXIT_FAILURE);
    }
    sortedRight.lines= readInput(rightOut, &sortedRight.counter);
    fclose(rightOut);

    // Wait for both children to finish
    int status;
    waitpid(leftChild, &status, 0);
    waitpid(rightChild, &status, 0);

    // Merge the sorted results and print them
    merge(stdout, sortedLeft.lines, sortedRight.lines, sortedLeft.counter, sortedRight.counter);
    
    close(pipeRightOut[0]);
    close(pipeLeftOut[0]);
    close(pipeLeftIn[1]);
    close(pipeRightIn[1]);
    free(sortedLeft.lines);
    free(sortedRight.lines);
    free(dataLeft.lines);
    free(dataRight.lines);
    free(data.lines);
    exit(EXIT_SUCCESS);
}


char** readInput(FILE *dataInput,unsigned int *count){
    FILE *input =dataInput ;
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
                perror("failed to allocate memory");
                free(arr);
                exit(EXIT_FAILURE);
            }
        }

        arr[counter] = (char *)malloc((nread) * sizeof(char));
        if(arr[counter]==NULL){
            perror("failed to allocate memory");
            free(arr);
            exit(EXIT_FAILURE);
        }
        strncpy(arr[counter],line, nread);
        arr[counter][nread-1]='\0';
        counter++;
    }
    free(line);

    arr = (char**)realloc(arr, counter * sizeof(char*));
    if (arr == NULL) {
        perror("final reallocation failed");
        free(arr);
        exit(EXIT_FAILURE);
    }

    *count=counter;
    return arr;
}



void merge(FILE* output, char** arr1, char** arr2, unsigned int counter1, unsigned int counter2) {

    int i=0, j=0;

    while (i < counter1 && j < counter2) {
        if (strcmp(arr1[i], arr2[j])<0) {
            fprintf(output, "%s\n", arr1[i]);
            i++;
        }
        else {
            fprintf(output, "%s\n", arr2[j]);
            j++;
        }
    }

    // Copy the remaining elements of arr1[], if any
    while (i < counter1) {
        fprintf(output, "%s\n", arr1[i]);
        i++;
    }

    // Copy the remaining elements of arr2[], if any
    while (j < counter2) {
        fprintf(output, "%s\n", arr2[j]);
        j++;
    }
}
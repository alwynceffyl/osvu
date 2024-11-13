
#include "forksort.h"


char *myprog;


int main(int argc, char **argv){
    myprog=argv[0];
    unsigned int count;
    char **line = readInput(&count);
    mergeSort(line, 0, count-1);
    printArr(line, count);
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


void merge(char** arr, int left, int mid, int right) {

    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Create temporary arrays
    char **leftArr= (char**) malloc(n1*sizeof(char*));
    char **rightArr= (char**) malloc(n2*sizeof(char*));

    // Copy data to temporary arrays
    for (i = 0; i < n1; i++){
        leftArr[i] = arr[left + i];
    }

    for (j = 0; j < n2; j++){
        rightArr[j] = arr[mid + 1 + j];
    }

    // Merge the temporary arrays back into arr[left..right]
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (strcmp(leftArr[i], rightArr[j])<0) {
            arr[k] = leftArr[i];
            i++;
        }
        else {
            arr[k] = rightArr[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of leftArr[], if any
    while (i < n1) {
        arr[k] = leftArr[i];
        i++;
        k++;
    }

    // Copy the remaining elements of rightArr[], if any
    while (j < n2) {
        arr[k] = rightArr[j];
        j++;
        k++;
    }

    free(rightArr);
    free(leftArr);
}

void mergeSort(char** arr, int left, int right) {
    if (left < right) {
      
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void printArr(char **arr, int count){
    for(int i=0; i< count; i++){
        fprintf(stdout, "%s", arr[i]);
    }
}
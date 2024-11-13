#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** readInput(unsigned int *count);
void freeMemory(char** arr, unsigned count);
void mergeSort(char** arr, int left, int right);
void merge(char** arr, int left, int mid, int right);
void printArr(char **arr, int count);
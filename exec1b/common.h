#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h> //für INT_MAX

/* für Shared Memory */
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
/* semaphore*/
#include <semaphore.h>


#define SHM_NAME "/12207461_SHM"
#define SEM_USED "/12207461_SEMUSED"
#define SEM_FREE "/12207461_SEMFREE"
#define SEM_GENERATOR "/12207461_SEMGENERATOR"

typedef enum COLOUR {
  RED = 0,
  BLUE=1,
  GREEN =2
} colour_t;


#define LEN (32)

typedef struct node{
  colour_t mycolour;
} node_t;

typedef struct solution {
    node_t node_from;
    node_t node_to;
} solution_t;


typedef struct circulabuffer {
    solution_t solution[LEN];
    bool stop;
} circulabuffer_t;


void handle_signal(int signal);
void usage(char* errormsg);
char* printedges(solution_t solution, int* n);
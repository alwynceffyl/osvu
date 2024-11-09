#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h> //für INT_MAX
#include <time.h> //für srand
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
  colour_t colour;
  int value;
} node_t;

typedef struct edges {
    node_t node_from;
    node_t node_to;
} edges_t;

typedef struct edgelist{
    int size;
    edges_t list[];
}edgelist_t;

typedef struct circulabuffer {
    edgelist_t* solution[LEN];
    bool stop;
} circulabuffer_t;


void handle_signal(int signal);
void usage(char* errormsg);
void printEdges(edgelist_t* solution);
edgelist_t* colouring(edges_t *params, int size);

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
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


#define PERMISSIONS 0600
#define LEN 32
#define MAX_EDGES 8

typedef enum COLOUR {
  RED = 0,
  BLUE=1,
  GREEN =2
} colour_t;



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
    edges_t list[MAX_EDGES];
}edgelist_t;

typedef struct circularbuffer {
    bool stop;
    unsigned int read_pos;
    unsigned int write_pos;
    edgelist_t solution[LEN];
} circularbuffer_t;


void handle_signal(int signal);
void usage(char* errormsg);
void printEdges(edgelist_t solution);
edgelist_t colouring(edges_t *params, int size);


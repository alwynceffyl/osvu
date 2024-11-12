#include "common.h"


/**
 * @file supervisor.c
 * @author Phillip Sassmann
 * @date 12.11.2024
 *
 * @brief This file implements a supervisor process that monitors solutions
 *        from a shared memory buffer, processes the best solution, and terminates
 *        when a 3-colorable solution is found or a solution limit is reached.
 */

char* myprog;
volatile sig_atomic_t quit = 0;


/**
 * @brief Signal handler for program termination.
 * 
 * @details Sets the global `quit` flag to true, allowing for a graceful exit
 * from ongoing processes.
 *
 * @param signal Signal number, such as `SIGINT` or `SIGTERM`.
 */
void handle_signal(int signal) {
    quit = 1;
}


/**
 * @brief Parses command-line options, initializes shared memory and semaphores, 
 *        and runs the main supervisor loop to read solutions from the circular 
 *        buffer. If a 3-colorable solution is found or the solution limit is reached, 
 *        it exits gracefully.
 * 
 * @param argc Argument count.
 * @param argv Argument values.
 * @return int Exit status (EXIT_SUCCESS or EXIT_FAILURE).
 */
int main(int argc,char *argv[]){

    myprog=argv[0];
    int opt;
    int limit=INT_MAX, delay=0;  
    int opt_n=0, opt_w=0;
    int num_solutions=0;
    int bestRemovedEdges=INT_MAX;


    while((opt=getopt(argc, argv, "n:w:"))!=-1){
        switch(opt){
        case 'n':
            opt_n++;
            limit=strtol(optarg, NULL , 0) > INT_MAX ? INT_MAX : strtol(optarg, NULL , 0) ;
            if(errno==ERANGE) usage("error in converting the optarg of [n]");
            break;
        case 'w':
            opt_w++;
            delay=strtol(optarg, NULL , 0) > INT_MAX ? INT_MAX : strtol(optarg, NULL , 0) ;
            if(errno==ERANGE) usage("error in converting the optarg of [w]");
            break;
        default: /* ? option */
            usage("invalid options");
        }
    }
    if(opt_n>1){
        usage("too many limits for generators");
    }

    if(opt_w>1){
        usage("too many delays defined");
    }

    int shmfd = shm_open(SHM_NAME, O_RDWR| O_CREAT, PERMISSIONS);
    if(shmfd == -1){
        perror("error in opening shared memory");
        exit(EXIT_FAILURE);
    } 

    if(ftruncate(shmfd, sizeof(circularbuffer_t))==-1){
        perror("error in sizing memory ");
        exit(EXIT_FAILURE);
    }

    circularbuffer_t *circularbuffer;
    circularbuffer=mmap(NULL, sizeof(*circularbuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(circularbuffer==MAP_FAILED){
        perror("error in mapping memory");
        exit(EXIT_FAILURE);
    }


    struct sigaction sa = { .sa_handler = handle_signal };
    if(sigaction(SIGINT, &sa, NULL)==-1 || sigaction(SIGTERM, &sa, NULL)==-1){  
        perror("error in signal handler action");
        exit(EXIT_FAILURE);
    }
 

    sem_t *sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL , PERMISSIONS, LEN);
    sem_t *sem_used = sem_open(SEM_USED, O_CREAT | O_EXCL , PERMISSIONS, 0);
    sem_t *sem_generator = sem_open(SEM_GENERATOR, O_CREAT | O_EXCL, PERMISSIONS, 1);
        
    if (sem_free == SEM_FAILED || sem_used == SEM_FAILED || sem_generator == SEM_FAILED) {
        perror("error in opening semaphores");
        exit(EXIT_FAILURE);
    }

    circularbuffer->stop =false;
    circularbuffer->read_pos =0;
    circularbuffer->write_pos =0;
    circularbuffer->numGen =0;

    sleep(delay);

    while (!quit && num_solutions<=limit ) {

        if(sem_wait(sem_used)==-1 && errno != EINTR){
            perror("error in semaphore wating (supervisor)");
            exit(EXIT_FAILURE);
            break;
        }

        //Read solution from buffer
        edgelist_t solution=circularbuffer->solution[circularbuffer->read_pos];
        circularbuffer->read_pos=(circularbuffer->read_pos+1) % (LEN);

        if(solution.size==0){
            bestRemovedEdges=0;
            fprintf(stdout,"The graph is 3-colorable!\n");
            quit=1;
            circularbuffer->stop;
        }

        if(solution.size < bestRemovedEdges){
            bestRemovedEdges=solution.size;
            //printf("Solution with %d edges: ",bestRemovedEdges);
            //printEdges(solution);
        }

        num_solutions++;

        if(sem_post(sem_free)==-1){
            perror("error in semaphore posting (supervisor)");
            exit(EXIT_FAILURE);
            break;
        }
    }
    

    if(bestRemovedEdges>0){
       fprintf(stdout,"The graph might not be 3-colorable, best solution removes %d edges.\n",bestRemovedEdges);
    }

    while(circularbuffer->numGen>0){
        sem_post(sem_free);
        circularbuffer->numGen=circularbuffer->numGen -1 ;
    }


    /* CLEAN UP */
    if(munmap(circularbuffer, sizeof(*circularbuffer))==-1){
        perror("error in unmapping memory");
        exit(EXIT_FAILURE);
    }

    if(close(shmfd)==-1){
        perror("error in closing shared memory fd");
        exit(EXIT_FAILURE);
    }

    if(shm_unlink(SHM_NAME)==-1){
        perror("error in unlinking shared memory");
        exit(EXIT_FAILURE);
    }

    if(sem_close(sem_free)==-1 || sem_close(sem_used)==-1 || sem_close(sem_generator)==-1){
        perror("error in closing semaphores");
        exit(EXIT_FAILURE);
    }
    if(sem_unlink(SEM_FREE)==-1 || sem_unlink(SEM_USED)==-1 || sem_unlink(SEM_GENERATOR)==-1){
        perror("error in unlinking semaphores");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}


/**
 * @brief Prints an error message and exits the program.
 *
 * @details This function is used to display program usage or error messages,
 * along with the proper command-line syntax.
 *
 * @param errormsg Custom error message to display.
 */
void usage(char* errormsg) {
    fprintf(stderr, "Usage: %s, supervisor [-n limit] [-w delay], errormessage: %s\n",myprog, errormsg);
    exit(EXIT_FAILURE);
}


/**
 * @brief Prints the edges in the provided `edgelist_t` solution.
 *
 * @details Iterates through the list of edges in `solution` and prints each edge as
 * a connection between two nodes. Used to display a generated graph solution.
 *
 * @param solution List of edges representing a graph solution.
 */
void printEdges(edgelist_t solution){
   int i=0;
    for( ; i<solution.size; i++){
        printf("%d-%d ", solution.list[i].node_from.value,solution.list[i].node_to.value);              
    }                                                
  
    printf("\n");
}
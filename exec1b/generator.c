#include "common.h"


/**
 * @file generator.c
 * @author Phillip Sassmann
 * @date 12.11.2024
 *
 * @brief This file contains functions for handling signals, processing
 *        command-line arguments, setting up shared memory, and managing
 *        semaphores to implement a simple graph coloring program.
 */

char* myprog;
volatile sig_atomic_t quit = 0;



/**
 * @details Signal handler to set a flag to terminate the main loop.
 * 
 * @param signal The signal number (e.g., SIGINT).
 */
void handle_signal(int signal) {
    quit = 1;
}


/**
 * @details Parses command-line arguments, sets up shared memory and semaphores, 
 *        and runs the main processing loop for graph coloring until a termination
 *        signal is received or the shared buffer indicates a stop.
 * 
 * @param argc Argument count.
 * @param argv Argument values.
 * @return int Exit status (EXIT_SUCCESS or EXIT_FAILURE).
 */
int main(int argc, char *argv[]) {
    
    myprog=argv[0];
    srand((unsigned int)time(NULL));


    if(argc < 2) usage("we need edges for the graph");

    edges_t params[argc-1];
    int i=1;
    for(; i<argc; i++){
        char* token = strtok(argv[i], "-");
        params[i-1].node_from.value= strtol(token, NULL , 0) > INT_MAX ? INT_MAX : strtol(token, NULL , 0) ;
        token = strtok(NULL, "-");
        params[i-1].node_to.value= strtol(token, NULL , 0) > INT_MAX ? INT_MAX : strtol(token, NULL , 0) ;
        if(strtok(NULL, "-")!=NULL) usage("too many connected nodes");
        if(errno==ERANGE) usage("error in converting the optarg of [n]");
    } 

    

    int shmfd = shm_open(SHM_NAME, O_RDWR, PERMISSIONS);
    if(shmfd == -1){
        perror("error in opening shared memory");
        exit(EXIT_FAILURE);
    } 

    circularbuffer_t *circularbuffer;
    circularbuffer=mmap(NULL,sizeof(*circularbuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(circularbuffer==MAP_FAILED){
        perror("error in mapping memory");
        exit(EXIT_FAILURE);
    }
    circularbuffer->numGen = circularbuffer->numGen +1;


    struct sigaction sa = { .sa_handler = handle_signal };
    if(sigaction(SIGINT, &sa, NULL)==-1 || sigaction(SIGTERM, &sa, NULL)==-1){  
        perror("error in signal handler action");
        exit(EXIT_FAILURE);
    }

    sem_t *sem_free = sem_open(SEM_FREE, PERMISSIONS, LEN);
    sem_t *sem_used = sem_open(SEM_USED,PERMISSIONS, 0);
    sem_t *sem_generator = sem_open(SEM_GENERATOR, PERMISSIONS, 1);


    if (sem_free == SEM_FAILED || sem_used == SEM_FAILED || sem_generator == SEM_FAILED) {
        perror("error in opening semaphores");
        exit(EXIT_FAILURE);
    }

    

    while(!quit && !(circularbuffer->stop)){


        if(sem_wait(sem_generator)==-1 && errno != EINTR){
            perror("error in semaphore wating (generator)");
            exit(EXIT_FAILURE);
            break;
        }
        
        edgelist_t solution=colouring(params, (argc-1));

        if(solution.size<MAX_EDGES){
            if(sem_wait(sem_free)==-1 && errno !=EINTR){
                perror("error in semaphore wating (generator)");
                exit(EXIT_FAILURE);
                break;
            }
        
            circularbuffer->solution[circularbuffer->write_pos]=solution;
            circularbuffer->write_pos=(circularbuffer->write_pos+1) % (LEN);
            

            if(sem_post(sem_used)==-1){
                perror("error in semaphore posting (generator)");
                exit(EXIT_FAILURE);
                break;
            }
        }
        if(sem_post(sem_generator)==-1){
            perror("error in semaphore posting (generator)");
            exit(EXIT_FAILURE);
            break;
        }      
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

    if(sem_close(sem_free)==-1 || sem_close(sem_used)==-1 || sem_close(sem_generator)==-1){
        perror("error in closing semaphores");
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
    fprintf(stderr, "Usage: %s , errormessage: %s\n",myprog, errormsg);
    exit(EXIT_FAILURE);
}

/**
 * @brief Generates a 3-color solution for the graph based on input edges.
 *
 * @details The function initializes each node’s color, then attempts to color each node
 * such that adjacent nodes have different colors. It returns an `edgelist_t` containing
 * edges that fail to meet the 3-coloring criteria.
 *
 * @param params Array of edges representing the graph.
 * @param size Number of edges in the `params` array.
 * @return edgelist_t List of edges that do not meet the 3-coloring condition.
 */
edgelist_t colouring(edges_t params[], int size){

    int i=0;
    for(; i<size; i++){
        params[i].node_from.colour=-1;
        params[i].node_to.colour=-1;
    } 

    i=0;
    for(; i< size; i++){
        if(params[i].node_from.colour==-1 && params[i].node_to.colour==-1){
            params[i].node_from.colour=rand() % 3;
            params[i].node_to.colour= rand() % 3;
        }   
        if(params[i].node_from.colour==-1){
            params[i].node_from.colour = rand() % 3;
        }
        if(params[i].node_to.colour==-1){
            params[i].node_to.colour =  rand() % 3;
        } 

        int j=i+1;
        for(; j<size; j++){
            if(params[i].node_from.value==params[j].node_from.value){
                params[j].node_from.colour=params[i].node_from.colour;
            }
            if(params[i].node_to.value==params[j].node_to.value){
                params[j].node_to.colour=params[i].node_to.colour;
            }
            if(params[i].node_from.value==params[j].node_to.value){
                params[j].node_to.colour=params[i].node_from.colour;
            }
            if(params[i].node_to.value==params[j].node_from.value){
                params[j].node_from.colour=params[i].node_to.colour;
            }
        }
    }
 

    int counter=0;
    i=0;
    for(; i < size; i++){
        if(params[i].node_from.colour==params[i].node_to.colour){
            counter++;
        }
    }


    edgelist_t writeToBuffer;
    writeToBuffer.size=counter;

    counter=0;
    i=0;
    for(; i < size && counter < MAX_EDGES; i++){
        if(params[i].node_from.colour==params[i].node_to.colour){
            writeToBuffer.list[counter].node_from=params[i].node_from;
            writeToBuffer.list[counter].node_to= params[i].node_to;
            counter++;
        }
    }
    return writeToBuffer;
}
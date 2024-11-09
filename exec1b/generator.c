#include "common.h"

char* myprog;
volatile sig_atomic_t quit = 0;

void handle_signal(int signal) {
    quit = 1;
}

int main(int argc, char *argv[]) {
    
    unsigned int write_pos=0;
    myprog=argv[0];
    if(argc < 2) usage("we need edges for the graph");

    edges_t params[argc-1];
    for(int i=1; i<argc; i++){

        char* token = strtok(argv[i], "-");
        params[i-1].node_from.value= strtol(token, NULL , 0) > INT_MAX ? INT_MAX : strtol(token, NULL , 0) ;
        params[i-1].node_from.colour=-1;
        token = strtok(NULL, "-");
        params[i-1].node_to.value= strtol(token, NULL , 0) > INT_MAX ? INT_MAX : strtol(token, NULL , 0) ;
        params[i-1].node_to.colour=-1;
        if(strtok(NULL, "-")!=NULL) usage("too many connected nodes");
        if(errno==ERANGE) usage("error in converting the optarg of [n]");
    } 

    

    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if(shmfd == -1){
        perror("error in opening shared memory");
        exit(EXIT_FAILURE);
    } 

    if(ftruncate(shmfd, sizeof(circulabuffer_t))<0){
        perror("error in sizing memory ");
        exit(EXIT_FAILURE);
    }

    circulabuffer_t *circulabuffer;
    circulabuffer=mmap(NULL, sizeof(*circulabuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(circulabuffer==MAP_FAILED){
        perror("error in mapping memory");
        exit(EXIT_FAILURE);
    }


    struct sigaction sa = { .sa_handler = handle_signal };
    if(sigaction(SIGINT, &sa, NULL)==-1){  
        perror("error in signal handler action");
        exit(EXIT_FAILURE);
    }

    if(sigaction(SIGTERM, &sa, NULL)==-1){  
        perror("error in signal handler action");
        exit(EXIT_FAILURE);
    }
 

    sem_t *sem_free = sem_open(SEM_FREE, O_CREAT , 0600, LEN);
    sem_t *sem_used = sem_open(SEM_USED, O_CREAT , 0600, 0);
    sem_t *sem_generator = sem_open(SEM_GENERATOR, O_CREAT , 0600, 1);


    if (sem_free == SEM_FAILED || sem_used == SEM_FAILED || sem_generator == SEM_FAILED) {
        perror("error in opening semaphores");
        exit(EXIT_FAILURE);
    }
    
    while(!quit && !(circulabuffer->stop)){

        if(sem_wait(sem_free)==-1){
            if(errno ==EINTR) continue;
            perror("error in semaphore wating (generator)");
            exit(EXIT_FAILURE);
            break;
        }

        //also check for generator if someone wants to write the buffer
        if(sem_wait(sem_generator)==-1){
            if(errno ==EINTR) continue;
            perror("error in semaphore wating (generator)");
            exit(EXIT_FAILURE);
            break;
        }

        //Write solution from buffer
        //critical section;        
        edgelist_t *solution = colouring(params, argc-1);
        circulabuffer->solution[write_pos] = solution;
        write_pos = (write_pos + 1) % LEN;

        if(sem_post(sem_generator)==-1){
            perror("error in semaphore posting (generator)");
            exit(EXIT_FAILURE);
            break;
        }
        if(sem_post(sem_used)==-1){
            perror("error in semaphore posting (generator)");
            exit(EXIT_FAILURE);
            break;
        }
    }



    /* CLEAN UP */
    if(munmap(circulabuffer, sizeof(circulabuffer_t))==-1){
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

    if(quit){
        fprintf(stderr, "a signal killed the generator");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}


void usage(char* errormsg) {
    fprintf(stderr, "Usage: %s , errormessage: %s\n",myprog, errormsg);
    exit(EXIT_FAILURE);
}

edgelist_t* colouring(edges_t params[], int size){
    int nodesize=0;


    for(int i=0; i< size; i++){
        if(params[i].node_from.colour==-1 && params[i].node_to.colour==-1){
            params[i].node_from.colour=rand() % 3;
            params[i].node_to.colour=rand() % 3;
            nodesize+=2;
        }   
        if(params[i].node_from.colour==-1){
            params[i].node_from.colour = rand() % 3;
            nodesize++;
        }
        if(params[i].node_to.colour==-1){
            params[i].node_to.colour =  rand() % 3;
            nodesize++;
        } 

        for(int j=i+1; j<size; j++){
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
        //printf("from node:=%d, to node :=%d\n", params[i].node_from.value, params[i].node_to.value);
        //printf("from colour:=%d, to colour :=%d\n", params[i].node_from.colour, params[i].node_to.colour);
    }
 
    srand((unsigned int)time(NULL));
    //printf("distinctive nodes size:=%d\n",nodesize);

    int counter=0;
    for(int i=0; i < size; i++){
        if(params[i].node_from.colour==params[i].node_to.colour){
            counter++;
        }
    }

    //printf("counter of removed edges:=%d\n", counter);

    edgelist_t *writeToBuffer;
    writeToBuffer=(edgelist_t *)malloc(counter* sizeof(edges_t)+sizeof(int));
    if(writeToBuffer==NULL){
        perror("couldnt allocate resources for edgelist for our buffer");
    }

    writeToBuffer->size=counter;

    counter=0;
    for(int i=0; i < size; i++){
        if(params[i].node_from.colour==params[i].node_to.colour){
            writeToBuffer->list[counter].node_from=params[i].node_from;
            writeToBuffer->list[counter].node_to= params[i].node_to;
            counter++;
        }
    }
    return writeToBuffer;
}
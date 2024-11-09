#include "common.h"


char* myprog;
volatile sig_atomic_t quit = 0;

void handle_signal(int signal) {
    quit = 1;
}

int main(int argc,char *argv[]){

    myprog=argv[0];
    int opt;
    int limit=INT_MAX, delay=0;
    int opt_n=0, opt_w=0;
    int num_solutions=0;
    edgelist_t *best_solutions;
    unsigned int read_pos;


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

    int shmfd = shm_open(SHM_NAME, O_RDWR| O_CREAT, 0600);
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
    sem_t *sem_generator = sem_open(SEM_GENERATOR, O_CREAT, 0600, 1);


    if (sem_free == SEM_FAILED || sem_used == SEM_FAILED || sem_generator == SEM_FAILED) {
        perror("error in opening semaphores");
        exit(EXIT_FAILURE);
    }

    //delay?
    sleep(delay);
    circulabuffer->stop =false;

    while (!quit && num_solutions<=limit ) {

        if(sem_wait(sem_used)==-1){
            if(errno ==EINTR) continue;
            perror("error in semaphore wating (supervisor)");
            exit(EXIT_FAILURE);
            break;
        }

        //also check for generator if someone wants to write the buffer
        if(sem_wait(sem_generator)==-1){
            if(errno ==EINTR) continue;
            perror("error in semaphore wating (supervisor)");
            exit(EXIT_FAILURE);
            break;
        }

        //Read solution from buffer
        //critical section;        
        edgelist_t *solution = circulabuffer->solution[read_pos];
        read_pos = (read_pos + 1) % LEN;

        if(sem_post(sem_free)==-1){
            perror("error in semaphore posting (supervisor)");
            exit(EXIT_FAILURE);
            break;
        }
    
        if(solution->size < best_solutions->size){
            best_solutions=solution;
            fprintf(stdout,"Solution with %d edges: %s",best_solutions->size);
            printEdges(solution);
        }
        if(solution->size==0){
            fprintf(stdout,"The graph is 3-colorable!");
            quit=1;
        }
    
        num_solutions++;
    }
    circulabuffer->stop=true;

    if(quit!=1){
       fprintf(stdout,"The graph might not be 3-colorable,\nbest solution removes %d edges.",best_solutions->size);
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


    
    if(quit){
        fprintf(stderr, "a signal killed the supervisor");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}


void usage(char* errormsg) {
    fprintf(stderr, "Usage: %s, supervisor [-n limit] [-w delay], errormessage: %s\n",myprog, errormsg);
    exit(EXIT_FAILURE);
}

void printEdges(edgelist_t* solution){
    char* edges[solution->size];

    for(int i=0; i<solution->size; i++){
        fprintf(stdout,"%ld-%ld ", solution->list[i].node_from.value,solution->list[i].node_from.value);              
    }                                                 
  
    printf("\n");
}
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
    int opt_n=0, opt_w;
    int num_solutions=0;
    solution_t *best_solutions;
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
        usage("error in opening shared memory");
    } 

    if(ftruncate(shmfd, sizeof(circulabuffer_t))<0){
        usage("error in sizing memory ");
    }

    circulabuffer_t *circulabuffer;
    circulabuffer=mmap(NULL, sizeof(*circulabuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(circulabuffer==MAP_FAILED){
        usage("error in mapping memory");
    }


    struct sigaction sa = { .sa_handler = handle_signal };
    if(sigaction(SIGINT, &sa, NULL)==-1){  
        usage("error in signal handler action");
    }

    if(sigaction(SIGTERM, &sa, NULL)==-1){  
        usage("error in signal handler action");
    }
 

    sem_t *sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, LEN);
    sem_t *sem_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
    sem_t *sem_generator = sem_open(SEM_GENERATOR, O_CREAT | O_EXCL, 0600, 1);

    if (sem_free == SEM_FAILED || sem_used == SEM_FAILED || sem_generator == SEM_FAILED) {
        usage("error in opening semaphores");
    }

    //delay?
    sleep(delay);
    circulabuffer->stop =false;

    while (!quit && num_solutions<=limit ) {
        quit=1;
        break;

        if(sem_wait(sem_used)==-1){
            if(errno ==EINTR) continue;
            usage("error in semaphore wating (supervisor)");
        }

        //also check for generator if someone wants to write the buffer
        if(sem_wait(sem_generator)==-1){
            if(errno ==EINTR) continue;
            usage("error in semaphore wating (supervisor)");
        }

        //Read solution from buffer
        //critical section;        
        solution_t *solution = &circulabuffer->solution[read_pos];
        read_pos = (read_pos + 1) % LEN;

        if(sem_post(sem_free)==-1){
            usage("error in semaphore posting (supervisor)");
        }
    
        if(solution->numRemovedEdges < best_solutions->numRemovedEdges){
            best_solutions=solution;
            fprintf(stdout,"Solution with %d edges: %s",best_solutions->numRemovedEdges, best_solutions->removedEdges);
        }
        if(solution->numRemovedEdges==0){
            fprintf(stdout,"The graph is 3-colorable!");
            quit=1;
        }
        num_solutions++;
    }
    circulabuffer->stop =true;

    if(quit!=1){
        fprintf(stdout,"The graph might not be 3-colorable,\nbest solution removes %d edges.",best_solutions->numRemovedEdges);
    }


    if(close(shmfd)==-1){
        usage("error in closing shared memory fd");
    }


    if(munmap(circulabuffer, sizeof(circulabuffer_t))==-1){
        usage("error in unmapping memory");
    }

    if(shm_unlink(SHM_NAME)==-1){
        usage("error in unlinking shared memory");
    }

    if(sem_close(sem_free)==-1 || sem_close(sem_used)==-1 || sem_close(sem_generator)==-1){
        usage("error in closing semaphores");
    }
    if(sem_unlink(SEM_FREE)==-1 || sem_unlink(SEM_USED)==-1 || sem_unlink(SEM_GENERATOR)==-1){
        usage("error in unlinking semaphores");
    }
    
    exit(EXIT_SUCCESS);
}


void usage(char* errormsg) {
    fprintf(stderr, "Usage: %s , errormessage: %s\n",myprog, errormsg);
    exit(EXIT_FAILURE);
}
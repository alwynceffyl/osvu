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
    for(int i=1; i<argc; i++){
        printf("%s\n", argv[i]);
    }

    
    struct sigaction sa = { .sa_handler = handle_signal };
    if(sigaction(SIGINT, &sa, NULL)==-1){  
        usage("error in signal handler action");
    }

    if(sigaction(SIGTERM, &sa, NULL)==-1){  
        usage("error in signal handler action");
    }

    int shmfd = shm_open(SHM_NAME, O_RDWR| O_CREAT, 0600);
    if(shmfd == -1){
        usage("error in opening shared memory");
    } 


    circulabuffer_t *circulabuffer;
    circulabuffer=mmap(NULL, sizeof(*circulabuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(circulabuffer==MAP_FAILED){
        usage("error in mapping memory");
    } 

    sem_t *sem_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, LEN);
    sem_t *sem_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
    sem_t *sem_generator = sem_open(SEM_GENERATOR, O_CREAT | O_EXCL, 0600, 1);

    if (sem_free == SEM_FAILED || sem_used == SEM_FAILED || sem_generator == SEM_FAILED) {
        usage("error in opening semaphores");
    }

    while(!quit &&  !circulabuffer->stop){

        if(sem_wait(sem_free)==-1){
            if(errno ==EINTR) continue;
                usage("error in semaphore wating (generator)");
        }

        //also check for generator if someone wants to write the buffer
        if(sem_wait(sem_generator)==-1){
            if(errno ==EINTR) continue;
            usage("error in semaphore wating (generator)");
        }

        //write solution from buffer
        //critical section;        
        solution_t solution={.numRemovedEdges=1, .removedEdges=""};
        circulabuffer->solution[write_pos]= solution;
        write_pos = (write_pos + 1) % LEN;

          if(sem_post(sem_generator)==-1){
            usage("error in semaphore posting (generator)");
        }

        if(sem_post(sem_used)==-1){
            usage("error in semaphore posting (generator)");
        }

    }
    
    if(munmap(circulabuffer, sizeof(circulabuffer_t))==-1){
        usage("error in unmapping memory");
    }

    if(sem_close(sem_free)==-1 || sem_close(sem_used)==-1 || sem_close(sem_generator)==-1){
        usage("error in closing semaphores");
    }
    */
    exit(EXIT_SUCCESS);
}


void usage(char* errormsg) {
    fprintf(stderr, "Usage: %s , errormessage: %s\n",myprog, errormsg);
    exit(EXIT_FAILURE);
}
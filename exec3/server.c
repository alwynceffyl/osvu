#include "server.h"

char* myprog;

int main(int argc,char *argv[]){

    myprog=argv[0];
    exit(EXIT_SUCCESS);
}


static void usage(char* errormsg) {
    fprintf(stderr, "Usage: %s, supervisor [-n limit] [-w delay], errormessage: %s\n",myprog, errormsg);
    exit(EXIT_FAILURE);
}

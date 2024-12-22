#include "server.h"

char *myprog;
volatile sig_atomic_t quit = 0;
#define BUFFER_SIZE 1024
/**
 * @details Signal handler to set a flag to terminate the main loop.
 * 
 * @param signal The signal number (e.g., SIGINT).
 */
void handle_signal(int signal) {
    quit = 1;
}


int main(int argc, char *argv[]){
    myprog=argv[0];
    char *port="8080";
    char *filename="index.html";
    char *docRoot;

    parse_arg(argc, argv, &port, &docRoot, &filename);
    //fprintf(stdout,"my docroot=%s\n" ,docRoot);
    //fprintf(stdout,"my port is :%s and my filename=%s\n" ,port, filename);

    struct sigaction sa = { .sa_handler = handle_signal };
    if(sigaction(SIGINT, &sa, NULL)==-1 || sigaction(SIGTERM, &sa, NULL)==-1){  
        perror("error in signal handler action");
        exit(EXIT_FAILURE);
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0){
        perror("error in socket");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof optval) < 0){
        perror("error in setsockopt");
        exit(EXIT_FAILURE);
    }


    errno=0;
    char *endptr;
    long val = strtol(optarg, &endptr, 10);
    if (errno != 0 ) {
        usage("error in parsing strtol");
    }

    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(val);

    if (bind(sockfd, (struct sockaddr *) &myaddr, sizeof(myaddr)) < 0){
        perror("error in bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 4) < 0){
        perror("error in listen");
        exit(EXIT_FAILURE);
    }

    //printf("Server listening on port %s\n", port);

    while(!quit){
        int connfd = accept(sockfd, NULL, NULL);
        if (connfd < 0){
           if(errno == EINTR){
                continue;
            }
            perror("error in accepting");
            exit(EXIT_FAILURE);
        }
        handle_client(connfd, docRoot, filename);
        

        close(connfd);
    }
    //printf("Server terminated.\n");
    exit(EXIT_SUCCESS);
}

void send_response(int client_sock, int status_code, const char *status_message, const char *content_type, const char *file_path) {
    char buffer[BUFFER_SIZE];
    char date[128];
    time_t now = time(NULL);
    struct tm *gmt = gmtime(&now);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmt);

    if (status_code == 200 && file_path) {
        FILE *file = fopen(file_path, "rb");
        if (!file) {
            send_response(client_sock, 404, "Not Found", NULL, NULL);
            return;
        }
        struct stat st;
        stat(file_path, &st);
        int content_length = st.st_size;

        snprintf(buffer, sizeof(buffer), 
                 "HTTP/1.1 200 OK\r\n"
                 "Date: %s\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: close\r\n\r\n", date, content_length);
        send(client_sock, buffer, strlen(buffer), 0);

        while ((content_length = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            send(client_sock, buffer, content_length, 0);
        }

        fclose(file);
    } else {
        snprintf(buffer, sizeof(buffer), 
                 "HTTP/1.1 %d %s\r\n"
                 "Date: %s\r\n"
                 "Connection: close\r\n\r\n", 
                 status_code, status_message, date);
        send(client_sock, buffer, strlen(buffer), 0);
    }

    close(client_sock);
}

void handle_client(int client_sock, const char *doc_root, const char *index_file) {
    char buffer[BUFFER_SIZE];
    if (recv(client_sock, buffer, sizeof(buffer) - 1, 0) <= 0) {
        close(client_sock);
        return;
    }

    char method[16], path[256], protocol[16];
    if (sscanf(buffer, "%15s %255s %15s", method, path, protocol) != 3) {
        send_response(client_sock, 400, "Bad Request", NULL, NULL);
        return;
    }

    if (strcmp(method, "GET") != 0) {
        send_response(client_sock, 501, "Not Implemented", NULL, NULL);
        return;
    }

    if (strcmp(protocol, "HTTP/1.1") != 0) {
        send_response(client_sock, 400, "Bad Request", NULL, NULL);
        return;
    }

    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", doc_root, path);

    if (full_path[strlen(full_path) - 1] == '/') {
        strncat(full_path, index_file, sizeof(full_path) - strlen(full_path) - 1);
    }

    struct stat st;
    if (stat(full_path, &st) != 0 || S_ISDIR(st.st_mode)) {
        send_response(client_sock, 404, "Not Found", NULL, NULL);
        return;
    }

    send_response(client_sock, 200, "OK", "text/html", full_path);
}

static void usage(char *message){
    fprintf(stderr,"my programm:%s , usage : server [-p PORT] [-i INDEX] DOC_ROOT : %s", myprog, message);
    exit(EXIT_FAILURE);
}


void parse_arg(int argc, char *argv[], char **port, char **docroot, char **filename){
    bool p_flag=false, i_flag=false;
    int opt;
    while((opt=getopt(argc, argv, "p:i:")) != -1){
        switch (opt){
        case 'p':
            if(p_flag){
                usage("too many -p arguments");
            }
            p_flag=true;
            errno=0;
            char *endptr;
            long val = strtol(optarg, &endptr, 10);
            if (errno != 0 || endptr == optarg || *endptr != '\0' || val< 0 || val > 65535) {
                usage("error in parsing port");
            }
              
            *port=optarg;
            break;
        case 'i':
            if(i_flag){
                usage("too many -i arguments");
            }
            *filename=optarg;
            break;
        default:
            usage("unknown argument found");
        }
    }

    if(argc-optind != 1){
        usage("error no docroot declared");
    }

    *docroot=argv[optind];
}

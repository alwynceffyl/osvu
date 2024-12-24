
/**
 * @file server.c
 * @brief Implementation of a basic HTTP server.
 * @author Phillip Sassmann - 12207461
 * @date 24.12.2024
 */


#include "server.h"

char *myprog;
volatile sig_atomic_t quit = 0;


/**
 * @brief Signal handler to terminate the main loop.
 * @details Sets the quit flag to terminate the server gracefully when SIGINT or SIGTERM is received.
 * 
 * @param signal The signal number.
 */
void handle_signal(int signal) {
    quit = 1;
}


/**
 * @brief Entry point for the HTTP server program.
 * @details Initializes the server configuration, sets up signal handling, creates a listening socket,
 *          and processes client requests in a loop until termination. The server can be terminated
 *          gracefully with SIGINT or SIGTERM signals.
 * 
 * @param argc The number of command-line arguments.
 * @param argv Array of command-line arguments. Expected arguments include optional flags for
 *             port (-p) and index file (-i), followed by the document root.
 * @return Returns 0 on successful termination, or exits with an error code on failure.
 */
int main(int argc, char *argv[]){
    myprog=argv[0];
    char *port="8080";
    char *filename="index.html";
    char *docRoot;
    int portInt=8080;

    parse_arg(argc, argv, &port, &docRoot, &filename, &portInt);
    fprintf(stdout,"my docroot=%s\n" ,docRoot);
    fprintf(stdout,"my port is :%s and my filename=%s\n" ,port, filename);

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



    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(portInt);


    if (bind(sockfd, (struct sockaddr *) &myaddr, sizeof(myaddr)) < 0){
        perror("error in bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 4) < 0){
        perror("error in listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %s\n", port);

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
    printf("Server terminated.\n");
    exit(EXIT_SUCCESS);
}


/**
 * @brief Sends an HTTP response to the client.
 * @details Constructs an HTTP response based on the status code and requested file.
 * 
 * @param stream File stream for the client connection.
 * @param status_code HTTP status code to send (e.g., 200, 404).
 * @param fileWrite Relative path to the requested file.
 * @param doc_root Path to the document root directory.
 */
void send_response(FILE *stream, int status_code, char *fileWrite, const char *doc_root) {
    
    char outstr[200];
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);
    if (tmp == NULL) {
        perror("localtime");
        exit(EXIT_FAILURE);
    }

    if (strftime(outstr, sizeof(outstr), "%a, %d %b %C %H:%M:%S", tmp) == 0) {
        fprintf(stderr, "strftime returned 0");
        exit(EXIT_FAILURE);
    }

    
    if (status_code == 200 && fileWrite != NULL) {

        char *file_path=(char *)malloc(strlen(fileWrite) + strlen(doc_root) + 1);
        strncpy(file_path, doc_root, strlen(doc_root));
        strncat(file_path, fileWrite, strlen(fileWrite));

        file_path[strlen(file_path)]='\0';

        FILE *file = fopen(file_path, "r+");
        if (file==NULL) {
            send_response(stream, 404, NULL, NULL);
            return;
        }

        fseek(file, 0L, SEEK_END);   
        long int res = ftell(file); 
        rewind(file);

        fprintf(stream,"HTTP/1.1 200 OK\r\n");
        fprintf(stream,"Date: %s\r\n", outstr);
        fprintf(stream,"Content-Length: %ld\r\n", res);
        fprintf(stream, "Connection: close\r\n\r\n");
        fflush(stream);

        char *line = NULL;
        size_t len = 0;
        ssize_t nread;
        while ((nread = getline(&line, &len, file)) != -1) {
            fwrite(line, nread, 1, stream);
            fflush(stream);

        }
        fclose(file);
        free(line);


    } else {
        char *message;
        switch(status_code){
            case 400: 
                message="BAD REQUEST";
                break;
            case 501:
                message="NOT IMPLEMENTED";
                break;
            case 404:
                message="NOT FOUND";
            default:
                break;
        }
        fprintf(stream,"HTTP/1.1 %d %s\r\n", status_code, message);
        fprintf(stream,"Date: %s\r\n", outstr);
        fprintf(stream, "Connection: close\r\n\r\n");
        fflush(stream);
    }
}

/**
 * @brief Handles a client request.
 * @details Reads the request from the client, parses it, and sends an appropriate HTTP response.
 * 
 * @param client_sock File descriptor for the connected client socket.
 * @param doc_root Path to the document root directory.
 * @param index_file Default file to serve when the root URL is requested.
 */
void handle_client(int client_sock, const char *doc_root, const char *index_file) {
    FILE *stream = fdopen(client_sock, "r+");
    if (!stream) {
        perror("fdopen failed");
        close(client_sock);
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int status_code = 200;
    char *method = NULL;
    char *filename = NULL;
    char *protocol = NULL;

    // Read the first line (request line)
    nread = getline(&line, &len, stream);
    if (nread == -1) {
        perror("getline failed");
        status_code = 400;
        goto cleanup;
    }

    // Parse the request line
    method = strtok(line, " ");
    filename = strtok(NULL, " ");
    protocol = strtok(NULL, " ");

    char *newFilename;
    if (method==NULL || strcmp(method, "GET") != 0) {
        status_code = 501; // Not Implemented
    }
    
    if (filename==NULL){
        status_code=400;
    }

    if (filename !=NULL && strcmp(filename, "/") == 0) {
        newFilename=(char*)malloc(strlen(index_file) + 2 );
        strcpy(newFilename, "/");
        strncat(newFilename, index_file, strlen(index_file));
        newFilename[strlen(newFilename)]='\0';
    }
    
    if (protocol==NULL || strncmp(protocol, "HTTP/1.1", 8) != 0) {
        status_code = 400; // Bad Request
    }

    if(newFilename==NULL) newFilename=filename;
    // Consume remaining headers

    char *header_line = NULL;
    while ((nread = getline(&header_line, &len, stream)) != -1) {
        if (strcmp(header_line, "\r\n") == 0 || strcmp(header_line, "\n") == 0) {
            break;
        }
    }

    if (nread == -1 && !feof(stream)) {
        perror("getline failed while reading headers");
        status_code = 400;
    }

    // Send the response
    send_response(stream, status_code, newFilename, doc_root);

cleanup:
    free(line);
    close(client_sock);
}


/**
 * @brief Prints an error message and program usage information, then exits.
 *
 * @details Displays an error message indicating incorrect usage or arguments, then
 * terminates the program.
 *
 * @param message Description of the encountered error.
 */
static void usage(char *message){
    fprintf(stderr,"my programm:%s , usage : server [-p PORT] [-i INDEX] DOC_ROOT : %s", myprog, message);
    exit(EXIT_FAILURE);
}


void parse_arg(int argc, char *argv[], char **port, char **docroot, char **filename, int *portInt){
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
            *portInt=val;
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

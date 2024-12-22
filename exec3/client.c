/**
 * @file client.c
 * @author 
 * @brief HTTP-Client zur Verarbeitung von URLs und Abrufen von Ressourcen.
 * @date 2024-12-22
 */
 
#include "client.h"

char *myprog;


/**
 * @brief Analysiert eine URL und extrahiert Hostnamen, Dateipfad und Dateinamen.
 * @details Die Funktion überprüft, ob die URL mit "http://" beginnt. Anschließend wird der Hostname,
 * der Dateipfad und optional der Dateiname extrahiert.
 * @param url Die zu analysierende URL (Eingabe).
 * @param filepath Zeiger auf den extrahierten Dateipfad.
 * @param filename Zeiger auf den extrahierten Dateinamen.
 * @param hostname Zeiger auf den extrahierten Hostnamen.
 */
void parse_url(char *url, char **filepath, char **filename, char **hostname) {

    if (strncmp(url, "http://", 7) != 0) {
        fprintf(stderr, "URL not suitable for us (must start with http://)\n");
        exit(EXIT_FAILURE);
    }


    char *copyUrl = strdup(url + 7); // Skip "http://"
    if (copyUrl == NULL) {
        perror("strdup");
        exit(EXIT_FAILURE);
    }

    int i = strcspn(copyUrl, ";/?:@=&");
    if(i==0){
        fprintf(stderr, "invalid hostname\n");
        exit(EXIT_FAILURE);
    }


    *filepath=&url[i+7];
    copyUrl[i]='\0';
    *hostname = copyUrl;
    i= strcspn(*(filepath)+1, ";/?:@=&");
    if(i==0) *filename="index.html";
    else{
        *filename=malloc(strlen(*(filepath)+1)-i);
        strncpy(*filename,*(filepath)+1,i);
    }
    
}

/**
 * @brief Überprüft, ob eine Zeichenkette leer oder nur aus Leerzeichen besteht.
 * @param str Die zu überprüfende Zeichenkette.
 * @return 1, wenn die Zeichenkette leer oder nur aus Leerzeichen besteht, sonst 0.
 */
int is_empty_or_spaces(const char *str) {
    if (str == NULL) {
        return 1;
    }
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}


/**
 * @brief Analysiert eine HTTP-Header-Zeile, um den Statuscode zu ermitteln.
 * @details Der HTTP-Header wird in Protokoll und Statuscode zerlegt. Die Funktion validiert das
 * Protokoll und gibt den Statuscode zurück.
 * @param str Der zu analysierende HTTP-Header (Eingabe).
 * @return 0, wenn der Statuscode 200 (OK) ist, 1 für andere Statuscodes, -1 bei Protokoll- oder
 * Header-Fehlern.
 */
int parse_header(char *str) {
    char *protocol, *code, *endptr;
    long val;

    protocol = strtok(str, " ");
    if (protocol == NULL || strcmp(protocol, "HTTP/1.1") != 0) {
        return -1; // Invalid protocol
    }

    code = strtok(NULL, " ");
    if (code == NULL) {
        return -1; // No status code
    }

    errno = 0;
    val = strtol(code, &endptr, 10);
    if (errno != 0 || endptr == code || *endptr != '\0') {
        return -1; // Invalid status code
    }
    return val == 200 ? 0 : 1;
}


/**
 * @brief Zeigt die Programmnutzung an und beendet das Programm.
 * @details Diese Funktion gibt eine Fehlermeldung aus und beendet das Programm mit einem
 * Fehlercode.
 * @param errormsg Die anzuzeigende Fehlermeldung.
 */
static void usage(char *errormsg) {
    fprintf(stderr, "Usage: %s [-p PORT] [ -o FILE | -d DIR ] URL\nError: %s\n", myprog, errormsg);
    exit(EXIT_FAILURE);
}



/**
 * @brief Analysiert die Argumente der Kommandozeile.
 * @details Unterstützt Optionen für Portnummer, Ausgabedatei, Ausgabeverzeichnis und URL. 
 * Die Funktion überprüft die Reihenfolge der Argumente und validiert die Eingaben.
 * @param argc Anzahl der Argumente.
 * @param argv Array der Argumente.
 * @param port Zeiger auf den zu verwendenden Port.
 * @param filename Zeiger auf die Datei, in die geschrieben werden soll.
 * @param url Zeiger auf die URL, die verarbeitet werden soll.
 * @param directory Zeiger auf das Ausgabeverzeichnis.
 */
void parse_arg(int argc, char *argv[], char **port, char **filename, char **url, char **directory) {
    int opt;
    int p_flag = 0, o_flag = 0, d_flag = 0;

    while ((opt = getopt(argc, argv, "p:o:d:")) != -1) {
        switch (opt) {
            case 'p':
                if (p_flag > 0) {
                    usage("Too many -p arguments");
                }
                p_flag++;
                *port = optarg;
                break;
            case 'o':
                if (o_flag > 0 || d_flag > 0) {
                    usage("Wrong sequence of arguments");
                }
                o_flag++;
                *filename = optarg;
                break;
            case 'd':
                if (o_flag > 0 || d_flag > 0) {
                    usage("Wrong sequence of arguments");
                }
                d_flag++;
                *directory = optarg;
                break;
            default:
                usage("Invalid argument");
        }
    }

    if (optind == argc) {
        usage("Missing URL argument");
    }
    *url = argv[optind];
    optind++;
    if (optind != argc) {
        usage("something is wrong");
    }

    errno=0;
    char *endptr; 
    long val= strtol(*port, &endptr, 0);

    if (errno != 0 || endptr==*port || *endptr!='\0' || val < 0 || val > 65535) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }
}


/**
 * @brief Hauptfunktion des Programms.
 * @details Die Hauptfunktion steuert den Ablauf des Programms. Sie analysiert die Argumente,
 * stellt eine Verbindung zu einem HTTP-Server her, fordert Daten an und speichert die Antwort.
 * @param argc Anzahl der Argumente.
 * @param argv Array der Argumente.
 * @return EXIT_SUCCESS bei erfolgreicher Ausführung, sonst ein Fehlercode.
 */
int main(int argc, char *argv[]) {
    myprog = argv[0];
    char *port = "80";
    char *fileWrite = NULL, *dirWrite = NULL, *url = NULL;
    char *filepath,*hostname = NULL, *filename = NULL;
    int socketfd;

    parse_arg(argc, argv, &port, &fileWrite, &url, &dirWrite);
    parse_url(url, &filepath, &filename, &hostname);

    struct addrinfo hints, *rp, *result;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int res = getaddrinfo(hostname, port, &hints, &result);
    if (res != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        socketfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socketfd == -1) continue;

        if (connect(socketfd, rp->ai_addr, rp->ai_addrlen) != -1) break; // Success
        close(socketfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        freeaddrinfo(result);
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);

    FILE *sockfile = fdopen(socketfd, "r+");
    if (sockfile == NULL) {
        perror("fdopen");
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    fprintf(sockfile, "GET %s HTTP/1.1\r\n", filepath);
    fprintf(sockfile, "Host: %s\r\n", hostname);
    fprintf(sockfile, "Connection: close\r\n\r\n");
    fflush(sockfile);

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    bool nonHeader = false;


    FILE *output = stdout;
    if (fileWrite != NULL) {
        output = fopen(fileWrite, "w");
        if (output == NULL) {
            perror("fopen");
            fclose(sockfile);
            exit(EXIT_FAILURE);
        }
    } else if (dirWrite != NULL) {
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", dirWrite,  filename);
        output = fopen(filepath, "w");
        if (output == NULL) {
            perror("fopen");
            fclose(sockfile);
            exit(EXIT_FAILURE);
        }
    }
    int header=0;
    while ((nread = getline(&line, &len, sockfile)) != -1) {
        if(header==0){
            
            char *temp=strdup(line);
            if(temp==NULL){
                perror("strdup");
                exit(EXIT_FAILURE);
            }

            int code=parse_header(temp);
            if(code<0){
                fprintf(stderr, "Protocol error!\n");
                free(temp);
                exit(2);
            }
            if(code>0){
                fprintf(stderr, "%s", line+9);
                free(temp);
                exit(3);
            }

            free(temp);

        }
        if (!nonHeader && is_empty_or_spaces(line)) {
            nonHeader = true;
        } else if (nonHeader) {
            fwrite(line, nread, 1, output);
        }
        header++;

    }
    free(hostname);
    //free(filename);

    free(line);
    fclose(sockfile);
    fclose(output);
    return EXIT_SUCCESS;
}

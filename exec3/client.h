/**
 * @file client.c
 * @author Phillip Sassman 12207461
 * @brief HTTP-Client zur Verarbeitung von URLs und Abrufen von Ressourcen.
 * @date 2024-12-22
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdbool.h>


/**
 * @brief Gibt eine Usage-Information aus und beendet das Programm.
 * @param errormsg Die Fehlermeldung, die angezeigt werden soll.
 */
static void usage(char* errormsg);



/**
 * @brief Analysiert eine URL und extrahiert den Hostnamen, Dateipfad und Dateinamen.
 * @details Die Funktion überprüft, ob die URL mit "http://" beginnt und zerlegt die URL in 
 * Hostnamen, Dateipfad und ggf. den Dateinamen.
 * @param url Die zu analysierende URL.
 * @param filepath Pointer zum extrahierten Dateipfad.
 * @param filename Pointer zum extrahierten Dateinamen.
 * @param hostname Pointer zum extrahierten Hostnamen.
 */
void parse_url(char *url, char **filepath, char **filename, char **hostname);

/**
 * @brief Analysiert die Argumente der Kommandozeile.
 * @details Unterstützt Argumente für Port, Ausgabedatei, Verzeichnis und URL.
 * @param argc Anzahl der Argumente.
 * @param argv Array der Argumente.
 * @param port Der zu verwendende Port.
 * @param filename Die Datei, in die geschrieben werden soll.
 * @param url Die URL, die verarbeitet werden soll.
 * @param directory Das Verzeichnis, in das geschrieben werden soll.
 */
void parse_arg(int argc, char *argv[], char **port, char **filename, char **url, char **directory) ;

/**
 * @brief Überprüft, ob eine Zeichenkette leer oder nur aus Leerzeichen besteht.
 * @param str Die zu überprüfende Zeichenkette.
 * @return 1, wenn die Zeichenkette leer oder nur Leerzeichen enthält, sonst 0.
 */
int is_empty_or_spaces(const char *str);

/**
 * @brief Analysiert den HTTP-Header, um den Statuscode zu ermitteln.
 * @param str Die zu analysierende Header-Zeile.
 * @return 0, wenn der Statuscode 200 (OK) ist, 1 für andere Statuscodes, -1 bei Fehlern.
 */
int parse_header(char *str);
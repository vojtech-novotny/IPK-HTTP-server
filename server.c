// IPK - project 1
// HTTP server
// *****************************
// @author -    Vojtěch Novotný
// @login -     xnovot1f
// @date -      16.03.2019

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

// Error codes
#define EC_ARGC 11
#define EC_ARGV 12
#define EC_RECV 31
#define EC_INVALID_REQUEST 41

// Error messages
const char *EM_ARGC = "Missing or extra arguments.";
const char *EM_ARGV = "Invalid arguments.";
const char *EM_RECV = "Something happened while receiving message.";
const char *EM_INVALID_REQUEST = "Invalid HTTP request.";

typedef enum {
    AS_READING_GET,
    AS_READING_ADRESS,
    AS_READING_VERSION,
    AS_READING_ATTRIBUTES
} Analyzer_State;

/// Function prints error_message on the STDERR stream
/// and exits with error_code.
void error(int error_code, const char * error_message);

void server (const int port_number);

int main (int argc, const char * argv[]) {
    // Program requires an argument.
    if (argc != 2)
        error(EC_ARGC, EM_ARGC);

    char *endptr;
    const int port_n = strtol(argv[1], &endptr, 10);
    if (*endptr)
        error(EC_ARGV, EM_ARGV);

    server(port_n);

    return 0;
}

void server (const int port_number) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    //int run = 1;

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    printf("Listening...\n");
    listen(server_socket, 10);

    int client_socket;
    char * buffer = malloc(1024);
    char * analyzer = malloc(1024);
    //while (run) {

    printf("Accepting...\n");
    client_socket = accept(server_socket, NULL, NULL);
    int message_length = recv(client_socket, buffer, 255, 0);
    printf("Receiving...\n");
    if (message_length == -1) {
        free(buffer);
        free(analyzer);
        error(EC_RECV, EM_RECV);
    }

    printf("%s\n", buffer);

    int reading = 0;
    int analyzer_i = 0;
    Analyzer_State state = AS_READING_GET;
    for (int i = 0; i < 255; i++) {
        if (buffer[i] != ' ') {
            reading = 1;
            analyzer[analyzer_i] = buffer[i];
            analyzer_i++;
        } else {
            if (reading == 1) {
                reading = 0;
                analyzer_i = 0;
                char* search_result;
                //search_result = strstr(analyzer, "GET");
                switch (state) {
                    case AS_READING_GET:
                    search_result = strstr(analyzer, "GET");

                    if (search_result == NULL) {
                        free(buffer);
                        free(analyzer);
                        error(EC_INVALID_REQUEST, EM_INVALID_REQUEST);
                    }

                    state = AS_READING_ADRESS;
                    break;
                    case AS_READING_ADRESS:
                    search_result = strstr(analyzer, "/");

                    if (search_result == NULL) {
                        free(buffer);
                        free(analyzer);
                        error(EC_INVALID_REQUEST, EM_INVALID_REQUEST);
                    }

                    const char * response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><body><p>LOL</p></body></html>";
                    send(client_socket, response, strlen(response), 0);
                    state = AS_READING_VERSION;
                    break;
                    case AS_READING_VERSION:
                    break;
                    case AS_READING_ATTRIBUTES:
                    break;
                }
                // if (search_result != NULL && state == AS_READING_GET) {
                //     printf("%s\n", buffer);
                // }
            }
        }

    }
    //}

    //send(client_socket, 10, sizeof(int), 0);
    free(buffer);
    free(analyzer);
    close(server_socket);
}

// int HTTP_ok() {
//     const char * response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<head></head><body><p>LOL</p></body>"
// }

void error(int error_code, const char * error_message) {
    fprintf(stderr, "Error %d!\n%s\n\n", error_code, error_message);
    exit(error_code);
}

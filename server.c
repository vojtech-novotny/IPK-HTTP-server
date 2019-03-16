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

// Error messages
const char *EM_ARGC = "Missing or extra arguments.";
const char *EM_ARGV = "Invalid arguments.";
const char *EM_RECV = "Something happened while receiving message.";

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
    int run = 1;

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    listen(server_socket, 10);

    int client_socket;
    void * buffer = malloc(1024);
    while (run) {
        client_socket = accept(server_socket, NULL, NULL);
        int message_length = recv(client_socket, buffer, 1024, 0);
        if (message_length == -1)
            error(EC_RECV, EM_RECV);
    }

    //send(client_socket, 10, sizeof(int), 0);

    close(server_socket);
}

void error(int error_code, const char * error_message) {
    fprintf(STDERR, "Error %d!\n%s\n\n", error_code, error_message);
    exit(error_code);
}

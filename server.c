/// IPK - project 1
/// HTTP server
/// *****************************
/// @author  Vojtěch Novotný
/// @login   xnovot1f
/// @date    16.03.2019

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <time.h>

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
    OF_TP,
    OF_AJ,
    OF_UNKNOWN
} Output_Format;

/// @var Bool determining whether the server should continue running.
/// Toggled by ctrl+c .
static volatile int run = 1;

/// @var Buffer for received messages. It's global so that the memory can be
/// freed ad any point.
char * buffer;

/// Function prints error_message on the STDERR stream
/// and exits with error_code.
/// @param  error_code      Code of the error. Error codes are defined
///                         at the beginning of the file.
/// @param  error_message   Message of the error. Error messages are
///                         defined at the beginning of the file.
void error(int error_code, const char * error_message);

/// Main functionality of the server.
/// @param  port_number The number of the port the server should use.
void server (const int port_number);

/// Super lightweight version of the server for testing.
/// @param  port_number The number of the port the server should use.
void small_server (const int port_number);

/// Handler called when ctrl+c is pressed.
/// @param  dummy   I don't know why, but it has to be there.
void close_handler (int dummy);

/// Function computes the cpu load from numbers in /proc/stat.
/// @return     int The cpu usage percentage.
/// @warning    The function is BLOCKING.
int get_load();

// ############################################################################
// ###                          START OF PROGRAM                            ###
// ############################################################################

/// Program entry point.
/// @param  argc    Number of program arguments.
/// @param  argv    Program arguments. There is a required argument -
///                 number of port, that will be used for the server.
int main (int argc, const char * argv[]) {
    // Program requires an argument.
    if (argc != 2)
        error(EC_ARGC, EM_ARGC);

    char *endptr;
    const int port_n = strtol(argv[1], &endptr, 10);
    if (*endptr)
        error(EC_ARGV, EM_ARGV);

    // Registering signal handler for CTRL+C.
    signal(SIGINT, close_handler);

    // Main program function.
    server(port_n);

    return 0;
}

void server (const int port_number) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Buffer for received messages
    buffer = malloc(1024*sizeof(char));

    // Buffer for building the response
    char response[512];

    // Geting name of Host
    char hostname[64];
    gethostname(hostname, sizeof(hostname) - 1);

    // Geting name of CPU
    char cpuname[64];
    FILE * cpuinfo_fp = fopen("/proc/cpuinfo", "r");
    int read_n = 0;
    while ((read_n = fscanf(cpuinfo_fp, "%s", cpuname)) == 1) {
        if (strstr(cpuname, "model") != NULL) {
            fscanf(cpuinfo_fp, "%s", cpuname);
            if (strstr(cpuname, "name") != NULL)
                break;
        }
    }
    fscanf(cpuinfo_fp, "%s", cpuname);
    getc(cpuinfo_fp);
    fgets(cpuname, 64, cpuinfo_fp);

    // Preparing cpuload (BLOCKING)
    int cpuload;
    char cpuload_string[14];

    // Construct universal parts of response
    const char response_OK[] = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: ";
    const char response_TP[] = "text/plain";
    const char response_AJ[] = "application/json";
    const char response_CL[] = "\r\nContent-Length: ";
    const char response_NL[] = "\r\n\r\n";
    const char response_default[] = "This is the default response.\nRequest objects:\n /hostname to find out name of the host\n /cpu-name to find out model name of the cpu\n /load to find out current cpu load\n / (empty) to see this page";

    Output_Format output_format = OF_UNKNOWN;



    // Server setup.
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    server_address.sin_addr.s_addr = INADDR_ANY;
    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    listen(server_socket, 10);

    // Request analysis variables.
    char analyzer[1024];
    char * current_token;
    char * found_string;

    // Server service loop.
    int client_socket;
    while (run) {
        // Accepting a connection.
        client_socket = accept(server_socket, NULL, NULL);

        // Waiting for a message.
        // Flags = MSG_DONTWAIT ... nonblocking version.
        // Returns -1 if no message arives.
        int message_length = recv(client_socket, buffer, 1023, 0);

        // To prevent sending messages on CTRL+C.
        if (run == 0) {
            close(client_socket);
            break;
        }

        printf("###\n###\n Got a message:\n%s\n", buffer);

        memcpy(analyzer, buffer, message_length);

        if ((found_string = strstr(buffer, "*/*")) != NULL)
            output_format = OF_TP;
        else if ((found_string = strstr(buffer, response_TP)) != NULL)
            output_format = OF_TP;
        else if ((found_string = strstr(buffer, response_AJ)) != NULL)
            output_format = OF_AJ;
        else
            output_format = OF_UNKNOWN;

        current_token = strtok(analyzer, " \n");
        if (output_format == OF_TP) {
            // If the request is GET request
            if ((found_string = strstr(current_token, "GET")) != NULL) {

                // Analyzing object path
                current_token = strtok(NULL, " \n");

                // Hostname
                if ((found_string = strstr(current_token, "/hostname")) != NULL) {
                    snprintf(response, sizeof(response), "%s%s%s%ld%s%s", response_OK, response_TP, response_CL, strlen(hostname), response_NL,  hostname);

                // CPU Name
                } else if ((found_string = strstr(current_token, "/cpu-name")) != NULL) {
                    snprintf(response, sizeof(response), "%s%s%s%ld%s%s", response_OK, response_TP, response_CL, strlen(cpuname), response_NL,  cpuname);

                // CPU Load
                } else if ((found_string = strstr(current_token, "/load")) != NULL) {
                    cpuload = get_load();
                    snprintf(cpuload_string, sizeof(cpuload_string), "%d %%", cpuload);
                    snprintf(response, sizeof(response), "%s%s%s%ld%s%s", response_OK, response_TP, response_CL, strlen(cpuload_string), response_NL, cpuload_string);

            // Else
                } else {
                    snprintf(response, sizeof(response), "%s%s%s%ld%s%s", response_OK, response_TP, response_CL, sizeof(response_default) - 1, response_NL, response_default);
                }
            } else {
                snprintf(response, sizeof(response), "%s%s%s%ld%s%s", response_OK, response_TP, response_CL, sizeof(response_default) - 1, response_NL, response_default);
            }
        } else if (output_format == OF_AJ) {
            // If the request is GET request
            if ((found_string = strstr(current_token, "GET")) != NULL) {

                // Analyzing object path
                current_token = strtok(NULL, " \n");

                // Hostname
                if ((found_string = strstr(current_token, "/hostname")) != NULL) {
                    snprintf(response, sizeof(response), "%s%s%s%ld%s{\"hostname\": %s}", response_OK, response_AJ, response_CL, strlen(hostname) + 14, response_NL,  hostname);

                // CPU Name
                } else if ((found_string = strstr(current_token, "/cpu-name")) != NULL) {
                    snprintf(response, sizeof(response), "%s%s%s%ld%s{\"cpu-name\": %s}", response_OK, response_AJ, response_CL, strlen(cpuname) + 14, response_NL,  cpuname);

                // CPU Load
                } else if ((found_string = strstr(current_token, "/load")) != NULL) {
                    cpuload = get_load();
                    snprintf(cpuload_string, sizeof(cpuload_string), "%d %%", cpuload);
                    snprintf(response, sizeof(response), "%s%s%s%ld%s{\"load\": %s}", response_OK, response_AJ, response_CL, strlen(cpuload_string) + 10, response_NL, cpuload_string);

            // Else
                } else {
                    snprintf(response, sizeof(response), "%s%s%s%ld%s\"%s\"", response_OK, response_AJ, response_CL, sizeof(response_default) + 1, response_NL, response_default);
                }
            } else {
                snprintf(response, sizeof(response), "%s%s%s%ld%s\"%s\"", response_OK, response_AJ, response_CL, sizeof(response_default) + 1, response_NL, response_default);
            }
        } else {
            snprintf(response, sizeof(response), "HTTP/1.1 406 Not Acceptable\r\n\r\n");
        }


        send(client_socket, response, strlen(response), 0);
        printf("###\n###\n Sent a message:\n%s\n", response);
        close(client_socket);
    }
    close(server_socket);
    free(buffer);
}

void small_server(const int port_number) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Buffer for received messages
    buffer = malloc(1024*sizeof(char));

    // Buffer for building the response
    char response[256];

    // Geting name of Host
    char hostname[64];
    gethostname(hostname, sizeof(hostname) - 1);

    // Geting name of CPU
    char cpuname[64];
    FILE * cpuinfo_fp = fopen("/proc/cpuinfo", "r");
    int read_n = 0;
    while ((read_n = fscanf(cpuinfo_fp, "%s", cpuname)) == 1) {
        if (strstr(cpuname, "model") != NULL) {
            fscanf(cpuinfo_fp, "%s", cpuname);
            if (strstr(cpuname, "name") != NULL)
                break;
        }
    }
    fscanf(cpuinfo_fp, "%s", cpuname);
    getc(cpuinfo_fp);
    fgets(cpuname, 64, cpuinfo_fp);

    // Getting cpuload (BLOCKING)
    int cpuload = get_load();
    printf("CPULOAD %d\n", cpuload);
    char cpuload_string[14];

    snprintf(cpuload_string, sizeof(cpuload_string), "%d %%", cpuload);

    // Construct universal parts of response
    const char * response_OK = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: ";
    const char * response_NL = "\r\n\r\n";

    snprintf(response, sizeof(response), "%s%ld%s%s", response_OK, strlen(cpuload_string), response_NL, cpuload_string);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    listen(server_socket, 10);

    int client_socket;
    while (run) {
        client_socket = accept(server_socket, NULL, NULL);
        recv(client_socket, buffer, 1023, 0);
        if (run == 0) {
            close(client_socket);
            break;
        }
        printf("###\n###\n Got a message:\n%s\n", buffer);

        cpuload = get_load();
        printf("CPULOAD %d\n", cpuload);
        char cpuload_string[12];
        snprintf(cpuload_string, sizeof(cpuload_string), "%d", cpuload);
        snprintf(response, sizeof(response), "%s%ld%s%s", response_OK, strlen(cpuload_string), response_NL, cpuload_string);

        send(client_socket, response, strlen(response), 0);
        printf("###\n###\n Sent a message:\n%s\n", response);
        close(client_socket);
    }
    free(buffer);
}

int get_load() {
    FILE * proc_stat_fp;
    char voidhole[16];

    long double user_old,       user;
    long double nice_old,       nice;
    long double system_old,     system;
    long double idle_old,       idle;
    long double iowait_old,     iowait;
    long double irq_old,        irq;
    long double softirq_old,    softirq;
    long double steal_old,      steal;
    long double guest_old,      guest;
    long double guest_nice_old, guest_nice;

    long double result;

    proc_stat_fp =  fopen("/proc/stat", "r");
    fscanf(proc_stat_fp, "%s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf", voidhole, &user_old, &nice_old, &system_old, &idle_old, &iowait_old, &irq_old, &softirq_old, &steal_old, &guest_old, &guest_nice_old );
    fclose(proc_stat_fp);

    sleep(1);

    proc_stat_fp =  fopen("/proc/stat", "r");
    fscanf(proc_stat_fp, "%s %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf", voidhole, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice );
    fclose(proc_stat_fp);

    long double nidle_old = idle_old + iowait_old;
    long double nidle = idle + iowait;

    long double nnonidle_old = user_old + nice_old + system_old + irq_old + softirq_old + steal_old;
    long double nnonidle = user + nice + system + irq + softirq + steal;

    long double ntotal_old = nidle_old + nnonidle_old;
    long double ntotal = nidle + nnonidle;

    long double totald = ntotal - ntotal_old;
    long double idled = nidle - nidle_old;

    result = (totald - idled) / totald;
    return (int)(result * 100);
}

void close_handler(int dummy) {
    int x = dummy * 0;
    run = x;
}

void error(int error_code, const char * error_message) {
    if (buffer != NULL) {
        printf("Freeing buffer.\n");
        free(buffer);
    }

    fprintf(stderr, "Error %d!\n%s\n\n", error_code, error_message);
    exit(error_code);
}

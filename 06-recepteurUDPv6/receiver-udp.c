#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP   "::1"
#define SIZE 100

void usage(char* message){
    fprintf(stderr, "Usage: ./sebder-udp port_number");
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

long cook_port_number(char* str_port, int* int_port){
    char* endptr;
    long port = strtol(str_port, &endptr, 10); 
    if (errno){
        perror("strtol error");
        exit(EXIT_FAILURE);
    }
    if (!(strcmp(endptr, "\0") == 0) || 
        !(port >= 10000 && port <= 65000)){
        return -1;
    }
    *int_port = port;
    return 0;
}

int main (int argc, char *argv [])
{
    /* test arg number */
    if (argc != 2){
        usage("");
    }

    /* convert and check port number */
    int port_number;
    if(cook_port_number(argv[1], &port_number) == -1){
        usage("10000 <= port_number <= 65000");
    };
    char* str_port_number = argv[1];

    /* create socket */
    int udp_socket;
    CHECK(udp_socket = socket(AF_INET6, SOCK_DGRAM, 0));
    
    /* complete struct sockaddr */

    /* link socket to local IP and PORT */

    /* wait for incoming message */
    
    /* print sender addr and port */

    /* close socket */

    /* free memory */

    return 0;
}

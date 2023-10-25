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

#define IP "127.0.0.1"

void usage(){
    fprintf(stderr, "usage: ./receiver-tcp ip_addr port_number");
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
    if (argc != 3){
        usage();
    }

    /* convert and check port number */
    int port_number;
    if(cook_port_number(argv[2], &port_number) == -1){
        usage();
    };
    char* ip_address = argv[1];
    char* str_port_number = argv[2];
    
    /* create socket */
    int tcp_socket;
    CHECK(tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

    /* SO_REUSEADDR option allows re-starting the program without delay */
    int iSetOption = 1;
    CHECK (setsockopt (tcp_socket, SOL_SOCKET, SO_REUSEADDR, &iSetOption, sizeof iSetOption));

    /* complete struct sockaddr */

    /* connect to the remote peer */

    /* send the message */

    /* close socket */

    /* free memory */

    return 0;
}

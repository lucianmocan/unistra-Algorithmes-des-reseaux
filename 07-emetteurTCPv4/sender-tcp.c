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

    /* complete sockaddr struct */
    struct addrinfo *ai;
    struct addrinfo hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    int error = getaddrinfo(IP, str_port_number, &hints, &ai);
    if (error){
	    errx(1, "%s", gai_strerror(error));
    };

    /* connect to the remote peer */
    CHECK(connect (tcp_socket, ai->ai_addr, ai->ai_addrlen));

    /* send the message */

    /* close socket */

    /* free memory */

    return 0;
}

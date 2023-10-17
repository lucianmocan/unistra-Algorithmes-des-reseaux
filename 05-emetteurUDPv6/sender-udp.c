#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <err.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP "::1"

void usage(){
    fprintf(stderr, "usage: ./sender-udp ip_addr port_number\n");
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
    char* str_port_number = argv[2];
    char* ip_address = argv[1];

    /* create socket */
    int udp_socket;
    CHECK(udp_socket = socket(AF_INET6, SOCK_DGRAM, 0));

    /* fill in dest IP and PORT */
    struct sockaddr_storage ss;
    struct sockaddr_in6 *in6 = (struct sockaddr_in6 *) &ss;
    in6->sin6_family = AF_INET6;
    in6->sin6_port = htons(port_number);

    struct addrinfo *ai;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    int error = getaddrinfo(ip_address, str_port_number, &hints, &ai);
    if (error){
	    errx(1, "%s", gai_strerror(error));
    };

    memcpy(in6, ai->ai_addr, ai->ai_addrlen);

    /* send message to remote peer */
    ssize_t n;
    const char* message = "hello world";
    int message_length = strlen(message);

    CHECK(n = sendto(udp_socket, message, message_length, 0, (struct sockaddr*)&ss, ai->ai_addrlen));
    if (n != message_length) {
        exit(EXIT_FAILURE);
    }

    /* close socket */
    CHECK(close(udp_socket));

    /* free memory */
    freeaddrinfo(ai);

    return 0;
}

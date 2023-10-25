#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <err.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP   "127.0.0.1"
#define SIZE 100
#define QUEUE_LENGTH 1

void usage(){
    fprintf(stderr, "usage: ./receiver-tcp ip_addr port_number\n");
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
    CHECK (setsockopt (tcp_socket, SOL_SOCKET, SO_REUSEADDR, &iSetOption,
		       sizeof iSetOption));

    /* complete struct sockaddr */
    struct addrinfo *ai;
    struct addrinfo hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    int error = getaddrinfo(ip_address, str_port_number, &hints, &ai);
    if (error){
	    errx(1, "%s", gai_strerror(error));
    };

    /* link socket to local IP and PORT */
    CHECK(bind(tcp_socket, ai->ai_addr, ai->ai_addrlen));

    /* set queue limit for incoming connections */
    CHECK(listen (tcp_socket, SIZE));

    /* wait for incoming TCP connection */
    struct sockaddr_storage ss;
    socklen_t ss_len = sizeof(ss);
    int acc_socket;
    CHECK(acc_socket = accept(tcp_socket, (struct sockaddr*)&ss, &ss_len));

    /* print sender addr and port */
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    if (getnameinfo((struct sockaddr*)&ss, ss_len, hbuf, sizeof(hbuf), 
        sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)){
            errx(1, "could not get numeric hostname");
        };
    printf("%s %s\n", hbuf, sbuf);

    /* wait for incoming message */
    char message[SIZE];
    message[SIZE] = '\0';
    CHECK(recv (acc_socket, message, SIZE, MSG_PEEK));

    /* close sockets */
    CHECK(close(acc_socket));
    CHECK(close(tcp_socket));

    /* free memory */
    freeaddrinfo(ai);

    /* print received message */
    printf("%s", message);

    return 0;
}

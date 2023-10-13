#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <err.h>
#include <errno.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP   "127.0.0.1"
#define SIZE 100

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


int checkIPv4address(char *ip_address){
    struct in_addr ss;
    int check = inet_pton(AF_INET, ip_address, &(ss.s_addr));
    CHECK(check);
    return check;
}

int main (int argc, char *argv [1])
{
    /* test arg number */
    if (argc != 3){
        fprintf(stderr, "usage: ./receiver-udp ip_addr port_number\n");
        exit(EXIT_FAILURE);
    }

    /* convert and check port number */
    int port_number;
    if(cook_port_number(argv[2], &port_number) == -1){
        fprintf(stderr, "usage: ./receiver-udp ip_addr port_number\n");
        exit(EXIT_FAILURE);
    };
    char* str_port_number = argv[2];

    // if (!checkIPv4address(argv[2])){
    //     fprintf(stderr, "usage: ./receiver-udp ip_addr port_number\n");
    //     exit(EXIT_FAILURE);
    // }
    char* ip_address = argv[1];

    /* create socket */
    int udp_socket;
    CHECK(udp_socket = socket(AF_INET, SOCK_DGRAM, 0));

    /* complete struct sockaddr */
    struct addrinfo *ai;
    struct addrinfo hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    int error = getaddrinfo(ip_address, str_port_number, &hints, &ai);
    if (error){
	    errx(1, "%s", gai_strerror(error));
    };

    /* link socket to local IP and PORT */
    CHECK(bind(udp_socket, ai->ai_addr, ai->ai_addrlen));

    /* wait for incoming message */
    char message[SIZE];
    memset(message, 0, SIZE);
    struct sockaddr_storage address;
    socklen_t address_len = sizeof(address);

    CHECK(recvfrom(udp_socket, message, sizeof(message)-1, MSG_PEEK, 
        (struct sockaddr*)&address, &address_len));
    printf("%s", message);


    /* print sender addr and port */
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    if (getnameinfo((struct sockaddr*)&address, address_len, hbuf, sizeof(hbuf), 
        sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)){
            errx(1, "could not get numeric hostname");
        };
    printf("%s %s\n", hbuf, sbuf);

    /* close socket */
    CHECK(close(udp_socket));

    /* free memory */
    freeaddrinfo(ai);

    return 0;
}

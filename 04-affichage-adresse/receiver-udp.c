#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <err.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP   "127.0.0.1"
#define SIZE 100

int main (int argc, char *argv [1])
{
    /* test arg number */
    if (argc < 2){
        fprintf(stderr, "Usage: ./receiver-udp port_number");
        exit(EXIT_FAILURE);
    }
    /* convert and check port number */
    int port_number = atoi(argv[1]);
    char *port_number_str = argv[1];
    
    if (port_number == 0){
        fprintf(stderr, "erreur atoi\n");
        exit(EXIT_FAILURE);
    }

    if (!(port_number >= 10000 && port_number <= 65000)){
        fprintf(stderr, "Numero de port doit etre contenu dans l'intervalle [10000, 65000]\n");
        exit(EXIT_FAILURE);
    }

    /* create socket */
    int udp_socket;
    CHECK(udp_socket = socket(AF_INET, SOCK_DGRAM, 0));

    /* complete struct sockaddr */
    struct addrinfo *ai;
    struct addrinfo hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_ADDRCONFIG;

    int error = getaddrinfo(IP, port_number_str, &hints, &ai);
    if (error){
	    errx(1, "%s", gai_strerror(error));
    };

    /* link socket to local IP and PORT */
    CHECK(bind(udp_socket, ai->ai_addr, ai->ai_addrlen));

    /* wait for incoming message */
    char message[SIZE];
    bzero(message, SIZE);
    struct sockaddr_storage address;
    socklen_t address_len;

    ssize_t bytes;
    CHECK(bytes = recvfrom(udp_socket, message, SIZE, MSG_PEEK, 
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

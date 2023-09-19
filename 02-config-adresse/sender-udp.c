#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <err.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP   "127.0.0.1"

int main (int argc, char *argv [])
{
    /* test arg number */
    if (argc < 2) {
        fprintf(stderr, "Usage: ./sender-udp port_number\n");
        exit(EXIT_FAILURE);
    }

    /* convert and check port number */
    int port_number = atoi(argv[1]);
    char* port_number_str = argv[1];
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

    /* complete sockaddr struct */
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

    /* send message to remote peer */

    ssize_t n;
    char* message = "hello world";
    int message_length = 12;

    CHECK(n = sendto(udp_socket, message, message_length, 0, ai->ai_addr, sizeof(*(ai->ai_addr))));
    if (n != message_length) {
        exit(EXIT_FAILURE);
    }

    /* close socket */
    CHECK(close(udp_socket));

    /* free memory */
    freeaddrinfo(ai);

    return 0;
}

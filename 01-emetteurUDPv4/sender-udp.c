#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <inttypes.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP      0x100007f /* 127.0.0.1 */
#define PORT(p) htons(p)

int main (int argc, char *argv [])
{
    /* test arg number */
    if (argc < 2) {
        fprintf(stderr, "Usage: ./sender-udp port_number\n");
        exit(EXIT_FAILURE);
    }

    /* convert and check port number */
    int port_number = atoi(argv[1]);
    if (port_number == 0){
        fprintf(stderr, "erreur atoi\n");
        exit(EXIT_FAILURE);
    }

    if (!(port_number >= 10000 && port_number <= 65000)){
        fprintf(stderr, "Numero de port doit etre contenu dans l'intervalle [10000, 65000]\n");
        exit(EXIT_FAILURE);
    }

    /* create socket */

    /* complete sockaddr struct */

    /* send message to remote peer */

    /* close socket */

    return 0;
}

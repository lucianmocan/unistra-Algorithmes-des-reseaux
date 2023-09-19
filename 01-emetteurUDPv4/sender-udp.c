#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <inttypes.h>
#include <netinet/udp.h>

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
    int udp_socket;
    CHECK(udp_socket = socket(AF_INET, SOCK_DGRAM, 0));

    /* complete sockaddr struct */

    struct sockaddr_storage ss;
    struct sockaddr_in *in = (struct sockaddr_in *) &ss;
    ss.sin_family = AF_INET;
    ss.sin_port = PORT(port_number);
    ss.sin_addr = IP;
    ss.sin_zero = 0;


    /* send message to remote peer */


    /* close socket */

    return 0;
}

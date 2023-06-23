#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP   "127.0.0.1"
#define SIZE 100
#define QUEUE_LENGTH 1

int main (int argc, char *argv [])
{
    /* test arg number */

    /* convert and check port number */

    /* create socket */

    /* SO_REUSEADDR option allows re-starting the program without delay */
    int iSetOption = 1;
    CHECK (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &iSetOption,
		       sizeof iSetOption));

    /* complete struct sockaddr */

    /* link socket to local IP and PORT */

    /* set queue limit for incoming connections */

    /* wait for incoming TCP connection */

    /* print sender addr and port */

    /* wait for incoming message */

    /* close sockets */

    /* free memory */

    /* print received message */

    return 0;
}

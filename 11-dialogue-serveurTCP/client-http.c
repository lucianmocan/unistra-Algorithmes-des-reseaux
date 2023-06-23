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

#define SIZE 100

void cpy (int src, int dst)
{
    return;
}

int main (int argc, char * argv[])
{
    /* test arg number */

    /* get the list of struct addrinfo */

    /* create socket */

    /* connect to the server */

    /* prepare GET cmd */

    /* send GET cmd and get the response */
    
    /* close socket */

    /* free struct addrinfo list */

    return 0;
}

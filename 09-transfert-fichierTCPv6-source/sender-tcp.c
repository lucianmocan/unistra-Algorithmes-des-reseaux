#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP   "::1"
#define SIZE 100

void cpy (int src, int dst)
{
    return;
}

int main (int argc, char *argv [])
{
    /* test arg number */

    /* convert and check port number */

    /* open file to send */

    /* create socket */

    /* complete struct sockaddr */

    /* connect to the remote peer */

    /* send the file content */

    /* close socket */

    /* close file */

    /* free memory */

    return 0;
}

#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <err.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP   "::1"
#define SIZE 100

void usage(){
    fprintf(stderr, "./sender-udp ip_addr port_number filename\n");
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

void cpy (int src, int dst)
{
    return;
}

int main (int argc, char *argv [])
{
    /* test arg number */
    if (argc != 4){
        usage();
    }

    /* convert and check port number */
    int port_number;
    if(cook_port_number(argv[2], &port_number) == -1){
        usage();
    };
    char* ip_address = argv[1];
    char* str_port_number = argv[2];
    char* filename = argv[3];

    /* open file to send */
    int fd;
    CHECK(fd = open(filename, O_RDONLY, 0444));

    /* create socket */

    /* complete struct sockaddr */

    /* connect to the remote peer */

    /* send the file content */

    /* close socket */

    /* close file */

    /* free memory */

    return 0;
}

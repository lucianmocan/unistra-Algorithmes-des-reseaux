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
#include <stdnoreturn.h>
#include <stdarg.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define IP   "::1"
#define SIZE 100

noreturn void raler (int syserr, const char* msg, ...){
	va_list ap;

	va_start (ap, msg);
	vfprintf (stderr, msg, ap);
	fprintf (stderr, "\n");
	va_end (ap);

	if (syserr == 1)
		perror ("");

	exit(EXIT_FAILURE);
}


void usage(){
    fprintf(stderr, "usage: ./sender-tcp ip_addr port_number filename\n");
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
    char buffer[BUFSIZ];
    ssize_t n;
	while ((n = read(src, buffer, BUFSIZ)) > 0){
		if (!(write(dst, buffer, n) == n)){
			raler(errno, "write");
		};
	}
	if (n == -1){
		raler(errno, "read");
	}
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
    int tcp_socket;
    CHECK(tcp_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP));

    /* complete struct sockaddr */
    struct addrinfo *ai;
    struct addrinfo hints;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    int error = getaddrinfo(ip_address, str_port_number, &hints, &ai);
    if (error){
	    errx(1, "%s", gai_strerror(error));
    };

    /* connect to the remote peer */
    CHECK(connect (tcp_socket, ai->ai_addr, ai->ai_addrlen));

    /* send the file content */
    cpy(fd, tcp_socket);

    /* close socket */
    CHECK(close(tcp_socket));

    /* close file */
    CHECK(close(fd));

    /* free memory */
    freeaddrinfo(ai);

    return 0;
}

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
#define QUEUE_LENGTH 1

void usage(){
    fprintf(stderr, "usage: ./receiver-tcp port_number\n");
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
    if (argc != 2){
        usage();
    }
    
    /* convert and check port number */
    int port_number;
    if(cook_port_number(argv[1], &port_number) == -1){
        usage();
    };
    char* str_port_number = argv[1];

    /* create socket */
    int tcp_socket;
    CHECK(tcp_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP));

    /* SO_REUSEADDR option allows re-starting the program without delay */
    int iSetOption = 1;
    CHECK (setsockopt (tcp_socket, SOL_SOCKET, SO_REUSEADDR, &iSetOption,
		       sizeof iSetOption));

    /* complete struct sockaddr */
    struct addrinfo *ai;
    struct addrinfo hints;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

    int error = getaddrinfo(IP, str_port_number, &hints, &ai);
    if (error){
	    errx(1, "%s", gai_strerror(error));
    };

    /* link socket to local IP and PORT */
    CHECK(bind(tcp_socket, ai->ai_addr, ai->ai_addrlen));

    /* set queue limit for incoming connections */
    CHECK(listen (tcp_socket, SIZE));

    /* wait for incoming TCP connection */
    struct sockaddr_storage ss;
    socklen_t ss_len = sizeof(ss);
    int acc_socket;
    CHECK(acc_socket = accept(tcp_socket, (struct sockaddr*)&ss, &ss_len));

    /* open local file */
    int fd;
    CHECK(fd = open("copy.tmp", O_RDWR | O_CREAT | O_TRUNC, 0666));

    /* get the transmitted file */
    cpy(acc_socket, fd);

    /* close sockets */
    CHECK(close(tcp_socket));
    CHECK(close(acc_socket));

    /* close file */
    CHECK(close(fd));

    /* free memory */
    freeaddrinfo(ai);

    return 0;
}

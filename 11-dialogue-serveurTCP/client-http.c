#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <err.h>
#include <stdnoreturn.h>
#include <stdarg.h>
#include <errno.h>

#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)

#define SIZE 100
#define SERVNAME "http"

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
    fprintf(stderr, "usage: ./client-http server_name\n");
    exit(EXIT_FAILURE);
}

int make_cmd(char *string, char* server_name){
    char *f = "GET / HTTP/1.1\r\nHost: ";
    char *e = "\r\nConnection:close\r\n\r\n";
    int sn = snprintf(string, BUFSIZ, "%s%s%s", f, server_name, e);
    if (sn < 0 || sn >= BUFSIZ) return -1;
    return 0;
}

void print_message (int src)
{   
    char buffer[BUFSIZ];
    ssize_t n;
	while ((n = read(src, buffer, BUFSIZ)) > 0){
        buffer[n]='\0';
        fprintf(stdout, "%s", buffer);
	}
	if (n == -1){
		raler(errno, "read");
	}
    return;
}

int main (int argc, char * argv[])
{
    /* test arg number */
    if (argc != 2){
        usage();
    }
    char* server_name = argv[1];

    /* get the list of struct addrinfo */
    struct addrinfo *ai;
    struct addrinfo hints;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    int error = getaddrinfo(server_name, "http", &hints, &ai);
    if (error){
	    errx(1, "%s", gai_strerror(error));
    };

    /* create socket */
    int tcp_socket;
    CHECK(tcp_socket = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol));

    /* connect to the server */
    CHECK(connect (tcp_socket, ai->ai_addr, ai->ai_addrlen));

    /* prepare GET cmd */
    char cmd[BUFSIZ];
    CHECK(make_cmd(cmd, server_name)); 

    /* send GET cmd and get the response */
    ssize_t n;
    int message_length = strlen(cmd);
    CHECK(n = send (tcp_socket, cmd, message_length, 0));
    if (n != message_length) {
        exit(EXIT_FAILURE);
    }

    /* get the transmitted file */
    print_message(tcp_socket);


    /* close socket */
    CHECK(close(tcp_socket));

    /* free struct addrinfo list */
    freeaddrinfo(ai);
    return 0;
}

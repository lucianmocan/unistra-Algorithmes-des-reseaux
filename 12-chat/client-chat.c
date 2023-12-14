#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <poll.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <stdnoreturn.h>


#define CHECK(op)   do { if ( (op) == -1) { perror (#op); exit (EXIT_FAILURE); } \
                    } while (0)
#define CHKN(op)                                                               \
    do                                                                         \
    {                                                                          \
        if ((op) == NULL) {                                                     \
            perror(#op); exit(EXIT_FAILURE);}                                   \
    } while (0)

#define PORT(p) htons(p)

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
    fprintf(stderr, "usage: ./client-chat port_number\n");
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

void send_message(
                char *message, 
                int fd, 
                struct sockaddr *dest_addr, 
                socklen_t dest_len
                ){
    ssize_t n;
    int message_length = strlen(message);
    CHECK(n = sendto(fd, message, message_length, 0, dest_addr, dest_len));
    if (n != message_length) {
        exit(EXIT_FAILURE);
    }
}

typedef struct recv_message{
    char *message;
    struct sockaddr_storage *address;
} recv_message;

recv_message* receive_message(int udp_socket){
    char *message; 
    CHKN(message = calloc(BUFSIZ+1, sizeof(char)));

    struct sockaddr_storage *address; 
    CHKN(address = calloc(1, sizeof(struct sockaddr_storage)));
    socklen_t address_len = sizeof(*address);
    
    ssize_t n;
    CHECK(n = recvfrom(udp_socket, message, BUFSIZ, 0, (struct sockaddr*)address, &address_len));
    message[n] = '\0';

    recv_message *rm;
    CHKN(rm = malloc(sizeof(recv_message)));
    rm->address = address;
    rm->message = message;
    return rm;
}

void receive_message_free(recv_message *rm){
    free(rm->message);
    free(rm->address);
    free(rm);
}

typedef enum status {
    CONNECTED, 
    QUIT, 
    UNKNOWN
} status;

void display_remote_info(struct sockaddr_storage* ss){
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    if (getnameinfo((struct sockaddr*)ss, sizeof(*ss), hbuf, sizeof(hbuf), 
        sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)){
            fprintf(stderr, "could not get numeric hostname\n");
            exit(EXIT_FAILURE);
        };
    printf("%s %s\n", hbuf, sbuf);
}

status deal_with_message(recv_message *rm){
    if (strcmp(rm->message, "/HELO\n") == 0){
        display_remote_info(rm->address);
        return CONNECTED;
    }
    else if (strcmp(rm->message, "/QUIT\n") == 0){
        return QUIT;
    }
    else {
        return UNKNOWN;
    }
}


int main (int argc, char *argv [])
{
    /* test arg number */
    if (argc != 2){
        usage();
    }
    /* convert and check port number */
    int port_number;
    if (cook_port_number(argv[1], &port_number) == -1){
        usage();
    }
    char* str_port_number = argv[1];

    /* create socket */
    int udp_socket;
    CHECK(udp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP));

    /* set dual stack socket */
    int value = 0;
    CHECK(setsockopt(udp_socket, IPPROTO_IPV6, IPV6_V6ONLY, &value, sizeof value));

    struct addrinfo *ai;
    struct addrinfo hints;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    int error = getaddrinfo(NULL, str_port_number, &hints, &ai);
    if (error){
        fprintf(stderr, "%s\n", gai_strerror(error));
	    exit(EXIT_FAILURE);
    };

    /* check if a client is already present */
    int reuseaddr = 1;
    CHECK(setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)));
    CHECK(bind(udp_socket, ai->ai_addr, ai->ai_addrlen));
    
    /* prepare struct pollfd with stdin and socket for incoming data */
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = udp_socket;
    fds[1].events = POLLIN;

    /* main loop */
    int functional = 1;
    while (functional){
        CHECK(poll(fds, sizeof(fds), -1));
        if(fds[1].revents & POLLIN){
            recv_message *rm = receive_message(fds[1].fd);
            status s = deal_with_message(rm);
            switch(s){
                case CONNECTED:
                        break;
                case QUIT:
                        functional = 0;
                        break;
                default:{;}
            }
            // traitement
            // instructions
            receive_message_free(rm);
        }
    }

    /* close socket */
    CHECK(close(udp_socket));

    /* free memory */
    freeaddrinfo(ai);
    return 0;
}

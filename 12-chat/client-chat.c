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

#define STRING_SIZE BUFSIZ + 1
#define START "/HELO"
#define END "/QUIT"

typedef enum status {
    CONNECTED, 
    QUIT, 
    DATA
} status;

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

char* string_create(int size){
    char* buffer;
    CHKN(buffer = calloc(size, sizeof(char)));
    return buffer;
}

char* string_enlarge(char* string, int size){
    CHKN(string = realloc(string, size * sizeof(char)));
    return string;
}

void string_delete(char* string){
    free(string);
}

void display_remote_info(struct sockaddr_storage* ss){
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    if (getnameinfo((struct sockaddr*)ss, sizeof(*ss), hbuf, sizeof(hbuf), 
        sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)){
            fprintf(stderr, "could not get numeric hostname\n");
            exit(EXIT_FAILURE);
        };
    printf("%s %s\n", hbuf, sbuf);
}


void send_message(char *message, int fd, struct sockaddr *dest_addr, socklen_t dest_len){
    ssize_t n;
    int message_length = strlen(message);
    CHECK(n = sendto(fd, message, message_length, 0, dest_addr, dest_len));
    if (n != message_length) {
        exit(EXIT_FAILURE);
    }
}

typedef struct recv_msg{
    char *message;
    struct sockaddr_storage *address;
} recv_msg;

recv_msg* receive_message(int udp_socket){
    char *message = string_create(STRING_SIZE);

    struct sockaddr_storage *address; 
    CHKN(address = calloc(1, sizeof(struct sockaddr_storage)));
    socklen_t address_len = sizeof(*address);
    
    ssize_t n;
    CHECK(n = recvfrom(udp_socket, message, STRING_SIZE - 1, 0, (struct sockaddr*)address, &address_len));
    message[n] = '\0';

    recv_msg *rm;
    CHKN(rm = malloc(sizeof(recv_msg)));
    rm->address = address;
    rm->message = message;
    return rm;
}

char* read_user_input(int fd, char* buffer){
	ssize_t n, buffer_size = STRING_SIZE, read_bytes = 0;

	while ((n = read(fd, buffer + read_bytes, STRING_SIZE - 1)) > 0){ // don't forget to do BUFSIZ + 1 when creating the string
        read_bytes += n;
        if (read_bytes == buffer_size - 1){
            buffer_size += BUFSIZ;
            buffer = string_enlarge(buffer, buffer_size);
        }
	}
	if (n == -1){
		raler(errno, "read");
	}
    buffer[read_bytes] = '\0';
    return buffer;
}

status deal_with_recv_message(recv_msg *rm){
    if (strncmp(rm->message, START, sizeof(START)) == 0){
        display_remote_info(rm->address);
        return CONNECTED;
    }
    else if (strncmp(rm->message, END, sizeof(END)) == 0){
        return QUIT;
    }
    return DATA;
}

status deal_with_input_message(char* user_input){
    if (strncmp(user_input, END, sizeof(END)) == 0){
        return QUIT;
    }
    return DATA;
}

status input_cmd_check(int fd, char* buffer){
    buffer = read_user_input(fd, buffer);
    status st = deal_with_input_message(buffer);
    return st;
}

void receive_message_free(recv_msg *rm){
    free(rm->message);
    free(rm->address);
    free(rm);
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

    /* create socket */
    int udp_socket;
    CHECK(udp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP));

    /* set dual stack socket */
    int value = 0;
    CHECK(setsockopt(udp_socket, IPPROTO_IPV6, IPV6_V6ONLY, &value, sizeof value));

    struct sockaddr_storage ss = {0};
    struct sockaddr *s = (struct sockaddr *)&ss;
    struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)&ss;
    in6->sin6_addr = in6addr_any;
    in6->sin6_port = htons(port_number);
    in6->sin6_family = PF_INET6;

    /* check if a client is already present */
    if(bind(udp_socket, s, sizeof(ss)) == -1){
        if (errno == EADDRINUSE){
            char init[6] = START;
            send_message(init, udp_socket, s, sizeof(ss));
        }
        else {
            exit(EXIT_FAILURE);
        }
    }
    
    /* prepare struct pollfd with stdin and socket for incoming data */
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = udp_socket;
    fds[1].events = POLLIN;

    /* main loop */
    int functional = 1;
    while (functional){
        CHECK(poll(fds, 2, -1));
        // wait for /HELO but also deal with /QUIT
        if(fds[1].revents & POLLIN){
            recv_msg *rm = receive_message(fds[1].fd);
            status s = deal_with_recv_message(rm);
            switch(s){
                case CONNECTED:
                        break;
                case QUIT:
                        functional = 0;
                        break;
                case DATA:
                        // printf("%s", rm->message);
                        break;
                default:{;}
            }
            receive_message_free(rm);
        }
        // user input from stdin
        if (fds[0].revents & POLLIN){
            char* buffer = string_create(STRING_SIZE);
            status st = input_cmd_check(fds[0].fd, buffer);
            if (st == QUIT) {
                string_delete(buffer);
                send_message(END, fds[1].fd, s, sizeof(ss));
                functional = 0;
            }
            // if not a /QUIT message then it's a DATA message
            else {
                buffer = read_user_input(fds[0].fd, buffer);
                send_message(buffer, fds[1].fd, s, sizeof(ss));
                string_delete(buffer);
            }
        }
    }

    /* close socket */
    CHECK(close(udp_socket));

    /* free memory */
    // freeaddrinfo(ai);
    return 0;
}

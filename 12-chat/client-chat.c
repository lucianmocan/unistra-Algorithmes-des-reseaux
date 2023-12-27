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
#define HELLO "/HELO"
#define QUIT "/QUIT"
#define CMDSIZE 6

typedef enum Status {
    UNKNOWN,
    CONNECTED, 
    DISCONNECTED
} Status;

typedef enum Event {
    saysHELLO, 
    saysQUIT, 
    saysDATA
} Event;

int connectionStatus = UNKNOWN;

long cookPortNumber(char* str_port, int* int_port){
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

void displayRemoteInfo(struct sockaddr_storage* ss){
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    if (getnameinfo((struct sockaddr*)ss, sizeof(*ss), hbuf, sizeof(hbuf), 
        sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV)){
            fprintf(stderr, "could not get numeric hostname\n");
            exit(EXIT_FAILURE);
        };
    printf("%s %s\n", hbuf, sbuf);
}

char* string_create(int size){
    char* buffer;
    CHKN(buffer = calloc(size, sizeof(char)));
    return buffer;
}

void string_delete(char* string){
    free(string);
}

void sendMessage(char *message, int fd, struct sockaddr_in6* in6, struct sockaddr_storage* ss){
    ssize_t n;
    socklen_t dest_len = sizeof(*ss);
    int message_length = strlen(message);
    CHECK(n = sendto(fd, message, message_length, 0, (struct sockaddr*)in6, dest_len));
    if (n != message_length) {
        exit(EXIT_FAILURE);
    }
}

char* receiveMessage(int udp_socket, struct sockaddr_in6* in6, struct sockaddr_storage* ss){
    char *message = string_create(BUFSIZ + 1);
    socklen_t address_len = sizeof(*ss);
    ssize_t n;
    CHECK(n = recvfrom(udp_socket, message, BUFSIZ, 0, (struct sockaddr*)in6, &address_len));
    message[n] = '\0';

    return message;
}

void destroyMessage(char* message){
    string_delete(message);
}

char* readUserInput(int fd){
    ssize_t n;
    char* buffer = string_create(BUFSIZ + 1);
    CHECK(n = read(fd, buffer, BUFSIZ));
    buffer[n] = '\0';
    return buffer;
}

Event getSocketMessageEvent(char* message){
    if (strncmp(message, HELLO, sizeof(HELLO)) == 0){
        return saysHELLO;
    }
    if (strncmp(message, QUIT, sizeof(QUIT)) == 0){
        return saysQUIT;
    }
    return saysDATA;
}

Event getInputMessageEvent(char* message){
    if (strncmp(message, QUIT, sizeof(QUIT)-1) == 0){
        if (message[sizeof(QUIT)] == '\0')
            return saysQUIT;
    }
    return saysDATA;
}

void actionOnSocket(struct pollfd* fds, struct sockaddr_in6* in6, struct sockaddr_storage* ss){
    if(fds[1].revents & POLLIN){
        char* message = receiveMessage(fds[1].fd, in6, ss);
        Event e = getSocketMessageEvent(message);
        switch(e){
            case saysHELLO: {
                if (connectionStatus == UNKNOWN){
                    connectionStatus = CONNECTED;
                    displayRemoteInfo(ss);
                }
                break;
            } 
            case saysQUIT: {
                if (connectionStatus == CONNECTED){
                    connectionStatus = DISCONNECTED;
                }
                break;
            }
            case saysDATA: {
                if (connectionStatus == CONNECTED){
                    printf("%s", message);
                }
                break;
            }
        }
        destroyMessage(message);
    }
}

void actionOnInput(struct pollfd* fds, struct sockaddr_in6* in6, struct sockaddr_storage* ss){
    if (fds[0].revents & POLLIN){
        char* userInput = readUserInput(fds[0].fd);
        Event e = getInputMessageEvent(userInput);
        switch(e){
            case saysHELLO:
            case saysQUIT: {
                connectionStatus = DISCONNECTED;
                char cmd[CMDSIZE] = QUIT;
                sendMessage(cmd, fds[1].fd, in6, ss);
                break;
            }
            case saysDATA: {
                sendMessage(userInput, fds[1].fd, in6, ss);
                break;
            }
        }
        destroyMessage(userInput);
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
    if (cookPortNumber(argv[1], &port_number) == -1){
        usage();
    }

    /* create socket */
    int udp_socket;
    CHECK(udp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP));

    /* set dual stack socket */
    int value = 0;
    CHECK(setsockopt(udp_socket, IPPROTO_IPV6, IPV6_V6ONLY, &value, sizeof(value)));

    struct sockaddr_storage ss = {0};
    struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)&ss;
    in6->sin6_addr = in6addr_any;
    in6->sin6_port = htons(port_number);
    in6->sin6_family = PF_INET6;

    /* check if a client is already present */
    if(bind(udp_socket, (struct sockaddr*)in6, sizeof(ss)) == -1){
        // if another client is already waiting send /HELO
        if (errno == EADDRINUSE){
            char cmd[CMDSIZE] = HELLO;
            sendMessage(cmd, udp_socket, in6, &ss);
            connectionStatus = CONNECTED;
        }
        else {
            exit(EXIT_FAILURE);
        }
    }
    // if no client is present on the port then WAIT
    
    /* prepare struct pollfd with stdin and socket for incoming data */
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = udp_socket;
    fds[1].events = POLLIN;

    /* main loop */
    while (connectionStatus != DISCONNECTED){
        CHECK(poll(fds, 2, -1));
        // wait for /HELO but also deal with /QUIT and DATA
        actionOnSocket(fds, in6, &ss);
        // user input from stdin
        actionOnInput(fds, in6, &ss);
    }

    /* close socket */
    CHECK(close(udp_socket));

    return 0;
}

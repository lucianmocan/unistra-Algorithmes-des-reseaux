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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>


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
#ifdef FILEIO
#define FILETRANSFER "/FILE "
#endif
#define CMDSIZE 6

#ifdef BIN
#define HELLO_BINARY 0
#define QUIT_BINARY 1
#ifdef FILEIO
#define FILE_BINARY 2
#endif
#endif

typedef enum Status {
    UNKNOWN,
    CONNECTED, 
    DISCONNECTED
} Status;

typedef enum Event {
    saysHELLO, 
    saysQUIT, 
    saysDATA
    #ifdef FILEIO
    ,saysFILE
    #endif
} Event;

int connectionStatus = UNKNOWN;

/*
Check port number and if input is correct then return 
an integer of the string.
*/
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

/*
Display remote address and port
*/
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

void sendMessage(char *message, int udp_socket, struct sockaddr_in6* in6){
    ssize_t n;
    socklen_t dest_len = sizeof(struct sockaddr_storage);
    int message_length = strlen(message);
    CHECK(n = sendto(udp_socket, message, message_length, 0, (struct sockaddr*)in6, dest_len));
    if (n != message_length) {
        exit(EXIT_FAILURE);
    }
}


char* receiveMessage(int udp_socket, struct sockaddr_in6* in6){
    char* message = string_create(BUFSIZ + 1); 
    socklen_t address_len = sizeof(struct sockaddr_storage);
    ssize_t n;
    CHECK(n = recvfrom(udp_socket, message, BUFSIZ, 0, (struct sockaddr*)in6, &address_len));
    #ifndef FILEIO
    message[n] = '\0';
    #endif
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

/*
    return saysHELLO, if message type is /HELO
    return saysQUIT, if message type is /QUIT
    return saysFILE, if message type is /FILE
    return saysDATA, for all else
*/
Event getSocketMessageEvent(char* message){
    #ifdef BIN
    if (message[0] == HELLO_BINARY){
        return saysHELLO;
    }
    if (message[0] == QUIT_BINARY){
        return saysQUIT;
    }
    #ifdef FILEIO
    if (message[0] == FILE_BINARY){
        return saysFILE;
    }
    #endif
    return saysDATA;
    #endif
    if (strncmp(message, HELLO, sizeof(HELLO)) == 0){
        return saysHELLO;
    }
    if (strncmp(message, QUIT, sizeof(QUIT)) == 0){
        return saysQUIT;
    }
    #ifdef FILEIO
    if (strncmp(message, FILETRANSFER, strlen(FILETRANSFER)) == 0){
        if (message[strlen(FILETRANSFER) - 1] == ' '){
            return saysFILE;
        }
    }
    #endif
    return saysDATA;
}

/*
    Only valid input is either /QUIT or /FILE for file transfer
    return saysQUIT for /QUIT
    return saysFILE for /FILE
    return saysDATA for all else :
    /HELO after connection status already CONNECTED; DATA.
*/
Event getInputMessageEvent(char* message){
    if (strncmp(message, QUIT, sizeof(QUIT) - 1) == 0){
        if (message[sizeof(QUIT)] == '\0')
            return saysQUIT;
    }
    #ifdef FILEIO
    if (strncmp(message, FILETRANSFER, strlen(FILETRANSFER)) == 0){
        if (message[strlen(FILETRANSFER)-  1] == ' '){
            return saysFILE;
        }
    }
    #endif
    return saysDATA;
}

#ifdef FILEIO

// the size of the payload / content of the message is 3
#define PAYLOADSIZECONTAINER 3

/*
    Get the path of the file indicated by user input:
    "/FILE <path/to/file>"
*/
char* getFilePath(char* message) {
    char buff[BUFSIZ];
    strcpy(buff, message);

    // Split the message to get the file path
    char s[2] = " ";
    strtok(buff, s);
    char* filepath = strtok(NULL, s);

    // Remove the trailing newline character, if any
    filepath[strcspn(filepath, "\n")] = '\0';

    return filepath;
}

/*
    Get the file name from the file path, in order
    to store it in the message to be sent.
    This will allow for the receiver to store the file
    under the same name as it was for the sender.
*/
char* getFileName(char* filePath){
    char buff[BUFSIZ];
    strcpy(buff, filePath);
    char s[2] = "/";
    char* token = strtok(buff, s);
    char* filename;
    while (token != NULL){
        filename = token;
        token = strtok(NULL, s);
    }
    return filename;
}

/*
    Count the number of characters that form the header.
    This allows to have a large file name.
*/
int getHeaderSize (char* message){
    int count_spaces = 0;
    int i = 0;
    while (count_spaces < 3){
        if (message[i] == ' ') count_spaces++;
        i++;
    }
    return i;
}

/*
    Check if the path indicated by the user is valid, 
    if so then return a file descriptor to the file.
    If not, display a warning to the user.
*/
int isValidPath(char* filePath){
    int fd;
    if ((fd = open(filePath, O_RDONLY, 0444)) == -1){
        printf("cannot access file %s\n", filePath);
        fflush(stdout);
        return -1;
    }
    return fd;
}

/*
    Store the size of the payload / content into an array 
    of chars. Since the size is an ssize_t it has to be stored
    (in this case) on 2 bytes = 2 characters. Add a ' ' (space
    character) at the end, in order to separate it from 
    the content.
*/
void ssize_t_to_chars(ssize_t value, char result[]) {
    result[0] = (char)((value >> 8) & 0xFF);
    result[1] = (char)(value & 0xFF);
    result[2] = ' ';
}

/*
    Get the payload size by doing the reverse instructions
    to ssize_t_to_chars(). This will allow to ignore the 
    rest of the buffer when transferring the last part of a file.
*/
ssize_t chars_to_ssize_t(const char *chars) {
    return chars[1] + (chars[0] << 8);
}

/*
    This function sends a file, be it text or binary.
    It creates a string with the following structure: messageType' 'fileName' 'payloadSize' 'content
    It takes in account that the connection might use a binary protocol.
    And it also allows for a very large fileName.... (maybe it shouldn't).
    It displays a message to user indicating that the file transfer has completed, 
    that said, it only means the file has successfully left the client's side.
*/
void fileTransferSend(char* message, int socket, struct sockaddr_in6* in6){
    char* filePath = getFilePath(message);
    int fd = isValidPath(filePath);
    if (fd != -1){
        char* fileName = getFileName(filePath);
        strcat(fileName, " ");
        char message[BUFSIZ];
        #ifdef BIN
        message[0] = FILE_BINARY;
        message[1] = ' ';
        int messageTypeSize = 2;
        #else
        int messageTypeSize = strlen(FILETRANSFER);
        strncpy(message, FILETRANSFER, messageTypeSize);
        #endif
        int fileNameSize = strlen(fileName);
        strncpy(message + messageTypeSize, fileName, fileNameSize);

        int headerSize = messageTypeSize + fileNameSize + PAYLOADSIZECONTAINER;
        int available_space = BUFSIZ - headerSize;
        ssize_t n;
        while ((n = read(fd, message + headerSize, available_space)) > 0){
            char payloadSize[PAYLOADSIZECONTAINER];
            ssize_t_to_chars(n, payloadSize);
            strncpy(message + messageTypeSize + fileNameSize, payloadSize, PAYLOADSIZECONTAINER);
            sendMessage(message, socket, in6);
	    }
	    if (n == -1){
		    raler(errno, "read");
	    }
        printf("File sending completed.\n");
        CHECK(close(fd));
    }
}

/*
    This function receives a file, be it text or binary;
    It doesn't check if a file with that name already exists in the repository.
    It displays a message to the user letting them know that the file was received. 
*/
void fileTransferReceive(char* message){
    char* filename = getFilePath(message);
    int fd; 
    CHECK(fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0666));
    int headerSize = getHeaderSize(message);
    ssize_t payloadSize = chars_to_ssize_t(message + headerSize - PAYLOADSIZECONTAINER);
    ssize_t n;
    CHECK(n = write(fd, message + headerSize, payloadSize));
    if (n != payloadSize){
        raler(errno, "write error");
    }
    if (n < BUFSIZ - headerSize - 1){
        printf("File receipt completed. File is called : %s\n", filename);
    }
    CHECK(close(fd));
}
#endif


/*
    This function is responsible for processing all events on
    the UDP socket.
    It decides based on the type of event.
    Events:
    - saysHELLO -> allows to start the "connection".
    - saysQUIT -> sets the connectionStatus to DISCONNECTED thus ending the main loop of the program.
    - saysDATA -> prints the message (if it is not a /FILE)
    - saysFILE -> gets a part of the file using fileTransferReceive()
*/
void actionOnSocket(struct pollfd* fds, struct sockaddr_in6* in6, struct sockaddr_storage* ss){
    if(fds[1].revents & POLLIN){
        char* message = receiveMessage(fds[1].fd, in6);
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
            #ifdef FILEIO
            case saysFILE: {
                if (connectionStatus == CONNECTED){
                    printf("Receiving file. Wait for completion.\n");
                    fflush(stdout);
                    fileTransferReceive(message);
                }
                break;
            }
            #endif
        }
        destroyMessage(message);
    }
}

/*
    This function is responsible for processing all events (input)
    on STDIN.
    It makes decisions based on events.
    Events: 
    - saysHELLO : this event is ignored. The only time it is a valid command
                  is only at the beggining (and it is not sent by the user).
    - saysQUIT : this sets the connectionStatus to DISCONNECTED thus ending the main loop of the program.
                 then it sends a command message /QUIT or 1 to the other client.
    - saysDATA : this sends the message typed by the user on STDIN. (The message is other than /QUIT or /FILE).
    - saysFILE : when FILEIO is set, it sends an entire file, indicated by the user (if the file exists).
*/
void actionOnInput(struct pollfd* fds, struct sockaddr_in6* in6){
    if (fds[0].revents & POLLIN){
        char* userInput = readUserInput(fds[0].fd);
        Event e = getInputMessageEvent(userInput);
        switch(e){
            case saysHELLO:
            case saysQUIT: {
                connectionStatus = DISCONNECTED;
                #ifdef BIN
                char cmd[1]; cmd[0] = QUIT_BINARY;
                #else
                char cmd[CMDSIZE] = QUIT;
                #endif
                sendMessage(cmd, fds[1].fd, in6);
                break;
            }
            case saysDATA: {
                sendMessage(userInput, fds[1].fd, in6);
                break;
            }
            #ifdef FILEIO
            case saysFILE: {
                fileTransferSend(userInput, fds[1].fd, in6);
                break;
            }
            #endif
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
            #ifdef BIN
            char cmd[1]; cmd[0] = HELLO_BINARY;
            #else
            char cmd[CMDSIZE] = HELLO;
            #endif
            sendMessage(cmd, udp_socket, in6);
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
        // wait for /HELO but also deal with /QUIT and DATA, /FILE
        actionOnSocket(fds, in6, &ss);
        // user input from stdin
        actionOnInput(fds, in6);
    }

    /* close socket */
    CHECK(close(udp_socket));

    return 0;
}
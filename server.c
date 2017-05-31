#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#define WSPACE      " "
#define EOFILE         "\0"
#define EOLINE         "\n"
#define NAME_LEN        2048
#define MSG_LEN         2048
#define BUFFERFSIZE    256 //Size of read buffer
#define SHORT_LEN       24 
#define BACKLOG         10
#define CMD_LEN_LO    4 // long commands length
#define CMD_LEN_SH    3 // short commands length

#define DEFAULT_PORT    "10258"

//Commands
#define LIST            "List"
#define GET             "Get"
#define PUT             "Put"
#define QUIT            "Quit"    

void listHandler(void);
void getHandler(void);
void putHandler(void);
void quitHandler(void);
void helpHandler(void);

struct addrinfo *result, *p; //server info / list

struct in_addr **addr_list;

int s_tcp; //socket descriptor
int cond; //socket descriptor

char *port; //port used by socket 
int channelPort; //channel port
void *ip_addr_cli; //client ip
void *ip_addr_s; //server ip

char msg[MSG_LEN]; // buffer for recv messages
char buffer[MSG_LEN]; // buffer for send messages
char hostname[NAME_LEN];
char ip_addr[NAME_LEN]; // client ip
char ip_addr_srv[NAME_LEN]; // server ip

struct sockaddr_storage sock_addr_stor; //cleint socket
socklen_t sock_addr_len = sizeof (struct sockaddr_storage); // size of socketinfo

int main(int argc, char *argv[]) {

    if (argc == 2) {
        port = argv[1];
    } else {
        printf("V 1.05 Default port: 10258\n");
        port = DEFAULT_PORT;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //get server name 
    if (gethostname(hostname, sizeof (hostname))) {
        perror("gethostname failed: ");
    }

    //prepare struct addrinfo
    if (getaddrinfo(NULL, port, &hints, &result) != 0) {
        perror("getaddrinfo failed: ");
        exit(EXIT_FAILURE);
    }

    //try to connect
    for (p = result; p != NULL; p = p->ai_next) {
        //create socket descriptor
        s_tcp = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s_tcp == -1) {
            perror("server: socket");
            continue;
        }
        // bind socket with ip and port
        if (bind(s_tcp, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
        //saddr = (struct sockaddr_in *) p->ai_addr;
        close(s_tcp);
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(EXIT_FAILURE);
    }

    // free address information
    freeaddrinfo(result);

    // socket listening for incoming connection
    if (listen(s_tcp, BACKLOG) == -1) {
        perror("listen");
        close(s_tcp);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for TCP connections ... \n");


    while (1) {
        //try to receive message 
        if (recv(cond, msg, MSG_LEN, 0) <= 0) {

            if ((cond = accept(s_tcp, (struct sockaddr*) &sock_addr_stor,
                    &sock_addr_len)) < 0) {
                perror("accept");
                close(s_tcp);
                exit(EXIT_FAILURE);
            }

            channelPort = ((struct sockaddr_in*) &sock_addr_stor)->sin_port;
            ip_addr_cli = &((struct sockaddr_in*) &sock_addr_stor)->sin_addr;

            //Convert a Internet address in binary network format   
            if (((struct sockaddr*) &sock_addr_stor)->sa_family == AF_INET6) {
                inet_ntop(((struct sockaddr*) &sock_addr_stor)->sa_family,
                        &((struct sockaddr_in6*) &sock_addr_stor)->sin6_addr,
                        ip_addr, sizeof (ip_addr));
            } else {
                //const uint8_t *bytes = ip_addr_cli.s6_addr;
                //bytes += 12;
                //struct in_addr addr = { *(const in_addr_t *)bytes };

                inet_ntop(((struct sockaddr*) &sock_addr_stor)->sa_family, ip_addr_cli,
                        ip_addr, sizeof (ip_addr));
            }

        } else { //if message received
            //use handler
            if (strncmp(msg, LIST, CMD_LEN_LO) == 0) {
                listHandler();
            } else if (strncmp(msg, GET, CMD_LEN_SH) == 0) {
                getHandler();
            } else if (strncmp(msg, PUT, CMD_LEN_SH) == 0) {
                putHandler();
                //} else if (strncmp(receive_buff, QUIT, CMD_LEN_LO) == 0) {
                //    quitHandler();
            } else {
                helpHandler();
            }

        }
    }
}

/*
 * Handler for command "LIST"
 * List will prepare a message with the Serverhostname, the used IP-Address, the local time 
 * and the server directory contents
 */
void listHandler(void) {
    // dirlist buffer
    char dir_list_buf[MSG_LEN];
    //stream
    DIR *dir;
    struct dirent *dir_entry;
    time_t currenttime;
    char time_str[SHORT_LEN];

    if ((dir = opendir(".")) != NULL) {
        //reset string
        memset(dir_list_buf, 0, strlen(dir_list_buf));
        while ((dir_entry = readdir(dir)) != NULL) {
            strncat(dir_list_buf, dir_entry->d_name, sizeof (dir_list_buf));
            strncat(dir_list_buf, " \n   ", sizeof (dir_list_buf));
        }
        closedir(dir);
    } else {
        perror("open dir failed");
        exit(EXIT_FAILURE);
    }

    //gettime and format string
    currenttime = time(NULL);
    strftime(time_str, sizeof (time_str), "%d.%m.%Y %H:%M:%S",
            localtime(&currenttime));


    struct hostent *he;
    if ((he = gethostbyname(hostname)) == NULL) { // get the host info
        herror("gethostbyname");
    }
    addr_list = (struct in_addr **) he->h_addr_list;

    //prepare send-buffer
    sprintf(buffer,
            "Server: %s from %s\n Date: %s\n Directory:\n  %s ",
            hostname, (inet_ntoa(*addr_list[0])), time_str, dir_list_buf);

    //send buffer to client
    if (send(cond, buffer, strlen(buffer), 0) > 0) {
        puts("Send...\n");
    }
}

/**/

/* Handler for command "GET"
 * The textfile contents at the local server directory will be prepared for sending to the client.
 */
void getHandler(void) {
    char string[MSG_LEN];
    //file attributes
    struct stat fileAttrBuf;
    FILE *pf;
    char *filename;

    //get filename
    filename = strtok(msg, WSPACE);
    filename = strtok(NULL, EOLINE);

    //Get file attributes
    stat(filename, &fileAttrBuf);
    //prepare head
    sprintf(string,
            "File: %s \nLast modification: %s\nSize: %d Bytes\nContents:\n",
            filename, ctime((const time_t *) &fileAttrBuf.st_mtim),
            (int) fileAttrBuf.st_size);

    char content[BUFFERFSIZE];
    char buf[BUFFERFSIZE];
    // open file and get content
    pf = fopen(filename, "r");

    if (pf == NULL) {
        perror("open file");
    } else {
        while (feof(pf) == 0) {
            if (fgets(buf, BUFFERFSIZE, pf) != NULL) {
                sprintf(content, "%s", buf);
                strncat(string, content, sizeof (string));
                printf("file: %s \n", content);
            }
        }
        fclose(pf);
    }

    //Write to output buffer
    sprintf(buffer, " Server: %s from %s \n %s", hostname, ip_addr,
            string);
    //send buffer to client
    if (send(cond, buffer, strlen(buffer), 0) > 0) {
        printf("%s ... sent\n", buffer);
    }
}

/*
 * putHandler handles a push-operation
 * the client is sending a textfile
 * the server does a console output an saves the file
 */
void putHandler(void) {
    struct sockaddr *hostAddr;
    char client_name[NAME_LEN];

    hostAddr = (struct sockaddr*) &sock_addr_stor;
    //get client address
    if (getnameinfo(hostAddr, sizeof (hostAddr), client_name, NAME_LEN, NULL, 0,
            0) != 0) {
        perror(("resolve adress: "));
        sprintf(client_name, "%s", ip_addr);
    }

    char *filename;
    //content of received file
    char *filecontent;
    size_t filesize;
    //get filename filename
    filename = strtok(msg, WSPACE);
    filename = strtok(NULL, WSPACE);
    //extract content from file
    filecontent = strtok(NULL, EOFILE);
    filesize = strlen(filecontent);

    //create and write to file
    FILE * rcv_fileptr;
    rcv_fileptr = fopen(filename, "w");
    fwrite(filecontent, 1, filesize, rcv_fileptr);
    fclose(rcv_fileptr);

    sprintf(buffer, " OK %s: %d \n", client_name, channelPort);
    //send buffer
    if (send(cond, buffer, strlen(buffer), 0) > 0) {
        printf("%s ... sent\n", buffer);
    }
}

/*
 *  unknown command handler
 *  The incorrect message and a help message for correct usage will be prepared
 */
void helpHandler(void) {
    printf("Command %s is wrong\n", msg);
    sprintf(buffer, "Command entered incorrectly or not supported.\nList of available commands:\n’List’\n’Get <filename>’\n’Put <filename>’\n’Quit’\n");
    if (send(cond, buffer, strlen(buffer), 0) > 0) {
        printf("%s sent...\n", buffer);
    }
}

///**
// * Handles quit request
// **/
//void quitHandler(void) {
//    close(cond);
//    exit(EXIT_SUCCESS);
//}
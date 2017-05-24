#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <netdb.h>



#define DEF_PORT	10258
#define MSG_LEN	256
#define CMND_LEN 4
#define LIST	"List"
#define GET	"Get "
#define PUT	"Put "
#define QUIT	"Quit"

int main(int argc, char* args[]) {

    int s_tcp; /* socket descriptor (listener)*/
    int messenger; /* socket descriptor (send and recv)*/
    char* port;
    struct addrinfo *result, *p;
    struct addrinfo hints;
    int n;


    char msg[MSG_LEN];
    char buffer[MSG_LEN];

    if (argc == 2) {
        port = args[1];
    } else if (argc == 1) {
        port = DEF_PORT;
    } else {
        perror("wrong parameters");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &result) != 0) {
        perror("getaddrdinfo failed");
        return 1;
    }

    for (p = result; p != NULL; p = p->ai_next) {
        if ((s_tcp = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) < 0) {
            perror("TCP Socket");
            exit(1);
        }
    }

    if (bind(s_tcp, result->ai_addr, result->ai_addrlen) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(s_tcp, 5) < 0) {
        perror("listen");
        close(s_tcp);
        exit(1);
    }
    printf("Waiting for TCP connections ... \n");
    
    
    if ((messenger = accept(s_tcp, (struct sockaddr*) &sa_client, &sa_len)) < 0) {
        perror("accept");
        close(s_tcp);
        exit(1);
    }
    
    
    
    
    //freeaddrinfo(result);
    
    


    

    while (1) {
        if (recv(news, info, sizeof (info), 0) > 0) {
            printf("Message received: %s \n", info);

            if ((strncmp(info, GET, CMND_LEN) == 0)) {

                printf("Compare is ok \n");

                filename = "my test file name";

                //filename = strtok(info, space_separator);
                ///filename = strtok(NULL, line_end_separator);

                printf("filename is ok \n");

                //stat(filename, &attrib);

                printf("stat is ok \n");
                sprintf(file_string, "[File]: %s \n",
                        filename);

                printf("sprintf is ok \n");
                if (send(news, file_string, strlen(file_string), 0) > 0) {
                    printf("antwort gesendet \n");
                }

            }
        }





    }

    close(s_tcp);
}
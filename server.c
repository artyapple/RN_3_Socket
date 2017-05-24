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
#include <string.h> /* memset */
#include <unistd.h> /* close */



#define DEF_PORT	10258
#define MSG_LEN	512
#define CMND_LEN 4
#define LIST	"List"
#define GET	"Get "
#define PUT	"Put "
#define QUIT	"Quit"

int main(int argc, char* args[]) {

    int s_tcp; /* socket descriptor (listener)*/
    int messenger; /* socket descriptor (send and recv)*/
    char* port = "10258";
    struct addrinfo *result, *p;
    struct addrinfo hints;
    int n;
    int yes = 1;
    struct sockaddr_storage their_addr; //client addr info 
    socklen_t sin_size;
    int recv_ready;


    char msg[MSG_LEN];
    char buffer[MSG_LEN];

    //if (argc == 2) {
    //    port = args[1];
    //} else if (argc == 1) {
    //    port = DEF_PORT;
    //} else {
    //   perror("wrong parameters");
    //  return 1;
    //}

    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &result) != 0) {
        perror("getaddrdinfo failed");
        return 1;
    }


    for (p = result; p != NULL; p = p->ai_next) {
        if ((s_tcp = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(s_tcp, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof (int)) == -1) {
            perror("setsockopt");
            continue;
        }

        if (bind(s_tcp, p->ai_addr, p->ai_addrlen) == -1) {
            close(s_tcp);
            perror("server: bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(result);

    if (listen(s_tcp, 5) < 0) {
        perror("listen");
        close(s_tcp);
        return 1;
    }
    printf("Waiting for TCP connections ... \n");




    while (1) {
        printf("accept ... \n");
        sin_size = sizeof their_addr;
        if ((messenger = accept(s_tcp, (struct sockaddr*) &their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        recv_ready = 1;
        while (recv_ready) {
            if (n = (recv(messenger, msg, MSG_LEN, 0)) <= 0) {
                recv_ready = 0;
                printf("Recv nothing ... \n");
                //s_tcp delete?           
                break;
            } else {
                buffer[n] = '\0';
                printf("Messege recived : \n%s", buffer);


                if ((strncmp(msg, GET, CMND_LEN) == 0)) {

                } else if ((strncmp(msg, PUT, CMND_LEN) == 0)) {

                } else if ((strncmp(msg, LIST, CMND_LEN) == 0)) {
                    sprintf(buffer, "this is answer from server (command LIST)");
                } else {
                    //help output
                    printf("wrong command\n");
                }

                if (send(messenger, buffer, strlen(buffer), 0) > 0) {
                    printf("%s sent...\n", buffer);
                }

            }
            memset(buffer, 0, strlen(buffer));
            memset(msg, 0, strlen(msg));

        }

    }

    close(s_tcp);
}
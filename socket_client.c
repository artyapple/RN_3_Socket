#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <resolv.h>
#include <stdlib.h>
#include <sys/select.h>

#define WSPACE " "
#define EOLINE "\n"
#define EOFILE '\0'
#define MSG_LEN 2048
#define LONG_LEN 4
#define SHORT_LEN 3
#define PUT "Put"
#define QUIT "Quit"

int s_tcp; // socket descriptor
int connected = 0;

int main(int argc, char *argv[])
{

    struct addrinfo *server, *p;
    struct addrinfo hints;
    char *port;

    if (argc == 3)
    {
        port = argv[2];
    }
    else
    {
        printf("Wrong arguments\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED;

    //get connection info
    if (getaddrinfo(argv[1], port, &hints, &server) != 0)
    {
        perror("getaddrinfo()");
        exit(1);
    }

    //try to connect
    for (p = server; p != NULL; p = p->ai_next)
    {
        s_tcp = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s_tcp == -1)
        {
            continue;
        }
        if (connect(s_tcp, p->ai_addr, p->ai_addrlen) != -1)
        {
            connected = 1;
            break;
        }
        close(s_tcp);
    }
    //free port
    freeaddrinfo(server);

    while (connected)
    {
        char *filename;
        char msg[MSG_LEN];
        char buffer[MSG_LEN];

        char string[MSG_LEN];
        char content[SHORT_LEN];
        int buf; //for char
        int rply;

        printf("\nEnter command: ");
        //read from console
        fgets(msg, MSG_LEN, stdin);

        //quit - close socket deskriptor
        if (strncmp(msg, QUIT, LONG_LEN) == 0)
        {
            close(s_tcp);
            exit(0);
        }
        //put the file on the server
        else if (strncmp(msg, PUT, SHORT_LEN) == 0)
        {
            //prepare filename
            filename = strtok(msg, WSPACE);
            filename = strtok(NULL, EOLINE);

            //read file and write it to content
            FILE *pf;
            pf = fopen(filename, "r");
            if (pf == NULL)
            {
                perror("open file");
            }
            else
            {
                while ((buf = fgetc(pf)) != EOF)
                {
                    sprintf(content, "%c", buf);
                    strncat(string, content, sizeof(string));
                }
                fclose(pf);
            }
            sprintf(msg, "Put %s %s", filename, string);
            //send buffer
            send(s_tcp, msg, strlen(msg), 0);
        }
        else
        {
            send(s_tcp, msg, strlen(msg), 0);
        }

        if ((rply = recv(s_tcp, buffer, sizeof(buffer), 0)) > 0)
        {
            buffer[rply] = EOFILE;
            printf("Answer:\n%s", buffer);
        }

        //reset buffer
        memset(buffer, 0, strlen(buffer));
        memset(msg, 0, strlen(msg));
    }
    return EXIT_SUCCESS;
}

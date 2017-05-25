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
#include <time.h>
#include <dirent.h>

#define DEF_PORT 10258
#define MSG_LEN 512
#define CMND_LEN 4
#define LIST "List"
#define GET "Get "
#define PUT "Put "
#define QUIT "Quit"

//int listHandler(void);

char buffer[MSG_LEN];

struct sockaddr_storage their_addr; //client addr info
struct addrinfo *result, *p;
char ipstr[100];
int cport;

int main(int argc, char *args[])
{

    int s_tcp;     /* socket descriptor (listener)*/
    int messenger; /* socket descriptor (send and recv)*/
    char *port = "10258";

    struct addrinfo hints;
    int n;
    int yes = 1;
    socklen_t sin_size;
    int recv_ready;
    char msg[MSG_LEN];
    //char buffer[MSG_LEN];

    //if (argc == 2) {
    //    port = args[1];
    //} else if (argc == 1) {
    //    port = DEF_PORT;
    //} else {
    //   perror("wrong parameters");
    //  return 1;
    //}

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &result) != 0)
    {
        perror("getaddrdinfo failed");
        return 1;
    }

    //try to connect
    for (p = result; p != NULL; p = p->ai_next)
    {
        //create socket descriptor
        if ((s_tcp = socket(p->ai_family, p->ai_socktype,
                            p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }
        // set socket option; indicates that the rules used in validating adresses supplied in a bind() call
        // should allow reuse of local addresses
        if (setsockopt(s_tcp, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            continue;
        }
        // bind socket with ip and port
        if (bind(s_tcp, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(s_tcp);
            perror("server: bind");
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    // server ip
    void *addr;
    if (p->ai_family == AF_INET)
    {
        struct sockaddr_in *ip = (struct sockaddr_in *)p->ai_addr;
        addr = &(ip->sin_addr);
        cport = &(ip->sin_port);
    }
    else
    {
        struct sockaddr_in6 *ip = (struct sockaddr_in6 *)p->ai_addr;
        addr = &(ip->sin6_addr);
        cport = &(ip->sin6_port);
    }
    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    // frees address information
    freeaddrinfo(result);
    // socket listening for incoming connections
    if (listen(s_tcp, 5) < 0)
    {
        perror("listen");
        close(s_tcp);
        return 1;
    }
    printf("Waiting for TCP connections ... \n");

    while (1)
    {
        sin_size = sizeof their_addr;
        if ((messenger = accept(s_tcp, (struct sockaddr *)&their_addr, &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }
        recv_ready = 1;
        while (recv_ready)
        {
            if (n = (recv(messenger, msg, MSG_LEN, 0)) <= 0)
            {
                recv_ready = 0;
                printf("Recv nothing ... \n");
                //s_tcp delete?
                break;
            }
            else
            {
                buffer[n] = '\0';
                printf("Messege recived : \n%s", buffer);

                if ((strncmp(msg, GET, CMND_LEN) == 0))
                {
                    getHandler(msg);
                }
                else if ((strncmp(msg, PUT, CMND_LEN) == 0))
                {
                    putHandler(msg);
                }
                else if ((strncmp(msg, LIST, CMND_LEN) == 0))
                {
                    listHandler();
                }
                else
                {
                    printf("wrong command\n");
                    wrongCmd(msg);
                }

                if (send(messenger, buffer, strlen(buffer), 0) > 0)
                {
                    printf("sent\n");
                }
            }
            memset(buffer, 0, strlen(buffer));
            memset(msg, 0, strlen(msg));
        }
    }
    close(s_tcp);
}

// Handler for command "LIST"
int listHandler(void)
{
    DIR *dir;
    struct dirent *ent;
    char list_string[MSG_LEN];

    if ((dir = opendir(".")) != NULL)
    {
        //empty the string
        memset(list_string, 0, strlen(list_string));

        while ((ent = readdir(dir)) != NULL)
        {
            //save every entry of the direction in list_string
            strncat(list_string, ent->d_name, sizeof(list_string));
            strncat(list_string, " \n   ", sizeof(list_string));
        }
        closedir(dir);
    }
    else
    {
        perror("opendir failed");
        return 1;
    }

    char host_name[100];
    int host;
    if (host = gethostname(host_name, sizeof(host_name)))
    {
        perror("Error getting Hostname gethostname()");
        sprintf(host_name, "Unknown Hostname");
    }

    //get servers locatime
    char times_string[24];
    time_t now = time(0);
    strftime(times_string, sizeof(times_string), "%d.%m.%Y %H:%M:%S", localtime(&now));

    //fill send-buffer with directory, time, host_name and IP-address

    sprintf(buffer, " [Server]: %s from %s\n [Date]: %s \n [Directory]:\n   %s ",
            host_name, (ipstr), times_string, list_string);
}

// Handler for command "GET"
int getHandler(char *cmd)
{
    char *fname;
    FILE *fp;
    struct stat attrib;
    int fc;
    char str[10];
    char tmp[MSG_LEN];
    int c;

    fname = strtok(cmd, " ");
    fname = strtok(NULL, "\n");

    stat(fname, &attrib);

    sprintf(tmp, "File: %s \nLast modification: %sSize: %d Bytes\nContents: \n",
            fname, ctime(&attrib.st_mtim), (int)attrib.st_size);

    if ((int)(attrib.st_size) > MSG_LEN)
    {
        //warning info
        char warn[50] = "===Warning:File cant be received completely!===";
        strncat(tmp, warn, sizeof(tmp));
    }

    fp = fopen(fname, "r");
    if (fp == NULL)
    {
        perror("error file");
    }
    else
    {
        while ((c = fgetc(fp)) != EOF)
        {

            sprintf(str, "%c", c);
            strncat(tmp, str, sizeof(tmp));
        }
        fclose(fp);
    }

    //fille send buffer
    sprintf(buffer, "%s", tmp);
}

// wrong command handler
int wrongCmd(char *cmd)
{
    char *output;
    sprintf(buffer, "%s - command entered incorrectly or not supported.\nList of available commands:\n’List’\n’Get <filename>’\n’Put <filename>’\n’Quit’\n", strncpy(output, cmd, strlen(cmd) - 1));
}

int putHandler(char *cmd)
{
    FILE *fp;
    char cname[100];
    char *filecontent, *filename;
    size_t filesize;

    if (getnameinfo((struct sockaddr *)&their_addr, sizeof((struct sockaddr *)&their_addr), cname, 100, NULL, 0, 0) != 0)
    {
        perror(("Cannot resolve adress\n"));
    }

    filename = strtok(cmd, " ");
    filename = strtok(NULL, " ");

    filecontent = strtok(NULL, EOF);
    filesize = strlen(filecontent);


    fp = fopen(filename, "w");
    fwrite(filecontent, 1, filesize, fp);
    fclose(fp);

   
    sprintf(buffer, " OK %s:%d \n ", cname, cport);
}
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



#define DEF_PORT	10258
#define MSG_LEN	512
#define CMND_LEN 4
#define LIST	"List"
#define GET	"Get "
#define PUT	"Put "
#define QUIT	"Quit"

int listHandler(void);

char buffer[MSG_LEN];
struct addrinfo *result, *p;
char ipstr[100];

int main(int argc, char* args[]) {

    int s_tcp; /* socket descriptor (listener)*/
    int messenger; /* socket descriptor (send and recv)*/
    char* port = "10258";

    struct addrinfo hints;
    int n;
    int yes = 1;
    struct sockaddr_storage their_addr; //client addr info 
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

    memset(&hints, 0, sizeof (hints));
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &result) != 0) {
        perror("getaddrdinfo failed");
        return 1;
    }

    //try to connect
    for (p = result; p != NULL; p = p->ai_next) {
        //create socket descriptor
        if ((s_tcp = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        // set socket option; indicates that the rules used in validating adresses supplied in a bind() call
        // should allow reuse of local addresses
        if (setsockopt(s_tcp, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof (int)) == -1) {
            perror("setsockopt");
            continue;
        }
        // bind socket with ip and port
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

    // server ip
    void *addr;
    if(p->ai_family==AF_INET){
        struct sockaddr_in *ip = (struct sockaddr_in *) p->ai_addr;
        addr = &(ip->sin_addr);
    } else {
        struct sockaddr_in6 *ip = (struct sockaddr_in6 *) p->ai_addr;
        addr = &(ip->sin6_addr);
    }
    inet_ntop(p->ai_family, addr, ipstr, sizeof (ipstr));
    // frees address information 
    freeaddrinfo(result);
    // socket listening for incoming connections
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
                    getHandler(msg);
                } else if ((strncmp(msg, PUT, CMND_LEN) == 0)) {

                } else if ((strncmp(msg, LIST, CMND_LEN) == 0)) {
                    listHandler();
                } else {
                    printf("wrong command\n");
                    wrongCmd(msg);
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


// Handler for command "LIST"
int listHandler(void) {
    DIR *dir;
    struct dirent *ent;
    char list_string[MSG_LEN];

    if ((dir = opendir(".")) != NULL) {
        //empty the string

        memset(list_string, 0, strlen(list_string));

        while ((ent = readdir(dir)) != NULL) {
            //save every entry of the direction in list_string
            strncat(list_string, ent->d_name, sizeof (list_string));
            strncat(list_string, " \n   ", sizeof (list_string));
        }
        closedir(dir);

    } else {
        perror("opendir failed");
        return 1;
    }

    char host_name[100];
    int host;
    if (host = gethostname(host_name, sizeof (host_name))) {
        perror("Error getting Hostname gethostname()");
        sprintf(host_name, "Unknown Hostname");
    }

    //get servers locatime
    char times_string[24];
    time_t now = time(0);
    strftime(times_string, sizeof (times_string), "%d.%m.%Y %H:%M:%S", localtime(&now));

    //fill send-buffer with directory, time, host_name and IP-address

    sprintf(buffer, " [Server]: %s from %s\n [Date]: %s \n [Directory]:\n   %s ",
            host_name, (ipstr), times_string, list_string);

}

// Handler for command "GET"
int getHandler(char *cmd){
    char *whitespace = " ";
    char *lineend = "\n";
    char *fname;
    FILE *fp;
    struct stat attrib;
    int fc;

    fname = strtok(cmd, whitespace);
    fname = strtok(NULL, lineend);

    stat(fname,&attrib);
    if((int)(attrib.st_size)>MSG_LEN){
        //warning info, file to big
    }

    
    fp = fopen(fname, "rb");
    if(!fp){
        perror(fname); return 0;
    } else {
        while(fc=fgetc(fp) != EOF){
            sprintf(str, "%c",fc);
            strncat()
        }
        fclose(fp);
    }
    sprintf(buffer, "File: %s \nLast modification: %s\nSize: %s\nContent:\n%s",)

    if(1!=fread(buffer, sizeof(fp), 1, fp)){
        fclose(fp);
        return 0;
    }

    sprintf(buffer, " [FILENAME]: %s\n", fname);
    
}

// wrong command handler
int wrongCmd(char* cmd){
    char *output;
    sprintf(buffer, "%s - command entered incorrectly or not supported.\nList of available commands:\n’List’\n’Get <filename>’\n’Put <filename>’\n’Quit’\n",strncpy(output, cmd, strlen(cmd)-1));
}
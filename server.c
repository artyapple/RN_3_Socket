#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>



#define SRV_PORT	10258
#define MSG_LEN	256
#define CMND_LEN 4
#define LIST	"List"
#define GET	"Get "
#define PUT	"Put "
#define QUIT	"Quit"

int main() {
    int s_tcp, news; /* socket descriptor */
    struct sockaddr_in sa,
            sa_client; /* socket address structure*/
    int sa_len = sizeof (struct sockaddr_in), n;
    char info[256];
    // to test
    struct stat attrib;           //for accessing file attributes
    char *filename;
    char space_separator[] = " ";
    char line_end_separator[] = "\n";
    char file_end_separator[] = "\0";
    char file_string[MSG_LEN];
    // to test ends

    sa.sin_family = AF_INET;
    sa.sin_port = htons(SRV_PORT);
    sa.sin_addr.s_addr = INADDR_ANY;


    if ((s_tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("TCP Socket");
        exit(1);
    }

    if (bind(s_tcp, (struct sockaddr*) &sa, sa_len) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(s_tcp, 5) < 0) {
        perror("listen");
        close(s_tcp);
        exit(1);
    }
    printf("Waiting for TCP connections ... \n");

    while (1) {
        if ((news = accept(s_tcp, (struct sockaddr*) &sa_client, &sa_len)) < 0) {
            perror("accept");
            close(s_tcp);
            exit(1);
        }
        if (recv(news, info, sizeof (info), 0)) {
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
                send(s_tcp,file_string, strlen(file_string),0);

            }
        }





    }

    close(s_tcp);
}
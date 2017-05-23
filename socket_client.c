/* 
 * File:   socket_client.c
 * Author: networker
 *
 * Created on 23 May 2017, 12:38
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

#define MSG_LEN	2000
#define CMND_LEN 4
#define LIST	"List"
#define GET	"Get "
#define PUT	"Put "
#define QUIT	"Quit"
//#define SRV_PORT	7777
/*
 * 
 */
int main(int argc, char* args[]) {
  
  int s_tcp;			/* socket descriptor */
  char* port;
  char* srv_adr;
  struct addrinfo *result,*p;
  struct addrinfo hints;
  int running = 1;
  int eingabeact = 1;  


  char msg[MSG_LEN];
  char buffer[MSG_LEN];
  
  srv_adr = args[1];
  
  if(argc==3){
      port = args[2];
  }
  else if(argc ==2){
      port = NULL;
  }else{
      perror("wrong parameters");
      return 1;	  
  }
  
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
  hints.ai_socktype = SOCK_STREAM;
    
  if(getaddrinfo(srv_adr, port,&hints ,&result)!=0){
      perror("getaddrdinfo failed");
      return 1;
  }
  

  
  for(p = result; p != NULL; p = p->ai_next) {      
    if ((s_tcp = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) < 0) {
	perror("TCP Socket");
	exit(1);
    }

    if (connect(s_tcp,result->ai_addr, result->ai_addrlen) < 0) {
	perror("Connect");
	exit(1);
    }      
  }
  
  freeaddrinfo(result);
  
  while(running){
    
    printf("Enter command: \n");
      
    if(eingabeact==1){
        fgets(msg, MSG_LEN, stdin);
    }
    
    
              
              
    if(strncmp(msg, QUIT, CMND_LEN)==0){
        printf("connection terminated\n"); 
        running = 0;
    
    } else if ((strncmp(msg, GET, CMND_LEN)==0)){
        if((send(s_tcp,msg, strlen(msg),0)) > 0) {
            //printf("Message %s sent.\n", msg); 
	}
        eingabeact = 0;
           
    } else if ((strncmp(msg, PUT, CMND_LEN)==0)){
    
    } else if ((strncmp(msg, LIST, CMND_LEN)==0)){
    
    } else {
       //help output
        printf("wrong command\n");
    }
    
    
    if((recv(s_tcp, buffer, strlen(buffer), 0)) > 0) {
        printf("Antwort is! : \n", buffer);
        eingabeact = 1;
    } 
    
//    if((n=send(s_tcp,msg, strlen(msg),0)) > 0) {
//	printf("Message %s sent ( %i Bytes).\n", msg, n); 
//	}

  
  }
  close(s_tcp);
  
  
  
  
  
  
  
  
  
  
  
  
  

  //if (inet_pton(sa.sin_family,args[1], &sa.sin_addr.s_addr) <= 0) {
//	perror("Address Conversion");
//	exit(1);
//	}

  

  
}


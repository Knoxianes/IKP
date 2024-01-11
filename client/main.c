#include <arpa/inet.h>
#include <asm-generic/ioctls.h>
#include <bits/types/struct_timeval.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX 80
#define PORT 5059
#define SLEEP 2

int main() {
  int socketClient, conn;
  struct sockaddr_in6 serverAddress;
  char buffer[MAX];
  int n;

  // Creating AF_INET socket
  socketClient = socket(AF_INET6, SOCK_STREAM, 0);
  if (socketClient == -1) {
    printf("Socket creating failed...\n");
    exit(1);
  } else {
    printf("Socket created...\n");
  }

  // Adding serverAddress data for connetion
  serverAddress.sin6_family = AF_INET6;
  inet_pton(AF_INET6, "::1", &serverAddress.sin6_addr);
  serverAddress.sin6_port = htons(PORT);

  if (connect(socketClient, (struct sockaddr *)&serverAddress,
              sizeof(serverAddress)) != 0) {
    printf("Connection with server failed...\n");
    close(socketClient);
    exit(2);
  } else {
    printf("Connected to the server...\n");
  }
  strcpy(buffer,"ok");
  int counter;
  
  while (1) {
    counter++;
    if(strncmp(buffer,"ok",2) != 0){
      if(strncmp(buffer,"Full",4) != 0){
        printf("Server error");
        break;
      }
      bzero(buffer,sizeof(buffer));
      read(socketClient,buffer,sizeof(buffer));
      if(strncmp(buffer,"Continue",8) != 0){
        printf("Server error"); 
        break;
      }
      bzero(buffer,sizeof(buffer));
    }
    bzero(buffer, sizeof(buffer));
    char tmp[MAX];
    sprintf(tmp,"Payload number %d",counter);
    strcpy(buffer,tmp);
    write(socketClient, buffer, sizeof(buffer));
    bzero(buffer, sizeof(buffer));
    read(socketClient, buffer, sizeof(buffer));
    if ((strncmp(buffer, "exit", 4)) == 0) {
      printf("Client Exit...\n");
      break;
    }
    sleep(rand()%SLEEP+1);
  }
  close(socketClient);
  
  return 0;
}

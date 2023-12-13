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
#define PORT 8080
#define FALSE 0
#define TRUE 1

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

  while (TRUE) {
    bzero(buffer, sizeof(buffer));
    printf("Enter the string : ");
    n = 0;
    while ((buffer[n++] = getchar()) != '\n')
      ;
    write(socketClient, buffer, sizeof(buffer));
    bzero(buffer, sizeof(buffer));
    read(socketClient, buffer, sizeof(buffer));
    printf("From Server : %s", buffer);
    if ((strncmp(buffer, "exit", 4)) == 0) {
      printf("Client Exit...\n");
      break;
    }
  }
  close(socketClient);

  return 0;
}

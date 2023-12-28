
#include <asm-generic/ioctls.h>
#include <bits/types/struct_timeval.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX 80
#define PORT 8080
#define BACKLOG 32
#define MAXWAIT 5
#define FALSE 0
#define TRUE 1

void *ServeClient(void *args) {
  int sd = *(int*)args;
  printf("  Descriptor %d is readable\n", sd);
  int i, len, rc, on = 1;
  char buffer[80];
  do {
    /**********************************************/
    /* Receive data on this connection until the  */
    /* recv fails with EWOULDBLOCK.  If any other */
    /* failure occurs, we will close the          */
    /* connection.                                */
    /**********************************************/
    rc = recv(sd, buffer, sizeof(buffer), 0);
    if (rc < 0) {
      if (errno != EWOULDBLOCK) {
        perror("  recv() failed");
      }
      break;
    }

    /**********************************************/
    /* Check to see if the connection has been    */
    /* closed by the client                       */
    /**********************************************/
    if (rc == 0) {
      printf("  Connection closed\n");
      break;
    }

    /**********************************************/
    /* Data was received                          */
    /**********************************************/
    len = rc;
    printf("  %d bytes received\n", len);

    /**********************************************/
    /* Echo the data back to the client           */
    /**********************************************/
    rc = send(sd, buffer, len, 0);
    if (rc < 0) {
      perror("  send() failed");
      break;
    }

  } while (TRUE);
}

int main(int argc, char *argv[]) {
  int i, len, rc, on = 1;
  int listen_sd, new_sd;
  int desc_ready, end_server = FALSE;
  char buffer[80];
  struct sockaddr_in6 addr;

  /*************************************************************/
  /* Create an AF_INET6 stream socket to receive incoming      */
  /* connections on                                            */
  /*************************************************************/
  listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
  if (listen_sd < 0) {
    perror("socket() failed");
    exit(-1);
  }

  /*************************************************************/
  /* Allow socket descriptor to be reuseable                   */
  /*************************************************************/
  rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on,
  sizeof(on)); if (rc < 0) {
    perror("setsockopt() failed");
    close(listen_sd);
    exit(-1);
  }

  /*************************************************************/
  /* Bind the socket                                           */
  /*************************************************************/
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
  addr.sin6_port = htons(PORT);
  rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
  if (rc < 0) {
    perror("bind() failed");
    close(listen_sd);
    exit(-1);
  }

  /*************************************************************/
  /* Set the listen back log                                   */
  /*************************************************************/
  rc = listen(listen_sd, 32);
  if (rc < 0) {
    perror("listen() failed");
    close(listen_sd);
    exit(-1);
  }

  do {
    printf("Waiting for accept\n");
    new_sd = accept(listen_sd, NULL, NULL);
    if (new_sd < 0) {
      if (errno != EWOULDBLOCK) {
        perror("  accept() failed");
        end_server = TRUE;
      }
    }
    pthread_t thread;
    pthread_create(&thread, NULL, ServeClient, (void*)(&new_sd));
  } while (end_server == FALSE);
}

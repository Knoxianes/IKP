
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/consts.h"
#include "../include/linkedlist.h"
#include "../include/queue.h"

int main(int argc, char *argv[]) {
  int i, len, rc, on = 1;
  int listen_sd, client_socket_sd;
  char buffer[BUFFER_SIZE];
  struct sockaddr_in6 addr;

  Queue* payloads = create_queue();
  LinkedList* list_of_workers = create_list();

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

  printf("Waiting for accept\n");
  client_socket_sd = accept(listen_sd, NULL, NULL);
  if (client_socket_sd < 0) {
    if (errno != EWOULDBLOCK) {
      perror("  accept() failed");
    }
  }
  printf("Accepted\n");


}


#include "../include/consts.h"
#include "../include/linkedlist.h"
#include "../include/queue.h"
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

pthread_mutex_t payloads_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t list_of_workers_mutex = PTHREAD_MUTEX_INITIALIZER;


typedef struct args {
  Queue *payloads;
  LinkedList *list_of_workers;
} ARGS;

void *SendPayLoads(void *);

int main(int argc, char *argv[]) {
  int i, len, rc, on = 1;
  int listen_sd, client_socket_sd;
  char buffer[BUFFER_SIZE];
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
  rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
  if (rc < 0) {
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

  Queue *payloads = create_queue();
  LinkedList *list_of_workers = create_list();
  ARGS *args = (ARGS *)malloc(sizeof(ARGS));
  args->payloads = payloads;
  args->list_of_workers = list_of_workers;
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, SendPayLoads, args);

  int full = 0;
  while (1) {
    if (payloads->size == payloads->max_size) {
      if (full == 0) {
        strcpy(buffer, "Full");
        rc = send(client_socket_sd, buffer, sizeof(buffer), 0);
        full = 1;
        bzero(buffer, sizeof(buffer));
      }
      continue;
    }
    if (full == 1) {
      strcpy(buffer, "Continue");
      rc = send(client_socket_sd, buffer, sizeof(buffer), 0);
      full = 0;
      bzero(buffer, sizeof(buffer));
    }
    rc = recv(client_socket_sd, buffer, sizeof(buffer), 0);
    if (rc <= 0) {
      break;
    }
    pthread_mutex_lock(&payloads_mutex);
    enqueue(payloads, buffer);
    pthread_mutex_unlock(&payloads_mutex);
    bzero(buffer, sizeof(buffer));
    strncpy(buffer, "ok",2);
    rc = send(client_socket_sd, buffer, sizeof(buffer), 0);
    bzero(buffer, sizeof(buffer));
  }
  free_queue(payloads);
  free_list(list_of_workers);
  free(args);
  return 0;
}

void *SendPayLoads(void *args) {
  ARGS *tmp = args;
  Queue *payloads = tmp->payloads;
  LinkedList *list_of_workers = tmp->list_of_workers;
  printf("Size of queue: %d, Size of Linkedlist: %d\n",payloads->max_size,list_of_workers->size);
  while(1){
    if(payloads->size == 0){
      continue;
    }
    pthread_mutex_lock(&payloads_mutex);
    printf("%s\n",dequeue(payloads));
    pthread_mutex_unlock(&payloads_mutex);
    sleep(3);
  }
  return NULL;
}

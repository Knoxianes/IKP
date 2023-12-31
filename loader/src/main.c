#include "../include/consts.h"
#include "../include/linkedlist.h"
#include "../include/queue.h"
#include "../include/sockets.h"
#include <asm-generic/errno.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
int end_program = 0;
pthread_mutex_t payloads_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t list_of_workers_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct payloadargs {
  Queue *payloads;
  LinkedList *list_of_workers;
} PAYLOADARGS;

typedef struct process_creatinon_args {
  LinkedList *list_of_workers;
  struct pollfd *fds;
  int *nfds;
} CREATIONARGS;

typedef struct processfinishedargs {
  LinkedList *list_of_workers;
  struct pollfd *fds;
  int *nfds;
  int process_socket_sd;
} FINISHEDARGS;
void *SendPayLoads(void *);
void *ListenForProcessFinish(void *);
void *ProcessCreation(void *);
void Work();

int main(int argc, char *argv[]) {
  char buffer[BUFFER_SIZE];
  int client_socket_sd, rc, process_socket_sd;

  client_socket_sd = create_client_socket(); // Waits for client to connect
  process_socket_sd =
      create_process_socket(); // Just creates and returns socket descriptor
                               // after listen func

  Queue *payloads = create_queue();
  LinkedList *list_of_workers = create_list();
  int nfds = 1;
  struct pollfd fds[200];
  fds[0].fd = process_socket_sd;
  fds[0].events = POLLIN;

  PAYLOADARGS *payloadargs = (PAYLOADARGS *)malloc(sizeof(PAYLOADARGS));
  payloadargs->payloads = payloads;
  payloadargs->list_of_workers = list_of_workers;

  FINISHEDARGS *finishedargs = (FINISHEDARGS *)malloc(sizeof(FINISHEDARGS));
  finishedargs->list_of_workers = list_of_workers;
  finishedargs->fds = fds;
  finishedargs->nfds = &nfds;
  finishedargs->process_socket_sd = process_socket_sd;

  pthread_t thread_payloads, thread_process_create, thread_process_finished;
  pthread_create(&thread_payloads, NULL, SendPayLoads, payloadargs);
  pthread_create(&thread_process_finished, NULL, ListenForProcessFinish,
                 finishedargs);

  int full = 0;
  while (!end_program) {
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
      end_program = 1;
      break;
    }
    pthread_mutex_lock(&payloads_mutex);
    enqueue(payloads, buffer);
    pthread_mutex_unlock(&payloads_mutex);
    bzero(buffer, sizeof(buffer));
    strncpy(buffer, "ok", 2);
    rc = send(client_socket_sd, buffer, sizeof(buffer), 0);
    bzero(buffer, sizeof(buffer));
  }
  free_queue(payloads);
  free_list(list_of_workers);
  free(payloadargs);
  free(finishedargs);
  close(client_socket_sd);
  close(process_socket_sd);
  return 0;
}

void *SendPayLoads(void *args) {
  PAYLOADARGS *tmp = args;
  Queue *payloads = tmp->payloads;
  LinkedList *list_of_workers = tmp->list_of_workers;
  char buffer[BUFFER_SIZE];
  while (!end_program) {
    bzero(buffer, sizeof(buffer));
    if (payloads->size == 0) {
      continue;
    }
    pthread_mutex_lock(&list_of_workers_mutex);
    ListNode *free_process = find_first_free(list_of_workers);
    pthread_mutex_unlock(&list_of_workers_mutex);
    if (free_process == NULL) {
      continue;
    }

    pthread_mutex_lock(&payloads_mutex);
    strcpy(buffer, dequeue(payloads));
    pthread_mutex_unlock(&payloads_mutex);

    int rc = send(free_process->sd, buffer, sizeof(buffer), 0);
    if (rc < 0) {
      printf("Error while sending data to process"); // Ovde postoji bug ako se
                                                     // desi send failed gubimo
                                                     // payload iz queue
      continue;
    }

    free_process->in_use = 1;
    sleep(3);
  }
  return NULL;
}

void *ListenForProcessFinish(void *args) {
  FINISHEDARGS *tmp = args;
  LinkedList *list_of_workers = tmp->list_of_workers;
  struct pollfd *fds = tmp->fds;
  int *nfds = tmp->nfds;
  int rc, current_size, new_sd;
  int process_socket_sd = tmp->process_socket_sd;
  char buffer[BUFFER_SIZE];
  int close_conn, compress_array = 0;
  do{

    rc = poll(fds, *nfds, -1);
    if (rc < 0) {
      printf("Error with poll()\n");
      end_program = 1;
      break;
    }
    current_size = *nfds;
    for (int i = 0; i < current_size; i++) {
      if (fds[i].revents == 0) {
        continue;
      }

      if (fds[i].revents != POLLIN) {
        printf("Error revents = %d\n", fds[i].revents);
        end_program = 1;
        break;
      }

      if (fds[i].fd == process_socket_sd) {
        do {
          new_sd = accept(process_socket_sd, NULL, NULL);
          if (new_sd < 0) {
            if (errno != EWOULDBLOCK) {
              printf("Error with accepting new_sd\n");
              end_program = 1;
            }
            break;
          }

          printf("New connection - %d\n", new_sd);
          fds[*nfds].fd = new_sd;
          fds[*nfds].events = POLLIN;
          (*nfds)++;
          ListNode *new_worker = (ListNode *)malloc(sizeof(ListNode));
          new_worker->sd = new_sd;
          new_worker->in_use = 0;
          pthread_mutex_lock(&list_of_workers_mutex);
          insert_at_end(list_of_workers, new_worker);
          pthread_mutex_unlock(&list_of_workers_mutex);

        } while (new_sd != -1);
      } else {
        printf("  Descriptor %d is readable\n", fds[i].fd);
        close_conn = 0;
        rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
        if (rc < 0) {
          if (errno != EWOULDBLOCK) {
            printf("RECV() failed\n");
            close_conn = 1;
          }
        } else if (rc == 0) {
          close_conn = 1;
        } else {
            if(strncmp(buffer,"finished",8) != 0){
            printf(" There is error with buffer of some process\n");
            end_program = 1;
            break;
          }
          pthread_mutex_lock(&list_of_workers_mutex);
          ListNode* worker = get_node(list_of_workers,fds[i].fd);
          pthread_mutex_unlock(&list_of_workers_mutex);
          worker->in_use = 0;
        }

        if(close_conn){
          close(fds[i].fd);
          pthread_mutex_lock(&list_of_workers_mutex);
          delete_specific_node(list_of_workers,fds[i].fd);
          pthread_mutex_unlock(&list_of_workers_mutex);
          fds[i].fd = -1;
          compress_array =  1;
        }
      }
    }
    if(compress_array){
      compress_array = 0;
      for(int i = 0; i < *nfds; i++){
        if(fds[i].fd == -1){
          for(int j = i; j< *nfds; j++){
            fds[j].fd = fds[j+1].fd;
          }
          i--;
          (*nfds)--;
        }
      }
    } 
  }while(!end_program);

  for(int i=0; i < *nfds;i++){
    if(fds[i].fd>0){
      close(fds[i].fd);
    }
  } 
  return NULL;
}
void *ProcessCreation(void *args) { return NULL; }
void Work();

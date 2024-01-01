#include "../include/consts.h"
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
pthread_mutex_t payloads_mutex;
pthread_mutex_t free_workers_mutex;
pthread_mutex_t fd_mutex;
pthread_mutex_t console_mutex;

typedef struct args {
  queue *payloads;
  queue *free_workers;
} ARGS;

void *get_payload(void *args);
void *send_payload(void *args);
void *load_balance(void *args);
void *process_create(void *args);

int main(int argc, char *argv[]) {
  pthread_t get_payload_thread, send_payload_thread, load_balance_thread,
      process_create_thread;
  queue *payloads = createQueue(BUFFER_SIZE);
  queue *free_workers = createQueue(sizeof(int));

  ARGS *args = (ARGS *)malloc(sizeof(ARGS));
  args->free_workers = free_workers;
  args->payloads = payloads;

  pthread_create(&load_balance_thread, NULL, load_balance, free_workers);
  pthread_create(&get_payload_thread, NULL, get_payload, payloads);
  pthread_create(&send_payload_thread, NULL, send_payload, args);
  pthread_create(&process_create_thread, NULL, process_create, args);
  if (fork() == 0) {
    execv("./process", NULL);
  }

  pthread_join(get_payload_thread, NULL);
  pthread_join(load_balance_thread, NULL);
  pthread_join(send_payload_thread, NULL);
  pthread_join(process_create_thread, NULL);

  free(args);
  destroyQueue(&free_workers);
  destroyQueue(&payloads);
  return 0;
}

void *get_payload(void *args) {
  char buffer[BUFFER_SIZE];
  queue *payloads = args;
  int client_socket_sd = create_client_socket(); // Waits for client to connect
  int rc, full = 0;

  while (!end_program) {
    if (getSize(payloads) == QUEUE_SIZE) {
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
    if (strncmp(buffer, "exit", 4) == 0) {
      end_program = 1;
      break;
    }
    char *tmp = (char *)malloc(sizeof(buffer));
    strcpy(tmp, buffer);
    pthread_mutex_lock(&payloads_mutex);
    enqueue(payloads, tmp);
    pthread_mutex_unlock(&payloads_mutex);

    bzero(buffer, sizeof(buffer));
    strncpy(buffer, "ok", 2);
    rc = send(client_socket_sd, buffer, sizeof(buffer), 0);
    bzero(buffer, sizeof(buffer));
  }
  close(client_socket_sd);
  return NULL;
}
void *load_balance(void *args) {
  queue *free_workers = args;
  int process_socket_sd, current_size = 0, new_sd;
  int nfds = 1, rc;
  int i, j, close_conn, compress_array = 0;
  char buffer[BUFFER_SIZE];
  process_socket_sd = create_process_socket();

  struct pollfd fds[200];
  fds[0].fd = process_socket_sd;
  fds[0].events = POLLIN;

  do {
    rc = poll(fds, nfds, -1);

    if (rc < 0) {
      printf("Error  poll()\n");
      end_program = 1;
      break;
    }

    if (rc == 0) {
      printf("Poll timeout\n");
      end_program = 1;
      break;
    }

    current_size = nfds;
    for (i = 0; i < current_size; i++) {
      if (fds[i].revents == 0)
        continue;

      if (fds[i].revents != POLLIN) {
        printf("Error! revents = %d\n", fds[i].revents);
        end_program = 1;
        break;
      }

      if (fds[i].fd == process_socket_sd) {
        do {

          new_sd = accept(process_socket_sd, NULL, NULL);
          if (new_sd < 0) {
            if (errno != EWOULDBLOCK) {
              printf("Accept failed()");
              end_program = 1;
            }
            break;
          }
          pthread_mutex_lock(&console_mutex);
          printf("New incoming connection - %d\n", new_sd);
          pthread_mutex_unlock(&console_mutex);
          int *tmp = (int *)malloc(sizeof(int));
          *tmp = new_sd;
          pthread_mutex_lock(&free_workers_mutex);
          enqueue(free_workers, tmp);
          pthread_mutex_unlock(&free_workers_mutex);

          fds[nfds].fd = new_sd;
          fds[nfds].events = POLLIN;
          nfds++;

        } while (new_sd != -1);
      } else {
        close_conn = 0;

        pthread_mutex_lock(&fd_mutex);
        rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
        pthread_mutex_unlock(&fd_mutex);
        if (rc < 0) {
          if (errno != EWOULDBLOCK) {
            printf(" recv() failed!\n");
            close_conn = 1;
          }
        } else if (rc == 0) {
          pthread_mutex_lock(&console_mutex);
          printf(" Connection closed fd: %d\n", fds[i].fd);
          pthread_mutex_unlock(&console_mutex);
          close_conn = 1;
        } else {
          if (strncmp(buffer, "finished", 8) != 0) {
            printf("Error worker sent something different from finished: %d\n",
                   fds[i].fd);
            end_program = 1;
            break;
          } else {
            int *tmp = (int *)malloc(sizeof(int));
            *tmp = fds[i].fd;
            pthread_mutex_lock(&free_workers_mutex);
            enqueue(free_workers, tmp);
            pthread_mutex_unlock(&free_workers_mutex);
          }
        }
        if (close_conn) {
          close(fds[i].fd);
          fds[i].fd = -1;
          compress_array = 1;
        }
      }
    }
    if (compress_array) {
      compress_array = 0;
      for (i = 0; i < nfds; i++) {
        if (fds[i].fd == -1) {
          for (j = i; j < nfds; j++) {
            fds[j].fd = fds[j + 1].fd;
          }
          i--;
          nfds--;
        }
      }
    }

  } while (!end_program);

  for (i = 0; i < nfds; i++) {
    if (fds[i].fd >= 0)
      close(fds[i].fd);
  }
  close(process_socket_sd);
  return NULL;
}
void *send_payload(void *args) {
  ARGS *tmp = args;
  queue *payloads = tmp->payloads;
  queue *free_workers = tmp->free_workers;
  int rc;
  int fd = 0;
  char buffer[BUFFER_SIZE];
  while (!end_program) {
    if (getSize(payloads) <= 0 || getSize(free_workers) <= 0) {
      continue;
    }
    pthread_mutex_lock(&free_workers_mutex);
    dequeue(free_workers, &fd);
    pthread_mutex_unlock(&free_workers_mutex);
    pthread_mutex_lock(&console_mutex);
    printf("Got fd of free worker: %d\n", fd);
    pthread_mutex_unlock(&console_mutex);
    pthread_mutex_lock(&payloads_mutex);
    dequeue(payloads, buffer);
    pthread_mutex_unlock(&payloads_mutex);
    pthread_mutex_lock(&console_mutex);
    printf("Got from payloads: %s\n", buffer);
    pthread_mutex_unlock(&console_mutex);
    pthread_mutex_lock(&fd_mutex);
    rc = send(fd, buffer, sizeof(buffer), 0);
    pthread_mutex_unlock(&fd_mutex);
    if (rc < 0) {
      printf("Error in send_payload with send() fd: %d\n", fd);
      if (getSize(free_workers) > 0) {
        printf("Trying one more time!\n");
        pthread_mutex_lock(&free_workers_mutex);
        dequeue(free_workers, &fd);
        pthread_mutex_unlock(&free_workers_mutex);
        pthread_mutex_lock(&fd_mutex);
        rc = send(fd, buffer, sizeof(buffer), 0);
        pthread_mutex_unlock(&fd_mutex);
        if (rc < 0) {
          end_program = 1;
          break;
        }
      }
    }
  }
  return NULL;
}
void *process_create(void *args) {
  ARGS *tmp = args;
  queue *payloads = tmp->payloads;
  queue *free_workers = tmp->free_workers;
  float load;
  int number_of_processes = 1;
  int fd, rc;
  char buffer[BUFFER_SIZE];
  while (!end_program) {
    load = (float)getSize(payloads) / (float)QUEUE_SIZE;
    if (load <= 0.3 && number_of_processes > 1 && getSize(free_workers) > 0) {
      pthread_mutex_lock(&free_workers_mutex);
      dequeue(free_workers, &fd);
      pthread_mutex_unlock(&free_workers_mutex);
      pthread_mutex_lock(&console_mutex);
      printf("Got fd of free worker to shutdown: %d\n", fd);
      pthread_mutex_unlock(&console_mutex);
      strcpy(buffer, "end");
      pthread_mutex_lock(&fd_mutex);
      rc = send(fd, buffer, sizeof(buffer), 0);
      pthread_mutex_unlock(&fd_mutex);
      if (rc < 0) {
        pthread_mutex_lock(&console_mutex);
        printf("Error in create process failed send to close process fd: %d\n",
               fd);
        pthread_mutex_unlock(&console_mutex);
      }
      number_of_processes--;
      pthread_mutex_lock(&console_mutex);
      printf("Current number of processes: %d\n", number_of_processes);
      pthread_mutex_unlock(&console_mutex);

    } else if (load >= 0.7) {
      number_of_processes++;
      pthread_mutex_lock(&console_mutex);
      printf("Current number of processes: %d\n", number_of_processes);
      pthread_mutex_unlock(&console_mutex);
      if (fork() == 0) {
        execv("./process", NULL);
      }
    }
    sleep(1);
  }
  return NULL;
}

#include "../../include/queue.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct queue *create_queue() {
  struct queue *queue = (struct queue *)malloc(sizeof(struct queue));
  queue->head = queue->tail = NULL;
  queue->size = 0;
  queue->max_size = QUEUE_SIZE;
  return queue;
}
int enqueue(struct queue *queue, char *payload) {
  if (queue->max_size == queue->size) {
    return 1;
  }
  struct queue_node* new_node = (struct queue_node*)malloc(sizeof(struct queue_node));
  strcpy(new_node->payload,payload);
  new_node->next = NULL;
  if (queue->tail != NULL) {
    queue->tail->next = new_node;
  }
  queue->tail = new_node;
  if (queue->head == NULL) {
    queue->head = new_node;
  }
  queue->size++;
  return 0;
}
char *dequeue(struct queue *queue) {
  if (queue->head == NULL || queue->tail == NULL || queue->size == 0) {
    return NULL;
  }
  char *ret;
  strcpy(ret,queue->head->payload);
  if (queue->size == 1) {
    free(queue->head);
    queue->head = NULL;
    queue->tail = NULL;
  } else {
    struct queue_node *tmp = queue->head;
    queue->head = queue->head->next;
    free(tmp);
  }
  queue->size--;

  return ret;
}
void free_queue(struct queue *queue) {
  if (queue->head == NULL || queue->tail == NULL || queue->size == 0) {
    free(queue);
    return;
  }
  struct queue_node *current = queue->head;
  for (; current != NULL;) {
    struct queue_node *tmp = current;
    current = current->next;
    free(tmp);
  }
  free(queue);
}
void print_queue(struct queue *queue) {
  struct queue_node *current = queue->head;
  int i = 1;
  for (; current != NULL; current = current->next, i++) {
    printf("%d. Payload: %s\n", i, current->payload);
  }
}

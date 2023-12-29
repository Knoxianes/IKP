#include "../../include/queue.h"


struct queue* create_queue();
void enqueue(struct queue* queue,struct queue_node* new_node);
struct queue_node* dequeue(struct queue* queue);
void free_queue(struct queue* queue);
void print_queue(struct queue* queue);

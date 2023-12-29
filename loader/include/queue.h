#ifndef __QUEUE
#define __QUEUE
#include "consts.h"

struct queue{
    int size;
    int max_size;
    struct queue_node* head;
    struct queue_node* tail;
};
struct queue_node{
    char payload[BUFFER_SIZE];
    struct queue_node* next;
};

typedef struct queue Queue;
typedef struct queue_node QueueNode;


struct queue* create_queue();
void enqueue(struct queue*,struct queue_node*);
struct queue_node* dequeue(struct queue*);
void free_queue(struct queue*);
void print_queue(struct queue*);




#endif // !__QUEUE

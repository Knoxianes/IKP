#include "../include/queue.h"
#include "stdio.h"
#include "stdlib.h"

int main(int argc, char *argv[]) {
    Queue *queue = create_queue();
    printf("\n---Enqueue---\n");
    enqueue(queue,"Ovo je test");        
    print_queue(queue);
    printf("Size: %d, Max-size: %d\n",queue->size,queue->max_size);

    printf("\n---Enqueue 2---\n");
    enqueue(queue,"Ovo je test 2");        
    print_queue(queue);
    printf("Size: %d, Max-size: %d\n",queue->size,queue->max_size);

    printf("\n---Enqueue 3---\n");
    enqueue(queue,"Ovo je test 3");        
    print_queue(queue);
    printf("Size: %d, Max-size: %d\n",queue->size,queue->max_size);

    printf("\n---Dequeue 1---\n");
    char *payload = dequeue(queue);
    print_queue(queue);
    printf("Ovo je dekuovano: %s\n",payload);
    printf("Size: %d, Max-size: %d\n",queue->size,queue->max_size);

    printf("\n---Dequeue 2---\n");
     payload = dequeue(queue);
    print_queue(queue);
    printf("Ovo je dekuovano: %s\n",payload);
    printf("Size: %d, Max-size: %d\n",queue->size,queue->max_size);

    printf("\n---Dequeue 3---\n");
    payload = dequeue(queue);
    print_queue(queue);
    printf("Ovo je dekuovano: %s\n",payload);
    printf("Size: %d, Max-size: %d\n",queue->size,queue->max_size);


    printf("\n---Dequeue 4---\n");
    payload = dequeue(queue);



    free_queue(queue);
    return 0;
}

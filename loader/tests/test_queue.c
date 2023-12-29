#include "../include/queue.h"


int main(int argc, char *argv[]) {
    Queue *queue = create_queue();
    
    free_queue(queue);
    return 0;
}

#include "../include/linkedlist.h"

int main(int argc, char *argv[]) {
    LinkedList *list = create_list();
    
    free_list(list);
    return 0;
}

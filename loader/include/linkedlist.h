#ifndef __LINKED_LIST
#define __LINKED_LIST

struct list_node{
    int sd; // stream descriptor
    int in_use;// if process is in use
    struct list_node* next;
    struct list_node* prev;
};

struct linkedlist{
    int size; // count of list
    struct list_node* head;
    struct list_node* tail;
};

typedef struct linkedlist LinkedList;
typedef struct list_node ListNode;

struct linkedlist* create_list();
void insert_at_beginning (struct linkedlist*, struct list_node*);
void insert_at_end(struct linkedlist*, struct list_node*);
struct list_node* get_node(struct linkedlist*, int);
struct list_node* get_index(struct linkedlist*, int);
struct list_node* find_first_free(struct linkedlist*);
int delete_node(struct linkedlist*);
void delete_specific_node(struct linkedlist*, int);
void free_list(struct linkedlist*);
void print_list(struct linkedlist*);

#endif // !__LINKED_LIST

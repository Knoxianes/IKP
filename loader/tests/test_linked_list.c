#include "../include/linkedlist.h"
#include "stdio.h"
#include "stdlib.h"

int main(int argc, char *argv[]) {
  LinkedList *list = create_list();
  printf("\n---Insert at beginning---");
  ListNode *listnode1 = (ListNode *)malloc(sizeof(ListNode));
  listnode1->sd = 1;
  listnode1->in_use = 0;

  insert_at_beginning(list, listnode1);
  print_list(list);
  printf("%d\n", list->size);

  printf("\n---Insert at end---");
  ListNode *listnode2 = (ListNode *)malloc(sizeof(ListNode));
  listnode2->sd = 2;
  listnode2->in_use = 0;

  insert_at_end(list, listnode2);
  print_list(list);
  printf("%d\n", list->size);

  printf("\n---Insert at beginning again---");
  ListNode *listnode3 = (ListNode *)malloc(sizeof(ListNode));
  listnode3->sd = 3;
  listnode3->in_use = 0;

  insert_at_beginning(list, listnode3);
  print_list(list);
  printf("%d\n", list->size);

  printf("\n---Get node with sd2---");
  ListNode *listnode4 = get_node(list, 2);
  printf("\nSD: %d\nInUse:%d\n", listnode4->sd, listnode4->in_use);

  printf("\n---Get node by index 1---");
  listnode4 = get_index(list, 1);
  printf("\nSD: %d\nInUse:%d\n", listnode4->sd, listnode4->in_use);

  printf("\n---Get node by index 10---");
  listnode4 = get_index(list, 10);

  printf("\n---Find first that is free---");
  listnode4 = find_first_free(list);
  printf("\nSD: %d\nInUse:%d\n", listnode4->sd, listnode4->in_use);

  printf("\n--Delete first node that is not in use---");
  int sd = delete_node(list);
  print_list(list);
  printf("Size:%d, Deleted sd: %d\n", list->size, sd);

  printf("\n--Delete first node that is not in use---");
  sd = delete_node(list);
  print_list(list);
  printf("Size:%d, Deleted sd: %d\n", list->size, sd);

  printf("\n--Delete first node that is not in use---");
  sd = delete_node(list);
  print_list(list);
  printf("\nSize:%d, Deleted sd: %d\n", list->size, sd);

  printf("\n--Delete first node that is not in use---");
  sd = delete_node(list);
  print_list(list);
  printf("\nSize:%d, Deleted sd: %d\n", list->size, sd);

  free_list(list);
  return 0;
}

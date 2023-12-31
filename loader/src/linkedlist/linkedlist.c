#include "../../include/linkedlist.h"
#include "stdio.h"
#include "stdlib.h"

struct linkedlist *create_list() {
  struct linkedlist *list =
      (struct linkedlist *)malloc(sizeof(struct linkedlist));
  list->size = 0;
  list->head = list->tail = NULL;
  return list;
}
void insert_at_beginning(struct linkedlist *ll, int sd) {
  ListNode *new_node = (ListNode *)malloc(sizeof(ListNode));
  new_node->sd = sd;
  new_node->in_use = 0;
  if (ll->head != NULL) {
    new_node->next = ll->head;
    new_node->prev = NULL;
    ll->head->prev = new_node;
    ll->head = new_node;
    ll->size++;
    return;
  }

  ll->head = new_node;
  ll->tail = new_node;
  new_node->next = NULL;
  new_node->prev = NULL;
  ll->size++;
}
void insert_at_end(struct linkedlist *ll, int sd) {
  ListNode *new_node = (ListNode *)malloc(sizeof(ListNode));
  printf("%d\n",sd);
  new_node->sd = sd;
  new_node->in_use = 0;
  if (ll->tail != NULL) {
    new_node->next = NULL;
    new_node->prev = ll->tail;
    ll->tail->next = new_node;
    ll->tail = new_node;
    ll->size++;
    return;
  }

  ll->head = new_node;
  ll->tail = new_node;
  new_node->next = NULL;
  new_node->prev = NULL;
  ll->size++;
}
struct list_node *get_node(struct linkedlist *ll, int node_to_find) {
  if (ll->head == NULL || ll->size == 0 || ll->tail == NULL) {
    return NULL;
  }

  struct list_node *current;
  for (current = ll->head; current != NULL; current = current->next) {
    if (current->sd == node_to_find) {
      return current;
    }
  }

  return NULL;
}
struct list_node *get_index(struct linkedlist *ll, int index) {
  if (index >= ll->size) {
    return NULL;
  }
  if (ll->head == NULL || ll->tail == NULL) {
    return NULL;
  }
  struct list_node *ret = ll->head;
  for (int i = 0; i < index; i++, ret = ret->next)
    ;
  return ret;
}
// Finds first process that is not in use and returns it
struct list_node *find_first_free(struct linkedlist *ll) {
  if (ll->head == NULL || ll->size == 0 || ll->tail == NULL) {
    return NULL;
  }
  struct list_node *current;
  for (current = ll->head; current != NULL; current = current->next) {
    if (current->in_use == 0) {
      return current;
    }
  }
  return NULL;
}

// Deletes first node that is no in use and returns stream descriptor
int delete_node(struct linkedlist *ll) {
  if (ll->head == NULL || ll->size == 0 || ll->tail == NULL) {
    return -1;
  }

  struct list_node *to_delete = find_first_free(ll);
  if (to_delete == NULL) {
    return -1;
  }
  int ret = to_delete->sd;
  if (ll->size == 1) {
    ll->head = NULL;
    ll->tail = NULL;
    free(to_delete);
  } else {
    if (to_delete->prev == NULL) {
      ll->head = to_delete->next;
      ll->head->prev = NULL;
      to_delete->next = NULL;
      free(to_delete);
    } else if (to_delete->next == NULL) {
      ll->tail = to_delete->prev;
      ll->tail->next = NULL;
      to_delete->prev = NULL;
      free(to_delete);
    } else {
      to_delete->next->prev = to_delete->prev;
      to_delete->prev->next = to_delete->next;
      to_delete->prev = to_delete->next = NULL;
      free(to_delete);
    }
  }
  ll->size--;
  return ret;
}
void delete_specific_node(struct linkedlist *ll, int sd) {
  if (ll->head == NULL || ll->size == 0 || ll->tail == NULL) {
    return;
  }
  struct list_node *to_delete = ll->head;
  for (; to_delete != NULL; to_delete = to_delete->next) {
    if (to_delete->sd == sd) {
      if (ll->size == 1) {
        ll->head = NULL;
        ll->tail = NULL;
        free(to_delete);
      } else {
        if (to_delete->prev == NULL) {
          ll->head = to_delete->next;
          ll->head->prev = NULL;
          to_delete->next = NULL;
          free(to_delete);
        } else if (to_delete->next == NULL) {
          ll->tail = to_delete->prev;
          ll->tail->next = NULL;
          to_delete->prev = NULL;
          free(to_delete);
        } else {
          to_delete->next->prev = to_delete->prev;
          to_delete->prev->next = to_delete->next;
          to_delete->prev = to_delete->next = NULL;
          free(to_delete);
        }
      }
      ll->size--;
      break;
    }
  }
}
void free_list(struct linkedlist *ll) {
  if (ll->head == NULL || ll->size == 0 || ll->tail == NULL) {
    free(ll);
    return;
  }

  struct list_node *current = ll->head;
  for (; current != NULL;) {
    struct list_node *tmp = current;
    current = current->next;
    free(tmp);
  }
  free(ll);
}
void print_list(struct linkedlist *ll) {
  struct list_node *current;
  for (current = ll->head; current != NULL; current = current->next) {
    printf("\nProcess stream descriptor: %d\nProcess in use: %d\n", current->sd,
           current->in_use);
  }
}

#include <stdio.h>
#include <stdlib.h>
#include "simos.h"

node_t *head = NULL;

void enqueue(request_t req)
{ node_t *new_node = malloc(sizeof(node_t));
  if (!new_node) return;

  new_node->request = req;
  new_node->next = head;
  head = new_node;
}

request_t* dequeue()
{ node_t *current, *prev = NULL;
  request_t *retreq = NULL;

  if (head == NULL) return NULL;

  current = head;
  while (current->next != NULL)
  { prev = current; current = current->next; }
  retreq = &(current->request);

  if (prev) prev->next = NULL;
  else head = NULL;

  return retreq;
}

void dump_queue()
{ node_t *current = NULL;

  printf("Dump Request Queue:\n");
  current = head;
  while (current != NULL)
  { printf("%s, %d, %d\n",
            //current->request.client_id,
            current->request.filename,
            current->request.sockfd,
            current->request.port);
    current = current->next;
  }
}


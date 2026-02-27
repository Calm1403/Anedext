
#include "mappings.h"

#include <stdlib.h>

node_t*
add_node(node_t** node, int (*map)(void), char key)
{
  node_t* new;
  if ((new = malloc(sizeof *new)) == NULL)
    return NULL;

  new->mapping = map;
  new->key = key;
  new->next = NULL;

  if (node == NULL)
    return new;

  node_t* temp;
  if ((temp = *node) == NULL)
    return (*node = new);

  while (temp->next != NULL)
    temp = temp->next;

  temp->next = new;
  return temp;
}

void
free_list(node_t* node)
{
  node_t* temp;
  do
  {
    temp = node->next;
    free(node);
    node = temp;
  } while (temp != NULL);
}

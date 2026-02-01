
#ifndef MAP_H
#define MAP_H

// Structure for itterating maps.
struct node_s
{
  int (*mapping)(void);
  unsigned char key;
  struct node_s* next;
};

typedef struct node_s node_t;

extern node_t*
add_node(node_t**, int (*)(void), char);

extern void
free_list(node_t*);

#endif

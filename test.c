
#include <stdio.h>
#include <stdlib.h>
int
main(void)
{
  char* string = "ABCD\n";
  int pos = 3, size = 6;

  char* shift = malloc(size - 1);
  for (int i = 0; i < pos; i++)
    shift[i] = string[i];

  for (int i = pos; i < size; i++)
    shift[i] = string[i + 1];

  printf("%s", shift);

  free(shift);
}


#ifndef FILE_H
#define FILE_H

#include <stdbool.h>
#include <stdio.h>

typedef struct file_buffer_s
{
  char* file_name;
  FILE* file_pointer;
  char* buffer;
  size_t size;
  bool save;
} file_buffer_t;

extern file_buffer_t*
create_fb(char*);

extern void
deallocate_fb(file_buffer_t* fb);

#endif

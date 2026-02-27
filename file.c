#include "file.h"
#include <stdlib.h>
#include <unistd.h>

file_buffer_t*
create_fb(char* location)
{
  file_buffer_t* fb;
  if ((fb = malloc(sizeof *fb)) == NULL)
    return NULL;

  fb->file_name = location;
  if ((fb->file_pointer = fopen(fb->file_name, "r+")) == NULL)
  {
    perror("\x1b[H\x1b[Jfopen failure");
    free(fb);
    return NULL;
  }

  fseek(fb->file_pointer, 0L, SEEK_END);
  if ((fb->size = (ftell(fb->file_pointer) + 1)) == 0)
    return NULL;
  fseek(fb->file_pointer, 0L, SEEK_SET);

  if ((fb->buffer = malloc(fb->size)) == NULL)
  {
    fclose(fb->file_pointer);
    free(fb);
    return NULL;
  }

  fb->buffer[fb->size - 1] = '\0';
  fread(fb->buffer, 1, fb->size, fb->file_pointer);
  return fb;
}

void
deallocate_fb(file_buffer_t* fb)
{
  free(fb->buffer);
  fclose(fb->file_pointer);
  free(fb);
}

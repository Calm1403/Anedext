
#include "file.h"

#include <stdlib.h>

file_buffer_t*
create_fb(char* location)
{
  file_buffer_t* fb;
  if ((fb = malloc(sizeof *fb)) == NULL)
    return NULL;
  fb->file_name = location;
  fb->save = false;
  if ((fb->file_pointer = fopen(fb->file_name, "r+")) == NULL)
  {
    free(fb);
    return NULL;
  }
  fseek(fb->file_pointer, 0L, SEEK_END);
  if ((fb->size = ftell(fb->file_pointer)) == 0)
    return NULL;
  fseek(fb->file_pointer, 0L, SEEK_SET);
  if ((fb->buffer = malloc(fb->size + 1)) == NULL)
  {
    fclose(fb->file_pointer);
    free(fb);
    return NULL;
  }
  fb->buffer[fb->size] = '\0';
  fread(fb->buffer, 1, fb->size, fb->file_pointer);
  return fb;
}

void
deallocate_fb(file_buffer_t* fb)
{
  if (fb->save == true)
  {
    fseek(fb->file_pointer, 0L, SEEK_SET);
    fwrite(fb->buffer, 1, fb->size, fb->file_pointer);
    printf("\x1b[H\x1b[JSaved %li bytes to %s.\n", fb->size, fb->file_name);
  }
  free(fb->buffer);
  fclose(fb->file_pointer);
  free(fb);
}

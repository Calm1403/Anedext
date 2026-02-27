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
    perror("\x1b[H\x1b[JFopen failure");
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

int
save_fb(file_buffer_t* fb)
{
  fb->file_pointer = freopen(fb->file_name, "w", fb->file_pointer);
  if (fb->file_pointer == NULL)
  {
    perror("\x1b[H\x1b[JFreopen failed");
    return 1;
  }

  if (fwrite(fb->buffer, fb->size, 1, fb->file_pointer) == -1)
  {
    perror("\x1b[H\x1b[JFwrite failed");
    return 1;
  }

  return 0;
}
void
deallocate_fb(file_buffer_t* fb)
{
  free(fb->buffer);
  fclose(fb->file_pointer);
  free(fb);
}

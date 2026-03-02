#include "file.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

file_buffer_t*
create_fb(char* location)
{
  file_buffer_t* fb;
  if ((fb = malloc(sizeof *fb)) == NULL)
    return NULL;

  *fb = (file_buffer_t){ location, NULL, NULL, 0 };
  if ((fb->file_pointer = fopen(fb->file_name, "r+")) == NULL)
  {
    perror("\x1b[H\x1b[JFopen failed..\n\nReason");
    goto fail;
  }

  fseek(fb->file_pointer, 0L, SEEK_END);
  if ((fb->size = (ftell(fb->file_pointer) + 1)) == 0)
  {
    perror("\x1b[H\x1b[JFtell failed..\n\nReason");
    goto fail;
  }
  fseek(fb->file_pointer, 0L, SEEK_SET);

  if ((fb->buffer = malloc(fb->size)) == NULL)
  {
    perror("\x1b[H\x1b[JMalloc failed..\n\nReason");
    goto fail;
  }

  fb->buffer[fb->size - 1] = '\0';
  if (fread(fb->buffer, 1, fb->size - 1, fb->file_pointer) != fb->size - 1)
  {
    perror("\x1b[H\x1b[JFread failed..\n\nReason");
    goto fail;
  }

  return fb;

fail:
  if (fb->file_pointer != NULL)
    fclose(fb->file_pointer);

  free(fb);

  return NULL;
}

int
save_fb(file_buffer_t* fb)
{
  fb->file_pointer = freopen(fb->file_name, "w", fb->file_pointer);
  if (fb->file_pointer == NULL)
    goto fail;

  if (fwrite(fb->buffer, fb->size - 1, 1, fb->file_pointer) == 0)
  {
    if (errno == 0)
      return 0;

    goto fail;
  }

  return 0;

fail:
  perror("\x1b[H\x1b[JSave failed..\n\nReason");
  return 1;
}

void
deallocate_fb(file_buffer_t* fb)
{
  free(fb->buffer);

  if (fb->file_pointer != NULL)
    fclose(fb->file_pointer);

  free(fb);
}

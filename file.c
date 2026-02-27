
/*
  Linux manual page regarding EINTR on close:

    "Retrying the close() after a failure return is the wrong thing to
    do, since this may cause a reused file descriptor from another
    thread to be closed."

  I'm not multithreading, using blocking I/o, fair enough, but what's
  interesting about it this about EINTR:

    "The caller must then once more use close() to close
    the file descriptor, to avoid file descriptor leaks.

    This divergence in implementation behaviors provides a difficult hurdle
    for portable applications, since on many implementations, close()
    must not be called again after an EINTR error, and on at least
    one, close() must be called again."
*/

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
    if (fclose(fb->file_pointer) == EOF)
    { /*  NOTE : Not entirely sure about this. */
      perror("\x1b[H\x1b[JFopen failure");
      if (errno == EINTR)
        close(fileno(fb->file_pointer));
    }
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

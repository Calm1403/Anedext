#include "file.h"
#include "rets.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define fb_failure(prompt)                                                     \
  {                                                                            \
    perror(prompt);                                                            \
    if (fb->file_pointer != NULL)                                              \
      fclose(fb->file_pointer);                                                \
                                                                               \
    free(fb);                                                                  \
    ret(NULL);                                                                 \
  }

file_buffer_t*
create_fb(char* location)
{
  file_buffer_t* fb;

  if ((fb = malloc(sizeof *fb)) == NULL)
    fb_failure("\x1b[H\x1b[JMalloc failed..\n\nReason");

  *fb = (file_buffer_t){ location, NULL, NULL, 0 };
  if ((fb->file_pointer = fopen(fb->file_name, "r")) == NULL)
    fb_failure("\x1b[H\x1b[JFopen failed..\n\nReason");

  if (fseek(fb->file_pointer, 0L, SEEK_END) == 1)
    fb_failure("\x1b[H\x1b[JFseek failed..\n\nReason");

  if ((fb->size = (ftell(fb->file_pointer) + 1)) == 0)
    fb_failure("\x1b[H\x1b[JFtell failed..\n\nReason");

  if (fseek(fb->file_pointer, 0L, SEEK_SET) == 1)
    fb_failure("\x1b[H\x1b[JFseek failed..\n\nReason");

  if ((fb->buffer = malloc(fb->size)) == NULL)
    fb_failure("\x1b[H\x1b[JMalloc failed..\n\nReason");

  fb->buffer[fb->size - 1] = '\0';
  if (fread(fb->buffer, 1, fb->size - 1, fb->file_pointer) != fb->size - 1)
    fb_failure("\x1b[H\x1b[JFread failed..\n\nReason");

  ret(fb);
}

int
save_fb(file_buffer_t* fb)
{
  fb->file_pointer = freopen(fb->file_name, "w", fb->file_pointer);
  if (fb->file_pointer == NULL)
    retape(1, "\x1b[H\x1b[JSave failed..\n\nReason");

  if (fwrite(fb->buffer, fb->size - 1, 1, fb->file_pointer) == 0)
  {
    if (errno == 0)
      ret(0);

    retape(1, "\x1b[H\x1b[JSave failed..\n\nReason");
  }

  ret(0);
}

void
deallocate_fb(file_buffer_t* fb)
{
  free(fb->buffer);

  if (fb->file_pointer != NULL)
    fclose(fb->file_pointer);

  free(fb);
}

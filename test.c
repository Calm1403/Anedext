
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

typedef struct
{
  size_t b;
  size_t l;
} fb_size_t;

typedef struct
{
  fb_size_t size;
  FILE* file_pointer;
  char* file_name;
  char* buffer;
  int page;
} fb_t;

#define B_DEFAULT 10 // Breadth default.
#define L_DEFAULT 10 // Length default.

/*
  What am I doing?

  I want to write an interface that refreshes a block buffer.

  I'll have two functions, one creates the interface; one refreshes
  it. It makes it seem convoluted to combine those two functionalities.
*/

static fb_t*
create_fb(char* location)
{
  fb_t* fb;
  if ((fb = malloc(sizeof *fb)) == NULL)
    goto fail;

  if ((fb->file_pointer = fopen(location, "r+")) == NULL)
    goto fail;

  fb->size = (fb_size_t){ .b = B_DEFAULT, .l = L_DEFAULT };
  if ((fb->buffer = malloc(fb->size.l * fb->size.b + 1)) == NULL)
    goto fail;

  fseek(fb->file_pointer, 0L, SEEK_END);

  off_t size_r;
  if ((size_r = ftello(fb->file_pointer) == -1))
    goto fail;

  if ((fb->size.l * fb->size.b > size_r) == 0) // Page is zero in this case.
    fb->size.b = fb->size.b - (size_r / fb->size.l);

  fseek(fb->file_pointer, 0L, SEEK_SET);

  fread(fb->buffer, 1, fb->size.l * fb->size.b, fb->file_pointer);

  fb->buffer[fb->size.l * fb->size.b] = '\0';
  fb->file_name = location;
  fb->page = 1;

  return fb;

fail:
  fclose(fb->file_pointer);
  free(fb);
  return NULL;
}

static void
page_refresh_fb(fb_t* fb, unsigned int page_no)
{
}

static struct termios old_state;

static signed
t_set_baked_mode(void)
{
  if (tcgetattr(STDIN_FILENO, &old_state) == -1)
    return -1;

  struct termios new_state;
  new_state.c_iflag &= ~(IGNBRK | IGNPAR | BRKINT | PARMRK | ISTRIP | INLCR);
  new_state.c_iflag &= ~(IGNCR | ICRNL | IXON | INPCK);
  new_state.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  new_state.c_cflag &= ~(CSIZE | PARENB);
  new_state.c_cflag |= CS8;
  new_state.c_oflag &= ~(OLCUC);
  new_state.c_oflag |= (OCRNL | ONLCR | OPOST);
  new_state.c_cc[VTIME] = 1; // Time for a read before return.
  new_state.c_cc[VMIN] = 0;  // Min number of characters before return.

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_state) == -1)
    return -1;

  return 0;
}

static signed
t_reset_mode(void)
{
  return tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_state);
}

static int
read_input(int (*process_input)(unsigned char))
{
  char pressed = 0;
  if (t_set_baked_mode() == -1)
    return 1;

  int nread;
  while ((nread = read(STDIN_FILENO, &pressed, 1)) >= 0)
  {
    if (nread == 1 && process_input(pressed) == 1)
      break;
  }
  if (t_reset_mode() == -1 || nread == -1)
    return 1;

  return 0;
}

int
main(void)
{
  return 0;
}

/*
  File setup:
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
*/

/*
  Size attainment:
  fseek(fb->file_pointer, 0L, SEEK_END);
  if ((fb->size = ftell(fb->file_pointer)) == 0)
    return NULL;
  fseek(fb->file_pointer, 0L, SEEK_SET);
*/

/*
  Buffer allocation.
  if ((fb->buffer = malloc(fb->size + 1)) == NULL)
  {
    fclose(fb->file_pointer);
    free(fb);
    return NULL;
  }
  fb->buffer[fb->size] = '\0';
  fread(fb->buffer, 1, fb->size, fb->file_pointer);
*/

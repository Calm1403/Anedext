
#include "process.h"
#include "file.h"
#include "input.h"
#include "mappings.h"
#include "rets.h"
#include "window.h"

#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

// Print state.
#define print_state                                                            \
  printf("\x1b[H\x1b[J\x1b[%i;0HPos: %li | Mode: %s | Size: %li | Poll: "      \
         "%i\x1b[H%s\n",                                                       \
         editor.ws.ws_row,                                                     \
         editor.pos,                                                           \
         modes[editor.mode],                                                   \
         editor.fb->size,                                                      \
         editor.poll,                                                          \
         editor.fb->buffer)

// Return and print state.
#define retaps(code)                                                           \
  do                                                                           \
  {                                                                            \
    print_state;                                                               \
    ret(code);                                                                 \
  } while (0)

static struct editor_s
{
  struct winsize ws;
  file_buffer_t* fb;
  node_t* key_maps;
  size_t pos;
  int poll;
  int mode;
} editor;

enum
{
  SPECIAL_NT = 2, // Special input; non program terminating.
  SPECIAL_T = 1,  // Special input; program terminating.
  NORMAL = 0,     // Non special input; printable characters.
};

static char* modes[2] = { "normal", "insert" };

/*
   TODO :

  static int
  display(void)
  {
    ... code for windowing.
    ret(1);
  }
*/

static int
increase_buffer(void)
{
  // editor.fb->size - 2 is the last non null character.
  if (editor.fb->size == 1 || editor.pos == editor.fb->size - 2)
  { // Ten is just the default rescale size.
    editor.fb->buffer = realloc(editor.fb->buffer, editor.fb->size += 10);
    if (editor.fb->buffer == NULL)
      retape(1, "\x1b[h\x1b[jRealloc failed..\n\nReason");

    // Memset the rest of the buffer because of crap contents.
    memset(&editor.fb->buffer[editor.pos], ' ', editor.fb->size - editor.pos);

    editor.fb->buffer[editor.fb->size - 1] = '\0';
  }
  ret(0);
}

static void
go_left(void)
{
  if ((editor.pos - 1) == -1)
    editor.pos = editor.fb->size - 2;
  else
    --editor.pos;
}

static void
go_right(void)
{
  if ((editor.pos + 1) > (editor.fb->size - 2))
    editor.pos = 0;
  else
    ++editor.pos;
}

static int
handle_h(void)
{ // This ensures that a user can't move about freely when size = 1.
  if (editor.fb->size == 1)
    ret(0);

  go_left();
  retaps(0);
}

static int
handle_l(void)
{ // This ensures that a user can't move about freely when size = 1.
  if (editor.fb->size == 1)
    ret(0);

  go_right();
  retaps(0);
}

static int
handle_0x1b(void)
{
  if (editor.mode == 1)
  {
    editor.mode = 0;
    retaps(0);
  }
  retapp(1, "\x1b[H\x1b[JExiting.. bye bye!\n", stdout);
}

static int
handle_0x09(void)
{
  editor.mode = 1;
  retaps(0);
}

static int
handle_0x13(void)
{
  if (save_fb(editor.fb) == 1)
    ret(1);

  fputs("\a", stdout);

  retaps(0);
}

static int
handle_0x7f(void)
{
  if (editor.mode == 0 || editor.fb->size == 1)
    ret(0);

  go_left();
  for (size_t i = editor.pos; i + 1 < editor.fb->size; ++i)
    editor.fb->buffer[i] = editor.fb->buffer[i + 1];

  editor.fb->buffer = realloc(editor.fb->buffer, editor.fb->size -= 1);
  if (editor.fb->buffer == NULL)
    retapp(1, "\x1b[H\x1b[JRealloc failed..\n", stderr);

  // Weird bug fix; user could remove null byte.
  if (editor.pos != 0)
  {
    if (editor.pos == editor.fb->size - 1)
      editor.pos -= 1;
  }
  retaps(0);
}

//  TODO : Change this; accomodate for newline input with sophistication.

#define poll_cond                                                              \
  ((editor.ws.ws_row - 1) * editor.ws.ws_col * (editor.poll + 1)) < editor.pos

#define insert(input)                                                          \
  do                                                                           \
  {                                                                            \
    if (editor.mode == 0)                                                      \
      ret(0);                                                                  \
                                                                               \
    if (increase_buffer() == 1)                                                \
      ret(1);                                                                  \
                                                                               \
    editor.fb->buffer[editor.pos] = input;                                     \
    go_right();                                                                \
                                                                               \
    if (poll_cond)                                                             \
      ++editor.poll;                                                           \
                                                                               \
    retaps(0);                                                                 \
  } while (0)

static int
handle_normal(unsigned char input)
{
  insert(input);
}

static int
handle_0x0d(void)
{
  insert('\n');
}

static node_t*
register_maps(void)
{
  if (add_node(&editor.key_maps, handle_0x1b, 0x1b) == NULL)
    goto free;

  if (add_node(&editor.key_maps, handle_0x09, 0x09) == NULL)
    goto free;

  if (add_node(&editor.key_maps, handle_0x7f, 0x7f) == NULL)
    goto free;

  if (add_node(&editor.key_maps, handle_0x13, 0x13) == NULL)
    goto free;

  if (add_node(&editor.key_maps, handle_0x0d, 0x0d) == NULL)
    goto free;

  if (add_node(&editor.key_maps, handle_h, 'h') == NULL)
    goto free;

  if (add_node(&editor.key_maps, handle_l, 'l') == NULL)
    goto free;

  ret(editor.key_maps);

free:
  free_list(editor.key_maps);
  ret(NULL);
}

static int
check_maps(unsigned char input)
{
  node_t* node = editor.key_maps;
  do
  {
    if (node->key == input)
    {
      if (editor.mode == 1)
      { //  TODO : Generalise this; not through maps.
        if ('a' < node->key && node->key < 'z')
          ret(NORMAL);

        if ('A' < node->key && node->key < 'Z')
          ret(NORMAL);
      }

      if (node->mapping() == 1)
        ret(SPECIAL_T);
    }
  } while (node->key != input && (node = node->next) != NULL);

  if (node == NULL)
    ret(NORMAL);

  ret(SPECIAL_NT);
}

static int
process_input(unsigned char input)
{
  switch (check_maps(input))
  {
    case SPECIAL_T:
      ret(1);

    case SPECIAL_NT:
      ret(0);

    case NORMAL:
      ret(handle_normal(input));
  }
  ret(0);
}

static int
editor_initialise(char* location)
{
  editor = (struct editor_s){ 0 };

  if ((editor.fb = create_fb(location)) == NULL)
    ret(1);

  if ((editor.key_maps = register_maps()) == NULL)
    ret(1);

  if (get_size(&editor.ws) == 1)
    ret(1);

  ret(0);
}

static void
editor_uninitialise(void)
{
  free_list(editor.key_maps);

  deallocate_fb(editor.fb);
}

static int
initialisation(char* location)
{
  if (editor_initialise(location) == 1)
    ret(1);

  print_state;

  if (read_input(&process_input) == 1)
  {
    editor_uninitialise();
    ret(1);
  }

  editor_uninitialise();
  ret(0);
}

int
begin_processing(char* location)
{
  fputs("\x1b[?25l", stdout);

  if (initialisation(location) == 1)
  {
    fputs("\x1b[?25h", stdout);
    ret(1);
  }

  fputs("\x1b[?25h", stdout);
  ret(0);
}


#include "process.h"
#include "file.h"
#include "input.h"
#include "mappings.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Print state.
#define print_state                                                            \
  printf("\x1b[H\x1b[J%s\n[Pos: %li | Mode: %s | Size: %li]\n",                \
         state.fb->buffer,                                                     \
         state.pos,                                                            \
         modes[state.mode],                                                    \
         state.fb->size);

// Return and print state.
#define retaps(code)                                                           \
  do                                                                           \
  {                                                                            \
    print_state;                                                               \
    return code;                                                               \
  } while (0)

// Return and print prompt.
#define retapp(code, prompt, stream)                                           \
  do                                                                           \
  {                                                                            \
    fputs(prompt, stream);                                                     \
    return code;                                                               \
  } while (0)

#define BUF_SCALE 10

static struct state_s
{
  int mode;
  size_t pos;
  node_t* key_maps;
  file_buffer_t* fb;
} state;

enum
{
  SPECIAL_NT = 2, // Special input; non program terminating.
  SPECIAL_T = 1,  // Special input; program terminating.
  NORMAL = 0,     // Non special input; printable characters.
};

static char* modes[2] = { "normal", "insert" };

static void
go_left()
{
  if ((state.pos - 1) == -1)
    state.pos = state.fb->size - 2;
  else
    --state.pos;
}

static void
go_right()
{
  if ((state.pos + 1) > (state.fb->size - 2))
    state.pos = 0;
  else
    ++state.pos;
}

static int
handle_h(void)
{ // This ensures that a user can't move about freely when the size is one.
  if (state.fb->size == 1)
    retaps(0);

  go_left();
  retaps(0);
}

static int
handle_l(void)
{ // This ensures that a user can't move about freely when the size is one.
  if (state.fb->size == 1)
    retaps(0);

  go_right();
  retaps(0);
}

static int
handle_0x1b(void)
{
  if (state.mode == 1)
  {
    state.mode = 0;
    retaps(0);
  }
  retapp(1, "\x1b[H\x1b[JExiting.. bye bye!\n", stdout);
}

static int
handle_0x09(void)
{
  state.mode = 1;
  retaps(0);
}

static int
handle_0x13(void)
{
  if (save_fb(state.fb) == 1)
    retapp(1, "Couldn't save the file buffer..\n", stderr);

  fputs("\a", stdout);

  retaps(0);
}

static int
handle_0x7f(void)
{
  if (state.mode == 0 || state.fb->size == 1)
    retaps(0);

  go_left();

  for (size_t i = state.pos; i + 1 < state.fb->size; ++i)
    state.fb->buffer[i] = state.fb->buffer[i + 1];

  state.fb->buffer = realloc(state.fb->buffer, state.fb->size -= 1);
  if (state.fb->buffer == NULL)
    retapp(1, "\x1b[H\x1b[JRealloc failed..\n", stderr);

  // Weird bug fix; user could remove null byte.
  if (state.pos != 0)
  {
    if (state.pos == state.fb->size - 1)
      state.pos -= 1;
  }

  retaps(0);
}

static int
scale_buffer(void)
{
  // State.fb->size - 2 is the last non null character.
  if (state.fb->size == 1 || state.pos == state.fb->size - 2)
  {
    state.fb->buffer = realloc(state.fb->buffer, state.fb->size += BUF_SCALE);
    if (state.fb->buffer == NULL)
      return 1;

    // Memset the rest of the buffer because of crap contents.
    memset(&state.fb->buffer[state.pos], ' ', state.fb->size - state.pos);

    state.fb->buffer[state.fb->size - 1] = '\0';
  }

  return 0;
}

static int
handle_normal(unsigned char input)
{
  if (state.mode == 0)
    retaps(0);

  if (scale_buffer() == 1)
    retapp(1, "\x1b[h\x1b[jrealloc failed..\n", stderr);

  state.fb->buffer[state.pos] = input;
  go_right();

  retaps(0);
}

static int
handle_0x0d(void)
{
  if (state.mode == 0)
    retaps(0);

  if (scale_buffer() == 1)
    retapp(1, "\x1b[h\x1b[jrealloc failed..\n", stderr);

  state.fb->buffer[state.pos] = '\n';
  go_right();

  retaps(0);
}

static node_t*
register_maps(void)
{
  if (add_node(&state.key_maps, handle_0x1b, 0x1b) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x09, 0x09) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x7f, 0x7f) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x13, 0x13) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x0d, 0x0d) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_h, 'h') == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_l, 'l') == NULL)
    goto free;

  return state.key_maps; // If everything went well..

free:
  free_list(state.key_maps);
  return NULL;
}

static int
check_maps(unsigned char input)
{
  node_t* node = state.key_maps;
  do
  {
    if (node->key == input)
    {
      if (state.mode == 1)
      { //  TODO : Generalise this; not through maps.
        if ('a' < node->key && node->key < 'z')
          return NORMAL;

        if ('A' < node->key && node->key < 'Z')
          return NORMAL;
      }

      if (node->mapping() == 1)
        return SPECIAL_T;
    }
  } while (node->key != input && (node = node->next) != NULL);

  if (node == NULL)
    return NORMAL;

  return SPECIAL_NT;
}

static int
process_input(unsigned char input)
{
  switch (check_maps(input))
  {
    case SPECIAL_T:
      return 1;

    case SPECIAL_NT:
      return 0;

    case NORMAL:
      return handle_normal(input);
  }
  return 0;
}

static int
state_initialise(char* location)
{
  state = (struct state_s){ 0, 0, NULL, NULL };
  if ((state.fb = create_fb(location)) == NULL)
    return 1;

  if ((state.key_maps = register_maps()) == NULL)
    return 1;

  return 0;
}

static void
state_uninitialise(void)
{
  free_list(state.key_maps);

  deallocate_fb(state.fb);
}

static int
initialisation(char* location)
{
  if (state_initialise(location) == 1)
    return 1;

  print_state;
  if (read_input(&process_input) == 1)
    goto fail;

  state_uninitialise();
  return 0;

fail:
  state_uninitialise();
  return 1;
}

int
begin_processing(char* location)
{
  fputs("\x1b[?25l", stdout);
  if (initialisation(location) == 1)
    goto fail;

  fputs("\x1b[?25h", stdout);
  return 0;

fail:
  fputs("\x1b[?25h", stdout);
  return 1;
}

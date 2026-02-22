
/*
  After thinking about it, looking
  at other text editor names, I've decided
  to call this program 'anedext' - a short
  hand word for the phrase 'an editor for text.'

  'ænedekst' is the way I pronounce it.

  This is because it's not a particularly remarkable
  editor and because, well, it's an editor in the sea of many
  editors; this is indicated by the artical 'an,' with the program's
  purpose made clear subsequently afterwards.

  It's also fun to say 'the anedext text editor is an
  editor for text.' Has a nice 'ring.'
*/

#include "process.h"
#include "file.h"
#include "input.h"
#include "mappings.h"

#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Print state.
#define print_state                                                            \
  printf("\x1b[H\x1b[J%s\n[Pos: %li | Mode: %s | Size: %li]\n",                \
         state.fb->buffer,                                                     \
         state.pos + 1,                                                        \
         modes[state.mode],                                                    \
         state.fb->size);

// Return and print state.
#define retaps(code)                                                           \
  {                                                                            \
    print_state;                                                               \
    return code;                                                               \
  }

// Return and print prompt.
#define retapp(code, prompt, stream)                                           \
  {                                                                            \
    fputs(prompt, stream);                                                     \
    return code;                                                               \
  }

typedef struct state_s
{
  int mode;
  size_t pos;
  node_t* key_maps;
  file_buffer_t* fb;
} state_t;

enum
{
  SPECIAL_NT = 2, // Special input; non terminating.
  SPECIAL_T = 1,  // Special input; terminating.
  NORMAL = 0,     // Non special input; printable characters.
};

static state_t state = { 0, 0, NULL, NULL };

static char* modes[2] = { "normal", "insert" };

static void
go_left()
{
  do
  {
    if ((state.pos - 1) == -1)
      state.pos = state.fb->size - 1;
    else
      --state.pos;

  } while (state.fb->buffer[state.pos] == '\0');
}

static void
go_right()
{
  do
  {
    if ((state.pos + 1) > (state.fb->size - 1))
      state.pos = 0;
    else
      ++state.pos;

  } while (state.fb->buffer[state.pos] == '\0');
}

static int
handle_h(void)
{
  go_left();
  retaps(0);
}

static int
handle_l(void)
{
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

/*
  This has actualy proven to be extremely hard to think
  of a solution for.

  For a given buffer, say "AAAAPA\n\0", where A's are the
  contents and P is the current position, the result
  of pressing back space should look like this.

              *0x08 pressed*
  AAAAPA\n\0     ------>     AAAPA\n\0

  The partial solution for this was to include the null byte
  in the size represented by state.fb->size; this
  way I can resize the sequence effectively, just have to be
  sure not to add the null terminator to the buffer later on.
*/

static int
handle_0x08_0x7f(void)
{
  if (state.mode == 0 || state.fb->size == 1)
    retaps(0);

  go_left();
  for (int i = state.pos; i + 1 < state.fb->size; ++i)
    state.fb->buffer[i] = state.fb->buffer[i + 1];

  /*
    "The returned pointer may be the same as p if the allocation was not moved
    (e.g., there was room to expand the allocation in-place), or different from
    p if the allocation was moved to a new address." - Realloc man.

    To emphasise, "(the return may be) different from p if the allocation was
    moved to a new address."

    And better yet, "If the area pointed to was moved, a free(p) is done."

    This code is kinda like playing russian roulet with the memory allocator.

    What fun!
  */

  if (realloc(state.fb->buffer, state.fb->size -= 1) == NULL)
    retapp(1, "\x1b[H\x1b[JRealloc failed..\n", stderr);

  retaps(0);
}

static int
handle_normal(unsigned int input)
{
  if (state.mode == 0)
    retaps(0);

  state.fb->buffer[state.pos] = input;
  go_right();

  retaps(0);
}

static node_t*
register_maps(void)
{
  if (add_node(&state.key_maps, handle_0x1b, 0x1b, false) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x09, 0x09, false) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x08_0x7f, 0x08, false) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x08_0x7f, 0x7f, false) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_h, 'h', true) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_l, 'l', true) == NULL)
    goto free;

  return state.key_maps; // If everything went well..

free:
  free_list(state.key_maps);
  return NULL;
}

static int
check_maps(unsigned int input)
{
  node_t* node = state.key_maps;
  do
  {
    if (node->key == input)
    { // Printables should be usable in insert mode.
      if (node->printable == true && state.mode == 1)
        return NORMAL;

      if (node->mapping() == 1)
        return SPECIAL_T;

      break;
    }
  } while ((node = node->next) != NULL);

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

    default:
      retapp(1, "\x1b[H\x1b[JStrange input..", stderr);
  }
}

static int
state_initialise(char* location)
{
  if ((state.fb = create_fb(location)) == NULL)
    return 1;

  print_state;

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

int
begin_processing(char* location)
{
  if (state_initialise(location) == 1)
    return 1;

  if (read_input(&process_input) == 1)
    return 1;

  state_uninitialise();

  return 0;
}

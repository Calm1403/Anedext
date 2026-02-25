
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

  There are currently numerous problems with this code; it's horribly naive.

  For the most part I assume that arithmetic overflow just isn't a thing; I have
  a plan for determining if it does occur though, which I'll implement later.

  For arithmetic overflow to occur, given two unsigned integers of the same bit
  width 'a' and 'b,' the sum of the two integers would need to surpass the
  maximum representable size that the common integer type may hold.

    a + b > int_max

  We can have int_max equate to (common type)(-1):

    a + b > (common type)(-1)

  Subtracting one of the integers, we get this:

    a > (common type)(-1) - b

  Note that (common type)(-1) - b is also the bitwise (one bit) complement of b.

    a > (2^n - 1) - b => a > ~b, with 'n' being the number of digits used by the
    type.

  Thus, we only need to determine if a number is greater than the complement of
  the other operand (which I assume to be either one, haven't really tested this
  idea).
*/

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

static struct state_s
{
  int mode;
  size_t pos;
  node_t* key_maps;
  file_buffer_t* fb;
} state;

enum
{
  SPECIAL_NT = 2, // Special input; non terminating.
  SPECIAL_T = 1,  // Special input; terminating.
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
{ // This ensures that a user can't move about freely when the size is zero.
  if (state.fb->size == 1)
    retaps(0);

  go_left();
  retaps(0);
}

static int
handle_l(void)
{ // This ensures that a user can't move about freely when the size is zero.
  if (state.fb->size == 1)
    retaps(0);

  /*
    Problem here, if Herr User moves forward before writing anything, the buffer
    isn't displayed.

    Stage 1 (Diagram Block):
    {
       P
      |0|| || || ||0| <-- These are buffer 'cells.'
    }

    *'l' is pressed, following a switch to insert then 'a'*

    Stage 2 (Diagram Block):
    {
          P
      |0||a|| || ||0|
    }

    'P' represents the position; zero is left in the buffer, causing the string
    to be null terminated prematurely.
  */

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
handle_0x08_0x7f(void)
{
  if (state.mode == 0 || state.pos == 0)
    retaps(0);

  go_left();
  for (size_t i = state.pos; i + 1 < state.fb->size; ++i)
    state.fb->buffer[i] = state.fb->buffer[i + 1];

  state.fb->buffer = realloc(state.fb->buffer, state.fb->size -= 1);
  if (state.fb->buffer == NULL)
  {
    if (state.fb->size == 0)
      retaps(0);

    retapp(1, "\x1b[H\x1b[JRealloc failed..\n", stderr);
  }

  retaps(0);
}

static int
handle_normal(unsigned int input)
{
  if (state.mode == 0)
    retaps(0);

  // The right operand is the last non-null character in the buffer.
  if (state.pos == state.fb->size - 2 || state.fb->size == 1)
  {
    state.fb->buffer = realloc(state.fb->buffer, state.fb->size += 10);
    if (state.fb->buffer == NULL)
      retapp(1, "\x1b[H\x1b[JRealloc failed..\n", stderr);

    // Memset the rest of the buffer because of crap contents.
    memset(&state.fb->buffer[state.pos], 0, state.fb->size - state.pos);

    state.fb->buffer[state.fb->size - 1] = '\0';
  }

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
  }
  return 0;
}

static int
state_initialise(char* location)
{
  state = (struct state_s){ 0, 0, NULL, NULL };
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

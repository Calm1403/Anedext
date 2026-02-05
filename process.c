
#include "process.h"
#include "file.h"
#include "input.h"
#include "mappings.h"

#include <unistd.h>

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

typedef struct state_s
{
  int mode;
  size_t pos;
  node_t* key_maps;
  file_buffer_t* fb;
} state_t;

static state_t state = { 0, 0, NULL, NULL };

static char* modes[2] = { "normal", "insert" };

static void
go_left()
{
  char cur_char = 0;
  do
  {
    if ((state.pos - 1) == -1)
      state.pos = state.fb->size - 1;
    else
      --state.pos;
    if ((cur_char = state.fb->buffer[state.pos]) == '\n')
      puts("\x1b[1E");
  } while (cur_char == '\n');
}

static void
go_right()
{
  char cur_char = 0;
  do
  {
    if ((state.pos + 1) > (state.fb->size - 1))
      state.pos = 0;
    else
      ++state.pos;
    cur_char = state.fb->buffer[state.pos];
  } while (cur_char == '\n');
}

static int
handle_0x1b(void)
{
  char seq[2] = { 0 };
  if (read(STDIN_FILENO, seq, 2) == 0)
  {
    printf("\x1b[H\x1b[J%s\n[%li | %s]\n\x1b[H",
           state.fb->buffer,
           state.pos,
           modes[state.mode]);

    if (state.mode == 1)
    {
      state.mode = 0;

      return 0;
    }
    return 1;
  }
  switch (seq[1])
  {
    case 'D':
      go_left();
      break;
    case 'C':
      go_right();
  }
  return 0;
}

static int
handle_0x13(void)
{
  state.fb->save = true;
  return 1;
}

static int
handle_0x09(void)
{
  state.mode = 1;
  return 0;
}

static int
handle_0x08_0x7f(void)
{
  go_left();
  state.fb->buffer[state.pos] = ' ';
  return 0;
}

static node_t*
register_maps(void)
{
  node_t* end = NULL;
  if (add_node(&state.key_maps, handle_0x1b, 0x1b) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x13, 0x13) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x09, 0x09) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x08_0x7f, 0x08) == NULL)
    goto free;

  if (add_node(&state.key_maps, handle_0x08_0x7f, 0x7f) == NULL)
    goto free;

  return state.key_maps; // If everything went well..

free:
  free_list(state.key_maps);
  return NULL;
}

static int
check_maps(node_t** node, unsigned int input)
{
  do
  {
    if ((*node)->key == input)
    {
      if ((*node)->mapping() == 1)
      {
        printf("\x1b[H\x1b[J%s\n[%li | %s]\n\x1b[H",
               state.fb->buffer,
               state.pos,
               modes[state.mode]);

        return 1; // Special input caused termination.
      }
      break;
    }
  } while ((*node = (*node)->next) != NULL);
  return 0;
}

static int
process_input(unsigned char input)
{
  node_t* node = state.key_maps;

  if (check_maps(&node, input) == 1)
    return 1;

  if (node == NULL)
  {
    if (state.mode == 1)
    {
    }
    else
    {
      state.fb->buffer[state.pos] = input;
      go_right();
    }
  }

  printf("\x1b[H\x1b[J%s\n[%li | %s]\n\x1b[H",
         state.fb->buffer,
         state.pos,
         modes[state.mode]);

  return 0;
}

static int
state_initialise(char* location)
{
  if ((state.fb = create_fb(location)) == NULL)
    return 1;

  printf("\x1b[H\x1b[J%s\n[%li | %s]\n\x1b[H",
         state.fb->buffer,
         state.pos,
         modes[state.mode]);

  if ((state.key_maps = register_maps()) == NULL)
    return 1;

  return 0;
}

static void
uninitialise_state(void)
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

  uninitialise_state();

  fputs("\x1b[H\x1b[JExiting.. bye bye!\n", stdout);

  return 0;
}

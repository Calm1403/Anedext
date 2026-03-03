#ifndef PRINTERS_H
#define PRINTERS_H

#include <errno.h>
#include <stdio.h>
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

// Return and print failure.
#define retapf(failure)                                                        \
  do                                                                           \
  {                                                                            \
    perror(failure);                                                           \
    return 1;                                                                  \
  } while (0)

#endif

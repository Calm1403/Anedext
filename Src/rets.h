
#ifndef RETS_H
#define RETS_H

// Return; makes the syntax consistent.
#define ret(code)                                                              \
  do                                                                           \
  {                                                                            \
    return code;                                                               \
  } while (0)

// Return and print prompt.
#define retapp(code, prompt, stream)                                           \
  do                                                                           \
  {                                                                            \
    fputs(prompt, stream);                                                     \
    ret(code);                                                                 \
  } while (0)

// Return and print error.
#define retape(code, prompt)                                                   \
  do                                                                           \
  {                                                                            \
    perror(prompt);                                                            \
    ret(code);                                                                 \
  } while (0)

#endif


#include "window.h"

int
get_size(struct winsize* ws)
{
  if ((ioctl(0, TIOCGWINSZ, ws)) == -1)
    return 1;

  return 0;
}

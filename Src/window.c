
#include "window.h"
#include "rets.h"

int
get_size(struct winsize* ws)
{
  if ((ioctl(0, TIOCGWINSZ, ws)) == -1)
    retape(1, "\x1b[H\x1b[JIoctl failed..\n\nReason");

  ret(0);
}

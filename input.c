
#include "input.h"

#include <termios.h>
#include <unistd.h>

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

int
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

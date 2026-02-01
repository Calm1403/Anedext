
#include "process.h"

int
main(int argc, char* argv[])
{
  if (argc != 2)
    return 1;

  if (begin_processing(argv[1]) == 1)
    return 1;

  return 0;
}

#include "tunistd.h"
#include "tty_public.h"
#include <stdio.h>
int main2()
{
//  printf("\n<PROC 2 is running>\n");
  write(TTY1,"bbbbbbbbbb",10);
  return 4;
}

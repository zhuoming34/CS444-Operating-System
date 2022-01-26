#include "tunistd.h"
#include "tty_public.h"
#include <stdio.h>
int main3()
{
//  printf("\n<PROC 3 is running>\n");
  write(TTY1,"cccccccccc",10);
  return 6;
}

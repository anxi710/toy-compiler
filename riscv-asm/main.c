#include <stdio.h>

extern int main0();

int
main()
{
  int ret_val = main0();
  printf("return value = %d\n", ret_val);

  return ret_val;
}

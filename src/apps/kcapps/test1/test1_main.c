
#include <nuttx/config.h>
#include <stdio.h>

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int test1_main(int argc, char *argv[])
#endif
{
  printf("test1\nThis is application test1 compiled from kcapps/test1.\n");
  return 0;
}

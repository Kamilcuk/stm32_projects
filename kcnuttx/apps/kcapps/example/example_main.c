
#include <nuttx/config.h>
#include <stdio.h>

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int example_main(int argc, char *argv[])
#endif
{
  printf("example\nThis is application example compiled from kcapps/example.\n");
  return 0;
}


#include <nuttx/config.h>
#include <stdio.h>

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int [[app_name]]_main(int argc, char *argv[])
#endif
{
  printf("[[app_name]]\n[[app_desc]]\n");
  return 0;
}

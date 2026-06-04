#include <bootprog.h>

int main(char *arg, int flags)
{
  INIT_BOOT_MODULE();

  return printf("Hello from echo2!\n");
}

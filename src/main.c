#include "arena.h"
#include "base.h"

#include <stdio.h>

int main(void) {
  mem_arena *perm_arena = arena_create(GiB(2), MiB(128));
  printf("Hola \n");
  arena_destroy(perm_arena);
  return 0;
}

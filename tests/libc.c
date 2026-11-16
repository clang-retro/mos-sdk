// The C library: formatted output, strings, sorting, the heap and setjmp.
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmp(const void *a, const void *b) {
  return *(const int *)a - *(const int *)b;
}

static jmp_buf env;

static void deep(int n) {
  if (n == 0)
    longjmp(env, 42);
  deep(n - 1);
}

int main(void) {
  int v[] = {9, -3, 27, 0, 14, -100, 5, 5};
  qsort(v, 8, sizeof v[0], cmp);
  for (int i = 0; i < 8; i++)
    printf("%d%c", v[i], i == 7 ? '\n' : ' ');

  char buf[40];
  snprintf(buf, sizeof buf, "[%5d|%-4x|%c|%s]", -42, 0xbe, 'Z', "6502");
  printf("%s %u\n", buf, (unsigned)strlen(buf));
  printf("%s\n", strstr("llvm-mos sdk", "mos"));
  printf("%ld %lu\n", strtol("-123456", NULL, 10), strtoul("ffff0000", NULL, 16));

  char *p = malloc(100), *q = malloc(200);
  if (!p || !q)
    return 1;
  memset(p, 'a', 99);
  p[99] = 0;
  memcpy(q, p, 100);
  free(p);
  q = realloc(q, 300);
  printf("heap %u %c\n", (unsigned)strlen(q), q[50]);
  free(q);

  int r = setjmp(env);
  if (r == 0)
    deep(10);
  printf("longjmp %d\n", r);
  return 0;
}

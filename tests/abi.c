// Calls the way C programs make them: structures passed and returned by
// value, variadic functions, function pointers and deep recursion.
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct small { uint8_t a; int16_t b; };
struct big { int32_t v[5]; char name[8]; };

__attribute__((noinline)) static struct small mk_small(uint8_t a, int16_t b) {
  struct small s = {a, b};
  return s;
}

__attribute__((noinline)) static struct big mk_big(int32_t base, const char *n) {
  struct big b;
  for (int i = 0; i < 5; i++)
    b.v[i] = base * (i + 1);
  strncpy(b.name, n, sizeof b.name);
  return b;
}

__attribute__((noinline)) static int32_t sum_big(struct big b, struct small s) {
  int32_t t = s.a + s.b;
  for (int i = 0; i < 5; i++)
    t += b.v[i];
  return t + (int32_t)strlen(b.name);
}

static int32_t vsum(int n, ...) {
  va_list ap;
  va_start(ap, n);
  int32_t t = 0;
  for (int i = 0; i < n; i++) {
    switch (va_arg(ap, int)) {
    case 'c': t += (signed char)va_arg(ap, int); break;
    case 'i': t += va_arg(ap, int); break;
    case 'l': t += va_arg(ap, long); break;
    case 'q': t += (int32_t)(va_arg(ap, long long) >> 8); break;
    }
  }
  va_end(ap);
  return t;
}

static int32_t fib(int n) { return n < 2 ? n : fib(n - 1) + fib(n - 2); }

static int add(int a, int b) { return a + b; }
static int sub(int a, int b) { return a - b; }
static int (*const ops[])(int, int) = {add, sub};

int main(void) {
  struct small s = mk_small(200, -1234);
  struct big b = mk_big(100000, "sixfive");
  printf("small %u %d\n", s.a, s.b);
  printf("big %ld %ld %s\n", (long)b.v[0], (long)b.v[4], b.name);
  printf("sum %ld\n", (long)sum_big(b, s));
  printf("vsum %ld\n", (long)vsum(4, 'c', -5, 'i', 30000, 'l', 1000000L, 'q', 0x123456789aLL));
  printf("fib %ld\n", (long)fib(20));
  volatile int which = 1;
  printf("ops %d %d\n", ops[0](7, 5), ops[which](7, 5));
  return 0;
}

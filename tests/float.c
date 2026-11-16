// Software floating point (float and double) through printf_flt.
#include <stdio.h>

static volatile float fx = 3.14159f;
static volatile double dx = 2.718281828459045;

int main(void) {
  float f = fx;
  double d = dx;
  for (int i = 0; i < 5; i++) {
    f = f * 1.5f - 0.25f;
    d = d / 1.75 + d * d / 100.0;
  }
  printf("%.4f %.6f\n", (double)f, d);
  printf("%d %ld\n", (int)(f * 10), (long)(d * 1e6));
  printf("%g %e\n", 1.0 / 3.0, 6502.0e10);
  return 0;
}

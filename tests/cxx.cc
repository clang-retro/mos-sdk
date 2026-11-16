// C++ without exceptions: classes, virtual calls, templates, constructors of
// globals, lambdas and operator overloading.
#include <stdint.h>
#include <stdio.h>

struct Shape {
  virtual ~Shape() = default;
  virtual int32_t area() const = 0;
  virtual const char *name() const = 0;
};

struct Rect : Shape {
  int16_t w, h;
  Rect(int16_t w, int16_t h) : w(w), h(h) {}
  int32_t area() const override { return (int32_t)w * h; }
  const char *name() const override { return "rect"; }
};

struct Square : Rect {
  explicit Square(int16_t s) : Rect(s, s) {}
  const char *name() const override { return "square"; }
};

template <typename T, int N> struct Ring {
  T items[N];
  int head = 0;
  void push(T t) { items[head++ % N] = t; }
  template <typename F> T fold(T init, F f) const {
    for (int i = 0; i < N; i++)
      init = f(init, items[i]);
    return init;
  }
};

struct Fixed {
  int16_t raw; // 8.8
  Fixed operator*(Fixed o) const { return {(int16_t)(((int32_t)raw * o.raw) >> 8)}; }
  Fixed operator+(Fixed o) const { return {(int16_t)(raw + o.raw)}; }
};

static int constructed;
struct Counter {
  Counter() { constructed++; }
};
static Counter counters[3];

int main() {
  Rect r(300, 200);
  Square s(181);
  const Shape *shapes[] = {&r, &s};
  for (const Shape *sh : shapes)
    printf("%s %ld\n", sh->name(), (long)sh->area());

  Ring<int32_t, 4> ring;
  for (int32_t i = 1; i <= 6; i++)
    ring.push(i * 1000);
  printf("ring %ld\n", (long)ring.fold(0, [](int32_t a, int32_t b) { return a + b; }));

  Fixed a{0x0180}, b{0x0240}; // 1.5, 2.25
  Fixed c = a * b + a;
  printf("fixed %04x\n", (unsigned)(uint16_t)c.raw);
  printf("globals %d\n", constructed);
  return 0;
}

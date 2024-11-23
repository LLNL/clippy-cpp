#include <iostream>

struct Foo {
  int x;
  void print() { std::cout << "non-const" << x << "\n"; }
  void print() const { std::cout << "const" << x << "\n"; }
};

int main() {
  Foo f{10};
  f.print();

  const Foo &g = f;
  g.print();
  Foo h = g;
  h.x = 20;
  g.print();
}

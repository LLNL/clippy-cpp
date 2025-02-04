#include <iostream>
#include <string>

class Foo {
 public:
  int &operator[](int k) { return k; }
  int &operator[](double k) { return int(k); }
  int &operator[](std::string k) { return -99; }
};

int main() {
  Foo f;
  std::cout << f[1] << std::endl;
  std::cout << f[1.1] << std::endl;
  std::cout << f["hello"] << std::endl;
  return 0;
}
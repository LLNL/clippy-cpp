#include <iostream>

int main(int argc, char **arv) {
  std::cout
      << R"json({"method_name":"returns_string","desc":"Tests returning a string","version":"0.1.0","returns":{"desc":"The string"}})json";
  return 0;
}

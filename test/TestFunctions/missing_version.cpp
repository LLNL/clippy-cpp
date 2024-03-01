#include <iostream>

int main(int argc, char **arv) {
  std::cout
      << R"json({"method_name":"returns_string","desc":"Tests returning a string","returns":{"desc":"The string"}})json";
  return 0;
}
#include "testdf.hpp"

#include <iostream>

int main() {
  testdf d{};
  d.add_col<bool>("boolean");
  d.add_col<int64_t>("int64");
  d.add_col<double>("double");
  std::cout << d.dtypes << std::endl;
}
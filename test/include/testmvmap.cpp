#include <iostream>
#include <string>

#include "mvmap.hpp"

int main() {
  mvmap::mvmap<std::string, std::string, double, int64_t, bool> m;

  m.add_series<std::string>("s1", "string1");
  m.add_series<double>("d1", "double1");
  m.add_series<int64_t>("i1", "int1");
  m.add_series<bool>("b1", "bool1");

  auto s1 = m.get_series<std::string>("s1");
  s1.value()["k1"] = "hello";

  s1.value().print();
  m.print();
}
#include <iostream>
#include <map>
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
  std::cout << "s1\n";
  s1.value().print();
  std::cout << "s1 done\n";
  auto s2 = m.get_series<int64_t>("i1");
  std::cout << "s2\n";
  s2.value()["k1"] = 42;
  s2.value()["k2"] = 43;
  std::cout << "s2 done\n";

  std::cout << "m\n";
  m.print();
  std::cout << "m done\n";
  using variants = std::variant<std::string, double, int64_t, bool>;
  std::map<std::string, variants> myrow{
      {"s1", "hello"}, {"d1", 3.14}, {"i1", 99}, {"b1", true}};

  m.add_row("rowkey", myrow);

  std::cout << "m again\n";
  m.print();
  std::cout << "m done\n";
};
#include "testdf.hpp"

#include <iostream>

int main() {
  testdf d{};
  d.add_col<bool>("b1");
  d.add_col<int64_t>("i1");
  d.add_col<double>("d1");
  d.add_col<std::string>("s1");
  auto dt = d.dtypes();

  std::cout << "dtypes:\n";
  for (auto [k, v] : dt) {
    std::cout << k << ": " << v << std::endl;
  }

  using variants = std::variant<std::string, double, int64_t, bool>;
  std::map<std::string, variants> row1{{"b1", true}, {"i1", 42}, {"d1", 3.14}};

  d.add_row(row1);

  std::vector<std::map<std::string, variants>> myrows = {
      {{"s1", "hello2"}, {"d1", 10.14}, {"i1", 11}, {"b1", false}},
      {{"s1", "world"}, {"d1", 11.71}, {"i1", 12}, {"b1", true}}};

  d.add_rows(myrows);

  d.print();
  std::cout << "to_csv:\n";
  d.to_csv();
}
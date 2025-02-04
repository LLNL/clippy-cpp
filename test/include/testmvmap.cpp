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

  std::cout << "adding rowkey\n";
  m.add_row("rowkey", myrow);

  std::map<std::string, variants> myrow_partial{
      {"d1", 2.22},
      {"i1", 22},
  };

  std::cout << "adding partial\n";
  m.add_row("partial", myrow_partial);
  std::cout << "m again\n";
  m.print();
  std::cout << "m done\n";

  std::cout << "m cols\n";
  std::vector<std::string> cols{"s1", "i1"};
  m.print_cols(cols);

  auto row = m.get_series_vals_at("partial", cols);
  std::cout << "row\n";
  for (auto el : row) {
    std::cout << el.first << " -> ";
    std::visit(
        [](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::string>) {
            std::cout << " (str) " << arg;
          } else if constexpr (std::is_same_v<T, double>) {
            std::cout << " (dbl) " << arg;
          } else if constexpr (std::is_same_v<T, int64_t>) {
            std::cout << " (int) " << arg;
          } else if constexpr (std::is_same_v<T, bool>) {
            std::cout << " (bool) " << arg;
          }
        },
        el.second);
    std::cout << std::endl;
  }
  auto s1pxy = m.get_series<std::string>("s1").value();
  std::cout << "row done\n";
};
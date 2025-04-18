#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <string>

static const std::string json_blob =
    R"({"rule":{"and":[{"<":[{"var":"temp"},110]},{"==":[{"var":"pie.filling"},"apple"]}]},"data":{"temp":100,"pie":{"filling":"apple"}},"expected":true})";

int main() {
  std::cout << json_blob << std::endl;
  auto data = boost::json::parse(json_blob).as_object();
  auto [_a /*unused*/, vars, _b /*unused*/] =
      jsonlogic::translateNode(data["rule"]);

  for (auto &v : vars) {
    std::cout << v << std::endl;
  }

  // grab the series proxies for the variables.
  // for all rows in the matrix, grab the values from the series proxies
  // and evaluate the expression.
  // will need to convert the values to JSON values - since we don't know
  // the type of the data per series proxy, will need to use visit. Then just
  // data[variable] = value
}

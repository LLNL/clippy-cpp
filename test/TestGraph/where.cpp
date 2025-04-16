#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "clippy/clippy-eval.hpp"
#include "clippy/selector.hpp"
#include "testgraph.hpp"

template <typename M>
auto parse_where_expression(M& mvmap_, boost::json::object& expression,
                            boost::json::object& submission_data) {
  std::cerr << "  parse_where_expression: expression: " << expression
            << std::endl;
  boost::json::object exp2(expression);

  // boost::json::object submission_data{};
  auto [_a /*unused*/, vars, _b /*unused*/] =
      jsonlogic::translateNode(exp2["rule"]);

  std::cerr << "parse_where: # of vars: " << vars.size() << std::endl;
  for (const auto& var : vars) {
    std::cerr << "  apply_jl var dump: var: " << var << std::endl;
  }

  auto apply_jl = [&expression, vars, &mvmap_,
                   &submission_data](mvmap::locator loc) {
    std::cerr << "    apply_jl:  # of vars: " << vars.size() << std::endl;
    for (const auto& var : vars) {
      std::cerr << "    apply_jl: var: " << var << std::endl;
      auto var_sel = selector(std::string(var));
      std::cerr << "  apply_jl: var_sel = " << var_sel << std::endl;
      // if (!var_sel.headeq("node")) {
      //   std::cerr << "selector is not a node selector; skipping." <<
      //   std::endl; continue;
      // }
      auto var_tail = var_sel.tail().value();
      std::string var_str = std::string(var_sel);
      std::cerr << "    apply_jl: var: " << var_sel << std::endl;
      if (mvmap_.has_series(var_tail)) {
        std::cerr << "    apply_jl: has series: " << var_sel << std::endl;
        auto val = mvmap_.get_as_variant(var_tail, loc);
        if (val.has_value()) {
          std::cerr << "    apply_jl: val has value" << std::endl;
          std::visit(
              [&submission_data, &loc, &var_str](auto&& v) {
                submission_data[var_str] = boost::json::value(v);
                std::cerr << "    apply_jl: submission_data[" << var_str
                          << "] = " << v << " at loc " << loc << "."
                          << std::endl;
              },
              *val);
        } else {
          std::cerr << "    apply_jl: no value for " << var_sel << std::endl;
          submission_data[var_str] = boost::json::value();
        }
      } else {
        std::cerr << "    apply_jl: no series for " << var_sel << std::endl;
      }
    }
    std::cerr << "    apply_jl: submission_data: " << submission_data
              << std::endl;
    jsonlogic::any_expr res =
        jsonlogic::apply(expression["rule"], submission_data);
    std::cerr << "    apply_jl: res: " << res << std::endl;
    return jsonlogic::unpack_value<bool>(res);
  };

  return apply_jl;
}

std::vector<testgraph::node_t> where_nodes(const testgraph::testgraph& g,
                                           boost::json::object& expression) {
  std::vector<testgraph::node_t> filtered_results;
  // boost::json::object exp2(expression);

  std::cerr << "  where: expression: " << expression << std::endl;
  // auto [_a /*unused*/, vars, _b /*unused*/] =
  //     jsonlogic::translateNode(exp2["rule"]);

  // auto nodemap = g.nodemap();
  // boost::json::object submission_data{};
  // auto apply_jl = [&expression, &vars, &nodemap,
  //                  &submission_data](testgraph::node_t key) {
  //   for (const auto& var : vars) {
  //     auto var_sel = selector(std::string(var));
  //     if (!var_sel.headeq("node")) {
  //       std::cerr << "selector is not a node selector; skipping." <<
  //       std::endl; continue;
  //     }
  //     auto var_tail = var_sel.tail().value();
  //     std::string var_str = std::string(var_sel);
  //     std::cerr << "    apply_jl: var: " << var_sel << std::endl;
  //     if (nodemap.has_series(var_tail)) {
  //       std::cerr << "    apply_jl: has series: " << var_sel << std::endl;
  //       auto val = nodemap.get_as_variant(var_tail, key);
  //       if (val.has_value()) {
  //         std::cerr << "    apply_jl: val has value" << std::endl;
  //         std::visit(
  //             [&submission_data, &key, &var_str](auto&& v) {
  //               submission_data[var_str] = boost::json::value(v);
  //               std::cerr << "    apply_jl: submission_data[" << var_str
  //                         << "] = " << v << " at key " << key << "."
  //                         << std::endl;
  //             },
  //             val.value());
  //       } else {
  //         std::cerr << "    apply_jl: no value for " << var_sel << std::endl;
  //         submission_data[var_str] = boost::json::value();
  //       }
  //     } else {
  //       std::cerr << "    apply_jl: no series for " << var_sel << std::endl;
  //     }
  //   }

  //   jsonlogic::any_expr res =
  //       jsonlogic::apply(expression["rule"], submission_data);
  //   std::cerr << "    apply_jl: res: " << res << std::endl;
  //   return jsonlogic::unpack_value<bool>(res);
  // };

  auto nodemap = g.nodemap();
  boost::json::object submission_data;
  auto apply_jl = parse_where_expression(nodemap, expression, submission_data);
  nodemap.for_all([&filtered_results, &apply_jl, &nodemap, &expression](
                      const auto& key, const auto& loc) {
    std::cerr << "  where for_all key: " << key << std::endl;
    if (apply_jl(loc)) {
      std::cerr << "  where: applied!" << std::endl;
      filtered_results.push_back(key);
    }
  });

  return filtered_results;
}
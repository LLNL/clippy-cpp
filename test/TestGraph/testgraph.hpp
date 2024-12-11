#pragma once
#include <cstdint>
#include <map>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include "boost/json.hpp"
#include "clippy/selector.hpp"
#include "mvmap.hpp"
#include "selector.hpp"

namespace testgraph {
// map of (src, dst) : weight
using node_t = std::string;
using edge_t = std::pair<node_t, node_t>;

template <typename T>
using sparsevec = std::map<uint64_t, T>;

// using variants = std::variant<bool, double, int64_t, std::string>;
class testgraph {
  using edge_mvmap = mvmap::mvmap<edge_t, bool, double, int64_t, std::string>;
  using node_mvmap = mvmap::mvmap<node_t, bool, double, int64_t, std::string>;
  template <typename T>
  using edge_series_proxy = edge_mvmap::series_proxy<T>;
  template <typename T>
  using node_series_proxy = node_mvmap::series_proxy<T>;
  node_mvmap node_table;
  edge_mvmap edge_table;

 public:
  static inline bool is_edge_selector(const selector &name) {
    return name.headeq("edge");
  }

  static inline bool is_node_selector(const selector &name) {
    return name.headeq("node");
  }

  static inline bool is_valid_selector(const std::string &name) {
    return is_edge_selector(name) || is_node_selector(name);
  }

  // static inline std::optional<std::string> selector_obj_to_string(
  //     boost::json::object from_selector_obj) {
  //   try {
  //     if (from_selector_obj["expression_type"].as_string() !=
  //         std::string("jsonlogic")) {
  //       std::cerr << " NOT A THINGY " << std::endl;
  //       return std::nullopt;
  //     }
  //     std::string from_selector =
  //         from_selector_obj["rule"].as_object()["var"].as_string().c_str();
  //     return from_selector;
  //   } catch (...) {
  //     std::cerr << "!! ERROR !!" << std::endl;
  //     return std::nullopt;
  //   }
  // }

  friend void tag_invoke(boost::json::value_from_tag /*unused*/,
                         boost::json::value &v, testgraph const &g) {
    v = {{"node_table", boost::json::value_from(g.node_table)},
         {"edge_table", boost::json::value_from(g.edge_table)}};
  }

  friend testgraph tag_invoke(boost::json::value_to_tag<testgraph> /*unused*/,
                              const boost::json::value &v) {
    const auto &obj = v.as_object();
    auto nt = boost::json::value_to<node_mvmap>(obj.at("node_table"));
    auto et = boost::json::value_to<edge_mvmap>(obj.at("edge_table"));
    return {nt, et};
  }
  testgraph() = default;
  testgraph(node_mvmap nt, edge_mvmap et)
      : node_table(std::move(nt)), edge_table(std::move(et)) {};

  // template <typename T>
  // std::optional<edge_series_proxy<T>> get_edge_series(const std::string
  // &name) {
  //   return edge_table.get_series<T>(name);
  // }
  // edge_series_proxy<T> get_or_create_edge_col(const std::string &name,
  //                                             const std::string &desc = "") {
  //   return edge_table.get_or_create_series<T>(name, desc);
  // }

  // this function requires that the "edge." prefix be removed from the name.
  template <typename T>
  std::optional<edge_series_proxy<T>> add_edge_series(
      const std::string &name, const std::string &desc = "") {
    return edge_table.add_series<T>(name, desc);
  }

  template <typename T>
  std::optional<edge_series_proxy<T>> add_edge_series(
      const std::string &name, const edge_series_proxy<T> &from,
      const std::string &desc = "") {
    return edge_table.add_series<T>(name, from, desc);
  }

  void drop_edge_series(const std::string &name) {
    edge_table.drop_series(name);
  }

  void drop_node_series(const std::string &name) {
    node_table.drop_series(name);
  }
  // this function requires that the "node." prefix be removed from the name.
  template <typename T>
  std::optional<node_series_proxy<T>> add_node_series(
      const std::string &name, const std::string &desc = "") {
    return node_table.add_series<T>(name, desc);
  }

  template <typename T>
  std::optional<edge_series_proxy<T>> add_node_series(
      const std::string &name, const node_series_proxy<T> &from,
      const std::string &desc = "") {
    return node_table.add_series<T>(name, from, desc);
  }
  template <typename T>
  std::optional<edge_series_proxy<T>> get_edge_series(const std::string &name) {
    return edge_table.get_series<T>(name);
  }

  bool copy_edge_series(const std::string &from, const std::string &to) {
    return edge_table.copy_series(from, to);
  }

  bool copy_node_series(const std::string &from, const std::string &to) {
    std::cerr << "copy_node_series: from = " << from << ", to = " << to
              << std::endl;

    return node_table.copy_series(from, to);
  }

  template <typename T>
  std::optional<node_series_proxy<T>> get_node_series(const std::string &name) {
    return node_table.get_series<T>(name);
  }

  // template <typename T>
  // void get_or_create_edge_col(const std::string &name,
  //                             std::map<edge_t, T> data) {
  //   auto proxy = get_or_create_edge_col<T>(name);
  //   for (const auto &[k, v] : data) {
  //     proxy[k] = v;
  //   }
  // }
  // template <typename T>
  // node_series_proxy<T> get_or_create_node_col(const std::string &name,
  //                                             const std::string &desc = "") {
  //   return node_table.get_or_create_series<T>(name, desc);
  // }

  // template <typename T>
  // void get_or_create_node_col(const std::string &name,
  //                             std::map<node_t, T> data) {
  //   auto proxy = get_or_create_node_col<T>(name);
  //   for (const auto &[k, v] : data) {
  //     proxy[k] = v;
  //   }
  // }

  [[nodiscard]] size_t nv() const { return node_table.size(); }
  [[nodiscard]] size_t ne() const { return edge_table.size(); }

  template <typename F>
  void for_all_edges(F f) {
    edge_table.for_all(f);
  }
  template <typename F>
  void for_all_nodes(F f) {
    node_table.for_all(f);
  }

  [[nodiscard]] std::vector<edge_t> edges() const {
    auto kv = edge_table.keys();
    return {kv.begin(), kv.end()};
  }
  [[nodiscard]] std::vector<node_t> nodes() const {
    auto kv = node_table.keys();
    return {kv.begin(), kv.end()};
  }

  bool add_node(const node_t &node) { return node_table.add_key(node); };
  bool add_edge(const node_t &src, const node_t &dst) {
    node_table.add_key(src);
    node_table.add_key(dst);
    return edge_table.add_key({src, dst});
  }

  bool has_node(const node_t &node) { return node_table.contains(node); };
  bool has_edge(const edge_t &edge) { return edge_table.contains(edge); };
  bool has_edge(const node_t &src, const node_t &dst) {
    return edge_table.contains({src, dst});
  };

  [[nodiscard]] bool has_series(const std::string &&name) const {
    if (is_node_selector(name)) {
      return has_node_series(name.substr(5));
    }
    if (is_edge_selector(name)) {
      return has_edge_series(name.substr(5));
    }
    return false;
  }

  [[nodiscard]] bool has_node_series(const std::string &name) const {
    return node_table.has_series(name);
  }

  [[nodiscard]] bool has_edge_series(const std::string &name) const {
    return edge_table.has_series(name);
  }

  [[nodiscard]] std::vector<node_t> out_neighbors(const node_t &node) const {
    std::vector<node_t> neighbors;
    for (const auto &[src, dst] : edge_table.keys()) {
      if (src == node) {
        neighbors.emplace_back(dst);
      }
    }
    return neighbors;
  }

  [[nodiscard]] std::vector<node_t> in_neighbors(const node_t &node) const {
    std::vector<node_t> neighbors;
    for (const auto &[src, dst] : edge_table.keys()) {
      if (dst == node) {
        neighbors.emplace_back(src);
      }
    }
    return neighbors;
  }

  [[nodiscard]] size_t in_degree(const node_t &node) const {
    return in_neighbors(node).size();
  }
  [[nodiscard]] size_t out_degree(const node_t &node) const {
    return out_neighbors(node).size();
  }

};  // class testgraph

}  // namespace testgraph

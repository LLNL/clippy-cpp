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
  static inline bool is_edge_selector(const selector &sel) {
    return sel.headeq("edge");
  }

  static inline bool is_node_selector(const selector &sel) {
    return sel.headeq("node");
  }

  static inline bool is_valid_selector(const selector &sel) {
    return is_edge_selector(sel) || is_node_selector(sel);
  }

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

  // this function requires that the "edge." prefix be removed from the name.
  template <typename T>
  std::optional<edge_series_proxy<T>> add_edge_series(
      const selector &sel, const std::string &desc = "") {
    return edge_table.add_series<T>(sel, desc);
  }

  template <typename T>
  std::optional<edge_series_proxy<T>> add_edge_series(
      const selector &sel, const edge_series_proxy<T> &from,
      const std::string &desc = "") {
    return edge_table.add_series<T>(sel, from, desc);
  }

  void drop_edge_series(const selector &sel) { edge_table.drop_series(sel); }

  // this function requires that the "node." prefix be removed from the name.
  void drop_node_series(const selector &sel) { node_table.drop_series(sel); }

  // this function requires that the "node." prefix be removed from the name.
  template <typename T>
  std::optional<node_series_proxy<T>> add_node_series(
      const selector &sel, const std::string &desc = "") {
    return node_table.add_series<T>(sel, desc);
  }

  template <typename T>
  std::optional<edge_series_proxy<T>> add_node_series(
      const selector &sel, const node_series_proxy<T> &from,
      const std::string &desc = "") {
    return node_table.add_series<T>(sel, from, desc);
  }
  template <typename T>
  std::optional<edge_series_proxy<T>> get_edge_series(const selector &sel) {
    return edge_table.get_series<T>(sel);
  }

  bool copy_edge_series(const selector &from, const selector &to) {
    return edge_table.copy_series(from, to);
  }

  bool copy_node_series(const selector &from, const selector &to) {
    std::cerr << "copy_node_series: from = " << from << ", to = " << to
              << std::endl;

    return node_table.copy_series(from, to);
  }

  template <typename T>
  std::optional<node_series_proxy<T>> get_node_series(const selector &sel) {
    return node_table.get_series<T>(sel);
  }

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

  // strips the head off the selector and passes the tail to the appropriate
  // method.
  [[nodiscard]] bool has_series(const selector &sel) const {
    auto tail = sel.tail();

    if (is_node_selector(sel) && tail.has_value()) {
      return has_node_series(tail.value());
    }
    if (is_edge_selector(sel) && tail.has_value()) {
      return has_edge_series(tail.value());
    }
    return false;
  }

  // assumes sel has already been tail'ed.
  [[nodiscard]] bool has_node_series(const selector &sel) const {
    return node_table.has_series(sel);
  }

  // assumes sel has already been tail'ed.
  [[nodiscard]] bool has_edge_series(const selector &sel) const {
    return edge_table.has_series(sel);
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

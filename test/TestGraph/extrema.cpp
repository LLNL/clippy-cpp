
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>

#include "clippy/clippy-eval.hpp"
#include "clippy/selector.hpp"
#include "testgraph.hpp"

static const std::string method_name = "extrema";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name,
                      "returns the extrema of a series based on selector"};
  clip.add_required<selector>("selector",
                              "Existing selector name to calculate extrema");
  clip.add_required_state<testgraph::testgraph>(state_name,
                                                "Internal container");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }
  auto sel_str = clip.get<selector>("selector");
  selector sel{sel_str};

  bool is_edge_sel = testgraph::testgraph::is_edge_selector(sel);
  bool is_node_sel = testgraph::testgraph::is_node_selector(sel);

  if (!is_edge_sel && !is_node_sel) {
    std::cerr << "Selector must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  auto tail_opt = sel.tail();
  if (!tail_opt) {
    std::cerr << "Selector must have a tail" << std::endl;
    return 1;
  }
  auto tail_sel = tail_opt.value();

  auto the_graph = clip.get_state<testgraph::testgraph>(state_name);

  if (is_edge_sel) {
    clip.returns<std::pair<std::pair<testgraph::edge_t, double>,
                           std::pair<testgraph::edge_t, double>>>(
        "min and max keys and values of the series");
    auto series = the_graph.get_edge_series<double>(tail_sel);
    if (!series) {
      std::cerr << "Edge series not found" << std::endl;
      return 1;
    }
    auto series_val = series.value();
    auto [min_tup, max_tup] = series_val.extrema();

    testgraph::edge_t min_key =
        min_tup ? std::get<1>(min_tup.value()) : std::make_pair("", "");
    testgraph::edge_t max_key =
        max_tup ? std::get<1>(max_tup.value()) : std::make_pair("", "");

    double min_val = min_tup ? std::get<0>(min_tup.value()) : 0.0;
    double max_val = max_tup ? std::get<0>(max_tup.value()) : 0.0;
    auto extrema = std::make_pair(std::make_pair(min_key, min_val),
                                  std::make_pair(max_key, max_val));
    clip.to_return(extrema);
  }

  if (is_node_sel) {
    // clip.returns<std::pair<std::pair<testgraph::node_t, double>,
    //                        std::pair<testgraph::node_t, double>>>(
    //     "min and max keys and values of the series");

    // clip.returns<std::pair<std::string, std::string>>(
    //     "min and max keys of the series");

    clip.returns<std::pair<std::pair<testgraph::node_t, double>,
                           std::pair<testgraph::node_t, double>>>(
        "min of the series");

    auto series = the_graph.get_node_series<double>(tail_sel);
    if (!series) {
      std::cerr << "Node series not found" << std::endl;
      return 1;
    }
    auto series_val = series.value();
    auto [min_tup, max_tup] = series_val.extrema();

    testgraph::node_t min_key = min_tup ? std::get<1>(min_tup.value()) : "";
    testgraph::node_t max_key = max_tup ? std::get<1>(max_tup.value()) : "";

    double min_val = min_tup ? std::get<0>(min_tup.value()) : 0.0;
    double max_val = max_tup ? std::get<0>(max_tup.value()) : 0.0;

    auto extrema = std::make_pair(std::make_pair(min_key, min_val),
                                  std::make_pair(max_key, max_val));
    clip.to_return<std::pair<std::pair<std::string, double>,
                             std::pair<std::string, double>>>(extrema);
  }

  clip.set_state(state_name, the_graph);
  return 0;
}

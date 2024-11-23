
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include "testgraph.hpp"
#include <boost/json.hpp>
#include <cassert>
#include <iostream>

namespace boostjsn = boost::json;

static const std::string method_name = "drop_series";
static const std::string graph_state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";
static const std::string sel_name = "selector";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Drops a subselector"};
  clip.add_required<boostjsn::object>(sel_name, "Selector to drop");

  clip.add_required_state<std::map<std::string, std::string>>(
      sel_state_name, "Internal container for pending selectors");
  clip.add_required_state<testgraph::testgraph>(graph_state_name,
                                                "Internal state for the graph");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto jo = clip.get<boostjsn::object>(sel_name);

  std::string sel;
  try {
    if (jo["expression_type"].as_string() != std::string("jsonlogic")) {
      std::cerr << " NOT A THINGY " << std::endl;
      exit(-1);
    }
    sel = jo["rule"].as_object()["var"].as_string().c_str();
  } catch (...) {
    std::cerr << "!! ERROR !!" << std::endl;
    exit(-1);
  }

  auto sel_state =
      clip.get_state<std::map<std::string, std::string>>(sel_state_name);
  if (!sel_state.contains(sel)) {
    std::cerr << "Selector name not found!" << std::endl;
    exit(-1);
  }
  auto the_graph = clip.get_state<testgraph::testgraph>(graph_state_name);
  auto subsel = sel.substr(5);
  if (sel.starts_with("edge.")) {
    if (the_graph.has_edge_series(subsel)) {
      the_graph.drop_edge_series(sel);
    }
  } else if (sel.starts_with("node.")) {
    if (the_graph.has_node_series(subsel)) {
      the_graph.drop_node_series(subsel);
    }
  } else {
    std::cerr << "Selector name must start with either \"edge.\" or \"node.\""
              << std::endl;
    exit(-1);
  }
  sel_state.erase(sel);
  clip.set_state(graph_state_name, the_graph);
  clip.set_state(sel_state_name, sel_state);
  clip.update_selectors(sel_state);

  return 0;
}

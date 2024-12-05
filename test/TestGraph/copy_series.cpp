// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <cassert>
#include <iostream>
#include <variant>

#include "clippy/clippy.hpp"
#include "testgraph.hpp"

namespace boostjsn = boost::json;

static const std::string method_name = "copy_series";
static const std::string graph_state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Copies a subselector to a new subselector"};
  clip.add_required<boostjsn::object>("from_sel", "Source Selector");
  clip.add_required<boostjsn::object>("to_sel", "Target Selector");

  clip.add_required_state<std::map<std::string, std::string>>(
      sel_state_name, "Internal container for pending selectors");
  clip.add_required_state<testgraph::testgraph>(graph_state_name,
                                                "Internal state for the graph");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto from_selector_obj = clip.get<boostjsn::object>("from_sel");
  auto to_selector_obj = clip.get<boostjsn::object>("to_sel");

  auto from_selector_opt =
      testgraph::testgraph::selector_obj_to_string(from_selector_obj);

  if (!from_selector_opt.has_value()) {
    std::cerr << "!! ERROR: invalid from_selector !!" << std::endl;
    exit(-1);
  }
  auto to_selector_opt =
      testgraph::testgraph::selector_obj_to_string(to_selector_obj);

  if (!to_selector_opt.has_value()) {
    std::cerr << "!! ERROR: invalid to_selector !!" << std::endl;
    exit(-1);
  }

  auto from_selector = from_selector_opt.value();
  auto to_selector = to_selector_opt.value();
  // std::map<std::string, std::string> selectors;
  auto the_graph = clip.get_state<testgraph::testgraph>(graph_state_name);

  bool edge_sel = false;
  if (testgraph::testgraph::is_edge_selector(from_selector)) {
    edge_sel = true;
  } else if (!testgraph::testgraph::is_node_selector(from_selector)) {
    std::cerr
        << "!! ERROR: Parent must be either \"edge\" or \"node\" (received "
        << from_selector << ") !!";
    exit(-1);
  }

  bool to_sel_has_prefix =
      testgraph::testgraph::is_edge_selector(to_selector) ||
      testgraph::testgraph::is_node_selector(to_selector);
  if (to_sel_has_prefix) {
    std::cerr << "Warning: stripping prefix from to_selector" << std::endl;
    to_selector = to_selector.substr(5);
  }
  if (edge_sel && the_graph.has_edge_series(to_selector)) {
    std::cerr << "!! ERROR: Selector name " << to_selector
              << " already exists in edge table !!" << std::endl;
    exit(-1);
  }
  if (!edge_sel && the_graph.has_node_series(to_selector)) {
    std::cerr << "!! ERROR: Selector name " << to_selector
              << " already exists in node table !!" << std::endl;
    exit(-1);
  }

  if (clip.has_state(sel_state_name)) {
    auto selectors =
        clip.get_state<std::map<std::string, std::string>>(sel_state_name);
    if (selectors.contains(to_selector)) {
      std::cerr << "Warning: Using unmanifested selector." << std::endl;
      selectors.erase(to_selector);
    }
    from_selector = from_selector.substr(5);
    if (edge_sel) {
      if (!the_graph.copy_edge_series(from_selector, to_selector)) {
        std::cerr << "!! ERROR: copy failed from " << from_selector << " to "
                  << to_selector << "!!" << std::endl;
        exit(1);
      };
    } else {
      if (!the_graph.copy_node_series(from_selector, to_selector)) {
        std::cerr << "!! ERROR: copy failed from " << from_selector << " to "
                  << to_selector << "!!" << std::endl;
        exit(1);
      };
    }
  }

  clip.set_state(graph_state_name, the_graph);
  clip.return_self();

  return 0;
}

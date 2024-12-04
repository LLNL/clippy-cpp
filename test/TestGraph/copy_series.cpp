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
  clip.add_required<std::string>("to_sel", "Name of new selector");
  clip.add_optional<std::string>("desc", "Description of new selector", "");

  clip.add_required_state<std::map<std::string, std::string>>(
      sel_state_name, "Internal container for pending selectors");
  clip.add_required_state<testgraph::testgraph>(graph_state_name,
                                                "Internal state for the graph");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto from_selector_obj = clip.get<boostjsn::object>("from_sel");
  auto to_selector = clip.get<std::string>("to_sel");
  auto desc = clip.get<std::string>("desc");

  std::string from_selector;
  try {
    if (from_selector_obj["expression_type"].as_string() !=
        std::string("jsonlogic")) {
      std::cerr << " NOT A THINGY " << std::endl;
      exit(-1);
    }
    from_selector =
        from_selector_obj["rule"].as_object()["var"].as_string().c_str();
  } catch (...) {
    std::cerr << "!! ERROR !!" << std::endl;
    exit(-1);
  }

  // std::map<std::string, std::string> selectors;
  auto the_graph = clip.get_state<testgraph::testgraph>(graph_state_name);
  std::string_view fs_view = from_selector;
  std::string_view top_level = fs_view.substr(5);

  bool edge_sel = false;
  if (testgraph::testgraph::is_edge_selector(top_level)) {
    edge_sel = true;
  } else if (!testgraph::testgraph::is_node_selector(top_level)) {
    std::cerr
        << "!! ERROR: Parent must be either \"edge\" or \"node\" (received "
        << top_level << ") !!";
    exit(-1);
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
    if (edge_sel) {
      if (!the_graph.copy_edge_series(from_selector, to_selector, desc)) {
        std::cerr << "!! ERROR: copy failed from " << from_selector << " to "
                  << to_selector << "!!" << std::endl;
        exit(1);
      };
    } else {
      if (!the_graph.copy_node_series(from_selector, to_selector, desc)) {
        std::cerr << "!! ERROR: copy failed from " << from_selector << " to "
                  << to_selector << "!!" << std::endl;
        exit(1);
      };
    }
  }

  return 0;
}

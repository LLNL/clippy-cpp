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

static const std::string method_name = "add_series";
static const std::string graph_state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Adds a subselector"};
  clip.add_required<boostjsn::object>("parent_sel", "Parent Selector");
  clip.add_required<std::string>("sub_sel", "Name of new selector");
  clip.add_optional<std::string>("desc", "Description of new selector", "");

  clip.add_required_state<std::map<std::string, std::string>>(
      sel_state_name, "Internal container for pending selectors");
  clip.add_required_state<testgraph::testgraph>(graph_state_name,
                                                "Internal state for the graph");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto jo = clip.get<boostjsn::object>("parent_sel");
  auto subsel = clip.get<std::string>("sub_sel");
  auto desc = clip.get<std::string>("desc");

  std::string parentname;
  try {
    if (jo["expression_type"].as_string() != std::string("jsonlogic")) {
      std::cerr << " NOT A THINGY " << std::endl;
      exit(-1);
    }
    parentname = jo["rule"].as_object()["var"].as_string().c_str();
  } catch (...) {
    std::cerr << "!! ERROR !!" << std::endl;
    exit(-1);
  }

  // std::map<std::string, std::string> selectors;
  auto the_graph = clip.get_state<testgraph::testgraph>(graph_state_name);
  auto fullname = parentname + "." + subsel;
  if (parentname == "edge") {
    if (the_graph.has_edge_series(subsel)) {
      std::cerr << "!! ERROR: Selector name already exists in edge table !!"
                << std::endl;
      exit(-1);
    }
  } else if (parentname == "node") {
    if (the_graph.has_node_series(subsel)) {
      std::cerr << "!! ERROR: Selector name already exists in node table !!"
                << std::endl;
      exit(-1);
    }
  } else {
    std::cerr
        << "((!! ERROR: Parent must be either \"edge\" or \"node\" (received "
        << parentname << ") !!)";
    exit(-1);
  }

  if (clip.has_state(sel_state_name)) {
    auto selectors =
        clip.get_state<std::map<std::string, std::string>>(sel_state_name);
    if (selectors.contains(fullname)) {
      std::cerr << "Warning: Selector name already exists; ignoring"
                << std::endl;
    } else {
      selectors[fullname] = desc;
      clip.set_state(sel_state_name, selectors);
      clip.update_selectors(selectors);
      clip.return_self();
    }
  }

  return 0;
}

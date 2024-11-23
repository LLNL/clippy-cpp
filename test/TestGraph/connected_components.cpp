

// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy-eval.hpp"
#include "testgraph.hpp"
#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <queue>

static const std::string method_name = "connected_components";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{
      method_name,
      "Populates a column containing the component id of each node in a graph"};
  clip.add_required<boost::json::object>(
      "selector",
      "Existing selector name into which the component id will be written");
  clip.add_required_state<testgraph::testgraph>(state_name,
                                                "Internal container");
  clip.add_required_state<std::map<std::string, std::string>>(
      sel_state_name, "Internal container for selectors");
  clip.returns_self();
  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto sel_json = clip.get<boost::json::object>("selector");

  std::string sel;
  try {
    if (sel_json["expression_type"].as_string() != std::string("jsonlogic")) {
      std::cerr << " NOT A THINGY " << std::endl;
      exit(-1);
    }
    sel = sel_json["rule"].as_object()["var"].as_string().c_str();
  } catch (...) {
    std::cerr << "!! ERROR !!" << std::endl;
    exit(-1);
  }

  if (!sel.starts_with("node.")) {
    std::cerr << "Selector must be a node subselector" << std::endl;
    return 1;
  }
  auto the_graph = clip.get_state<testgraph::testgraph>(state_name);

  auto selectors =
      clip.get_state<std::map<std::string, std::string>>(sel_state_name);
  if (!selectors.contains(sel)) {
    std::cerr << "Selector not found" << std::endl;
    return 1;
  }
  auto subsel = sel.substr(5);
  if (the_graph.has_node_series(subsel)) {
    std::cerr << "Selector already populated" << std::endl;
    return 1;
  }

  auto cc_o = the_graph.add_node_series<long int>(subsel, selectors.at(sel));
  if (!cc_o) {
    std::cerr << "Unable to manifest node series" << std::endl;
    return 1;
  }

  auto cc = cc_o.value();
  std::map<testgraph::node_t, long int> ccmap;

  long int i = 0;
  for (auto &node : the_graph.nodes()) {
    ccmap[node] = i++;
  }
  std::vector<std::vector<long int>> adj(the_graph.nv());
  the_graph.for_all_edges([&adj, &ccmap](auto edge, mvmap::locator /*unused*/) {
    long i = ccmap[edge.first];
    long j = ccmap[edge.second];
    adj[i].push_back(j);
    adj[j].push_back(i);
  });

  std::vector<bool> visited(the_graph.nv(), false);
  std::vector<long int> components(the_graph.nv());
  std::iota(components.begin(), components.end(), 0);

  for (long int i = 0; i < the_graph.nv(); ++i) {
    if (!visited[i]) {
      std::queue<long int> q;
      q.push(i);
      while (!q.empty()) {
        long int v = q.front();
        q.pop();
        visited[v] = true;
        for (long int u : adj[v]) {
          if (!visited[u]) {
            q.push(u);
            components[u] = components[i];
          }
        }
      }
    }
  }

  the_graph.for_all_nodes(
      [&components, &ccmap, &cc](auto node, mvmap::locator /*unused*/) {
        long int i = ccmap[node];
        cc[node] = components[i];
      });

  clip.set_state(state_name, the_graph);
  // clip.set_state(sel_state_name, selectors);
  // clip.update_selectors(selectors);

  clip.return_self();
  return 0;
}

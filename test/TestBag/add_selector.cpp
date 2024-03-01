// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include <boost/json.hpp>
#include <cassert>
#include <iostream>

namespace boostjsn = boost::json;

static const std::string method_name = "add_selector";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Returns the size of the bag"};
  clip.add_required<boostjsn::object>("selector", "Base selector");
    // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto jo = clip.get<boostjsn::object>("selector");

  std::string name;
  try {
    if(jo["expression_type"].as_string() != std::string("jsonlogic")) {
      std::cerr << " NOT A THINGY " << std::endl;
      exit(-1);
    }
    name = jo["rule"].as_object()["var"].as_string().c_str();
  } catch (...) {
    std::cerr << "!! ERROR !!" << std::endl;
    exit(-1);
  }

  std::cerr << name << std::endl;

  auto the_bag = clip.get_state<std::vector<std::string>>(state_name);

  return 0;
}

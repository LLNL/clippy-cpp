// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>

#include "clippy/clippy.hpp"
#include "testdf.hpp"

namespace boostjsn = boost::json;

static const std::string method_name = "__init__";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Initializes a TestDF"};

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  // testgraph needs to be convertible to json
  testdf the_df{};
  clip.set_state(state_name, the_df);
  std::map<std::string, std::string> selectors;
  clip.set_state(sel_state_name, selectors);

  return 0;
}

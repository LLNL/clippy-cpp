// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include <boost/json.hpp>
#include <iostream>

namespace boostjsn = boost::json;

static const std::string class_name = "ClippyBag";
static const std::string method_name = "insert";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Inserts a string into a ClippyBag"};

  clip.member_of("ClippyBag", "Example bag container");
  clip.add_required<std::string>("item", "Item to insert");
  clip.add_required_state<std::vector<std::string>>(state_name,
                                                    "Internal container");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto item = clip.get<std::string>("item");
  auto the_bag = clip.get_state<std::vector<std::string>>(state_name);
  the_bag.push_back(item);
  clip.set_state(state_name, the_bag);

  return 0;
}

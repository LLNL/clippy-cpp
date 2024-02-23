// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include <boost/json.hpp>
#include <cassert>
#include <iostream>

namespace boostjsn = boost::json;

static const std::string method_name = "size";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Returns the size of the bag"};

  // clip.add_required<boost::json::object>("bag", "Bag");
  clip.returns<size_t>("Size of bag.");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  // auto jo = clip.get<boost::json::object>("bag");
  // auto class_name = boost::json::value_to<std::string>(
  //     jo["__clippy_type__"].as_object()["__class__"]);
  // assert(class_name == "ClippyBag");

  auto the_bag = clip.get_state<std::vector<std::string>>(state_name);
  // auto vec = boost::json::value_to<std::vector<std::string>>(
  //     jo["__clippy_type__"].as_object()["state"].as_object()["INTERNAL"]);

  clip.to_return<size_t>(the_bag.size());
  return 0;
}

// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <list>
// #include <logic.hpp>

namespace boostjsn = boost::json;

static const std::string class_name = "ClippyBag";
static const std::string method_name = "remove_if";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Removes a string from a ClippyBag"};
  clip.add_required<boostjsn::object>("expression", "Remove If Expression");
  clip.add_required_state<std::list<int>>(state_name, "Internal container");
  clip.returns_self();
  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto expression = clip.get<boostjsn::object>("expression");
  auto the_bag = clip.get_state<std::list<int>>(state_name);

  //
  // Expression here
  jsonlogic::logic_rule jlrule = jsonlogic::create_logic(expression["rule"]);

  auto apply_jl = [&jlrule](int value) {
    boostjsn::object data;
    data["value"] = value;
    return truthy(jlrule.apply(jsonlogic::json_accessor(std::move(data))));
  };

  the_bag.remove_if(apply_jl);

  clip.set_state(state_name, the_bag);
  clip.return_self();
  return 0;
}

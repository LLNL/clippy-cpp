// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <set>

namespace boostjsn = boost::json;

static const std::string method_name = "remove_if";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Removes a string from a TestSet"};
  clip.add_required<boostjsn::object>("expression", "Remove If Expression");
  clip.add_required_state<std::set<int>>(state_name, "Internal container");
  clip.returns_self();
  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto expression = clip.get<boostjsn::object>("expression");
  auto the_set = clip.get_state<std::set<int>>(state_name);

  //
  // Expression here
  auto apply_jl = [&expression](int value) {
    boostjsn::object data;
    data["value"] = value;
    jsonlogic::any_expr res = jsonlogic::apply(expression["rule"], data);
    return jsonlogic::unpack_value<bool>(res);
  };

  for (auto first = the_set.begin(), last = the_set.end(); first != last;) {
    if (apply_jl(*first))
      first = the_set.erase(first);
    else
      ++first;
  }

  clip.set_state(state_name, the_set);
  clip.return_self();
  return 0;
}

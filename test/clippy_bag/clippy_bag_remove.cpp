// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <boost/json.hpp>
#include <list>
#include <algorithm>
#include "clippy/clippy.hpp"


namespace boostjsn = boost::json;

static const std::string class_name = "ClippyBag";
static const std::string method_name = "remove";
static const std::string state_name = "INTERNAL";


int main(int argc, char** argv)
{
  clippy::clippy clip{method_name, "Initializes a ClippyBag of strings"};

  clip.member_of("ClippyBag", "Example bag container");
  clip.add_required<std::string>("item", "Item to remove");
  clip.add_optional<bool>("all", "Remove all?", false);
  clip.add_required_state<std::list<std::string>>(state_name, "Internal container");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) { return 0; }
  
  auto item = clip.get<std::string>("item");
  bool all = clip.get<bool>("all");
  auto the_bag = clip.get_state<std::list<std::string>>(state_name);
  if(all) {
    the_bag.remove(item);
  } else {
    auto itr = std::find(the_bag.begin(), the_bag.end(), item);
    if(itr != the_bag.end()) {
      the_bag.erase(itr);
    }
  }
  clip.set_state(state_name, the_bag);
  return 0;
}



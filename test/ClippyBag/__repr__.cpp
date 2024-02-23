// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <boost/json.hpp>
#include "clippy/clippy.hpp"


namespace boostjsn = boost::json;

static const std::string class_name = "ClippyBag";
static const std::string method_name = "__repr__";
static const std::string state_name = "INTERNAL";


int main(int argc, char** argv)
{
  clippy::clippy clip{method_name, "Initializes a ClippyBag of strings"};

  clip.member_of("ClippyBag", "Example bag container");
  clip.add_required_state<std::vector<std::string>>(state_name, "Internal container");

  clip.returns<std::string>("String of data.");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) { return 0; }
  
  auto the_bag = clip.get_state<std::vector<std::string>>(state_name);
  clip.set_state(state_name, the_bag);
  
  std::stringstream sstr;
  for(auto item : the_bag) {
    sstr << item << " ";
  }
  clip.to_return(sstr.str());

  return 0;
}



// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <boost/json.hpp>
#include "clippy/clippy.hpp"


namespace boostjsn = boost::json;

static const std::string class_name = "ClippyBag";
static const std::string method_name = "__init__";
static const std::string state_name = "INTERNAL";


int main(int argc, char** argv)
{
  clippy::clippy clip{method_name, "Initializes a ClippyBag of strings"};

  clip.member_of("ClippyBag", "Example bag container");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) { return 0; }
  
  std::vector<std::string> the_bag;
  clip.set_state(state_name, the_bag);
  
  return 0;
}



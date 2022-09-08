// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <boost/json.hpp>
#include "clippy/clippy.hpp"

namespace boostjsn = boost::json;

static const std::string methodName = "setGreeting";
static const std::string stGreeting = "greeting";
static const std::string stGreeted  = "greeted";

int main(int argc, char** argv)
{
  clippy::clippy clip{methodName, "Initializes a Greeter object"};

  clip.member_of("Greeter", "Customizable Greeting Generator");
  clip.add_required<std::string>(stGreeting, "Formal greeting");

  // object-state requirements
  clip.add_required_state<std::string>(stGreeted,  "Name to greet");
  clip.add_required_state<std::string>(stGreeting, "Formal greeting");

  if (clip.parse(argc, argv)) { return 0; }

  // the real thing
  {
    clip.set_state(stGreeting, clip.get<std::string>(stGreeting));
  }

  return 0;
}

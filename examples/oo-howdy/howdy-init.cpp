// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <boost/json.hpp>
#include "clippy/clippy.hpp"


namespace boostjsn = boost::json;

static const std::string methodName = "__init__";
static const std::string stGreeting = "greeting";
static const std::string stGreeted  = "greeted";

int main(int argc, char** argv)
{
  clippy::clippy clip{methodName, "Initializes a Greeter object"};

  clip.member_of("Greeter", "Customizable Greeting Generator");

  clip.add_optional<std::string>(stGreeting, "Formal greeting", "Howdy");
  clip.add_optional<std::string>(stGreeted,  "Name to greet",   "Texas");

  if (clip.parse(argc, argv)) { return 0; }

  // the real thing
  {
    boostjsn::object state;

    state[stGreeting] = boostjsn::value_from(clip.get<std::string>(stGreeting));
    state[stGreeted]  = boostjsn::value_from(clip.get<std::string>(stGreeted));

    clip.return_state(std::move(state));
  }
  return 0;
}



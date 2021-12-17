// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <boost/json.hpp>
#include "clippy/clippy.hpp"

namespace boostjsn = boost::json;

static const std::string methodName = "greet";
static const std::string stGreeting = "greeting";
static const std::string stGreeted  = "greeted";

int main(int argc, char** argv)
{
  clippy::clippy clip{methodName, "Initializes a Greeter object"};

  clip.member_of("Greeter", "Customizable Greeting Generator");

  // object-state requirements
  clip.add_required_state<std::string>(stGreeted, "Name to greet");
  clip.add_required_state<std::string>(stGreeting, "Formal greeting");

  // define return
  clip.returns<std::string>("The greeting text");

  if (clip.parse(argc, argv)) { return 0; }

  // the real thing
  {
    std::string text{clip.get_state<std::string>(stGreeting)};

    text += ' ';
    text += clip.get_state<std::string>(stGreeted);

    clip.to_return(std::move(text));
  }
  return 0;
}


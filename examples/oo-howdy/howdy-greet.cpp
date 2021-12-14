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
  clippy::clippy clip{methodName, "Composes the greeting"};

  clip.member_of("Greeter", "Customizable Greeting Generator");
  clip.returns<std::string>("The greeting text");

  if (clip.parse(argc, argv)) { return 0; }

  // the real thing
  {
    boostjsn::object        state = clip.get_state();
    const boostjsn::string& greeting = state[stGreeting].as_string();
    std::string             text{greeting.cbegin(), greeting.cend()};

    text += ' ';
    text += state[stGreeted].as_string().c_str();

    clip.to_return(std::move(text));
    clip.return_state(std::move(state));
  }

  return 0;
}


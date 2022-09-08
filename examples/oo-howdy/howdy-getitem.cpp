// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <boost/json.hpp>
#include "clippy/clippy.hpp"


namespace boostjsn = boost::json;

static const std::string className = "Greeter";
static const std::string methodName = "__getitem__";
static const std::string expr = "expressions";
static const std::string selLetters  = "letters";
static const std::string selFieldAll  = "all";
static const std::string selFieldVowels  = "vowels";
static const std::string selFieldConsonants  = "consonants";
static const std::string stGreeting = "greeting";
static const std::string stGreeted  = "greeted";
static const std::string stSelected = "selected";

int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{methodName, "Selector example"};

  clip.member_of(className, "Customizable Greeting Generator");
  
  clip.add_required<std::vector<boost::json::object>>(expr, "Expression selection");
  clip.add_selector<std::string>(selLetters, "selector for letters of the greeting");
  //~ clip.returns<std::vector<std::string>>("Expression result");
  
  if (clip.parse(argc, argv)) { return 0; }
  
  // the real thing
  try
  { 
    using JsonExpression = std::vector<boost::json::object>;
    
    std::string    greeting = clip.get_state<std::string>(stGreeting);
    std::string    greeted  = clip.get_state<std::string>(stGreeted);
    JsonExpression jsonExpression = clip.get<JsonExpression>(expr);
    clippy::object res;
    clippy::object clippy_type;
    clippy::object state;
    
    state.set_val(stGreeting, std::move(greeting));
    state.set_val(stGreeted,  std::move(greeted));
    state.set_val(stSelected, std::move(jsonExpression));
    
    clippy_type.set_val("__class__", className);
    clippy_type.set_json("state",    std::move(state));
    
    res.set_json("__clippy_type__",  std::move(clippy_type));    
    clip.to_return(std::move(res));
  }
  catch (const std::exception& err)
  {
    clip.to_return(err.what());
    error_code = 1;
  }

  return error_code;
}



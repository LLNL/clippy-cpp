// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <boost/json.hpp>
#include "clippy/clippy.hpp"


namespace boostjsn = boost::json;

static const std::string methodName = "__getitem__";
static const std::string expr = "expressions";
static const std::string selLetters  = "letters";
static const std::string selFieldAll  = "all";
static const std::string selFieldVowels  = "vowels";
static const std::string selFieldConsonants  = "consonants";

int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{methodName, "Initializes a Greeter object"};

  clip.member_of("Greeter", "Customizable Greeting Generator");
  
  clip.add_required<std::vector<boost::json::object>>(expr, "Expression selection");
  //~ clip.add_required<boost::json::object>(expr, "Expression selection");
  //~ clip.add_required<clippy::object>(expr, "Expression selection");
  clip.add_selector<std::string>(selLetters, "selector for letters of the greeting");
  clip.returns<std::vector<std::string>>("Expression result");
  
  if (clip.parse(argc, argv)) { return 0; }
  
  // the real thing
  try
  { 
    std::vector<boost::json::object> json_expression = clip.get<std::vector<boost::json::object>>(expr);
    //~ boost::json::object json_expression = clip.get<boost::json::object>(expr);
    //~ clippy::object json_expression = clip.get<clippy::object>(expr);
    
    /*
    std::ofstream logfile{"clippy.log", std::ofstream::app};
    logfile << "json_expression:" << json_expression[1] << std::endl;
    */

    // TODO actually filter the greeter state strings (greeting and greeted) based on the json_expression
    //      and return a new Greeter with the state set based on the results
    //
    //      for now just return a vector to complete this cycle
    auto result = std::vector<std::string>();
    result.push_back("a");
    result.push_back("b");
    result.push_back("c");
    clip.to_return(result);
  }
  catch (const std::exception& err)
  {
    clip.to_return(err.what());
    error_code = 1;
  }

  return error_code;
}



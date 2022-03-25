// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include "jsonlines-common.hpp"

static const std::string METHOD_NAME = "__init__";

int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{METHOD_NAME, "Initializes a JsonLines object"};

  clip.member_of(CLASS_NAME, "A JsonLines class");

  clip.add_required<std::string>(ST_METALL_LOCATION, "Location of the Metall store");
  //~ clip.add_required<std::string>(ST_JSONLINES_KEY,  "Name of the JsonLines object within the Metall store");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) { return 0; }

  // the real thing
  try
  {
    // try to create the object
    std::string               location  = clip.get<std::string>(ST_METALL_LOCATION);
    //~ std::string             key       = clip.get<std::string>(ST_JSONLINES_KEY);

    metall::manager           manager(metall::create_only, location.c_str());
    const auto*               vec = manager.construct<vector_json_type>(metall::unique_instance)(manager.get_allocator());

    if (vec == nullptr)
      throw std::runtime_error("unable to allocate a JsonLines-Object in Metall");

    // set the return values
    clip.set_state(ST_METALL_LOCATION, std::move(location));
    //~ clip.set_state(ST_JSONLINES_KEY,  std::move(key));
  }
  catch (const std::runtime_error& ex)
  {
    error_code = 1;
    clip.to_return(ex.what());
  }

  return error_code;
}



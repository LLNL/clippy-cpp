// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"

#include "jsonlines-common.hpp"

namespace metaljsn = metall::container::experimental::json;

const std::string ARG_IMPORTED = "Json file";
const std::string METHOD_NAME  = "importFile";

int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{METHOD_NAME, "Imports Json Data from a file into the JsonLines objects"};

  clip.member_of(CLASS_NAME, "A JsonLines class");

  clip.add_required<std::string>(ARG_IMPORTED, "A JSON file.");
  clip.add_required_state<std::string>(ST_METALL_LOCATION, "Metall storage location");
  //~ clip.add_required_state<std::string>(ST_JSONLINES_KEY,  "Name of the JsonLines object");

  if (clip.parse(argc, argv)) { return 0; }

  try
  {
    std::string        filename = clip.get<std::string>(ARG_IMPORTED);
    std::string        location = clip.get_state<std::string>(ST_METALL_LOCATION);
    //~ std::string      key = clip.get_state<std::string>(ST_JSONLINES_KEY);
    metall::manager    manager(metall::open_only, location.c_str());
    vector_json_type*  vec = manager.find<vector_json_type>(metall::unique_instance).first;

    if (vec == nullptr)
      throw std::runtime_error("Unable to open JsonObject");

    std::ifstream      inp(filename);
    std::string        line;
    int                imported = 0;
    int                initialSize = vec->size();

    while (std::getline(inp, line))
    {
      ++imported;
      vec->emplace_back(metaljsn::parse(line, manager.get_allocator()));
    }

    assert(int(vec->size()) == initialSize + imported);

    std::stringstream  msg;

    msg << imported << " rows imported" << std::flush;
    clip.to_return(msg.str());
  }
  catch (const std::exception& err)
  {
    error_code = 1;
    clip.to_return(err.what());
  }

  return error_code;
}

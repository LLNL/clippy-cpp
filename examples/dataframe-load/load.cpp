// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include "df-common.hpp"

using ColumnDescription = std::pair<std::string, std::string>;

const std::string& type(const ColumnDescription& col) { return col.second; } 
const std::string& name(const ColumnDescription& col) { return col.first; } 

static const std::string FUNCTION_NAME = "load";
static const std::string ARG_COLUMN_DESC = "columns";


int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{FUNCTION_NAME, "Returns a Dataframe object"};

  clip.add_required<std::string>(ST_METALL_LOCATION, "Location of the Metall store");
  clip.add_required<std::string>(ST_DATAFRAME_NAME,  "Name of the dataframe object within the Metall store");
    
  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) { return 0; }

  // the real thing
  try
  {
    // try to create the object
    std::string    location  = clip.get<std::string>(ST_METALL_LOCATION);
    std::string    key       = clip.get<std::string>(ST_DATAFRAME_NAME);
    clippy::object res;
    clippy::object clippy_type;
    clippy::object state;
    
    state.set_val(ST_METALL_LOCATION, std::move(location));
    state.set_val(ST_DATAFRAME_NAME,  std::move(key));
    
    clippy_type.set_val("__class__", "Dataframe");
    clippy_type.set_json("state",     std::move(state));
    
    res.set_json("__clippy_type__",   std::move(clippy_type));    
    clip.to_return(std::move(res));
  }
  catch (const std::runtime_error& ex)
  {
    error_code = 1;
    clip.to_return(ex.what());
  }

  return error_code;
}



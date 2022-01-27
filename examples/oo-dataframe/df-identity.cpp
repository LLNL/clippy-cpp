#include <iostream>

#include "clippy/clippy.hpp"
#include "experimental/dataframe.hpp"
#include "df-common.hpp"

namespace xpr = experimental;

const std::string METHOD_NAME         = "identity";

int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{METHOD_NAME, "Initializes a Dataframe object"};

  clip.member_of("Dataframe", "A dataframe");

  clip.add_required_state<std::string>(ST_METALL_LOCATION, "Metall storage location");
  clip.add_required_state<std::string>(ST_DATAFRAME_NAME,  "Name of the dataframe object");

  if (clip.parse(argc, argv)) { return 0; }

  try
  {  
    std::string    location = clip.get_state<std::string>(ST_METALL_LOCATION);
    std::string    key = clip.get_state<std::string>(ST_DATAFRAME_NAME);
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
  catch (const std::exception& err)
  {
    clip.to_return(err.what());
    error_code = 1;    
  }

  return error_code;
}

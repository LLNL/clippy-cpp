#include <iostream>
#include <fstream>
#include <functional>
#include <numeric>

#include "clippy/clippy.hpp"
#include "df-common.hpp"

namespace xpr = experimental;

const std::string METHOD_NAME = "metadata";
                                                    
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
    std::string                     location = clip.get_state<std::string>(ST_METALL_LOCATION);
    std::string                     key = clip.get_state<std::string>(ST_DATAFRAME_NAME);
    std::unique_ptr<xpr::DataFrame> dfp = makeDataFrame(false, location, key);
    xpr::DataFrame&                 df  = *dfp;

    std::vector<int>                colIndcs = allColumnIndices(df);
    std::vector<xpr::ColumnDesc>    colDescs = df.get_column_descriptors(colIndcs);
    std::vector<std::string>        colNames = df.get_column_names();
    
    assert(colDescs.size() == colNames.size());

    std::stringstream               msg;
    
    msg << "{";
    
    for (size_t i = 0; i < colNames.size(); ++i)
    {
      msg << (i == 0 ? "\n" : ",\n")
          << "  [ \"name\" : \"" << colNames.at(i) << "\", "
          << "\"type\" : \"" << colDescs.at(i).column_type << "\", " 
          << "\"sparse\" : \"" << (colDescs.at(i).is_sparse_column ? "true" : "false") << "\"]"; 
    }
    
    msg << "\n}";
    
    clip.to_return(msg.str());
  }
  catch (const std::exception& err)
  {
    clip.to_return(err.what());
    error_code = 1;
  }
  
  return error_code;
}

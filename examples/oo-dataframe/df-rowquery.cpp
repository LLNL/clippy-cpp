#include <iostream>

#include "clippy/clippy.hpp"
#include "experimental/dataframe.hpp"
#include "df-common.hpp"

namespace xpr = experimental;

const std::string METHOD_NAME         = "rowquery";
const std::string ARG_ROWID_ARRAY     = "rows";

template <class ColType>
bool printVariant(clippy::object& data, const std::string& colname, const xpr::dataframe_variant_t& el)
{
  if (const auto& val = std::get_if<ColType>(&el))
  {
    data.set_val(colname, *val);
    return true;
  }
  
  return false;
}

void setVariant(clippy::object& data, const std::string& colname, const xpr::dataframe_variant_t& el)
{
  FirstMatch
  || printVariant<xpr::int_t>   (data, colname, el)
  || printVariant<xpr::uint_t>  (data, colname, el)
  || printVariant<xpr::real_t>  (data, colname, el)
  || printVariant<xpr::string_t>(data, colname, el)
  || fail("unknown dataframe variant type");
}


int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{METHOD_NAME, "Initializes a Dataframe object"};

  clip.member_of("Dataframe", "A dataframe");

  clip.add_required_state<std::string>(ST_METALL_LOCATION, "Metall storage location");
  clip.add_required_state<std::string>(ST_DATAFRAME_NAME,  "Name of the dataframe object");

  clip.add_required<std::vector<int>>(ARG_ROWID_ARRAY, "array of row ids");

  if (clip.parse(argc, argv)) { return 0; }

  try
  {  
    std::string                     location = clip.get_state<std::string>(ST_METALL_LOCATION);
    std::string                     key = clip.get_state<std::string>(ST_DATAFRAME_NAME);
    std::unique_ptr<xpr::DataFrame> dfp = makeDataFrame(false, location, key);
    xpr::DataFrame&                 df  = *dfp;

    std::vector<int>                selected = clip.get<std::vector<int>>(ARG_ROWID_ARRAY);
    
    std::vector<int>                colIndcs = allColumnIndices(df);
    std::vector<xpr::ColumnDesc>    colDescs = df.get_column_descriptors(colIndcs);
    std::vector<std::string>        colNames = df.get_column_names();
    
    clippy::array                   res;

    for (int row : selected)
    {
      clippy::object elem;
      
      std::vector<xpr::dataframe_variant_t> rowval = df.get_row_variant(row, colIndcs);
      
      if (colNames.size() != rowval.size())
        fail("internal error: size mismatch between columns and column names"); 
      
      elem.set_val("id", row);      
      for (size_t i = 0; i < colNames.size(); ++i)
        setVariant(elem, colNames.at(i), rowval.at(i)); 
      
      res.append_json(std::move(elem));
    }
    
    clip.to_return(std::move(res));
  }
  catch (const std::exception& err)
  {
    clip.to_return(err.what());
    error_code = 1;    
  }

  return error_code;
}

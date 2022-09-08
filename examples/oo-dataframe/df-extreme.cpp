#include <iostream>
#include <boost/algorithm/string.hpp>

#include "clippy/clippy.hpp"
#include "df-common.hpp"

namespace xpr = experimental;

const std::string METHOD_NAME         = "extreme";
const std::string ARG_CRITERIA_FUN    = "function";
const std::string ARG_CRITERIA_COL    = "column";

/*
template <class ColType>
std::string quote(const ColType*) { return ""; }

template<>
std::string quote<xpr::string_t>(const xpr::string_t*) { return "\""; }
*/

int columnIndex(const std::vector<std::string>& all, const std::string& colname)
{
  const std::vector<std::string>::const_iterator aa  = all.begin();
  const std::vector<std::string>::const_iterator zz  = all.end();
  std::vector<std::string>::const_iterator       pos = std::find(aa, zz, colname);

  if (pos == zz)
    fail("Column name not found: " + colname);

  return std::distance(aa, pos);
}

template <class ColType>
bool
applyColFunctionIf(clippy::object& res, xpr::DataFrame& df, size_t colnum, const xpr::ColumnDesc& desc, const std::string& funname)
{
  using Iterator = xpr::AnyColumnIterator<ColType>;

  if (desc.column_type != xpr::to_string(xpr::tag<ColType>()))
    return false;

  std::pair<Iterator, Iterator> range = df.get_any_column<ColType>(colnum);
  
  Iterator pos = (funname == "min") ? std::min_element(range.first, range.second)
                                    : std::max_element(range.first, range.second)
                                    ;
  
  res.set_val("id",    pos.row());
  res.set_val("value", *pos);                                   
  return true;
}

clippy::object
columnFunction(xpr::DataFrame& df, int colnum, const xpr::ColumnDesc& desc, const std::string& funname)
{
  clippy::object res;
  
  FirstMatch
  || applyColFunctionIf<xpr::int_t>   (res, df, colnum, desc, funname)
  || applyColFunctionIf<xpr::uint_t>  (res, df, colnum, desc, funname)
  || applyColFunctionIf<xpr::real_t>  (res, df, colnum, desc, funname)
  || applyColFunctionIf<xpr::string_t>(res, df, colnum, desc, funname)
  || fail("internal error: unexpected column type" + desc.column_type);
  
  return res;
}

std::string validateFunction(std::string func)
{
  boost::to_lower(func);

  if ((func != "min") && (func != "max"))
    fail("Invalid extreme function: " + func); 

  return func;
}

int main(int argc, char** argv)
{
  int            error_code = 0;
  clippy::clippy clip{METHOD_NAME, "Initializes a Dataframe object"};

  clip.member_of("Dataframe", "A dataframe");

  clip.add_required_state<std::string>(ST_METALL_LOCATION, "Metall storage location");
  clip.add_required_state<std::string>(ST_DATAFRAME_NAME,  "Name of the dataframe object");
  clip.add_required<std::string>(ARG_CRITERIA_FUN, "function name (min|max)");
  clip.add_required<std::string>(ARG_CRITERIA_COL, "column name");

  if (clip.parse(argc, argv)) { return 0; }

  try
  {
    std::string                     location = clip.get_state<std::string>(ST_METALL_LOCATION);
    std::string                     key = clip.get_state<std::string>(ST_DATAFRAME_NAME);
    std::unique_ptr<xpr::DataFrame> dfp = makeDataFrame(false, location, key);
    xpr::DataFrame&                 df  = *dfp;

		std::string                     funname = validateFunction(clip.get<std::string>(ARG_CRITERIA_FUN));
    std::string                     colname = clip.get<std::string>(ARG_CRITERIA_COL);
    int                             col      = columnIndex(df.get_column_names(), colname);
		std::vector<xpr::ColumnDesc>    colDescs = df.get_column_descriptors(std::vector<int>{col});

    if (colDescs.size() != 1)
      fail("internal error: unexpected number of columns");
      
    clippy::object                  res = columnFunction(df, col, colDescs.front(), funname);
    
    clip.to_return(std::move(res));
  }
  catch (const std::exception& err)
  {
    clip.to_return(err.what());
    error_code = 1;
  }

  return error_code;
}

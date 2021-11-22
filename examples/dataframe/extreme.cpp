#include <iostream>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>

#include "experimental/dataframe.hpp"
#include "experimental/csv-io.hpp"

#include "clip.hpp"
#include "common.hpp"

namespace boostjsn = boost::json;
namespace xpr = experimental;


const char* const METHOD_NAME         = "extreme";
const char* const ARG_METALL_LOCATION = "metall_location";
const char* const ARG_DATAFRAME_KEY   = "dataframe_key";
const char* const ARG_CRITERIA_FUN    = "function";
const char* const ARG_CRITERIA_COL    = "column";


void apiJason(std::ostream& out)
{
  std::size_t pos = 0;

  out << "{\n"
      << "  \"method_name\": \"" << METHOD_NAME << "\",\n"
      << "  \"args\":\n"
      << "  {\n";

      //~ << "      \"" << ARG_METALL_INIT << "\": { \"desc\": \"create or connect\", \"position\":0, \"required\":true, \"type\":\"string\"}\n"
  out << "      \"" << ARG_METALL_LOCATION << "\": { \"desc\": \"location of metall file\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"},\n";

  out << "      \"" << ARG_DATAFRAME_KEY << "\": { \"desc\": \"dataframe name\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"},\n";

  out << "      \"" << ARG_CRITERIA_COL << "\": { \"desc\": \"column where the criteria shall be applied\", \"position\":" << (pos++) << ", \"required\":false, \"type\":\"int\"},\n";

  out << "      \"" << ARG_CRITERIA_FUN << "\": { \"desc\": \"criteria (min, max)\", \"position\":" << (pos++) << ", \"required\":false, \"type\":\"int\"}\n";

  out << "  },\n"
      << "  \"returns\": { \"desc\": \"status\", \"type\":\"string\"}\n"
      << "}\n"
      << std::flush;
}


using DataConverter = std::function<xpr::dataframe_variant_t(const std::string& s)>;

template <class DfCellType>
struct ConverterFn
{
  xpr::dataframe_variant_t operator()(const std::string& s)
  {
    DfCellType elem = boost::lexical_cast<DfCellType>(s);

    return elem;
  }

  xpr::DataFrame* df;
};



template <class ColType>
std::string quote(const ColType*) { return ""; }

template<>
std::string quote<xpr::string_t>(const xpr::string_t*) { return "\""; }


template <class ColType>
bool
applyColFunctionIf(std::ostream& os, xpr::DataFrame& df, size_t colnum, const xpr::ColumnDesc& desc, const std::string& funname)
{
  using Iterator = xpr::AnyColumnIterator<ColType>;

  if (desc.column_type != xpr::to_string(xpr::tag<ColType>()))
    return false;

  std::pair<Iterator, Iterator> range = df.get_any_column<ColType>(colnum);
  std::string                   quotes = quote(xpr::tag<ColType>());

  Iterator pos = (funname == "min") ? std::min_element(range.first, range.second)
                                    : std::max_element(range.first, range.second)
                                    ;

  os << "{ \"id\": " << pos.row() << ", \"value\": " << quotes << *pos << quotes
     << "}"
     << std::endl;

  return true;
}

void columnFunction(std::ostream& os, xpr::DataFrame& df, int colnum, const xpr::ColumnDesc& desc, const std::string& funname)
{
  firstMatch
  || applyColFunctionIf<xpr::int_t>   (os, df, colnum, desc, funname)
  || applyColFunctionIf<xpr::uint_t>  (os, df, colnum, desc, funname)
  || applyColFunctionIf<xpr::real_t>  (os, df, colnum, desc, funname)
  || applyColFunctionIf<xpr::string_t>(os, df, colnum, desc, funname)
  || fail("internal error: unexpected column type" + desc.column_type);
}


int main(int argc, char** argv)
{
  std::string arg;

  if (argc > 1) arg = argv[1];

  if (arg == std::string{"--clippy-help"})
  {
    apiJason(std::cout);
    return 0;
  }

  boostjsn::value input = parseFile(std::cin);

  try
  {
    const bool                      make_new_partition  = false;
    std::string                     persistent_location;
    std::string                     dataframe_key;
    std::string                     funname;
    std::string                     colname;

    setValueIfAvail(input, ARG_METALL_LOCATION, persistent_location);
    setValueIfAvail(input, ARG_DATAFRAME_KEY,   dataframe_key);
    setValueIfAvail(input, ARG_CRITERIA_COL,    colname);
    setValueIfAvail(input, ARG_CRITERIA_FUN,    funname);

    clippy_assert(funname == "min" || funname == "max", std::string{"Invalid filter function name: "} + funname);

    std::unique_ptr<xpr::DataFrame> dfp      = makeDataFrame(make_new_partition, persistent_location, dataframe_key);
    xpr::DataFrame&                 df       = *dfp;
    int                             col      = columnIndex(df.get_column_names(), colname);
    std::vector<xpr::ColumnDesc>    colDescs = df.get_column_descriptors(std::vector<int>{col});

    clippy_assert(colDescs.size() == 1, "internal error: unexpected number of columns");

    columnFunction(std::cout, df, col, colDescs.front(), funname);
  }
  catch (const std::exception& err)
  {
    errorJson(std::cout, err.what());
  }

  return 0;
}

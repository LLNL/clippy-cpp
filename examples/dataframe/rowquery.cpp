#include <iostream>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>

#include "experimental/dataframe.hpp"
#include "experimental/csv-io.hpp"

#include "clip.hpp"
#include "common.hpp"

namespace boostjsn = boost::json;
namespace xpr = experimental;


const char* const METHOD_NAME         = "rowquery";
const char* const ARG_METALL_LOCATION = "metall_location";
const char* const ARG_DATAFRAME_KEY   = "dataframe_key";
const char* const ARG_ID_ARRAY        = "rows";
const char* const ARG_ID_VALUE        = "rowid";


void apiJason(std::ostream& out)
{
  std::size_t pos = 0;

  out << "{\n"
      << "  \"method_name\": \"" << METHOD_NAME << "\",\n"
      << "  \"args\":\n"
      << "  {\n"
      //~ << "      \"" << ARG_METALL_INIT << "\": { \"desc\": \"create or connect\", \"position\":0, \"required\":true, \"type\":\"string\"}\n"
      << "      \"" << ARG_METALL_LOCATION << "\": { \"desc\": \"location of metall file\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"},\n";
      
  out << "      \"" << ARG_DATAFRAME_KEY << "\": { \"desc\": \"dataframe name\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"},\n";
  
  out << "      \"" << ARG_ID_ARRAY << "\":\n"
      << "             {\n"
      << "               \"" << ARG_ID_VALUE << "\": { \"desc\": \"row id\", \"required\":false, \"type\":\"int\" } \n"
      << "             }\n"
      << "  },\n"
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


std::vector<int>
columnIDs(const boostjsn::value& args)
{
  std::vector<int>                 res;
  const boostjsn::object&          argsobj = args.as_object();

  boostjsn::object::const_iterator pos     = argsobj.find(ARG_ID_ARRAY);
  if (pos == argsobj.end()) throw std::runtime_error{"Row IDs undefined"};
  
  const boostjsn::array&           columns = pos->value().as_array();
  
  for (const boostjsn::value& col : columns)
    res.emplace_back(col.as_int64());
  
  return res;
}

template <class ColType>
std::string quote(const ColType*) { return ""; }

template<>
std::string quote<xpr::string_t>(const xpr::string_t*) { return "\""; }


template <class ColType>
bool printVariant(std::ostream& os, const xpr::dataframe_variant_t& el)
{
  if (auto val = std::get_if<ColType>(&el))
  {
    std::string quotes = quote(xpr::tag<ColType>());
    
    os << quotes << *val << quotes;
    return true;
  }
  
  return false;
}

struct VariantWriter
{
  xpr::dataframe_variant_t el;
};

std::ostream& operator<<(std::ostream& os, const VariantWriter& v)
{
  firstMatch
  || printVariant<xpr::int_t>   (os, v.el)
  || printVariant<xpr::uint_t>  (os, v.el)
  || printVariant<xpr::real_t>  (os, v.el)
  || printVariant<xpr::string_t>(os, v.el)
  || fail("unknown dataframe variant type");
  
  return os;
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
    std::string                     persistent_location;
    std::string                     dataframe_key;
    const bool                      make_new_partition  = false;
    
    setValueIfAvail(input, ARG_METALL_LOCATION, persistent_location);
    setValueIfAvail(input, ARG_DATAFRAME_KEY,   dataframe_key);
    
    std::vector<int>                selected = columnIDs(input);
    
    std::unique_ptr<xpr::DataFrame> dfp = makeDataFrame(make_new_partition, persistent_location, dataframe_key);
    xpr::DataFrame&                 df  = *dfp;
    
    std::vector<int>                colIndcs = allColumnIndices(df);
    std::vector<xpr::ColumnDesc>    colDescs = df.get_column_descriptors(colIndcs);
    std::vector<std::string>        colNames = df.get_column_names();
    bool                            firstrow = true;

    std::cout << "{\n \"data\": [";
    
    for (int row : selected)
    {
      std::vector<xpr::dataframe_variant_t> rowval = df.get_row_variant(row, colIndcs);
      
      clippy_assert(colNames.size() == rowval.size(), "internal error: size mismatch between columns and column names"); 
      
      std::cout << (firstrow ? "" : ",\n  ")
                << "{ \"id\": " << row;
                
      for (size_t i = 0; i < colNames.size(); ++i)
        std::cout << ", \"" << colNames.at(i) << "\": " << VariantWriter{rowval.at(i)};
      
      std::cout << "}";
      firstrow = false;
    }
    std::cout << "\n]}" << std::endl;
  }
  catch (const std::exception& err)
  {
    errorJson(std::cout, err.what());
  }

  return 0;
}

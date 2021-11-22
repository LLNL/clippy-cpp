#include <iostream>
#include <fstream>
#include <functional>
#include <variant>
#include <numeric>

#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>

#include "experimental/dataframe.hpp"
#include "experimental/csv-io.hpp"

#include "clip.hpp"
#include "common.hpp"

namespace boostjsn = boost::json;
namespace xpr = experimental;


const char* const METHOD_NAME         = "metadataquery";
const char* const ARG_METALL_LOCATION = "metall_location";
const char* const ARG_DATAFRAME_KEY   = "dataframe_key";

                                                    

void apiJason(std::ostream& out)
{
  std::size_t pos = 0;
  
  out << "{\n"
      << "  \"method_name\": \"" << METHOD_NAME << "\",\n"
      << "  \"args\":\n"
      << "  {\n"
      //~ << "      \"" << ARG_METALL_INIT << "\": { \"desc\": \"create or connect\", \"position\":0, \"required\":true, \"type\":\"string\"}\n"
      << "      \"" << ARG_METALL_LOCATION << "\": { \"desc\": \"location of metall file\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"},\n";
      
  out << "      \"" << ARG_DATAFRAME_KEY << "\": { \"desc\": \"dataframe name\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"}\n";
  
  //~ out << "      \"" << ARG_FILE << "\": { \"desc\": \"path to CSV file\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"},\n"      
  out << "  },\n"
      << "  \"returns\": { \"desc\": \"status\", \"type\":\"string\"}\n"
      << "}\n"
      << std::flush;
}


int main(int argc, char** argv)
{
  std::string arg;
  int         result_status = 0;

  if (argc > 1) arg = argv[1];

  if (arg == std::string{"--clippy-help"})
  {
    apiJason(std::cout);
    return 0;
  }
  
  //~ boostjsn::value input = (argc > 1) ? parse_file(std::cin) : parse_file(arg);
  boostjsn::value input = parseFile(std::cin);
  
  
  try
  {  
    std::string persistent_location;
    std::string dataframe_key;
    const bool  make_new_partition  = false;  

    setValueIfAvail(input, ARG_METALL_LOCATION, persistent_location);
    setValueIfAvail(input, ARG_DATAFRAME_KEY,   dataframe_key);
    

    std::unique_ptr<xpr::DataFrame> dfp = makeDataFrame(make_new_partition, persistent_location, dataframe_key);
    xpr::DataFrame&                 df  = *dfp;
    std::vector<int>                colIndcs = allColumnIndices(df);
    std::vector<xpr::ColumnDesc>    colDescs = df.get_column_descriptors(colIndcs);
    std::vector<std::string>        colNames = df.get_column_names();
    
    assert(colDescs.size() == colNames.size());
    
    std::cout << "{";
    
    for (size_t i = 0; i <= colNames.size(); ++i)
    {
      std::cout << (i == 0 ? "\n" : ",\n")
                << "  [ \"name\" : \"" << colNames.at(i) << "\", "
                << "\"type\" : \"" << colDescs.at(i).column_type << "\", " 
                << "\"sparse\" : \"" << (colDescs.at(i).is_sparse_column ? "true" : "false") << "\"]"; 
    }
    
    std::cout << "\n}";
    
    
  }
  catch (const std::exception& err)
  {
    errorJson(std::cout, err.what());
    result_status = 1;
  }
  

  //~ return result_status;
  return 0;
}

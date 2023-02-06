#include <iostream>
#include <sstream>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>

#include "experimental/dataframe.hpp"
#include "experimental/csv-io.hpp"

#include "clip.hpp"
#include "common.hpp"

namespace boostjsn = boost::json;
namespace xpr = experimental;


const char* const ARG_FILE            = "csvfile";
//~ const char* const ARG_METALL_INIT     = "metall_init";
const char* const ARG_METALL_LOCATION = "metall_location";
const char* const ARG_DATAFRAME_KEY   = "dataframe_key";


void apiJason(std::ostream& out)
{
  std::size_t pos = 0;

  out << "{\n"
      << "  \"method_name\": \"importCSV\",\n"
      << "  \"args\":\n"
      << "  {\n"
      //~ << "      \"" << ARG_METALL_INIT << "\": { \"desc\": \"create or connect\", \"position\":0, \"required\":true, \"type\":\"string\"}\n"
      << "      \"" << ARG_METALL_LOCATION << "\": { \"desc\": \"location of metall file\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"},\n";

  out << "      \"" << ARG_DATAFRAME_KEY << "\": { \"desc\": \"dataframe name\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"},\n";

  out << "      \"" << ARG_FILE << "\": { \"desc\": \"path to CSV file\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"}\n"
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
    //~ std::cerr << "in: " << s << " " << typeid(DfCellType).name() << std::flush;
    DfCellType elem = boost::lexical_cast<DfCellType>(s);
    //~ std::cerr << " ok " << std::endl;

    return elem;
  }

  xpr::DataFrame* df;
};

template <>
struct ConverterFn<xpr::string_t>
{
  xpr::dataframe_variant_t operator()(const std::string& s)
  {
    return df->persistent_string_std(s);
  }

  xpr::DataFrame* df;
};



template <class ColType>
bool push_conv_if(std::vector<DataConverter>& res, xpr::DataFrame& df, const xpr::ColumnDesc& coldesc)
{
  if (!coldesc.is<ColType>()) return false;

  res.push_back(ConverterFn<ColType>{&df});
  return true;
}

std::vector<DataConverter>
makeDataConverter(xpr::DataFrame& df)
{
  const std::vector<int>             colIndcs = allColumnIndices(df);
  const std::vector<xpr::ColumnDesc> colDescs = df.get_column_descriptors(colIndcs);
  std::vector<DataConverter>         res;

  for (const xpr::ColumnDesc& col : colDescs)
  {
    firstMatch
    || push_conv_if<xpr::int_t>   (res, df, col)
    || push_conv_if<xpr::uint_t>  (res, df, col)
    || push_conv_if<xpr::real_t>  (res, df, col)
    || push_conv_if<xpr::string_t>(res, df, col)
    || fail("invalid Column Name");
  }

  return res;
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
    std::string                     filename;
    std::string                     persistent_location;
    std::string                     dataframe_key;
    const bool                      make_new_partition  = false;

    setValueIfAvail(input, ARG_FILE,            filename);
    setValueIfAvail(input, ARG_METALL_LOCATION, persistent_location);
    setValueIfAvail(input, ARG_DATAFRAME_KEY,   dataframe_key);

    if (filename.size() == 0)
      throw std::runtime_error("Input file (CSV) not provided");

    std::unique_ptr<xpr::DataFrame> dfp = makeDataFrame(make_new_partition, persistent_location, dataframe_key);
    xpr::DataFrame&                 df  = *dfp;
    int                             numrows = df.rows();

    xpr::importCSV_variant(df, filename, makeDataConverter(df));

    std::stringstream               msg;

    msg << (df.rows()-numrows) << " records imported" << std::flush;

    resultOK(std::cout, msg.str());
  }
  catch (const std::exception& err)
  {
    errorJson(std::cout, err.what());
  }

  return 0;
}

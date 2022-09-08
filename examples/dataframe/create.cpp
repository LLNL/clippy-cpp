#include <iostream>
#include <fstream>
#include <functional>
#include <variant>

#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>

#include "experimental/dataframe.hpp"
#include "experimental/csv-io.hpp"

#include "clip.hpp"
#include "common.hpp"

namespace boostjsn = boost::json;
namespace xpr = experimental;


const char* const METHOD_NAME         = "create";
const char* const ARG_METALL_LOCATION = "metall_location";
const char* const ARG_DATAFRAME_KEY   = "dataframe_key";
const char* const ARG_COLUMN_ARRAY    = "columns";
const char* const ARG_COLUMN_TYPE     = "type";
const char* const ARG_COLUMN_NAME     = "name";


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

  //~ out << "      \"" << ARG_FILE << "\": { \"desc\": \"path to CSV file\", \"position\":" << (pos++) << ", \"required\":true, \"type\":\"string\"},\n"
  out << "      \"" << ARG_COLUMN_ARRAY << "\":\n"
      << "             {\n"
      << "               \"" << ARG_COLUMN_TYPE << "\": { \"desc\": \"column type (int|uint|real|bool)\", \"required\":false, \"type\":\"string\" }, \n"
      << "               \"" << ARG_COLUMN_NAME << "\": { \"desc\": \"column name\", \"required\":false, \"type\":\"string\" } \n"
      << "             }\n"
      << "  },\n"
      << "  \"returns\": { \"desc\": \"status\", \"type\":\"string\"}\n"
      << "}\n"
      << std::flush;
}



void setColumnDesc(const boostjsn::value& args, std::string& type, std::string& name)
{
  static const std::string UNDEF_COLUMN_TYPE{"column type undefined"};

  const boostjsn::array&          argsobj = args.as_array();
  boostjsn::array::const_iterator pos     = argsobj.begin();
  boostjsn::array::const_iterator zz      = argsobj.end();

  if (pos == zz) throw std::runtime_error{UNDEF_COLUMN_TYPE};
  type = pos->as_string().c_str();

  ++pos;
  if (pos == zz) return;

  name = pos->as_string().c_str();
}

struct ColumnDesc : std::tuple<std::string, std::string>
{
  using base = std::tuple<std::string, std::string>;
  using base::base;

  /*
  std::string& type() { return std::get<0>(*this); }
  std::string& name() { return std::get<1>(*this); }
  */

  const std::string& type() const { return std::get<0>(*this); }
  const std::string& name() const { return std::get<1>(*this); }
};

std::vector<ColumnDesc>
columnDesc(const boostjsn::value& args)
{
  std::vector<ColumnDesc>          res;
  const boostjsn::object&          argsobj = args.as_object();
  boostjsn::object::const_iterator pos     = argsobj.find(ARG_COLUMN_ARRAY);
  if (pos == argsobj.end()) throw std::runtime_error{"Columns undefined"};

  const boostjsn::array&           columns = pos->value().as_array();

  for (const boostjsn::value& col : columns)
  {
    std::string name;
    std::string type;

    setColumnDesc(col, type, name);

    res.emplace_back(std::move(type), std::move(name));
  }

  return res;
}

/*
xpr::dataframe_variant_t
readString(const std::string& val)
{
  return xpr::string_t{val.c_str()};
}

xpr::dataframe_variant_t
readInt(const std::string& val)
{
  return boost::lexical_cast<xpr::int_t>(val);
}

xpr::dataframe_variant_t
readUint(const std::string& val)
{
  return boost::lexical_cast<xpr::uint_t>(val);
}

xpr::dataframe_variant_t
readReal(const std::string& val)
{
  return boost::lexical_cast<xpr::real_t>(val);
}
*/

void appendColumn(xpr::DataFrame& df, const ColumnDesc& desc)
{
  static const std::string UNKNOWN_COLUMN_TYPE{"unknown column type: "};

  if (desc.type() == "uint")
    df.add_column_default_value(xpr::dense<xpr::uint_t>{0});
  else if (desc.type() == "int")
    df.add_column_default_value(xpr::dense<xpr::int_t>{0});
  else if (desc.type() == "real")
    df.add_column_default_value(xpr::dense<xpr::real_t>{0});
  else if (desc.type() == "string")
    df.add_column_default_value(xpr::dense<xpr::string_t>{df.persistent_string("")});
  else
    throw std::runtime_error{UNKNOWN_COLUMN_TYPE + desc.name()};

  df.name_last_column(df.persistent_string(desc.name().c_str()));
}

void appendColumns(xpr::DataFrame& df, const std::vector<ColumnDesc>& cols)
{
  for (const ColumnDesc& desc : cols)
    appendColumn(df, desc);
}

/*
std::function<xpr::dataframe_variant_t(const std::string&)>
cellBuilder(const ColumnDesc& desc)
{
  if (desc.type() == "uint")
    return readUint;

  if (desc.type() == "int")
    return readInt;

  if (desc.type() == "real")
    return readReal;

  if (desc.type() == "string")
    return readString;

  throw std::logic_error("cell type check failed");
}

std::vector<std::function<xpr::dataframe_variant_t(const std::string&)> >
rowBuilder(const std::vector<ColumnDesc>& row)
{
  std::vector<std::function<xpr::dataframe_variant_t(const std::string&)> > res;

  for (const ColumnDesc& col : row)
    res.emplace_back(cellBuilder(col));

  return res;
}
*/

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

  //~ pretty_print(input);


  try
  {
    std::string persistent_location;
    std::string dataframe_key;

    setValueIfAvail(input, ARG_METALL_LOCATION, persistent_location);
    setValueIfAvail(input, ARG_DATAFRAME_KEY,   dataframe_key);

    std::vector<ColumnDesc> coldesc = columnDesc(input);
    const bool              make_new_partition  = true;

    std::unique_ptr<xpr::DataFrame> dfp = makeDataFrame(make_new_partition, persistent_location, dataframe_key);
    xpr::DataFrame&                 df  = *dfp;

    appendColumns(df, coldesc);

    //~ xpr::importCSV(df, "data/sample.csv", xpr::tag<data_record>());
    resultOK(std::cout);
  }
  catch (const std::exception& err)
  {
    errorJson(std::cout, err.what());
    result_status = 1;
  }


  //~ return result_status;
  return 0;
}
